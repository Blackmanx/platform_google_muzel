/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Google Inc.
 *
 * mtkisp7.cpp - IPA implementation for MtkISP7
 */

#include "mtkisp7.h"

#include <sys/resource.h>
#include <sys/wait.h>

#include "libcamera/internal/formats.h"

#include "halisp/hal_isp.h"
#include "libcamera/base/log.h"
#include "libcamera/control_ids.h"
#include "libcamera/controls.h"
#include "libcamera/ipa/ipa_module_info.h"
#include "pipeline/mtkisp7/hal3a/const.h"
#include "pipeline/mtkisp7/imgsys/const.h"
#include "platform/mtkisp7/cam_cal_helper.h"
#include "platform/mtkisp7/mtkcam-chrom/custom/mt8188/hal/inc/debug_exif/cam/dbg_cam_param.h"
#include "platform/mtkisp7/platform_utils.h"
#include "platform/mtkisp7/sensor/sensor_info.h"

#include "formats.h"

namespace libcamera {

LOG_DEFINE_CATEGORY(IPAMtkISP7)

namespace ipa::mtkisp7 {

namespace {
// TODO(chenghaoyang): Set a big core according to models.
// Ciri's big cores are CPU 6 and 7.
static const std::vector<int> k3AThreadCpuAffinity{ 0, 1, 2, 3, 4, 5 };
static const std::vector<int> kIspThreadCpuAffinity{ 0, 1, 2, 3, 4, 5 };
} // namespace

IPAMtkISP7::IPAMtkISP7()
	: aaaManager_(nullptr, Object::Deleter),
	  ispManager_(nullptr, Object::Deleter)
{
}

IPAMtkISP7::~IPAMtkISP7()
{
	stop();
}

int IPAMtkISP7::init(const std::string &model, const int32_t sensorIdx,
		     const std::vector<uint8_t> &eeprom,
		     const std::vector<ipa::mtkisp7::CamSysData> &camSysDataArray)
{
	adjustRLimit();
	PlatformUtils::setWithModelName(model);
	CamCalHelper::getInstance(sensorIdx)->setEepromData(eeprom);

	if (PlatformUtils::platform_ == PlatformUtils::MtkISP7Platform::NONE)
		LOG(IPAMtkISP7, Fatal) << "Invalid model: " << model;

	LOG(IPAMtkISP7, Debug)
		<< "Running on platform "
		<< PlatformUtils::enumToString(PlatformUtils::platform_)
		<< ", model: " << model;

	std::vector<SensorInfo::CamSysData> dataArray;
	for (const auto &data : camSysDataArray) {
		dataArray.emplace_back(data.has_af, data.mbus_code);
	}
	SensorInfo::add_sensor(dataArray);

	sensorIdx_ = sensorIdx;
	halIsp_ = std::make_unique<HalIsp>(&onDeviceTuner_);
	hal3A_ = std::make_unique<Hal3A>(sensorIdx, halIsp_.get(), &onDeviceTuner_);

	onDeviceTuner_.initialize(true);

	int32_t sensorDev = 0;
	switch (sensorIdx) {
	case 0:
		sensorDev = 1;
		break;
	case 1:
		sensorDev = 2;
		break;

	default:
		LOG(IPAMtkISP7, Fatal) << "Unsupported sensorIdx: " << sensorIdx;
		break;
	}
	halIsp_->init(sensorIdx, sensorDev, hal3A_.get());

	aieParser_ = std::make_unique<AieParser>();

	return 0;
}

/**
 * \brief Perform any processing required before the first frame
 */
void IPAMtkISP7::start(const uint32_t rawMetaBufferId,
		       SensorSetting *sensorSetting,
		       int32_t *lens_position)
{
	IPAMappedBuffer *rawMetaBuffer = getMappedBufferIter(rawMetaBufferId);
	if (!rawMetaBuffer) {
		LOG(IPAMtkISP7, Error) << "Could not find rawMeta buffer!";
		*lens_position = -1;
		return;
	}

	hal3A_->start(reinterpret_cast<mtk_cam_uapi_meta_raw_stats_cfg *>(
		rawMetaBuffer->mapped->planes()[0].data()));

	aaaThread_.start();
	aaaThread_.setThreadAffinity(k3AThreadCpuAffinity);
	aaaManager_.reset(new AAAManager(this));
	aaaManager_->moveToThread(&aaaThread_);

	ispThread_.start();
	ispThread_.setThreadAffinity(kIspThreadCpuAffinity);
	ispManager_.reset(new IspManager(this));
	ispManager_->moveToThread(&ispThread_);

	uint32_t exposureTimeMs;
	hal3A_->getExposureAndGain(sensorSetting, exposureTimeMs);
	*lens_position = hal3A_->r3AResult_.af_result.lens_position;
}

/**
 * \brief Ensure that all processing has completed
 */
void IPAMtkISP7::stop()
{
	aaaManager_.reset();

	if (aaaThread_.isRunning()) {
		aaaThread_.exit();
		aaaThread_.wait();
	}

	ispManager_.reset();

	if (ispThread_.isRunning()) {
		ispThread_.exit();
		ispThread_.wait();
	}
}

/**
 * \brief Configure the MtkISP7 IPA
 * \param[in] sensorIdx The index of the sensor being used now
 */
int IPAMtkISP7::configure(const Size &camsysYuvSize, const Size &maxVideoSize,
			  const Size &maxStillSize, const std::string &sensorId,
			  const uint32_t camsysIndex, const int32_t sessionTimestamp,
			  bool isVideo, const Size &sensorFullSize,
			  const Size &swmeAlignedSize,
			  Size *wrappingMapSize, Size *confMapSize)
{
	ImagiqAdapter::sensorIdMap.emplace(
		sensorId, NSCam::TuningUtils::eSensorId(sensorIdx_));

	sensorFullSize_ = sensorFullSize;
	swmeAlignedSize_ = swmeAlignedSize;

	onDeviceTuner_.configure(sensorId, camsysIndex, sessionTimestamp);

	// If only still capture stream is configured. Force 3A to progress
	// with still capture frames.
	bool force3AConsistency = false;
	if (maxVideoSize.isNull())
		force3AConsistency = true;

	hal3A_->configure(camsysYuvSize, maxVideoSize, isVideo, force3AConsistency);
	halIsp_->configure(maxVideoSize, maxStillSize, isVideo);

	bssWrapper_ = std::make_shared<BssWrapper>(sensorIdx_);
	bssWrapper_->bssInit(camsysYuvSize);

	int ret = aieParser_->initialize();
	if (ret != 0) {
		return ret;
	}
	aieParser_->configure();

	for (auto i = 0; i < kInputRawCount - 1; i++) {
		std::shared_ptr<SwmeWrapper> swmeWrapper = std::make_shared<SwmeWrapper>();
		swmeWrapper->setMotionEstimationResolution(swmeAlignedSize);
		swmeWrapper->init();
		swmeWrapper_.push_back(swmeWrapper);
	}

	*wrappingMapSize = swmeWrapper_[0]->getWarppingMapSize();
	*confMapSize = swmeWrapper_[0]->getConfMapSize();

	mfnrExifData_[MF_TAG_VERSION] = MF_DEBUG_TAG_VERSION;
	mfnrExifData_[MF_TAG_CAPTURE_M] = 4;
	mfnrExifData_[MF_TAG_BLENDED_N] = 4;
	mfnrExifData_[MF_TAG_AEVC_AE_EN] = 1;
	mfnrExifData_[MF_TAG_AEVC_LCSO_EN] = 1;
	mfnrExifData_[MF_TAG_MFNR_ISO_TH] = 90;
	mfnrExifData_[MF_TAG_MAX_FRAME_NUMBER] = 4;
	mfnrExifData_[MF_TAG_PROCESSING_NUMBER] = 3;
	mfnrExifData_[MF_TAG_RAW_WIDTH] = sensorFullSize.width;
	mfnrExifData_[MF_TAG_RAW_HEIGHT] = sensorFullSize.height;
	mfnrExifData_[MF_TAG_BLD_YUV_WIDTH] = sensorFullSize.width;
	mfnrExifData_[MF_TAG_BLD_YUV_HEIGHT] = sensorFullSize.height;
	mfnrExifData_[MF_TAG_P2_ME_IN_WIDTH] = swmeAlignedSize.width;
	mfnrExifData_[MF_TAG_P2_ME_IN_HEIGHT] = swmeAlignedSize.height;
	mfnrExifData_[MF_TAG_ME_IN_WIDTH] = swmeAlignedSize.width;
	mfnrExifData_[MF_TAG_ME_IN_HEIGHT] = swmeAlignedSize.height;

	return aieParser_->initialize();
}

/**
 * \brief Adjust the fd limit to 2048
 */
void IPAMtkISP7::adjustRLimit()
{
	struct rlimit rlim;

	if (getrlimit(RLIMIT_NOFILE, &rlim) != 0) {
		perror("getrlimit");
		exit(EXIT_FAILURE);
	}

	if (rlim.rlim_cur == RLIM_INFINITY)
		LOG(IPAMtkISP7, Info) << "Current file descriptor limit: unlimited ";
	else
		LOG(IPAMtkISP7, Info) << "Current file descriptor limit:" << rlim.rlim_cur;

	if (rlim.rlim_max == RLIM_INFINITY)
		LOG(IPAMtkISP7, Info) << "Maximum file descriptor limit: unlimited:";
	else
		LOG(IPAMtkISP7, Info) << "Maximum file descriptor limit: " << rlim.rlim_max;

	// Increase the soft limit to 2048
	rlim.rlim_cur = 2048;
	LOG(IPAMtkISP7, Info) << "rlim_cur:" << rlim.rlim_cur;

	if (setrlimit(RLIMIT_NOFILE, &rlim) != 0) {
		perror("setrlimit");
		exit(EXIT_FAILURE);
	}

	if (getrlimit(RLIMIT_NOFILE, &rlim) != 0) {
		perror("getrlimit");
		exit(EXIT_FAILURE);
	}
}

/**
 * \brief Map the parameters and stats buffers allocated in the pipeline handler
 * \param[in] buffers The buffers to map
 */
void IPAMtkISP7::mapBuffers(const std::vector<IPABuffer> &buffers)
{
	for (const IPABuffer &buffer : buffers) {
		buffers_.emplace(buffer.id, buffer.planes);
	}
}

/**
 * \brief Unmap the parameters and stats buffers
 * \param[in] ids The IDs of the buffers to unmap
 */
void IPAMtkISP7::unmapBuffers(const std::vector<unsigned int> &ids)
{
	for (unsigned int id : ids) {
		auto it = buffers_.find(id);
		if (it == buffers_.end())
			continue;

		buffers_.erase(it);
	}
}

void IPAMtkISP7::writeStillCaptureDebugMetadata(
	const uint32_t camSysMetaRequestId,
	ControlList *metadata)
{
	*metadata = controls::controls;
	onDeviceTuner_.writeStillCaptureDebugMetadata(*metadata,
						      hal3A_->resultHistory_.query(camSysMetaRequestId),
						      mfnrExifData_);
}

void IPAMtkISP7::notifyRequestBegin(const uint32_t baseFrame,
				    const uint32_t curFrame,
				    const bool hasStillCapture)
{
	onDeviceTuner_.notifyRequestBegin(curFrame);

	if (!hasStillCapture)
		onDeviceTuner_.notifyVideoOnly(curFrame);
	else
		onDeviceTuner_.notifyStillCapture(baseFrame, curFrame);
}

void IPAMtkISP7::notifyRequestEnd(const uint32_t frame)
{
	onDeviceTuner_.notifyRequestEnd(frame);
}

void IPAMtkISP7::notifyExportBegin(const uint32_t exportBegin,
				   const uint32_t exportEnd)
{
	onDeviceTuner_.notifyExportBegin(exportBegin, exportEnd);
}

void IPAMtkISP7::notifyImportBegin(const uint32_t importBegin,
				   const uint32_t importEnd)
{
	onDeviceTuner_.notifyImportBegin(importBegin, importEnd);
}

void IPAMtkISP7::aieParse(
	const uint32_t inputImageBufferId,
	const uint32_t faceDetectionMetadataBufferId,
	const uint32_t faceToneClassificationMetadataBufferId,
	const Size &currentSensorSize,
	const uint32_t camSysMetaRequestId)
{
	auto itInputImage = buffers_.find(inputImageBufferId);
	if (itInputImage == buffers_.end()) {
		LOG(IPAMtkISP7, Error) << "Could not find input image buffer!";
		return;
	}
	FrameBuffer *inputBuffer = &itInputImage->second.buffer;

	auto itFDMetadata = buffers_.find(faceDetectionMetadataBufferId);
	if (itFDMetadata == buffers_.end()) {
		LOG(IPAMtkISP7, Error) << "Could not find FD metadata buffer!";
		return;
	}
	FrameBuffer *faceMetatBuffer = &itFDMetadata->second.buffer;

	FrameBuffer *FTCMetadataFrameBuffer = nullptr;
	if (faceToneClassificationMetadataBufferId != 0) {
		auto itFTCMetadata = buffers_.find(faceToneClassificationMetadataBufferId);
		if (itFTCMetadata == buffers_.end()) {
			LOG(IPAMtkISP7, Error) << "Could not find FTC metadata buffer!";
			return;
		}

		FTCMetadataFrameBuffer = &itFTCMetadata->second.buffer;
	}

	PrimaryFaceData faceData;
	ControlList faceControls(controls::controls);
	int ret = aieParser_->doParse(inputBuffer, faceMetatBuffer, FTCMetadataFrameBuffer,
				      currentSensorSize, camSysMetaRequestId,
				      faceData, faceControls);
	if (ret) {
		LOG(IPAMtkISP7, Error) << "Failed to run Aie Parse: " << ret;
		AieParseResultReady.emit(false, faceData, faceControls);
		return;
	}

	aieParser_->getLatestOutput(latestFaceMetadata_);

	AieParseResultReady.emit(true, faceData, faceControls);
}

void IPAMtkISP7::doCalculation3A(const uint32_t frame,
				 const uint32_t stat0BufferId, const uint32_t stat1BufferId,
				 const uint64_t timestamp, const uint32_t camSysMetaRequestId,
				 const uint32_t afCamSysMetaRequestId,
				 const bool isStillCapture, const uint32_t rawMetaBufferId,
				 const GyroSampleData &gyroSample,
				 const uint32_t internalRequestIdApplied,
				 const int32_t featureEnum,
				 const VcmFocusInformation &vcmFocusInfo,
				 const ControlList &controls)
{
	auto itStat0 = buffers_.find(stat0BufferId);
	if (itStat0 == buffers_.end()) {
		LOG(IPAMtkISP7, Error) << "Could not find stat0 buffer!";
		return;
	}

	IPAMappedBuffer *rawMetaBuffer = getMappedBufferIter(rawMetaBufferId);
	if (!rawMetaBuffer) {
		LOG(IPAMtkISP7, Error) << "Could not find rawMeta buffer!";
		return;
	}

	GyroSensor::SensorSample sample;
	sample.x_value = gyroSample.x_value;
	sample.y_value = gyroSample.y_value;
	sample.z_value = gyroSample.z_value;
	sample.timestamp = gyroSample.timestamp;

	FrameBuffer *statistics1 = nullptr;
	::VcmFocusInformation vcm;
	if (stat1BufferId) {
		// TODO: use another thread.
		vcm.focus_position = vcmFocusInfo.focus_position;
		vcm.previous_focus_position = vcmFocusInfo.previous_focus_position;
		vcm.moving_timestamp = vcmFocusInfo.moving_timestamp;
		vcm.previous_moving_timestamp = vcmFocusInfo.previous_moving_timestamp;

		auto itStat1 = buffers_.find(stat1BufferId);
		if (itStat1 == buffers_.end()) {
			LOG(IPAMtkISP7, Error) << "Could not find stat1 buffer!";
			return;
		}
		statistics1 = &itStat1->second.buffer;
	}

	aaaManager_->invokeMethod(
		&IPAMtkISP7::AAAManager::doCalculation, ConnectionTypeQueued,
		&itStat0->second.buffer, statistics1, timestamp, frame,
		camSysMetaRequestId, afCamSysMetaRequestId, isStillCapture,
		rawMetaBuffer->buffer.planes()[0].fd.get(),
		rawMetaBuffer->mapped->planes()[0].data(), vcm,
		latestFaceMetadata_, sample, internalRequestIdApplied,
		controls, featureEnum);
}

IPAMtkISP7::IPAMappedBuffer *
IPAMtkISP7::getMappedBufferIter(unsigned int bufferId)
{
	auto it = buffers_.find(bufferId);
	if (it == buffers_.end())
		return nullptr;

	if (!it->second.mapped) {
		it->second.mapped = std::make_unique<MappedFrameBuffer>(
			&it->second.buffer,
			MappedFrameBuffer::MapFlag::ReadWrite);
	}

	return &it->second;
}

void IPAMtkISP7::getImgSysMetaTuning(
	const uint64_t cookie, const uint32_t camSysMetaRequestId,
	const uint32_t frame, const bool needCropTNC16x9,
	const uint32_t featureEnum,
	const std::vector<ipa::mtkisp7::ImgMetaRequestData> &imgMetaRequests,
	const ControlList &controls)
{
	std::vector<IPAMtkISP7::IspManager::DataMappedBuffers>
		dataMappedBuffersList(imgMetaRequests.size());

	for (uint32_t i = 0; i < imgMetaRequests.size(); ++i) {
		const auto &requestData = imgMetaRequests[i];
		auto &dataMappedBuffers = dataMappedBuffersList[i];
		dataMappedBuffers.valid = true;

		IPAMappedBuffer *tuningBuffer = getMappedBufferIter(
			requestData.tuningBufferId);
		if (!tuningBuffer) {
			LOG(IPAMtkISP7, Error) << "Could not find tuning buffer!";
			dataMappedBuffers.valid = false;
			continue;
		}
		dataMappedBuffers.tuning = tuningBuffer;

		if (requestData.statisticsBufferId > 0) {
			IPAMappedBuffer *statBuffer = getMappedBufferIter(
				requestData.statisticsBufferId);
			if (!statBuffer) {
				LOG(IPAMtkISP7, Error)
					<< "Could not find stat buffer!";
				dataMappedBuffers.valid = false;
				continue;
			}
			dataMappedBuffers.statistics = statBuffer;
		}

		IPAMappedBuffer *swHistBuffer = nullptr;
		if (requestData.swHistBufferId > 0) {
			swHistBuffer = getMappedBufferIter(requestData.swHistBufferId);
			if (!swHistBuffer) {
				LOG(IPAMtkISP7, Error)
					<< "Could not find swHist buffer!";
				dataMappedBuffers.valid = false;
				continue;
			}
			dataMappedBuffers.swHist = swHistBuffer;
		}

		for (const auto &[_, bufferId] : requestData.reserved) {
			IPAMappedBuffer *mappedBuffer = getMappedBufferIter(bufferId);
			if (!mappedBuffer) {
				LOG(IPAMtkISP7, Error)
					<< "Could not find reserved buffer: "
					<< bufferId;
				dataMappedBuffers.valid = false;
				break;
			}
			dataMappedBuffers.reserved.push_back(mappedBuffer);
		}
	}

	ispManager_->invokeMethod(
		&IPAMtkISP7::IspManager::getImgSysMetaTuning,
		ConnectionTypeQueued,
		cookie, camSysMetaRequestId, frame, needCropTNC16x9,
		static_cast<Feature>(featureEnum),
		imgMetaRequests, std::move(dataMappedBuffersList),
		controls);
}

void IPAMtkISP7::doSwme(
	const uint64_t cookie,
	const std::vector<ipa::mtkisp7::SwmeFramesData> &swmeFramesData)
{
	for (size_t i = 0; i < kInputRawCount - 1; i++) {
		std::shared_ptr<SwmeWrapper> swmewrapper = swmeWrapper_[i];

		if (i >= swmeFramesData.size()) {
			LOG(IPAMtkISP7, Error) << "Missing SwmeFramesData for index: " << i;
			break;
		}

		const SwmeFramesData &data = swmeFramesData[i];

		SwmeFramesBuffers swmeFramesBuffers;

		std::vector<DmaSyncer> syncers;

		swmeFramesBuffers.in.db_param = halIsp_->getIspSwmeParam();

		if (swmeWorkbuf_.empty()) {
			const PixelFormatInfo &info = PixelFormatInfo::info(formats::Y8_MTISP);
			uint32_t bufferSize = 0;
			for (unsigned int j = 0; j < info.numPlanes(); j++)
				bufferSize += info.planeSize(swmeWrapper_[0]->getAlgorithmWorkBufferSize(), j);

			swmeWorkbuf_.resize(bufferSize);
		}
		swmeFramesBuffers.in.workbuf = &swmeWorkbuf_;

		{
			IPAMappedBuffer *buffer = getMappedBufferIter(data.base_buf);
			if (!buffer) {
				LOG(IPAMtkISP7, Error) << "Could not find swme base buffer!: ";
				continue;
			}
			swmeFramesBuffers.in.base_buf = buffer->mapped.get();
		}

		{
			IPAMappedBuffer *buffer = getMappedBufferIter(data.ref_buf);
			if (!buffer) {
				LOG(IPAMtkISP7, Error) << "Could not find swme ref buffer!";
				continue;
			}
			swmeFramesBuffers.in.ref_buf = buffer->mapped.get();
		}

		{
			IPAMappedBuffer *buffer = getMappedBufferIter(data.bss_buf);
			if (!buffer) {
				LOG(IPAMtkISP7, Error) << "Could not find swme bss buffer!";
				continue;
			}
			swmeFramesBuffers.in.bss_buf = buffer->mapped.get();
		}

		{
			IPAMappedBuffer *buffer = getMappedBufferIter(data.tuningInfo);
			if (!buffer) {
				LOG(IPAMtkISP7, Error) << "Could not find swme tuningInfo buffer!";
				continue;
			}
			swmeFramesBuffers.in.tuningInfo = buffer->mapped.get();
		}

		{
			IPAMappedBuffer *buffer = getMappedBufferIter(data.conf_map);
			if (!buffer) {
				LOG(IPAMtkISP7, Error) << "Could not find swme conf_map buffer!";
				continue;
			}
			swmeFramesBuffers.out.conf_map = buffer->mapped.get();
			swmeFramesBuffers.out.conf_map_buffer = &buffer->buffer;

			syncers.emplace_back(buffer->buffer.planes()[0].fd.get());
		}

		{
			IPAMappedBuffer *buffer = getMappedBufferIter(data.warpping_map);
			if (!buffer) {
				LOG(IPAMtkISP7, Error) << "Could not find swme warpping_map buffer!";
				continue;
			}
			swmeFramesBuffers.out.warpping_map = buffer->mapped.get();
			swmeFramesBuffers.out.warpping_map_buffer = &buffer->buffer;

			syncers.emplace_back(buffer->buffer.planes()[0].fd.get());
			syncers.emplace_back(buffer->buffer.planes()[1].fd.get());
		}

		{
			IPAMappedBuffer *buffer = getMappedBufferIter(data.mcmv);
			if (!buffer) {
				LOG(IPAMtkISP7, Error) << "Could not find swme mcmv buffer!";
				continue;
			}
			swmeFramesBuffers.out.mcmv = buffer->mapped.get();
			swmeFramesBuffers.out.mcmv_buffer = &buffer->buffer;

			syncers.emplace_back(buffer->buffer.planes()[0].fd.get());
		}

		{
			IPAMappedBuffer *buffer = getMappedBufferIter(data.paramOutInfo);
			if (!buffer) {
				LOG(IPAMtkISP7, Error) << "Could not find swme paramOutInfo buffer!";
				continue;
			}
			swmeFramesBuffers.out.paramOutInfo = buffer->mapped.get();
		}

		IMFBLL_SET_PROC_INFO_STRUCT_IPC paramIn;
		SwmeWrapper::prepareParam(
			paramIn,
			latestFaceMetadata_,
			swmeFramesBuffers,
			sensorFullSize_,
			swmeAlignedSize_,
			i);
		swmewrapper->featureCtrl(IMFBLL_FTCTRL_SET_PROC_INFO, &paramIn, NULL);
		IMFBLL_PROC1_OUT_STRUCT_IPC paramOut;
		SwmeWrapper::prepareOutParam(
			&paramOut,
			swmeFramesBuffers);

		MRESULT ErrCode = swmewrapper->swmeMain(IMFBLL_PROC1, NULL, &paramOut);
		if (ErrCode)
			LOG(IPAMtkISP7, Error) << "Some error with in swmeMain, ErrCode = " << ErrCode;

		{
			IPAMappedBuffer *buffer = getMappedBufferIter(data.paramInInfo);
			if (!buffer) {
				LOG(IPAMtkISP7, Error) << "Could not find swme paramInInfo buffer!";
				continue;
			}

			DmaSyncer syncer(buffer->buffer.planes()[0].fd.get());

			memcpy(reinterpret_cast<void *>(buffer->mapped->planes()[0].data()),
			       reinterpret_cast<void *>(&paramIn),
			       sizeof(IMFBLL_SET_PROC_INFO_STRUCT));
		}

		{
			IPAMappedBuffer *buffer = getMappedBufferIter(data.tuningInfo);
			if (!buffer) {
				LOG(IPAMtkISP7, Fatal) << "Could not find swme tuning buffer!";
				continue;
			}

			DmaSyncer syncer(buffer->buffer.planes()[0].fd.get());

			memcpy(reinterpret_cast<void *>(buffer->mapped->planes()[0].data()),
			       reinterpret_cast<void *>(swmeFramesBuffers.in.db_param.get()),
			       sizeof(mtk::isphal::v1::isp_swme_Param));
		}

		{
			IPAMappedBuffer *buffer = getMappedBufferIter(data.paramOutInfo);
			if (!buffer) {
				LOG(IPAMtkISP7, Fatal) << "Could not find swme paramOutInfo buffer!";
				continue;
			}

			DmaSyncer syncer(buffer->buffer.planes()[0].fd.get());

			memcpy(reinterpret_cast<void *>(buffer->mapped->planes()[0].data()),
			       reinterpret_cast<void *>(&paramOut),
			       sizeof(IMFBLL_PROC1_OUT_STRUCT));
		}
	}

	SwmeResultReady.emit(cookie);
}

void IPAMtkISP7::doBss(const uint64_t cookie, const BssFramesData &bssFramesData,
		       const uint32_t internalRequestId)
{
	const int kInputRawCount = 4;
	uint64_t startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
				     std::chrono::system_clock::now().time_since_epoch())
				     .count();

	BssFramesBuffers bssFramesBuffers;

	std::vector<DmaSyncer> syncers;
	{
		IPAMappedBuffer *bssParamBuffer = getMappedBufferIter(bssFramesData.bssParamInfoId);
		if (!bssParamBuffer) {
			LOG(IPAMtkISP7, Error) << "Could not find bssParam buffer!";
			BssResultReady.emit(cookie, {});
			return;
		}
		bssFramesBuffers.in.bssParamInfo = bssParamBuffer->mapped.get();

		syncers.emplace_back(bssParamBuffer->buffer.planes()[0].fd.get());
	}

	{
		IPAMappedBuffer *bssDataGBuffer = getMappedBufferIter(bssFramesData.bssDataGInfoId);
		if (!bssDataGBuffer) {
			LOG(IPAMtkISP7, Error) << "Could not find bssDataG buffer!";
			BssResultReady.emit(cookie, {});
			return;
		}
		bssFramesBuffers.in.bssDataGInfo = bssDataGBuffer->mapped.get();

		syncers.emplace_back(bssDataGBuffer->buffer.planes()[0].fd.get());
	}

	{
		IPAMappedBuffer *bssVerBuffer = getMappedBufferIter(bssFramesData.bssVerInfoId);
		if (!bssVerBuffer) {
			LOG(IPAMtkISP7, Error) << "Could not find bssVer buffer!";
			BssResultReady.emit(cookie, {});
			return;
		}
		bssFramesBuffers.in.bssVerInfo = bssVerBuffer->mapped.get();

		syncers.emplace_back(bssVerBuffer->buffer.planes()[0].fd.get());
	}

	bssFramesBuffers.in.db_param = halIsp_->getIspBssParam();

	{
		IPAMappedBuffer *bssTuningBuffer = getMappedBufferIter(bssFramesData.bssTuningInfoId);
		if (!bssTuningBuffer) {
			LOG(IPAMtkISP7, Error) << "Could not find bssTuning buffer!";
			BssResultReady.emit(cookie, {});
			return;
		}
		bssFramesBuffers.in.bssTuningInfo = bssTuningBuffer->mapped.get();

		syncers.emplace_back(bssTuningBuffer->buffer.planes()[0].fd.get());

		memcpy(reinterpret_cast<void *>(bssFramesBuffers.in.bssTuningInfo->planes()[0].data()),
		       bssFramesBuffers.in.db_param.get(), sizeof(mtk::isphal::v1::isp_bss_Param));
	}

	for (auto id : bssFramesData.bssFdMainInfoId) {
		IPAMappedBuffer *buffer = getMappedBufferIter(id);
		if (!buffer) {
			LOG(IPAMtkISP7, Error) << "Could not find bssFdMainInfo buffer!";
			BssResultReady.emit(cookie, {});
			return;
		}
		bssFramesBuffers.in.bssFdMainInfo.push_back(buffer->mapped.get());
	}

	for (auto id : bssFramesData.imgiId) {
		IPAMappedBuffer *buffer = getMappedBufferIter(id);
		if (!buffer) {
			LOG(IPAMtkISP7, Error) << "Could not find imgi buffer!";
			BssResultReady.emit(cookie, {});
			return;
		}
		bssFramesBuffers.in.imgi.push_back(buffer->mapped.get());
		bssFramesBuffers.in.imgiBuffers.push_back(&buffer->buffer);
	}

	for (auto id : bssFramesData.bssFdInfoId) {
		IPAMappedBuffer *buffer = getMappedBufferIter(id);
		if (!buffer) {
			LOG(IPAMtkISP7, Error) << "Could not find bssFd buffer!";
			BssResultReady.emit(cookie, {});
			return;
		}
		bssFramesBuffers.in.bssFdInfo.push_back(buffer->mapped.get());
	}

	for (auto id : bssFramesData.bssFaceInfoId) {
		IPAMappedBuffer *buffer = getMappedBufferIter(id);
		if (!buffer) {
			LOG(IPAMtkISP7, Error) << "Could not find bssFace buffer!";
			BssResultReady.emit(cookie, {});
			return;
		}
		bssFramesBuffers.in.bssFaceInfo.push_back(buffer->mapped.get());
	}

	for (auto id : bssFramesData.bssPosInfoId) {
		IPAMappedBuffer *buffer = getMappedBufferIter(id);
		if (!buffer) {
			LOG(IPAMtkISP7, Error) << "Could not find bssPos buffer!";
			BssResultReady.emit(cookie, {});
			return;
		}
		bssFramesBuffers.in.bssPosInfo.push_back(buffer->mapped.get());
	}

	{
		IPAMappedBuffer *bssOutDataBuffer = getMappedBufferIter(bssFramesData.bssOutDataInfoId);
		if (!bssOutDataBuffer) {
			LOG(IPAMtkISP7, Error) << "Could not find bssOutData buffer!";
			BssResultReady.emit(cookie, {});
			return;
		}
		bssFramesBuffers.out.bssOutDataInfo = bssOutDataBuffer->mapped.get();

		syncers.emplace_back(bssOutDataBuffer->buffer.planes()[0].fd.get());
	}

	auto bssOrder = bssWrapper_->doBss(kInputRawCount, bssFramesBuffers);

	mfnrExifData_[MF_TAG_EXPOSURE] = bssFramesData.exposure;
	mfnrExifData_[MF_TAG_ISO] = bssFramesData.iso;
	for (auto &item : bssWrapper_->getExifData())
		mfnrExifData_[item.first] = item.second;

	uint64_t endTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
				   std::chrono::system_clock::now().time_since_epoch())
				   .count();
	uint64_t bssExecTime = endTime - startTime;
	std::stringstream sstream;
	sstream << "BSS execute time: " << bssExecTime / 1000000 << "ms, frame order: ";

	for (auto order : bssOrder) {
		sstream << "F" << order + internalRequestId << " ";
	}
	onDeviceTuner_.writeLogScenarioRecorder(internalRequestId, internalRequestId, EStage_BSS, sstream.str());

	BssResultReady.emit(cookie, bssOrder);
}

void IPAMtkISP7::doAAAResultReady(uint32_t frame, SensorSetting sensorSetting,
				  const AaaIspExchange &aaaIspExchange,
				  LensPositionInfo lensPositionInfo)
{
	AAAResultReady.emit(frame, sensorSetting, aaaIspExchange, lensPositionInfo);
}

void IPAMtkISP7::doImgSysMetaTuningDone(uint64_t taskCounter)
{
	ImgSysMetaTuningDone.emit(taskCounter);
}

IPAMtkISP7::AAAManager::AAAManager(IPAMtkISP7 *ipa)
	: ipa_(ipa)
{
}

void IPAMtkISP7::AAAManager::doCalculation(FrameBuffer *statistics0, FrameBuffer *statistics1,
					   uint64_t timestamp,
					   uint32_t internalRequestId,
					   uint32_t camSysMetaRequestId,
					   const uint32_t afCamSysMetaRequestId,
					   bool isStillCapture, int rawMetaFd,
					   unsigned char *rawMetaBuffer,
					   ::VcmFocusInformation vcmFocusInfo,
					   std::optional<MtkCameraFaceMetadata> metadata,
					   GyroSensor::SensorSample gyroSample,
					   const uint32_t internalRequestIdApplied,
					   const ControlList &controls,
					   const int32_t featureEnum)
{
	SensorSetting sensorSetting;
	if (statistics1) {
		ipa_->hal3A_->doCalculationAF(statistics1, timestamp, internalRequestId,
					      afCamSysMetaRequestId, vcmFocusInfo,
					      metadata, gyroSample, &sensorSetting.position, controls);
	}
	AaaIspExchange aaaIspExchange;
	aaaIspExchange.aaaMetadata = controls::controls;
	LensPositionInfo lensPositionInfo;
	std::optional<uint32_t> idApplied = std::nullopt;
	if (internalRequestIdApplied != 0)
		idApplied = internalRequestIdApplied;

	std::optional<Feature> featureApplied = std::nullopt;
	if (featureEnum >= 0)
		featureApplied = static_cast<Feature>(featureEnum);
	{
		DmaSyncer syncer(rawMetaFd);

		ipa_->hal3A_->doCalculation(statistics0, timestamp, internalRequestId,
					    camSysMetaRequestId, isStillCapture,
					    rawMetaBuffer,
					    metadata, gyroSample,
					    &sensorSetting, &aaaIspExchange,
					    idApplied, featureApplied,
					    &lensPositionInfo, controls);

		ipa_->halIsp_->getCamSysMetaTuning(
			internalRequestId, internalRequestId, rawMetaFd,
			(intptr_t)rawMetaBuffer, 0,
			kHal3ARawMetaSize, isStillCapture,
			metadata ? &metadata.value() : nullptr,
			internalRequestIdApplied, featureApplied,
			&aaaIspExchange, controls);
	}

	ipa_->invokeMethod(
		&IPAMtkISP7::doAAAResultReady, ConnectionTypeQueued,
		internalRequestId, sensorSetting, aaaIspExchange,
		lensPositionInfo);

	if (idApplied) {
		ipa_->onDeviceTuner_.tune3AState(
			internalRequestIdApplied,
			statistics0, &ipa_->hal3A_->r3AResult_);
	}
}

IPAMtkISP7::IspManager::IspManager(IPAMtkISP7 *ipa)
	: ipa_(ipa)
{
}

void IPAMtkISP7::IspManager::getImgSysMetaTuning(
	const uint64_t cookie, const uint32_t camSysMetaRequestId,
	const uint32_t frame, const bool needCropTNC16x9,
	const Feature feature,
	const std::vector<ipa::mtkisp7::ImgMetaRequestData> imgMetaRequests,
	const std::vector<DataMappedBuffers> dataMappedBuffersList,
	const ControlList &controls)
{
	for (uint32_t i = 0; i < imgMetaRequests.size(); ++i) {
		const auto &requestData = imgMetaRequests[i];
		const auto &dataMappedBuffers = dataMappedBuffersList[i];

		if (!dataMappedBuffers.valid)
			continue;

		std::vector<DmaSyncer> syncers;

		IPAMappedBuffer *tuningBuffer = dataMappedBuffers.tuning;
		if (!tuningBuffer) {
			LOG(IPAMtkISP7, Fatal) << "Could not find tuning buffer!";
			continue;
		}
		syncers.emplace_back(tuningBuffer->buffer.planes()[0].fd.get());

		IPAMappedBuffer *statBuffer = nullptr;
		if (requestData.statisticsBufferId > 0) {
			statBuffer = dataMappedBuffers.statistics;
			if (!statBuffer) {
				LOG(IPAMtkISP7, Fatal) << "Could not find stat buffer!";
				continue;
			}

			syncers.emplace_back(statBuffer->buffer.planes()[0].fd.get());
		}

		IPAMappedBuffer *swHistBuffer = nullptr;
		if (requestData.swHistBufferId > 0) {
			swHistBuffer = dataMappedBuffers.swHist;
			if (!swHistBuffer) {
				LOG(IPAMtkISP7, Fatal) << "Could not find swHist buffer!";
				continue;
			}
		}

		ImgMetaRequest request = {
			.isCapture = requestData.isCapture,
			.isMfnr = requestData.isMfnr,
			.stage = static_cast<NSIspTuning::EStage_T>(requestData.stage),
			.tuningBuffer = &tuningBuffer->buffer,
			.mappedTuningBuffer = tuningBuffer->mapped.get(),
			.statisticsBuffer = statBuffer ? &statBuffer->buffer : nullptr,
			.mappedStatisticsBuffer = statBuffer ? statBuffer->mapped.get() : nullptr,
			.swHistBuffer = swHistBuffer ? &swHistBuffer->buffer : nullptr,
			.mappedSwHistBuffer = swHistBuffer ? swHistBuffer->mapped.get() : nullptr,
			.inputSize = requestData.inputSize,
			.outputSize = requestData.outputSize,
			.outputSize2 = requestData.outputSize2,
			.fullDipSize = requestData.fullDipSize,
			.tnr_frameIndex = requestData.tnr_frameIndex,
			.tnr_frameTotal = requestData.tnr_frameTotal,
			.index = requestData.index,
			.isGolden = requestData.isGolden,
			.reserved = {}
		};

		uint32_t j = 0;
		for (const auto &[keyInt, bufferId] : requestData.reserved) {
			if (j >= dataMappedBuffers.reserved.size()) {
				LOG(IPAMtkISP7, Fatal)
					<< "Invalid reserved mapped buffer index: "
					<< j << ", size: "
					<< dataMappedBuffers.reserved.size();
			}

			IPAMappedBuffer *mappedBuffer = dataMappedBuffers.reserved[j++];
			if (!mappedBuffer) {
				LOG(IPAMtkISP7, Fatal)
					<< "Invalid reserved mapped buffer: "
					<< j << ", size: "
					<< dataMappedBuffers.reserved.size();
				continue;
			}

			mtk::isphal::kISPExtBuf key =
				static_cast<mtk::isphal::kISPExtBuf>(keyInt);
			request.reserved[key] =
				std::make_pair<FrameBuffer *, MappedFrameBuffer *>(
					&mappedBuffer->buffer,
					mappedBuffer->mapped.get());

			switch (key) {
			case mtk::isphal::kISPExtBif_IN_HWME_STAT_FST_MD0:
			case mtk::isphal::kISPExtBif_IN_HWME_STAT_FST_MD1:
			case mtk::isphal::kISPExtBif_IN_HWME_STAT_FMB_MD0:
			case mtk::isphal::kISPExtBif_OUT_FWMM_MIL:
				syncers.emplace_back(mappedBuffer->buffer.planes()[0].fd.get());
				break;
			default:
				break;
			}
		}

		ipa_->halIsp_->getImgSysMetaTuning(
			camSysMetaRequestId, request, frame,
			requestData.hasFrameNumber
				? requestData.frameNumber
				: frame,
			needCropTNC16x9, feature, controls);
	}

	ipa_->invokeMethod(
		&IPAMtkISP7::doImgSysMetaTuningDone,
		ConnectionTypeQueued, cookie);
}

} // namespace ipa::mtkisp7

/**
 * \brief External IPA module interface
 *
 * The IPAModuleInfo is required to match an IPA module construction against the
 * intented pipeline handler with the module. The API and pipeline handler
 * versions must match the corresponding IPA interface and pipeline handler.
 *
 * \sa struct IPAModuleInfo
 */
extern "C" {
const struct IPAModuleInfo ipaModuleInfo = {
	IPA_MODULE_API_VERSION,
	1,
	"PipelineHandlerMtkISP7",
	"mtkisp7",
};

/**
 * \brief Create an instance of the IPA interface
 *
 * This function is the entry point of the IPA module. It is called by the IPA
 * manager to create an instance of the IPA interface for each camera. When
 * matched against with a pipeline handler, the IPAManager will construct an IPA
 * instance for each associated Camera.
 */
IPAInterface *ipaCreate()
{
	auto *ptr = new ipa::mtkisp7::IPAMtkISP7();
	return ptr;
}
}

} // namespace libcamera

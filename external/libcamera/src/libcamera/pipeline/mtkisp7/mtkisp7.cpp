/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Google Inc.
 *
 * mtkisp7.cpp - Pipeline handler for Mediatek MtkISP7
 */

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <memory>
#include <string>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <vector>

#include <libcamera/base/log.h>

#include <libcamera/camera.h>
#include <libcamera/control_ids.h>
#include <libcamera/controls.h>
#include <libcamera/formats.h>
#include <libcamera/property_ids.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>

#include "libcamera/internal/camera.h"
#include "libcamera/internal/device_enumerator.h"
#include "libcamera/internal/gyro_sensor.h"
#include "libcamera/internal/ipa_manager.h"
#include "libcamera/internal/mailbox.h"
#include "libcamera/internal/media_device.h"
#include "libcamera/internal/pipeline_handler.h"
#include "libcamera/internal/task_scheduler.h"

#include "camsys/camsys.h"
#include "camsys/capture.h"
#include "hal3a/aaa.h"
#include "halisp/ITuningDataProvider.h"
#include "halisp/lpnr_tun.h"
#include "halisp/mcnr_tun.h"
#include "halisp/mfnr_tun.h"
#include "imgsys/imgsys.h"
#include "imgsys/lpnr.h"
#include "imgsys/mcnr.h"
#include "imgsys/mfnr.h"
#include "libfdft_lib/faces.h"
#include "mtkcam-core/libcamera_ext/lib/libMfbllWrapper/MTKMfbllHeader/include/EMfbll.h"
#include "pipeline/mtkisp7/face_detect/detector.h"
#include "pipeline/mtkisp7/ipa/ipa_delegate.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/feature.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"
#include "platform/mtkisp7/platform_utils.h"
#include "platform/mtkisp7/utils/history.h"
#include "sensor/sensor_info.h"

#include "mtkisp7_ipa_interface.h"

namespace libcamera {
LOG_DEFINE_CATEGORY(MtkISP7)

static const ControlInfoMap::Map MtkISP7Controls = {
	{ &controls::draft::PipelineDepth, ControlInfo(8, 8, 8) },
	{ &controls::draft::NoiseReductionMode, ControlInfo(controls::draft::NoiseReductionModeValues) },
};

// TODO(chenghaoyang): Set a big core according to models.
// Ciri's big cores are CPU 6 and 7.
static const std::vector<int> kMainThreadCpuAffinity{ 0, 1, 2, 3, 4, 5 };

enum MtkISP7TaskGroup {
	SofGroup = 0,
	CaptureQueueGroup,
	CaptureDequeueGroup,
	AAAGroup,
	MeAGroup,
	MeBGroup,
	MeATunGroup,
	MeBTunGroup,
	TrGroup,
	TrTunGroup,
	XtrGroup,
	Dip1Group,
	Dip2Group,
	DipTunGroup,
	LpnrDipGroup,
	LpnrTunXtrTaskGroup,
	LpnrTunDipTaskGroup,
	AieFaceDetectionGroup,
	AieFaceToneClassificationGroup,
	AieParseGroup,
	BfbldTaskGroup,
	McdsF1Group,
	BfmeGroup,
	DsGroup,
	DsVbiGroup,
	Msbld1stGroup,
	Msbld2ndGroup,
	AfbldGroup,
	BssTunTaskGroup,
	BfbldTunTaskGroup,
	McdsF1TunGroup,
	BfmeTunGroup,
	SwmeTunGroup,
	DsTunGroup,
	DsVbiTunGroup,
	MsbldTun1stGroup,
	MsbldTun2ndGroup,
	AfbldTunGroup,
	CompleteGroup,
};

static const std::map<MtkISP7TaskGroup, std::string> kGroupName{
	{ SofGroup, "SofGroup" },
	{ CaptureQueueGroup, "CaptureQueueGroup" },
	{ CaptureDequeueGroup, "CaptureDequeueGroup" },
	{ AAAGroup, "AAAGroup" },
	{ MeAGroup, "MeAGroup" },
	{ MeBGroup, "MeBGroup" },
	{ MeATunGroup, "MeATunGroup" },
	{ MeBTunGroup, "MeBTunGroup" },
	{ TrTunGroup, "TrTunGroup" },
	{ TrGroup, "TrGroup" },
	{ XtrGroup, "XtrGroup" },
	{ DipTunGroup, "DipTunGroup" },
	{ Dip1Group, "Dip1Group" },
	{ Dip2Group, "Dip2Group" },
	{ LpnrTunXtrTaskGroup, "LpnrTunXtrTaskGroup" },
	{ LpnrTunDipTaskGroup, "LpnrTunDipTaskGroup" },
	{ AieFaceDetectionGroup, "AieFaceDetectionGroup" },
	{ AieFaceToneClassificationGroup, "AieFaceToneClassificationGroup" },
	{ BfbldTaskGroup, "BfbldTaskGroup" },
	{ McdsF1Group, "McdsF1Group" },
	{ BfmeGroup, "BfmeGroup" },
	{ DsGroup, "DsGroup" },
	{ DsVbiGroup, "DsVbiGroup" },
	{ Msbld1stGroup, "Msbld1stGroup" },
	{ Msbld2ndGroup, "Msbld2ndGroup" },
	{ AfbldGroup, "AfbldGroup" },
	{ BssTunTaskGroup, "BssTunTaskGroup" },
	{ BfbldTunTaskGroup, "BfbldTunTaskGroup" },
	{ McdsF1TunGroup, "McdsF1TunGroup" },
	{ BfmeTunGroup, "BfmeTunGroup" },
	{ SwmeTunGroup, "SwmeTunGroup" },
	{ DsTunGroup, "DsTunGroup" },
	{ DsVbiTunGroup, "DsVbiTunGroup" },
	{ MsbldTun1stGroup, "MsbldTun1stGroup" },
	{ MsbldTun2ndGroup, "MsbldTun2ndGroup" },
	{ AfbldTunGroup, "AfbldTunGroup" },
	{ CompleteGroup, "CompleteGroup" },
};

class CompleteRequestTask : public Task
{
public:
	CompleteRequestTask(Scheduler *scheduler, const std::string &id,
			    Request *request, uint32_t internalRequestId,
			    uint32_t camSysMetaRequestId, PipelineHandler *pipe,
			    IPADelegate *ipa,
			    OnDeviceTuner *odt, FaceDetector *faceDetector,
			    SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange,
			    std::set<const Stream *> mfnrStreams);

	virtual void run() override final;

private:
	PipelineHandler *pipe_;
	Request *request_;
	uint32_t internalRequestId_;
	uint32_t camSysMetaRequestId_;
	FaceDetector *faceDetector_;
	IPADelegate *ipa_;
	OnDeviceTuner *onDeviceTuner_;
	SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange_;
	std::set<const Stream *> mfnrStreams_;
};

CompleteRequestTask::CompleteRequestTask(
	Scheduler *scheduler,
	const std::string &id,
	Request *request,
	uint32_t internalRequestId,
	uint32_t camSysMetaRequestId,
	PipelineHandler *pipe,
	IPADelegate *ipa,
	OnDeviceTuner *odt,
	FaceDetector *faceDetector,
	SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange,
	std::set<const Stream *> mfnrStreams)
	: Task(scheduler, id), pipe_(pipe), request_(request),
	  internalRequestId_(internalRequestId),
	  camSysMetaRequestId_(camSysMetaRequestId),
	  faceDetector_(faceDetector), ipa_(ipa), onDeviceTuner_(odt),
	  aaaIspExchange_(aaaIspExchange), mfnrStreams_(std::move(mfnrStreams))
{
}

struct CaptureResult {
	SharedMailBox<InfoFrame> tuningOutput;
	SharedMailBox<ipa::mtkisp7::SensorSetting> exposureAndGainOutput;
};

struct AaaIspExchangeResult {
	uint32_t camSysMetaRequestId;
	SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange;
};

class MtkISP7CameraData : public Camera::Private
{
public:
	MtkISP7CameraData(PipelineHandler *pipe, CamSysDevice *camSysDev,
			  ImgSysDevice *imgSysDev, GyroSensor *gyroSensor, OnDeviceTuner *odt,
			  FaceDetector *faceDetector, DmaHeap *dmaHeap,
			  int32_t sensor_idx)
		: Camera::Private(pipe), camSysDev_(camSysDev), imgSysDev_(imgSysDev),
		  gyroSensor_(gyroSensor),
		  ipa_(std::make_unique<IPADelegate>()),
		  captureManager(odt), mcnrManager(imgSysDev, dmaHeap, odt),
		  lpnrManager(imgSysDev, dmaHeap, odt),
		  mfnrManager(imgSysDev, dmaHeap, odt),
		  lpnrTunManager(dmaHeap, ipa_.get(), odt),
		  mcnrTunManager(dmaHeap, ipa_.get(), odt),
		  mfnrTunManager(dmaHeap, ipa_.get(), odt),
		  onDeviceTuner_(odt),
		  faceDetector_(faceDetector), dmaHeap_(dmaHeap),
		  captureResult_(5),
		  aaaIspExchangeResult_(10 + CaptureTasksManager::kRawMetaDelay),
		  mfnrInput_(MFNR_QUEUE_SIZE), sensor_idx_(sensor_idx),
		  control_cache_(nullptr)
	{
	}

	bool loadIPA();

	int configure(CameraConfiguration *c);
	int queueRequest(Request *request);

	int start(const ControlList *controls);
	void stopDevice();

	bool acquireDevice();
	void releaseDevice();

	void frameStart(uint32_t sequence);

	bool is3aControlChanged(std::shared_ptr<ControlList> controls_cur, std::shared_ptr<ControlList> controls_cache);

	std::tuple<QueueTask *, DequeueTask *, SofTask *,
		   AAATask *, uint32_t>
	makeTasks(const std::string &id, Request *request,
		  CaptureFrames &captureFrames, uint32_t internalRequestId,
		  bool hasStillCapture, bool needRaw, bool hasVideo,
		  bool useMfnr);
	void setTasksDependencies(QueueTask *taskQBuf, DequeueTask *taskDQBuf,
				  SofTask *sofTask, AAATask *aaaTask);

	void allocateIPABuffers();
	void registerIPABuffers(InfoFramePool *pool);
	void freeIPABuffers();

	void IPADisconnected();

	Stream video1Stream_;
	Stream video2Stream_;
	Stream still1Stream_;
	Stream still2Stream_;

	uint32_t frameSequence_ = 0;
	std::list<SofTask *> pendingSofTasks_;

	Size sensorFullSize_;
	CamSysDevice *camSysDev_;
	ImgSysDevice *imgSysDev_;
	GyroSensor *gyroSensor_;

	std::unique_ptr<IPADelegate> ipa_;

	CaptureTasksManager captureManager;
	Hal3AManager hal3AManager_;

	MCNRPrevOutput mcnrPrev;
	McnrTasksManager mcnrManager;
	LpnrTasksManager lpnrManager;
	MfnrTasksManager mfnrManager;

	LpnrTunTasksManager lpnrTunManager;
	McnrTunManager mcnrTunManager;
	MfnrTunManager mfnrTunManager;

	OnDeviceTuner *onDeviceTuner_;
	FaceDetector *faceDetector_;
	DmaHeap *dmaHeap_;

	History<CaptureResult> captureResult_;
	History<AaaIspExchangeResult> aaaIspExchangeResult_;

	uint32_t requestCount_ = 0;

	History<MfnrInput> mfnrInput_;

	int getSensorIdx() { return sensor_idx_; }

private:
	int sensor_idx_;
	std::shared_ptr<ControlList> control_cache_;
	std::vector<unsigned int> ipaBufferIds_;
	uint32_t ipaBufferCnt_;

	ipa::mtkisp7::SensorSetting initSensorSetting_;

	uint32_t latestStillCapture_ = 0;
};

class MtkISP7CameraConfiguration : public CameraConfiguration
{
public:
	MtkISP7CameraConfiguration(MtkISP7CameraData *data);
	Status validate() override;

private:
	/*
	 * The MtkISP7CameraData instance is guaranteed to be valid as long as the
	 * corresponding Camera instance is valid. In order to borrow a
	 * reference to the camera data, store a new reference to the camera.
	 */
	const MtkISP7CameraData *data_;
};

class PipelineHandlerMtkISP7 : public PipelineHandler
{
public:
	PipelineHandlerMtkISP7(CameraManager *manager);

	std::unique_ptr<CameraConfiguration> generateConfiguration(Camera *camera,
								   Span<const StreamRole> roles) override;
	int configure(Camera *camera, CameraConfiguration *config) override;

	int exportFrameBuffers(Camera *camera, Stream *stream,
			       std::vector<std::unique_ptr<FrameBuffer>> *buffers) override;

	int start(Camera *camera, const ControlList *controls) override;
	void stopDevice(Camera *camera) override;

	bool acquireDevice(Camera *camera) override;
	void releaseDevice(Camera *camera) override;

	int queueRequestDevice(Camera *camera, Request *request) override;

	bool match(DeviceEnumerator *enumerator) override;

	void adjustRLimit();

	std::unique_ptr<CategorizedScheduler<MtkISP7TaskGroup>> scheduler_;
	std::unique_ptr<DmaHeap> dmaHeap_;

	OnDeviceTuner onDeviceTuner_;

	MediaDevice *camSysMedia_;
	CamSysDevice camSysDev_[2];
	GyroSensor gyroSensor_;

	MediaDevice *imgSysMedia_;
	ImgSysDevice imgSysDev_;

	MediaDevice *aieMedia_;

	FaceDetector faceDetector_;

	std::vector<ipa::mtkisp7::CamSysData> camSysDataArray_;

private:
	MtkISP7CameraData *cameraData(Camera *camera)
	{
		return static_cast<MtkISP7CameraData *>(camera->_d());
	}

	void loadModelName();
};

void CompleteRequestTask::run()
{
	ControlList metadata;
	const auto &scalerCrop = request_->controls().get(controls::ScalerCrop);
	if (scalerCrop) {
		metadata.set(controls::ScalerCrop, *scalerCrop);
	}

	metadata.set(controls::draft::PipelineDepth, 8);

	int32_t testPatternMode = controls::draft::TestPatternModeOff;
	const auto &testPatternControl = request_->controls().get(controls::draft::TestPatternMode);
	if (testPatternControl)
		testPatternMode = *testPatternControl;

	metadata.set(controls::draft::TestPatternMode, testPatternMode);

	// todo(yerlandinata, before CTS): check if face metadata is requested
	ControlList faceControls;
	faceDetector_->getLatestFaceControls(faceControls);

	metadata.merge(faceControls);

	if (aaaIspExchange_->valid() &&
	    onDeviceTuner_->isEnabled() &&
	    onDeviceTuner_->isStillCaptureRequest(internalRequestId_)) {
		ControlList debugMetadata;
		ipa_->writeStillCaptureDebugMetadata(
			camSysMetaRequestId_, &debugMetadata);
		metadata.merge(debugMetadata);
	}

	pipe_->metadataAvailable(request_, metadata);

	for (auto it : request_->buffers()) {
		FrameBuffer *buffer = it.second;

		if (mfnrStreams_.find(it.first) == mfnrStreams_.end())
			pipe_->completeBuffer(request_, buffer);
	}

	if (onDeviceTuner_->isEnabled()) {
		ipa_->notifyRequestEnd(internalRequestId_);
		onDeviceTuner_->notifyRequestEnd(internalRequestId_);
	}

	pipe_->completeRequest(request_);
	Task::notifyDone();
}

MtkISP7CameraConfiguration::MtkISP7CameraConfiguration(MtkISP7CameraData *data)
	: CameraConfiguration()
{
	data_ = data;
}

CameraConfiguration::Status MtkISP7CameraConfiguration::validate()
{
	static const std::vector<Size> resolutions = {
		{ 320, 240 },
		{ 640, 360 },
		{ 640, 480 },
		{ 1280, 720 },
		{ 1280, 960 },
		{ 1440, 1080 },
		{ 1920, 1080 },
		{ 1920, 1440 },
		{ 2560, 1440 },
		{ 2560, 1920 },
	};

	const Stream *vidStreams[2]{
		&data_->video1Stream_,
		&data_->video2Stream_
	};

	const Stream *stillStreams[2]{
		&data_->still1Stream_,
		&data_->still2Stream_
	};

	int videoCnt = 0;
	int stillCnt = 0;
	for (StreamConfiguration &cfg : config_) {
		/* Allows the predefined resolutions plus the sensor size */
		if (!std::count(resolutions.begin(), resolutions.end(), cfg.size) &&
		    cfg.size != data_->sensorFullSize_)
			return Invalid;

		cfg.pixelFormat = formats::NV12;
		cfg.bufferCount = CamSysDevice::kBufferCount;

		const PixelFormatInfo &info = PixelFormatInfo::info(cfg.pixelFormat);
		cfg.stride = info.stride(cfg.size.width, 0, 64);

		switch (cfg.role) {
		case StreamRole::Viewfinder:
		case StreamRole::VideoRecording:
			if (videoCnt >= 2) {
				LOG(MtkISP7, Error)
					<< "Support only 2 Preview/Video streams";
				return Invalid;
			}
			cfg.setStream(const_cast<Stream *>(vidStreams[videoCnt++]));
			break;
		case StreamRole::YuvSnapshot:
		case StreamRole::StillCapture:
			if (stillCnt >= 2) {
				LOG(MtkISP7, Error)
					<< "Support only 2 StillCapture streams";
				return Invalid;
			}
			cfg.setStream(const_cast<Stream *>(stillStreams[stillCnt++]));
			break;
		default:
			LOG(MtkISP7, Error) << "Invalid StreamRole " << cfg.role;
			return Invalid;
		}
	}

	return Valid;
}

PipelineHandlerMtkISP7::PipelineHandlerMtkISP7(CameraManager *manager)
	: PipelineHandler(manager),
	  camSysDev_{ { &onDeviceTuner_ }, { &onDeviceTuner_ } },
	  imgSysDev_(&onDeviceTuner_)
{
	scheduler_ = std::make_unique<CategorizedScheduler<MtkISP7TaskGroup>>(kGroupName);
	dmaHeap_ = std::make_unique<DmaHeap>();

	adjustRLimit();

	thread()->setThreadAffinity(kMainThreadCpuAffinity);

	loadModelName();
}

void PipelineHandlerMtkISP7::loadModelName()
{
	std::string model_name_path = "/run/chromeos-config/v1/name";
	std::fstream model_name_file;
	model_name_file.open(model_name_path, std::ios::in);
	std::string model;
	if (model_name_file.is_open()) {
		getline(model_name_file, model);
		model_name_file.close();
	} else {
		LOG(MtkISP7, Fatal) << "Unable to open file " << model_name_path;
	}

	PlatformUtils::setWithModelName(model);
}

std::unique_ptr<CameraConfiguration>
PipelineHandlerMtkISP7::generateConfiguration(Camera *camera, Span<const StreamRole> roles)
{
	MtkISP7CameraData *data = cameraData(camera);
	std::unique_ptr<MtkISP7CameraConfiguration> config =
		std::make_unique<MtkISP7CameraConfiguration>(data);

	if (roles.empty())
		return config;

	Size maxSize = data->sensorFullSize_;
	PixelFormat pixelFormat = formats::NV12;

	std::map<PixelFormat, std::vector<SizeRange>> streamFormats = { { pixelFormat, { { CamSysDevice::kMinResolution, maxSize } } } };
	StreamFormats formats(streamFormats);

	Stream *streams[2]{
		&data->video1Stream_,
		&data->video2Stream_
	};

	int videoCnt = 0;
	for (const StreamRole role : roles) {
		StreamConfiguration cfg(formats);
		cfg.size = maxSize;
		cfg.pixelFormat = pixelFormat;
		cfg.bufferCount = CamSysDevice::kBufferCount;

		switch (role) {
		case StreamRole::StillCapture:
		case StreamRole::YuvSnapshot:
			cfg.setStream(&data->still1Stream_);
			cfg.role = StreamRole::StillCapture;
			break;

		case StreamRole::Viewfinder:
		case StreamRole::VideoRecording:
			if (videoCnt >= 2) {
				LOG(MtkISP7, Error)
					<< "Support only 2 Preview/Video streams";
				return nullptr;
			}
			cfg.setStream(streams[videoCnt++]);
			cfg.role = role;
			break;

		case StreamRole::Raw:
		default:
			LOG(MtkISP7, Error)
				<< "Requested stream role not supported: " << role;
			return nullptr;
		}

		config->addConfiguration(cfg);
	}

	if (config->validate() == CameraConfiguration::Invalid)
		return {};

	return config;
}

int PipelineHandlerMtkISP7::exportFrameBuffers([[maybe_unused]] Camera *camera,
					       [[maybe_unused]] Stream *stream,
					       [[maybe_unused]] std::vector<std::unique_ptr<FrameBuffer>> *buffers)
{
	/* todo: Generate frame buffers by DMA heap */
	return -EINVAL;
}

void PipelineHandlerMtkISP7::adjustRLimit()
{
	struct sysinfo info;

	if (sysinfo(&info) != 0) {
		perror("sysinfo");
		exit(EXIT_FAILURE);
	}

	struct rlimit rlim;

	if (getrlimit(RLIMIT_NOFILE, &rlim) != 0) {
		perror("getrlimit");
		exit(EXIT_FAILURE);
	}

	if (rlim.rlim_cur == RLIM_INFINITY)
		LOG(MtkISP7, Info) << "Current file descriptor limit: unlimited ";
	else
		LOG(MtkISP7, Info) << "Current file descriptor limit:" << rlim.rlim_cur;

	if (rlim.rlim_max == RLIM_INFINITY)
		LOG(MtkISP7, Info) << "Maximum file descriptor limit: unlimited:";
	else
		LOG(MtkISP7, Info) << "Maximum file descriptor limit: " << rlim.rlim_max;

	// Increase the soft limit to 2048
	rlim.rlim_cur = 2048;

	if (setrlimit(RLIMIT_NOFILE, &rlim) != 0) {
		perror("setrlimit");
		exit(EXIT_FAILURE);
	}

	if (getrlimit(RLIMIT_NOFILE, &rlim) != 0) {
		perror("getrlimit");
		exit(EXIT_FAILURE);
	}
}

int PipelineHandlerMtkISP7::configure(Camera *camera, CameraConfiguration *c)
{
	return cameraData(camera)->configure(c);
}

int PipelineHandlerMtkISP7::start(Camera *camera, [[maybe_unused]] const ControlList *controls)
{
	return cameraData(camera)->start(controls);
}

void PipelineHandlerMtkISP7::stopDevice(Camera *camera)
{
	cameraData(camera)->stopDevice();
}

bool PipelineHandlerMtkISP7::acquireDevice(Camera *camera)
{
	return cameraData(camera)->acquireDevice();
}

void PipelineHandlerMtkISP7::releaseDevice(Camera *camera)
{
	cameraData(camera)->releaseDevice();
}

int PipelineHandlerMtkISP7::queueRequestDevice(Camera *camera, Request *request)
{
	return cameraData(camera)->queueRequest(request);
}

bool PipelineHandlerMtkISP7::match(DeviceEnumerator *enumerator)
{
	onDeviceTuner_.initialize(false);

	DeviceMatch camSysDM("mtk-cam");
	camSysDM.add("mtk-cam raw-0");
	camSysDM.add("mtk-cam raw-1");

	camSysMedia_ = acquireMediaDevice(enumerator, camSysDM);

	if (!camSysMedia_)
		return false;

	if (camSysMedia_->disableLinks())
		return false;

	int errGyro = gyroSensor_.init(GyroSensor::Location::kLid);
	if (errGyro) {
		LOG(MtkISP7, Warning) << "No gyroscope available";
	}

	DeviceMatch imgSysDM("mtk-imgsys");
	imgSysDM.add("MTK-ISP-DIP-V4L2");

	imgSysMedia_ = acquireMediaDevice(enumerator, imgSysDM);

	if (!imgSysMedia_)
		return false;

	if (imgSysMedia_->disableLinks())
		return false;

	DeviceMatch aieDM("mtk-aie-5.3");
	aieMedia_ = acquireMediaDevice(enumerator, aieDM);
	if (!aieMedia_) {
		LOG(MtkISP7, Error) << "Failed to match AIE Media";
		return false;
	}
	if (faceDetector_.init(aieMedia_, dmaHeap_.get()) != 0) {
		LOG(MtkISP7, Error) << "Failed to init AIE device";
		return false;
	}

	camSysDataArray_.resize(2);
	uint32_t sensorCnt = 0;
	for (unsigned int i = 0; i < 2; i++) {
		if (camSysDev_[i].init(camSysMedia_, i))
			continue;

		camSysDataArray_[i].has_af = camSysDev_[i].getCameraLens();
		camSysDataArray_[i].mbus_code = camSysDev_[i].mbusCode();
	}

	for (unsigned int i = 0; i < 2; i++) {
		if (!camSysDev_[i].isValid())
			continue;

		ControlList properties = camSysDev_[i].properties();

		// Fill ControlInfoMap
		ControlInfoMap::Map controls = MtkISP7Controls;

		// Increase the pipeline depth when ODT is enabled to ensure
		// the FPS is consistent with CCA, since it needs every
		// request to preserve enough pending 3A tasks according to
		// CaptureTasksManager::kRawMetaDelay.
		if (onDeviceTuner_.isEnabled())
			controls[&controls::draft::PipelineDepth] = ControlInfo(10, 10, 10);

		// todo: Fix the frame duration to 30fps for now. It should be
		// updated on stream configuration
		controls[&controls::FrameDurationLimits] = ControlInfo((int64_t)33'333,
								       (int64_t)66'666,
								       (int64_t)33'333);

		// todo: Fix the ExposureTime for now. It should be updated from
		// sensor config
		controls[&controls::ExposureTime] = ControlInfo((int32_t)80,
								(int32_t)100'000,
								(int32_t)33'333);

		// todo: Fix the AnalogueGain for now. It should be updated from
		// sensor config
		controls[&controls::AnalogueGain] = ControlInfo((float)100,
								(float)1600,
								(float)100);

		// todo: Assign correct crop range
		const Size &pixelArraySize = properties.get(properties::PixelArraySize).value_or(Size{});
		Rectangle maxCrop = Rectangle{ pixelArraySize };
		controls[&controls::ScalerCrop] = ControlInfo(maxCrop, maxCrop, maxCrop);

		// todo: Assigned the test pattern unconditionally due to the
		// sensor driver is not ready for test patterns. Use the test
		// patterns reported by the sensor when the driver is ready.
		std::vector<ControlValue> patterns;
		patterns.emplace_back(static_cast<int32_t>(controls::draft::TestPatternModeOff));
		patterns.emplace_back(static_cast<int32_t>(controls::draft::TestPatternModeColorBars));
		controls[&controls::draft::TestPatternMode] = ControlInfo(patterns);

		std::vector<ControlValue> supportedFaceDetectModes{
			static_cast<int32_t>(controls::draft::FaceDetectModeOff),
			static_cast<int32_t>(controls::draft::FaceDetectModeSimple)
		};
		controls[&controls::draft::FaceDetectMode] = ControlInfo(supportedFaceDetectModes);
		controls[&controls::draft::AeMode] = ControlInfo(controls::draft::AeModeValues);
		controls[&controls::AeLocked] = ControlInfo(true, false);
		controls[&controls::draft::AeAntiBandingMode] = ControlInfo(controls::draft::AeAntiBandingModeValues);

		controls[&controls::AwbMode] = ControlInfo(controls::AwbModeValues);
		controls[&controls::AwbEnable] = ControlInfo(true, false);
		controls[&controls::AwbLocked] = ControlInfo(true, false);

		controls[&controls::draft::AePrecaptureTrigger] = ControlInfo(controls::draft::AePrecaptureTriggerValues);

		controls[&controls::FrameDuration] = ControlInfo(
			static_cast<int64_t>(33'333'333),
			static_cast<int64_t>(66'333'333),
			static_cast<int64_t>(33'333'333));

		controls[&controls::AfMode] = ControlInfo(controls::AfModeValues);
		controls[&controls::AfTrigger] = ControlInfo(controls::AfTriggerValues);
		controls[&controls::AfWindows] = ControlInfo(Rectangle{}, Rectangle{}, Rectangle{});

		controls[&controls::draft::ColorCorrectionGains] = ControlInfo(0.0f, 100.0f);
		controls[&controls::ColourCorrectionMatrix] = ControlInfo(-100.0f, 100.0f);
		controls[&controls::draft::ColorCorrectionMode] = ControlInfo(controls::draft::ColorCorrectionModeValues);

		controls[&controls::draft::TonemapMode] = ControlInfo(controls::draft::TonemapModeValues);
		controls[&controls::draft::TonemapCurveRed] = ControlInfo(0.0f, 1.0f);
		controls[&controls::draft::TonemapCurveGreen] = ControlInfo(0.0f, 1.0f);
		controls[&controls::draft::TonemapCurveBlue] = ControlInfo(0.0f, 1.0f);

		if (camSysDev_[i].getCameraLens()) {
			// TODO, update minimum focus distance from real lens setting.
			float infiniteFocusDistance = 0.1f;
			float minimumFocusDistance = 1.0f / 0.08f; // 1 / 0.08(m) = 12.5 diopters
			controls[&controls::draft::LensFocusDistance] =
				ControlInfo(infiniteFocusDistance, minimumFocusDistance, 1.0f);
			controls[&controls::LensPosition] = ControlInfo(0.0f, 1000.0f);
		} else {
			controls[&controls::draft::LensFocusDistance] =
				ControlInfo(0.0f, 0.0f);
			controls[&controls::LensPosition] = ControlInfo(0.0f, 0.0f);
		}

		// For now these two controls are ignored.
		// However, because MTK 3A algo is configured to prioritize
		// human face, ignoring any combination of these two controls
		// does not violate Android Camera API specs.
		std::vector<ControlValue> supported3AModes{
			static_cast<uint8_t>(controls::Mode3AOff),
			static_cast<uint8_t>(controls::Mode3AAuto),
			static_cast<uint8_t>(controls::Mode3AUseSceneMode),
		};
		controls[&controls::Mode3A] = ControlInfo(supported3AModes);

		std::vector<ControlValue> supportedSceneModes{
			static_cast<uint8_t>(controls::SceneModeDisabled),
			static_cast<uint8_t>(controls::SceneModeFacePriority),
		};
		controls[&controls::SceneMode] = ControlInfo(supportedSceneModes);

		controls[&controls::draft::EdgeMode] = ControlInfo(controls::draft::EdgeModeValues);

		controls[&controls::draft::StillCaptureMultiFrameNoiseReduction] =
			ControlInfo(false, true, false);

		// Create CameraData
		std::unique_ptr<MtkISP7CameraData> data =
			std::make_unique<MtkISP7CameraData>(
				this, &camSysDev_[i], &imgSysDev_,
				errGyro ? nullptr : &gyroSensor_,
				&onDeviceTuner_, &faceDetector_,
				dmaHeap_.get(), i);
		std::vector<ControlValue> availableFocalLength;
		Rectangle cropRegion = Rectangle{ pixelArraySize };
		switch (PlatformUtils::platform_) {
		case PlatformUtils::MtkISP7Platform::NONE:
			LOG(MtkISP7, Fatal) << "Platform unconfigured";
			break;
		case PlatformUtils::MtkISP7Platform::GOOGLE:
			if (i == 0) {
				// TODO, focal length is not verified
				availableFocalLength.push_back((2.42f));
				cropRegion = Rectangle{ Size{ 4208, 3102 } };
			} else {
				// TODO, focal length is not verified
				availableFocalLength.push_back((2.24f));
				cropRegion = Rectangle{ Size{ 3264, 2448 } };
			}
			break;
		case PlatformUtils::MtkISP7Platform::LENOVO:
			if (i == 0) {
				availableFocalLength.push_back((2.42f));
				cropRegion = Rectangle{ Size{ 3264, 2448 } };
			} else {
				availableFocalLength.push_back((2.24f));
				cropRegion = Rectangle{ Size{ 2592, 1944 } };
			}
			break;
		}
		controls[&controls::draft::LensFocalLength] = ControlInfo(availableFocalLength);
		properties.set(controls::ScalerCrop, cropRegion);
		std::set<Stream *> streams = { &data->video1Stream_,
					       &data->video2Stream_,
					       &data->still1Stream_ };

		data->sensorFullSize_ = pixelArraySize;
		data->properties_ = properties;
		data->controlInfo_ = ControlInfoMap(std::move(controls), controls::controls);

		// Create and register the Camera
		std::shared_ptr<Camera> camera =
			Camera::create(std::move(data), camSysDev_[i].cameraId(), streams);
		registerCamera(std::move(camera));

		ImagiqAdapter::sensorIdMap.emplace(
			camSysDev_[i].cameraId(),
			NSCam::TuningUtils::eSensorId(i));

		sensorCnt++;
		LOG(MtkISP7, Info) << "Registered Camera[" << camSysDev_[i].cameraId() << "]";
	}

	if (sensorCnt < 2)
		return false;

	// TODO: Only init imgsys when there is sensor detected.
	// A temporary hack for factory testing. Find a more proper way to
	// handle this case.
	imgSysDev_.init(imgSysMedia_, dmaHeap_.get());
	std::vector<SensorInfo::CamSysData> dataArray;
	for (const auto &data : camSysDataArray_) {
		dataArray.emplace_back(data.has_af, data.mbus_code);
	}
	SensorInfo::add_sensor(dataArray);

	// TODO(chenghaoyang): Check if this is necessary. Theoretically
	// libcamera doesn't depend on this API anymore.
	NSCam::IHalSensorList *const pHalSensorList = NSCam::IHalSensorList::get();
	pHalSensorList->searchSensors();

	return true;
}

bool MtkISP7CameraData::loadIPA()
{
	if (ipa_->isValid())
		return true;

	std::string eepromPath;

	switch (PlatformUtils::platform_) {
	case PlatformUtils::MtkISP7Platform::NONE:
		LOG(MtkISP7, Fatal) << "Platform unconfigured";
		break;

	case PlatformUtils::MtkISP7Platform::GOOGLE:
		if (sensor_idx_ == 0)
			eepromPath = "/sys/bus/i2c/devices/6-0058/eeprom";
		else
			eepromPath = "/sys/bus/i2c/devices/5-0051/eeprom";

		break;

	case PlatformUtils::MtkISP7Platform::LENOVO:
		if (sensor_idx_ == 0)
			eepromPath = "/sys/bus/i2c/devices/6-0058/eeprom";
		else
			eepromPath = "/sys/bus/i2c/devices/5-0050/eeprom";

		break;
	}

	std::vector<uint8_t> buffer;
	std::ifstream file(eepromPath, std::ios::in | std::ios::binary | std::ios::ate);

	if (file.is_open()) {
		// Get the file size
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		// Create a buffer to hold the file contents
		buffer.resize(size);

		// Read the entire file into the buffer
		if (!file.read((char *)buffer.data(), size)) {
			LOG(MtkISP7, Fatal) << "Error reading file.";
		} else {
			LOG(MtkISP7, Info) << "Eeprom read size: " << size;
		}

		file.close();
	} else {
		LOG(MtkISP7, Fatal) << "Unable to open file: " << eepromPath;
	}

	auto *pipeline = static_cast<PipelineHandlerMtkISP7 *>(pipe());

	auto ipa = IPAManager::createIPA<ipa::mtkisp7::IPAProxyMtkISP7>(pipe(), 1, 1, true, IPCPipeUnixSocket::kCpuPath);
	if (!ipa) {
		LOG(MtkISP7, Error) << "Failed to load IPA";
		return false;
	}

	auto *signalDisconnected = ipa->disconnected();
	if (signalDisconnected) {
		signalDisconnected->connect(this, &MtkISP7CameraData::IPADisconnected);
	} else {
		LOG(MtkISP7, Error) << "Couldn't get signal disconnected";
	}

	if (ipa_->init(std::move(ipa), PlatformUtils::model_, sensor_idx_,
		       buffer, pipeline->camSysDataArray_)) {
		LOG(MtkISP7, Error) << "IPA init failed";
		return false;
	}

	return true;
}

int MtkISP7CameraData::start([[maybe_unused]] const ControlList *controls)
{
	auto *pipeline = static_cast<PipelineHandlerMtkISP7 *>(pipe());
	auto *scheduler = pipeline->scheduler_.get();
	control_cache_.reset();
	camSysDev_->frameStart().disconnect(this);
	camSysDev_->frameStart().connect(this, &MtkISP7CameraData::frameStart);

	camSysDev_->start();

	imgSysDev_->start();

	mcnrManager.start();
	lpnrManager.start();
	faceDetector_->start();

	mfnrManager.start();

	if (gyroSensor_)
		gyroSensor_->startReading(30); // Assume FPS == 30

	// AieDev_ allocate buffers in `FaceDetector::start()`;
	allocateIPABuffers();
	int ret;

	int32_t lens_position;
	ipa_->start(
		hal3AManager_.getDummyTuning().second->get().buffer()->cookie(),
		&initSensorSetting_, &lens_position);
	if (lens_position < 0)
		return -EINVAL;

	ret = hal3AManager_.start(lens_position);
	if (ret)
		return ret;

	scheduler->schedule();
	return 0;
}

std::tuple<QueueTask *, DequeueTask *, SofTask *, AAATask *, uint32_t>
MtkISP7CameraData::makeTasks(const std::string &id, Request *request,
			     CaptureFrames &captureFrames,
			     uint32_t internalRequestId, bool hasStillCapture,
			     bool needRaw, bool hasVideo, bool useMfnr)
{
	auto *pipeline = static_cast<PipelineHandlerMtkISP7 *>(pipe());
	auto *scheduler = pipeline->scheduler_.get();

	captureManager.makeCaptureFrames(
		captureFrames, needRaw || onDeviceTuner_->isEnabled(),
		(useMfnr && needRaw) || hasVideo, hasVideo);

	if (internalRequestId >= CaptureTasksManager::kAAToSofDelay) {
		uint32_t aaRequestId = internalRequestId - CaptureTasksManager::kAAToSofDelay;
		CaptureResult *aaCaptureResult = captureResult_.query(aaRequestId);
		captureFrames.exposureAndGain = aaCaptureResult->exposureAndGainOutput;
	} else {
		captureFrames.exposureAndGain = makeMailBox<ipa::mtkisp7::SensorSetting>();
		captureFrames.exposureAndGain->put(initSensorSetting_, nullptr);
	}

	uint32_t camSysMetaRequestId = 0;
	if (internalRequestId >= CaptureTasksManager::kRawMetaDelay) {
		camSysMetaRequestId = internalRequestId - CaptureTasksManager::kRawMetaDelay;
		CaptureResult *aaCaptureResult = captureResult_.query(camSysMetaRequestId);
		captureFrames.tuning = aaCaptureResult->tuningOutput;
	} else {
		auto [dummyId, dummyTuning] = hal3AManager_.getDummyTuning();
		camSysMetaRequestId = dummyId;
		captureFrames.tuning = dummyTuning;
	}

	CaptureResult captureResult;
	captureResult.tuningOutput = captureFrames.tuningOutput;
	captureResult.exposureAndGainOutput = captureFrames.exposureAndGainOutput;

	captureResult_.add(internalRequestId, captureResult);

	AaaIspExchangeResult aaaIspExchangeResult;
	aaaIspExchangeResult.camSysMetaRequestId = camSysMetaRequestId;
	aaaIspExchangeResult.aaaIspExchange = captureFrames.aaaIspExchange;

	aaaIspExchangeResult_.add(internalRequestId, aaaIspExchangeResult);
	SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange;
	if (request) {
		aaaIspExchange = aaaIspExchangeResult_.query(camSysMetaRequestId)
					 ->aaaIspExchange;
	}

	auto [taskQBuf, taskDQBuf, sofTask] = captureManager.makeCaptureTasks(
		scheduler, id, request, captureFrames, internalRequestId,
		&hal3AManager_, aaaIspExchange);

	if (onDeviceTuner_->isEnabled()) {
		onDeviceTuner_->notifyRequestBegin(internalRequestId);
		bool captureInProcessing = hasStillCapture;
		if (captureInProcessing) {
			if (useMfnr) {
				onDeviceTuner_->notifyStillCapture(internalRequestId, internalRequestId);
				onDeviceTuner_->notifyStillCapture(internalRequestId, internalRequestId + 1);
				onDeviceTuner_->notifyStillCapture(internalRequestId, internalRequestId + 2);
				onDeviceTuner_->notifyStillCapture(internalRequestId, internalRequestId + 3);
			} else {
				onDeviceTuner_->notifyStillCapture(internalRequestId, internalRequestId);
			}
		}

		// Make sure it's ahead of everything else.
		if (captureInProcessing && useMfnr) {
			ipa_->notifyRequestBegin(internalRequestId, internalRequestId, captureInProcessing);
			ipa_->notifyRequestBegin(internalRequestId, internalRequestId + 1, captureInProcessing);
			ipa_->notifyRequestBegin(internalRequestId, internalRequestId + 2, captureInProcessing);
			ipa_->notifyRequestBegin(internalRequestId, internalRequestId + 3, captureInProcessing);
		} else {
			ipa_->notifyRequestBegin(internalRequestId, internalRequestId, captureInProcessing);
		}
	}
	auto *aaaTask = hal3AManager_.make3ATasks(
		scheduler, request, captureFrames, internalRequestId,
		camSysMetaRequestId, faceDetector_);

	setTasksDependencies(taskQBuf, taskDQBuf, sofTask, aaaTask);

	return std::make_tuple(taskQBuf, taskDQBuf, sofTask,
			       aaaTask, camSysMetaRequestId);
}

void MtkISP7CameraData::setTasksDependencies(
	QueueTask *taskQBuf, DequeueTask *taskDQBuf, SofTask *sofTask,
	AAATask *aaaTask)
{
	auto *pipeline = static_cast<PipelineHandlerMtkISP7 *>(pipe());
	auto *scheduler = pipeline->scheduler_.get();

	Scheduler::precede(sofTask, taskDQBuf);
	Scheduler::precede(taskQBuf, taskDQBuf);
	Scheduler::precede(taskDQBuf, aaaTask);

	if (taskQBuf->data_->frames.raw) {
		// TODO: check if raw buffers released there.
		scheduler->succeedPrevTaskByStep(BfbldTaskGroup, 0, taskQBuf);
	}

	scheduler->succeedPrevTaskByStep(CaptureQueueGroup, 0, taskQBuf);
	scheduler->succeedPrevTaskByStep(CaptureDequeueGroup, 0, taskDQBuf);
	scheduler->succeedPrevTaskByStep(AAAGroup, 0, aaaTask);

	scheduler->succeedPrevTaskByStep(AAAGroup, CaptureTasksManager::kAAToSofDelay - 1, sofTask);
	scheduler->succeedPrevTaskByStep(SofGroup, CaptureTasksManager::kExposureAndGainDelay - 1, taskQBuf);
	scheduler->succeedPrevTaskByStep(AAAGroup, CaptureTasksManager::kRawMetaDelay - 1, taskQBuf);
	// TODO(chenghaoyang@): Check again after we confirm the logic to switch between lpnr & mfnr.
	scheduler->succeedPrevTaskByStep(XtrGroup, MFNR_QUEUE_SIZE - 1, taskQBuf);
	int32_t pipelineDepth = controlInfo_.at(&controls::draft::PipelineDepth)
					.max()
					.get<int32_t>();
	scheduler->succeedPrevTaskByStep(CompleteGroup, pipelineDepth - 1, taskQBuf);

	/* At most 5 request can be queued into CamSys */
	scheduler->succeedPrevTaskByStep(CaptureDequeueGroup, 4, taskQBuf);

	scheduler->queueTask(sofTask, SofGroup);
	scheduler->queueTask(taskQBuf, CaptureQueueGroup);
	scheduler->queueTask(taskDQBuf, CaptureDequeueGroup);
	scheduler->queueTask(aaaTask, AAAGroup);

	pendingSofTasks_.push_back(sofTask);
}

void MtkISP7CameraData::allocateIPABuffers()
{
	ipaBufferCnt_ = 1;

	registerIPABuffers(&captureManager.faceDetectPool_);
	registerIPABuffers(&faceDetector_->resultMetadataPool_);

	registerIPABuffers(&captureManager.yuvo1Pool_);
	registerIPABuffers(&captureManager.statistics0Pool_);
	registerIPABuffers(&captureManager.statistics1Pool_);
	registerIPABuffers(&hal3AManager_.tuningPool_);

	registerIPABuffers(&lpnrManager.lpnrStt_);
	registerIPABuffers(&lpnrTunManager.lpnrTun_);

	registerIPABuffers(&mcnrTunManager.fwmeFst_);
	registerIPABuffers(&mcnrTunManager.fwmmFst_);
	registerIPABuffers(&mcnrTunManager.fwmmRst_);
	registerIPABuffers(&mcnrTunManager.fwmmMil_);
	registerIPABuffers(&mcnrTunManager.fwmmGyro_);
	registerIPABuffers(&mcnrTunManager.swHist_);
	registerIPABuffers(&mcnrTunManager.meTun_);
	registerIPABuffers(&mcnrTunManager.wpeTun_);
	registerIPABuffers(&mcnrTunManager.dipTun_);
	registerIPABuffers(&mcnrTunManager.trawTun_);
	registerIPABuffers(&mcnrManager.fwmeFst_);
	registerIPABuffers(&mcnrManager.fwmmFst);
	registerIPABuffers(&mcnrManager.fwmmRst_);
	registerIPABuffers(&mcnrManager.fwmmMil_);
	registerIPABuffers(&mcnrManager.fwmmGyro_);
	registerIPABuffers(&mcnrManager.meIn_);
	registerIPABuffers(&mcnrManager.meMv0_);
	registerIPABuffers(&mcnrManager.meMv1_);
	registerIPABuffers(&mcnrManager.meFst_);
	registerIPABuffers(&mcnrManager.meLmi_);
	registerIPABuffers(&mcnrManager.meFmb0_);
	registerIPABuffers(&mcnrManager.meFmb1_);
	registerIPABuffers(&mcnrManager.meMmap0_);
	registerIPABuffers(&mcnrManager.meMmap1_);
	registerIPABuffers(&mcnrManager.meMmap2_);
	registerIPABuffers(&mcnrManager.meMmap3_);
	registerIPABuffers(&mcnrManager.meConf0_);
	registerIPABuffers(&mcnrManager.meConf4_);
	registerIPABuffers(&mcnrManager.meConf5_);
	registerIPABuffers(&mcnrManager.trawStt_);
	registerIPABuffers(&mcnrManager.idi_);
	registerIPABuffers(&mcnrManager.tnrSo_);
	registerIPABuffers(&mcnrManager.img4oF0_);
	registerIPABuffers(&mcnrManager.img4oF1_);

	for (unsigned i = 0; i < mcnrManager.wt_.size(); ++i)
		registerIPABuffers(&mcnrManager.wt_[i]);
	for (unsigned i = 0; i < mcnrManager.img3o_.size(); ++i)
		registerIPABuffers(&mcnrManager.img3o_[i]);
	for (unsigned i = 0; i < mcnrManager.tnrmo_.size(); ++i)
		registerIPABuffers(&mcnrManager.tnrmo_[i]);
	for (unsigned i = 0; i < mcnrManager.vbi_.size(); ++i)
		registerIPABuffers(&mcnrManager.vbi_[i]);

	registerIPABuffers(&mfnrTunManager.mfnrTun_);

	registerIPABuffers(&mfnrTunManager.bssParamPool_);
	registerIPABuffers(&mfnrTunManager.bssDataGPool_);
	registerIPABuffers(&mfnrTunManager.bssVerPool_);
	registerIPABuffers(&mfnrTunManager.bssTuningPool_);
	registerIPABuffers(&mfnrTunManager.bssFdMainPool_);
	registerIPABuffers(&mfnrTunManager.bssFdPool_);
	registerIPABuffers(&mfnrTunManager.bssFacePool_);
	registerIPABuffers(&mfnrTunManager.bssPosPool_);
	registerIPABuffers(&mfnrTunManager.bssOutDataPool_);

	registerIPABuffers(&mfnrTunManager.tnrciPool_);
	registerIPABuffers(&mfnrTunManager.wrap2pPool_);
	registerIPABuffers(&mfnrTunManager.fourBytes_1_16_pool_);
	registerIPABuffers(&mfnrTunManager.swmeOutPool_);
	registerIPABuffers(&mfnrTunManager.swmeParamPool_);
	registerIPABuffers(&mfnrTunManager.swmeTuningPool_);

	registerIPABuffers(&mfnrManager.p2sttoPool_);
	registerIPABuffers(&mfnrManager.y8_1_4_pool_aligned16_);
}

void MtkISP7CameraData::registerIPABuffers(InfoFramePool *pool)
{
	std::vector<IPABuffer> ipaBuffers;
	for (std::unique_ptr<FrameBuffer> &buffer : pool->content()) {
		buffer->setCookie(ipaBufferCnt_++);
		ipaBuffers.emplace_back(buffer->cookie(), buffer->planes());
		ipaBufferIds_.emplace_back(buffer->cookie());
	}

	ipa_->mapBuffers(ipaBuffers);
}

void MtkISP7CameraData::freeIPABuffers()
{
	ipa_->unmapBuffers(ipaBufferIds_);
	ipaBufferIds_.clear();
}

void MtkISP7CameraData::IPADisconnected()
{
	ipa_->releaseProxy();

	if (isAcquired())
		notifyDisconnection();
}

void MtkISP7CameraData::stopDevice()
{
	camSysDev_->frameStart().disconnect(this);

	mfnrInput_.release();

	camSysDev_->stop();
	imgSysDev_->stop();

	/* Release the transient frames for MCNR before stopping mcnrManger */
	mcnrPrev = {};

	mcnrManager.stop();
	lpnrManager.stop();

	mfnrManager.stop();

	faceDetector_->stop();

	if (gyroSensor_)
		gyroSensor_->stopReading();
	frameSequence_ = 0;

	ipa_->stop();
	freeIPABuffers();
}

bool MtkISP7CameraData::acquireDevice()
{
	return loadIPA();
}

void MtkISP7CameraData::releaseDevice()
{
	captureResult_.release();
	requestCount_ = 0;

	ipa_->releaseProxy();

	faceDetector_->releaseBuffers();

	imgSysDev_->releaseAllBuffers();
	camSysDev_->releaseAllBuffers();

	/* Release the transient frames of MCNR */
	mcnrPrev = {};

	captureManager.releaseBuffers();
	hal3AManager_.releaseBuffers();
	mcnrManager.releaseBuffers();
	lpnrManager.releaseBuffers();

	lpnrTunManager.releaseBuffers();
	mcnrTunManager.releaseBuffers();

	mfnrManager.releaseBuffers();
	mfnrTunManager.releaseBuffers();
}

/*
 * \brief Handle the start of frame exposure signal
 * \param[in] sequence The sequence number of frame
 */
void MtkISP7CameraData::frameStart(uint32_t sequence)
{
	if (sequence <= frameSequence_) {
		LOG(MtkISP7, Debug) << "Underrun dropping " << sequence;
		return;
	}

	while (frameSequence_ < sequence) {
		++frameSequence_;

		if (pendingSofTasks_.empty()) {
			LOG(MtkISP7, Error) << "Frame Start No Sof tasks when camsys writes a frame " << frameSequence_;
			continue;
		}

		SofTask *task = pendingSofTasks_.front();
		pendingSofTasks_.pop_front();

		task->trigger();
	}
}

int MtkISP7CameraData::configure(CameraConfiguration *c)
{
	Size camsysYuvSize;
	Size video1 = Size{ 0, 0 };
	Size video2 = Size{ 0, 0 };
	Size still1 = Size{ 0, 0 };
	Size still2 = Size{ 0, 0 };

	/* Only cover the video resolution */
	for (auto &cfg : *c) {
		if (cfg.stream() == &video1Stream_)
			video1 = cfg.size;
		else if (cfg.stream() == &video2Stream_)
			video2 = cfg.size;
		else if (cfg.stream() == &still1Stream_)
			still1 = cfg.size;
		else if (cfg.stream() == &still2Stream_)
			still2 = cfg.size;
		else
			return -EINVAL;
	}

	camsysYuvSize.expandTo(video1);
	camsysYuvSize.expandTo(video2);

	/* Only support 4:3 resolution as output of CamSys */
	std::vector<Size> supportedSize = {
		{ 1280, 960 },
		{ 1440, 1080 },
		{ 1600, 1200 },
		{ 1920, 1440 },
		{ 2560, 1920 },
	};

	/* Support sensor full size. */
	supportedSize.push_back(sensorFullSize_);

	/* Find the smallest supported size which covers video streams */
	bool found = false;
	for (auto &size : supportedSize) {
		if (size >= camsysYuvSize) {
			camsysYuvSize = size;
			found = true;
			break;
		}
	}

	if (!found) {
		LOG(MtkISP7, Error)
			<< "Video/Preview resolution is larger than the limit "
			<< supportedSize.back();
		return -EINVAL;
	}

	bool isVideo;
	switch (c->captureIntent) {
	case CameraConfiguration::CaptureIntent::Video:
		isVideo = true;
		break;
	case CameraConfiguration::CaptureIntent::StillCapture:
		isVideo = false;
		break;
	case CameraConfiguration::CaptureIntent::Unknown:
	default:
		// If any still capture stream is requested, assume it be still mode.
		isVideo = (still1.isNull() && still2.isNull());
		break;
	}

	auto *pipeline = static_cast<PipelineHandlerMtkISP7 *>(pipe());

	onDeviceTuner_->configure(camSysDev_->cameraId(),
				  camSysDev_->getIndex(), 0, ipa_.get());
	camSysDev_->configure(sensorFullSize_, camsysYuvSize);

	Size wrappingMapSize, confMapSize;
	ipa_->configure(camsysYuvSize, faceDetector_,
			video1 > video2 ? video1 : video2,
			still1 > still2 ? still1 : still2, camSysDev_->cameraId(),
			camSysDev_->getIndex(),
			onDeviceTuner_->getSessionTimestamp(),
			isVideo, sensorFullSize_,
			MfnrTasksManager::getSizeAligned(sensorFullSize_),
			&wrappingMapSize, &confMapSize);

	mfnrManager.configure(&mfnrInput_, sensorFullSize_,
			      still1, still2,
			      video1, video2,
			      confMapSize,
			      sensor_idx_);
	mfnrTunManager.configure(sensorFullSize_, still1, still2,
				 MfnrTasksManager::getSizeAligned(sensorFullSize_),
				 wrappingMapSize,
				 confMapSize);

	imgSysDev_->configure(sensorFullSize_, camsysYuvSize,
			      video1, video2, still1, still2, wrappingMapSize, confMapSize);

	int32_t pipelineDepth = controlInfo_.at(&controls::draft::PipelineDepth)
					.max()
					.get<int32_t>();

	captureManager.configure(dmaHeap_, camSysDev_, pipeline, &mfnrInput_, sensorFullSize_,
				 camsysYuvSize, pipelineDepth);
	faceDetector_->configure(sensorFullSize_, ipa_.get());
	hal3AManager_.configure(dmaHeap_, camSysDev_, gyroSensor_, ipa_.get());

	mcnrManager.configure(camsysYuvSize, video1, video2);
	lpnrManager.configure(sensorFullSize_, still1, still2);
	lpnrTunManager.configure(sensorFullSize_, still1, still2);
	mcnrTunManager.configure(camsysYuvSize, video1, video2);

	return 0;
}

int MtkISP7CameraData::queueRequest(Request *request)
{
	FrameBuffer *video1Buffer = request->findBuffer(&video1Stream_);
	FrameBuffer *video2Buffer = request->findBuffer(&video2Stream_);
	FrameBuffer *still1Buffer = request->findBuffer(&still1Stream_);
	FrameBuffer *still2Buffer = request->findBuffer(&still2Stream_);

	if (!video1Buffer && !video2Buffer && !still1Buffer && !still2Buffer)
		return -EINVAL;

	auto *pipeline = static_cast<PipelineHandlerMtkISP7 *>(pipe());
	auto *scheduler = pipeline->scheduler_.get();

	std::string sequence = std::to_string(request->sequence());

	// TODO: Implement the padding condition for per-frame control
	std::shared_ptr<ControlList> controls_cur = std::make_shared<ControlList>(request->controls());

	bool aaControlChanged = is3aControlChanged(controls_cur, control_cache_);

	control_cache_ = controls_cur;

	bool nddEnabled = onDeviceTuner_->isEnabled();
	bool hasStillCapture = still1Buffer || still2Buffer;

	bool useMfnr = request->controls().get(controls::draft::StillCaptureMultiFrameNoiseReduction).value_or(false);

	if (requestCount_ == 0 || aaControlChanged || nddEnabled) {
		size_t needed = 0;

		if (requestCount_ == 0 || aaControlChanged || nddEnabled) {
			std::list<Task *> &capture3ATasks = scheduler->groupTasks(AAAGroup);
			size_t numberOfPending3ATasks = 0;
			for (auto *task : capture3ATasks)
				if (!task->isRunning())
					numberOfPending3ATasks++;

			if (CaptureTasksManager::kRawMetaDelay > numberOfPending3ATasks)
				needed = CaptureTasksManager::kRawMetaDelay - numberOfPending3ATasks;
		}

		for (size_t i = 0; i < needed; ++i) {
			bool needRaw = needed <= i + 3;
			CaptureFrames captureFrames;
			uint32_t internalRequestId = requestCount_++;
			makeTasks("Padding capture", nullptr, captureFrames,
				  internalRequestId, false,
				  needRaw,
				  false, useMfnr);
		}
	}

	bool onlyStillCapture = hasStillCapture && (!video1Buffer && !video2Buffer);

	uint32_t internalRequestId = requestCount_++;
	if (hasStillCapture) {
		latestStillCapture_ = internalRequestId;
	} else {
		if (latestStillCapture_ != 0 &&
		    internalRequestId == latestStillCapture_ + 100) {
			LOG(MtkISP7, Debug) << "Resetting still capture buffers";
			pipeline->imgSysDev_.resetBuffers(ImgSysDevice::kUserIdLpnr);
			lpnrManager.releaseElasticBuffers();
			pipeline->imgSysDev_.resetBuffers(ImgSysDevice::kUserIdMfnr);
			mfnrManager.releaseElasticBuffers();

			captureManager.releaseElasticBuffers();
		}
	}

	// todo(yerlandinata): check whether we need Feature::video or not.
	Feature feature = Feature::Preview;
	if (hasStillCapture) {
		if (useMfnr)
			feature = Feature::Capture_mfnr;
		else
			feature = Feature::Capture_lpnr;
	}

	if (aaControlChanged || nddEnabled) {
		std::list<Task *> &capture3ATasks = scheduler->groupTasks(AAAGroup);
		if (capture3ATasks.size() >= CaptureTasksManager::kRawMetaDelay) {
			auto iter = capture3ATasks.rbegin();
			for (uint32_t shift = 0; shift < CaptureTasksManager::kRawMetaDelay; ++shift) {
				auto *prevAAATask = static_cast<AAATask *>(*iter);
				prevAAATask->setInternalRequestIdApplied(internalRequestId);
				prevAAATask->setFeatureApplied(feature);
				prevAAATask->setPerFrameControl(
					AAATask::PerFrameControl{
						.delayIdx = static_cast<int>(shift),
						// TODO: Rename the parameter to onlyStillCapture.
						.isStillCapture = onlyStillCapture,
						.controls = request->controls() });
				iter++;
			}
		}
	}

	CaptureFrames captureFrames;

	bool hasVideo = video1Buffer || video2Buffer;
	auto [taskQBuf, taskDQBuf, sofTask, aaaTask, camSysMetaRequestId] = makeTasks(
		"Capture " + sequence, request, captureFrames, internalRequestId,
		hasStillCapture, true, hasVideo, useMfnr);
	aaaTask->setPerFrameControl(
		AAATask::PerFrameControl{
			.delayIdx = 0,
			.isStillCapture = hasStillCapture,
			.controls = request->controls() });

	Task *taskTr = nullptr;
	Task *taskDip2 = nullptr;

	auto aaaIspExchange = aaaIspExchangeResult_.query(camSysMetaRequestId)->aaaIspExchange;

	std::set<const Stream *> mfnrStreams;
	if (useMfnr) {
		mfnrStreams.insert(&still1Stream_);
		mfnrStreams.insert(&still2Stream_);
	}
	CompleteRequestTask *completeTask = new CompleteRequestTask(
		scheduler, "Complete " + sequence, request, internalRequestId,
		camSysMetaRequestId, pipeline, ipa_.get(), onDeviceTuner_,
		faceDetector_, aaaIspExchange, mfnrStreams);

	/* Face Detection Task */
	Task *faceDetectTask = faceDetector_->makeFaceDetectionTask(
		scheduler, request, captureFrames.faceDetection,
		aaaTask->camSysMetaRequestId_);

	Scheduler::precede(taskDQBuf, faceDetectTask);
	Scheduler::precede(faceDetectTask, completeTask);
	scheduler->succeedPrevTaskByStep(AieFaceDetectionGroup, 0, faceDetectTask);

	scheduler->queueTask(faceDetectTask, AieFaceDetectionGroup);

	if (hasVideo) {
		MCNRFrames mcnr;
		mcnrManager.makeMCNRFrames(mcnr, mcnrPrev,
					   captureFrames.me,
					   captureFrames.yuvo1,
					   captureFrames.yuvo2,
					   video1Buffer,
					   video2Buffer);

		auto [meATunTask, meBTunTask, trTunTask, dipTunTask] =
			mcnrTunManager.makeMcnrTunTasks(mcnr, camSysMetaRequestId, scheduler,
							"MCNR " + sequence, request, internalRequestId);

		auto [taskMeA, taskMeB, tempTaskTr, taskDip1, tempTaskDip2] =
			mcnrManager.makeMcnrTasks(mcnr, scheduler, "MCNR " + sequence,
						  request, internalRequestId, imgSysDev_);

		taskTr = tempTaskTr;
		taskDip2 = tempTaskDip2;

		// Limit the interval from a producer task of tuning buffers
		// to its corresponding consumer task as 2, and we only need
		// to allocate 3 set of the tuning buffers instead of 8.
		// The reason for the regulation is that the V4L2 will cause
		// cache miss, it there are too many associated FDs for one
		// video node, and all of the tuning buffers are using the
		// video node for ImgSys.
		scheduler->succeedPrevTaskByStep(MeAGroup, 2, meATunTask);
		scheduler->succeedPrevTaskByStep(MeBGroup, 2, meBTunTask);
		scheduler->succeedPrevTaskByStep(TrGroup, 2, trTunTask);
		scheduler->succeedPrevTaskByStep(Dip2Group, 2, dipTunTask);

		// TODO: Add the dependency due to that taskMeB holds trMeTun
		// which only used by TaskMeA. Removes the dependency once we
		// resolve the life time.
		scheduler->succeedPrevTaskByStep(MeATunGroup, 2, taskMeB);

		// Block the taskMe until latest MFNR task to finish due to
		// imgsys resource limit.
		scheduler->succeedPrevTaskByStep(AfbldGroup, 0, taskMeA);

		Scheduler::precede(taskDQBuf, meATunTask);
		scheduler->succeedPrevTaskByStep(MeATunGroup, 0, meATunTask);
		scheduler->succeedPrevTaskByStep(MeBTunGroup, 0, meATunTask);
		scheduler->queueTask(meATunTask, MeATunGroup);

		Scheduler::precede(meATunTask, taskMeA);
		Scheduler::precede(taskDQBuf, taskMeA);
		scheduler->succeedPrevTaskByStep(MeAGroup, 0, taskMeA);
		scheduler->queueTask(taskMeA, MeAGroup);

		Scheduler::precede(taskMeA, meBTunTask);
		scheduler->succeedPrevTaskByStep(MeBTunGroup, 0, meBTunTask);
		scheduler->queueTask(meBTunTask, MeBTunGroup);

		Scheduler::precede(meBTunTask, taskMeB);
		Scheduler::precede(taskMeA, taskMeB);
		scheduler->succeedPrevTaskByStep(MeBGroup, 0, taskMeB);
		scheduler->queueTask(taskMeB, MeBGroup);

		Scheduler::precede(taskMeB, trTunTask);
		scheduler->succeedPrevTaskByStep(TrTunGroup, 0, trTunTask);
		scheduler->queueTask(trTunTask, TrTunGroup);

		Scheduler::precede(trTunTask, taskTr);
		Scheduler::precede(taskMeB, taskTr);
		scheduler->succeedPrevTaskByStep(TrGroup, 0, taskTr);
		scheduler->queueTask(taskTr, TrGroup);

		Scheduler::precede(taskTr, dipTunTask);
		scheduler->succeedPrevTaskByStep(DipTunGroup, 0, dipTunTask);
		scheduler->queueTask(dipTunTask, DipTunGroup);

		Scheduler::precede(dipTunTask, taskDip1);
		Scheduler::precede(taskTr, taskDip1);
		scheduler->succeedPrevTaskByStep(Dip1Group, 0, taskDip1);
		scheduler->queueTask(taskDip1, Dip1Group);

		Scheduler::precede(taskDip1, taskDip2);
		scheduler->succeedPrevTaskByStep(Dip2Group, 0, taskDip2);
		scheduler->queueTask(taskDip2, Dip2Group);

		Scheduler::precede(taskDip2, completeTask);
	}

	if (hasStillCapture) {
		if (useMfnr) {
			LOG(MtkISP7, Info) << "[CAT][MFNR] Trigger MFNR !";
			MFNRFrames mfnr;
			uint32_t mfnrRequestId = mfnrInput_.lastId();
			mfnrManager.makeMFNRFrames(mfnr, mfnrRequestId, still1Buffer, still2Buffer);

			uint32_t prevCamSysMetaRequestId = aaaIspExchangeResult_.query(mfnrRequestId)->camSysMetaRequestId;
			auto prevAaaIspExchange = aaaIspExchangeResult_.query(prevCamSysMetaRequestId)->aaaIspExchange;

			auto [mfnrTunBssTask, mfnrTunBfbldTask, mfnrTunBfmeTask,
			      mfnrTunSwmeTask, mfnrTunDsTask, mfnrTunDsVbiTask, mfnrTunMcdsF1Task,
			      mfnrTunMsbldTask1st, mfnrTunMsbldTask2nd, mfnrTunAfbldTask] =
				mfnrTunManager.makeMfnrTunTasks(mfnr, prevAaaIspExchange, prevCamSysMetaRequestId, scheduler, "MfnrTun " + std::to_string(mfnrRequestId), request, mfnrRequestId);

			auto [mfnrBfbldTask, mfnrBfmeTask, mfnrMcdsF1Task,
			      mfnrDsTask, mfnrDsVbiTask, mfnrMsbldTask1st,
			      mfnrMsbldTask2nd, mfnrAfbldTask] =
				mfnrManager.makeMfnrTasks(mfnr, scheduler,
							  "Mfnr " + std::to_string(mfnrRequestId),
							  request, mfnrRequestId,
							  imgSysDev_, pipeline);

			scheduler->succeedPrevTaskByStep(BssTunTaskGroup, 0, mfnrTunBssTask);
			scheduler->queueTask(mfnrTunBssTask, BssTunTaskGroup);
			Scheduler::precede(mfnrTunBssTask, mfnrTunBfbldTask);
			Scheduler::precede(mfnrTunBfbldTask, mfnrBfbldTask);
			scheduler->succeedPrevTaskByStep(BfbldTunTaskGroup, 0, mfnrTunBfbldTask);
			scheduler->queueTask(mfnrTunBfbldTask, BfbldTunTaskGroup);

			Scheduler::precede(mfnrTunBssTask, mfnrBfbldTask);
			scheduler->succeedPrevTaskByStep(BfbldTaskGroup, 0, mfnrBfbldTask);
			scheduler->queueTask(mfnrBfbldTask, BfbldTaskGroup);

			Scheduler::precede(mfnrTunBssTask, mfnrTunBfmeTask);
			Scheduler::precede(mfnrBfbldTask, mfnrTunBfmeTask);
			Scheduler::precede(mfnrTunBfmeTask, mfnrBfmeTask);
			scheduler->succeedPrevTaskByStep(BfmeTunGroup, 0, mfnrTunBfmeTask);
			scheduler->queueTask(mfnrTunBfmeTask, BfmeTunGroup);

			Scheduler::precede(mfnrBfbldTask, mfnrBfmeTask);
			scheduler->succeedPrevTaskByStep(BfmeGroup, 0, mfnrBfmeTask);
			scheduler->queueTask(mfnrBfmeTask, BfmeGroup);

			Scheduler::precede(mfnrBfmeTask, mfnrTunSwmeTask);
			scheduler->succeedPrevTaskByStep(SwmeTunGroup, 0, mfnrTunSwmeTask);
			scheduler->queueTask(mfnrTunSwmeTask, SwmeTunGroup);

			Scheduler::precede(mfnrTunSwmeTask, mfnrTunMcdsF1Task);
			Scheduler::precede(mfnrTunMcdsF1Task, mfnrMcdsF1Task);
			scheduler->succeedPrevTaskByStep(McdsF1TunGroup, 0, mfnrTunMcdsF1Task);
			scheduler->queueTask(mfnrTunMcdsF1Task, McdsF1TunGroup);

			Scheduler::precede(mfnrBfbldTask, mfnrMcdsF1Task);
			scheduler->succeedPrevTaskByStep(McdsF1Group, 0, mfnrMcdsF1Task);
			scheduler->queueTask(mfnrMcdsF1Task, McdsF1Group);

			Scheduler::precede(mfnrTunBssTask, mfnrTunDsTask);
			Scheduler::precede(mfnrTunMcdsF1Task, mfnrTunDsTask);
			Scheduler::precede(mfnrTunDsTask, mfnrDsTask);
			scheduler->succeedPrevTaskByStep(DsTunGroup, 0, mfnrTunDsTask);
			scheduler->queueTask(mfnrTunDsTask, DsTunGroup);

			Scheduler::precede(mfnrMcdsF1Task, mfnrDsTask);
			scheduler->succeedPrevTaskByStep(DsGroup, 0, mfnrDsTask);
			scheduler->queueTask(mfnrDsTask, DsGroup);

			Scheduler::precede(mfnrTunDsTask, mfnrTunDsVbiTask);
			Scheduler::precede(mfnrTunDsVbiTask, mfnrDsVbiTask);
			scheduler->succeedPrevTaskByStep(DsVbiTunGroup, 0, mfnrTunDsVbiTask);
			scheduler->queueTask(mfnrTunDsVbiTask, DsVbiTunGroup);

			Scheduler::precede(mfnrMcdsF1Task, mfnrDsVbiTask);
			scheduler->succeedPrevTaskByStep(DsVbiGroup, 0, mfnrDsVbiTask);
			scheduler->queueTask(mfnrDsVbiTask, DsVbiGroup);

			Scheduler::precede(mfnrTunBssTask, mfnrTunMsbldTask1st);
			Scheduler::precede(mfnrTunDsVbiTask, mfnrTunMsbldTask1st);
			Scheduler::precede(mfnrTunMsbldTask1st, mfnrMsbldTask1st);
			scheduler->succeedPrevTaskByStep(MsbldTun1stGroup, 0, mfnrTunMsbldTask1st);
			scheduler->queueTask(mfnrTunMsbldTask1st, MsbldTun1stGroup);

			Scheduler::precede(mfnrDsVbiTask, mfnrMsbldTask1st);
			Scheduler::precede(mfnrDsTask, mfnrMsbldTask1st);
			Scheduler::precede(mfnrMcdsF1Task, mfnrMsbldTask1st);
			scheduler->succeedPrevTaskByStep(Msbld1stGroup, 0, mfnrMsbldTask1st);
			scheduler->queueTask(mfnrMsbldTask1st, Msbld1stGroup);

			Scheduler::precede(mfnrTunMsbldTask1st, mfnrTunMsbldTask2nd);
			Scheduler::precede(mfnrTunBssTask, mfnrTunMsbldTask2nd);
			Scheduler::precede(mfnrTunDsVbiTask, mfnrTunMsbldTask2nd);
			Scheduler::precede(mfnrTunMsbldTask2nd, mfnrMsbldTask2nd);
			scheduler->succeedPrevTaskByStep(MsbldTun2ndGroup, 0, mfnrTunMsbldTask2nd);
			scheduler->queueTask(mfnrTunMsbldTask2nd, MsbldTun2ndGroup);

			Scheduler::precede(mfnrMsbldTask1st, mfnrMsbldTask2nd);
			Scheduler::precede(mfnrDsVbiTask, mfnrMsbldTask2nd);
			Scheduler::precede(mfnrDsTask, mfnrMsbldTask2nd);
			Scheduler::precede(mfnrMcdsF1Task, mfnrMsbldTask2nd);
			scheduler->succeedPrevTaskByStep(Msbld2ndGroup, 0, mfnrMsbldTask2nd);
			scheduler->queueTask(mfnrMsbldTask2nd, Msbld2ndGroup);

			Scheduler::precede(mfnrTunMsbldTask2nd, mfnrTunAfbldTask);
			Scheduler::precede(mfnrBfbldTask, mfnrTunAfbldTask);
			Scheduler::precede(mfnrTunAfbldTask, mfnrAfbldTask);
			scheduler->succeedPrevTaskByStep(AfbldTunGroup, 0, mfnrTunAfbldTask);
			scheduler->queueTask(mfnrTunAfbldTask, AfbldTunGroup);

			Scheduler::precede(mfnrMsbldTask2nd, mfnrAfbldTask);
			scheduler->succeedPrevTaskByStep(AfbldGroup, 0, mfnrAfbldTask);
			scheduler->queueTask(mfnrAfbldTask, AfbldGroup);

			Scheduler::precede(mfnrAfbldTask, completeTask);
		} else {
			LPNRFrames lpnr;
			lpnrManager.makeLPNRFrames(lpnr, captureFrames.raw, still1Buffer, still2Buffer);

			auto [lpnrTunXtrTask, lpnrTunDipTask] = lpnrTunManager.makeLpnrTunTasks(
				lpnr, aaaIspExchange, camSysMetaRequestId, scheduler, "LpnrTun " + sequence, request, internalRequestId);

			auto [taskXtr, taskLpnrDip] = lpnrManager.makeLpnrTasks(
				lpnr, scheduler, "Lpnr " + sequence, request,
				internalRequestId, imgSysDev_);

			if (hasVideo) {
				Scheduler::precede(taskTr, taskXtr);
				Scheduler::precede(taskDip2, taskLpnrDip);
			}

			// Limit the interval from a producer task of tuning buffers
			// to its corresponding consumer task as 2.
			scheduler->succeedPrevTaskByStep(XtrGroup, 2, lpnrTunXtrTask);
			scheduler->succeedPrevTaskByStep(LpnrDipGroup, 2, lpnrTunDipTask);

			scheduler->succeedPrevTaskByStep(LpnrDipGroup, 4, taskXtr);

			scheduler->succeedPrevTaskByStep(
				AAAGroup, CaptureTasksManager::kRawMetaDelay,
				lpnrTunXtrTask);

			Scheduler::precede(lpnrTunXtrTask, taskXtr);
			scheduler->succeedPrevTaskByStep(LpnrTunXtrTaskGroup, 0, lpnrTunXtrTask);
			scheduler->queueTask(lpnrTunXtrTask, LpnrTunXtrTaskGroup);

			Scheduler::precede(taskXtr, lpnrTunDipTask);
			Scheduler::precede(lpnrTunDipTask, taskLpnrDip);
			scheduler->succeedPrevTaskByStep(LpnrTunDipTaskGroup, 0, lpnrTunDipTask);
			scheduler->queueTask(lpnrTunDipTask, LpnrTunDipTaskGroup);

			Scheduler::precede(taskDQBuf, taskXtr);
			scheduler->succeedPrevTaskByStep(XtrGroup, 0, taskXtr);
			scheduler->succeedPrevTaskByStep(LpnrTunDipTaskGroup, 2, taskXtr);
			scheduler->queueTask(taskXtr, XtrGroup);

			Scheduler::precede(taskXtr, taskLpnrDip);
			scheduler->succeedPrevTaskByStep(LpnrDipGroup, 0, taskLpnrDip);
			scheduler->queueTask(taskLpnrDip, LpnrDipGroup);

			Scheduler::precede(taskLpnrDip, completeTask);
		}
	}

	scheduler->succeedPrevTaskByStep(CompleteGroup, 0, completeTask);
	scheduler->queueTask(completeTask, CompleteGroup);

	scheduler->schedule();
	return 0;
}

bool MtkISP7CameraData::is3aControlChanged(std::shared_ptr<ControlList> controls_cur, std::shared_ptr<ControlList> controls_cache)
{
	if (!(controls_cache.get())) {
		return true;
	}
	std::vector<int32_t> checkList{
		controls::draft::AE_MODE,
		controls::AE_LOCKED,
		controls::EXPOSURE_TIME,
		controls::ANALOGUE_GAIN,
		controls::draft::AE_PRECAPTURE_TRIGGER,
		controls::AWB_MODE,
		controls::AWB_ENABLE,
		controls::AWB_LOCKED,
		controls::draft::AE_ANTI_BANDING_MODE,
		controls::FRAME_DURATION,
		controls::FRAME_DURATION_LIMITS,
		controls::AF_MODE,
		controls::AF_TRIGGER,
		controls::AF_WINDOWS,
		controls::LENS_POSITION,
		controls::draft::LENS_FOCUS_DISTANCE,
		controls::draft::COLOR_CORRECTION_MODE,
		controls::draft::COLOR_CORRECTION_GAINS,
		controls::COLOUR_CORRECTION_MATRIX,
	};

	for (auto id : checkList) {
		if (controls_cur->contains(id) && controls_cache->contains(id)) {
			auto value = controls_cur->get(id);
			auto value_cache = controls_cache->get(id);
			if (value != value_cache) {
				LOG(MtkISP7, Debug) << "id:" << controls::controls.at(id) << " value changed!! ";
				return true;
			}
		} else if (controls_cur->contains(id) || controls_cache->contains(id)) {
			LOG(MtkISP7, Debug) << "id:" << controls::controls.at(id) << " value changed!! ";
			return true;
		}
	}

	return false;
}

REGISTER_PIPELINE_HANDLER(PipelineHandlerMtkISP7)

} /* namespace libcamera */

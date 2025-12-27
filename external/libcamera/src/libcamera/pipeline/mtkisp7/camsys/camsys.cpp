/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Google Inc.
 *
 * camsys.cpp - MTK MtkISP7 Camsys device
 */

#include "camsys.h"

#include <libcamera/formats.h>
#include <libcamera/framebuffer.h>
#include <libcamera/geometry.h>

#include "libcamera/internal/camera_sensor_properties.h"
#include "libcamera/internal/media_device.h"
#include "libcamera/internal/request.h"

#include "kernel-headers/imgsensor-user.h"
#include "linux/v4l2-controls.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

namespace {

constexpr unsigned int PAD_SENSOR_OUT = 0;
constexpr unsigned int PAD_SENINF_OUT = 1;
constexpr unsigned int PAD_SENINF_IN = 0;
constexpr unsigned int PAD_RAW_IN = 0;
constexpr unsigned int PAD_RAWI2_IN = 2;
constexpr unsigned int PAD_MAIN = 5;
constexpr unsigned int PAD_YUV1 = 6;
constexpr unsigned int PAD_YUV2 = 7;
constexpr unsigned int PAD_DRZS4NO3 = 13;
constexpr unsigned int PAD_RZH1N2TO1 = 14;

constexpr Size kMeSize = Size{ 576, 432 };
constexpr Size kFdSize = Size{ 640, 480 };

const std::string kRawPrefix = "mtk-cam raw-";
const std::string kSeninfPrefix = "seninf-";

constexpr unsigned int kRequestCount = 24;

} /* namespace */

CamSysDevice::CamSysDevice(OnDeviceTuner *odt)
	: onDeviceTuner_(odt)
{
}

int CamSysDevice::init(MediaDevice *media, unsigned int index)
{
	index_ = index;
	media_ = media;

	MediaEntity *videoHubEntity =
		media_->getEntityByName(kRawPrefix + std::to_string(index_));

	MediaEntity *seninfEntity =
		media_->getEntityByName(kSeninfPrefix + std::to_string(index_));

	if (initSensor(seninfEntity)) {
		LOG(MtkISP7, Info) << "No sensor attached to CamSys " << index;
		return -ENODEV;
	}

	seninf_ = std::make_unique<V4L2Subdevice>(seninfEntity);
	videoHub_ = std::make_unique<V4L2Subdevice>(videoHubEntity);

	if (videoHub_->open()) {
		LOG(MtkISP7, Error) << "Fail to open "
				    << videoHub_->entity()->id();
		close();
		return -ENODEV;
	}

	if (seninf_->open()) {
		LOG(MtkISP7, Error) << "Fail to open "
				    << seninf_->entity()->id();
		close();
		return -ENODEV;
	}

	/* Helper function to create video nodes and collect them into
	 * allVideoDevices_ for easier StreamOn/Off. */
	auto getVideoDevice = [this](const std::string &name,
				     std::unique_ptr<V4L2VideoDevice> &videoDevice) {
		MediaEntity *entity = media_->getEntityByName(name);
		if (!entity) {
			LOG(MtkISP7, Error) << "Could not find video device " << name;
			return;
		}

		videoDevice = std::make_unique<V4L2VideoDevice>(entity);

		allVideoDevices_.emplace_back(videoDevice.get());
	};

	const std::string &hubName = videoHubEntity->name();
	getVideoDevice(hubName + " meta-input", metaInput_);
	getVideoDevice(hubName + " rawi-2", rawi2_);
	getVideoDevice(hubName + " main-stream", mainStream_);
	getVideoDevice(hubName + " yuvo-1", yuvo1_);
	getVideoDevice(hubName + " yuvo-2", yuvo2_);
	getVideoDevice(hubName + " yuvo-3", yuvo3_);
	getVideoDevice(hubName + " yuvo-4", yuvo4_);
	getVideoDevice(hubName + " yuvo-5", yuvo5_);
	getVideoDevice(hubName + " drzs4no-1", drzs4no1_);
	getVideoDevice(hubName + " drzs4no-2", drzs4no2_);
	getVideoDevice(hubName + " drzs4no-3", drzs4no3_);
	getVideoDevice(hubName + " rzh1n2to-1", rzh1n2to1_);
	getVideoDevice(hubName + " rzh1n2to-2", rzh1n2to2_);
	getVideoDevice(hubName + " rzh1n2to-3", rzh1n2to3_);
	getVideoDevice(hubName + " partial-meta-0", partialMeta0_);
	getVideoDevice(hubName + " partial-meta-1", partialMeta1_);
	getVideoDevice(hubName + " partial-meta-2", partialMeta2_);

	for (V4L2VideoDevice *device : allVideoDevices_) {
		int ret = device->open();
		// todo: Fix rawi2 failing to enum formats properly from driver.
		// For now, ignore the open error for rawi2.
		if (ret && device != rawi2_.get()) {
			LOG(MtkISP7, Error) << "Fail to open "
					    << device->devicePath();
			close();
			return ret;
		}
	}

	std::vector<UniqueFD> requests;
	media_->allocateRequests(kRequestCount, requests);
	mediaRequestPool_.setData(requests);

	yuvo1_->requestBufferReady.connect(this, &CamSysDevice::bufferReady);
	yuvo2_->requestBufferReady.connect(this, &CamSysDevice::bufferReady);
	drzs4no3_->requestBufferReady.connect(this, &CamSysDevice::bufferReady);
	rzh1n2to1_->requestBufferReady.connect(this, &CamSysDevice::bufferReady);
	metaInput_->requestBufferReady.connect(this, &CamSysDevice::bufferReady);
	mainStream_->requestBufferReady.connect(this, &CamSysDevice::bufferReady);
	partialMeta0_->requestBufferReady.connect(this, &CamSysDevice::bufferReady);
	partialMeta1_->requestBufferReady.connect(this, &CamSysDevice::bufferReady);
	rawi2_->requestBufferReady.connect(this, &CamSysDevice::bufferReady);

	return 0;
}

int CamSysDevice::start()
{
	int ret = setTestPattern(controls::draft::TestPatternModeOff);
	if (ret)
		LOG(MtkISP7, Warning) << "Fail to reset test pattern";

	for (V4L2VideoDevice *device : allVideoDevices_) {
		if (device == mainStream_.get())
			/*
			 * A rough approximate number of raw buffers to support
			 * ZSL, as we cache 4 more raw buffers.
			 */
			ret |= device->importBuffers(8);
		else
			ret |= device->importBuffers(16);
	}

	if (ret) {
		LOG(MtkISP7, Error) << "Fail to import buffers";
		return ret;
	}

	for (V4L2VideoDevice *device : allVideoDevices_) {
		ret = device->streamOn();
		if (ret) {
			LOG(MtkISP7, Error) << "Fail to streamOn "
					    << device->devicePath();
			return ret;
		}
	}

	ret = videoHub_->setFrameStartEnabled(true);
	if (ret) {
		LOG(MtkISP7, Error) << "Fatal due to cannot enable frame start "
				    << strerror(-ret);
		return ret;
	}

	if (sensor_->focusLens())
		sensor_->focusLens()->open();

	ASSERT(pendingRequests_.empty());
	ASSERT(completedRequests_.empty());

	return 0;
}

int CamSysDevice::stop()
{
	int ret;
	for (V4L2VideoDevice *device : allVideoDevices_) {
		ret = device->streamOff();
		if (ret) {
			LOG(MtkISP7, Error) << "Fail to streamOff "
					    << device->devicePath();
			return ret;
		}
	}

	for (V4L2VideoDevice *device : allVideoDevices_) {
		ret = device->releaseBuffers();
		if (ret) {
			LOG(MtkISP7, Error) << "Fail to release buffers "
					    << device->devicePath();
			return ret;
		}
	}

	ret = videoHub_->setFrameStartEnabled(false);
	if (ret)
		LOG(MtkISP7, Error) << "Fatal due to cannot disable frame start " << strerror(-ret);

	if (sensor_->focusLens())
		sensor_->focusLens()->close();

	ASSERT(pendingRequests_.empty());
	ASSERT(completedRequests_.empty());

	return ret;
}

void CamSysDevice::close()
{
	videoHub_.reset();
	seninf_.reset();
	sensor_.reset();

	metaInput_.reset();
	rawi2_.reset();

	mainStream_.reset();
	yuvo1_.reset();
	yuvo2_.reset();
	yuvo3_.reset();
	yuvo4_.reset();
	yuvo5_.reset();

	drzs4no1_.reset();
	drzs4no2_.reset();
	drzs4no3_.reset();
	rzh1n2to1_.reset();
	rzh1n2to2_.reset();
	rzh1n2to3_.reset();

	partialMeta0_.reset();
	partialMeta1_.reset();
	partialMeta2_.reset();

	allVideoDevices_.clear();
}

int CamSysDevice::releaseAllBuffers()
{
	int ret = 0;
	for (V4L2VideoDevice *device : allVideoDevices_)
		ret |= device->releaseBuffers();

	return ret;
}

int CamSysDevice::configure(const Size &rawFrameSize, const Size &yuvFrameSize)
{
	rawFrameSize_ = rawFrameSize;
	yuvFrameSize_ = yuvFrameSize;

	setupLinks(false);

	int ret = configureSensor();
	ret |= configureMtkCamRaw();

	if (ret)
		LOG(MtkISP7, Error) << "Fail to configure CamSys";

	return ret;
}

int CamSysDevice::queueRequest(Request *request)
{
	int mediaRequest = mediaRequestPool_.get();
	request->mediaRequest = mediaRequest;

	int ret = metaInput_->queueBuffer(request->tuning, mediaRequest);
	ret |= partialMeta0_->queueBuffer(request->statistics0, mediaRequest);
	ret |= partialMeta1_->queueBuffer(request->statistics1, mediaRequest);
	ret |= rzh1n2to1_->queueBuffer(request->faceDetect, mediaRequest);

	unsigned int queuedBuffers = 4;
	if (request->main) {
		ret |= mainStream_->queueBuffer(request->main, mediaRequest);
		++queuedBuffers;
	}
	if (request->yuvo1) {
		ret |= yuvo1_->queueBuffer(request->yuvo1, mediaRequest);
		++queuedBuffers;
	}
	if (request->yuvo2) {
		ret |= yuvo2_->queueBuffer(request->yuvo2, mediaRequest);
		++queuedBuffers;
	}
	if (request->me) {
		ret |= drzs4no3_->queueBuffer(request->me, mediaRequest);
		++queuedBuffers;
	}

	if (onDeviceTuner_->isCamsysDebugFrameEnabled()) {
		++queuedBuffers;
		ret |= rawi2_->queueBuffer(request->rawInject, mediaRequest);
	}

	ret |= media_->queueRequest(mediaRequest);
	if (ret) {
		LOG(MtkISP7, Error) << "Fail to queue request";
		return ret;
	}

	pendingRequests_.emplace_back(
		PendingRequest{ request, mediaRequest, queuedBuffers });

	return 0;
}

int CamSysDevice::claimCompletedRequest(Request *request)
{
	for (auto iter = completedRequests_.begin();
	     iter != completedRequests_.end(); ++iter) {
		if (*iter == request) {
			completedRequests_.erase(iter);
			return 0;
		}
	}
	return -EINVAL;
}

int CamSysDevice::setVBlank(uint32_t vblank)
{
	ControlList ctrl(sensor_->controls());
	ctrl.set(V4L2_CID_VBLANK, (int32_t)vblank);

	return sensor_->device()->setControls(&ctrl);
}

int CamSysDevice::setExposureGain(uint32_t exposure, uint32_t gain)
{
	ControlList ctrl(sensor_->controls());
	ctrl.set(V4L2_CID_EXPOSURE, (int32_t)exposure);
	ctrl.set(V4L2_CID_ANALOGUE_GAIN, (int32_t)gain);

	return sensor_->device()->setControls(&ctrl);
}

int CamSysDevice::setTestPattern(controls::draft::TestPatternModeEnum mode)
{
	return sensor_->setTestPatternMode(mode);
}

int CamSysDevice::setupResource()
{
	V4L2SubdeviceFormat format = {};

	format.mbus_code = mbusCode_;
	format.size = rawFrameSize_;

	int ret = videoHub_->setFormat(PAD_RAW_IN, &format);
	if (ret) {
		LOG(MtkISP7, Error) << "Fail to set format for " << videoHub_->entity();
		return -EINVAL;
	}

	struct mtk_cam_resource camsysResource;

	struct v4l2_mbus_framefmt framefmt;
	framefmt.width = format.size.width;
	framefmt.height = format.size.height;
	camsysResource.sink_fmt = (__u64)&framefmt;

	auto &sensorResource = camsysResource.sensor_res;

	IPACameraSensorInfo sensorInfo;
	sensor_->sensorInfo(&sensorInfo);
	auto ctrls = sensor_->getControls({ V4L2_CID_HBLANK, V4L2_CID_VBLANK });

	const ControlInfo hblank = ctrls.infoMap()->at(V4L2_CID_HBLANK);
	const ControlInfo vblank = ctrls.infoMap()->at(V4L2_CID_VBLANK);

	sensorResource.interval = { 1, 30 };
	sensorResource.hblank = hblank.min().get<int32_t>();
	sensorResource.vblank = vblank.max().get<int32_t>();
	sensorResource.pixel_rate = sensorInfo.pixelRate;
	sensorResource.cust_pixel_rate = sensorInfo.pixelRate;

	auto &rawResource(camsysResource.raw_res);
	rawResource.feature = 0;
	rawResource.strategy = 3;
	rawResource.raw_max = (uint8_t)MTK_CAM_RESOURCE_DEFAULT;
	rawResource.raw_min = (uint8_t)MTK_CAM_RESOURCE_DEFAULT;
	rawResource.raw_used = 0;
	rawResource.bin = 0;
	rawResource.path_sel = (uint8_t)MTK_CAM_RESOURCE_DEFAULT;
	rawResource.pixel_mode = 0;
	rawResource.throughput = 0;

	struct v4l2_ext_control ext_ctrl{
		.id = V4L2_CID_MTK_CAM_RAW_RESOURCE_CALC,
		.size = sizeof(camsysResource),
		.reserved2 = {},
		.ptr = &camsysResource
	};

	ret = videoHub_->setExtControl(&ext_ctrl);
	if (ret) {
		LOG(MtkISP7, Error) << "Fail to calculate resources for "
				    << videoHub_->entity()->name();
		return ret;
	}

	return ret;
}

int CamSysDevice::setFormat(V4L2Subdevice *device, int pad,
			    uint32_t mbus_code, Size size)
{
	V4L2SubdeviceFormat format = { mbus_code, size, {} };
	int ret = device->setFormat(pad, &format);
	if (ret)
		LOG(MtkISP7, Error) << "Fail to set format to "
				    << device->entity()->id()
				    << " pad " << pad << " format " << format;

	return ret;
}

int CamSysDevice::setFormat(V4L2VideoDevice *device, const PixelFormat &format,
			    Size size)
{
	const PixelFormatInfo &info = PixelFormatInfo::info(format);

	V4L2DeviceFormat outputFormat = {
		.fourcc = device->toV4L2PixelFormat(format),
		.size = size,
		.colorSpace = std::nullopt,
		.planes = { { { 0, info.stride(size.width, 0) } } },
		.planesCount = 1,
	};

	return device->setFormat(&outputFormat);
}

int CamSysDevice::initSensor(MediaEntity *seninfEntity)
{
	const std::vector<MediaPad *> &pads = seninfEntity->pads();
	if (pads.empty())
		return -ENODEV;

	/* seninf have a single sink pad from sensor at index 0. */
	MediaPad *sink = pads[0];

	const std::vector<MediaLink *> &links = sink->links();
	if (links.empty())
		return -ENODEV;

	MediaLink *link = links[0];
	MediaEntity *entity = link->source()->entity();

	auto sensor = std::make_unique<CameraSensor>(entity);
	if (sensor->init())
		return -ENODEV;

	/* Supported mbus codes */
	static std::map<unsigned int, PixelFormat> mbusCodes = {
		{ MEDIA_BUS_FMT_SBGGR10_1X10, formats::SBGGR10_MTISP },
		{ MEDIA_BUS_FMT_SGBRG10_1X10, formats::SGBRG10_MTISP },
		{ MEDIA_BUS_FMT_SGRBG10_1X10, formats::SGRBG10_MTISP },
		{ MEDIA_BUS_FMT_SRGGB10_1X10, formats::SRGGB10_MTISP },
	};

	/* Find one mbus supported by both the sensor and Camsys */
	mbusCode_ = 0;
	for (auto code : sensor->mbusCodes()) {
		const auto &iter = mbusCodes.find(code);
		if (iter != mbusCodes.end()) {
			mbusCode_ = code;
			bayerFormat_ = iter->second;
		}
	}
	if (!mbusCode_)
		return -ENODEV;

	sensor_ = std::move(sensor);

	// Close the lens by default to save power.
	if (sensor_->focusLens())
		sensor_->focusLens()->close();

	return 0;
}

int CamSysDevice::configureSensor()
{
	V4L2SubdeviceFormat format;
	format.mbus_code = mbusCode_;
	format.size = rawFrameSize_;

	if (sensor_->device()->setFormat(PAD_SENSOR_OUT, &format)) {
		LOG(MtkISP7, Error) << "Fail to set format to sensor: "
				    << sensor_->entity()->name();
		return -EINVAL;
	}

	if (seninf_->setFormat(PAD_SENINF_IN, &format)) {
		LOG(MtkISP7, Error) << "Fail to set format to seninf in: "
				    << seninf_->entity()->name();
		return -EINVAL;
	}

	if (seninf_->setFormat(PAD_SENINF_OUT, &format)) {
		LOG(MtkISP7, Error) << "Fail to set format to seninf out: "
				    << seninf_->entity()->name();
		return -EINVAL;
	}

	return 0;
}

int CamSysDevice::configureMtkCamRaw()
{
	int ret = setupLinks(true);
	if (ret) {
		LOG(MtkISP7, Error) << "Fail to setup links";
		return ret;
	}

	ret = setupResource();
	if (ret) {
		LOG(MtkISP7, Error) << "Fail to calculate resources";
		return ret;
	}

	Size halfYuvSize = yuvFrameSize_ / 2;

	ret = configureVideo(mainStream_.get(), bayerFormat_, rawFrameSize_,
			     videoHub_.get(), PAD_MAIN,
			     mbusCode_);
	ret |= configureVideo(yuvo1_.get(), formats::NV12_10P_MTISP, yuvFrameSize_,
			      videoHub_.get(), PAD_YUV1,
			      MEDIA_BUS_FMT_SRGGB10_1X10);
	ret |= configureVideo(yuvo2_.get(), formats::NV12_12P_MTISP, halfYuvSize,
			      videoHub_.get(), PAD_YUV2,
			      MEDIA_BUS_FMT_SRGGB10_1X10);
	ret |= configureVideo(drzs4no3_.get(), formats::GREY, kMeSize,
			      videoHub_.get(), PAD_DRZS4NO3,
			      MEDIA_BUS_FMT_SRGGB10_1X10);
	ret |= configureVideo(rzh1n2to1_.get(), formats::NV12, kFdSize,
			      videoHub_.get(), PAD_RZH1N2TO1,
			      MEDIA_BUS_FMT_SRGGB10_1X10);

	if (onDeviceTuner_->isCamsysDebugFrameEnabled()) {
		ret |= configureVideo(rawi2_.get(), bayerFormat_, rawFrameSize_,
				      videoHub_.get(), PAD_RAWI2_IN, mbusCode_);
	}

	if (ret) {
		LOG(MtkISP7, Error) << "Fail to configure video nodes";
		return ret;
	}

	Rectangle fullRect(rawFrameSize_);

	ret |= yuvo1_->setSelection(V4L2_SEL_TGT_CROP, &fullRect);
	ret |= yuvo2_->setSelection(V4L2_SEL_TGT_CROP, &fullRect);
	ret |= drzs4no3_->setSelection(V4L2_SEL_TGT_CROP, &fullRect);
	ret |= rzh1n2to1_->setSelection(V4L2_SEL_TGT_CROP, &fullRect);

	if (ret) {
		LOG(MtkISP7, Error) << "Fail to set selection for video nodes";
		return ret;
	}

	return ret;
}

int CamSysDevice::configureVideo(V4L2VideoDevice *device, const PixelFormat &format,
				 Size resolution, V4L2Subdevice *rawPipe,
				 int pad, uint32_t mbus)
{
	int ret = setFormat(rawPipe, pad, mbus, resolution);
	ret |= setFormat(device, format, resolution);
	return ret;
}

int CamSysDevice::setupLinks(bool enable)
{
	MediaLink *link = nullptr;

	link = media_->link(sensor_->entity(), PAD_SENSOR_OUT,
			    seninf_->entity(), PAD_SENINF_IN);
	int ret = link->setEnabled(enable);

	link = media_->link(seninf_->entity(), PAD_SENINF_OUT,
			    videoHub_->entity(), PAD_RAW_IN);
	if (onDeviceTuner_->isCamsysDebugFrameEnabled()) {
		ret |= link->setEnabled(false);
	} else {
		ret |= link->setEnabled(enable);
	}

	return ret;
}

void CamSysDevice::bufferReady(std::pair<FrameBuffer *, int> bufferWithRequest)
{
	auto [buffer, mediaRequest] = bufferWithRequest;
	bool foundRequest = false;
	for (auto iter = pendingRequests_.begin();
	     iter != pendingRequests_.end(); iter++) {
		Request *request = iter->request;
		if (request->mediaRequest != mediaRequest)
			continue;

		foundRequest = true;
		if (--iter->pending > 0)
			break;

		media_->reInitRequest(iter->mediaRequest);
		mediaRequestPool_.put(iter->mediaRequest);

		completedRequests_.emplace_back(request);
		requestCompleted.emit(request);

		pendingRequests_.erase(iter);
		break;
	}

	ASSERT(foundRequest == true);
}

} /* namespace libcamera */

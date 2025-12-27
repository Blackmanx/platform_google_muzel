/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Google Inc.
 *
 * aie.h - MtkISP7 AI Engine device
 */

#include "aie.h"

#include <sys/syscall.h>

#include <libcamera/formats.h>

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

namespace {

constexpr Size kDefaultInputSize{ 640, 480 };
constexpr const char *kInitControlName = "FD detection init";
constexpr const char *kInferenceParamControlName = "FD detection param";

} // namespace

AieDevice::AieDevice()
	: inputSize_(kDefaultInputSize), bufferNum_(8), sourceVideo_(nullptr),
	  resultMeta_(nullptr), media_(nullptr),
	  driverInitConfig_{
		  .src_max_width = inputSize_.width,
		  .src_max_height = inputSize_.width * 3 / 2,
		  .src_pyramid_width = 640,
		  .src_pyramid_height = 640,
		  .feature_threshold = unsigned(-64)
	  }
{
}

int AieDevice::configure()
{
	V4L2DeviceFormat format;
	format.fourcc = sourceVideo_->toV4L2PixelFormat(formats::NV12);
	format.size = inputSize_;
	unsigned int stride = 8;
	format.planes = { { { 0, stride } } };
	format.planesCount = 1;
	int ret = sourceVideo_->setFormatVideo(&format);
	if (ret) {
		LOG(MtkISP7, Error) << "Error when setting image format: "
				    << ret;
		return ret;
	}
	v4l2_ext_control extControl{
		.id = initControlId_,
		.size = sizeof(FdDrv_init_struct),
		.reserved2 = {},
		.p_u32 = reinterpret_cast<__u32 *>(&driverInitConfig_),
	};

	ret = sourceVideo_->setExtControl(&extControl);

	if (ret) {
		LOG(MtkISP7, Error) << "Error when setting init controls: "
				    << ret;
		return ret;
	}
	return ret;
}

FdDrv_input_struct AieDevice::createDefaultDriverConfig()
{
	return {
		.fd_mode = 0,
		.src_img_fmt = NSCam::NSIoPipe::FMT_YUV420_1P,
		.src_img_width = inputSize_.width,
		.src_img_height = inputSize_.height,
		.src_img_stride = inputSize_.width,
		.pyramid_base_width = 0,
		.pyramid_base_height = 0,
		.number_of_pyramid = 3,
		.input_rotate_degree = 0,
		.en_roi = 0,
		.src_roi = { 0, 0, 0, 0 },
		.en_padding = 0,
		.src_padding = { 0, 0, 0, 0 },
		.freq_level = 0,
		.fld_face_num = 0,
		.fld_input = {}
	};
}

FdDrv_input_struct AieDevice::createFaceDetectionDriverConfig()
{
	FdDrv_input_struct config(createDefaultDriverConfig());
	config.fd_mode = NSCam::NSIoPipe::FDMODE;
	config.pyramid_base_width = 480;
	config.pyramid_base_height = 360;
	config.en_roi = false;
	config.en_padding = false;
	return config;
}

FdDrv_input_struct AieDevice::createFaceToneClassificationDriverConfig()
{
	FdDrv_input_struct config(createDefaultDriverConfig());
	config.fd_mode = NSCam::NSIoPipe::ATTRIBUTEMODE;
	config.pyramid_base_width = 0;
	config.pyramid_base_height = 0;
	config.en_roi = true;
	config.en_padding = true;
	return config;
}

int AieDevice::init(MediaDevice *media)
{
	media_ = media;

	auto mediaEntity = media_->getEntityByName("mtk-aie-5.3-source");
	std::string deviceNode = mediaEntity->deviceNode();

	sourceVideo_ = std::make_unique<V4L2VideoDevice>(deviceNode);
	resultMeta_ = std::make_unique<V4L2VideoDevice>(deviceNode);

	SharedFD fd(syscall(SYS_openat, AT_FDCWD, deviceNode.c_str(),
			    O_RDWR | O_NONBLOCK));

	if (!fd.isValid()) {
		LOG(MtkISP7, Error) << "Error opening video device node!";
		return -errno;
	}

	int ret = sourceVideo_->open(fd, V4L2_BUF_TYPE_VIDEO_OUTPUT);

	if (ret) {
		LOG(MtkISP7, Error) << "Error opening video device: source";
		return ret;
	}

	bool foundInitCtrlId = false;
	bool foundInferenceParamCtrlId = false;
	for (const auto &[id, info] : sourceVideo_->controls()) {
		if (id->name().find(kInitControlName) != std::string::npos) {
			LOG(MtkISP7, Debug) << "Found init control ID: "
					    << id->id();
			initControlId_ = id->id();
			foundInitCtrlId = true;
		} else if (id->name().find(kInferenceParamControlName) !=
			   std::string::npos) {
			LOG(MtkISP7, Debug) << "Found inference param control ID: "
					    << id->id();
			inferenceParamControlId_ = id->id();
			foundInferenceParamCtrlId = true;
		}
	}
	if (!foundInitCtrlId || !foundInferenceParamCtrlId) {
		LOG(MtkISP7, Error) << "Failed to find AIE control: "
				    << "Found init control: "
				    << (foundInitCtrlId ? "yes" : "no")
				    << ". Found inference param control: "
				    << (foundInferenceParamCtrlId ? "yes" : "no");
		return -ENODEV;
	}

	ret = resultMeta_->open(fd, V4L2_BUF_TYPE_META_CAPTURE);

	if (ret) {
		LOG(MtkISP7, Error) << "error opening video device: meta";
		return ret;
	}

	LOG(MtkISP7, Debug) << "AIE Device init success!";

	return 0;
}

int AieDevice::releaseBuffers()
{
	int retSrcVideo = sourceVideo_->releaseBuffers();
	if (retSrcVideo != 0) {
		LOG(MtkISP7, Error) << "AIE device releaseBuffers failed: "
				    << "faceDetectionSourceVideo_";
	}
	// Do not return early, try to release the resultMeta_ buffers.

	int retResultMeta = resultMeta_->releaseBuffers();
	if (retResultMeta != 0) {
		LOG(MtkISP7, Error) << "AIE device releaseBuffers failed: "
				    << "faceDetectionResultMeta_";
	}

	return std::min(retSrcVideo, retResultMeta);
}

int AieDevice::requestBuffers()
{
	int ret = sourceVideo_->importBuffers(bufferNum_);
	if (ret < 0) {
		LOG(MtkISP7, Error) << "AIE device failed to requestBuffers: "
				    << "failed to prepare device to "
				    << "import DMA buffers";
		return ret;
	}

	V4L2DeviceFormat metaFormat;
	resultMeta_->getFormat(&metaFormat);
	LOG(MtkISP7, Debug) << "sizeof(FdDrv_output_struct): "
			    << sizeof(FdDrv_output_struct);
	if (metaFormat.planes[0].size != sizeof(FdDrv_output_struct)) {
		LOG(MtkISP7, Warning)
			<< "metaFormat.planes[0].size = "
			<< metaFormat.planes[0].size
			<< "while sizeof(FdDrv_output_struct) = "
			<< sizeof(FdDrv_output_struct);
		// todo(MTK): structures from cam_fdvt_v4l2.h should be moved
		// to the driver as public header.
		// Discrepancy in these two sizes may be sign of discrepancy of
		// the struct definition used by the driver and struct
		// definition used here.
	}
	ret = resultMeta_->importBuffers(bufferNum_);
	if (ret < 0) {
		LOG(MtkISP7, Error) << "AIE device failed to requestBuffers: "
				    << "failed to allocate FD results buffers";
		if (sourceVideo_->releaseBuffers()) {
			LOG(MtkISP7, Error) << "Failed to handle errors "
					    << "in AIE device requestBuffers: "
					    << "failed to release prepared "
					    << "DMA buffers";
		}
	}
	if (ret > 0)
		return 0;
	return ret;
}

int AieDevice::start()
{
	static const std::string startFailed = "Failed to start AIE device: ";
	static const std::string errorHandlingFailed =
		"AIE device error handling failed: ";
	int ret = requestBuffers();
	if (ret) {
		LOG(MtkISP7, Error) << startFailed
				    << "requestBuffers: " << ret;
		return ret;
	}

	const auto cancelRequestBuffers = [&]() {
		if (releaseBuffers() != 0) {
			LOG(MtkISP7, Error) << errorHandlingFailed
					    << "releaseBuffers";
		}
	};

	ret = sourceVideo_->streamOn();
	if (ret) {
		LOG(MtkISP7, Error) << startFailed
				    << "sourceVideo_.streamOn: " << ret;
		cancelRequestBuffers();
		return ret;
	}
	const auto cancelSrcVideoStreamOn = [&]() {
		if (sourceVideo_->streamOff()) {
			LOG(MtkISP7, Error) << errorHandlingFailed
					    << "sourceVideo_ streamOff";
		}
	};

	ret = resultMeta_->streamOn();
	if (ret) {
		LOG(MtkISP7, Error) << startFailed
				    << "resultMeta_.streamOn: " << ret;
		cancelSrcVideoStreamOn();
		cancelRequestBuffers();
		return ret;
	}

	return ret;
}

int AieDevice::stop()
{
	int retSrcVideo = sourceVideo_->streamOff();
	if (retSrcVideo != 0) {
		LOG(MtkISP7, Error) << "AIE device got failure when stop: "
				    << "faceDetectionSourceVideo_ streamOff";
	}
	// Do not return early, try to attempt other cleanup.

	int retResultMeta = resultMeta_->streamOff();
	if (retResultMeta != 0) {
		LOG(MtkISP7, Error) << "AIE device got failure when stop: "
				    << "faceDetectionResultMeta_ streamOff";
	}

	int retReleaseBuff = releaseBuffers();
	if (retReleaseBuff != 0) {
		LOG(MtkISP7, Error) << "AIE device got failure when stop: "
				    << "releaseBuffers";
	}

	return std::min(std::min(retSrcVideo, retResultMeta), retReleaseBuff);
}

} /* namespace libcamera */

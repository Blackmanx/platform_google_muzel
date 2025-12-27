/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * camera_device.cpp - libcamera Android Camera Device
 */

#include "camera_device.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <sys/mman.h>
#include <unistd.h>
#include <unordered_set>
#include <vector>

#ifdef HAVE_ANDROID_OS
#include <cutils/properties.h>
#endif

#include <libcamera/base/log.h>
#include <libcamera/base/span.h>
#include <libcamera/base/unique_fd.h>
#include <libcamera/base/utils.h>

#include <libcamera/control_ids.h>
#include <libcamera/controls.h>
#include <libcamera/fence.h>
#include <libcamera/formats.h>
#include <libcamera/geometry.h>
#include <libcamera/property_ids.h>

#include <system/camera_metadata.h>

#include "camera_buffer.h"
#include "camera_capabilities.h"
#include "camera_hal_config.h"
#include "camera_ops.h"
#include "camera_request.h"
#include "hal_framebuffer.h"
#include "vendor_tag.h"

using namespace libcamera;

LOG_DECLARE_CATEGORY(HAL)

namespace {

const std::vector<uint32_t> kDuplicatedMetadata = { ANDROID_LENS_FOCUS_RANGE };

/*
 * \struct Camera3StreamConfig
 * \brief Data to store StreamConfiguration associated with camera3_stream(s)
 * \var streams List of the pairs of a stream requested by Android HAL client
 * and CameraStream::Type associated with the stream
 * \var config StreamConfiguration for streams
 */
struct Camera3StreamConfig {
	struct Camera3Stream {
		camera3_stream_t *stream;
		CameraStream::Type type;
	};

	std::vector<Camera3Stream> streams;
	StreamConfiguration config;
};

/*
 * Reorder the configurations so that libcamera::Camera can accept them as much
 * as possible. The sort rule is as follows.
 * 1.) The configuration for NV12 request whose resolution is the largest.
 * 2.) The configuration for JPEG request.
 * 3.) Others. Larger resolutions and different formats are put earlier.
 */
void sortCamera3StreamConfigs(std::vector<Camera3StreamConfig> &unsortedConfigs,
			      const camera3_stream_t *jpegStream)
{
	const Camera3StreamConfig *jpegConfig = nullptr;

	std::map<PixelFormat, std::vector<const Camera3StreamConfig *>> formatToConfigs;
	for (const auto &streamConfig : unsortedConfigs) {
		if (jpegStream && !jpegConfig) {
			const auto &streams = streamConfig.streams;
			if (std::find_if(streams.begin(), streams.end(),
					 [jpegStream](const auto &stream) {
						 return stream.stream == jpegStream;
					 }) != streams.end()) {
				jpegConfig = &streamConfig;
				continue;
			}
		}
		formatToConfigs[streamConfig.config.pixelFormat].push_back(&streamConfig);
	}

	if (jpegStream && !jpegConfig)
		LOG(HAL, Fatal) << "No Camera3StreamConfig is found for JPEG";

	for (auto &fmt : formatToConfigs) {
		auto &streamConfigs = fmt.second;

		/* Sorted by resolution. Smaller is put first. */
		std::sort(streamConfigs.begin(), streamConfigs.end(),
			  [](const auto *streamConfigA, const auto *streamConfigB) {
				  const Size &sizeA = streamConfigA->config.size;
				  const Size &sizeB = streamConfigB->config.size;
				  return sizeA < sizeB;
			  });
	}

	std::vector<Camera3StreamConfig> sortedConfigs;
	sortedConfigs.reserve(unsortedConfigs.size());

	/*
	 * NV12 is the most prioritized format. Put the configuration with NV12
	 * and the largest resolution first.
	 */
	const auto nv12It = formatToConfigs.find(formats::NV12);
	if (nv12It != formatToConfigs.end()) {
		auto &nv12Configs = nv12It->second;
		const Camera3StreamConfig *nv12Largest = nv12Configs.back();

		/*
		 * If JPEG will be created from NV12 and the size is larger than
		 * the largest NV12 configurations, then put the NV12
		 * configuration for JPEG first.
		 */
		if (jpegConfig && jpegConfig->config.pixelFormat == formats::NV12) {
			const Size &nv12SizeForJpeg = jpegConfig->config.size;
			const Size &nv12LargestSize = nv12Largest->config.size;

			if (nv12LargestSize < nv12SizeForJpeg) {
				LOG(HAL, Debug) << "Insert " << jpegConfig->config.toString();
				sortedConfigs.push_back(std::move(*jpegConfig));
				jpegConfig = nullptr;
			}
		}

		LOG(HAL, Debug) << "Insert " << nv12Largest->config.toString();
		sortedConfigs.push_back(*nv12Largest);
		nv12Configs.pop_back();

		if (nv12Configs.empty())
			formatToConfigs.erase(nv12It);
	}

	/* If the configuration for JPEG is there, then put it. */
	if (jpegConfig) {
		LOG(HAL, Debug) << "Insert " << jpegConfig->config.toString();
		sortedConfigs.push_back(std::move(*jpegConfig));
		jpegConfig = nullptr;
	}

	/*
	 * Put configurations with different formats and larger resolutions
	 * earlier.
	 */
	while (!formatToConfigs.empty()) {
		for (auto it = formatToConfigs.begin(); it != formatToConfigs.end();) {
			auto &configs = it->second;
			LOG(HAL, Debug) << "Insert " << configs.back()->config.toString();
			sortedConfigs.push_back(*configs.back());
			configs.pop_back();

			if (configs.empty())
				it = formatToConfigs.erase(it);
			else
				it++;
		}
	}

	ASSERT(sortedConfigs.size() == unsortedConfigs.size());

	unsortedConfigs = sortedConfigs;
}

const char *rotationToString(int rotation)
{
	switch (rotation) {
	case CAMERA3_STREAM_ROTATION_0:
		return "0";
	case CAMERA3_STREAM_ROTATION_90:
		return "90";
	case CAMERA3_STREAM_ROTATION_180:
		return "180";
	case CAMERA3_STREAM_ROTATION_270:
		return "270";
	}
	return "INVALID";
}

const char *directionToString(int stream_type)
{
	switch (stream_type) {
	case CAMERA3_STREAM_OUTPUT:
		return "Output";
	case CAMERA3_STREAM_INPUT:
		return "Input";
	case CAMERA3_STREAM_BIDIRECTIONAL:
		return "Bidirectional";
	default:
		LOG(HAL, Warning) << "Unknown stream type: " << stream_type;
		return "Unknown";
	}
}

bool isPreviewStream(camera3_stream_t *stream)
{
	return (GRALLOC_USAGE_HW_COMPOSER & stream->usage);
}

bool isVideoStream(camera3_stream_t *stream)
{
	return (GRALLOC_USAGE_HW_VIDEO_ENCODER & stream->usage);
}

// Vendor extended buffer usage bit for the HAL to identify the still capture YUV stream
#define IS_STILL_USAGE(usage) (((usage)&GRALLOC_USAGE_PRIVATE_1) == GRALLOC_USAGE_PRIVATE_1)
bool hasStillCaptureFlag(camera3_stream_t *stream)
{
	return (IS_STILL_USAGE(stream->usage));
}

bool isYuvSnapshotStream(camera3_stream_t *stream)
{
	return (!isVideoStream(stream) && !isPreviewStream(stream) &&
		(HAL_PIXEL_FORMAT_YCbCr_420_888 == stream->format));
}

bool isJpegStream(camera3_stream_t *stream)
{
	return (HAL_PIXEL_FORMAT_BLOB == stream->format);
}

[[maybe_unused]] int buildStreamConfigsDefault(const CameraCapabilities &capabilities,
					       camera3_stream_configuration_t *stream_list,
					       std::vector<Camera3StreamConfig> &streamConfigs)
{
	/* First handle all non-MJPEG streams. */
	camera3_stream_t *jpegStream = nullptr;
	for (unsigned int i = 0; i < stream_list->num_streams; ++i) {
		camera3_stream_t *stream = stream_list->streams[i];
		Size size(stream->width, stream->height);

		PixelFormat format = capabilities.toPixelFormat(stream->format);

		/* Defer handling of MJPEG streams until all others are known. */
		if (stream->format == HAL_PIXEL_FORMAT_BLOB) {
			if (jpegStream) {
				LOG(HAL, Error)
					<< "Multiple JPEG streams are not supported";
				return -EINVAL;
			}

			stream->usage |= (GRALLOC_USAGE_HW_CAMERA_WRITE |
					  GRALLOC_USAGE_SW_READ_OFTEN |
					  GRALLOC_USAGE_SW_WRITE_NEVER);

			jpegStream = stream;
			continue;
		}

		/*
		 * If a CameraStream with the same size and format as the
		 * current stream has already been requested, associate the two.
		 */
		auto iter = std::find_if(
			streamConfigs.begin(), streamConfigs.end(),
			[&size, &format](const Camera3StreamConfig &streamConfig) {
				return streamConfig.config.size == size &&
				       streamConfig.config.pixelFormat == format;
			});
		if (iter != streamConfigs.end()) {
			/* Add usage to copy the buffer in streams[0] to stream. */
			iter->streams[0].stream->usage |= (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN);
			stream->usage |= (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN);
			iter->streams.push_back({ stream, CameraStream::Type::Mapped });
			continue;
		}

		Camera3StreamConfig streamConfig;
		streamConfig.streams = { { stream, CameraStream::Type::Direct } };
		streamConfig.config.size = size;
		streamConfig.config.pixelFormat = format;
		streamConfigs.push_back(std::move(streamConfig));
	}

	/* Now handle the MJPEG streams, adding a new stream if required. */
	if (jpegStream) {
		CameraStream::Type type;
		int index = -1;

		/* Search for a compatible stream in the non-JPEG ones. */
		for (size_t i = 0; i < streamConfigs.size(); ++i) {
			Camera3StreamConfig &streamConfig = streamConfigs[i];
			const auto &cfg = streamConfig.config;

			/*
			 * \todo The PixelFormat must also be compatible with
			 * the encoder.
			 */
			if (cfg.size.width != jpegStream->width ||
			    cfg.size.height != jpegStream->height)
				continue;

			LOG(HAL, Info)
				<< "Android JPEG stream mapped to libcamera stream " << i;

			type = CameraStream::Type::Mapped;
			index = i;

			/*
			 * The source stream will be read by software to
			 * produce the JPEG stream.
			 */
			camera3_stream_t *stream = streamConfig.streams[0].stream;
			stream->usage |= GRALLOC_USAGE_SW_READ_OFTEN;
			break;
		}

		/*
		 * Without a compatible match for JPEG encoding we must
		 * introduce a new stream to satisfy the request requirements.
		 */
		if (index < 0) {
			/*
			 * \todo The pixelFormat should be a 'best-fit' choice
			 * and may require a validation cycle. This is not yet
			 * handled, and should be considered as part of any
			 * stream configuration reworks.
			 */
			Camera3StreamConfig streamConfig;
			streamConfig.config.size.width = jpegStream->width;
			streamConfig.config.size.height = jpegStream->height;
			streamConfig.config.pixelFormat = formats::NV12;
			streamConfigs.push_back(std::move(streamConfig));

			LOG(HAL, Info) << "Adding " << streamConfig.config.toString()
				       << " for MJPEG support";

			type = CameraStream::Type::Internal;
			index = streamConfigs.size() - 1;
		}

		/* The JPEG stream will be produced by software. */
		jpegStream->usage |= GRALLOC_USAGE_SW_WRITE_OFTEN;

		streamConfigs[index].streams.push_back({ jpegStream, type });
	}

	sortCamera3StreamConfigs(streamConfigs, jpegStream);
	return 0;
}

int buildStreamConfigsNoMap(const CameraCapabilities &capabilities,
			    camera3_stream_configuration_t *stream_list,
			    std::vector<Camera3StreamConfig> &streamConfigs)
{
	for (unsigned int i = 0; i < stream_list->num_streams; ++i) {
		camera3_stream_t *stream = stream_list->streams[i];
		Size size(stream->width, stream->height);

		PixelFormat format = capabilities.toPixelFormat(stream->format);

		/*
		 * While gralloc usage flags are supposed to report usage
		 * patterns to select a suitable buffer allocation strategy, in
		 * practice they're also used to make other decisions, such as
		 * selecting the actual format for the IMPLEMENTATION_DEFINED
		 * HAL pixel format. To avoid issues, we thus have to set the
		 * GRALLOC_USAGE_HW_CAMERA_WRITE flag unconditionally, even for
		 * streams that will be produced in software.
		 */
		stream->usage |= (GRALLOC_USAGE_HW_CAMERA_WRITE |
				  GRALLOC_USAGE_SW_READ_OFTEN |
				  GRALLOC_USAGE_SW_WRITE_OFTEN);

		Camera3StreamConfig streamConfig;
		streamConfig.config.size = size;
		streamConfig.config.pixelFormat = format;

		if (isJpegStream(stream)) {
			continue;
		} else if (hasStillCaptureFlag(stream)) {
			streamConfig.streams = { { stream, CameraStream::Type::Direct } };
			streamConfig.config.role = StreamRole::StillCapture;
		} else if (isYuvSnapshotStream(stream)) {
			streamConfig.streams = { { stream, CameraStream::Type::Direct } };
			streamConfig.config.role = StreamRole::YuvSnapshot;
		} else if (isPreviewStream(stream)) {
			streamConfig.streams = { { stream, CameraStream::Type::Direct } };
			streamConfig.config.role = StreamRole::Viewfinder;
		} else if (isVideoStream(stream)) {
			streamConfig.streams = { { stream, CameraStream::Type::Direct } };
			streamConfig.config.role = StreamRole::VideoRecording;
		} else {
			streamConfig.streams = { { stream, CameraStream::Type::Direct } };
			streamConfig.config.role = StreamRole::Viewfinder;
		}
		streamConfigs.push_back(std::move(streamConfig));
	}

	for (unsigned int i = 0; i < stream_list->num_streams; ++i) {
		camera3_stream_t *stream = stream_list->streams[i];
		Size size(stream->width, stream->height);

		PixelFormat format = capabilities.toPixelFormat(stream->format);

		if (!isJpegStream(stream))
			continue;

		bool found = false;
		for (auto &cfg : streamConfigs) {
			if (cfg.config.role == StreamRole::StillCapture &&
			    cfg.streams[0].stream->width == size.width &&
			    cfg.streams[0].stream->height == size.height) {
				cfg.streams.push_back({ stream, CameraStream::Type::Mapped });
				found = true;
				break;
			}
		}

		if (!found) {
			Camera3StreamConfig streamConfig;
			streamConfig.config.size = size;
			streamConfig.config.pixelFormat = format;
			streamConfig.streams = { { stream, CameraStream::Type::Internal } };
			streamConfig.config.role = StreamRole::StillCapture;
			streamConfigs.push_back(std::move(streamConfig));
		}
	}

	/*
	Hardware support maxium 2 video + 2 still capture strem, when still capture
	stream is higher than 2, move the rest to video stream.
	(cts: android.hardware.camera2.cts.RobustnessTest#testMandatoryOutputCombinations)
	*/
	int stillCnt = 0;
	for (auto &streamCfg : streamConfigs) {
		if (streamCfg.config.role == StreamRole::StillCapture)
			stillCnt += 1;
	}
	if (stillCnt > 2) {
		for (auto &streamCfg : streamConfigs) {
			if (streamCfg.config.role == StreamRole::StillCapture &&
			    streamCfg.streams[0].stream->format != HAL_PIXEL_FORMAT_BLOB) {
				streamCfg.config.role = StreamRole::Viewfinder;
				stillCnt -= 1;
				if (stillCnt == 2) {
					break;
				}
			}
		}
	}

	return 0;
}

#if defined(OS_CHROMEOS)
/*
 * Check whether the crop_rotate_scale_degrees values for all streams in
 * the list are valid according to the Chrome OS camera HAL API.
 */
bool validateCropRotate(const camera3_stream_configuration_t &streamList)
{
	ASSERT(streamList.num_streams > 0);
	const int cropRotateScaleDegrees =
		streamList.streams[0]->crop_rotate_scale_degrees;
	for (unsigned int i = 0; i < streamList.num_streams; ++i) {
		const camera3_stream_t &stream = *streamList.streams[i];

		switch (stream.crop_rotate_scale_degrees) {
		case CAMERA3_STREAM_ROTATION_0:
		case CAMERA3_STREAM_ROTATION_90:
		case CAMERA3_STREAM_ROTATION_270:
			break;

		/* 180° rotation is specified by Chrome OS as invalid. */
		case CAMERA3_STREAM_ROTATION_180:
		default:
			LOG(HAL, Error) << "Invalid crop_rotate_scale_degrees: "
					<< stream.crop_rotate_scale_degrees;
			return false;
		}

		if (cropRotateScaleDegrees != stream.crop_rotate_scale_degrees) {
			LOG(HAL, Error) << "crop_rotate_scale_degrees in all "
					<< "streams are not identical";
			return false;
		}
	}

	return true;
}
#endif

} /* namespace */

/*
 * \class CameraDevice
 *
 * The CameraDevice class wraps a libcamera::Camera instance, and implements
 * the camera3_device_t interface, bridging calls received from the Android
 * camera service to the CameraDevice.
 *
 * The class translates parameters and operations from the Camera HALv3 API to
 * the libcamera API to provide static information for a Camera, create request
 * templates for it, process capture requests and then deliver capture results
 * back to the framework using the designated callbacks.
 */

CameraDevice::CameraDevice(unsigned int id, std::shared_ptr<Camera> camera)
	: id_(id), state_(State::Stopped), camera_(std::move(camera)),
	  facing_(CAMERA_FACING_FRONT), orientation_(0), resourceCost_(100),
	  opened_(false)
{
	/* Set RequestCompletionMode to Immediately to send results early */
	camera_->setRequestCompletionMode(Camera::Immediately);

	camera_->requestCompleted.connect(this, &CameraDevice::requestComplete);
	camera_->bufferCompleted.connect(this, &CameraDevice::bufferComplete);
	camera_->metadataAvailable.connect(this, &CameraDevice::metadataAvailable);
	camera_->disconnected.connect(this, &CameraDevice::cameraDisconnected);
#ifdef HAVE_ANDROID_OS
	mBufferAdapter = std::make_shared<android::BufferAdapter>();
#endif
}

CameraDevice::~CameraDevice() = default;

std::unique_ptr<CameraDevice> CameraDevice::create(unsigned int id,
						   std::shared_ptr<Camera> cam)
{
	return std::unique_ptr<CameraDevice>(
		new CameraDevice(id, std::move(cam)));
}

/*
 * Initialize the camera static information retrieved from the
 * Camera::properties or from the cameraConfigData.
 *
 * cameraConfigData is optional for external camera devices and can be
 * nullptr.
 *
 * This function is called before the camera device is opened.
 */
int CameraDevice::initialize(const CameraConfigData *cameraConfigData)
{
#ifdef HAVE_ANDROID_OS
	if (!mBufferAdapter->init()) {
		LOG(HAL, Error) << "Failed to initialize mBufferAdapter";
		return -EINVAL;
	}
#endif
	/*
	 * Initialize orientation and facing side of the camera.
	 *
	 * If the libcamera::Camera provides those information as retrieved
	 * from firmware use them, otherwise fallback to values parsed from
	 * the configuration file. If the configuration file is not available
	 * the camera is external so its location and rotation can be safely
	 * defaulted.
	 */
	const ControlList &properties = camera_->properties();

	const auto &location = properties.get(properties::Location);
	if (location) {
		switch (*location) {
		case properties::CameraLocationFront:
			facing_ = CAMERA_FACING_FRONT;
			break;
		case properties::CameraLocationBack:
			facing_ = CAMERA_FACING_BACK;
			break;
		case properties::CameraLocationExternal:
			/*
			 * If the camera is reported as external, but the
			 * CameraHalManager has overriden it, use what is
			 * reported in the configuration file. This typically
			 * happens for UVC cameras reported as 'External' by
			 * libcamera but installed in fixed position on the
			 * device.
			 */
			if (cameraConfigData && cameraConfigData->facing != -1)
				facing_ = cameraConfigData->facing;
			else
				facing_ = CAMERA_FACING_EXTERNAL;
			break;
		}

		if (cameraConfigData && cameraConfigData->facing != -1 &&
		    facing_ != cameraConfigData->facing) {
			LOG(HAL, Warning)
				<< "Camera location does not match"
				<< " configuration file. Using " << facing_;
		}
	} else if (cameraConfigData) {
		if (cameraConfigData->facing == -1) {
			LOG(HAL, Error)
				<< "Camera facing not in configuration file";
			return -EINVAL;
		}
		facing_ = cameraConfigData->facing;
	} else {
		facing_ = CAMERA_FACING_EXTERNAL;
	}

	/*
	 * The Android orientation metadata specifies its rotation correction
	 * value in clockwise direction whereas libcamera specifies the
	 * rotation property in anticlockwise direction. Read the libcamera's
	 * rotation property (anticlockwise) and compute the corresponding
	 * value for clockwise direction as required by the Android orientation
	 * metadata.
	 */
	const auto &rotation = properties.get(properties::Rotation);
	if (rotation) {
		orientation_ = (360 - *rotation) % 360;
		if (cameraConfigData && cameraConfigData->rotation != -1 &&
		    orientation_ != cameraConfigData->rotation) {
			LOG(HAL, Warning)
				<< "Camera orientation does not match"
				<< " configuration file. Using " << orientation_;
		}
	} else if (cameraConfigData) {
		if (cameraConfigData->rotation == -1) {
			LOG(HAL, Error)
				<< "Camera rotation not in configuration file";
			return -EINVAL;
		}
		orientation_ = cameraConfigData->rotation;
	} else {
		orientation_ = 0;
	}

	const auto &resourceCost = properties.get(properties::ResourceCost);
	if (resourceCost) {
		resourceCost_ = *resourceCost;
	} else {
		resourceCost_ = 100;  // Allowing only one camera
	}

	return capabilities_.initialize(camera_, orientation_, facing_);
}

/*
 * Open a camera device. The static information on the camera shall have been
 * initialized with a call to CameraDevice::initialize().
 */
int CameraDevice::open(const hw_module_t *hardwareModule)
{
	if (opened_)
		return -EUSERS;

	opened_ = true;

	int ret = camera_->acquire();
	if (ret) {
		LOG(HAL, Error) << "Failed to acquire the camera";
		opened_ = false;
		return ret;
	}

	/* Initialize the hw_device_t in the instance camera3_module_t. */
	camera3Device_.common.tag = HARDWARE_DEVICE_TAG;
	camera3Device_.common.version = CAMERA_DEVICE_API_VERSION_3_5;
	camera3Device_.common.module = (hw_module_t *)hardwareModule;
	camera3Device_.common.close = hal_dev_close;

	/*
	 * The camera device operations. These actually implement
	 * the Android Camera HALv3 interface.
	 */
	camera3Device_.ops = &hal_dev_ops;
	camera3Device_.priv = this;

	/*
	 * The manufacturer info is only available after the Android VM booted.
	 * The camera service may load and initialize libcamera before the VM
	 * boot, but when opening the camera we are sure the VM already booted.
	 */
	if (!maker_.has_value() || !model_.has_value()) {
		queryManufacturerInfo();
	}

	return 0;
}

void CameraDevice::close()
{
	flushAndStop();

	camera_->release();
	opened_ = false;
}

void CameraDevice::flushAndStop()
{
	{
		MutexLocker stateLock(stateMutex_);
		if (state_ != State::Running)
			return;

		state_ = State::Flushing;
	}

	/* TODO: Add a flush() method in pipeline handler to do the flushing */
	{
		MutexLocker locker(pendingRequestMutex_);
		pendingRequestsCv_.wait(
			locker,
			[&]() LIBCAMERA_TSA_REQUIRES(pendingRequestMutex_) {
				return pendingRequests_.empty();
			});
		ASSERT(pendingRequests_.empty());
	}

	camera_->stop();

	MutexLocker stateLock(stateMutex_);
	state_ = State::Stopped;
}

unsigned int CameraDevice::maxJpegBufferSize() const
{
	return capabilities_.maxJpegBufferSize();
}

void CameraDevice::setCallbacks(const camera3_callback_ops_t *callbacks)
{
	callbacks_ = callbacks;
}

const camera_metadata_t *CameraDevice::getStaticMetadata()
{
	return capabilities_.staticMetadata()->getMetadata();
}

/*
 * Produce a metadata pack to be used as template for a capture request.
 */
const camera_metadata_t *CameraDevice::constructDefaultRequestSettings(int type)
{
	auto it = requestTemplates_.find(type);
	if (it != requestTemplates_.end())
		return it->second->getMetadata();

	/* Use the capture intent matching the requested template type. */
	std::unique_ptr<CameraMetadata> requestTemplate;
	uint8_t captureIntent;
	switch (type) {
	case CAMERA3_TEMPLATE_PREVIEW:
		captureIntent = ANDROID_CONTROL_CAPTURE_INTENT_PREVIEW;
		requestTemplate = capabilities_.requestTemplatePreview();
		break;
	case CAMERA3_TEMPLATE_STILL_CAPTURE:
		/*
		 * Use the preview template for still capture, they only differ
		 * for the torch mode we currently do not support.
		 */
		captureIntent = ANDROID_CONTROL_CAPTURE_INTENT_STILL_CAPTURE;
		requestTemplate = capabilities_.requestTemplateStill();
		break;
	case CAMERA3_TEMPLATE_VIDEO_RECORD:
		captureIntent = ANDROID_CONTROL_CAPTURE_INTENT_VIDEO_RECORD;
		requestTemplate = capabilities_.requestTemplateVideo();
		break;
	case CAMERA3_TEMPLATE_VIDEO_SNAPSHOT:
		captureIntent = ANDROID_CONTROL_CAPTURE_INTENT_VIDEO_SNAPSHOT;
		requestTemplate = capabilities_.requestTemplateVideo();
		break;
	case CAMERA3_TEMPLATE_MANUAL:
		captureIntent = ANDROID_CONTROL_CAPTURE_INTENT_MANUAL;
		requestTemplate = capabilities_.requestTemplateManual();
		break;
	/* \todo Debug the exception of android.camera.cts.api25test.EnableZslTest. */
	case CAMERA3_TEMPLATE_ZERO_SHUTTER_LAG:
		captureIntent = ANDROID_CONTROL_CAPTURE_INTENT_ZERO_SHUTTER_LAG;
		requestTemplate = capabilities_.requestTemplateZsl();
		break;
	/* \todo Implement templates generation for the remaining use cases. */
	default:
		LOG(HAL, Error) << "Unsupported template request type: " << type;
		return nullptr;
	}

	if (!requestTemplate || !requestTemplate->isValid()) {
		LOG(HAL, Error) << "Failed to construct request template";
		return nullptr;
	}

	requestTemplate->updateEntry(ANDROID_CONTROL_CAPTURE_INTENT,
				     captureIntent);

	requestTemplates_[type] = std::move(requestTemplate);
	return requestTemplates_[type]->getMetadata();
}

/*
 * Inspect the stream_list to produce a list of StreamConfiguration to
 * be use to configure the Camera.
 */
int CameraDevice::configureStreams(camera3_stream_configuration_t *stream_list)
{
	/* Before any configuration attempt, stop the camera. */
	flushAndStop();

	/* Configure streams can only be called after all pending requests
	 * from the previous session finish. */
	{
		MutexLocker descriptorsLock(pendingRequestMutex_);

		ASSERT(pendingRequests_.empty());
		ASSERT(pendingPartialResults_.empty());
		for (auto &[_, streamBuffers] : pendingStreamBuffers_)
			ASSERT(streamBuffers.empty());

		pendingStreamBuffers_.clear();
	}

	if (stream_list->num_streams == 0) {
		LOG(HAL, Error) << "No streams in configuration";
		return -EINVAL;
	}

#if defined(OS_CHROMEOS)
	if (!validateCropRotate(*stream_list))
		return -EINVAL;
#endif

	for (unsigned int i = 0; i < stream_list->num_streams; ++i) {
		camera3_stream_t *stream = stream_list->streams[i];
		Size size(stream->width, stream->height);

		PixelFormat format = capabilities_.toPixelFormat(stream->format);

		LOG(HAL, Info) << "Stream #" << i
			       << ", direction: " << directionToString(stream->stream_type)
			       << ", width: " << stream->width
			       << ", height: " << stream->height
			       << ", format: " << utils::hex(stream->format)
			       << ", rotation: " << rotationToString(stream->rotation)
#if defined(OS_CHROMEOS)
			       << ", crop_rotate_scale_degrees: "
			       << rotationToString(stream->crop_rotate_scale_degrees)
#endif
			       << " (" << format << ")";

		if (!format.isValid())
			return -EINVAL;

		/* \todo Support rotation. */
		if (stream->rotation != CAMERA3_STREAM_ROTATION_0) {
			LOG(HAL, Error) << "Rotation is not supported";
			return -EINVAL;
		}
#if defined(OS_CHROMEOS)
		if (stream->crop_rotate_scale_degrees != CAMERA3_STREAM_ROTATION_0) {
			LOG(HAL, Error) << "Rotation is not supported";
			return -EINVAL;
		}
#endif
	}

	/*
	 * Clear and remove any existing configuration from previous calls, and
	 * ensure the required entries are available without further
	 * reallocation.
	 */
	streams_.clear();
	streams_.reserve(stream_list->num_streams);

	std::vector<Camera3StreamConfig> streamConfigs;
	streamConfigs.reserve(stream_list->num_streams);

	if (buildStreamConfigsNoMap(capabilities_, stream_list, streamConfigs))
		return -EINVAL;

	/*
	 * Generate an empty configuration, and construct a StreamConfiguration
	 * for each camera3_stream to add to it.
	 */
	std::unique_ptr<CameraConfiguration> config = camera_->generateConfiguration();
	if (!config) {
		LOG(HAL, Error) << "Failed to generate camera configuration";
		return -EINVAL;
	}

	for (const auto &streamConfig : streamConfigs) {
		config->addConfiguration(streamConfig.config);

		CameraStream *sourceStream = nullptr;
		for (auto &stream : streamConfig.streams) {
			streams_.emplace_back(this, config.get(), stream.type,
					      stream.stream, sourceStream,
					      config->size() - 1);
			stream.stream->priv = static_cast<void *>(&streams_.back());

			/*
			 * The streamConfig.streams vector contains as its first
			 * element a Direct (or Internal) stream, and then an
			 * optional set of Mapped streams derived from the
			 * Direct stream. Cache the Direct stream pointer, to
			 * be used when constructing the subsequent mapped
			 * streams.
			 */
			if (stream.type == CameraStream::Type::Direct)
				sourceStream = &streams_.back();
		}
	}

	switch (config->validate()) {
	case CameraConfiguration::Valid:
		break;
	case CameraConfiguration::Adjusted:
		LOG(HAL, Info) << "Camera configuration adjusted";

		for (const StreamConfiguration &cfg : *config)
			LOG(HAL, Info) << " - " << cfg.toString();

		return -EINVAL;
	case CameraConfiguration::Invalid:
		LOG(HAL, Info) << "Camera configuration invalid";
		return -EINVAL;
	}

	sessionSettings_ = CameraMetadata();
	if (stream_list->session_parameters)
		sessionSettings_ = stream_list->session_parameters;

	/* CCA uses the Target AE FPS to differentiate Video or Still usecase */
	camera_metadata_ro_entry_t entry;
	if (sessionSettings_.getEntry(ANDROID_CONTROL_AE_TARGET_FPS_RANGE, &entry)) {
		const int32_t *data = entry.data.i32;
		int32_t minFps = data[0];
		int32_t maxFps = data[1];

		if (minFps == 15 && maxFps == 30)
			config->captureIntent = libcamera::CameraConfiguration::StillCapture;
		else if (minFps == 30 && maxFps == 30)
			config->captureIntent = libcamera::CameraConfiguration::Video;
		else
			config->captureIntent = libcamera::CameraConfiguration::Unknown;
	}

	/*
	 * Once the CameraConfiguration has been adjusted/validated
	 * it can be applied to the camera.
	 */
	int ret = camera_->configure(config.get());
	if (ret) {
		LOG(HAL, Error) << "Failed to configure camera '"
				<< camera_->id() << "'";
		return ret;
	}

	/*
	 * Configure the HAL CameraStream instances using the associated
	 * StreamConfiguration and set the number of required buffers in
	 * the Android camera3_stream_t.
	 */
	for (CameraStream &cameraStream : streams_) {
		ret = cameraStream.configure();
		if (ret) {
			LOG(HAL, Error) << "Failed to configure camera stream";
			return ret;
		}
	}

	config_ = std::move(config);

	/*
	 * camera_->start() includes several I/O operations and take some
	 * time to process. The old process only triggered camera_->start()
	 * at process_capture_request and cause frame delay at the first
	 * request, the frame delay makes the cts test
	 * "android.hardware.camera2.cts.RecordingTest#testVideoSnapshot"
	 * easy to fail. Therefore, trigger camera_->start() earlier at
	 * configure_streams stage to avoid such situation.
	 */
	MutexLocker stateLock(stateMutex_);
	if (state_ == State::Stopped) {
		ret = camera_->start();
		if (ret) {
			LOG(HAL, Error) << "Failed to start camera";
			return ret;
		}

		state_ = State::Running;
	}
	return 0;
}

std::unique_ptr<HALFrameBuffer>
CameraDevice::createFrameBuffer(const buffer_handle_t camera3buffer,
				PixelFormat pixelFormat, const Size &size)
{
	CameraBuffer buf(camera3buffer, pixelFormat, size, PROT_READ);
	if (!buf.isValid()) {
		LOG(HAL, Fatal) << "Failed to create CameraBuffer";
		return nullptr;
	}

	std::vector<FrameBuffer::Plane> planes(buf.numPlanes());
	for (size_t i = 0; i < buf.numPlanes(); ++i) {
		SharedFD fd{ camera3buffer->data[i] };
		if (!fd.isValid()) {
			LOG(HAL, Fatal) << "No valid fd";
			return nullptr;
		}

		planes[i].fd = fd;
		planes[i].offset = buf.offset(i);
		planes[i].length = buf.size(i);
	}

	return std::make_unique<HALFrameBuffer>(planes, camera3buffer);
}

int CameraDevice::processControls(Camera3RequestDescriptor *descriptor)
{
	const CameraMetadata &settings = descriptor->settings_;
	if (!settings.isValid())
		return 0;

	/* Translate the Android request settings to libcamera controls. */
	ControlList &controls = descriptor->request_->controls();
	camera_metadata_ro_entry_t entry;
	if (settings.getEntry(ANDROID_SCALER_CROP_REGION, &entry)) {
		const int32_t *data = entry.data.i32;
		Rectangle cropRegion{ data[0], data[1],
				      static_cast<unsigned int>(data[2]),
				      static_cast<unsigned int>(data[3]) };
		controls.set(controls::ScalerCrop, cropRegion);
	}

	if (settings.getEntry(ANDROID_STATISTICS_FACE_DETECT_MODE, &entry)) {
		const int32_t *data = entry.data.i32;
		controls.set(controls::draft::FaceDetectMode, data[0]);
		if (!controls.get(controls::draft::FaceDetectMode)) {
			LOG(HAL, Warning) << "Pipeline doesn't support controls::draft::FaceDetectMode";
		}
	}

	if (settings.getEntry(ANDROID_CONTROL_AF_REGIONS, &entry)) {
		const int32_t *data = entry.data.i32;
		std::vector<Rectangle> afWindows;
		for (size_t i = 0; i + 4 < entry.count; i += 5) {
			size_t j = i * 5;
			afWindows.push_back(Rectangle{
				data[j], data[j + 1],
				static_cast<unsigned int>(data[j + 2] - data[j]),
				static_cast<unsigned int>(data[j + 3] - data[j + 1]) });
		}
		controls.set(controls::AfWindows, afWindows);
	}

	if (settings.getEntry(ANDROID_CONTROL_MODE, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::Mode3A, data[0]);
		if (!controls.get(controls::Mode3A)) {
			LOG(HAL, Warning) << "Pipeline doesn't support controls::Mode3A";
		}
	}

	if (settings.getEntry(ANDROID_CONTROL_SCENE_MODE, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::SceneMode, data[0]);
		if (!controls.get(controls::SceneMode)) {
			LOG(HAL, Warning) << "Pipeline doesn't support controls::SceneMode";
		}
	}

	if (settings.getEntry(ANDROID_SENSOR_TEST_PATTERN_MODE, &entry)) {
		const int32_t data = *entry.data.i32;
		int32_t testPatternMode = controls::draft::TestPatternModeOff;
		switch (data) {
		case ANDROID_SENSOR_TEST_PATTERN_MODE_OFF:
			testPatternMode = controls::draft::TestPatternModeOff;
			break;

		case ANDROID_SENSOR_TEST_PATTERN_MODE_SOLID_COLOR:
			testPatternMode = controls::draft::TestPatternModeSolidColor;
			break;

		case ANDROID_SENSOR_TEST_PATTERN_MODE_COLOR_BARS:
			testPatternMode = controls::draft::TestPatternModeColorBars;
			break;

		case ANDROID_SENSOR_TEST_PATTERN_MODE_COLOR_BARS_FADE_TO_GRAY:
			testPatternMode = controls::draft::TestPatternModeColorBarsFadeToGray;
			break;

		case ANDROID_SENSOR_TEST_PATTERN_MODE_PN9:
			testPatternMode = controls::draft::TestPatternModePn9;
			break;

		case ANDROID_SENSOR_TEST_PATTERN_MODE_CUSTOM1:
			testPatternMode = controls::draft::TestPatternModeCustom1;
			break;

		default:
			LOG(HAL, Error)
				<< "Unknown test pattern mode: " << data;

			return -EINVAL;
		}

		controls.set(controls::draft::TestPatternMode, testPatternMode);
	}

	if (settings.getEntry(ANDROID_CONTROL_AE_MODE, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::draft::AeMode, static_cast<int>(data[0]));
	}

	if (settings.getEntry(ANDROID_SENSOR_EXPOSURE_TIME, &entry)) {
		const int64_t *data = entry.data.i64;
		controls.set(controls::ExposureTime, static_cast<int32_t>(data[0] / 1000));
	}

	if (settings.getEntry(ANDROID_SENSOR_SENSITIVITY, &entry)) {
		const int32_t *data = entry.data.i32;
		controls.set(controls::AnalogueGain, static_cast<float>(data[0]));
	}

	if (settings.getEntry(ANDROID_SENSOR_FRAME_DURATION, &entry)) {
		const int64_t *data = entry.data.i64;
		controls.set(controls::FrameDuration, data[0]);
	}

	if (settings.getEntry(ANDROID_CONTROL_AE_LOCK, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::AeLocked, static_cast<bool>(data[0]));
	}

	if (settings.getEntry(ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER, &entry)) {
		const int32_t *data = entry.data.i32;
		controls.set(controls::draft::AePrecaptureTrigger, static_cast<int32_t>(data[0]));
	}

	if (settings.getEntry(ANDROID_CONTROL_AWB_MODE, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::AwbMode, data[0]);
	}

	if (settings.getEntry(ANDROID_CONTROL_AWB_LOCK, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::AwbLocked, static_cast<bool>(data[0]));
	}

	if (settings.getEntry(ANDROID_CONTROL_AE_ANTIBANDING_MODE, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::draft::AeAntiBandingMode, static_cast<int32_t>(data[0]));
	}

	if (settings.getEntry(ANDROID_CONTROL_AE_TARGET_FPS_RANGE, &entry)) {
		const int32_t *data = entry.data.i32;
		int64_t maxFrameDuration = 1'000'000 / static_cast<int64_t>(data[0]);
		int64_t minFrameDuration = 1'000'000 / static_cast<int64_t>(data[1]);
		controls.set(controls::FrameDurationLimits, { minFrameDuration, maxFrameDuration });
	}

	if (settings.getEntry(ANDROID_CONTROL_AF_MODE, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::AfMode, static_cast<int>(data[0]));
	}

	if (settings.getEntry(ANDROID_CONTROL_AF_TRIGGER, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::AfTrigger, static_cast<uint8_t>(data[0]));
	}

	if (settings.getEntry(ANDROID_LENS_FOCUS_DISTANCE, &entry)) {
		const float *data = entry.data.f;
		controls.set(controls::draft::LensFocusDistance, static_cast<float>(data[0]));
	}

	if (settings.getEntry(ANDROID_COLOR_CORRECTION_MODE, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::draft::ColorCorrectionMode, static_cast<int>(data[0]));
	}

	if (settings.getEntry(ANDROID_COLOR_CORRECTION_TRANSFORM, &entry)) {
		std::array<float, 9> correctionMatrix;
		for (int i = 0; i < 9; i++) {
			const camera_metadata_rational_t *data_r = entry.data.r + i;
			int32_t numerator = data_r->numerator;
			int32_t denominator = data_r->denominator;
			if (denominator)
				correctionMatrix[i] = static_cast<float>(numerator) / static_cast<float>(denominator);
			else
				correctionMatrix[i] = 0.0f;
		}
		controls.set(controls::ColourCorrectionMatrix, correctionMatrix);
	}

	if (settings.getEntry(ANDROID_COLOR_CORRECTION_GAINS, &entry)) {
		std::array<float, 4> correctionGains;
		for (int i = 0; i < 4; i++) {
			const float *data_f = entry.data.f + i;
			correctionGains[i] = *data_f;
		}
		controls.set(controls::draft::ColorCorrectionGains, correctionGains);
	}

	if (settings.getEntry(ANDROID_TONEMAP_MODE, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::draft::TonemapMode, static_cast<int>(data[0]));
	}

	if (settings.getEntry(ANDROID_TONEMAP_CURVE_RED, &entry)) {
		std::vector<float> tonemapCurveRed;
		for (int i = 0; i < (int)entry.count; i++) {
			tonemapCurveRed.push_back(*(entry.data.f + i));
		}
		controls.set(controls::draft::TonemapCurveRed, tonemapCurveRed);
	}

	if (settings.getEntry(ANDROID_TONEMAP_CURVE_GREEN, &entry)) {
		std::vector<float> tonemapCurveGreen;
		for (int i = 0; i < (int)entry.count; i++) {
			tonemapCurveGreen.push_back(*(entry.data.f + i));
		}
		controls.set(controls::draft::TonemapCurveGreen, tonemapCurveGreen);
	}

	if (settings.getEntry(ANDROID_TONEMAP_CURVE_BLUE, &entry)) {
		std::vector<float> tonemapCurveBlue;
		for (int i = 0; i < (int)entry.count; i++) {
			tonemapCurveBlue.push_back(*(entry.data.f + i));
		}
		controls.set(controls::draft::TonemapCurveBlue, tonemapCurveBlue);
	}

	if (settings.getEntry(ANDROID_NOISE_REDUCTION_MODE, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::draft::NoiseReductionMode, static_cast<int>(data[0]));
	}

	if (settings.getEntry(ANDROID_EDGE_MODE, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::draft::EdgeMode, static_cast<int>(data[0]));
	}

	if (settings.getEntry(VENDOR_TAG_STILL_CAPTURE_MULTI_FRAME_NOISE_REDUCTION, &entry)) {
		const uint8_t *data = entry.data.u8;
		controls.set(controls::draft::StillCaptureMultiFrameNoiseReduction, static_cast<bool>(data[0]));
	}

	if (settings.getEntry(ANDROID_CONTROL_CAPTURE_INTENT, &entry) &&
	    entry.data.u8[0] == ANDROID_CONTROL_CAPTURE_INTENT_STILL_CAPTURE) {
		if (settings.getEntry(ANDROID_CONTROL_ENABLE_ZSL, &entry)) {
			const uint8_t *data = entry.data.u8;
			controls.set(controls::draft::EnableZsl, static_cast<bool>(data[0]));
		}
	}

	return 0;
}

/* abortRequest() is only called before the request is queued into the device,
 * i.e., there is no need to remove it from pendingRequests_ and
 * pendingStreamBuffers_.
 */
void CameraDevice::abortRequest(Camera3RequestDescriptor *descriptor)
{
	/*
	 * Since the failed buffers do not have to follow the strict ordering
	 * valid buffers do, and could be out-of-order with respect to valid
	 * buffers, it's safe to send the aborted result back to the framework
	 * immediately.
	 */
	descriptor->status_ = Camera3RequestDescriptor::Status::Cancelled;
	descriptor->finalResult_ = std::make_unique<Camera3ResultDescriptor>(descriptor);

	Camera3ResultDescriptor *result = descriptor->finalResult_.get();

	result->metadataPackIndex_ = 0;
	for (auto &buffer : descriptor->buffers_) {
		buffer.status = StreamBuffer::Status::Error;
		result->buffers_.emplace_back(&buffer);
	}

	/*
	 * After CAMERA3_MSG_ERROR_REQUEST is notified, for a given frame,
	 * only process_capture_results with buffers of the status
	 * CAMERA3_BUFFER_STATUS_ERROR are allowed. No further notifies or
	 * process_capture_result with non-null metadata is allowed.
	 */
	notifyError(descriptor->frameNumber_, nullptr, CAMERA3_MSG_ERROR_REQUEST);

	sendCaptureResult(result);
}

bool CameraDevice::isValidRequest(camera3_capture_request_t *camera3Request) const
{
	if (!camera3Request) {
		LOG(HAL, Error) << "No capture request provided";
		return false;
	}

	if (!camera3Request->num_output_buffers ||
	    !camera3Request->output_buffers) {
		LOG(HAL, Error) << "No output buffers provided";
		return false;
	}

	/* configureStreams() has not been called or has failed. */
	if (streams_.empty() || !config_) {
		LOG(HAL, Error) << "No stream is configured";
		return false;
	}

	for (uint32_t i = 0; i < camera3Request->num_output_buffers; i++) {
		const camera3_stream_buffer_t &outputBuffer =
			camera3Request->output_buffers[i];
		if (!outputBuffer.buffer || !(*outputBuffer.buffer)) {
			LOG(HAL, Error) << "Invalid native handle";
			return false;
		}

		const native_handle_t *handle = *outputBuffer.buffer;
		constexpr int kNativeHandleMaxFds = 1024;
		if (handle->numFds < 0 || handle->numFds > kNativeHandleMaxFds) {
			LOG(HAL, Error)
				<< "Invalid number of fds (" << handle->numFds
				<< ") in buffer " << i;
			return false;
		}

		constexpr int kNativeHandleMaxInts = 1024;
		if (handle->numInts < 0 || handle->numInts > kNativeHandleMaxInts) {
			LOG(HAL, Error)
				<< "Invalid number of ints (" << handle->numInts
				<< ") in buffer " << i;
			return false;
		}

		const camera3_stream *camera3Stream = outputBuffer.stream;
		if (!camera3Stream)
			return false;

		const CameraStream *cameraStream =
			static_cast<CameraStream *>(camera3Stream->priv);

		auto found = std::find_if(streams_.begin(), streams_.end(),
					  [cameraStream](const CameraStream &stream) {
						  return &stream == cameraStream;
					  });
		if (found == streams_.end()) {
			LOG(HAL, Error)
				<< "No corresponding configured stream found";
			return false;
		}
	}

	return true;
}

int CameraDevice::processCaptureRequest(camera3_capture_request_t *camera3Request)
{
	if (!isValidRequest(camera3Request))
		return -EINVAL;

	/*
	 * Save the request descriptors for use at completion time.
	 * The descriptor and the associated memory reserved here are freed
	 * at request complete time.
	 */
#ifdef HAVE_ANDROID_OS
	auto descriptor = std::make_unique<Camera3RequestDescriptor>(camera_.get(),
								     mBufferAdapter, camera3Request);
#else
	auto descriptor = std::make_unique<Camera3RequestDescriptor>(camera_.get(),
								     camera3Request);
#endif

	/*
	 * \todo The Android request model is incremental, settings passed in
	 * previous requests are to be effective until overridden explicitly in
	 * a new request. Do we need to cache settings incrementally here, or is
	 * it handled by the Android camera service ?
	 */
	if (camera3Request->settings)
		lastSettings_ = camera3Request->settings;
	else
		descriptor->settings_ = lastSettings_;

	LOG(HAL, Debug) << "Queueing request " << descriptor->request_->cookie()
			<< " with " << descriptor->buffers_.size() << " streams";

	/*
	 * Process all the Direct and Internal streams first, they map directly
	 * to a libcamera stream. Streams of type Mapped will be handled later.
	 *
	 * Collect the CameraStream associated to each requested capture stream.
	 * Since requestedDirectBuffers is an std:map<>, no duplications can
	 * happen.
	 */
	std::map<CameraStream *, libcamera::FrameBuffer *> requestedDirectBuffers;
	for (const auto &[i, buffer] : utils::enumerate(descriptor->buffers_)) {
		CameraStream *cameraStream = buffer.stream;
		camera3_stream_t *camera3Stream = cameraStream->camera3Stream();

		std::stringstream ss;
		ss << i << " - (" << camera3Stream->width << "x"
		   << camera3Stream->height << ")"
		   << "[" << utils::hex(camera3Stream->format) << "] -> "
		   << "(" << cameraStream->configuration().size << ")["
		   << cameraStream->configuration().pixelFormat << "]";

		/*
		 * Inspect the camera stream type, create buffers opportunely
		 * and add them to the Request if required.
		 */
		FrameBuffer *frameBuffer = nullptr;
		UniqueFD acquireFence;

		switch (cameraStream->type()) {
		case CameraStream::Type::Mapped:
			/* Mapped streams will be handled in the next loop. */
			continue;

		case CameraStream::Type::Direct:
			/*
			 * Create a libcamera buffer using the dmabuf
			 * descriptors of the camera3Buffer for each stream and
			 * associate it with the Camera3RequestDescriptor for
			 * lifetime management only.
			 */
			buffer.frameBuffer =
				createFrameBuffer(*buffer.camera3Buffer,
						  cameraStream->configuration().pixelFormat,
						  cameraStream->configuration().size);
			frameBuffer = buffer.frameBuffer.get();
			acquireFence = std::move(buffer.fence);

			requestedDirectBuffers[cameraStream] = frameBuffer;
			LOG(HAL, Debug) << ss.str() << " (direct)";
			break;

		case CameraStream::Type::Internal:
			/*
			 * Get the frame buffer from the source stream's
			 * internal buffer pool. The buffer has to be returned
			 * to the source stream once it has been processed.
			 */
			frameBuffer = cameraStream->getBuffer();
			buffer.srcBuffer = frameBuffer;

			/* Track the allocated internal buffers, which will be
			 * recycled when the descriptor destroyed.
			 * */
			descriptor->internalBuffers_[cameraStream] = frameBuffer;
			LOG(HAL, Debug) << ss.str() << " (internal)";
			break;
		}

		if (!frameBuffer) {
			LOG(HAL, Error) << "Failed to create frame buffer";
			return -ENOMEM;
		}

		auto fence = std::make_unique<Fence>(std::move(acquireFence));
		descriptor->request_->addBuffer(cameraStream->stream(),
						frameBuffer, std::move(fence));
	}

	/*
	 * Now handle the Mapped streams. If no buffer has been added for them
	 * because their corresponding direct source stream is not part of this
	 * particular request, add one here.
	 */
	for (const auto &[i, buffer] : utils::enumerate(descriptor->buffers_)) {
		CameraStream *cameraStream = buffer.stream;
		camera3_stream_t *camera3Stream = cameraStream->camera3Stream();

		if (cameraStream->type() != CameraStream::Type::Mapped)
			continue;

		LOG(HAL, Debug) << i << " - (" << camera3Stream->width << "x"
				<< camera3Stream->height << ")"
				<< "[" << utils::hex(camera3Stream->format) << "] -> "
				<< "(" << cameraStream->configuration().size << ")["
				<< cameraStream->configuration().pixelFormat << "]"
				<< " (mapped)";

		/*
		 * Make sure the CameraStream this stream is mapped on has been
		 * added to the request.
		 */
		CameraStream *sourceStream = cameraStream->sourceStream();
		ASSERT(sourceStream);
		ASSERT(sourceStream->type() == CameraStream::Type::Direct);

		/*
		 * If the buffer for the source stream has been requested as
		 * Direct, use its framebuffer as the source buffer for
		 * post-processing. No need to recycle the buffer since it's
		 * owned by Android.
		 */
		auto iterDirectBuffer = requestedDirectBuffers.find(sourceStream);
		if (iterDirectBuffer != requestedDirectBuffers.end()) {
			buffer.srcBuffer = iterDirectBuffer->second;
			continue;
		}

		/*
		 * If that's not the case, we use an internal buffer allocated
		 * from the source stream.
		 *
		 * If an internal buffer has been requested for the source
		 * stream before, we should reuse it.
		 */
		auto iterInternalBuffer = descriptor->internalBuffers_.find(sourceStream);
		if (iterInternalBuffer != descriptor->internalBuffers_.end()) {
			buffer.srcBuffer = iterInternalBuffer->second;
			continue;
		}

		/*
		 * Otherwise, we need to create an internal buffer to the
		 * request for the source stream. Get the frame buffer from the
		 * source stream's internal buffer pool. The buffer has to be
		 * returned to the source stream once it has been processed.
		 */
		FrameBuffer *frameBuffer = sourceStream->getBuffer();
		buffer.srcBuffer = frameBuffer;

		descriptor->request_->addBuffer(sourceStream->stream(),
						frameBuffer, nullptr);

		/* Track the allocated internal buffer. */
		descriptor->internalBuffers_[sourceStream] = frameBuffer;
	}

	/*
	 * Translate controls from Android to libcamera and queue the request
	 * to the camera.
	 */
	int ret = processControls(descriptor.get());
	if (ret)
		return ret;

	/*
	 * If flush is in progress set the request status to error and place it
	 * on the queue to be later completed. If the camera has been stopped we
	 * have to re-start it to be able to process the request.
	 */
	MutexLocker stateLock(stateMutex_);

	if (state_ == State::Flushing) {
		abortRequest(descriptor.get());
		return 0;
	}

	if (state_ == State::Stopped) {
		ret = camera_->start();
		if (ret) {
			LOG(HAL, Error) << "Failed to start camera";
			return ret;
		}

		state_ = State::Running;
	}

	Request *request = descriptor->request_.get();

	{
		MutexLocker descriptorsLock(pendingRequestMutex_);
		for (auto &buffer : descriptor->buffers_)
			pendingStreamBuffers_[buffer.stream].push_back(&buffer);
		pendingRequests_.emplace_back(std::move(descriptor));
	}

	camera_->queueRequest(request);

	return 0;
}

void CameraDevice::bufferComplete(libcamera::Request *request, libcamera::FrameBuffer *frameBuffer)
{
	Camera3RequestDescriptor *descriptor =
		reinterpret_cast<Camera3RequestDescriptor *>(request->cookie());

	descriptor->partialResults_.emplace_back(new Camera3ResultDescriptor(descriptor));
	Camera3ResultDescriptor *camera3Result = descriptor->partialResults_.back().get();

	for (auto &buffer : descriptor->buffers_) {
		CameraStream *cameraStream = buffer.stream;
		if (buffer.srcBuffer != frameBuffer &&
		    buffer.frameBuffer.get() != frameBuffer)
			continue;

		buffer.result = camera3Result;
		camera3Result->buffers_.emplace_back(&buffer);

		StreamBuffer::Status status = StreamBuffer::Status::Success;
		if (frameBuffer->metadata().status != FrameMetadata::FrameSuccess) {
			status = StreamBuffer::Status::Error;
		}
		setBufferStatus(buffer, status);

		switch (cameraStream->type()) {
		case CameraStream::Type::Direct: {
			ASSERT(buffer.frameBuffer.get() == frameBuffer);
			/*
			 * Streams of type Direct have been queued to the
			 * libcamera::Camera and their acquire fences have
			 * already been waited on by the library.
			 */
			std::unique_ptr<Fence> fence = buffer.frameBuffer->releaseFence();
			if (fence)
				buffer.fence = fence->release();
			break;
		}
		case CameraStream::Type::Mapped:
		case CameraStream::Type::Internal:
			ASSERT(buffer.srcBuffer == frameBuffer);
			if (status == StreamBuffer::Status::Error)
				break;

			/*
			 * Acquire fences of streams of type Internal and Mapped
			 * will be handled during post-processing.
			 */
			camera3Result->pendingBuffersToProcess_.emplace_back(&buffer);

			if (cameraStream->isJpegStream()) {
				generateJpegExifMetadata(descriptor, &buffer);

				/*
				 * Allocate for post-processor to fill
				 * in JPEG related metadata.
				 */
				camera3Result->resultMetadata_ = getJpegPartialResultMetadata();
			}

			break;
		}
	}

	for (auto iter = camera3Result->pendingBuffersToProcess_.begin();
	     iter != camera3Result->pendingBuffersToProcess_.end();) {
		StreamBuffer *buffer = *iter;
		int ret = buffer->stream->process(buffer);
		if (ret) {
			iter = camera3Result->pendingBuffersToProcess_.erase(iter);
			setBufferStatus(*buffer, StreamBuffer::Status::Error);
			LOG(HAL, Error) << "Failed to run post process of request "
					<< descriptor->frameNumber_;
		} else {
			iter++;
		}
	}

	if (camera3Result->pendingBuffersToProcess_.empty())
		checkAndCompleteReadyPartialResults(camera3Result);
}

void CameraDevice::metadataAvailable(libcamera::Request *request,
				     const libcamera::ControlList &metadata)
{
	ASSERT(!metadata.empty());

	Camera3RequestDescriptor *descriptor =
		reinterpret_cast<Camera3RequestDescriptor *>(request->cookie());

	descriptor->partialResults_.emplace_back(new Camera3ResultDescriptor(descriptor));
	Camera3ResultDescriptor *camera3Result = descriptor->partialResults_.back().get();

	/*
	 * Notify shutter as soon as we have received SensorTimestamp.
	 */
	const auto &timestamp = metadata.get(controls::SensorTimestamp);
	if (timestamp) {
		notifyShutter(descriptor->frameNumber_, *timestamp);
		LOG(HAL, Debug) << "Request " << request->cookie() << " notifies shutter";
	}

	camera3Result->resultMetadata_ = getPartialResultMetadata(metadata);

	completePartialResultDescriptor(camera3Result);
}

void CameraDevice::requestComplete(Request *request)
{
	Camera3RequestDescriptor *camera3Request =
		reinterpret_cast<Camera3RequestDescriptor *>(request->cookie());

	switch (request->status()) {
	case Request::RequestComplete:
		camera3Request->status_ = Camera3RequestDescriptor::Status::Success;
		break;
	case Request::RequestCancelled:
		camera3Request->status_ = Camera3RequestDescriptor::Status::Cancelled;
		break;
	case Request::RequestPending:
		LOG(HAL, Fatal) << "Try to complete an unfinished request";
		break;
	}

	camera3Request->finalResult_ = std::make_unique<Camera3ResultDescriptor>(camera3Request);
	Camera3ResultDescriptor *result = camera3Request->finalResult_.get();

	/*
	 * On Android, The final result with metadata has to set the field as
	 * CameraCapabilities::MaxMetadataPackIndex, and should be returned by
	 * the submission order of the requests. Create a result as the final
	 * result which is guranteed be sent in order by CompleteRequestDescriptor().
	 */
	result->resultMetadata_ = getFinalResultMetadata(camera3Request,
							 request->metadata());
	result->metadataPackIndex_ = CameraCapabilities::MaxMetadataPackIndex;

	/*
	 * We need to check whether there are partial results pending for
	 * post-processing, before we complete the request descriptor. Otherwise,
	 * the callback of post-processing will complete the request instead.
	 */
	for (auto &r : camera3Request->partialResults_)
		if (!r->completed_)
			return;

	completeRequestDescriptor(camera3Request);
}

void CameraDevice::checkAndCompleteReadyPartialResults(Camera3ResultDescriptor *result)
{
	/*
	 * Android requires buffers for a given stream must be returned in FIFO
	 * order. However, different streams are independent of each other, so
	 * it is acceptable and expected that the buffer for request 5 for
	 * stream A may be returned after the buffer for request 6 for stream
	 * B is. And it is acceptable that the result metadata for request 6
	 * for stream B is returned before the buffer for request 5 for stream
	 * A is. As a result, if all buffers of a result are the most front
	 * buffers of each stream, or the result contains no buffers, the result
	 * is allowed to send. Collect ready results to send in the order which
	 * follows the above rule.
	 *
	 * \todo The reprocessing result can be returned ahead of the pending
	 * normal output results. But the FIFO ordering must be maintained for
	 * all reprocessing results. Track the reprocessing buffer's order
	 * independently when we have reprocessing API.
	 */
	MutexLocker lock(pendingRequestMutex_);

	pendingPartialResults_.emplace_front(result);
	std::list<Camera3ResultDescriptor *> readyResults;

	/*
	 * Error buffers do not have to follow the strict ordering as valid
	 * buffers do. They're ready to be sent directly. Therefore, remove them
	 * from the pendingBuffers so it won't block following valid buffers.
	 */
	for (auto &buffer : result->buffers_)
		if (buffer->status == StreamBuffer::Status::Error)
			pendingStreamBuffers_[buffer->stream].remove(buffer);

	/*
	 * Exhaustly collect results which is ready to sent.
	 */
	bool keepChecking;
	do {
		keepChecking = false;
		auto iter = pendingPartialResults_.begin();
		while (iter != pendingPartialResults_.end()) {
			/*
			 * A result is considered as ready when all of the valid
			 * buffers of the result are at the front of the pending
			 * buffers associated with its stream.
			 */
			bool ready = true;
			for (auto &buffer : (*iter)->buffers_) {
				if (buffer->status == StreamBuffer::Status::Error)
					continue;

				auto &pendingBuffers = pendingStreamBuffers_[buffer->stream];

				ASSERT(!pendingBuffers.empty());

				if (pendingBuffers.front() != buffer) {
					ready = false;
					break;
				}
			}

			if (!ready) {
				iter++;
				continue;
			}

			for (auto &buffer : (*iter)->buffers_)
				if (buffer->status != StreamBuffer::Status::Error)
					pendingStreamBuffers_[buffer->stream].pop_front();

			/* Keep checking since pendingStreamBuffers has updated */
			keepChecking = true;

			readyResults.emplace_back(*iter);
			iter = pendingPartialResults_.erase(iter);
		}
	} while (keepChecking);

	lock.unlock();

	for (auto &res : readyResults) {
		completePartialResultDescriptor(res);
	}
}

void CameraDevice::completePartialResultDescriptor(Camera3ResultDescriptor *result)
{
	Camera3RequestDescriptor *request = result->request_;
	result->completed_ = true;

	/*
	 * Android requires value of metadataPackIndex of partial results
	 * set it to 0 if the result contains only buffers, Otherwise set it
	 * Incrementally from 1 to MaxMetadataPackIndex - 1.
	 */
	if (result->resultMetadata_)
		result->metadataPackIndex_ = request->nextPartialResultIndex_++;
	else
		result->metadataPackIndex_ = 0;

	sendCaptureResult(result);

	/*
	 * The Status would be changed from Pending to Success or Cancelled only
	 * when the requestComplete() has been called. It's garanteed that no
	 * more partial results will be added to the request and the final result
	 * is ready. In the case, if all partial results are completed, we can
	 * complete the request.
	 */
	if (request->status_ == Camera3RequestDescriptor::Status::Pending)
		return;

	for (auto &r : request->partialResults_)
		if (!r->completed_)
			return;

	completeRequestDescriptor(request);
}

/**
 * \brief Complete the Camera3RequestDescriptor
 * \param[in] descriptor The Camera3RequestDescriptor
 *
 * The function shall complete the descriptor only when all of the partial
 * result has sent back to the framework, and send the final result according
 * to the submission order of the requests.
 */
void CameraDevice::completeRequestDescriptor(Camera3RequestDescriptor *request)
{
	MutexLocker locker(pendingRequestMutex_);
	request->completed_ = true;

	while (!pendingRequests_.empty()) {
		auto &descriptor = pendingRequests_.front();
		if (!descriptor->completed_)
			break;

		/*
		 * Android requires the final result of each request returns in
		 * their submission order.
		 */
		ASSERT(descriptor->finalResult_);
		sendCaptureResult(descriptor->finalResult_.get());

		/*
		 * Call notify with CAMERA3_MSG_ERROR_RESULT to indicate some
		 * of the expected result metadata might not be available
		 * because the capture is cancelled by the camera. Only notify
		 * it when the final result is sent, since Android will ignore
		 * the following metadata.
		 */
		if (descriptor->status_ == Camera3RequestDescriptor::Status::Cancelled)
			notifyError(descriptor->frameNumber_, nullptr, CAMERA3_MSG_ERROR_RESULT);

		pendingRequests_.pop_front();
	}

	if (pendingRequests_.empty()) {
		locker.unlock();
		pendingRequestsCv_.notify_one();
		return;
	}
}

void CameraDevice::setBufferStatus(StreamBuffer &streamBuffer,
				   StreamBuffer::Status status) const
{
	streamBuffer.status = status;
	if (status != StreamBuffer::Status::Success) {
		notifyError(streamBuffer.request->frameNumber_,
			    streamBuffer.stream->camera3Stream(),
			    CAMERA3_MSG_ERROR_BUFFER);
	}
}

void CameraDevice::sendCaptureResult(Camera3ResultDescriptor *result) const
{
	std::vector<camera3_stream_buffer_t> resultBuffers;
	resultBuffers.reserve(result->buffers_.size());

	for (auto &buffer : result->buffers_) {
		camera3_buffer_status status = CAMERA3_BUFFER_STATUS_ERROR;

		if (buffer->status == StreamBuffer::Status::Success)
			status = CAMERA3_BUFFER_STATUS_OK;

		camera3_stream_buffer_t outputBuffer = { buffer->stream->camera3Stream(),
							 buffer->camera3Buffer, status,
							 -1, buffer->fence.release() };

#ifdef HAVE_ANDROID_OS
		camera3_stream_buffer_t outputBufferInternal = outputBuffer;
		mBufferAdapter->decodeStreamBufferPtr(&outputBufferInternal, &outputBuffer);
#endif
		/*
		 * Pass the buffer fence back to the camera framework as
		 * a release fence. This instructs the framework to wait
		 * on the acquire fence in case we haven't done so
		 * ourselves for any reason.
		 */
		resultBuffers.push_back(outputBuffer);
	}

	camera3_capture_result_t captureResult = {};

	captureResult.frame_number = result->request_->frameNumber_;
	captureResult.num_output_buffers = resultBuffers.size();
	captureResult.output_buffers = resultBuffers.data();
	captureResult.partial_result = result->metadataPackIndex_;

	if (result->resultMetadata_)
		captureResult.result = result->resultMetadata_->getMetadata();

	callbacks_->process_capture_result(callbacks_, &captureResult);

	LOG(HAL, Debug) << "Send result of frameNumber: "
			<< captureResult.frame_number
			<< " index: " << captureResult.partial_result
			<< " has metadata: " << (!!captureResult.result)
			<< " has buffers " << captureResult.num_output_buffers;
}

void CameraDevice::streamProcessingCompleteDelegate(StreamBuffer *streamBuffer,
						    StreamBuffer::Status status)
{
	/*
	 * Delegate the callback to the camera manager thread to simplify race condition.
	 */
	auto *method = new BoundMethodMember{
		this, camera_.get(), &CameraDevice::streamProcessingComplete, ConnectionTypeQueued
	};

	method->activate(streamBuffer, status);
}

/**
 * \brief Handle post-processing completion of a stream in a capture request
 * \param[in] streamBuffer The StreamBuffer for which processing is complete
 * \param[in] status Stream post-processing status
 *
 * This function is called from the camera's thread whenever a camera
 * stream has finished post processing. The corresponding entry is dropped from
 * the result's pendingBufferToProcess_ list.
 *
 * If the pendingBufferToProcess_ list is then empty, all streams requiring to
 * be generated from post-processing have been completed.
 */
void CameraDevice::streamProcessingComplete(StreamBuffer *streamBuffer,
					    StreamBuffer::Status status)
{
	setBufferStatus(*streamBuffer, status);
	streamBuffer->dstBuffer = nullptr;

	Camera3ResultDescriptor *result = streamBuffer->result;
	result->pendingBuffersToProcess_.remove(streamBuffer);

	if (!result->pendingBuffersToProcess_.empty())
		return;

	checkAndCompleteReadyPartialResults(result);
}

std::string CameraDevice::logPrefix() const
{
	return "'" + camera_->id() + "'";
}

void CameraDevice::notifyShutter(uint32_t frameNumber, uint64_t timestamp)
{
	camera3_notify_msg_t notify = {};

	notify.type = CAMERA3_MSG_SHUTTER;
	notify.message.shutter.frame_number = frameNumber;
	notify.message.shutter.timestamp = timestamp;

	callbacks_->notify(callbacks_, &notify);
}

void CameraDevice::notifyError(uint32_t frameNumber, camera3_stream_t *stream,
			       camera3_error_msg_code code) const
{
	camera3_notify_msg_t notify = {};

	notify.type = CAMERA3_MSG_ERROR;
	notify.message.error.error_stream = stream;
	notify.message.error.frame_number = frameNumber;
	notify.message.error.error_code = code;

	callbacks_->notify(callbacks_, &notify);
}

std::unique_ptr<CameraMetadata> CameraDevice::getJpegPartialResultMetadata() const
{
	/*
         * Reserve more capacity for the JPEG metadata set by the post-processor.
         * Currently: 8 entries, 82 bytes extra capaticy.
         *
         * ANDROID_JPEG_GPS_COORDINATES (double x 3) = 24 bytes
         * ANDROID_JPEG_GPS_PROCESSING_METHOD (byte x 32) = 32 bytes
         * ANDROID_JPEG_GPS_TIMESTAMP (int64) = 8 bytes
         * ANDROID_JPEG_SIZE (int32_t) = 4 bytes
         * ANDROID_JPEG_QUALITY (byte) = 1 byte
         * ANDROID_JPEG_ORIENTATION (int32_t) = 4 bytes
         * ANDROID_JPEG_THUMBNAIL_QUALITY (byte) = 1 byte
         * ANDROID_JPEG_THUMBNAIL_SIZE (int32 x 2) = 8 bytes
         * Total bytes for JPEG metadata: 82
         */
	std::unique_ptr<CameraMetadata> resultMetadata =
		std::make_unique<CameraMetadata>(8, 82);
	if (!resultMetadata->isValid()) {
		LOG(HAL, Error) << "Failed to allocate result metadata";
		return nullptr;
	}

	return resultMetadata;
}

std::unique_ptr<CameraMetadata>
CameraDevice::getPartialResultMetadata(const ControlList &metadata) const
{
	/*
	 * \todo Keep this in sync with the actual number of entries.
	 *
	 * Reserve capacity for the metadata larger than 4 bytes which cannot
	 * store in entries.
	 * Currently: 6 entries, 40 bytes extra capaticy.
	 *
	 * ANDROID_SENSOR_TIMESTAMP (int64) = 8 bytes
	 * ANDROID_SENSOR_EXPOSURE_TIME (int64) = 8 bytes
	 * ANDROID_SENSOR_FRAME_DURATION (int64) = 8 bytes
	 * ANDROID_SCALER_CROP_REGION (int32 X 4) = 16 bytes
	 * Total bytes for capacity: 40
	 *
	 * Reserve more capacity for the JPEG metadata set by the post-processor.
	 * Currently: 8 entries, 72 bytes extra capaticy.
	 *
	 * ANDROID_JPEG_GPS_COORDINATES (double x 3) = 24 bytes
	 * ANDROID_JPEG_GPS_PROCESSING_METHOD (byte x 32) = 32 bytes
	 * ANDROID_JPEG_GPS_TIMESTAMP (int64) = 8 bytes
	 * ANDROID_JPEG_SIZE (int32_t) = 4 bytes
	 * ANDROID_JPEG_QUALITY (byte) = 1 byte
	 * ANDROID_JPEG_ORIENTATION (int32_t) = 4 bytes
	 * ANDROID_JPEG_THUMBNAIL_QUALITY (byte) = 1 byte
	 * ANDROID_JPEG_THUMBNAIL_SIZE (int32 x 2) = 8 bytes
	 * Total bytes for JPEG metadata: 72
	 *
	 * \todo Calculate the entries and capacity by the input ControlList.
	 */
	std::unique_ptr<CameraMetadata> resultMetadata =
		std::make_unique<CameraMetadata>(14, 224);
	if (!resultMetadata->isValid()) {
		LOG(HAL, Error) << "Failed to allocate result metadata";
		return nullptr;
	}

	/* Add metadata tags reported by libcamera. */
	const auto &timestamp = metadata.get(controls::SensorTimestamp);
	if (timestamp)
		resultMetadata->addEntry(ANDROID_SENSOR_TIMESTAMP, *timestamp);

	const auto &pipelineDepth = metadata.get(controls::draft::PipelineDepth);
	if (pipelineDepth)
		resultMetadata->addEntry(ANDROID_REQUEST_PIPELINE_DEPTH,
					 *pipelineDepth);

	if (metadata.contains(controls::EXPOSURE_TIME)) {
		const auto &exposureTime = metadata.get(controls::ExposureTime);
		int64_t exposure_time = static_cast<int64_t>(exposureTime.value_or(33'333));
		resultMetadata->addEntry(ANDROID_SENSOR_EXPOSURE_TIME, exposure_time * 1000ULL);
	}

	if (metadata.contains(controls::draft::AE_STATE)) {
		const auto &aeState = metadata.get(controls::draft::AeState);
		resultMetadata->addEntry(ANDROID_CONTROL_AE_STATE, aeState.value_or(0));
	}

	if (metadata.contains(controls::AF_STATE)) {
		const auto &afState = metadata.get(controls::AfState);
		resultMetadata->addEntry(ANDROID_CONTROL_AF_STATE, afState.value_or(0));
	}

	if (metadata.contains(controls::draft::LENS_FOCUS_DISTANCE)) {
		const auto &lensFocusDistance = metadata.get(controls::draft::LensFocusDistance);
		resultMetadata->addEntry(ANDROID_LENS_FOCUS_DISTANCE, lensFocusDistance.value_or(0));
	}

	if (metadata.contains(controls::draft::LENS_FOCUS_RANGE)) {
		const auto &lensFocusRange = metadata.get(controls::draft::LensFocusRange);
		resultMetadata->addEntry(ANDROID_LENS_FOCUS_RANGE, *lensFocusRange);
	}

	if (metadata.contains(controls::ANALOGUE_GAIN)) {
		const auto &sensorSensitivity = metadata.get(controls::AnalogueGain).value_or(100);
		resultMetadata->addEntry(ANDROID_SENSOR_SENSITIVITY, static_cast<int>(sensorSensitivity));
	}

	const auto &awbState = metadata.get(controls::draft::AwbState);
	if (metadata.contains(controls::draft::AWB_STATE)) {
		resultMetadata->addEntry(ANDROID_CONTROL_AWB_STATE, awbState.value_or(0));
	}

	const auto &frameDuration = metadata.get(controls::FrameDuration);
	if (metadata.contains(controls::FRAME_DURATION)) {
		resultMetadata->addEntry(ANDROID_SENSOR_FRAME_DURATION, frameDuration.value_or(33'333'333));
	}

	const auto &lensState = metadata.get(controls::draft::LensState);
	if (metadata.contains(controls::draft::LENS_STATE)) {
		resultMetadata->addEntry(ANDROID_LENS_STATE, lensState.value_or(0));
	}

	const auto &faceDetectRectangles =
		metadata.get(controls::draft::FaceDetectFaceRectangles);
	if (faceDetectRectangles) {
		std::vector<int32_t> flatRectangles;
		for (const Rectangle &rect : *faceDetectRectangles) {
			flatRectangles.push_back(rect.x);
			flatRectangles.push_back(rect.y);
			flatRectangles.push_back(rect.x + rect.width);
			flatRectangles.push_back(rect.y + rect.height);
		}
		resultMetadata->addEntry(
			ANDROID_STATISTICS_FACE_RECTANGLES, flatRectangles);
	}

	const auto &faceDetectFaceScores =
		metadata.get(controls::draft::FaceDetectFaceScores);
	if (faceDetectRectangles && faceDetectFaceScores) {
		if (faceDetectFaceScores->size() != faceDetectRectangles->size()) {
			LOG(HAL, Error) << "Pipeline returned wrong number of face scores; "
					<< "Expected: " << faceDetectRectangles->size()
					<< ", got: " << faceDetectFaceScores->size();
		}
		resultMetadata->addEntry(ANDROID_STATISTICS_FACE_SCORES,
					 *faceDetectFaceScores);
	}

	const auto &faceDetectFaceLandmarks =
		metadata.get(controls::draft::FaceDetectFaceLandmarks);
	if (faceDetectRectangles && faceDetectFaceLandmarks) {
		size_t expectedLandmarks = faceDetectRectangles->size() * 3;
		if (faceDetectFaceLandmarks->size() != expectedLandmarks) {
			LOG(HAL, Error) << "Pipeline returned wrong number of face landmarks; "
					<< "Expected: " << expectedLandmarks
					<< ", got: " << faceDetectFaceLandmarks->size();
		}

		std::vector<int32_t> androidLandmarks;
		for (const Point &landmark : *faceDetectFaceLandmarks) {
			androidLandmarks.push_back(landmark.x);
			androidLandmarks.push_back(landmark.y);
		}
		resultMetadata->addEntry(
			ANDROID_STATISTICS_FACE_LANDMARKS, androidLandmarks);
	}

	const auto &faceDetectFaceIds = metadata.get(controls::draft::FaceDetectFaceIds);
	if (faceDetectRectangles && faceDetectFaceIds) {
		if (faceDetectFaceIds->size() != faceDetectRectangles->size()) {
			LOG(HAL, Error) << "Pipeline returned wrong number of face ids; "
					<< "Expected: " << faceDetectRectangles->size()
					<< ", got: " << faceDetectFaceIds->size();
		}
		resultMetadata->addEntry(ANDROID_STATISTICS_FACE_IDS, *faceDetectFaceIds);
	}

	const auto &scalerCrop = metadata.get(controls::ScalerCrop);
	if (scalerCrop) {
		const Rectangle &crop = *scalerCrop;
		int32_t cropRect[] = {
			crop.x,
			crop.y,
			static_cast<int32_t>(crop.width),
			static_cast<int32_t>(crop.height),
		};
		resultMetadata->addEntry(ANDROID_SCALER_CROP_REGION, cropRect);
	}

	const auto &testPatternMode = metadata.get(controls::draft::TestPatternMode);
	if (testPatternMode)
		resultMetadata->addEntry(ANDROID_SENSOR_TEST_PATTERN_MODE,
					 *testPatternMode);

	/*
	 * Return the result metadata pack even is not valid: get() will return
	 * nullptr.
	 */
	if (!resultMetadata->isValid()) {
		LOG(HAL, Error) << "Failed to construct result metadata";
	}

	if (resultMetadata->resized()) {
		auto [entryCount, dataCount] = resultMetadata->usage();
		LOG(HAL, Info)
			<< "Result metadata resized: " << entryCount
			<< " entries and " << dataCount << " bytes used";
	}

	return resultMetadata;
}

/*
 * Set jpeg metadata used to generate EXIF in the JPEG post processing.
 */
void CameraDevice::generateJpegExifMetadata(Camera3RequestDescriptor *request,
					    StreamBuffer *buffer) const
{
	const ControlList &metadata = request->request_->metadata();
	auto &jpegExifMetadata = buffer->jpegExifMetadata;
	jpegExifMetadata.emplace(StreamBuffer::JpegExifMetadata());

	const int64_t exposureTime = metadata.get(controls::ExposureTime).value_or(0);
	jpegExifMetadata->sensorExposureTime = exposureTime;

	/*
	 * todo: Android Sensitivity should only include analog gain X digital
	 * gain from sensor. Digital gain on ISP shouldn't be included.
	 * Calculate sensitivity accordingly when we can differentiate
	 * the source of digital gains.
	 * For now assuming digital gain = 1, therefore
	 * ISO sensitivity = analog gain.
	 */
	int32_t intIso = static_cast<int32_t>(
		metadata.get(controls::AnalogueGain).value_or(100));
	jpegExifMetadata->sensorSensitivityISO = intIso;

	camera_metadata_ro_entry_t entry;
	if (request->settings_.getEntry(ANDROID_LENS_FOCAL_LENGTH, &entry)) {
		jpegExifMetadata->lensFocalLength = *entry.data.f;
	} else {
		jpegExifMetadata->lensFocalLength = 1.0f;
	}
}

/*
 * Produce a result metadata for the final result.
 */
std::unique_ptr<CameraMetadata>
CameraDevice::getFinalResultMetadata(
	Camera3RequestDescriptor *camera3Request,
	const libcamera::ControlList &metadata) const
{
	camera_metadata_ro_entry_t entry;
	bool found;

	/*
	 * \todo Retrieve metadata from corresponding libcamera controls.
	 * \todo Keep this in sync with the actual number of entries.
	 *
	 * Reserve capacity for the metadata larger than 4 bytes which cannot
	 * store in entries.
	 * Currently: 31 entries, 16 bytes
	 *
	 * ANDROID_CONTROL_AE_TARGET_FPS_RANGE (int32 X 2) = 8 bytes
	 * ANDROID_SENSOR_ROLLING_SHUTTER_SKEW (int64) = 8 bytes
	 *
	 * Total bytes: 16
	 */
	std::unique_ptr<CameraMetadata> resultMetadata =
		std::make_unique<CameraMetadata>(69, 8212);
	if (!resultMetadata->isValid()) {
		LOG(HAL, Error) << "Failed to allocate result metadata";
		return nullptr;
	}

	std::unordered_set<uint32_t> tagsToAvoid;
	for (const auto &partialResult : camera3Request->partialResults_) {
		if (!partialResult->resultMetadata_) continue;

		for (auto tag : kDuplicatedMetadata) {
			if (partialResult->resultMetadata_->hasEntry(tag))
				tagsToAvoid.insert(tag);
		}
	}

	resultMetadata->addTagsToAvoid(tagsToAvoid);

	if (!metadata.contains(controls::draft::AE_STATE))
		resultMetadata->addEntry(ANDROID_CONTROL_AE_STATE,
					 ANDROID_CONTROL_AE_STATE_CONVERGED);

	if (!metadata.contains(controls::AF_STATE))
		resultMetadata->addEntry(ANDROID_CONTROL_AF_STATE,
					 ANDROID_CONTROL_AF_STATE_INACTIVE);

	if (!metadata.contains(controls::draft::AWB_STATE))
		resultMetadata->addEntry(ANDROID_CONTROL_AWB_STATE,
					 ANDROID_CONTROL_AWB_STATE_CONVERGED);

	if (!metadata.contains(controls::draft::LENS_STATE))
		resultMetadata->addEntry(ANDROID_LENS_STATE,
					 ANDROID_LENS_STATE_STATIONARY);

	if (!metadata.get(controls::draft::TestPatternMode))
		resultMetadata->addEntry(ANDROID_SENSOR_TEST_PATTERN_MODE,
					 ANDROID_SENSOR_TEST_PATTERN_MODE_OFF);
	/*
	 * \todo The value of the results metadata copied from the settings
	 * will have to be passed to the libcamera::Camera and extracted
	 * from libcamera::Request::metadata.
	 */

	uint8_t value = ANDROID_COLOR_CORRECTION_ABERRATION_MODE_OFF;
	resultMetadata->addEntry(ANDROID_COLOR_CORRECTION_ABERRATION_MODE,
				 value);

	int32_t value32 = 0;
	resultMetadata->addEntry(ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION,
				 value32);

	const CameraMetadata &settings = camera3Request->settings_;

	if (settings.getEntry(ANDROID_CONTROL_AE_LOCK, &entry)) {
		value = *entry.data.u8;
		resultMetadata->addEntry(ANDROID_CONTROL_AE_LOCK, value);
	}

	if (settings.getEntry(ANDROID_CONTROL_AE_MODE, &entry)) {
		value = *entry.data.u8;
		resultMetadata->addEntry(ANDROID_CONTROL_AE_MODE, value);
	} else {
		resultMetadata->addEntry(ANDROID_CONTROL_AE_MODE, ANDROID_CONTROL_AE_MODE_ON);
	}

	if (settings.getEntry(ANDROID_CONTROL_AE_ANTIBANDING_MODE, &entry)) {
		value = *entry.data.u8;
		resultMetadata->addEntry(ANDROID_CONTROL_AE_ANTIBANDING_MODE, value);
	} else {
		resultMetadata->addEntry(ANDROID_CONTROL_AE_ANTIBANDING_MODE, ANDROID_CONTROL_AE_ANTIBANDING_MODE_OFF);
	}

	if (settings.getEntry(ANDROID_BLACK_LEVEL_LOCK, &entry)) {
		value = *entry.data.u8;
		resultMetadata->addEntry(ANDROID_BLACK_LEVEL_LOCK, value);
	} else {
		resultMetadata->addEntry(ANDROID_BLACK_LEVEL_LOCK, ANDROID_BLACK_LEVEL_LOCK_OFF);
	}

	found = settings.getEntry(ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER, &entry);
	value = found ? *entry.data.u8 : (uint8_t)ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE;
	resultMetadata->addEntry(ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER, value);

	if (settings.getEntry(ANDROID_CONTROL_AF_MODE, &entry)) {
		resultMetadata->addEntry(ANDROID_CONTROL_AF_MODE, *entry.data.u8);
	} else {
		resultMetadata->addEntry(ANDROID_CONTROL_AF_MODE, ANDROID_CONTROL_AF_MODE_OFF);
	}

	if (settings.getEntry(ANDROID_CONTROL_AWB_MODE, &entry)) {
		resultMetadata->addEntry(ANDROID_CONTROL_AWB_MODE, *entry.data.u8);
	} else {
		value = ANDROID_CONTROL_AWB_MODE_AUTO;
		resultMetadata->addEntry(ANDROID_CONTROL_AWB_MODE, value);
	}

	if (settings.getEntry(ANDROID_CONTROL_AWB_LOCK, &entry)) {
		resultMetadata->addEntry(ANDROID_CONTROL_AWB_LOCK, *entry.data.u8);
	} else {
		value = ANDROID_CONTROL_AWB_LOCK_OFF;
		resultMetadata->addEntry(ANDROID_CONTROL_AWB_LOCK, value);
	}

	value = ANDROID_CONTROL_CAPTURE_INTENT_PREVIEW;
	resultMetadata->addEntry(ANDROID_CONTROL_CAPTURE_INTENT, value);

	value = ANDROID_CONTROL_EFFECT_MODE_OFF;
	resultMetadata->addEntry(ANDROID_CONTROL_EFFECT_MODE, value);

	if (settings.getEntry(ANDROID_CONTROL_MODE, &entry)) {
		resultMetadata->addEntry(ANDROID_CONTROL_MODE, *entry.data.u8);
	} else {
		value = ANDROID_CONTROL_MODE_AUTO;
		resultMetadata->addEntry(ANDROID_CONTROL_MODE, value);
	}

	if (settings.getEntry(ANDROID_CONTROL_SCENE_MODE, &entry)) {
		resultMetadata->addEntry(ANDROID_CONTROL_SCENE_MODE, *entry.data.u8);
	} else {
		value = ANDROID_CONTROL_SCENE_MODE_DISABLED;
		resultMetadata->addEntry(ANDROID_CONTROL_SCENE_MODE, value);
	}

	value = ANDROID_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
	resultMetadata->addEntry(ANDROID_CONTROL_VIDEO_STABILIZATION_MODE, value);

	value = ANDROID_FLASH_MODE_OFF;
	resultMetadata->addEntry(ANDROID_FLASH_MODE, value);

	value = ANDROID_FLASH_STATE_UNAVAILABLE;
	resultMetadata->addEntry(ANDROID_FLASH_STATE, value);

	if (settings.getEntry(ANDROID_LENS_APERTURE, &entry))
		resultMetadata->addEntry(ANDROID_LENS_APERTURE, entry.data.f, 1);

	value = ANDROID_LENS_OPTICAL_STABILIZATION_MODE_OFF;
	resultMetadata->addEntry(ANDROID_LENS_OPTICAL_STABILIZATION_MODE,
				 value);

	settings.getEntry(ANDROID_STATISTICS_FACE_DETECT_MODE, &entry);
	resultMetadata->addEntry(ANDROID_STATISTICS_FACE_DETECT_MODE,
				 entry.data.u8, 1);

	value = ANDROID_STATISTICS_LENS_SHADING_MAP_MODE_OFF;
	resultMetadata->addEntry(ANDROID_STATISTICS_LENS_SHADING_MAP_MODE,
				 value);

	value = ANDROID_STATISTICS_HOT_PIXEL_MAP_MODE_OFF;
	resultMetadata->addEntry(ANDROID_STATISTICS_HOT_PIXEL_MAP_MODE, value);

	value = ANDROID_STATISTICS_SCENE_FLICKER_NONE;
	resultMetadata->addEntry(ANDROID_STATISTICS_SCENE_FLICKER, value);

	if (settings.getEntry(ANDROID_NOISE_REDUCTION_MODE, &entry)) {
		resultMetadata->addEntry(ANDROID_NOISE_REDUCTION_MODE, *entry.data.u8);
	} else {
		value = ANDROID_NOISE_REDUCTION_MODE_OFF;
		resultMetadata->addEntry(ANDROID_NOISE_REDUCTION_MODE, value);
	}

	/* 33.3 msec */
	const int64_t rolling_shutter_skew = 33300000;
	resultMetadata->addEntry(ANDROID_SENSOR_ROLLING_SHUTTER_SKEW,
				 rolling_shutter_skew);

	// Support FULL mode metadata
	if (settings.getEntry(ANDROID_COLOR_CORRECTION_MODE, &entry)) {
		resultMetadata->addEntry(ANDROID_COLOR_CORRECTION_MODE, *entry.data.u8);
	}

	if (settings.getEntry(ANDROID_COLOR_CORRECTION_TRANSFORM, &entry)) {
		resultMetadata->addEntry(ANDROID_COLOR_CORRECTION_TRANSFORM, entry.data.r, entry.count);
	}

	if (settings.getEntry(ANDROID_COLOR_CORRECTION_GAINS, &entry)) {
		resultMetadata->addEntry(ANDROID_COLOR_CORRECTION_GAINS, entry.data.f, entry.count);
	}

	if (settings.getEntry(ANDROID_EDGE_MODE, &entry)) {
		resultMetadata->addEntry(ANDROID_EDGE_MODE, *entry.data.u8);
	}

	if (settings.getEntry(ANDROID_HOT_PIXEL_MODE, &entry)) {
		resultMetadata->addEntry(ANDROID_HOT_PIXEL_MODE, *entry.data.u8);
	}

	if (settings.getEntry(ANDROID_LENS_FILTER_DENSITY, &entry)) {
		resultMetadata->addEntry(ANDROID_LENS_FILTER_DENSITY, entry.data.f, 1);
	}

	if (settings.getEntry(ANDROID_LENS_FOCUS_RANGE, &entry)) {
		resultMetadata->addEntry(ANDROID_LENS_FOCUS_RANGE, entry.data.f, entry.count);
	}

	if (settings.getEntry(ANDROID_SHADING_MODE, &entry)) {
		resultMetadata->addEntry(ANDROID_SHADING_MODE, *entry.data.u8);
	}

	if (settings.getEntry(ANDROID_TONEMAP_GAMMA, &entry)) {
		resultMetadata->addEntry(ANDROID_TONEMAP_GAMMA, *entry.data.f);
	}

	if (settings.getEntry(ANDROID_TONEMAP_PRESET_CURVE, &entry)) {
		resultMetadata->addEntry(ANDROID_TONEMAP_PRESET_CURVE, *entry.data.u8);
	}

	if (settings.getEntry(ANDROID_TONEMAP_MODE, &entry)) {
		resultMetadata->addEntry(ANDROID_TONEMAP_MODE, *entry.data.u8);
	}

	if (settings.getEntry(ANDROID_TONEMAP_CURVE_RED, &entry)) {
		resultMetadata->addEntry(ANDROID_TONEMAP_CURVE_RED, entry.data.f, entry.count);
	}

	if (settings.getEntry(ANDROID_TONEMAP_CURVE_GREEN, &entry)) {
		resultMetadata->addEntry(ANDROID_TONEMAP_CURVE_GREEN, entry.data.f, entry.count);
	}

	if (settings.getEntry(ANDROID_TONEMAP_CURVE_BLUE, &entry)) {
		resultMetadata->addEntry(ANDROID_TONEMAP_CURVE_BLUE, entry.data.f, entry.count);
	}

	if (settings.getEntry(ANDROID_CONTROL_AE_TARGET_FPS_RANGE, &entry)) {
		resultMetadata->addEntry(ANDROID_CONTROL_AE_TARGET_FPS_RANGE, entry.data.i32, 2);
	}

	if (settings.getEntry(ANDROID_CONTROL_AF_TRIGGER, &entry)) {
		resultMetadata->addEntry(ANDROID_CONTROL_AF_TRIGGER, *entry.data.u8);
	} else {
		resultMetadata->addEntry(ANDROID_CONTROL_AF_TRIGGER, ANDROID_CONTROL_AF_TRIGGER_IDLE);
	}

	if (settings.getEntry(ANDROID_CONTROL_AF_REGIONS, &entry)) {
		resultMetadata->addEntry(ANDROID_CONTROL_AF_REGIONS, entry.data.i32, entry.count);
	}

	if (settings.getEntry(ANDROID_LENS_FOCAL_LENGTH, &entry)) {
		resultMetadata->addEntry(ANDROID_LENS_FOCAL_LENGTH, *entry.data.f);
	} else {
		float lensFocalLength = 1.0f;
		resultMetadata->addEntry(ANDROID_LENS_FOCAL_LENGTH, lensFocalLength);
	}

	// Todo, update this with real lens shading map from calbration data
	if (settings.getEntry(ANDROID_STATISTICS_LENS_SHADING_MAP_MODE, &entry) && *entry.data.u8) {
		std::vector<float> lensShadingMap(4 * 17 * 17, 1.0f);
		resultMetadata->addEntry(ANDROID_STATISTICS_LENS_SHADING_MAP, lensShadingMap);
	}

	/*
	 * Return the result metadata pack even is not valid: get() will return
	 * nullptr.
	 */
	if (!resultMetadata->isValid()) {
		LOG(HAL, Error) << "Failed to construct result metadata";
	}

	if (resultMetadata->resized()) {
		auto [entryCount, dataCount] = resultMetadata->usage();
		LOG(HAL, Info)
			<< "Result metadata resized: " << entryCount
			<< " entries and " << dataCount << " bytes used";
	}

	return resultMetadata;
}

void CameraDevice::cameraDisconnected()
{
	notifyError(0, nullptr, CAMERA3_MSG_ERROR_DEVICE);
}

void CameraDevice::queryManufacturerInfo()
{
#ifdef HAVE_ANDROID_OS
	char model[255] = { '\0' };
	property_get("ro.product.model", model, defaultModel_.c_str());
	model_ = model;

	char maker[255] = { '\0' };
	property_get("ro.product.manufacturer", maker, defaultMaker_.c_str());
	maker_ = maker;
#else
	/* \todo Support getting properties on Android */
	std::ifstream fstream("/var/cache/camera/camera.prop");
	if (!fstream.is_open())
		return;

	std::string line;
	while (std::getline(fstream, line)) {
		std::string::size_type delimPos = line.find("=");
		if (delimPos == std::string::npos)
			continue;
		std::string key = line.substr(0, delimPos);
		std::string val = line.substr(delimPos + 1);

		if (!key.compare("ro.product.model"))
			model_ = val;
		else if (!key.compare("ro.product.manufacturer"))
			maker_ = val;
	}
#endif
}

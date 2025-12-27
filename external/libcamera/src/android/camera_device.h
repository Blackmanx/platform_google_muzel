/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * camera_device.h - libcamera Android Camera Device
 */

#pragma once

#include <map>
#include <memory>
#include <optional>
#include <vector>

#include <libcamera/base/class.h>
#include <libcamera/base/log.h>
#include <libcamera/base/message.h>
#include <libcamera/base/mutex.h>

#include <libcamera/camera.h>
#include <libcamera/controls.h>
#include <libcamera/framebuffer.h>
#include <libcamera/geometry.h>
#include <libcamera/pixel_format.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>

#include <hardware/camera3.h>
#include <queue>

#include "jpeg/encoder.h"

#include "camera_capabilities.h"
#include "camera_metadata.h"
#include "camera_stream.h"
#include "hal_framebuffer.h"
#ifdef HAVE_ANDROID_OS
#include "mm/BufferAdapter.h"
#endif

class Camera3RequestDescriptor;
struct CameraConfigData;

class CameraDevice : protected libcamera::Loggable
{
public:
	static std::unique_ptr<CameraDevice> create(unsigned int id,
						    std::shared_ptr<libcamera::Camera> cam);
	~CameraDevice();

	int initialize(const CameraConfigData *cameraConfigData);

	int open(const hw_module_t *hardwareModule);
	void close();
	void flushAndStop();

	unsigned int id() const { return id_; }
	camera3_device_t *camera3Device() { return &camera3Device_; }
	const CameraCapabilities *capabilities() const { return &capabilities_; }
	const std::shared_ptr<libcamera::Camera> &camera() const { return camera_; }

	const std::string &maker() const
	{
		return maker_.has_value() ? *maker_ : defaultMaker_;
	}

	const std::string &model() const
	{
		return model_.has_value() ? *model_ : defaultModel_;
	}

	int facing() const { return facing_; }
	int orientation() const { return orientation_; }
	int resourceCost() const { return resourceCost_; }
	unsigned int maxJpegBufferSize() const;

	void setCallbacks(const camera3_callback_ops_t *callbacks);
	const camera_metadata_t *getStaticMetadata();
	const camera_metadata_t *constructDefaultRequestSettings(int type);
	int configureStreams(camera3_stream_configuration_t *stream_list);
	int processCaptureRequest(camera3_capture_request_t *request);

	void bufferComplete(libcamera::Request *request, libcamera::FrameBuffer *frameBuffer);
	void metadataAvailable(libcamera::Request *request, const libcamera::ControlList &metadata);
	void requestComplete(libcamera::Request *request);
	void streamProcessingCompleteDelegate(StreamBuffer *bufferStream,
					      StreamBuffer::Status status);
	void streamProcessingComplete(StreamBuffer *bufferStream,
				      StreamBuffer::Status status);

protected:
	std::string logPrefix() const override;

private:
	LIBCAMERA_DISABLE_COPY_AND_MOVE(CameraDevice)

	CameraDevice(unsigned int id, std::shared_ptr<libcamera::Camera> camera);

	enum class State {
		Stopped,
		Flushing,
		Running,
	};

	std::unique_ptr<HALFrameBuffer>
	createFrameBuffer(const buffer_handle_t camera3buffer,
			  libcamera::PixelFormat pixelFormat,
			  const libcamera::Size &size);
	void abortRequest(Camera3RequestDescriptor *descriptor);
	bool isValidRequest(camera3_capture_request_t *request) const;
	void notifyShutter(uint32_t frameNumber, uint64_t timestamp);
	void notifyError(uint32_t frameNumber, camera3_stream_t *stream,
			 camera3_error_msg_code code) const;
	int processControls(Camera3RequestDescriptor *descriptor);

	void checkAndCompleteReadyPartialResults(Camera3ResultDescriptor *result);
	void completePartialResultDescriptor(Camera3ResultDescriptor *result);
	void completeRequestDescriptor(Camera3RequestDescriptor *descriptor);

	void sendCaptureResult(Camera3ResultDescriptor *partialResult) const;
	void setBufferStatus(StreamBuffer &buffer, StreamBuffer::Status status) const;
	void generateJpegExifMetadata(Camera3RequestDescriptor *request,
				      StreamBuffer *buffer) const;

	std::unique_ptr<CameraMetadata> getJpegPartialResultMetadata() const;
	std::unique_ptr<CameraMetadata> getPartialResultMetadata(
		const libcamera::ControlList &metadata) const;
	std::unique_ptr<CameraMetadata> getFinalResultMetadata(
		Camera3RequestDescriptor *camera3Request,
		const libcamera::ControlList &metadata) const;

	void cameraDisconnected();
	void queryManufacturerInfo();

	unsigned int id_;
	camera3_device_t camera3Device_;
#ifdef HAVE_ANDROID_OS
	std::shared_ptr<android::BufferAdapter> mBufferAdapter;
#endif

	/* Protects access to the camera state. */
	libcamera::Mutex stateMutex_;
	State state_ LIBCAMERA_TSA_GUARDED_BY(stateMutex_);

	std::shared_ptr<libcamera::Camera> camera_;
	std::unique_ptr<libcamera::CameraConfiguration> config_;
	CameraCapabilities capabilities_;

	std::map<unsigned int, std::unique_ptr<CameraMetadata>> requestTemplates_;
	const camera3_callback_ops_t *callbacks_;

	std::vector<CameraStream> streams_;

	/* Protects access to the pending requests and stream buffers. */
	libcamera::Mutex pendingRequestMutex_;
	std::list<std::unique_ptr<Camera3RequestDescriptor>> pendingRequests_
		LIBCAMERA_TSA_GUARDED_BY(pendingRequestMutex_);
	std::map<CameraStream *, std::list<StreamBuffer *>> pendingStreamBuffers_
		LIBCAMERA_TSA_GUARDED_BY(pendingRequestMutex_);

	std::list<Camera3ResultDescriptor *> pendingPartialResults_;
	libcamera::ConditionVariable pendingRequestsCv_;

	std::optional<std::string> maker_;
	std::optional<std::string> model_;

	const std::string defaultMaker_ = "libcamera";
	const std::string defaultModel_ = "cameraModel";

	int facing_;
	int orientation_;
	int resourceCost_;

	CameraMetadata lastSettings_;
	CameraMetadata sessionSettings_;

	bool opened_;
};

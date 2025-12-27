/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019-2021, Google Inc.
 *
 * camera_request.h - libcamera Android Camera Request Descriptor
 */

#pragma once

#include <map>
#include <memory>
#include <vector>

#include <libcamera/base/class.h>
#include <libcamera/base/mutex.h>
#include <libcamera/base/unique_fd.h>

#include <libcamera/camera.h>
#include <libcamera/framebuffer.h>

#include <hardware/camera3.h>

#include "camera_metadata.h"
#include "hal_framebuffer.h"
#ifdef HAVE_ANDROID_OS
#include "mm/BufferAdapter.h"
#endif

class CameraBuffer;
class CameraStream;
class Camera3ResultDescriptor;
class Camera3RequestDescriptor;

class StreamBuffer
{
public:
	enum class Status {
		Success,
		Error,
	};

	StreamBuffer(CameraStream *stream,
		     const camera3_stream_buffer_t &buffer,
		     Camera3RequestDescriptor *request);
	~StreamBuffer();

	StreamBuffer(StreamBuffer &&);
	StreamBuffer &operator=(StreamBuffer &&);

	struct JpegExifMetadata {
		int64_t sensorExposureTime;
		int32_t sensorSensitivityISO;
		float lensFocalLength;
	};

	CameraStream *stream;
	buffer_handle_t *camera3Buffer;
	std::unique_ptr<HALFrameBuffer> frameBuffer;
	libcamera::UniqueFD fence;
	Status status = Status::Success;
	const libcamera::FrameBuffer *srcBuffer = nullptr;
	std::unique_ptr<CameraBuffer> dstBuffer;
	std::optional<JpegExifMetadata> jpegExifMetadata;
	Camera3ResultDescriptor *result;
	Camera3RequestDescriptor *request;

private:
	LIBCAMERA_DISABLE_COPY(StreamBuffer)
};

class Camera3ResultDescriptor
{
public:
	Camera3ResultDescriptor(Camera3RequestDescriptor *request);
	~Camera3ResultDescriptor();

	Camera3RequestDescriptor *request_;
	uint32_t metadataPackIndex_;

	std::unique_ptr<CameraMetadata> resultMetadata_;
	std::vector<StreamBuffer *> buffers_;

	/* Keeps track of buffers waiting for post-processing. */
	std::list<StreamBuffer *> pendingBuffersToProcess_;

	bool completed_;

private:
	LIBCAMERA_DISABLE_COPY(Camera3ResultDescriptor)
};

class Camera3RequestDescriptor
{
public:
	enum class Status {
		Pending,
		Success,
		Cancelled,
	};

	Camera3RequestDescriptor(libcamera::Camera *camera,
#ifdef HAVE_ANDROID_OS
				 std::shared_ptr<android::BufferAdapter> bufferAdaptor,
#endif
				 const camera3_capture_request_t *camera3Request);
	~Camera3RequestDescriptor();

	uint32_t frameNumber_ = 0;

	std::vector<StreamBuffer> buffers_;
	CameraMetadata settings_;

	std::unique_ptr<libcamera::Request> request_;

	std::map<CameraStream *, libcamera::FrameBuffer *> internalBuffers_;

	Status status_;
	uint32_t nextPartialResultIndex_;
	std::unique_ptr<Camera3ResultDescriptor> finalResult_;
	std::vector<std::unique_ptr<Camera3ResultDescriptor>> partialResults_;

	bool completed_;

private:
	LIBCAMERA_DISABLE_COPY(Camera3RequestDescriptor)
};

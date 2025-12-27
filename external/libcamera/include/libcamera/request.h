/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * request.h - Capture request handling
 */

#pragma once

#include <map>
#include <memory>
#include <ostream>
#include <stdint.h>
#include <string>
#include <unordered_set>

#include <libcamera/base/class.h>
#include <libcamera/base/signal.h>

#include <libcamera/controls.h>
#include <libcamera/fence.h>

namespace libcamera {

class Camera;
class CameraControlValidator;
class FrameBuffer;
class Request;
class Stream;

class Result
{
public:
	Result(Request *request);
	Result(Result &&result);
	~Result();

	Request *request() const { return request_; }
	const ControlList &metadata() const { return metadata_; }

	const std::vector<FrameBuffer *> &buffers() const { return buffers_; }
	int addBuffer(FrameBuffer *buffer);

	template<typename T, typename V>
	void set(const Control<T> &ctrl, const V &value)
	{
		metadata_.set(ctrl, value);
	}

	void set(unsigned int id, const ControlValue &value);
	void merge(const ControlList &source);

	std::string toString() const;

private:
	LIBCAMERA_DISABLE_COPY(Result)

	Request *request_;
	ControlList metadata_;
	std::vector<FrameBuffer *> buffers_;
};

std::ostream &operator<<(std::ostream &out, const Result &r);

class Request : public Extensible
{
	LIBCAMERA_DECLARE_PRIVATE()

public:
	enum Status {
		RequestPending,
		RequestComplete,
		RequestCancelled,
	};

	enum ReuseFlag {
		Default = 0,
		ReuseBuffers = (1 << 0),
	};

	using BufferMap = std::map<const Stream *, FrameBuffer *>;
	using ResultList = std::vector<Result>;

	Request(Camera *camera, uint64_t cookie = 0);
	~Request();

	void reuse(ReuseFlag flags = Default);

	ControlList &controls() { return *controls_; }
	ControlList &metadata() { return *metadata_; }
	const BufferMap &buffers() const { return bufferMap_; }
	ResultList &resultList() { return results_; }
	Result *addResult(Result &&result);
	int addBuffer(const Stream *stream, FrameBuffer *buffer,
		      std::unique_ptr<Fence> fence = nullptr);
	FrameBuffer *findBuffer(const Stream *stream) const;
	const Stream *findStream(const FrameBuffer *buffer) const;

	uint32_t sequence() const;
	uint64_t cookie() const { return cookie_; }
	Status status() const { return status_; }

	bool hasPendingBuffers() const;

	std::string toString() const;

private:
	LIBCAMERA_DISABLE_COPY(Request)

	ControlList *controls_;
	ControlList *metadata_;
	BufferMap bufferMap_;
	ResultList results_;

	const uint64_t cookie_;
	Status status_;
};

std::ostream &operator<<(std::ostream &out, const Request &r);

} /* namespace libcamera */

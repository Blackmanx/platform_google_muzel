/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Google Inc.
 *
 * request.tp - Tracepoints for the request object
 */

#include <libcamera/framebuffer.h>

#include "libcamera/internal/request.h"
#include "libcamera/internal/tracepoints.h"

void LIBCAMERA_TRACE_EVENT_request(libcamera::Request *req)
{
	TRACE_EVENT("libcamera", "request",
		    "request_ptr", reinterpret_cast<uintptr_t>(req),
		    "cookie", req->cookie(),
		    "status", req->status());
}

void LIBCAMERA_TRACE_EVENT_request_construct(libcamera::Request *req)
{
	TRACE_EVENT("libcamera", "request_construct",
		    "request_ptr", reinterpret_cast<uintptr_t>(req));
}

void LIBCAMERA_TRACE_EVENT_request_destroy(libcamera::Request *req)
{
	TRACE_EVENT("libcamera", "request_destroy",
		    "request_ptr", reinterpret_cast<uintptr_t>(req));
}

void LIBCAMERA_TRACE_EVENT_request_reuse(libcamera::Request *req)
{
	TRACE_EVENT("libcamera", "request_reuse",
		    "request_ptr", reinterpret_cast<uintptr_t>(req));
}

void LIBCAMERA_TRACE_EVENT_request_queue(libcamera::Request *req)
{
	TRACE_EVENT("libcamera", "request_queue",
		    "request_ptr", reinterpret_cast<uintptr_t>(req));
}

void LIBCAMERA_TRACE_EVENT_request_device_queue(libcamera::Request *req)
{
	TRACE_EVENT("libcamera", "request_device_queue",
		    "request_ptr", reinterpret_cast<uintptr_t>(req));
}

void LIBCAMERA_TRACE_EVENT_request_complete(libcamera::Request::Private *req)
{
	TRACE_EVENT("libcamera", "request_complete",
		    "request_private_ptr", reinterpret_cast<uintptr_t>(req));
}

void LIBCAMERA_TRACE_EVENT_request_cancel(libcamera::Request::Private *req)
{
	TRACE_EVENT("libcamera", "request_cancel",
		    "request_private_ptr", reinterpret_cast<uintptr_t>(req));
}

void LIBCAMERA_TRACE_EVENT_request_complete_buffer(
	libcamera::Request::Private *req,
	libcamera::FrameBuffer *buf)
{
	TRACE_EVENT("libcamera", "request_complete_buffer",
		    "request_private_ptr", reinterpret_cast<uintptr_t>(req),
		    "cookie", req->_o<libcamera::Request>()->cookie(),
		    "status", req->_o<libcamera::Request>()->status(), // TODO
		    "buffer_ptr", reinterpret_cast<uintptr_t>(buf),
		    "buffer_status", buf->metadata().status); // TODO
}

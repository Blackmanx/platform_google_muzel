/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Google Inc.
 *
 * pipeline.tp - Tracepoints for pipelines
 */

#include "libcamera/internal/tracepoints.h"

void LIBCAMERA_TRACE_EVENT_ipa_call_begin(const char *pipe, const char *func)
{
	// TODO: Consider TRACE_EVENT_BEGIN
	TRACE_EVENT("libcamera", "ipa_call_begin",
		    "pipeline_name", pipe,
		    "function_name", func);
}

void LIBCAMERA_TRACE_EVENT_ipa_call_end(const char *pipe, const char *func)
{
	// TODO: Consider TRACE_EVENT_END
	TRACE_EVENT("libcamera", "ipa_call_end",
		    "pipeline_name", pipe,
		    "function_name", func);
}

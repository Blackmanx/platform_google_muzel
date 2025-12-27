/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2020, Google Inc.
 *
 * tracepoints.cpp - Tracepoints with lttng
 */

#if HAVE_PERFETTO

#include "libcamera/internal/tracepoints.h"

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

#else

#define TRACEPOINT_CREATE_PROBES
#define TRACEPOINT_DEFINE

#include "libcamera/internal/tracepoints.h"

#endif /* HAVE_PERFETTO */

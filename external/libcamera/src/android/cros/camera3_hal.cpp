/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2021, Google Inc.
 *
 * camera3_hal.cpp - cros-specific components of Android Camera HALv3 module
 */

#include "libcamera/internal/tracepoints.h"

#include <cros-camera/cros_camera_hal.h>

#include "../camera_hal_manager.h"
#include "../cros_mojo_token.h"

static void set_up(cros::CameraMojoChannelManagerToken *token)
{
	gCrosMojoToken = token;

	perfetto::TracingInitArgs args;
	args.backends |= perfetto::kSystemBackend;
	perfetto::Tracing::Initialize(args);
	perfetto::TrackEvent::Register();
}

static void tear_down()
{
	delete CameraHalManager::instance();
}

cros::cros_camera_hal_t CROS_CAMERA_EXPORT CROS_CAMERA_HAL_INFO_SYM = {
	.set_up = set_up,
	.tear_down = tear_down
};

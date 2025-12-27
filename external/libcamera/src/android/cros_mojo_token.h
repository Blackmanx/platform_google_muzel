/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Google Inc.
 *
 * cros_mojo_token.h - cros-specific mojo token
 */

#pragma once

#include <cros-camera/camera_mojo_channel_manager_token.h>

inline cros::CameraMojoChannelManagerToken *gCrosMojoToken = nullptr;

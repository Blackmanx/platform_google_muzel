/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * action.h - MtkISP7 on device tuner action enum.
 */
#pragma once

#include <map>
#include <string>

namespace libcamera {

enum class Action {
	Preview,
	Video,
	Capture,
};

const std::map<Action, std::string> kActionStrMap{
	{ Action::Preview, "Preview" },
	{ Action::Video, "Video" },
	{ Action::Capture, "Capture" },
};
} // namespace libcamera

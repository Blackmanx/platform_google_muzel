/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Google Inc.
 *
 * platform_utils.cpp - Platform utility library to get platform enum.
 */

#include "platform_utils.h"

namespace libcamera {

// static
PlatformUtils::MtkISP7Platform PlatformUtils::platform_ = PlatformUtils::MtkISP7Platform::NONE;
std::string PlatformUtils::model_ = "";

// static
void PlatformUtils::setWithModelName(const std::string &model)
{
	model_ = model;
	if (!model_.compare("geralt")) {
		platform_ = MtkISP7Platform::GOOGLE;
	} else if (!model_.compare("ciri")) {
		platform_ = MtkISP7Platform::LENOVO;
	}
}

// static
std::string PlatformUtils::enumToString(MtkISP7Platform platform)
{
	switch (platform) {
	case MtkISP7Platform::NONE:
		return "None";

	case MtkISP7Platform::GOOGLE:
		return "Google";

	case MtkISP7Platform::LENOVO:
		return "Lenovo";
	}
}

} // namespace libcamera

/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Google Inc.
 *
 * platform_utils.h - Platform utility library to get platform enum.
 */
#pragma once

#include <string>
namespace libcamera {

class PlatformUtils
{
public:
	enum MtkISP7Platform {
		NONE = 0,
		GOOGLE = 1,
		LENOVO = 2,
	};

	static void setWithModelName(const std::string &model);

	static std::string enumToString(MtkISP7Platform platform);

	static MtkISP7Platform platform_;
	static std::string model_;
};

} // namespace libcamera

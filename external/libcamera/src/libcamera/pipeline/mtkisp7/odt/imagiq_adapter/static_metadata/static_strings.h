/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * static_strings.h - MtkISP7 on device tuner static strings.
 */

#pragma once

#include <array>
#include <map>

#include "pipeline/mtkisp7/odt/imagiq_adapter/mtk_headers/ndd_autogen_def.h"

namespace libcamera {

//TODO(yerlandinata): yaml autogen maybe (low priority)

class StaticStrings
{
public:
	static const int kInvalidValue;

	static const std::map<NSCam::TuningUtils::eCategory, std::string> kCategoryStrMap;
	static const std::map<NSCam::TuningUtils::eModule, std::string> kModuleStrMap;

	static const std::map<std::string, NSCam::TuningUtils::eCategory> kStrCategoryMap;
	static const std::map<std::string, NSCam::TuningUtils::eModule> kStrModuleMap;

	static const std::map<NSCam::TuningUtils::eSensorId, std::string>
		kSensorNames;

	static bool formatKeyExists(const std::string &key);

	static std::string format(
		const std::string &key, const NSCam::TuningUtils::NddData &ndd);

private:
	static const std::array<std::string, 23> kFormatKeys;
	static std::string formatPlatform(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatPrefix(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatSensorId(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatStage(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatFeature(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatAction(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatUserStr(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatPadding(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatSize(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatBit(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatBayerOrder(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatSign(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatCell(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatDelay(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatLayer(const NSCam::TuningUtils::NddData &ndd);
	static std::string formatVersion(const NSCam::TuningUtils::NddData &ndd);
};

} // namespace libcamera

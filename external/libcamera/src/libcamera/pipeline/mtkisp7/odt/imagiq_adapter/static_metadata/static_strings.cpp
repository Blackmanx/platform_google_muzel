/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * static_strings.cpp - MtkISP7 on device tuner static strings.
 */

#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/static_strings.h"

#include <algorithm>
#include <array>
#include <string>

#include "pipeline/mtkisp7/odt/imagiq_adapter/mtk_headers/ndd_autogen_def.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/action.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/feature.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/stage.h"

namespace libcamera {

const int StaticStrings::kInvalidValue = -1;

const std::array<std::string, 23> StaticStrings::kFormatKeys{
	"Platform",
	"Prefix",
	"SensorId",
	"Stage",
	"Feature",
	"Action",
	"SensorFeatureMode",
	"UserStr",
	"Padding",
	"Size",
	"Bit",
	"BayerOrder",
	"Sign",
	"Cell",
	"Port",
	"Plane",
	"Format",
	"Delay",
	"Layer",
	"SensorExp",
	"ExpOdr",
	"DualCamId",
	"Version",
};

bool StaticStrings::formatKeyExists(const std::string &key)
{
	auto it = std::find(kFormatKeys.begin(), kFormatKeys.end(), key);
	return it != kFormatKeys.end();
}

std::string StaticStrings::format(
	const std::string &key, const NSCam::TuningUtils::NddData &ndd)
{
	if (key == "Platform")
		return formatPlatform(ndd);
	if (key == "Prefix")
		return formatPrefix(ndd);
	if (key == "SensorId")
		return formatSensorId(ndd);
	if (key == "Stage")
		return formatStage(ndd);
	if (key == "Feature")
		return formatFeature(ndd);
	if (key == "Action")
		return formatAction(ndd);
	if (key == "SensorFeatureMode")
		return "";
	if (key == "UserStr")
		return formatUserStr(ndd);
	if (key == "Padding")
		return formatPadding(ndd);
	if (key == "Size")
		return formatSize(ndd);
	if (key == "Bit")
		return formatBit(ndd);
	if (key == "BayerOrder")
		return formatBayerOrder(ndd);
	if (key == "Sign")
		return formatSign(ndd);
	if (key == "Cell")
		return formatCell(ndd);
	if (key == "Port")
		return "";
	if (key == "Plane")
		return "";
	if (key == "Format")
		return "";
	if (key == "Delay")
		return formatDelay(ndd);
	if (key == "Layer")
		return formatLayer(ndd);
	if (key == "SensorExp")
		return "";
	if (key == "ExpOdr")
		return "";
	if (key == "DualCamId")
		return "";
	if (key == "Version")
		return formatVersion(ndd);

	return "";
}

const std::map<NSCam::TuningUtils::eCategory, std::string>
	StaticStrings::kCategoryStrMap{
		{ NSCam::TuningUtils::eCategory::kCAPTURE, "Capture" },
		{ NSCam::TuningUtils::eCategory::kSTREAMING, "Streaming" },
	};

const std::map<std::string, NSCam::TuningUtils::eCategory>
	StaticStrings::kStrCategoryMap =
		[] {
			std::map<std::string, NSCam::TuningUtils::eCategory> result;
			for (const auto &[category, name] : kCategoryStrMap) {
				result[name] = category;
			}
			return result;
		}();

const std::map<NSCam::TuningUtils::eModule, std::string>
	StaticStrings::kModuleStrMap{
		{ NSCam::TuningUtils::eModule::kAAHO,
		  "AAHO" },
		{ NSCam::TuningUtils::eModule::kAAO,
		  "AAO" },
		{ NSCam::TuningUtils::eModule::kACTSO,
		  "ACTSO" },
		{ NSCam::TuningUtils::eModule::kAE_OUT,
		  "AE_OUT" },
		{ NSCam::TuningUtils::eModule::kAFO,
		  "AFO" },
		{ NSCam::TuningUtils::eModule::kAI_DEPTH,
		  "AI_DEPTH" },
		{ NSCam::TuningUtils::eModule::kAI_DEPTH_CFM,
		  "AI_DEPTH_CFM" },
		{ NSCam::TuningUtils::eModule::kAI_DEPTH_PRE_WPE,
		  "AI_DEPTH_PRE_WPE" },
		{ NSCam::TuningUtils::eModule::kAIFA,
		  "AIFA" },
		{ NSCam::TuningUtils::eModule::kAIFLASH_INFO,
		  "AIFLASH_INFO" },
		{ NSCam::TuningUtils::eModule::kAIHDR_DRCCORE_AE_EVD,
		  "AIHDR_DRCCORE_AE_EVD" },
		{ NSCam::TuningUtils::eModule::kAIHDR_DRCCORE_GTM_LV,
		  "AIHDR_DRCCORE_GTM_LV" },
		{ NSCam::TuningUtils::eModule::kAIHDR_DRCCORE_IN_APPLY_RAWIN,
		  "AIHDR_DRCCORE_IN_APPLY_RAWIN" },
		{ NSCam::TuningUtils::eModule::kAIHDR_DRCCORE_IN_FACEINFO,
		  "AIHDR_DRCCORE_IN_FACEINFO" },
		{ NSCam::TuningUtils::eModule::kAIHDR_DRCCORE_IN_NVRAM,
		  "AIHDR_DRCCORE_IN_NVRAM" },
		{ NSCam::TuningUtils::eModule::kAIHDR_DRCCORE_IN_STAT_RAWIN,
		  "AIHDR_DRCCORE_IN_STAT_RAWIN" },
		{ NSCam::TuningUtils::eModule::kAIHDR_DRCCORE_IN_STRUCT,
		  "AIHDR_DRCCORE_IN_STRUCT" },
		{ NSCam::TuningUtils::eModule::kAIHDR_DRCCORE_LIN_TSF,
		  "AIHDR_DRCCORE_LIN_TSF" },
		{ NSCam::TuningUtils::eModule::kAIHDR_DRCCORE_OUT_APPLY_GTMT,
		  "AIHDR_DRCCORE_OUT_APPLY_GTMT" },
		{ NSCam::TuningUtils::eModule::kAIHDR_DRCCORE_OUT_APPLY_LTM,
		  "AIHDR_DRCCORE_OUT_APPLY_LTM" },
		{ NSCam::TuningUtils::eModule::kAIHDR_DRCCORE_OUT_APPLY_RAWOUT,
		  "AIHDR_DRCCORE_OUT_APPLY_RAWOUT" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_CONFIG,
		  "AIHDR_HDRCORE_CONFIG" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_DLA_IN,
		  "AIHDR_HDRCORE_DLA_IN" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_DLA_OUT,
		  "AIHDR_HDRCORE_DLA_OUT" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_0,
		  "AIHDR_HDRCORE_INPUT_4CH_0" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_1,
		  "AIHDR_HDRCORE_INPUT_4CH_1" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_10,
		  "AIHDR_HDRCORE_INPUT_4CH_10" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_11,
		  "AIHDR_HDRCORE_INPUT_4CH_11" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_12,
		  "AIHDR_HDRCORE_INPUT_4CH_12" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_13,
		  "AIHDR_HDRCORE_INPUT_4CH_13" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_14,
		  "AIHDR_HDRCORE_INPUT_4CH_14" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_15,
		  "AIHDR_HDRCORE_INPUT_4CH_15" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_2,
		  "AIHDR_HDRCORE_INPUT_4CH_2" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_3,
		  "AIHDR_HDRCORE_INPUT_4CH_3" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_4,
		  "AIHDR_HDRCORE_INPUT_4CH_4" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_5,
		  "AIHDR_HDRCORE_INPUT_4CH_5" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_6,
		  "AIHDR_HDRCORE_INPUT_4CH_6" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_7,
		  "AIHDR_HDRCORE_INPUT_4CH_7" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_8,
		  "AIHDR_HDRCORE_INPUT_4CH_8" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_INPUT_4CH_9,
		  "AIHDR_HDRCORE_INPUT_4CH_9" },
		{ NSCam::TuningUtils::eModule::kAIHDR_HDRCORE_OUTPUT,
		  "AIHDR_HDRCORE_OUTPUT" },
		{ NSCam::TuningUtils::eModule::kAIHDR_MODEL_NVRAM_OUT,
		  "AIHDR_MODEL_NVRAM_OUT" },
		{ NSCam::TuningUtils::eModule::kAIMA,
		  "AIMA" },
		{ NSCam::TuningUtils::eModule::kAINR_MODEL_NVRAM,
		  "AINR_MODEL_NVRAM" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_CONFIG,
		  "AINR_NRCORE_CONFIG" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_DLA_IN,
		  "AINR_NRCORE_DLA_IN" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_DLA_OUT,
		  "AINR_NRCORE_DLA_OUT" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_0,
		  "AINR_NRCORE_INPUT_4CH_0" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_1,
		  "AINR_NRCORE_INPUT_4CH_1" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_10,
		  "AINR_NRCORE_INPUT_4CH_10" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_11,
		  "AINR_NRCORE_INPUT_4CH_11" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_12,
		  "AINR_NRCORE_INPUT_4CH_12" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_13,
		  "AINR_NRCORE_INPUT_4CH_13" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_14,
		  "AINR_NRCORE_INPUT_4CH_14" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_15,
		  "AINR_NRCORE_INPUT_4CH_15" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_2,
		  "AINR_NRCORE_INPUT_4CH_2" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_3,
		  "AINR_NRCORE_INPUT_4CH_3" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_4,
		  "AINR_NRCORE_INPUT_4CH_4" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_5,
		  "AINR_NRCORE_INPUT_4CH_5" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_6,
		  "AINR_NRCORE_INPUT_4CH_6" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_7,
		  "AINR_NRCORE_INPUT_4CH_7" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_8,
		  "AINR_NRCORE_INPUT_4CH_8" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_INPUT_4CH_9,
		  "AINR_NRCORE_INPUT_4CH_9" },
		{ NSCam::TuningUtils::eModule::kAINR_NRCORE_OUTPUT,
		  "AINR_NRCORE_OUTPUT" },
		{ NSCam::TuningUtils::eModule::kAINR_TUNING_BIN,
		  "AINR_TUNING_BIN" },
		{ NSCam::TuningUtils::eModule::kAINR_UPKO,
		  "AINR_UPKO" },
		{ NSCam::TuningUtils::eModule::kAINR_WPEI,
		  "AINR_WPEI" },
		{ NSCam::TuningUtils::eModule::kAINR_WPEO,
		  "AINR_WPEO" },
		{ NSCam::TuningUtils::eModule::kAINROUTPUT,
		  "AinrOutput" },
		{ NSCam::TuningUtils::eModule::kAISA,
		  "AISA" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_CONFIG,
		  "AIShutter20_HDRCORE_CONFIG" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_DLA_IN,
		  "AIShutter20_HDRCORE_DLA_IN" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_DLA_OUT,
		  "AIShutter20_HDRCORE_DLA_OUT" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_0,
		  "AIShutter20_HDRCORE_INPUT_4CH_0" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_1,
		  "AIShutter20_HDRCORE_INPUT_4CH_1" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_10,
		  "AIShutter20_HDRCORE_INPUT_4CH_10" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_11,
		  "AIShutter20_HDRCORE_INPUT_4CH_11" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_12,
		  "AIShutter20_HDRCORE_INPUT_4CH_12" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_13,
		  "AIShutter20_HDRCORE_INPUT_4CH_13" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_14,
		  "AIShutter20_HDRCORE_INPUT_4CH_14" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_15,
		  "AIShutter20_HDRCORE_INPUT_4CH_15" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_2,
		  "AIShutter20_HDRCORE_INPUT_4CH_2" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_3,
		  "AIShutter20_HDRCORE_INPUT_4CH_3" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_4,
		  "AIShutter20_HDRCORE_INPUT_4CH_4" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_5,
		  "AIShutter20_HDRCORE_INPUT_4CH_5" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_6,
		  "AIShutter20_HDRCORE_INPUT_4CH_6" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_7,
		  "AIShutter20_HDRCORE_INPUT_4CH_7" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_8,
		  "AIShutter20_HDRCORE_INPUT_4CH_8" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_INPUT_4CH_9,
		  "AIShutter20_HDRCORE_INPUT_4CH_9" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_HDRCORE_OUTPUT,
		  "AIShutter20_HDRCORE_OUTPUT" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION1_IN_AAHO_HIST_Y_LE_MSTM,
		  "AIShutter20_ImageSelection1_IN_aaho_hist_y_le_mstm" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION1_IN_AE_INFO,
		  "AIShutter20_ImageSelection1_IN_AE_INFO" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION1_IN_IMGSELINPUT_1,
		  "AIShutter20_ImageSelection1_IN_IMGSELInput_1" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION1_IN_NVRAM,
		  "AIShutter20_ImageSelection1_IN_NVRAM" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION1_IN_STRUCTURE,
		  "AIShutter20_ImageSelection1_IN_STRUCTURE" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION1_IN_ZOOM_INFO,
		  "AIShutter20_ImageSelection1_IN_ZOOM_INFO" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION1_OUT_IMGSELOUTPUT_1,
		  "AIShutter20_ImageSelection1_OUT_IMGSELOutput_1" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION1_OUT_SHRUCTURE,
		  "AIShutter20_ImageSelection1_OUT_SHRUCTURE" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION2_IN_AAHO_HIST_Y_LE_MSTM,
		  "AIShutter20_ImageSelection2_IN_aaho_hist_y_le_mstm" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION2_IN_AE_INFO,
		  "AIShutter20_ImageSelection2_IN_AE_INFO" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION2_IN_BSS_GROUP_INFO,
		  "AIShutter20_ImageSelection2_IN_BSS_GROUP_INFO" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION2_IN_IMGSELINPUT_2,
		  "AIShutter20_ImageSelection2_IN_IMGSELInput_2" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION2_IN_NVRAM,
		  "AIShutter20_ImageSelection2_IN_NVRAM" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION2_IN_ZOOM_INFO,
		  "AIShutter20_ImageSelection2_IN_ZOOM_INFO" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION2_OUT_IMGSELOUTPUT_2,
		  "AIShutter20_ImageSelection2_OUT_IMGSELOutput_2" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_IMAGESELECTION2_OUT_SHRUCTURE,
		  "AIShutter20_ImageSelection2_OUT_SHRUCTURE" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MODEL_NVRAM_OUT,
		  "AIShutter20_MODEL_NVRAM_OUT" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_IN_APPLY_RAWIN,
		  "AIShutter20_MSTMCORE_IN_APPLY_RAWIN" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_IN_BAYER,
		  "AIShutter20_MSTMCORE_IN_BAYER" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_IN_FACEINFO,
		  "AIShutter20_MSTMCORE_IN_FACEINFO" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_IN_HDR_RATIO,
		  "AIShutter20_MSTMCORE_IN_HDR_RATIO" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_IN_MSTMINPUT,
		  "AIShutter20_MSTMCORE_IN_MSTMInput" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_IN_NVRAM,
		  "AIShutter20_MSTMCORE_IN_NVRAM" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_IN_NVRAMTABLE1LIST,
		  "AIShutter20_MSTMCORE_IN_NVRAMTable1List" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_IN_NVRAMTABLE2LIST,
		  "AIShutter20_MSTMCORE_IN_NVRAMTable2List" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_IN_NVRAMTABLE3LIST,
		  "AIShutter20_MSTMCORE_IN_NVRAMTable3List" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_IN_NVRAMTABLE4LIST,
		  "AIShutter20_MSTMCORE_IN_NVRAMTable4List" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_IN_NVRAMTABLE5LIST,
		  "AIShutter20_MSTMCORE_IN_NVRAMTable5List" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_OUT_APPLY_LTM,
		  "AIShutter20_MSTMCORE_OUT_APPLY_LTM" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_OUT_APPLY_RAWOUT,
		  "AIShutter20_MSTMCORE_OUT_APPLY_RAWOUT" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_OUT_APPLY_TNC_STRUCTURE,
		  "AIShutter20_MSTMCORE_OUT_APPLY_TNC_STRUCTURE" },
		{ NSCam::TuningUtils::eModule::kAISHUTTER20_MSTMCORE_OUT_MSTMOUTPUT,
		  "AIShutter20_MSTMCORE_OUT_MSTMOutput" },
		{ NSCam::TuningUtils::eModule::kAPU_TUNING,
		  "APU_TUNING" },
		{ NSCam::TuningUtils::eModule::kBSS_ACTS_STAT,
		  "BSS_ACTS_STAT" },
		{ NSCam::TuningUtils::eModule::kBSS_DATAG,
		  "BSS_DATAG" },
		{ NSCam::TuningUtils::eModule::kBSS_FACES,
		  "BSS_FACES" },
		{ NSCam::TuningUtils::eModule::kBSS_FD_INFO,
		  "BSS_FD_INFO" },
		{ NSCam::TuningUtils::eModule::kBSS_FD_MAIN,
		  "BSS_FD_MAIN" },
		{ NSCam::TuningUtils::eModule::kBSS_GM,
		  "BSS_GM" },
		{ NSCam::TuningUtils::eModule::kBSS_IN,
		  "BSS_IN" },
		{ NSCam::TuningUtils::eModule::kBSS_INFO,
		  "BSS_INFO" },
		{ NSCam::TuningUtils::eModule::kBSS_OUT,
		  "BSS_OUT" },
		{ NSCam::TuningUtils::eModule::kBSS_PARA_BIN,
		  "BSS_PARA_BIN" },
		{ NSCam::TuningUtils::eModule::kBSS_PARAM,
		  "BSS_PARAM" },
		{ NSCam::TuningUtils::eModule::kBSS_POS,
		  "BSS_POS" },
		{ NSCam::TuningUtils::eModule::kBSS_TUNING,
		  "BSS_TUNING" },
		{ NSCam::TuningUtils::eModule::kCAC,
		  "CAC" },
		{ NSCam::TuningUtils::eModule::kCAMSV_IMGO,
		  "CAMSV_IMGO" },
		{ NSCam::TuningUtils::eModule::kCONF_MAP,
		  "CONF_MAP" },
		{ NSCam::TuningUtils::eModule::kDBGBO_T1,
		  "DBGBO_T1" },
		{ NSCam::TuningUtils::eModule::kDBGO_T1,
		  "DBGO_T1" },
		{ NSCam::TuningUtils::eModule::kDEBUGPORT,
		  "DebugPort" },
		{ NSCam::TuningUtils::eModule::kDEPI,
		  "DEPI" },
		{ NSCam::TuningUtils::eModule::kDHZAI,
		  "DHZAI" },
		{ NSCam::TuningUtils::eModule::kDHZAO,
		  "DHZAO" },
		{ NSCam::TuningUtils::eModule::kDHZDI,
		  "DHZDI" },
		{ NSCam::TuningUtils::eModule::kDHZFEBI,
		  "DHZFEBI" },
		{ NSCam::TuningUtils::eModule::kDHZFEI,
		  "DHZFEI" },
		{ NSCam::TuningUtils::eModule::kDHZGI,
		  "DHZGI" },
		{ NSCam::TuningUtils::eModule::kDHZGO,
		  "DHZGO" },
		{ NSCam::TuningUtils::eModule::kDHZPAI,
		  "DHZPAI" },
		{ NSCam::TuningUtils::eModule::kDHZPGI,
		  "DHZPGI" },
		{ NSCam::TuningUtils::eModule::kDMGI,
		  "DMGI" },
		{ NSCam::TuningUtils::eModule::kDPEI_IMG_M,
		  "DPEI_IMG_M" },
		{ NSCam::TuningUtils::eModule::kDPEI_IMG_S,
		  "DPEI_IMG_S" },
		{ NSCam::TuningUtils::eModule::kDPEI_MASK_M,
		  "DPEI_MASK_M" },
		{ NSCam::TuningUtils::eModule::kDPEI_MASK_S,
		  "DPEI_MASK_S" },
		{ NSCam::TuningUtils::eModule::kDRZS4NO_R3,
		  "DRZS4NO_R3" },
		{ NSCam::TuningUtils::eModule::kDVP_ASF_ARM,
		  "DVP_ASF_ARM" },
		{ NSCam::TuningUtils::eModule::kDVP_ASF_HF,
		  "DVP_ASF_HF" },
		{ NSCam::TuningUtils::eModule::kDVP_ASF_R,
		  "DVP_ASF_R" },
		{ NSCam::TuningUtils::eModule::kDVP_DEPTH,
		  "DVP_DEPTH" },
		{ NSCam::TuningUtils::eModule::kDVS_CFM,
		  "DVS_CFM" },
		{ NSCam::TuningUtils::eModule::kDVS_DEPTH,
		  "DVS_DEPTH" },
		{ NSCam::TuningUtils::eModule::kDVS_DV,
		  "DVS_DV" },
		{ NSCam::TuningUtils::eModule::kFEFM_DEC_OUT,
		  "FEFM_DEC_OUT" },
		{ NSCam::TuningUtils::eModule::kFEO,
		  "FEO" },
		{ NSCam::TuningUtils::eModule::kFEO_D1,
		  "FEO_D1" },
		{ NSCam::TuningUtils::eModule::kFMB_L0,
		  "FMB_L0" },
		{ NSCam::TuningUtils::eModule::kFMB_L1,
		  "FMB_L1" },
		{ NSCam::TuningUtils::eModule::kFMB_L1_M0,
		  "FMB_L1_M0" },
		{ NSCam::TuningUtils::eModule::kFMO,
		  "FMO" },
		{ NSCam::TuningUtils::eModule::kFPRI_R1,
		  "FPRI_R1" },
		{ NSCam::TuningUtils::eModule::kFST,
		  "FST" },
		{ NSCam::TuningUtils::eModule::kFW_ME_TCY_O,
		  "FW_ME_TCY_O" },
		{ NSCam::TuningUtils::eModule::kFW_ME_TCY_P,
		  "FW_ME_TCY_P" },
		{ NSCam::TuningUtils::eModule::kFWDHZ_LEVEL,
		  "FWDHZ_LEVEL" },
		{ NSCam::TuningUtils::eModule::kFWDHZ_LEVEL_PREV,
		  "FWDHZ_LEVEL_PREV" },
		{ NSCam::TuningUtils::eModule::kFWDHZ_MAP,
		  "FWDHZ_MAP" },
		{ NSCam::TuningUtils::eModule::kFWDHZ_MAP_PREV,
		  "FWDHZ_MAP_PREV" },
		{ NSCam::TuningUtils::eModule::kFWME_FW_FST,
		  "FWME_FW_FST" },
		{ NSCam::TuningUtils::eModule::kFWME_FW_FST_P,
		  "FWME_FW_FST_P" },
		{ NSCam::TuningUtils::eModule::kFWME_HW_FST_1PASS,
		  "FWME_HW_FST_1PASS" },
		{ NSCam::TuningUtils::eModule::kFWME_HW_FST_MD0,
		  "FWME_HW_FST_MD0" },
		{ NSCam::TuningUtils::eModule::kFWME_HW_FST_MD0_3PASS_0,
		  "FWME_HW_FST_MD0_3PASS_0" },
		{ NSCam::TuningUtils::eModule::kFWME_HW_FST_MD1_3PASS_0,
		  "FWME_HW_FST_MD1_3PASS_0" },
		{ NSCam::TuningUtils::eModule::kFWME_MMG_FBFST,
		  "FWME_MMG_FBFST" },
		{ NSCam::TuningUtils::eModule::kFWME_MMG_RST,
		  "FWME_MMG_RST" },
		{ NSCam::TuningUtils::eModule::kFWME_TCY_FST,
		  "FWME_TCY_FST" },
		{ NSCam::TuningUtils::eModule::kFWME_WPE_FST,
		  "FWME_WPE_FST" },
		{ NSCam::TuningUtils::eModule::kFWMM_FBFST,
		  "FWMM_FBFST" },
		{ NSCam::TuningUtils::eModule::kFWMM_GYRO_MV,
		  "FWMM_GYRO_MV" },
		{ NSCam::TuningUtils::eModule::kFWMM_HW_FMB,
		  "FWMM_HW_FMB" },
		{ NSCam::TuningUtils::eModule::kFWMM_HW_FST,
		  "FWMM_HW_FST" },
		{ NSCam::TuningUtils::eModule::kFWMM_MIL,
		  "FWMM_MIL" },
		{ NSCam::TuningUtils::eModule::kFWMM_RST,
		  "FWMM_RST" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_BCE_CURV_F0,
		  "FWTNC_DHZ_BCE_CURV_F0" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_BCE_CURV_SL,
		  "FWTNC_DHZ_BCE_CURV_SL" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_BCE_FD,
		  "FWTNC_DHZ_BCE_FD" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_BCE_FLTCURV,
		  "FWTNC_DHZ_BCE_FLTCURV" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_BCE_FLTCURV_PREV,
		  "FWTNC_DHZ_BCE_FLTCURV_PREV" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_CROP,
		  "FWTNC_DHZ_CROP" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_GGM_CURV,
		  "FWTNC_DHZ_GGM_CURV" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_GTM_AE,
		  "FWTNC_DHZ_GTM_AE" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_GTM_CURV_PREV,
		  "FWTNC_DHZ_GTM_CURV_PREV" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_IGGM_CURV,
		  "FWTNC_DHZ_IGGM_CURV" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_LEVEL,
		  "FWTNC_DHZ_LEVEL" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_MAP,
		  "FWTNC_DHZ_MAP" },
		{ NSCam::TuningUtils::eModule::kGF_BLUR,
		  "GF_BLUR" },
		{ NSCam::TuningUtils::eModule::kGF_BLUR_BOK,
		  "GF_BLUR_BOK" },
		{ NSCam::TuningUtils::eModule::kGF_NVRAM,
		  "GF_NVRAM" },
		{ NSCam::TuningUtils::eModule::kGF_RESULT_INFO,
		  "GF_RESULT_INFO" },
		{ NSCam::TuningUtils::eModule::kGRIDMAP,
		  "GridMap" },
		{ NSCam::TuningUtils::eModule::kGUIDE_IMG,
		  "GUIDE_IMG" },
		{ NSCam::TuningUtils::eModule::kGUIDE_IMG_C,
		  "GUIDE_IMG_C" },
		{ NSCam::TuningUtils::eModule::kGUIDE_IMG_PRE_WPE,
		  "GUIDE_IMG_PRE_WPE" },
		{ NSCam::TuningUtils::eModule::kGUIDE_IMG_Y,
		  "GUIDE_IMG_Y" },
		{ NSCam::TuningUtils::eModule::kHOMO_IN,
		  "HOMO_IN" },
		{ NSCam::TuningUtils::eModule::kHOMO_OUT,
		  "HOMO_OUT" },
		{ NSCam::TuningUtils::eModule::kHOMO_PARA,
		  "HOMO_PARA" },
		{ NSCam::TuningUtils::eModule::kIMG2BO,
		  "IMG2BO" },
		{ NSCam::TuningUtils::eModule::kIMG2O,
		  "IMG2O" },
		{ NSCam::TuningUtils::eModule::kIMG3BO,
		  "IMG3BO" },
		{ NSCam::TuningUtils::eModule::kIMG3O,
		  "IMG3O" },
		{ NSCam::TuningUtils::eModule::kIMG4BO,
		  "IMG4BO" },
		{ NSCam::TuningUtils::eModule::kIMG4O,
		  "IMG4O" },
		{ NSCam::TuningUtils::eModule::kIMGBI_D1,
		  "IMGBI_D1" },
		{ NSCam::TuningUtils::eModule::kIMGBI_T1,
		  "IMGBI_T1" },
		{ NSCam::TuningUtils::eModule::kIMGCI_D1,
		  "IMGCI_D1" },
		{ NSCam::TuningUtils::eModule::kIMGCI_T1,
		  "IMGCI_T1" },
		{ NSCam::TuningUtils::eModule::kIMGDI_D1,
		  "IMGDI_D1" },
		{ NSCam::TuningUtils::eModule::kIMGI_D1,
		  "IMGI_D1" },
		{ NSCam::TuningUtils::eModule::kIMGI_T1,
		  "IMGI_T1" },
		{ NSCam::TuningUtils::eModule::kIMGO,
		  "IMGO" },
		{ NSCam::TuningUtils::eModule::kIMGO_E1,
		  "IMGO_E1" },
		{ NSCam::TuningUtils::eModule::kIMGO_E2,
		  "IMGO_E2" },
		{ NSCam::TuningUtils::eModule::kIMGO_E3,
		  "IMGO_E3" },
		{ NSCam::TuningUtils::eModule::kIMGO_PROCESS,
		  "IMGO_PROCESS" },
		{ NSCam::TuningUtils::eModule::kISPINFO,
		  "ISPINFO" },
		{ NSCam::TuningUtils::eModule::kJPG,
		  "JPG" },
		{ NSCam::TuningUtils::eModule::kLMI,
		  "LMI" },
		{ NSCam::TuningUtils::eModule::kLSC,
		  "LSC" },
		{ NSCam::TuningUtils::eModule::kLSC_IN,
		  "LSC_IN" },
		{ NSCam::TuningUtils::eModule::kLSC_OUT,
		  "LSC_OUT" },
		{ NSCam::TuningUtils::eModule::kLSC_OUTINFO,
		  "LSC_OUTINFO" },
		{ NSCam::TuningUtils::eModule::kLTM_OUT,
		  "LTM_OUT" },
		{ NSCam::TuningUtils::eModule::kLTMSO,
		  "LTMSO" },
		{ NSCam::TuningUtils::eModule::kMEI_L0,
		  "MEI_L0" },
		{ NSCam::TuningUtils::eModule::kMEI_L0_P,
		  "MEI_L0_P" },
		{ NSCam::TuningUtils::eModule::kMEI_L1,
		  "MEI_L1" },
		{ NSCam::TuningUtils::eModule::kMEI_L1_P,
		  "MEI_L1_P" },
		{ NSCam::TuningUtils::eModule::kMETA_P1,
		  "META_P1" },
		{ NSCam::TuningUtils::eModule::kMETA_P2,
		  "META_P2" },
		{ NSCam::TuningUtils::eModule::kMIL,
		  "MIL" },
		{ NSCam::TuningUtils::eModule::kMMAP_DS0_X,
		  "MMAP_DS0_X" },
		{ NSCam::TuningUtils::eModule::kMMAP_DS0_Y,
		  "MMAP_DS0_Y" },
		{ NSCam::TuningUtils::eModule::kMMAP_DS1_X,
		  "MMAP_DS1_X" },
		{ NSCam::TuningUtils::eModule::kMMAP_DS1_Y,
		  "MMAP_DS1_Y" },
		{ NSCam::TuningUtils::eModule::kMMAP_DS2_X,
		  "MMAP_DS2_X" },
		{ NSCam::TuningUtils::eModule::kMMAP_DS2_Y,
		  "MMAP_DS2_Y" },
		{ NSCam::TuningUtils::eModule::kMMAP_X,
		  "MMAP_X" },
		{ NSCam::TuningUtils::eModule::kMMAP_Y,
		  "MMAP_Y" },
		{ NSCam::TuningUtils::eModule::kMV_L0,
		  "MV_L0" },
		{ NSCam::TuningUtils::eModule::kMV_L0_M0,
		  "MV_L0_M0" },
		{ NSCam::TuningUtils::eModule::kMV_L0_M1_P,
		  "MV_L0_M1_P" },
		{ NSCam::TuningUtils::eModule::kMV_L0_M2_P,
		  "MV_L0_M2_P" },
		{ NSCam::TuningUtils::eModule::kMV_L1,
		  "MV_L1" },
		{ NSCam::TuningUtils::eModule::kMV_L1_M0_P,
		  "MV_L1_M0_P" },
		{ NSCam::TuningUtils::eModule::kMV_L1_M2_P,
		  "MV_L1_M2_P" },
		{ NSCam::TuningUtils::eModule::kMV_PACK,
		  "MV_PACK" },
		{ NSCam::TuningUtils::eModule::kMV_PACK_M0,
		  "MV_PACK_M0" },
		{ NSCam::TuningUtils::eModule::kMV_PACK_M0_P,
		  "MV_PACK_M0_P" },
		{ NSCam::TuningUtils::eModule::kMV_PACK_M1_P,
		  "MV_PACK_M1_P" },
		{ NSCam::TuningUtils::eModule::kMV_PACK_M2_P,
		  "MV_PACK_M2_P" },
		{ NSCam::TuningUtils::eModule::kN3D_INIT_INFO,
		  "N3D_INIT_INFO" },
		{ NSCam::TuningUtils::eModule::kN3D_LOG,
		  "N3D_LOG" },
		{ NSCam::TuningUtils::eModule::kN3D_NVRAM_IN,
		  "N3D_NVRAM_IN" },
		{ NSCam::TuningUtils::eModule::kN3D_NVRAM_OUT,
		  "N3D_NVRAM_OUT" },
		{ NSCam::TuningUtils::eModule::kN3D_PROC_INFO,
		  "N3D_PROC_INFO" },
		{ NSCam::TuningUtils::eModule::kP1_DEPTH,
		  "P1_DEPTH" },
		{ NSCam::TuningUtils::eModule::kP1_FE,
		  "P1_FE" },
		{ NSCam::TuningUtils::eModule::kPIMGBI_P1,
		  "PIMGBI_P1" },
		{ NSCam::TuningUtils::eModule::kPIMGCI_P1,
		  "PIMGCI_P1" },
		{ NSCam::TuningUtils::eModule::kPIMGI_P1,
		  "PIMGI_P1" },
		{ NSCam::TuningUtils::eModule::kPOST_SW,
		  "POST_SW" },
		{ NSCam::TuningUtils::eModule::kPS_MAP_X,
		  "PS_MAP_X" },
		{ NSCam::TuningUtils::eModule::kPS_MAP_Y,
		  "PS_MAP_Y" },
		{ NSCam::TuningUtils::eModule::kRAWI_R2,
		  "RAWI_R2" },
		{ NSCam::TuningUtils::eModule::kRECBI_D1,
		  "RECBI_D1" },
		{ NSCam::TuningUtils::eModule::kRECBI_D2,
		  "RECBI_D2" },
		{ NSCam::TuningUtils::eModule::kRECI_D1,
		  "RECI_D1" },
		{ NSCam::TuningUtils::eModule::kRECI_D2,
		  "RECI_D2" },
		{ NSCam::TuningUtils::eModule::kREG_DHZE,
		  "REG_DHZE" },
		{ NSCam::TuningUtils::eModule::kREG_DIP,
		  "REG_DIP" },
		{ NSCam::TuningUtils::eModule::kREG_LTRAW,
		  "REG_LTRAW" },
		{ NSCam::TuningUtils::eModule::kREG_ME,
		  "REG_ME" },
		{ NSCam::TuningUtils::eModule::kREG_P1,
		  "REG_P1" },
		{ NSCam::TuningUtils::eModule::kREG_PQDIP_A,
		  "REG_PQDIP_A" },
		{ NSCam::TuningUtils::eModule::kREG_PQDIP_B,
		  "REG_PQDIP_B" },
		{ NSCam::TuningUtils::eModule::kREG_TRAW,
		  "REG_TRAW" },
		{ NSCam::TuningUtils::eModule::kREG_WPE,
		  "REG_WPE" },
		{ NSCam::TuningUtils::eModule::kREG_WPEC,
		  "REG_WPEC" },
		{ NSCam::TuningUtils::eModule::kREG_WPET,
		  "REG_WPET" },
		{ NSCam::TuningUtils::eModule::kREG_XTRAW,
		  "REG_XTRAW" },
		{ NSCam::TuningUtils::eModule::kRZH1N2TBO_R1,
		  "RZH1N2TBO_R1" },
		{ NSCam::TuningUtils::eModule::kRZH1N2TBO_R3,
		  "RZH1N2TBO_R3" },
		{ NSCam::TuningUtils::eModule::kRZH1N2TO_R1,
		  "RZH1N2TO_R1" },
		{ NSCam::TuningUtils::eModule::kRZH1N2TO_R2,
		  "RZH1N2TO_R2" },
		{ NSCam::TuningUtils::eModule::kRZH1N2TO_R3,
		  "RZH1N2TO_R3" },
		{ NSCam::TuningUtils::eModule::kSHAD_CAL_INFO,
		  "SHAD_CAL_INFO" },
		{ NSCam::TuningUtils::eModule::kSKIP,
		  "SKIP" },
		{ NSCam::TuningUtils::eModule::kSNR_FACE_INFO,
		  "SNR_FACE_INFO" },
		{ NSCam::TuningUtils::eModule::kSTEREO_TUNING,
		  "stereo_tuning" },
		{ NSCam::TuningUtils::eModule::kSTT_PACK,
		  "STT_PACK" },
		{ NSCam::TuningUtils::eModule::kSTT_PACK_M0,
		  "STT_PACK_M0" },
		{ NSCam::TuningUtils::eModule::kSWME_CONF_MAP,
		  "SWME_CONF_MAP" },
		{ NSCam::TuningUtils::eModule::kSWME_GMV_IN,
		  "SWME_GMV_IN" },
		{ NSCam::TuningUtils::eModule::kSWME_IN,
		  "SWME_IN" },
		{ NSCam::TuningUtils::eModule::kSWME_IN_BASE,
		  "SWME_IN_BASE" },
		{ NSCam::TuningUtils::eModule::kSWME_IN_REF,
		  "SWME_IN_REF" },
		{ NSCam::TuningUtils::eModule::kSWME_OUT,
		  "SWME_OUT" },
		{ NSCam::TuningUtils::eModule::kSWME_PARA_BIN,
		  "SWME_PARA_BIN" },
		{ NSCam::TuningUtils::eModule::kSWME_PARAM,
		  "SWME_PARAM" },
		{ NSCam::TuningUtils::eModule::kSWME_TUNING,
		  "SWME_TUNING" },
		{ NSCam::TuningUtils::eModule::kSWME_WPE_X_MAP,
		  "SWME_WPE_X_MAP" },
		{ NSCam::TuningUtils::eModule::kSWME_WPE_Y_MAP,
		  "SWME_WPE_Y_MAP" },
		{ NSCam::TuningUtils::eModule::kSWME_WPEX_MAP,
		  "SWME_WPEX_MAP" },
		{ NSCam::TuningUtils::eModule::kSWME_WPEY_MAP,
		  "SWME_WPEY_MAP" },
		{ NSCam::TuningUtils::eModule::kTCCSI_P1,
		  "TCCSI_P1" },
		{ NSCam::TuningUtils::eModule::kTCCSO_P1,
		  "TCCSO_P1" },
		{ NSCam::TuningUtils::eModule::kTIMGO,
		  "TIMGO" },
		{ NSCam::TuningUtils::eModule::kTIMGO_T1,
		  "TIMGO_T1" },
		{ NSCam::TuningUtils::eModule::kTNC_OUT,
		  "TNC_OUT" },
		{ NSCam::TuningUtils::eModule::kTNCBO_D1,
		  "TNCBO_D1" },
		{ NSCam::TuningUtils::eModule::kTNCO_D1,
		  "TNCO_D1" },
		{ NSCam::TuningUtils::eModule::kTNCSBO_D1,
		  "TNCSBO_D1" },
		{ NSCam::TuningUtils::eModule::kTNCSBO_T1,
		  "TNCSBO_T1" },
		{ NSCam::TuningUtils::eModule::kTNCSHO_D1,
		  "TNCSHO_D1" },
		{ NSCam::TuningUtils::eModule::kTNCSHO_T1,
		  "TNCSHO_T1" },
		{ NSCam::TuningUtils::eModule::kTNCSO_D1,
		  "TNCSO_D1" },
		{ NSCam::TuningUtils::eModule::kTNCSO_T1,
		  "TNCSO_T1" },
		{ NSCam::TuningUtils::eModule::kTNCSYO_D1,
		  "TNCSYO_D1" },
		{ NSCam::TuningUtils::eModule::kTNCSYO_R1,
		  "TNCSYO_R1" },
		{ NSCam::TuningUtils::eModule::kTNCSYO_T1,
		  "TNCSYO_T1" },
		{ NSCam::TuningUtils::eModule::kTNRCI,
		  "TNRCI" },
		{ NSCam::TuningUtils::eModule::kTNRLI,
		  "TNRLI" },
		{ NSCam::TuningUtils::eModule::kTNRMI,
		  "TNRMI" },
		{ NSCam::TuningUtils::eModule::kTNRMO,
		  "TNRMO" },
		{ NSCam::TuningUtils::eModule::kTNRSI,
		  "TNRSI" },
		{ NSCam::TuningUtils::eModule::kTNRSO,
		  "TNRSO" },
		{ NSCam::TuningUtils::eModule::kTNRVBI,
		  "TNRVBI" },
		{ NSCam::TuningUtils::eModule::kTNRWI,
		  "TNRWI" },
		{ NSCam::TuningUtils::eModule::kTNRWO,
		  "TNRWO" },
		{ NSCam::TuningUtils::eModule::kTSFDBG_R1,
		  "TSFDBG_R1" },
		{ NSCam::TuningUtils::eModule::kTSFDBG_R2,
		  "TSFDBG_R2" },
		{ NSCam::TuningUtils::eModule::kTSFSO_R1,
		  "TSFSO_R1" },
		{ NSCam::TuningUtils::eModule::kTSFSO_R2,
		  "TSFSO_R2" },
		{ NSCam::TuningUtils::eModule::kUFDI_T1,
		  "UFDI_T1" },
		{ NSCam::TuningUtils::eModule::kUNPACKCHANNEL,
		  "UnpackChannel" },
		{ NSCam::TuningUtils::eModule::kVIPBI,
		  "VIPBI" },
		{ NSCam::TuningUtils::eModule::kVIPI,
		  "VIPI" },
		{ NSCam::TuningUtils::eModule::kWARP_RSZ_X_IN,
		  "WARP_RSZ_X_IN" },
		{ NSCam::TuningUtils::eModule::kWARP_RSZ_X_OUT,
		  "WARP_RSZ_X_OUT" },
		{ NSCam::TuningUtils::eModule::kWARP_RSZ_Y_IN,
		  "WARP_RSZ_Y_IN" },
		{ NSCam::TuningUtils::eModule::kWARP_RSZ_Y_OUT,
		  "WARP_RSZ_Y_OUT" },
		{ NSCam::TuningUtils::eModule::kWDMAO,
		  "WDMAO" },
		{ NSCam::TuningUtils::eModule::kWPE_MAP_RZ_X_IN,
		  "WPE_MAP_RZ_X_IN" },
		{ NSCam::TuningUtils::eModule::kWPE_MAP_RZ_X_OUT,
		  "WPE_MAP_RZ_X_OUT" },
		{ NSCam::TuningUtils::eModule::kWPE_MAP_RZ_Y_IN,
		  "WPE_MAP_RZ_Y_IN" },
		{ NSCam::TuningUtils::eModule::kWPE_MAP_RZ_Y_OUT,
		  "WPE_MAP_RZ_Y_OUT" },
		{ NSCam::TuningUtils::eModule::kWPE_MAP_X,
		  "WPE_MAP_X" },
		{ NSCam::TuningUtils::eModule::kWPE_MAP_Y,
		  "WPE_MAP_Y" },
		{ NSCam::TuningUtils::eModule::kWPE_MASK,
		  "WPE_MASK" },
		{ NSCam::TuningUtils::eModule::kWPEC_MAP_X,
		  "WPEC_MAP_X" },
		{ NSCam::TuningUtils::eModule::kWPEC_MAP_Y,
		  "WPEC_MAP_Y" },
		{ NSCam::TuningUtils::eModule::kWPEC_MASK,
		  "WPEC_MASK" },
		{ NSCam::TuningUtils::eModule::kWPECI,
		  "WPECI" },
		{ NSCam::TuningUtils::eModule::kWPECO,
		  "WPECO" },
		{ NSCam::TuningUtils::eModule::kWPEI,
		  "WPEI" },
		{ NSCam::TuningUtils::eModule::kWPEO,
		  "WPEO" },
		{ NSCam::TuningUtils::eModule::kWPET_MAP_X,
		  "WPET_MAP_X" },
		{ NSCam::TuningUtils::eModule::kWPET_MAP_Y,
		  "WPET_MAP_Y" },
		{ NSCam::TuningUtils::eModule::kWPET_MASK,
		  "WPET_MASK" },
		{ NSCam::TuningUtils::eModule::kWPETI,
		  "WPETI" },
		{ NSCam::TuningUtils::eModule::kWPETO,
		  "WPETO" },
		{ NSCam::TuningUtils::eModule::kWROTO,
		  "WROTO" },
		{ NSCam::TuningUtils::eModule::kXTFDBO,
		  "XTFDBO" },
		{ NSCam::TuningUtils::eModule::kXTFDO,
		  "XTFDO" },
		{ NSCam::TuningUtils::eModule::kXTMEO,
		  "XTMEO" },
		{ NSCam::TuningUtils::eModule::kYUVBO_R1,
		  "YUVBO_R1" },
		{ NSCam::TuningUtils::eModule::kYUVBO_R2,
		  "YUVBO_R2" },
		{ NSCam::TuningUtils::eModule::kYUVBO_R3,
		  "YUVBO_R3" },
		{ NSCam::TuningUtils::eModule::kYUVBO_R4,
		  "YUVBO_R4" },
		{ NSCam::TuningUtils::eModule::kYUVBO_R5,
		  "YUVBO_R5" },
		{ NSCam::TuningUtils::eModule::kYUVBO_T1,
		  "YUVBO_T1" },
		{ NSCam::TuningUtils::eModule::kYUVBO_T2,
		  "YUVBO_T2" },
		{ NSCam::TuningUtils::eModule::kYUVBO_T3,
		  "YUVBO_T3" },
		{ NSCam::TuningUtils::eModule::kYUVBO_T4,
		  "YUVBO_T4" },
		{ NSCam::TuningUtils::eModule::kYUVCO_R1,
		  "YUVCO_R1" },
		{ NSCam::TuningUtils::eModule::kYUVO_R1,
		  "YUVO_R1" },
		{ NSCam::TuningUtils::eModule::kYUVO_R2,
		  "YUVO_R2" },
		{ NSCam::TuningUtils::eModule::kYUVO_R3,
		  "YUVO_R3" },
		{ NSCam::TuningUtils::eModule::kYUVO_R4,
		  "YUVO_R4" },
		{ NSCam::TuningUtils::eModule::kYUVO_R5,
		  "YUVO_R5" },
		{ NSCam::TuningUtils::eModule::kYUVO_T1,
		  "YUVO_T1" },
		{ NSCam::TuningUtils::eModule::kYUVO_T2,
		  "YUVO_T2" },
		{ NSCam::TuningUtils::eModule::kYUVO_T3,
		  "YUVO_T3" },
		{ NSCam::TuningUtils::eModule::kYUVO_T4,
		  "YUVO_T4" },
		{ NSCam::TuningUtils::eModule::kYUVO_T5,
		  "YUVO_T5" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_GTM_STR_TBL,
		  "FWTNC_DHZ_GTM_STR_TBL" },
		{ NSCam::TuningUtils::eModule::kFWTNC_DHZ_GTM_FACE_PARA,
		  "FWTNC_DHZ_GTM_FACE_PARA" },
		{ NSCam::TuningUtils::eModule::kFSC_OUT,
		  "FSC_OUT" },
		{ NSCam::TuningUtils::eModule::kVAINR_AUXI_IN_DATA,
		  "VAINR_AUXI_IN_DATA" },
		{ NSCam::TuningUtils::eModule::kVAINR_AUXI_OUT_DATA,
		  "VAINR_AUXI_OUT_DATA" },
		{ NSCam::TuningUtils::eModule::kVAINR_PROC_IN_AUXI,
		  "VAINR_PROC_IN_AUXI" },
		{ NSCam::TuningUtils::eModule::kVAINR_PROC_IN_FRC,
		  "VAINR_PROC_IN_FRC" },
		{ NSCam::TuningUtils::eModule::kVAINR_PROC_IN_CURRENT_G,
		  "VAINR_PROC_IN_CURRENT_G" },
		{ NSCam::TuningUtils::eModule::kVAINR_PROC_IN_CURRENT_R,
		  "VAINR_PROC_IN_CURRENT_R" },
		{ NSCam::TuningUtils::eModule::kVAINR_PROC_IN_CURRENT_B,
		  "VAINR_PROC_IN_CURRENT_B" },
		{ NSCam::TuningUtils::eModule::kVAINR_PROC_IN_PREVIOUS_G,
		  "VAINR_PROC_IN_PREVIOUS_G" },
		{ NSCam::TuningUtils::eModule::kVAINR_PROC_IN_PREVIOUS_RB,
		  "VAINR_PROC_IN_PREVIOUS_RB" },
		{ NSCam::TuningUtils::eModule::kVAINR_PROC_OUT_FRC,
		  "VAINR_PROC_OUT_FRC" },
		{ NSCam::TuningUtils::eModule::kVAINR_PROC_OUT_RNN_G,
		  "VAINR_PROC_OUT_RNN_G" },
		{ NSCam::TuningUtils::eModule::kVAINR_PROC_OUT_RNN_RB,
		  "VAINR_PROC_OUT_RNN_RB" },
		{ NSCam::TuningUtils::eModule::kVAINR_PROC_OUT_ISP_G,
		  "VAINR_PROC_OUT_ISP_G" },
		{ NSCam::TuningUtils::eModule::kVAINR_PROC_OUT_ISP_R,
		  "VAINR_PROC_OUT_ISP_R" },
		{ NSCam::TuningUtils::eModule::kVAINR_PROC_OUT_ISP_B,
		  "VAINR_PROC_OUT_ISP_B" },
		{ NSCam::TuningUtils::eModule::kVAINR_APU_NVRAM,
		  "VAINR_APU_NVRAM" },
		{ NSCam::TuningUtils::eModule::kVAINR_SYS_NVRAM,
		  "VAINR_SYS_NVRAM" },
		{ NSCam::TuningUtils::eModule::kVAINR_WRK_MDLA_G,
		  "VAINR_WRK_MDLA_G" },
		{ NSCam::TuningUtils::eModule::kVAINR_WRK_MDLA_RBA,
		  "VAINR_WRK_MDLA_RBA" },
		{ NSCam::TuningUtils::eModule::kVAINR_WRK_IN_FRC,
		  "VAINR_WRK_IN_FRC" },
		{ NSCam::TuningUtils::eModule::kVAINR_WRK_OUT_FRC,
		  "VAINR_WRK_OUT_FRC" },
		{ NSCam::TuningUtils::eModule::kVAINR_WRK_ALPHA,
		  "VAINR_WRK_ALPHA" },
		{ NSCam::TuningUtils::eModule::kVAINR_WRK_RESIDUAL,
		  "VAINR_WRK_RESIDUAL" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_INITINFO,
		  "VAINR_PARAM_INITINFO" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_WRKBUFINFO,
		  "VAINR_PARAM_WRKBUFINFO" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_DETAILEXTBUFINFO,
		  "VAINR_PARAM_DETAILEXTBUFINFO" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_AUXIINPUT,
		  "VAINR_PARAM_AUXIINPUT" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_AUXIOUTPUT,
		  "VAINR_PARAM_AUXIOUTPUT" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_PROCESSINPUT,
		  "VAINR_PARAM_PROCESSINPUT" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_PROCESSOUTPUT,
		  "VAINR_PARAM_PROCESSOUTPUT" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_WPEQUERYIN,
		  "VAINR_PARAM_WPEQUERYIN" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_WPEROIINFO,
		  "VAINR_PARAM_WPEROIINFO" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_WRK_MDLA_G,
		  "VAINR_PARAM_WRK_MDLA_G" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_WRK_MDLA_RBA,
		  "VAINR_PARAM_WRK_MDLA_RBA" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_WRK_IN_FRC,
		  "VAINR_PARAM_WRK_IN_FRC" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_WRK_OUT_FRC,
		  "VAINR_PARAM_WRK_OUT_FRC" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_WRK_ALPHA,
		  "VAINR_PARAM_WRK_ALPHA" },
		{ NSCam::TuningUtils::eModule::kVAINR_PARAM_WRK_RESIDUAL,
		  "VAINR_PARAM_WRK_RESIDUAL" },
	};

const std::map<std::string, NSCam::TuningUtils::eModule>
	StaticStrings::kStrModuleMap =
		[] {
			std::map<std::string, NSCam::TuningUtils::eModule> result;
			for (const auto &[module, name] : kModuleStrMap) {
				result[name] = module;
			}
			return result;
		}();

const std::map<NSCam::TuningUtils::eSensorId, std::string>
	StaticStrings::kSensorNames{
		{ NSCam::TuningUtils::eSensorId::kMAIN, "main" },
		{ NSCam::TuningUtils::eSensorId::kSUB, "sub" },
	};

std::string StaticStrings::formatPlatform(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if (ndd.platform >= 0)
		snprintf(out_char, sizeof(out_char), "-p%d", ndd.platform);
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatPrefix(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if ((ndd.timestamp >= 0) && (ndd.requestNo >= 0) && (ndd.frameNo >= 0))
		snprintf(out_char, sizeof(out_char), "%09d-%04d-%04d", ndd.timestamp,
			 ndd.requestNo % 10000, ndd.frameNo % 10000);
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatSensorId(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if (kSensorNames.count(ndd.sensorId) > 0)
		snprintf(out_char, sizeof(out_char), "-%s",
			 kSensorNames.at(ndd.sensorId).c_str());
	else
		snprintf(out_char, sizeof(out_char), "-INVALID_SENSORID");
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatStage(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if (ndd.stage >= 0 && static_cast<size_t>(ndd.stage) < kStageStrMap.size())
		snprintf(out_char, sizeof(out_char), "-%s",
			 kStageStrMap.at(static_cast<Stage>(ndd.stage)).c_str());
	else
		snprintf(out_char, sizeof(out_char), "-INVALID_STAGE");
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatFeature(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if (ndd.feature != kInvalidValue && ndd.feature >= 0)
		snprintf(out_char, sizeof(out_char), "-%s",
			 kFeatureStrMap.at(static_cast<Feature>(ndd.feature)).c_str());
	else
		snprintf(out_char, sizeof(out_char), "-INVALID_FEATURE");
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatAction(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if (ndd.action != kInvalidValue && ndd.action >= 0)
		snprintf(out_char, sizeof(out_char), "-%s",
			 kActionStrMap.at(static_cast<Action>(ndd.action)).c_str());
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatUserStr(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if (ndd.userstring.size() != 0)
		snprintf(out_char, sizeof(out_char), "-%s", ndd.userstring.c_str());
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatPadding(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if ((ndd.pixelWidth >= 0) && (ndd.pixelHeight >= 0) &&
	    (ndd.byteWidth >= 0))
		snprintf(out_char, sizeof(out_char), "-PW%d-PH%d-BW%d", ndd.pixelWidth,
			 ndd.pixelHeight, ndd.byteWidth);
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatSize(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if ((ndd.width >= 0) && (ndd.height >= 0))
		snprintf(out_char, sizeof(out_char), "__%dx%d", ndd.width, ndd.height);
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatBit(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if (ndd.bitResultion >= 0)
		snprintf(out_char, sizeof(out_char), "_%d", ndd.bitResultion);
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatBayerOrder(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if (ndd.bayerOrder >= 0)
		snprintf(out_char, sizeof(out_char), "_%d", ndd.bayerOrder);
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatSign(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if (ndd.signedness >= 0)
		snprintf(out_char, sizeof(out_char), "_s%d", ndd.signedness);
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatCell(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if (ndd.cell >= 0)
		snprintf(out_char, sizeof(out_char), "_c%d", ndd.cell);
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatDelay(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if (ndd.delay >= 0)
		snprintf(out_char, sizeof(out_char), "_delay%d", ndd.delay);
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatLayer(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if (ndd.layer >= 0)
		snprintf(out_char, sizeof(out_char), "-F%d", ndd.layer);
	string out_str(out_char);
	return out_str;
}

std::string StaticStrings::formatVersion(const NSCam::TuningUtils::NddData &ndd)
{
	char out_char[100] = { '\0' };
	if (ndd.version >= 0)
		snprintf(out_char, sizeof(out_char), "-v%d", ndd.version);
	string out_str(out_char);
	return out_str;
}

} // namespace libcamera

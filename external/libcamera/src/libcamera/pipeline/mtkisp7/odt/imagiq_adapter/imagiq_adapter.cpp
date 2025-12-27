/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * imagiq_adapter.h - MtkISP7 OnDeviceTuner Imagiq Adapter
 */

#include "pipeline/mtkisp7/odt/imagiq_adapter/imagiq_adapter.h"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>

#include <libcamera/base/log.h>

#include <libcamera/formats.h>
#include <libcamera/pixel_format.h>

#include "libcamera/internal/formats.h"
#include "libcamera/internal/mapped_framebuffer.h"

#include "halisp/ScenarioRecorder/ScenarioRecorderDef.h"
#include "mtkcam-halif/def/BuiltinTypes.h"
#include "mtkcam-halif/def/TypeManip.h"
#include "mtkcam-halif/utils/metadata/1.x/IMetadata.h"
#include "mtkcam-interfaces/utils/ScenarioRecorder/IScenarioRecorder.h"
#include "mtkcam-interfaces/utils/debug/Properties.h"
#include "mtkcam-interfaces/utils/metadata/hal/mtk_platform_metadata_tag.h"
#include "mtkcam-interfaces/utils/ndd/INdd.h"
#include "mtkcam-interfaces/utils/ndd/ndd_autogen_def.h"
#include "mtkcam-interfaces/utils/odt/IOnDeviceTuning.h"
#include "mtkcam-interfaces/utils/std/Time.h"
#include "mtkcam-interfaces/utils/std/ULogDef.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/mtk_headers/ndd_autogen_def.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/dump_metadata.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/feature.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/stage.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/static_strings.h"
#include "platform/mtkisp7/single_device_helper.h"
#include "tuning_mapping/cam_idx_struct_ext_pub.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

namespace {

const std::filesystem::path kDumpConfigPath = "dump.cfg";
const std::filesystem::path kImportConfigPath = "dump_import.cfg";

} // namespace

std::unique_ptr<NSCam::TuningUtils::NddInitializer>
	ImagiqAdapter::mtkTuningInitializer_;

std::unique_ptr<NSCam::TuningUtils::scenariorecorder::ScenarioRecorderInitializer>
	ImagiqAdapter::mtkScenarioRecorderInitializer_;

ImagiqAdapter::SensorIdMap ImagiqAdapter::sensorIdMap;

const std::array<std::string, 2> ImagiqAdapter::kYcPlaneNames{
	"-yplane", "-cplane"
};

const std::array<std::string, 3> ImagiqAdapter::kYuvPlaneNames{
	"-yplane", "-uplane", "-vplane"
};

const std::array<std::string, 2> ImagiqAdapter::kWarpPlaneNames{
	"-mapx", "-mapy"
};

std::string ImagiqAdapter::createImportConfigId(const Dump &dump)
{
	return StaticStrings::kModuleStrMap.at(dump.metadata.moduleId) + ',' +
	       std::to_string(dump.requestNumber) + ',' +
	       kStageStrMap.at(dump.metadata.stage) + ',' +
	       std::to_string(dump.metadata.layer) + ',' +
	       std::to_string(dump.metadata.action.has_value() ? static_cast<int>(dump.metadata.action.value()) : -1);
}

void ImagiqAdapter::configureScenarioRecorder(
	int requestNumber, int frameNumber,
	int timestamp, bool highIsoMode,
	bool isStillCapture, Feature feature,
	std::string logMessage, EStage_T stage, std::string sensorId)
{
	using NSCam::TuningUtils::scenariorecorder::IScenarioRecorder;
	if (!IScenarioRecorder::getInstance()->isScenarioRecorderOn()) {
		return;
	}

	// Notify scenario recorder
	IScenarioRecorder::getInstance()->recordNddInfo(
		timestamp, requestNumber, requestNumber, isStillCapture);

	// Add scenario recorder headline entry
	NSCam::IMetadata metadata;
	NSCam::TuningUtils::INdd::getInstance()->update_uniquekey(
		metadata, timestamp, requestNumber, frameNumber);
	NSCam::TuningUtils::scenariorecorder::ExecResultInput resultParam;
	resultParam.decisionType =
		NSCam::TuningUtils::scenariorecorder::DECISION_FEATURE;

	using NSCam::TuningUtils::scenariorecorder::IScenarioRecorder;
	NSCam::TuningUtils::scenariorecorder::UserStaticInfo &staticInfo =
		resultParam.staticInfo;

	eCategory outputCategory;
	NddData outputNddData;
	uint32_t tuningFeatureTag = 0;
	int32_t mtkSensorId =
		static_cast<int32_t>(ImagiqAdapter::sensorIdMap.at(sensorId));

	std::stringstream ss;
	switch (stage) {
	case EStage_NUM:
		resultParam.writeToHeadline = true;
		ss << "trigger feature:";
		if (isStillCapture) {
			ss << kFeatureStrMap.at(feature);
			if (feature == Feature::Capture_lpnr) {
				if (highIsoMode) {
					ss << ", mode: high iso";
				} else {
					ss << ", mode: low iso";
				}
			}
			staticInfo.moduleId = NSCam::Utils::ULog::MOD_FPIPE_CAPTURE;
			tuningFeatureTag = MTK_TUNING_FEATURE_CAPTURE_HINT;
		} else {
			staticInfo.moduleId = NSCam::Utils::ULog::MOD_FPIPE_STREAMING;
			ss << kFeatureStrMap.at(Feature::Preview) << ","
			   << "camera_act:CamActPrv";
		}
		break;
	case EStage_BSS:
		staticInfo.moduleId = NSCam::Utils::ULog::MOD_CAPTURE_BSS;
		ss << logMessage;
		staticInfo.sensorId = mtkSensorId;
		resultParam.stageId = NSIspTuning::EStage_BSS;
		// hack a magicNum
		resultParam.magicNum = requestNumber;
		resultParam.writeToHeadline = false;
		break;
	default:
		break;
	}
	// In case of still capture, must notify NDD as well to
	// enable scenario recorder.
	NSCam::IMetadata::IEntry entry(tuningFeatureTag);
	entry.push_back(1, NSCam::Type2Type<MINT64>());
	metadata.update(entry.tag(), entry);
	using NSCam::TuningUtils::INdd;
	// The notification: seems like const function, but not!
	INdd::getInstance()->query_ndd_info(
		metadata, outputCategory, outputNddData);

	if (requestNumber == frameNumber) {
		IScenarioRecorder::getInstance()->submitExecutionRecord(
			&metadata, resultParam, ss.str().c_str());
	}
}

/**
 * @brief Enable tuning tool for MTK HAL3A / HALISP library
 *
 * Libcamera doesn't have the pointer to the dump data,
 * and doesn't have control to the dump export timing.
 * This function allows MTK private library to do dump
 * export/import on their own.
 * 
 * @return 0 if success
 */
int ImagiqAdapter::enableMtkTuningTool(std::filesystem::path workDir)
{
	int ret = NSCam::Utils::Properties::property_set(
		"vendor.debug.ndd.thdnum", "1");
	ret |= NSCam::Utils::Properties::property_set(
		"vendor.debug.ndd.rootpath", (workDir.string() + "/").c_str());
	ret |= NSCam::Utils::Properties::property_set(
		"vendor.debug.ndd.subdir", "1");
	ret |= NSCam::Utils::Properties::property_set(
		"vendor.debug.ndd.cfgpath", (workDir / kDumpConfigPath).c_str());
	ret |= NSCam::Utils::Properties::property_set(
		"vendor.debug.ndd.gen_cfg", "1");
	ret |= NSCam::Utils::Properties::property_set(
		"vendor.debug.ndd.prv_ready", "0");
	ret |= NSCam::Utils::Properties::property_set(
		"vendor.debug.odt.config",
		(workDir / kImportConfigPath).c_str());
	ret |= NSCam::Utils::Properties::property_set(
		"vendor.debug.odt.enable", "1");
	ret |= NSCam::Utils::Properties::property_set(
		"vendor.debug.odt.start", "0");
	ret |= NSCam::Utils::Properties::property_set(
		"vendor.debug.odt.streaming", "1");
	if (ret != 0) {
		LOG(MtkISP7, Error) << "Failed to setprop";
		return ret;
	}
	if (mtkTuningInitializer_.get() == nullptr) {
		using NSCam::TuningUtils::NddInitializer;
		mtkTuningInitializer_.reset(new NddInitializer());
	}
	if (mtkScenarioRecorderInitializer_.get() == nullptr) {
		using NSCam::TuningUtils::scenariorecorder::ScenarioRecorderInitializer;
		mtkScenarioRecorderInitializer_.reset(new ScenarioRecorderInitializer());
	}
	return 0;
}

ImagiqAdapter::ExportResult ImagiqAdapter::exportArrayDump(
	const Dump &dump, const NSCam::TuningUtils::NddData &ndd)
{
	ExportResult result{ .dump = dump };
	const std::filesystem::path exportPath =
		getDumpFileNameSingleFile(dump, ndd);
	std::ofstream exportFile(exportPath, std::ios::binary);
	if (!exportFile) {
		LOG(MtkISP7, Error) << "Failed to open export file: "
				    << exportPath;
		result.errorCode = -EIO;
		return result;
	}
	exportFile.write(
		reinterpret_cast<char *>(const_cast<uint8_t *>(dump.array->data())),
		dump.array->size());
	if (!exportFile.good()) {
		LOG(MtkISP7, Error) << "Error writing plane to file: " << exportPath;
		result.errorCode = -EIO;
		return result;
	}
	LOG(MtkISP7, Info) << "Dump id: " << static_cast<int>(dump.id)
			   << "; write file size: " << exportFile.tellp()
			   << "; File name: " << exportPath;
	result.paths = { { exportPath } };
	return result;
}

ImagiqAdapter::ExportResult ImagiqAdapter::exportFrameDump(
	const Dump &dump, const NSCam::TuningUtils::NddData &ndd)
{
	DmaSyncer syncer(dump.frame->buffer()->planes()[0].fd.get());
	const MappedFrameBuffer mappedBuffer(
		dump.frame->buffer(), MappedFrameBuffer::MapFlag::Read);

	const PixelFormat &pixelFormat = dump.frame->format();
	const std::string fileSuffix = getFileExtension(pixelFormat);
	if (shouldSplitExport(pixelFormat)) {
		return exportDumpSplitPlanes(
			dump, mappedBuffer, ndd, pixelFormat, fileSuffix);
	} else {
		return exportDumpMergePlanes(
			dump, mappedBuffer, ndd, fileSuffix);
	}
}

ImagiqAdapter::ExportResult ImagiqAdapter::exportDump(const Dump &dump)
{
	const NSCam::TuningUtils::NddData ndd(parseNdd(dump));
	if (dump.frame.has_value()) {
		return exportFrameDump(dump, ndd);
	} else {
		return exportArrayDump(dump, ndd);
	}
}

ImagiqAdapter::ExportResult ImagiqAdapter::exportDumpMergePlanes(
	const Dump &dump, const MappedFrameBuffer &mappedBuffer,
	const NSCam::TuningUtils::NddData &ndd,
	const std::string &fileSuffix)
{
	ExportResult result{ .dump = dump };
	const std::filesystem::path exportPath =
		getDumpFileNameSingleFile(dump, ndd, fileSuffix);
	std::ofstream exportFile(exportPath, std::ios::binary);
	if (!exportFile) {
		LOG(MtkISP7, Error) << "Failed to open export file: "
				    << exportPath;
		result.errorCode = -EIO;
		return result;
	}
	const PixelFormatInfo formatInfo =
		PixelFormatInfo::info(dump.frame->format());
	for (unsigned int i = 0; i < formatInfo.numPlanes(); i++) {
		exportFile.write(
			reinterpret_cast<char *>(mappedBuffer.planes()[i].data()),
			mappedBuffer.planes()[i].size());
		LOG(MtkISP7, Info) << "Dump id: " << static_cast<int>(dump.id)
				   << "; plane: " << i << "; plane size: " << mappedBuffer.planes()[i].size()
				   << "; write file size: " << exportFile.tellp()
				   << "; File name: " << exportPath;
		if (!exportFile.good()) {
			LOG(MtkISP7, Error) << "Error writing plane to file: " << exportPath;
			result.errorCode = -EIO;
			return result;
		}
	}
	result.paths = { { exportPath } };
	return result;
}

ImagiqAdapter::ExportResult ImagiqAdapter::exportDumpSplitPlanes(
	const Dump &dump, const MappedFrameBuffer &mappedBuffer,
	const NSCam::TuningUtils::NddData &ndd, const PixelFormat &pixelFormat,
	const std::string &fileSuffix)
{
	ExportResult result{ .dump = dump };
	const PixelFormatInfo formatInfo =
		PixelFormatInfo::info(dump.frame->format());
	for (size_t i = 0; i < formatInfo.numPlanes(); i++) {
		const std::filesystem::path planeExportPath =
			getDumpFileNameSplitPlanes(
				dump, ndd, i, pixelFormat, fileSuffix);
		std::ofstream outFile(planeExportPath, std::ios::binary);
		if (!outFile) {
			LOG(MtkISP7, Error) << "Failed to open dump dump file: "
					    << planeExportPath;
			result.errorCode = -EIO;
			return result;
		}
		outFile.write(
			reinterpret_cast<char *>(mappedBuffer.planes()[i].data()),
			mappedBuffer.planes()[i].size());
		if (!outFile.good()) {
			LOG(MtkISP7, Error) << "Error writing plane to file: " << planeExportPath;
			result.errorCode = -EIO;
			return result;
		}
		LOG(MtkISP7, Info) << "Dump id: " << static_cast<int>(dump.id)
				   << "; plane: " << i << "; plane size: " << mappedBuffer.planes()[i].size()
				   << "; File name: " << planeExportPath;
		if (!result.paths) {
			result.paths = std::vector<std::filesystem::path>();
		}
		result.paths->push_back(planeExportPath);
	}
	return result;
}

/**
 * @brief Copy MTK's reimport config to the session directory
 *
 * TODO: Pipeline handler and IPC process need to avoid writing at the same time.
 */
void ImagiqAdapter::flushPrivateReimportConfig(
	std::filesystem::path rootWorkPath,
	std::filesystem::path sessionWorkPath,
	int dumpSessionTimestamp)
{
	std::string timestampStr = formatTimestamp(dumpSessionTimestamp);
	std::optional<std::filesystem::path> privateCfgPath = std::nullopt;
	for (const auto &dir : std::filesystem::directory_iterator(rootWorkPath)) {
		std::string pathStr = dir.path();
		if (pathStr.find("dumpin.cfg") != std::string::npos &&
		    pathStr.find(timestampStr) != std::string::npos) {
			privateCfgPath = dir.path();
			break;
		}
	}
	if (!privateCfgPath.has_value()) {
		return;
	}
	if (std::filesystem::file_size(*privateCfgPath) == 0) {
		return;
	}
	std::ofstream sessionReimport(
		sessionWorkPath / kImportConfigPath, std::ios::app);
	std::ifstream privateReimportIn(*privateCfgPath);
	std::string line;
	while (std::getline(privateReimportIn, line)) {
		sessionReimport << line << std::endl;
	}
	privateReimportIn.close();
	std::ofstream privateReimportOut(*privateCfgPath);
}

std::string ImagiqAdapter::formatPlaneName(int planeNumber, const PixelFormat &pixelFormat)
{
	if (pixelFormat == formats::WARP2P_MTISP) {
		return kWarpPlaneNames[planeNumber];
	}

	int planeCount = PixelFormatInfo::info(pixelFormat).numPlanes();
	if (planeCount == 2) {
		return kYcPlaneNames[planeNumber];
	} else if (planeCount == 3) {
		return kYuvPlaneNames[planeNumber];
	}
	return "";
}

std::string ImagiqAdapter::formatTimestamp(int timestamp)
{
	std::ostringstream ss;
	ss << std::setw(9) << std::setfill('0') << timestamp;
	return ss.str();
}

/**
 * @brief Generate acceptable timestamp for Imagiq
 *
 * Imagiq wants a 9-digit timestamp, this function will call
 * an MTK library to do just that.
 * 
 * @return timestamp ranged [1e8, 1e9)
 */
int ImagiqAdapter::generateDumpTimestamp()
{
	return NSCam::Utils::TimeTool::getReadableTime();
}

std::string ImagiqAdapter::getDumpFileName(const Dump &dump)
{
	return getDumpFileNameSingleFile(dump, parseNdd(dump));
}

std::filesystem::path ImagiqAdapter::getDumpFileNameSingleFile(
	const Dump &dump, const NSCam::TuningUtils::NddData &ndd,
	const std::string &suffix)
{
	return getDumpFileNameSplitPlanes(dump, ndd, 1, std::nullopt, suffix);
}

std::filesystem::path ImagiqAdapter::getDumpFileNameSplitPlanes(
	const Dump &dump, const NSCam::TuningUtils::NddData &ndd,
	int planeNumber, const std::optional<PixelFormat> pixelFormat,
	const std::string &suffix)
{
	std::string fileName = "";
	for (const auto &format : dump.config.dumpFileNameFormat) {
		if (format == "Format") {
			fileName += suffix;
			continue;
		}
		if (!StaticStrings::formatKeyExists(format)) {
			fileName += format;
			continue;
		}
		if (format == "Padding" && pixelFormat.has_value()) {
			fileName += formatPlaneName(planeNumber, *pixelFormat);
		}
		fileName += StaticStrings::format(format, ndd);
	}
	std::filesystem::path filePath(fileName);
	return dump.workPath / filePath;
}

std::string ImagiqAdapter::getFileExtension(const PixelFormat &pixelFormat)
{
	switch (pixelFormat) {
	case formats::NV12:
		return "nv12";
	case formats::NV21:
		return "nv21";
	case formats::GREY:
		return "y";
	case formats::Y8_MTISP:
		return "y";
	}
	return "packed_word";
}

int ImagiqAdapter::importDump(const Dump &dump)
{
	int totalDumps = dump.config.savedDumps.size();
	if (totalDumps == 0) {
		return 0;
	}
	int importOffset = dump.requestNumber % totalDumps;
	auto it = dump.config.savedDumps.begin();
	std::advance(it, importOffset);
	auto planesPath = it->second;
	if (planesPath.size() == 0) {
		// illegal state, somehow
		return -EINVAL;
	}

	DmaSyncer syncer(dump.frame->buffer()->planes()[0].fd.get());

	MappedFrameBuffer mappedBuffer(
		dump.frame->buffer(), MappedFrameBuffer::MapFlag::ReadWrite);
	PixelFormatInfo formatInfo = PixelFormatInfo::info(dump.frame->format());

	if (formatInfo.numPlanes() == planesPath.size()) {
		for (size_t i = 0; i < formatInfo.numPlanes(); i++) {
			std::ifstream dumpFile(planesPath[i], std::ios::binary);
			std::streampos startPos = dumpFile.tellg();
			dumpFile.seekg(0, std::ios::end);
			std::streampos savedDumpSize = dumpFile.tellg();
			dumpFile.seekg(startPos, std::ios::beg);
			if (static_cast<size_t>(savedDumpSize) != mappedBuffer.planes()[i].size()) {
				LOG(MtkISP7, Error) << "Expected dump size: "
						    << mappedBuffer.planes()[i].size()
						    << " got: " << savedDumpSize
						    << ". Maybe wrong sensor / camera facing."
						    << " Not importing this dump id: "
						    << static_cast<int>(dump.id)
						    << ". Dump file: "
						    << planesPath[i]
						    << ". Plane num: " << i;
				return -EINVAL;
			}
			dumpFile.read(
				reinterpret_cast<char *>(mappedBuffer.planes()[i].data()), savedDumpSize);
			LOG(MtkISP7, Info) << "Dump " << static_cast<int>(dump.id)
					   << " plane " << i
					   << " is replaced with " << planesPath[0]
					   << " size: " << savedDumpSize;
		}
	} else if (formatInfo.numPlanes() > 1 && planesPath.size() == 1) {
		std::ifstream dumpFile(planesPath[0], std::ios::binary);
		// todo(yerlandinata): check if remaining file size is enough
		for (size_t i = 0; i < formatInfo.numPlanes(); i++) {
			dumpFile.read(
				reinterpret_cast<char *>(mappedBuffer.planes()[i].data()),
				mappedBuffer.planes()[i].size());
			// readPos += mappedBuffer.planes()[i].size();
			LOG(MtkISP7, Info) << "Dump " << static_cast<int>(dump.id)
					   << " plane " << i
					   << " is replaced with " << planesPath[0]
					   << " size: " << mappedBuffer.planes()[i].size();
		}
	} else {
		LOG(MtkISP7, Warning) << "Dump " << static_cast<int>(dump.id)
				      << " has " << formatInfo.numPlanes() << " planes "
				      << " but the saved dump file count is: "
				      << planesPath.size() << " such case is not implemented";
		return -EINVAL;
	}

	return 0;
}

int ImagiqAdapter::loadConfig(
	std::map<Dump::Id, Dump::Config> &config,
	const std::filesystem::path &workPath)
{
	int ret = loadBaseConfig(config, workPath);
	if (ret) {
		return ret;
	}
	ret = loadImportConfig(config, workPath);
	if (ret) {
		LOG(MtkISP7, Error) << "Failed to load dump import config, "
				    << "disabling import";
	}
	return 0;
}

int ImagiqAdapter::loadBaseConfig(
	std::map<Dump::Id, Dump::Config> &config,
	const std::filesystem::path &workPath)
{
	std::ifstream dumpCfgFile(workPath / kDumpConfigPath);
	if (!dumpCfgFile.is_open()) {
		LOG(MtkISP7, Error) << "Failed to open config file";
		return -EIO;
	}

	std::string line;
	while (std::getline(dumpCfgFile, line)) {
		std::stringstream ss(line);
		std::string token;
		std::vector<std::string> parsed;
		while (std::getline(ss, token, ',')) {
			parsed.push_back(token);
		}
		if (parsed.size() != 7) {
			LOG(MtkISP7, Error) << "Dump config file error, expected 7 tokens, "
					    << "but got " << parsed.size() << ": " << line;
			return -EINVAL;
		}
		std::string featureStr = parsed[0];
		std::string stageStr = parsed[1];
		std::string categoryStr = parsed[3];
		std::string moduleStr = parsed[4];

		// We (will) only register ~200 dump IDs,
		// while MTK have 20K, so most of the 20K not be mapped.

		if (kStrFeatureMap.count(featureStr) == 0) {
			LOG(MtkISP7, Debug) << "Feature not found: " << featureStr
					    << "Config: " << line;
			continue;
		}
		Feature feature = kStrFeatureMap.at(featureStr);

		if (kStrStageMap.count(stageStr) == 0) {
			LOG(MtkISP7, Debug) << "Stage not found: " << stageStr
					    << "Config: " << line;
			continue;
		}
		Stage stage = kStrStageMap.at(stageStr);

		if (StaticStrings::kStrCategoryMap.count(categoryStr) == 0) {
			LOG(MtkISP7, Debug) << "Category not found: " << categoryStr
					    << "Config: " << line;
			continue;
		}
		NSCam::TuningUtils::eCategory category = StaticStrings::kStrCategoryMap.at(categoryStr);

		if (StaticStrings::kStrModuleMap.count(moduleStr) == 0) {
			LOG(MtkISP7, Debug) << "Module not found: " << moduleStr
					    << "Config: " << line;
			continue;
		}
		NSCam::TuningUtils::eModule module = StaticStrings::kStrModuleMap.at(moduleStr);

		// Multiple dump ids will match because of different layers share
		// config from the .cfg.
		std::vector<Dump::Id> dumpIds;

		// todo(yerlandinata): ~20K config lines (don't know negotiable) and
		// next loop is ~200 steps (length of kDumpMetadata).
		// const map kDumpMetadata may grow in the future development,
		// but won't reach 1000, probably.
		for (const auto &[id, metadata] : kDumpMetadata) {
			if (metadata.featureId == feature && metadata.stage == stage &&
			    metadata.category == category && metadata.moduleId == module) {
				dumpIds.push_back(id);
			}
		}

		if (dumpIds.empty()) {
			LOG(MtkISP7, Debug) << "(" << featureStr << ","
					    << stageStr << ","
					    << categoryStr << ","
					    << moduleStr << ") not found";
			continue;
		}

		std::vector<std::string> fileNameFormat;
		// Parsing the filename format keys.
		bool bracketOpen = false;
		std::string current = "";
		for (const auto &c : parsed[2]) {
			switch (c) {
			case '[':
				if (bracketOpen) {
					LOG(MtkISP7, Error) << "Config file error, "
							    << "invalid dump filename format: " << line;
					return -EINVAL;
				}
				bracketOpen = true;
				if (!current.empty()) {
					fileNameFormat.push_back(current);
					current = "";
				}
				break;
			case ']':
				if (!bracketOpen || current.empty()) {
					LOG(MtkISP7, Error) << "Config file error, "
							    << "invalid dump filename format: " << line;
					return -EINVAL;
				}
				bracketOpen = false;
				// Bracketed format must be valid!
				if (!StaticStrings::formatKeyExists(current)) {
					LOG(MtkISP7, Error) << "Config file error, "
							    << "invalid dump filename format: " << line;
					return -EINVAL;
				}
				fileNameFormat.push_back(current);
				current = "";
				break;
			default:
				current += c;
				break;
			}
		}
		if (bracketOpen) {
			LOG(MtkISP7, Error) << "Config file error, "
					    << "invalid dump filename format: " << line;
			return -EINVAL;
		}
		if (current.size() != 0) {
			fileNameFormat.push_back(current);
		}
		for (Dump::Id id : dumpIds) {
			config[id] = {
				.dumpFileNameFormat = fileNameFormat,
				.enableExport = parsed[5] == "1",
				.writeReimportConfig = parsed[6].length() > 0 ? parsed[6][0] == '1' : false
			};
		}
	}

	LOG(MtkISP7, Info) << "Loaded dump config for " << config.size() << " different dumps.";
	return 0;
}

int ImagiqAdapter::loadImportConfig(
	std::map<Dump::Id, Dump::Config> &config,
	const std::filesystem::path &workPath)
{
	std::ifstream dumpImportCfgFile(workPath / kImportConfigPath);
	if (!dumpImportCfgFile.good()) {
		LOG(MtkISP7, Info) << "No dump import config";
		return 0;
	}
	std::string line;
	while (std::getline(dumpImportCfgFile, line)) {
		std::stringstream ss(line);
		std::string token;
		std::vector<std::string> parsedKeyVal;
		while (std::getline(ss, token, ';')) {
			parsedKeyVal.push_back(token);
		}
		if (parsedKeyVal.size() != 2) {
			LOG(MtkISP7, Error)
				<< "Dump import config file error, expected 'key;value'"
				<< "but got "
				<< ": " << line;
			return -EINVAL;
		}
		if (parsedKeyVal[1].back() == '\n') {
			parsedKeyVal[1].pop_back();
		}
		std::filesystem::path dumpPath = parsedKeyVal[1];
		std::vector<std::string> parsedKeys;
		ss = std::stringstream(line);
		while (std::getline(ss, token, ',')) {
			parsedKeys.push_back(token);
		}
		std::string moduleStr = parsedKeys[0];
		int frameNumber = std::stoi(parsedKeys[1]);
		std::string stageStr = parsedKeys[2];
		int layer = std::stoi(parsedKeys[3]);
		std::optional<Action> action = std::nullopt;
		if (parsedKeys[4] != "-1") {
			action = static_cast<Action>(std::stoi(parsedKeys[4]));
		}
		// Unlike dump cfg, this must be 1:1 match because
		// layers are specified in the cfg.
		std::vector<Dump::Id> matchesStageModule;
		for (const auto &[id, metadata] : kDumpMetadata) {
			if (StaticStrings::kModuleStrMap.at(metadata.moduleId) == moduleStr &&
			    kStageStrMap.at(metadata.stage) == stageStr) {
				matchesStageModule.push_back(id);
			}
		}

		if (matchesStageModule.empty()) {
			LOG(MtkISP7, Warning) << "Dump import config not recognized: "
					      << line;
			continue;
		}

		// Matching the module and stage not enough, matching the action.
		std::vector<Dump::Id> matchesStageModuleAction;
		if (matchesStageModule.size() > 1) {
			for (const auto &id : matchesStageModule) {
				Dump::Metadata metadata = kDumpMetadata.at(id);
				if ((action.has_value() && action.value() == metadata.action) ||
				    (!action.has_value() && !metadata.action.has_value())) {
					matchesStageModuleAction.push_back(id);
				}
			}
		} else {
			matchesStageModuleAction = matchesStageModule;
		}

		if (matchesStageModuleAction.empty()) {
			LOG(MtkISP7, Warning) << "Dump import config not recognized: "
					      << line;
			continue;
		}

		// Match by layer if still not enough.
		std::vector<Dump::Id> matchAllKeys;
		if (matchesStageModuleAction.size() > 1) {
			for (const auto &id : matchesStageModuleAction) {
				Dump::Metadata metadata = kDumpMetadata.at(id);
				if (layer == metadata.layer) {
					matchAllKeys.push_back(id);
				}
			}
		} else {
			matchAllKeys = matchesStageModuleAction;
		}

		if (matchAllKeys.size() != 1) {
			LOG(MtkISP7, Warning) << "Dump import config not recognized: "
					      << line;
			continue;
		}

		Dump::Id id = matchAllKeys[0];
		int currentPlane = config[id].savedDumps[frameNumber].size();
		config[id].savedDumps[frameNumber].push_back(dumpPath);
		LOG(MtkISP7, Info) << "Found dump for " << parsedKeyVal[0]
				   << " frame num: " << frameNumber
				   << " plane: " << currentPlane
				   << " path: " << dumpPath;
	}

	return 0;
}

/**
 * @brief Merge MTK 2A histogram arrays. 
 *
 * MTK 3A library histogram structure:
 * h1t1, h1t2, ..., h1tN, h2t1, h2t2, ..., h2tN, h3t1, h3t2, ..., h3tN
 * However, what imagiq wants:
 * h1t1, h2t1, h3t1, h1t2, h2t2, h3t2, ..., h1tN, h2tN, h3tN
 */
void ImagiqAdapter::merge2AHistogram(std::vector<uint8_t> &out,
				     mtk_cam_uapi_meta_raw_stats_0 *stats)
{
	std::array<int32_t, 3> temp{ 0, 0, 0 };
	out.resize(MTK_CAM_UAPI_AAHO_HIST_SIZE);
	for (size_t j = 0; j < out.size() / 3; j++) {
		for (size_t i = 0; i < stats->pipeline_config.num_of_core; i++) {
			std::memcpy(&temp[i],
				    reinterpret_cast<void *>(reinterpret_cast<intptr_t>(stats) +
							     stats->ae_awb_stats.aaho_buf.offset + (MTK_CAM_UAPI_AAHO_HIST_SIZE * i) + (3 * j)),
				    3);
		}
		out[3 * j] = (temp[0] + temp[1] + temp[2]) & 0xFF;
		out[3 * j + 1] = ((temp[0] + temp[1] + temp[2]) >> 8) & 0xFF;
		out[3 * j + 2] = ((temp[0] + temp[1] + temp[2]) >> 16) & 0xFF;
	}
}

void ImagiqAdapter::notifyExportRequest(int count)
{
	int ret = NSCam::Utils::Properties::property_set(
		"vendor.debug.ndd.prv_ready", std::to_string(count).c_str());
	if (ret != 0) {
		LOG(MtkISP7, Error) << "Failed to setprop!";
	}
}

void ImagiqAdapter::notifyImportRequest(int count)
{
	int ret = NSCam::Utils::Properties::property_set(
		"vendor.debug.odt.start", std::to_string(count).c_str());
	if (ret != 0) {
		LOG(MtkISP7, Error) << "Failed to setprop!";
	}
}

void ImagiqAdapter::notifyNewSession(std::string sensorId, int dumpTimestamp)
{
	for (const auto &[_, mtkSensorId] : sensorIdMap) {
		int sensorIdInt = static_cast<int>(mtkSensorId);
		NSCam::TuningUtils::INdd::getInstance()->stream_off(
			{ sensorIdInt });
		NSCam::TuningUtils::IOdtUtils::getInstance(sensorIdInt)->stream_off();
	}
	int sensorIdInt =
		static_cast<int>(static_cast<int>(sensorIdMap.at(sensorId)));
	NSCam::TuningUtils::INdd::getInstance()->stream_on(
		{ sensorIdInt }, dumpTimestamp);
	NSCam::TuningUtils::IOdtUtils::getInstance(sensorIdInt)->stream_on();
}

void ImagiqAdapter::notifyRequestBegin(
	std::string sensorId, int requestNumber)
{
	int sensorIdInt =
		static_cast<int>(static_cast<int>(sensorIdMap.at(sensorId)));
	NSCam::TuningUtils::INdd::getInstance()->frame_begin(
		sensorIdInt, requestNumber, true);
	NSCam::TuningUtils::IOdtUtils::getInstance(sensorIdInt)->frame_begin(requestNumber);
}

void ImagiqAdapter::notifyRequestEnd(
	std::string sensorId, int requestNumber,
	int dumpSessionTimestamp, bool hasPendingExport,
	std::filesystem::path rootWorkPath,
	std::filesystem::path sessionWorkPath)
{
	int sensorIdInt =
		static_cast<int>(static_cast<int>(sensorIdMap.at(sensorId)));
	NSCam::TuningUtils::INdd::getInstance()->frame_end(
		sensorIdInt, requestNumber);
	NSCam::TuningUtils::IOdtUtils::getInstance(sensorIdInt)->frame_end(requestNumber);
	if (!hasPendingExport) {
		flushPrivateReimportConfig(
			rootWorkPath, sessionWorkPath, dumpSessionTimestamp);
	}
}

NSCam::TuningUtils::NddData ImagiqAdapter::parseNdd(const Dump &dump)
{
	NSCam::TuningUtils::NddData ndd;
	ndd.requestNo = dump.requestNumber;
	ndd.frameNo = dump.frameNumber;
	ndd.timestamp = dump.timestamp;
	ndd.sensorId = sensorIdMap.at(dump.sensorId);

	ndd.feature = static_cast<int>(dump.metadata.featureId);
	ndd.stage = static_cast<int>(dump.metadata.stage);
	ndd.action = dump.metadata.action.has_value() ? static_cast<int>(dump.metadata.action.value()) : -1;
	ndd.layer = dump.metadata.layer;
	ndd.version = dump.metadata.version;
	ndd.platform = 8188;

	if (dump.array.has_value()) {
		ndd.width = dump.array->size();
		if (dump.metadata.elementSize.has_value()) {
			ndd.width /= *dump.metadata.elementSize;
		}
		ndd.height = 1;
	}

	if (!dump.frame.has_value()) {
		return ndd;
	}

	const PixelFormatInfo pixelFormatInfo =
		PixelFormatInfo::info(dump.frame->format());
	ndd.bitResultion = pixelFormatInfo.planes[0].bytesPerGroup * 8 /
			   pixelFormatInfo.pixelsPerGroup;

	// Quirk-able fields
	unsigned int bitsPerPixel = pixelFormatInfo.bitsPerPixel;
	ndd.bayerOrder = -1;
	ndd.signedness = 0;

	// Quirks
	switch (dump.frame->format()) {
	case formats::SBGGR10_MTISP:
		ndd.bayerOrder = SENSOR_FORMAT_ORDER_RAW_B;
		ndd.signedness = -1;
		break;
	case formats::SGBRG10_MTISP:
		ndd.bayerOrder = SENSOR_FORMAT_ORDER_RAW_Gb;
		ndd.signedness = -1;
		break;
	case formats::SGRBG10_MTISP:
		ndd.bayerOrder = SENSOR_FORMAT_ORDER_RAW_Gr;
		ndd.signedness = -1;
		break;
	case formats::SRGGB10_MTISP:
		ndd.bayerOrder = SENSOR_FORMAT_ORDER_RAW_R;
		ndd.signedness = -1;
		break;
	case formats::NV12:
	case formats::NV21:
		bitsPerPixel = 8;
		break;
	case formats::NV12_10P_MTISP:
		bitsPerPixel = 10;
		break;
	default:
		break;
	}
	unsigned int stride = pixelFormatInfo.stride(
		dump.frame->size().width,
		0,
		dump.frame->strideAlign());
	ndd.byteWidth = stride;
	ndd.pixelWidth = stride * 8 / bitsPerPixel;
	ndd.pixelHeight = pixelFormatInfo.planeSize(
				  dump.frame->size(),
				  0, dump.frame->strideAlign(),
				  dump.frame->scanAlign()) /
			  stride;

	ndd.width = dump.frame->size().width;
	ndd.height = dump.frame->size().height;

	return ndd;
}

int ImagiqAdapter::prepareReimport(const ExportResult &exportResult)
{
	if (!exportResult.paths.has_value() || exportResult.paths->empty()) {
		return -EINVAL;
	}

	const std::filesystem::path importConfigPath =
		exportResult.dump.workPath / kImportConfigPath;
	std::ofstream dumpLoadConfigFile(importConfigPath, std::ios::app);

	if (!dumpLoadConfigFile.good()) {
		LOG(MtkISP7, Error) << "Failed to open dump import config file: "
				    << importConfigPath;
		return -EIO;
	}

	for (const auto &path : *exportResult.paths) {
		const std::string config = createImportConfigId(exportResult.dump) + ';' +
					   path.string();
		dumpLoadConfigFile << config << std::endl;

		if (!dumpLoadConfigFile.good()) {
			LOG(MtkISP7, Error) << "Failed to write to file: "
					    << importConfigPath;
			return -EIO;
		}
		LOG(MtkISP7, Info) << "Dump " << path
				   << " is prepared to be reload, config: "
				   << importConfigPath;
	}
	return 0;
}

void ImagiqAdapter::serializeExif(
	std::vector<uint8_t> &out,
	const mtk::isphal::v1_0::ExifInfo3A &exif3a,
	const mtk::isphal::v1_0::ExifInfoP2 &exifIsp)
{
	out.resize(6 + exif3a.size + 4 + exifIsp.size);

	// 3A
	uint8_t *exif3aArray = out.data();
	exif3aArray[0] = 0;
	exif3aArray[1] = 0;
	exif3aArray[2] = 0xFF;
	exif3aArray[3] = 0xE6;
	exif3aArray[4] = ((exif3a.size + 2) >> 8);
	exif3aArray[5] = ((exif3a.size + 2) & 0xFF);
	std::memcpy(&(exif3aArray[6]), exif3a.data, exif3a.size);

	// ISP
	uint8_t *exifIspArray = out.data() + 6 + exif3a.size;
	exifIspArray[0] = 0xFF;
	exifIspArray[1] = 0xE7;
	exifIspArray[2] = ((exifIsp.size + 2) >> 8);
	exifIspArray[3] = ((exifIsp.size + 2) & 0xFF);
	memcpy(&(exifIspArray[4]), exifIsp.data, exifIsp.size);
}

bool ImagiqAdapter::shouldSplitExport(const PixelFormat &pixelFormat)
{
	switch (pixelFormat) {
	case formats::NV21:
	case formats::NV12:
		return false;
	default:
		return PixelFormatInfo::info(pixelFormat).numPlanes() > 1;
	}
}

void ImagiqAdapter::writeScenarioRecorderSettings(
	mtk::isphal::v1_0::scenarioRecordParam &outParam,
	NSCam::IMetadata *metadata,
	int dumpSessionTimestamp,
	int requestNumber,
	int frameNumber,
	EStage_T stage,
	const std::string &sensorId)
{
	using NSCam::TuningUtils::scenariorecorder::IScenarioRecorder;
	if (!IScenarioRecorder::getInstance()->isScenarioRecorderOn()) {
		return;
	}
	int32_t mtkSensorId =
		static_cast<int32_t>(ImagiqAdapter::sensorIdMap.at(sensorId));
	outParam.enable = 1;

	NSCam::TuningUtils::INdd::getInstance()->update_uniquekey(
		*metadata, dumpSessionTimestamp, requestNumber, frameNumber);

	outParam.pHalMeta = metadata;
	// decision log
	outParam.decision_param.magicNum =
		static_cast<uint32_t>(requestNumber);
	outParam.decision_param.staticInfo.sensorId =
		mtkSensorId;
	outParam.decision_param.staticInfo.moduleId =
		NSCam::Utils::ULog::MOD_ISP_MGR;
	outParam.decision_param.decisionType =
		NSCam::TuningUtils::scenariorecorder::DECISION_TOP_CONTROL;
	// execution log
	outParam.result_param.magicNum =
		static_cast<uint32_t>(requestNumber);
	outParam.result_param.staticInfo.sensorId =
		mtkSensorId;
	outParam.result_param.staticInfo.moduleId =
		NSCam::Utils::ULog::MOD_ISP_MGR;
	outParam.result_param.decisionType =
		NSCam::TuningUtils::scenariorecorder::DECISION_TOP_CONTROL;
	outParam.result_param.stageId =
		static_cast<int32_t>(stage);
}

} // namespace libcamera

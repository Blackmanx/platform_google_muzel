/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * imagiq_adapter.h - MtkISP7 OnDeviceTuner Imagiq Adapter
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

#include "libcamera/internal/mapped_framebuffer.h"

#include "halisp/IspControls.h"
#include "mtkcam-halif/utils/metadata/1.x/IMetadata.h"
#include "mtkcam-interfaces/utils/ScenarioRecorder/IScenarioRecorder.h"
#include "mtkcam-interfaces/utils/ndd/INdd.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/dump.h"
#include "platform/mtkisp7/halisp/TuningParam.h"
#include "tuning_mapping/cam_idx_struct_ext_pub.h"

#include "mtk_cam_metabuf.h"
#include "mtkisp7_ipa_interface.h"
namespace libcamera {

class ImagiqAdapter
{
public:
	struct ExportResult {
		Dump dump;
		std::optional<int> errorCode = std::nullopt;
		std::optional<std::vector<std::filesystem::path>> paths = std::nullopt;
	};
	using SensorIdMap = std::map<std::string, NSCam::TuningUtils::eSensorId>;

	static void configureScenarioRecorder(
		int requestNumber, int frameNumber,
		int timestamp, bool highIsoMode,
		bool isStillCapture, Feature feature,
		std::string logMessage, EStage_T stage, std::string sensorId);

	static int enableMtkTuningTool(std::filesystem::path workDir);

	static ExportResult exportDump(const Dump &dump);

	static std::string formatTimestamp(int timestamp);

	static int loadConfig(
		std::map<Dump::Id, Dump::Config> &config,
		const std::filesystem::path &workPath);

	static int generateDumpTimestamp();

	static std::string getDumpFileName(const Dump &dump);

	static int importDump(const Dump &dump);

	static void merge2AHistogram(std::vector<uint8_t> &out,
				     mtk_cam_uapi_meta_raw_stats_0 *stats);

	static void notifyExportRequest(int count);

	static void notifyImportRequest(int count);

	static void notifyNewSession(std::string sensorId, int dumpTimestamp);

	static void notifyRequestBegin(
		std::string sensorId, int requestNumber);

	static void notifyRequestEnd(
		std::string sensorId, int requestNumber,
		int dumpSessionTimestamp, bool hasPendingExport,
		std::filesystem::path rootWorkPath,
		std::filesystem::path sessionWorkPath);

	static int prepareReimport(const ExportResult &dumpResult);

	static void serializeExif(
		std::vector<uint8_t> &out,
		const mtk::isphal::v1_0::ExifInfo3A &exif3a,
		const mtk::isphal::v1_0::ExifInfoP2 &exifIsp);

	static void writeScenarioRecorderSettings(
		mtk::isphal::v1_0::scenarioRecordParam &outParam,
		NSCam::IMetadata *metadata,
		int dumpSessionTimestamp,
		int requestNumber,
		int frameNumber,
		EStage_T stage,
		const std::string &sensorId);

	static SensorIdMap sensorIdMap;

private:
	static std::string createImportConfigId(const Dump &dump);

	static ExportResult exportArrayDump(
		const Dump &dump, const NSCam::TuningUtils::NddData &ndd);

	static ExportResult exportDumpMergePlanes(
		const Dump &dumpInfo, const MappedFrameBuffer &mappedBuffer,
		const NSCam::TuningUtils::NddData &ndd,
		const std::string &fileSuffix = "");

	static ExportResult exportDumpSplitPlanes(
		const Dump &dumpInfo, const MappedFrameBuffer &mappedBuffer,
		const NSCam::TuningUtils::NddData &ndd,
		const PixelFormat &pixelFormat,
		const std::string &fileSuffix = "");

	static ExportResult exportFrameDump(
		const Dump &dump, const NSCam::TuningUtils::NddData &ndd);

	static void flushPrivateReimportConfig(
		std::filesystem::path rootWorkPath,
		std::filesystem::path sessionWorkPath,
		int dumpSessionTimestamp);

	static std::string formatPlaneName(int planeNumber,
					   const PixelFormat &pixelFormat);

	static std::filesystem::path getDumpFileNameSplitPlanes(
		const Dump &dumpInfo, const NSCam::TuningUtils::NddData &ndd,
		int planeNumber, const std::optional<PixelFormat> pixelFormat,
		const std::string &suffix = "");

	static std::filesystem::path getDumpFileNameSingleFile(
		const Dump &dumpInfo, const NSCam::TuningUtils::NddData &ndd,
		const std::string &suffix = "");

	static std::string getFileExtension(const PixelFormat &pixelFormat);

	static int loadBaseConfig(
		std::map<Dump::Id, Dump::Config> &config,
		const std::filesystem::path &workPath);

	static int loadImportConfig(
		std::map<Dump::Id, Dump::Config> &config,
		const std::filesystem::path &workPath);

	static NSCam::TuningUtils::NddData
	parseNdd(const Dump &dumpInfo);

	static bool shouldSplitExport(const PixelFormat &pixelFormat);

	static std::unique_ptr<NSCam::TuningUtils::NddInitializer>
		mtkTuningInitializer_;

	static std::unique_ptr<NSCam::TuningUtils::scenariorecorder::ScenarioRecorderInitializer>
		mtkScenarioRecorderInitializer_;

	static const std::array<std::string, 2> kYcPlaneNames;
	static const std::array<std::string, 3> kYuvPlaneNames;
	static const std::array<std::string, 2> kWarpPlaneNames;
};

} // namespace libcamera

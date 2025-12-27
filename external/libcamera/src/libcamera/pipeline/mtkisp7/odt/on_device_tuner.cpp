/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * on_device_tuner.cpp - MtkISP7 On Device Tuner module.
 */

#include "pipeline/mtkisp7/odt/on_device_tuner.h"

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <unistd.h>
#include <vector>

#include <libcamera/base/log.h>

#include <libcamera/control_ids.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>

#include <libcamera/ipa/mtkisp7_ipa_interface.h>

#include "debug_exif/aaa/dbg_aaa_param.h"
#include "linux/mtkisp7/drv/7.1/ctrl_meta.h"
#include "mtkcam-halif/utils/metadata/1.x/IMetadata.h"
#include "mtkcam-interfaces/utils/metadata/hal/mtk_platform_metadata_tag.h"
#include "mtkcam-interfaces/utils/ndd/ndd_autogen_def.h"
#include "pipeline/mtkisp7/camsys/capture.h"
#include "pipeline/mtkisp7/imgsys/lpnr.h"
#include "pipeline/mtkisp7/imgsys/mcnr.h"
#include "pipeline/mtkisp7/imgsys/mfnr.h"
#include "pipeline/mtkisp7/ipa/ipa_delegate.h"
#include "pipeline/mtkisp7/odt/camsys_driver_debug.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/dump.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/imagiq_adapter.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/dump_metadata.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/feature.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/stage.h"
#include "pipeline/mtkisp7/odt/imgsys_driver_debug.h"
#include "platform/mtkisp7/halisp/IspControls.h"
#include "platform/mtkisp7/mtkcam-chrom/custom/mt8188/hal/inc/debug_exif/cam/dbg_cam_param.h"
#include "platform/mtkisp7/single_device_helper.h"
#include "tuning_mapping/cam_idx_struct_ext_pub.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

namespace {

constexpr const char *kEnableTuningPath = "/run/camera/enable_tuning";
constexpr const char *kEnforceLowIsoLpnr = "/run/camera/enforce_low_iso_lpnr";
constexpr const char *kExportRequestPath = "/run/camera/export_dump";
constexpr const char *kImportRequestPath = "/run/camera/import_dump";
constexpr const char *kWorkDir = "/mnt/stateful_partition/vendor/camera_dump";
constexpr const char *kEnableCamsysDebugFrame =
	"/run/camera/camsys_debug_frame";

constexpr const uint32_t kDelayOdt = 3;

} // namespace

std::vector<ImagiqAdapter::ExportResult> OnDeviceTuner::batchExport(
	const std::vector<Dump> &dumps)
{
	std::vector<ImagiqAdapter::ExportResult> results;
	results.reserve(dumps.size());
	for (auto dump : dumps) {
		if (dump.config.enableExport) {
			results.push_back(ImagiqAdapter::exportDump(dump));
		}
	}
	return results;
}

void OnDeviceTuner::batchImport(const std::vector<Dump> &dumps)
{
	for (auto dump : dumps) {
		ImagiqAdapter::importDump(dump);
	}
}

void OnDeviceTuner::batchPrepareReimport(
	const std::vector<ImagiqAdapter::ExportResult> &exportResults)
{
	for (const auto &result : exportResults) {
		if (!result.errorCode.has_value() &&
		    result.dump.config.writeReimportConfig) {
			ImagiqAdapter::prepareReimport(result);
		}
	}
}

void OnDeviceTuner::configure(
	const std::string &sensorId, unsigned int camsysIndex,
	int sessionTimestamp, IPADelegate *ipa)
{
	if (!enabled_) {
		return;
	}

	enabled_ = true;
	exportBegin_ = 0;
	exportEnd_ = 0;
	importBegin_ = 0;
	importEnd_ = 0;
	prevStartedRequestNum_ = -1;
	prevEndedRequestNum_ = -1;
	sensorId_ = sensorId;
	enableCamsysDebugFrame_ = false;
	camsysDebug_ = CamsysDebug::create(camsysIndex);
	if (isIpa_)
		sessionTimestamp_ = sessionTimestamp;
	else {
		sessionTimestamp_ = ImagiqAdapter::generateDumpTimestamp();
		ipa_ = ipa;
	}

	if (std::filesystem::exists(kEnforceLowIsoLpnr)) {
		enforceLowIsoLpnr_ = true;
		LOG(MtkISP7, Warning) << "LPNR is forced to use low-ISO mode!";
	}

	if (std::filesystem::exists(kEnableCamsysDebugFrame)) {
		enableCamsysDebugFrame_ = true;
		LOG(MtkISP7, Warning) << "Camsys raw-inject debug frame "
				      << "is enabled!";
	}

	// Immediately create one directory for any capture dumps.
	prepareNewExportDirectory();

	if (isIpa_)
		ImagiqAdapter::notifyNewSession(sensorId, sessionTimestamp_);
}

void OnDeviceTuner::fillCamsysDebugFrame(
	uint32_t internalRequestId, SharedMailBox<InfoFrame> debugMailBox)
{
	if (!enabled_ || !enableCamsysDebugFrame_) {
		return;
	}

	if (!shouldImportDumpNow(internalRequestId)) {
		return;
	}

	Dump::Config imgoCfg = dumpConfig_[Dump::Id::P1_IMGO];
	Dump imgo{
		.id = Dump::Id::P1_IMGO,
		.requestNumber = internalRequestId,
		.frameNumber = internalRequestId,
		.sensorId = sensorId_,
		.timestamp = sessionTimestamp_,
		.workPath = currentExportPath_,
		.frame = debugMailBox->get(),
		.array = std::nullopt,
		.metadata = kDumpMetadata.at(Dump::Id::P1_IMGO),
		.config = imgoCfg
	};
	ImagiqAdapter::importDump(imgo);
}

void OnDeviceTuner::initialize(bool isIpa)
{
	isIpa_ = isIpa;
	enabled_ = false;
	enforceLowIsoLpnr_ = false;

	if (!std::filesystem::exists(kEnableTuningPath)) {
		return;
	}

	int ret = ImagiqAdapter::loadConfig(dumpConfig_, kWorkDir);
	if (ret) {
		LOG(MtkISP7, Error) << "Failed to load the config, error code: " << ret;
		return;
	}

	if (isIpa) {
		ret = ImagiqAdapter::enableMtkTuningTool(kWorkDir);
		if (ret) {
			LOG(MtkISP7, Error) << "Failed to enable MTK tuning tool";
			return;
		}
	}

	LOG(MtkISP7, Warning) << "Pipeline tuning enabled";
	enabled_ = true;
}

bool OnDeviceTuner::isCamsysDebugFrameEnabled()
{
	return enabled_ && enableCamsysDebugFrame_;
}

bool OnDeviceTuner::isLowIsoLpnrEnforced()
{
	return enabled_ && enforceLowIsoLpnr_;
}

void OnDeviceTuner::notifyExportBegin(
	const uint32_t exportBegin,
	const uint32_t exportEnd)
{
	if (!isIpa_)
		LOG(MtkISP7, Fatal) << "Calling notifyExportBegin in pipeline handler";

	exportBegin_ = exportBegin;
	exportEnd_ = exportEnd;

	if (prevStartedRequestNum_ == (int)exportBegin_)
		ImagiqAdapter::notifyExportRequest(exportEnd_ - exportBegin_);
}

void OnDeviceTuner::notifyImportBegin(
	const uint32_t importBegin,
	const uint32_t importEnd)
{
	if (!isIpa_)
		LOG(MtkISP7, Fatal) << "Calling notifyImportBegin in pipeline handler";

	importBegin_ = importBegin;
	importEnd_ = importEnd;

	if (prevStartedRequestNum_ == (int)importBegin_)
		ImagiqAdapter::notifyImportRequest(importEnd_ - importBegin_);
}

void OnDeviceTuner::loadTuneRequest(int requestNumber)
{
	if (isIpa_)
		LOG(MtkISP7, Fatal) << "Calling loadTuneRequest in ipa";

	if (!enabled_) {
		return;
	}
	std::ifstream exportRequestFile(kExportRequestPath);
	if (exportRequestFile.good()) {
		int exportRequestCount = 0;
		exportRequestFile >> exportRequestCount;
		LOG(MtkISP7, Info) << "Loaded dump export request from file: "
				   << exportRequestCount << " frames";
		exportRequestFile.close();
		std::filesystem::path path(kExportRequestPath);
		std::filesystem::remove(path);
		exportBegin_ = requestNumber + kDelayOdt; // Delay 3 frames to wait for IPA.
		exportEnd_ = exportBegin_ + exportRequestCount;

		ipa_->notifyExportBegin(exportBegin_, exportEnd_);
	}

	std::ifstream importRequestFile(kImportRequestPath);
	if (importRequestFile.good()) {
		int importRequestCount = 0;
		importRequestFile >> importRequestCount;
		LOG(MtkISP7, Info) << "Loaded dump import request from file: "
				   << importRequestCount << " frames";
		importRequestFile.close();
		std::filesystem::path path(kImportRequestPath);
		std::filesystem::remove(path);
		importBegin_ = requestNumber + kDelayOdt; // Delay 3 frames to wait for IPA.
		importEnd_ = importBegin_ + importRequestCount;

		ipa_->notifyImportBegin(importBegin_, importEnd_);
	}
}

InfoFrame OnDeviceTuner::getFrameInfoFromRequest(
	Request *request, FrameBuffer *buffer)
{
	const auto stream = request->findStream(buffer);
	if (!stream) {
		LOG(MtkISP7, Fatal) << "Buffer doesn't exist in request!";
	}
	const auto streamCfg = stream->configuration();

	/* Android requires NV12 to align with 64 for buffers from application */
	LOG(MtkISP7, Error) << "pixelFormat = " << std::hex << streamCfg.pixelFormat;
	return InfoFrame(streamCfg.pixelFormat, streamCfg.size, buffer, 64);
}

NSCam::IMetadata *OnDeviceTuner::getMtkMetadata(int requestNumber)
{
	if (!isIpa_)
		LOG(MtkISP7, Fatal) << "getMtkMetadata is called in pipeline handler";

	NSCam::IMetadata *metadata = nullptr;
	if (mtkMetadata_.count(requestNumber) == 0) {
		metadata = new NSCam::IMetadata;
		mtkMetadata_.emplace(
			requestNumber,
			std::unique_ptr<NSCam::IMetadata>(metadata));

		// Hack: without this the metadata is invalid.
		// The IMetadata default constructor does not allocate
		// any memory for the metadata, but the remove function
		// actually allocate some memory if there wasn't any.
		metadata->remove(MTK_NDD_INFORMATION_KEY);

	} else {
		metadata = mtkMetadata_.at(requestNumber).get();
	}
	return metadata;
}

bool OnDeviceTuner::isImgsysCaptureStage(PEU_Stage stage)
{
	return std::find(
		       kImgsysCaptureStages.begin(), kImgsysCaptureStages.end(), stage) !=
	       kImgsysCaptureStages.end();
}

bool OnDeviceTuner::isStillCaptureFeature(Feature feature)
{
	return feature == Feature::Capture_lpnr ||
	       feature == Feature::Capture_mfnr;
}

void OnDeviceTuner::notifyRequestBegin(int requestNumber)
{
	if (!enabled_ || requestNumber <= prevStartedRequestNum_) {
		return;
	}
	prevStartedRequestNum_ = requestNumber;

	if (!isIpa_)
		loadTuneRequest(requestNumber);

	if (isIpa_) {
		if (prevStartedRequestNum_ == (int)exportBegin_) {
			ImagiqAdapter::notifyExportRequest(
				exportEnd_ - exportBegin_);
		}

		if (prevStartedRequestNum_ == (int)importBegin_) {
			ImagiqAdapter::notifyImportRequest(
				importEnd_ - importBegin_);
		}

		ImagiqAdapter::notifyRequestBegin(
			sensorId_, requestNumber);
	}
}

void OnDeviceTuner::notifyRequestEnd(int requestNumber)
{
	if (!enabled_ || requestNumber <= prevEndedRequestNum_) {
		return;
	}
	prevEndedRequestNum_ = requestNumber;
	stillCaptureFrames_.erase(requestNumber);

	if (isIpa_) {
		ImagiqAdapter::notifyRequestEnd(
			sensorId_, requestNumber, sessionTimestamp_,
			shouldExportDumpNow(requestNumber),
			kWorkDir, currentExportPath_);

		mtkMetadata_.erase(requestNumber);
	}
}

void OnDeviceTuner::notifyVideoOnly(int requestNumber)
{
	if (!isIpa_)
		LOG(MtkISP7, Fatal) << "notifyVideoOnly is called in pipeline handler";

	ImagiqAdapter::configureScenarioRecorder(
		requestNumber, requestNumber,
		sessionTimestamp_, false, false,
		Feature::NUM, "", EStage_NUM, sensorId_);
}

void OnDeviceTuner::notifyStillCapture(int baseRequestNumber, int frameNumber)
{
	stillCaptureFrames_.insert({ frameNumber, baseRequestNumber });
}

bool OnDeviceTuner::isStillCaptureRequest(int requestNumber)
{
	return stillCaptureFrames_.count(requestNumber) > 0;
}

bool OnDeviceTuner::parseHalIspNdd(
	uint32_t internalRequestId,
	uint32_t frameNumber,
	mtk::isphal::v1_0::NddInfo &ndd,
	Feature feature)
{
	uint32_t requestNumber = internalRequestId;
	// |feature| from IPC getImgSysMetaTuning.
	bool isStillCapture = isStillCaptureFeature(feature);
	if (!enabled_ || (!shouldExportDumpNow(requestNumber) && !isStillCapture)) {
		return false;
	}
	if (isStillCapture) {
		ndd.ndd_data.action =
			static_cast<int>(Action::Capture);
		ndd.ndd_category = NSCam::TuningUtils::eCategory::kCAPTURE;
	} else {
		ndd.ndd_category =
			NSCam::TuningUtils::eCategory::kSTREAMING;
	}
	ndd.ndd_data.feature = static_cast<int>(feature);
	ndd.ndd_data.requestNo = internalRequestId;
	ndd.ndd_data.frameNo = frameNumber;
	ndd.ndd_data.platform = 8188;
	ndd.ndd_data.timestamp = sessionTimestamp_;
	ndd.ndd_data.sensorId =
		ImagiqAdapter::sensorIdMap.at(sensorId_);
	ndd.ndd_data.dualCamId = NSCam::TuningUtils::eDualCamId::kINVALID;
	ndd.ndd_data.pixelHeight = -1;
	ndd.ndd_data.pixelWidth = -1;
	return true;
}

int OnDeviceTuner::prepareNewExportDirectory()
{
	std::filesystem::path workPath(kWorkDir);
	std::filesystem::path newPath =
		workPath /
		("UKey" + ImagiqAdapter::formatTimestamp(sessionTimestamp_));
	if (!isIpa_) {
		auto cmd = "mkdir -p " + newPath.string();
		int ret = system(cmd.c_str());
		if (ret != 0) {
			LOG(MtkISP7, Error) << "Failed to prepare dump directory, error code: "
					    << ret;
			return ret;
		}
	}
	LOG(MtkISP7, Info) << "Current dump directory: " << newPath.string();
	currentExportPath_ = newPath;
	return 0;
}

bool OnDeviceTuner::shouldExportDumpNow(uint32_t requestNumber)
{
	return enabled_ && exportBegin_ <= requestNumber &&
	       requestNumber < exportEnd_;
}

bool OnDeviceTuner::shouldImportDumpNow(uint32_t requestNumber)
{
	return enabled_ && importBegin_ <= requestNumber &&
	       requestNumber < importEnd_;
}

void OnDeviceTuner::tune(
	uint32_t requestNumber,
	std::vector<NamedFrame> namedFrames,
	bool forceDump)
{
	tune(requestNumber, requestNumber, namedFrames, forceDump);
}

void OnDeviceTuner::tune(
	uint32_t requestNumber,
	uint32_t frameNumber,
	std::vector<NamedFrame> namedFrames,
	bool forceDump)
{
	if (!enabled_ && !forceDump &&
	    !shouldExportDumpNow(requestNumber) && !shouldImportDumpNow(requestNumber)) {
		return;
	}
	std::vector<Dump> dumps;
	for (auto namedFrame : namedFrames) {
		Dump::Metadata metadata = kDumpMetadata.at(namedFrame.id);
		Dump::Config config = dumpConfig_[namedFrame.id];
		dumps.push_back({ .id = namedFrame.id,
				  .requestNumber = requestNumber,
				  .frameNumber = frameNumber,
				  .sensorId = sensorId_,
				  .timestamp = sessionTimestamp_,
				  .workPath = currentExportPath_,
				  .frame = namedFrame.frame,
				  .array = std::nullopt,
				  .metadata = metadata,
				  .config = config });
	}
	if (forceDump || shouldExportDumpNow(requestNumber)) {
		const auto exportResults = batchExport(dumps);
		// TODO: Pipeline handler and IPC process need to avoid writing at the same time.
		batchPrepareReimport(exportResults);
	}
	if (shouldImportDumpNow(requestNumber)) {
		batchImport(dumps);
	}
}

void OnDeviceTuner::tune(
	uint32_t requestNumber,
	std::vector<NamedPointer> namedPointers,
	bool forceDump)
{
	if (!enabled_ && !forceDump &&
	    !shouldExportDumpNow(requestNumber) && !shouldImportDumpNow(requestNumber)) {
		return;
	}
	uint32_t baseRequestId = requestNumber;
	if (isStillCaptureRequest(requestNumber)) {
		baseRequestId = stillCaptureFrames_.at(requestNumber);
	}
	std::vector<Dump> dumps;
	for (auto namedPtr : namedPointers) {
		Dump::Metadata metadata = kDumpMetadata.at(namedPtr.id);
		Dump::Config config = dumpConfig_[namedPtr.id];
		std::vector<uint8_t> buffer(namedPtr.size);
		std::memcpy(buffer.data(), namedPtr.ptr, namedPtr.size);
		dumps.push_back({ .id = namedPtr.id,
				  .requestNumber = baseRequestId,
				  .frameNumber = requestNumber,
				  .sensorId = sensorId_,
				  .timestamp = sessionTimestamp_,
				  .workPath = currentExportPath_,
				  .frame = std::nullopt,
				  .array = buffer,
				  .metadata = metadata,
				  .config = config });
	}
	if (forceDump || shouldExportDumpNow(requestNumber)) {
		const auto exportResults = batchExport(dumps);
		batchPrepareReimport(exportResults);
	}
	// todo(yerlandinata, before merge): decide what to do next
	// if (shouldImportDumpNow(requestNumber)) {
	//     batchImport(dumps);
	// }
}

void OnDeviceTuner::tuneCamsys(uint32_t internalRequestId, CaptureFrames &frames)
{
	uint32_t requestNumber = internalRequestId;
	if (!enabled_ || (!shouldExportDumpNow(requestNumber) &&
			  !shouldImportDumpNow(requestNumber))) {
		return;
	}
	tune(
		internalRequestId, { { Dump::Id::P1_IMGO, frames.raw->get() },
				     { Dump::Id::P1_YUVO_R1, frames.yuvo1->get() },
				     { Dump::Id::P1_YUVO_R2, frames.yuvo2->get() },
				     { Dump::Id::P1_DRZS4NO_R3, frames.me->get() },
				     { Dump::Id::P1_META_P1, frames.tuning->get() } });

	// Driver's registers
	Dump::Config drvRegConfig = dumpConfig_[Dump::Id::P1_REG_P1];
	if (shouldExportDumpNow(requestNumber) && drvRegConfig.enableExport && camsysDebug_) {
		Dump registerDump{
			.id = Dump::Id::P1_REG_P1,
			.requestNumber = requestNumber,
			.frameNumber = requestNumber,
			.sensorId = sensorId_,
			.timestamp = sessionTimestamp_,
			.workPath = currentExportPath_,
			.frame = std::nullopt,
			.array = std::nullopt,
			.metadata = kDumpMetadata.at(Dump::Id::P1_REG_P1),
			.config = drvRegConfig
		};
		std::filesystem::path dumpPath =
			ImagiqAdapter::getDumpFileName(registerDump);
		camsysDebug_->exportDump(
			frames.raw->get().buffer()->metadata().hwSequence,
			requestNumber, dumpPath);
	}
}

bool OnDeviceTuner::tuneCamsysHalIsp(
	uint32_t internalRequestId,
	mtk::isphal::v1_0::TuningParamP1 &tuningParam,
	mtk::isphal::v1_0::ReturnParamP1 &tuningResult,
	mtk::hal3a::v1_0::mtk_3a_result &mtk3AResult,
	Feature feature, bool highIsoMode)
{
	if (!enabled_) {
		return false;
	}
	tuningParam.cam_info->rNdd_info.ndd_data.stage =
		static_cast<int>(Stage::P1);
	tuningParam.cam_info->rNdd_info.ndd_data.action = -1;
	tuningParam.is_need_exif = 1;
	tuningResult.exif.valid = true;
	std::memcpy(tuningResult.exif.data, reinterpret_cast<uint8_t *>(&mtk3AResult.debug_isp_info), sizeof(AAA_DEBUG_INFO2_T));

	int baseRequestId = internalRequestId;
	if (isStillCaptureRequest(internalRequestId)) {
		baseRequestId = stillCaptureFrames_.at(internalRequestId);
	}
	ImagiqAdapter::configureScenarioRecorder(
		baseRequestId, internalRequestId, sessionTimestamp_,
		highIsoMode && !enforceLowIsoLpnr_,
		isStillCaptureRequest(internalRequestId), feature,
		"", EStage_NUM, sensorId_);

	ImagiqAdapter::writeScenarioRecorderSettings(
		tuningParam.cam_info->sr_para, getMtkMetadata(internalRequestId),
		sessionTimestamp_, internalRequestId, internalRequestId,
		NSIspTuning::EStage_P1, sensorId_);

	return parseHalIspNdd(internalRequestId, internalRequestId,
			      tuningParam.cam_info->rNdd_info, feature);
}

void OnDeviceTuner::tuneExif(
	uint32_t internalRequestId,
	uint32_t frameNumber,
	const mtk::isphal::v1_0::ExifInfo3A &exif3a,
	const mtk::isphal::v1_0::ExifInfoP2 &exifIsp,
	EStage_T stage, Feature feature)
{
	uint32_t requestNumber = internalRequestId;
	// |feature| from IPC getImgSysMetaTuning.
	bool isStillCapture = isStillCaptureFeature(feature);
	if (!enabled_ || (!shouldExportDumpNow(requestNumber) && !isStillCapture)) {
		return;
	}
	bool dumpIdFound = false;
	Dump::Id dumpId;
	switch (feature) {
	case Feature::Preview:
		if (kMcnrExifDumpIdMap.count(stage) > 0) {
			dumpId = kMcnrExifDumpIdMap.at(stage);
			dumpIdFound = true;
		}
		break;
	case Feature::Capture_lpnr:
		if (kLpnrExifDumpIdMap.count(stage) > 0) {
			dumpId = kLpnrExifDumpIdMap.at(stage);
			dumpIdFound = true;
		}
		break;
	case Feature::Capture_mfnr:
		if (kMfnrExifDumpIdMap.count(stage) > 0) {
			dumpId = kMfnrExifDumpIdMap.at(stage);
			dumpIdFound = true;
		}
		break;
	default:
		LOG(MtkISP7, Fatal) << "Unsupported feature: "
				    << static_cast<int>(feature);
	}

	if (!dumpIdFound) {
		LOG(MtkISP7, Error) << "No exif dump id for stage: "
				    << static_cast<int>(stage);
		return;
	}

	std::vector<uint8_t> exifArray;
	ImagiqAdapter::serializeExif(exifArray, exif3a, exifIsp);

	batchExport({ {
		.id = dumpId,
		.requestNumber = requestNumber,
		.frameNumber = frameNumber,
		.sensorId = sensorId_,
		.timestamp = sessionTimestamp_,
		.workPath = currentExportPath_,
		.frame = std::nullopt,
		.array = exifArray,
		.metadata = kDumpMetadata.at(dumpId),
		.config = dumpConfig_[dumpId],
	} });
}

void OnDeviceTuner::tuneImgsysHalIsp(
	uint32_t internalRequestId,
	uint32_t frameNumber,
	mtk::isphal::v1_0::TuningParamDip &tuningParam,
	mtk::isphal::v1_0::ReturnParamDip &tuningResult,
	mtk::hal3a::v1_0::mtk_3a_result &mtk3AResult,
	EStage_T stage, Feature feature)
{
	if (!enabled_) {
		return;
	}
	tuningParam.cam_info.rNdd_info.ndd_data.stage = stage;
	tuningParam.cam_info.sr_para.decision_param.staticInfo.sensorId =
		static_cast<int32_t>(ImagiqAdapter::sensorIdMap.at(sensorId_));
	tuningParam.cam_info.rNdd_info.ndd_data.action =
		static_cast<int>(Action::Preview);
	tuningParam.is_need_exif = 1;
	tuningParam.exif_3a.size = sizeof(AAA_DEBUG_INFO1_T);
	tuningParam.exif_3a.data =
		reinterpret_cast<uint8_t *>(&mtk3AResult.debug_3a_info);

	tuningResult.exif.valid = true;
	tuningResult.exif.size = sizeof(AAA_DEBUG_INFO2_T);
	tuningResult.exif.data =
		reinterpret_cast<uint8_t *>(&mtk3AResult.debug_isp_info);

	ImagiqAdapter::writeScenarioRecorderSettings(
		tuningParam.cam_info.sr_para, getMtkMetadata(internalRequestId),
		sessionTimestamp_, internalRequestId, frameNumber, stage, sensorId_);

	parseHalIspNdd(internalRequestId, frameNumber, tuningParam.cam_info.rNdd_info, feature);
}

void OnDeviceTuner::tuneImgsysMetadata(
	uint32_t internalRequestId,
	uint32_t frameNumber,
	int layer,
	const std::vector<PEU_Stage> &stages,
	const InfoFrame &metaFrame,
	int mediaRequestFd)
{
	if (!enabled_) {
		return;
	}
	ctrl_meta_t *imgSysMetadata =
		reinterpret_cast<ctrl_meta_t *>(metaFrame.address(0));
	for (size_t i = 0; i < stages.size(); i++) {
		if (kPeuStageDumpIdMap.count(stages[i]) == 0) {
			LOG(MtkISP7, Error) << "Unrecognized stageEnum: "
					    << stages[i];
			continue;
		}
		// Capture must always export dump.
		if (!shouldExportDumpNow(internalRequestId) &&
		    !isImgsysCaptureStage(stages[i])) {
			continue;
		}
		Dump::Id id = kPeuStageDumpIdMap.at(stages[i]);
		Dump::Metadata dumpMetadata = kDumpMetadata.at(id);
		Dump::Config config = dumpConfig_[id];
		if (layer != -1) {
			dumpMetadata.layer = layer;
		}
		imgSysMetadata[i].common.needDump = true;
		auto dumpFileName = ImagiqAdapter::getDumpFileName({
			.id = id,
			.requestNumber = internalRequestId,
			.frameNumber = frameNumber,
			.sensorId = sensorId_,
			.timestamp = sessionTimestamp_,
			.workPath = currentExportPath_,
			.frame = std::nullopt,
			.array = std::nullopt,
			.metadata = dumpMetadata,
			.config = config,
		});
		strncpy(imgSysMetadata[i].common.nddfp, dumpFileName.c_str(),
			dumpFileName.size());
		LOG(MtkISP7, Info) << "Requested imgsys driver to dump register --"
				   << " request number: " << internalRequestId
				   << " mediaRequestFd: " << mediaRequestFd
				   << " stage: " << stages[i]
				   << " dump file prefix: " << dumpFileName;
	}
}

void OnDeviceTuner::tuneImgsysDriver(int internalRequestId,
				     int mediaRequestFd,
				     std::vector<PEU_Stage> stages)
{
	if (!enabled_) {
		return;
	}
	bool containsImgsysCaptureStage = false;
	for (PEU_Stage stage : stages) {
		containsImgsysCaptureStage = isImgsysCaptureStage(stage);
		if (containsImgsysCaptureStage) {
			break;
		}
	}
	if (!shouldExportDumpNow(internalRequestId) &&
	    !containsImgsysCaptureStage) {
		return;
	}

	imgsysDebug_.exportDump(mediaRequestFd, stages.size());
}

void OnDeviceTuner::tune3ARequest(
	uint32_t internalRequestId,
	mtk::hal3a::v1_0::mtk_3a_request &aaaRequest,
	Feature feature)
{
	uint32_t requestNumber = internalRequestId;
	bool isStillCapture = isStillCaptureRequest(requestNumber);
	if (!enabled_ || (!shouldExportDumpNow(requestNumber) && !isStillCapture)) {
		return;
	}
	aaaRequest.ndd_data.timestamp = sessionTimestamp_;
	aaaRequest.ndd_data.requestNo = internalRequestId;
	aaaRequest.ndd_data.frameNo = internalRequestId;
	aaaRequest.ndd_data.platform = 8188;
	aaaRequest.ndd_data.feature = static_cast<int>(feature);
	if (isStillCapture) {
		aaaRequest.ndd_category = NSCam::TuningUtils::eCategory::kCAPTURE;
	} else {
		aaaRequest.ndd_category = NSCam::TuningUtils::eCategory::kSTREAMING;
	}
}

void OnDeviceTuner::tune3AState(uint32_t internalRequestId,
				FrameBuffer *statistics0,
				mtk::hal3a::v1_0::mtk_3a_result *mtk3AResult)
{
	uint32_t requestNumber = internalRequestId;
	bool isStillCapture = isStillCaptureRequest(requestNumber);
	if (!enabled_ || (!shouldExportDumpNow(requestNumber) && !isStillCapture)) {
		return;
	}
	MappedFrameBuffer mapped(
		statistics0, MappedFrameBuffer::MapFlag::Read);
	mtk_cam_uapi_meta_raw_stats_0 *stats =
		reinterpret_cast<mtk_cam_uapi_meta_raw_stats_0 *>(
			mapped.planes()[0].data());
	std::vector<uint8_t> merged2AHist;
	ImagiqAdapter::merge2AHistogram(merged2AHist, stats);
	std::vector<uint8_t> data(stats->ae_awb_stats.aao_buf.size);
	std::memcpy(data.data(), reinterpret_cast<uint8_t *>(stats) + stats->ae_awb_stats.aao_buf.offset, data.size());

	std::vector<NamedPointer> namedPointers{
		// 3A Statistics
		{
			.id = Dump::Id::P1_AAO,
			.ptr = reinterpret_cast<uint8_t *>(stats) + stats->ae_awb_stats.aao_buf.offset,
			.size = stats->ae_awb_stats.aao_buf.size },
		{ .id = Dump::Id::P1_AAHO,
		  .ptr = merged2AHist.data(),
		  .size = merged2AHist.size() },
		{ .id = Dump::Id::P1_TSFSO_R1,
		  .ptr = reinterpret_cast<uint8_t *>(stats) + stats->tsf_stats.tsfo_r1_buf.offset,
		  .size = stats->tsf_stats.tsfo_r1_buf.size },
		{ .id = Dump::Id::P1_TSFSO_R2,
		  .ptr = reinterpret_cast<uint8_t *>(stats) + stats->tsf_stats.tsfo_r2_buf.offset,
		  .size = stats->tsf_stats.tsfo_r2_buf.size },
		{ .id = Dump::Id::P1_LTMSO,
		  .ptr = reinterpret_cast<uint8_t *>(stats) + stats->ltm_stats.ltmso_buf.offset,
		  .size = stats->ltm_stats.ltmso_buf.size },
		{ .id = Dump::Id::P1_TNCSYO_R1,
		  .ptr = reinterpret_cast<uint8_t *>(stats) + stats->tncy_stats.tncsyo_buf.offset,
		  .size = stats->tncy_stats.tncsyo_buf.size },
		// 3A Result
		{
			.id = Dump::Id::P1_LTM_OUT,
			.ptr = reinterpret_cast<uint8_t *>(mtk3AResult->tone_result.p_ltm_alg_data),
			.size = static_cast<size_t>(mtk3AResult->tone_result.ltm_alg_data_size),
		},
		{
			.id = Dump::Id::P1_AE_OUT,
			.ptr = reinterpret_cast<uint8_t *>(mtk3AResult->ae_result.p_ae_alg_data),
			.size = static_cast<size_t>(mtk3AResult->ae_result.ae_alg_data_size),
		},
		{
			.id = Dump::Id::P1_FW_ME_TCY_P,
			.ptr = reinterpret_cast<uint8_t *>(mtk3AResult->tone_result.p_me_tcy_in_workbuf_data),
			.size = static_cast<size_t>(mtk3AResult->tone_result.me_tcy_in_workbuf_data_size),
		},
		{
			.id = Dump::Id::P1_FW_ME_TCY_O,
			.ptr = reinterpret_cast<uint8_t *>(mtk3AResult->tone_result.p_me_tcy_fst_o_data),
			.size = static_cast<size_t>(mtk3AResult->tone_result.me_tcy_fst_o_data_size),
		}
	};
	LOG(MtkISP7, Debug) << "AE_OUT size: " << mtk3AResult->ae_result.ae_alg_data_size;
	LOG(MtkISP7, Debug) << "FW_ME_TCY_P size: " << mtk3AResult->tone_result.me_tcy_in_workbuf_data_size;
	LOG(MtkISP7, Debug) << "FW_ME_TCY_O size: " << mtk3AResult->tone_result.me_tcy_fst_o_data_size;
	tune(requestNumber, namedPointers, isStillCapture);
}

void OnDeviceTuner::tuneMeA(uint32_t internalRequestId, MeFrames &frames)
{
	uint32_t requestNumber = internalRequestId;
	if (!enabled_ || (!shouldExportDumpNow(requestNumber) &&
			  !shouldImportDumpNow(requestNumber))) {
		return;
	}
	tune(
		internalRequestId, {
					   { Dump::Id::LTR_ME_L1_IMGI_T1, frames.in.meL0->get() },
					   { Dump::Id::LTR_ME_L1_YUVO_T2, frames.out.meL1->get() },
					   { Dump::Id::LTR_ME_L1_META_P2, frames.in.trMeTun->get() },
					   { Dump::Id::ME_3PASS_MODE0_MEI_L0, frames.in.meL0->get() },
					   { Dump::Id::ME_3PASS_MODE0_MEI_L0_P, frames.in.prevMeL0->get() },
					   { Dump::Id::ME_3PASS_MODE0_MEI_L1, frames.out.meL1->get() },
					   { Dump::Id::ME_3PASS_MODE0_MEI_L1_P, frames.in.prevMeL1->get() },
					   { Dump::Id::ME_3PASS_MODE0_MV_L1_M0_P, frames.in.prevMeAMv1->get() },
					   { Dump::Id::ME_3PASS_MODE0_MV_L0_M1_P, frames.in.prevMeBMv0->get() },
					   { Dump::Id::ME_3PASS_MODE0_CONF_MAP, frames.out.meConf0->get() },
					   { Dump::Id::ME_3PASS_MODE0_MV_L0, frames.out.meAMv0->get() },
					   { Dump::Id::ME_3PASS_MODE0_MV_L1, frames.out.meAMv1->get() },
					   { Dump::Id::ME_3PASS_MODE0_FMB_L0, frames.out.meAFmb0->get() },
					   { Dump::Id::ME_3PASS_MODE0_FMB_L1, frames.out.meAFmb1->get() },
					   { Dump::Id::ME_3PASS_MODE0_FST, frames.out.meAFst->get() },
					   { Dump::Id::ME_3PASS_MODE0_META_P2, frames.in.meATun->get() },

					   /* The following are input of ME_3PASS_MODE1 and output of ME_3PASS_MODE0.
					      * Because it will be overwriten by ME_3PASS_MODE1, it should be dumped right
					      * after ME_3PASS_MODE0. */
					   { Dump::Id::ME_3PASS_MODE1_MV_L0_M0, frames.out.meAMv0->get() }, // confirmed
					   { Dump::Id::ME_3PASS_MODE1_FMB_L1_M0, frames.out.meAFmb1->get() }, // ?
				   });
}

void OnDeviceTuner::tuneMeB(uint32_t internalRequestId, MeBFrames &frames)
{
	uint32_t requestNumber = internalRequestId;
	if (!enabled_ || (!shouldExportDumpNow(requestNumber) &&
			  !shouldImportDumpNow(requestNumber))) {
		return;
	}
	tune(
		internalRequestId, {
					   { Dump::Id::ME_3PASS_MODE1_MEI_L0, frames.in.meL0->get() },
					   { Dump::Id::ME_3PASS_MODE1_MEI_L0_P, frames.in.prevMeL0->get() },
					   { Dump::Id::ME_3PASS_MODE1_MEI_L1_P, frames.in.prevMeL1->get() },
					   { Dump::Id::ME_3PASS_MODE1_MIL, frames.in.meMil->get() },
					   { Dump::Id::ME_3PASS_MODE1_MMAP, frames.out.meMmap[0]->get() },
					   { Dump::Id::ME_3PASS_MODE1_CONF_MAP, frames.out.meConf0->get() },
					   { Dump::Id::ME_3PASS_MODE1_MV_L0, frames.out.meBMv0->get() }, // ??
					   { Dump::Id::ME_3PASS_MODE1_FMB_L0, frames.out.meBFmb0->get() }, // ?
					   { Dump::Id::ME_3PASS_MODE1_LMI, frames.out.meBLmi->get() },
					   { Dump::Id::ME_3PASS_MODE1_FST, frames.out.meBFst->get() },
					   { Dump::Id::ME_3PASS_MODE1_META_P2, frames.in.meBTun->get() },
				   });
}

void OnDeviceTuner::tuneMeMM(uint32_t internalRequestId,
			     SharedMailBox<InfoFrame> tuning)
{
	uint32_t requestNumber = internalRequestId;
	if (!enabled_ || (!shouldExportDumpNow(requestNumber) &&
			  !shouldImportDumpNow(requestNumber))) {
		return;
	}
	tune(
		internalRequestId, {
					   { Dump::Id::ME_3PASS_MM_META_P2, tuning->get() },
				   });
}

void OnDeviceTuner::tuneTr(uint32_t internalRequestId, TrFrames &frames)
{
	uint32_t requestNumber = internalRequestId;
	if (!enabled_ || (!shouldExportDumpNow(requestNumber) &&
			  !shouldImportDumpNow(requestNumber))) {
		return;
	}
	tune(
		internalRequestId, { { Dump::Id::TR_DSMAP_MMAP, frames.in.meMmap[0]->get() },
				     { Dump::Id::TR_DSMAP_MMAP_DS0, frames.in.meMmap[1]->get() },
				     { Dump::Id::TR_DSMAP_MMAP_DS1, frames.in.meMmap[2]->get() },
				     { Dump::Id::TR_DSMAP_MMAP_DS2, frames.in.meMmap[3]->get() },
				     { Dump::Id::TR_Y2Y_F1_IMGI_T1, frames.in.p1F1->get() },
				     { Dump::Id::TR_Y2Y_F1_YUVO_T2, frames.out.dipImgi[2]->get() },
				     { Dump::Id::TR_Y2Y_F1_YUVO_T3, frames.out.dipImgi[3]->get() },
				     { Dump::Id::TR_Y2Y_F1_YUVO_T4, frames.out.dipImgi[4]->get() },
				     { Dump::Id::TR_Y2Y_F1_META_P2, frames.in.trTunF1->get() },
				     { Dump::Id::TR_Y2Y_F4_IMGI_T1, frames.out.dipImgi[4]->get() },
				     { Dump::Id::TR_Y2Y_F4_YUVO_T2, frames.out.dipImgi[5]->get() },
				     { Dump::Id::TR_Y2Y_F4_YUVO_T3, frames.out.dipImgi[6]->get() },
				     { Dump::Id::TR_Y2Y_F4_META_P2, frames.in.trTunF4->get() },
				     { Dump::Id::TR_Y2Y_Conf_IMGI_T1, frames.in.meConf0->get() },
				     { Dump::Id::TR_Y2Y_Conf_F4_YUVO_T5, frames.out.meConf4->get() },
				     { Dump::Id::TR_Y2Y_Conf_F5_YUVO_T5, frames.out.meConf5->get() } });
}

void OnDeviceTuner::tuneDip1(uint32_t internalRequestId, Dip1Frames &frames)
{
	uint32_t requestNumber = internalRequestId;
	if (!enabled_ || (!shouldExportDumpNow(requestNumber) &&
			  !shouldImportDumpNow(requestNumber))) {
		return;
	}
	std::vector<NamedFrame> namedFrames{
		{ Dump::Id::WPE_LTR_Y2Y_F1_WPEI, frames.in.prevImg4oF1->get() },
		{ Dump::Id::WPE_LTR_Y2Y_F1_WPE_MAP, frames.out.meMmap[0]->get() },
		{ Dump::Id::WPE_LTR_Y2Y_F1_WPEO, frames.out.dipVipi[1]->get() },
		{ Dump::Id::WPE_LTR_Y2Y_F1_YUVO_T2, frames.out.dipVipi[2]->get() },
		{ Dump::Id::WPE_LTR_Y2Y_F1_YUVO_T3, frames.out.dipVipi[3]->get() },
		{ Dump::Id::WPE_LTR_Y2Y_F1_YUVO_T4, frames.out.dipVipi[4]->get() },
		{ Dump::Id::WPE_LTR_Y2Y_F1_YUVO_T5, frames.out.dipVbi[2]->get() },
		{ Dump::Id::WPE_LTR_Y2Y_F1_META_P2, frames.in.ltrTunF1->get() },
		{ Dump::Id::LTR_VBI_IMGI_T1, frames.out.dipVbi[2]->get() },
		{ Dump::Id::LTR_VBI_YUVO_T2, frames.out.dipVbi[3]->get() },
		{ Dump::Id::LTR_VBI_YUVO_T3, frames.out.dipVbi[4]->get() },
		{ Dump::Id::LTR_VBI_YUVO_T4, frames.out.dipVbi[5]->get() },
		{ Dump::Id::LTR_VBI_META_P2, frames.in.ltrTunVbi->get() },
		{ Dump::Id::LTR_Y2Y_F4_IMGI_T1, frames.out.dipVipi[4]->get() },
		{ Dump::Id::LTR_Y2Y_F4_YUVO_T2, frames.out.dipVipi[5]->get() },
		{ Dump::Id::LTR_Y2Y_F4_YUVO_T3, frames.out.dipVipi[6]->get() },
		{ Dump::Id::LTR_Y2Y_F4_META_P2, frames.in.ltrTunF4->get() },
		{ Dump::Id::WPE_WghtMap_META_P2, frames.in.wpeTun->get() },
		{ Dump::Id::P2_MS_F1_IMG4O, frames.out.img4oF1->get() },
		{ Dump::Id::P2_MS_F_SMALL_RECI_D1, frames.out.dipImgi[6]->get() }
	};

	// Stage WPE_WghtMap_WPEI_F0 until WPE_WghtMap_WPEI_F5
	for (int level = 0; level <= 5; level++) {
		namedFrames.push_back({ Dump::kWpeInputImageDumpIds[level],
					frames.in.prevDipTnrwo[level]->get() });
		namedFrames.push_back({ Dump::kWpeWeightMapDumpIds[level],
					frames.out.wpeVeci[level]->get() });
		namedFrames.push_back({ Dump::kWpeOutputImageDumpIds[level],
					frames.out.dipTnrwi[level]->get() });
	}

	// Stage P2_MS_F1 until P2_MS_F4 + P2_MS_F_SMALL (lv5) + P2_IDI (lv6)
	for (int i = 0; i < 6; i++) {
		int level = i + 1;
		namedFrames.push_back({ Dump::kDip1ImgiDumpIds[i],
					frames.out.dipImgi[level]->get() });
		namedFrames.push_back({ Dump::kDip1VipiDumpIds[i],
					frames.out.dipVipi[level]->get() });
		// P2_IDI_TNRSI uses previous frame's tnrso
		if (level == 6)
			namedFrames.push_back({ Dump::kDip1TnrsiDumpIds[i],
						frames.in.preDipTnrso->get() });
		else
			namedFrames.push_back({ Dump::kDip1TnrsiDumpIds[i],
						frames.out.dipTnrso->get() });

		namedFrames.push_back({ Dump::kDip1TnrsoDumpIds[i],
					frames.out.dipTnrso->get() });
		if (Dump::kDip1Img3oDumpIds[i] == Dump::Id::P2_IDI_IMG3O)
			namedFrames.push_back({ Dump::Id::P2_IDI_IMG3O,
						frames.out.tnrlfdi->get() });
		else
			namedFrames.push_back({ Dump::kDip1Img3oDumpIds[i],
						frames.out.img3o[level]->get() });
		namedFrames.push_back({ Dump::kDip1MetaP2DumpIds[i],
					frames.in.dipTun[level]->get() });
	}

	// Stage P2_MS_F1 until P2_MS_F4 + P2_MS_F_SMALL (lv5)
	for (int i = 0; i < 5; i++) {
		int level = i + 1;
		namedFrames.push_back({ Dump::kDip1TnrwiDumpIds[i],
					frames.out.dipTnrwi[level]->get() });
		namedFrames.push_back({ Dump::kDip1TnrciDumpIds[i],
					frames.out.dipTnrci[level]->get() });
		namedFrames.push_back({ Dump::kDip1TnrliDumpIds[i],
					frames.out.tnrlfdi->get() });
		namedFrames.push_back({ Dump::kDip1TnrvbiDumpIds[i],
					frames.out.dipVbi[level]->get() });
		namedFrames.push_back({ Dump::kDip1TnrmoDumpIds[i],
					frames.out.dipTnrmo[level]->get() });
		namedFrames.push_back({ Dump::kDip1TnrwoDumpIds[i],
					frames.out.dipTnrwo[level]->get() });
	}

	// Stage P2_MS_F1 until P2_MS_F4
	for (int i = 0; i < 4; i++) {
		int level = i + 1;
		namedFrames.push_back({ Dump::kDip1ReciDumpIds[i],
					frames.out.reci[level]->get() });
		namedFrames.push_back({ Dump::kDip1TnrmiDumpIds[i],
					frames.out.dipTnrmi[level]->get() });
	}
	tune(internalRequestId, namedFrames);
}

void OnDeviceTuner::tuneDip2(
	Request *request, uint32_t internalRequestId, Dip2Frames &frames,
	FrameBuffer *videoOut1, FrameBuffer *videoOut2)
{
	uint32_t requestNumber = internalRequestId;
	if (!enabled_ || (!shouldExportDumpNow(requestNumber) &&
			  !shouldImportDumpNow(requestNumber))) {
		return;
	}
	std::vector<NamedFrame> namedFrames{
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_WPETI, frames.in.prevImg4oF0->get() },
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_WPET_MAP, frames.in.meMmap[0]->get() },
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_TNRSI, frames.out.dipTnrso->get() },
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_TNRWI, frames.in.dipTnrwi[0]->get() },
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_TNRMI, frames.in.dipTnrmi[0]->get() },
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_TNRCI, frames.in.dipTnrci[0]->get() },
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_TNRLI, frames.in.tnrlfdi->get() },
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_TNRSO, frames.out.dipTnrso->get() },
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_TNRWO, frames.out.dipTnrwo[0]->get() },
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_RECI_D1, frames.in.reci[0]->get() },
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_IMG3O, frames.out.img3o[0]->get() },
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_IMG4O, frames.out.img4oF0->get() },
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_IMGI_D1, frames.in.dipImgi[0]->get() },
		{ Dump::Id::WPE_P2_PQDIP_MS_F0_META_P2, frames.in.dipTun[0]->get() },
	};

	if (videoOut1) {
		InfoFrame video1 = getFrameInfoFromRequest(request, videoOut1);
		namedFrames.push_back({ Dump::Id::WPE_P2_PQDIP_MS_F0_WDMAO, video1 });
	}
	if (videoOut2) {
		InfoFrame video2 = getFrameInfoFromRequest(request, videoOut2);
		namedFrames.push_back({ Dump::Id::WPE_P2_PQDIP_MS_F0_WDMAO, video2 });
	}
	tune(internalRequestId, namedFrames);
}

void OnDeviceTuner::tuneXtr(uint32_t internalRequestId, XtrFrames &frames)
{
	if (!enabled_) {
		return;
	}
	// Capture: always export dumps!
	std::vector<NamedFrame> namedFrames{
		{ Dump::Id::TR_R2Y_IMGI_T1, frames.in.p1Raw->get() },
		{ Dump::Id::TR_R2Y_YUVO_T1, frames.out.dipImgi[0]->get() },
		{ Dump::Id::TR_R2Y_YUVO_T2, frames.out.dipImgi[1]->get() },
		{ Dump::Id::TR_R2Y_YUVO_T3, frames.out.dipImgi[2]->get() },
		{ Dump::Id::TR_R2Y_YUVO_T4, frames.out.dipImgi[3]->get() },
		{ Dump::Id::TR_R2Y_META_P2, frames.in.xtrTun->get() },
	};
	tune(internalRequestId, namedFrames, true);
}

void OnDeviceTuner::tuneLpnrDip(Request *request, uint32_t internalRequestId,
				LpnrDipFrames &frames,
				std::vector<SharedMailBox<InfoFrame>> reci,
				std::vector<SharedMailBox<InfoFrame>> dipImg3o,
				FrameBuffer *still1Output,
				FrameBuffer *still2Output)
{
	if (!enabled_) {
		return;
	}
	// Capture: always export dumps!
	std::vector<NamedFrame> namedFrames{
		{ Dump::Id::P2_MS_F3_IMGI_D1_LPNR, frames.in.dipImgi[3]->get() },
		{ Dump::Id::P2_MS_F3_IMG3O_LPNR, dipImg3o[3]->get() },
		{ Dump::Id::P2_MS_F3_META_P2_LPNR, frames.in.dipTun[3]->get() },
		{ Dump::Id::P2_MS_F2_IMGI_D1_LPNR, frames.in.dipImgi[2]->get() },
		{ Dump::Id::P2_MS_F2_RECI_D1_LPNR, reci[2]->get() },
		{ Dump::Id::P2_MS_F2_IMG3O_LPNR, dipImg3o[2]->get() },
		{ Dump::Id::P2_MS_F2_META_P2_LPNR, frames.in.dipTun[2]->get() },
		{ Dump::Id::P2_MS_F1_IMGI_D1_LPNR, frames.in.dipImgi[1]->get() },
		{ Dump::Id::P2_MS_F1_RECI_D1_LPNR, reci[1]->get() },
		{ Dump::Id::P2_MS_F1_IMG3O_LPNR, dipImg3o[1]->get() },
		{ Dump::Id::P2_MS_F1_META_P2_LPNR, frames.in.dipTun[1]->get() },
	};

	if ((frames.in.highIsoMode->valid() && !frames.in.highIsoMode->get())) {
		namedFrames.push_back({ Dump::Id::P2_MS_F0_PQ_DIP_IMGI_D1, frames.in.dipImgi[0]->get() });
		namedFrames.push_back({ Dump::Id::P2_MS_F0_PQ_DIP_RECI_D1, reci[0]->get() });
		namedFrames.push_back({ Dump::Id::P2_MS_F0_PQ_DIP_IMG3O, dipImg3o[0]->get() });

		if (still1Output) {
			InfoFrame still1Frame = getFrameInfoFromRequest(request, still1Output);
			namedFrames.push_back({ Dump::Id::P2_MS_F0_PQ_DIP_WDMAO, still1Frame });
		}

		if (still2Output) {
			InfoFrame still2Frame = getFrameInfoFromRequest(request, still2Output);
			namedFrames.push_back({ Dump::Id::P2_MS_F0_PQ_DIP_WDMAO, still2Frame });
		}

		namedFrames.push_back({ Dump::Id::P2_MS_F0_PQ_DIP_META_P2, frames.in.dipTunPq->get() });
	} else {
		namedFrames.push_back({ Dump::Id::P2_MS_F0_H_IMGI_D1, frames.in.dipImgi[0]->get() });
		namedFrames.push_back({ Dump::Id::P2_MS_F0_H_RECI_D1, reci[0]->get() });
		namedFrames.push_back({ Dump::Id::P2_MS_F0_H_IMG3O, dipImg3o[0]->get() });
		namedFrames.push_back({ Dump::Id::P2_MS_F0_H_META_P2, frames.in.dipTun[0]->get() });

		if (still1Output) {
			InfoFrame still1Frame = getFrameInfoFromRequest(request, still1Output);
			namedFrames.push_back({ Dump::Id::P2_Y2Y_PQ_DIP_WDMAO, still1Frame });
		}

		if (still2Output) {
			InfoFrame still2Frame = getFrameInfoFromRequest(request, still2Output);
			namedFrames.push_back({ Dump::Id::P2_Y2Y_PQ_DIP_WDMAO, still2Frame });
		}

		namedFrames.push_back({ Dump::Id::P2_Y2Y_PQ_DIP_IMG3O, dipImg3o[0]->get() });
		namedFrames.push_back({ Dump::Id::P2_Y2Y_PQ_DIP_META_P2, frames.in.dipTunY2YPq->get() });
	}

	tune(internalRequestId, namedFrames, true);
}

void OnDeviceTuner::tuneBss(uint32_t internalRequestId, [[maybe_unused]] BssFrames &frames, int frameCount)
{
	if (!enabled_) {
		return;
	}
	std::vector<NamedFrame> namedFramesMain;
	namedFramesMain.push_back({ Dump::Id::BSS_PARAM, frames.in.bssParamInfo->get() });
	namedFramesMain.push_back({ Dump::Id::BSS_DATAG, frames.in.bssDataGInfo->get() });
	namedFramesMain.push_back({ Dump::Id::BSS_TUNING, frames.in.bssTuningInfo->get() });
	namedFramesMain.push_back({ Dump::Id::BSS_VER, frames.in.bssVerInfo->get() });
	namedFramesMain.push_back({ Dump::Id::BSS_OUT_DATA, frames.out.bssOutDataInfo->get() });
	tune(internalRequestId, internalRequestId + frameCount - 1, namedFramesMain, true);
	for (int i = 0; i < frameCount; i++) {
		std::vector<NamedFrame> namedFrames;
		namedFrames.push_back({ Dump::Id::BSS_IMGI, frames.in.imgi[i]->get() });
		namedFrames.push_back({ Dump::Id::BSS_FDMAIN, frames.in.bssFdMainInfo[i]->get() });
		namedFrames.push_back({ Dump::Id::BSS_FD, frames.in.bssFdInfo[i]->get() });
		namedFrames.push_back({ Dump::Id::BSS_FACE, frames.in.bssFaceInfo[i]->get() });
		namedFrames.push_back({ Dump::Id::BSS_POS, frames.in.bssPosInfo[i]->get() });

		tune(internalRequestId, internalRequestId + i, namedFrames, true);
	}
}

void OnDeviceTuner::tuneBfbld(uint32_t internalRequestId, BfbldFrames &frames, std::vector<int> order)
{
	if (!enabled_) {
		return;
	}
	// Capture: always export dumps!
	for (int i = 0; i < (int)order.size(); i++) {
		std::vector<NamedFrame> namedFrames;
		if (i == 0) {
			namedFrames.push_back({ Dump::Id::BFBLD_BASE_TIMGI, frames.in.timgi[i]->get() });
			namedFrames.push_back({ Dump::Id::BFBLD_BASE_TUNBUF, frames.in.tunbufi[i]->get() });
			namedFrames.push_back({ Dump::Id::BFBLD_BASE_IMG2O, frames.out.img2o[i]->get() });
			namedFrames.push_back({ Dump::Id::BFBLD_BASE_IMG3O, frames.out.img3o[i]->get() });
		} else {
			namedFrames.push_back({ Dump::Id::BFBLD_REF_TIMGI, frames.in.timgi[i]->get() });
			namedFrames.push_back({ Dump::Id::BFBLD_REF_TUNBUF, frames.in.tunbufi[i]->get() });
			namedFrames.push_back({ Dump::Id::BFBLD_REF_IMG2O, frames.out.img2o[i]->get() });
			namedFrames.push_back({ Dump::Id::BFBLD_REF_IMG3O, frames.out.img3o[i]->get() });
		}
		tune(internalRequestId, internalRequestId + order[i], namedFrames, true);
	}
}
void OnDeviceTuner::tuneBfme(uint32_t internalRequestId, BfmeFrames &frames, std::vector<int> order)
{
	if (!enabled_) {
		return;
	}
	// Capture: always export dumps!
	LOG(MtkISP7, Info) << "tuneBfme";
	for (int i = 0; i < (int)order.size(); i++) {
		LOG(MtkISP7, Info) << "tuneBfme " << internalRequestId + order[i];
		std::vector<NamedFrame> namedFrames;
		namedFrames.push_back({ Dump::Id::BFME_IMGI, frames.in.imgi[i]->get() });
		namedFrames.push_back({ Dump::Id::BFME_TUNBUF, frames.in.tunbufi[i]->get() });
		namedFrames.push_back({ Dump::Id::BFME_IMG2O, frames.out.img2o[i]->get() });

		tune(internalRequestId, internalRequestId + order[i], namedFrames, true);
	}
}
void OnDeviceTuner::tuneSwme(uint32_t internalRequestId, SwmeFrames &frames, std::vector<int> order)
{
	if (!enabled_) {
		return;
	}
	// Capture: always export dumps!
	for (int i = 0; i < (int)order.size() - 1; i++) {
		std::vector<NamedFrame> namedFrames;
		namedFrames.push_back({ Dump::Id::SWME_IN_BASE, frames.in.base_buf[i]->get() });
		namedFrames.push_back({ Dump::Id::SWME_IN_REF, frames.in.ref_buf[i]->get() });
		namedFrames.push_back({ Dump::Id::SWME_PARAM, frames.in.paramInInfo[i]->get() });
		namedFrames.push_back({ Dump::Id::SWME_TUNING, frames.in.tuningInfo[i]->get() });
		namedFrames.push_back({ Dump::Id::SWME_OUT, frames.out.paramOutInfo[i]->get() });
		namedFrames.push_back({ Dump::Id::SWME_CONF_MAP, frames.out.conf_map[i]->get() });
		namedFrames.push_back({ Dump::Id::SWME_WPEX_MAP, frames.out.warpping_map[i]->get() });

		tune(internalRequestId, internalRequestId + order[i + 1], namedFrames, true);
	}
}
void OnDeviceTuner::tuneDs(uint32_t internalRequestId, DsFrames &frames, std::vector<int> order)
{
	if (!enabled_) {
		return;
	}
	// Capture: always export dumps!
	for (int i = 0; i < (int)order.size() + 1; i++) {
		std::vector<NamedFrame> namedFrames;
		namedFrames.push_back({ Dump::Id::DS_IMGI_T1, frames.in.ltimgi[i]->get() });
		namedFrames.push_back({ Dump::Id::DS_TUNBUF, frames.in.tunbufi[i]->get() });
		namedFrames.push_back({ Dump::Id::DS_YUVO_T2, frames.out.ltyuv2o[i]->get() });
		namedFrames.push_back({ Dump::Id::DS_YUVO_T3, frames.out.ltyuv3o[i]->get() });
		namedFrames.push_back({ Dump::Id::DS_YUVO_T4, frames.out.ltyuv4o[i]->get() });
		if (i == 0 || i == 1) {
			tune(internalRequestId, internalRequestId + order[0], namedFrames, true);
		} else {
			tune(internalRequestId, internalRequestId + order[i - 1], namedFrames, true);
		}
	}
}

void OnDeviceTuner::tuneDsVbi(uint32_t internalRequestId, DsVbiFrames &ds_vbi_v2, DsVbiFrames &ds_vbi_v5, std::vector<int> order)
{
	if (!enabled_) {
		return;
	}
	// Capture: always export dumps!
	for (int i = 0; i < (int)order.size() - 1; i++) {
		std::vector<NamedFrame> namedFrames;
		namedFrames.push_back({ Dump::Id::DS_VBI_V2_IMGI_T1, ds_vbi_v2.in.timgi[i]->get() });
		namedFrames.push_back({ Dump::Id::DS_VBI_V2_TUNBUF, ds_vbi_v2.in.tunbufi[i]->get() });
		namedFrames.push_back({ Dump::Id::DS_VBI_V2_YUVO_T2, ds_vbi_v2.out.tyuv2o[i]->get() });
		namedFrames.push_back({ Dump::Id::DS_VBI_V2_YUVO_T3, ds_vbi_v2.out.tyuv3o[i]->get() });
		namedFrames.push_back({ Dump::Id::DS_VBI_V2_YUVO_T4, ds_vbi_v2.out.tyuv4o[i]->get() });
		namedFrames.push_back({ Dump::Id::DS_VBI_V5_IMGI_T1, ds_vbi_v5.in.timgi[i]->get() });
		namedFrames.push_back({ Dump::Id::DS_VBI_V5_TUNBUF, ds_vbi_v5.in.tunbufi[i]->get() });
		namedFrames.push_back({ Dump::Id::DS_VBI_V5_YUVO_T2, ds_vbi_v5.out.tyuv2o[i]->get() });
		tune(internalRequestId, internalRequestId + order[i + 1], namedFrames, true);
	}
}

void OnDeviceTuner::tuneMcdsF1(uint32_t internalRequestId, McdsF1Frames &frames, std::vector<int> order)
{
	if (!enabled_) {
		return;
	}
	// Capture: always export dumps!
	for (int i = 1; i < (int)order.size(); i++) {
		int idx = i - 1;
		std::vector<NamedFrame> namedFrames;
		namedFrames.push_back({ Dump::Id::MCDSF1_WPE_WPEI, frames.in.wpe_wpei[idx]->get() });
		namedFrames.push_back({ Dump::Id::MCDSF1_WPE_VCEI, frames.in.wpe_veci[idx]->get() });
		namedFrames.push_back({ Dump::Id::MCDSF1_TUNBUF, frames.in.tunbufi[idx]->get() });
		namedFrames.push_back({ Dump::Id::MCDSF1_WPE_WPEO, frames.out.wpe_wpeo[idx]->get() });
		namedFrames.push_back({ Dump::Id::MCDSF1_LTYUV2O, frames.out.ltyuv2o[idx]->get() });
		namedFrames.push_back({ Dump::Id::MCDSF1_LTYUV3O, frames.out.ltyuv3o[idx]->get() });
		namedFrames.push_back({ Dump::Id::MCDSF1_LTYUV4O, frames.out.ltyuv4o[idx]->get() });
		namedFrames.push_back({ Dump::Id::MCDSF1_LTYUV5O, frames.out.ltyuv5o[idx]->get() });

		tune(internalRequestId, internalRequestId + order[i], namedFrames, true);
	}
}

void OnDeviceTuner::tuneMsbld(
	uint32_t internalRequestId, MsbldFrames msbldF0_, MsbldFrames msbldF1_, MsbldFrames msbldF2_,
	MsbldFrames msbldF3_, MsbldFrames msbldF4_, MsbldFrames msbldF5_, MsbldFrames msbldF6_, std::vector<int> order, int msbld_idx)
{
	if (!enabled_) {
		return;
	}

	int i = msbld_idx;
	std::vector<NamedFrame> namedFrames;

	namedFrames.push_back({ Dump::Id::MSBLD_F0_IMGI_D1, msbldF0_.in.imgi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F0_VIPI, msbldF0_.in.vipi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F0_RECI_D1, msbldF0_.in.rec_dsi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F0_TNRCI, msbldF0_.in.tnrci->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F0_TNRMI, msbldF0_.in.tnrmi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F0_TNRLI, msbldF0_.in.tnrlfdi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F0_TNRSI, msbldF6_.in.tnrsi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F0_TNRVBI, msbldF0_.in.tnrvbi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F0_TNRWI, msbldF0_.in.tnrwi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F0_TUNBUF, msbldF0_.in.tunbufi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F0_TNRSO, msbldF0_.out.tnrso->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F0_TNRWO, msbldF0_.out.tnrwo->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F0_IMG4O, msbldF0_.out.img4o->get() });

	namedFrames.push_back({ Dump::Id::MSBLD_F1_IMGI_D1, msbldF1_.in.imgi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F1_VIPI, msbldF1_.in.vipi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F1_RECI_D1, msbldF1_.in.rec_dsi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F1_TNRCI, msbldF1_.in.tnrci->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F1_TNRLI, msbldF1_.in.tnrlfdi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F1_TNRMI, msbldF1_.in.tnrmi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F1_TNRSI, msbldF6_.in.tnrsi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F1_TNRVBI, msbldF1_.in.tnrvbi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F1_TNRWI, msbldF1_.in.tnrwi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F1_TUNBUF, msbldF1_.in.tunbufi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F1_TNRMO, msbldF1_.out.tnrmo->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F1_TNRSO, msbldF0_.out.tnrso->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F1_TNRWO, msbldF1_.out.tnrwo->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F1_IMG4O, msbldF1_.out.img4o->get() });

	namedFrames.push_back({ Dump::Id::MSBLD_F2_IMGI_D1, msbldF2_.in.imgi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F2_VIPI, msbldF2_.in.vipi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F2_RECI_D1, msbldF2_.in.rec_dsi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F2_TNRCI, msbldF2_.in.tnrci->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F2_TNRLI, msbldF2_.in.tnrlfdi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F2_TNRMI, msbldF2_.in.tnrmi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F2_TNRSI, msbldF6_.in.tnrsi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F2_TNRVBI, msbldF2_.in.tnrvbi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F2_TNRWI, msbldF2_.in.tnrwi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F2_TUNBUF, msbldF2_.in.tunbufi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F2_TNRMO, msbldF2_.out.tnrmo->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F2_TNRSO, msbldF0_.out.tnrso->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F2_TNRWO, msbldF2_.out.tnrwo->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F2_IMG4O, msbldF2_.out.img4o->get() });

	namedFrames.push_back({ Dump::Id::MSBLD_F3_IMGI_D1, msbldF3_.in.imgi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F3_VIPI, msbldF3_.in.vipi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F3_RECI_D1, msbldF3_.in.rec_dsi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F3_TNRCI, msbldF3_.in.tnrci->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F3_TNRLI, msbldF3_.in.tnrlfdi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F3_TNRMI, msbldF3_.in.tnrmi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F3_TNRSI, msbldF6_.in.tnrsi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F3_TNRVBI, msbldF3_.in.tnrvbi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F3_TNRWI, msbldF3_.in.tnrwi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F3_TUNBUF, msbldF3_.in.tunbufi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F3_TNRMO, msbldF3_.out.tnrmo->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F3_TNRSO, msbldF0_.out.tnrso->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F3_TNRWO, msbldF3_.out.tnrwo->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F3_IMG4O, msbldF3_.out.img4o->get() });

	namedFrames.push_back({ Dump::Id::MSBLD_F4_IMGI_D1, msbldF4_.in.imgi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F4_VIPI, msbldF4_.in.vipi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F4_RECI_D1, msbldF4_.in.rec_dsi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F4_TNRCI, msbldF4_.in.tnrci->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F4_TNRLI, msbldF4_.in.tnrlfdi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F4_TNRMI, msbldF4_.in.tnrmi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F4_TNRSI, msbldF6_.in.tnrsi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F4_TNRVBI, msbldF4_.in.tnrvbi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F4_TNRWI, msbldF4_.in.tnrwi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F4_TUNBUF, msbldF4_.in.tunbufi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F4_TNRMO, msbldF4_.out.tnrmo->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F4_TNRSO, msbldF0_.out.tnrso->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F4_TNRWO, msbldF4_.out.tnrwo->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F4_IMG4O, msbldF4_.out.img4o->get() });

	namedFrames.push_back({ Dump::Id::MSBLD_F5_IMGI_D1, msbldF5_.in.imgi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F5_VIPI, msbldF5_.in.vipi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F5_RECI_D1, msbldF5_.in.rec_dsi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F5_TNRCI, msbldF5_.in.tnrci->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F5_TNRLI, msbldF5_.in.tnrlfdi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F5_TNRSI, msbldF6_.in.tnrsi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F5_TNRVBI, msbldF5_.in.tnrvbi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F5_TNRWI, msbldF5_.in.tnrwi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F5_TUNBUF, msbldF5_.in.tunbufi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F5_TNRMO, msbldF5_.out.tnrmo->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F5_TNRSO, msbldF0_.out.tnrso->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F5_TNRWO, msbldF5_.out.tnrwo->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F5_IMG4O, msbldF5_.out.img4o->get() });

	namedFrames.push_back({ Dump::Id::MSBLD_F6_IMGI_D1, msbldF6_.in.imgi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F6_VIPI, msbldF6_.in.vipi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F6_TNRSI, msbldF6_.in.tnrsi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F6_TUNBUF, msbldF6_.in.tunbufi->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F6_TNRSO, msbldF0_.out.tnrso->get() });
	namedFrames.push_back({ Dump::Id::MSBLD_F6_IMG4O, msbldF6_.out.img4o->get() });

	tune(internalRequestId, internalRequestId + order[i], namedFrames, true);
}

void OnDeviceTuner::tuneAfbld(
	[[maybe_unused]] Request *request, uint32_t internalRequestId,
	AfbldFrames afbldF0_, AfbldFrames afbldF1_, AfbldFrames afbldF2_,
	AfbldFrames afbldF3_, AfbldFrames afbldF4_, AfbldFrames afbldF5_, AfbldFrames afbldF6_,
	std::vector<int> order, FrameBuffer *still1Output, FrameBuffer *still2Output)
{
	if (!enabled_) {
		return;
	}
	// Capture: always export dumps!

	std::vector<NamedFrame> namedFrames;
	int i = 0;
	namedFrames.push_back({ Dump::Id::AFBLD_F0_IMGI_D1, afbldF0_.in.imgi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F0_VIPI, afbldF0_.in.vipi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F0_RECI_D1, afbldF0_.in.rec_dsi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F0_TNRCI, afbldF0_.in.tnrci[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F0_TNRMI, afbldF0_.in.tnrmi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F0_TNRLI, afbldF0_.in.tnrlfdi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F0_TNRSI, afbldF6_.in.tnrsi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F0_TNRVBI, afbldF0_.in.tnrvbi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F0_TNRWI, afbldF0_.in.tnrwi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F0_TUNBUF, afbldF0_.in.tunbufi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F0_TNRWO, afbldF0_.out.tnrwo[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F0_IMG3O, afbldF0_.out.img3o[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F0_TNRSO, afbldF0_.out.tnrso[i]->get() });

	if (still1Output) {
		InfoFrame still1Frame = getFrameInfoFromRequest(request, still1Output);
		namedFrames.push_back({ Dump::Id::AFBLD_F0_WDMAO, still1Frame });
	}

	if (still2Output) {
		InfoFrame still2Frame = getFrameInfoFromRequest(request, still2Output);
		namedFrames.push_back({ Dump::Id::AFBLD_F0_WROTO, still2Frame });
	}

	namedFrames.push_back({ Dump::Id::AFBLD_F1_IMGI_D1, afbldF1_.in.imgi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F1_VIPI, afbldF1_.in.vipi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F1_RECI_D1, afbldF1_.in.rec_dsi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F1_TNRCI, afbldF1_.in.tnrci[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F1_TNRLI, afbldF1_.in.tnrlfdi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F1_TNRMI, afbldF1_.in.tnrmi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F1_TNRSI, afbldF6_.in.tnrsi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F1_TNRVBI, afbldF1_.in.tnrvbi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F1_TNRWI, afbldF1_.in.tnrwi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F1_TUNBUF, afbldF1_.in.tunbufi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F1_TNRMO, afbldF1_.out.tnrmo[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F1_TNRSO, afbldF0_.out.tnrso[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F1_TNRWO, afbldF1_.out.tnrwo[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F1_IMG3O, afbldF1_.out.img3o[i]->get() });

	namedFrames.push_back({ Dump::Id::AFBLD_F2_IMGI_D1, afbldF2_.in.imgi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F2_VIPI, afbldF2_.in.vipi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F2_RECI_D1, afbldF2_.in.rec_dsi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F2_TNRCI, afbldF2_.in.tnrci[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F2_TNRLI, afbldF2_.in.tnrlfdi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F2_TNRMI, afbldF2_.in.tnrmi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F2_TNRSI, afbldF6_.in.tnrsi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F2_TNRVBI, afbldF2_.in.tnrvbi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F2_TNRWI, afbldF2_.in.tnrwi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F2_TUNBUF, afbldF2_.in.tunbufi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F2_TNRMO, afbldF2_.out.tnrmo[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F2_TNRSO, afbldF0_.out.tnrso[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F2_TNRWO, afbldF2_.out.tnrwo[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F2_IMG3O, afbldF2_.out.img3o[i]->get() });

	namedFrames.push_back({ Dump::Id::AFBLD_F3_IMGI_D1, afbldF3_.in.imgi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F3_VIPI, afbldF3_.in.vipi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F3_RECI_D1, afbldF3_.in.rec_dsi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F3_TNRCI, afbldF3_.in.tnrci[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F3_TNRLI, afbldF3_.in.tnrlfdi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F3_TNRMI, afbldF3_.in.tnrmi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F3_TNRSI, afbldF6_.in.tnrsi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F3_TNRVBI, afbldF3_.in.tnrvbi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F3_TNRWI, afbldF3_.in.tnrwi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F3_TUNBUF, afbldF3_.in.tunbufi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F3_TNRMO, afbldF3_.out.tnrmo[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F3_TNRSO, afbldF0_.out.tnrso[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F3_TNRWO, afbldF3_.out.tnrwo[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F3_IMG3O, afbldF3_.out.img3o[i]->get() });

	namedFrames.push_back({ Dump::Id::AFBLD_F4_IMGI_D1, afbldF4_.in.imgi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F4_VIPI, afbldF4_.in.vipi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F4_RECI_D1, afbldF4_.in.rec_dsi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F4_TNRCI, afbldF4_.in.tnrci[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F4_TNRLI, afbldF4_.in.tnrlfdi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F4_TNRMI, afbldF4_.in.tnrmi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F4_TNRSI, afbldF6_.in.tnrsi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F4_TNRVBI, afbldF4_.in.tnrvbi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F4_TNRWI, afbldF4_.in.tnrwi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F4_TUNBUF, afbldF4_.in.tunbufi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F4_TNRMO, afbldF4_.out.tnrmo[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F4_TNRSO, afbldF0_.out.tnrso[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F4_TNRWO, afbldF4_.out.tnrwo[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F4_IMG3O, afbldF4_.out.img3o[i]->get() });

	namedFrames.push_back({ Dump::Id::AFBLD_F5_IMGI_D1, afbldF5_.in.imgi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F5_VIPI, afbldF5_.in.vipi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F5_RECI_D1, afbldF5_.in.rec_dsi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F5_TNRCI, afbldF5_.in.tnrci[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F5_TNRLI, afbldF5_.in.tnrlfdi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F5_TNRSI, afbldF6_.in.tnrsi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F5_TNRVBI, afbldF5_.in.tnrvbi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F5_TNRWI, afbldF5_.in.tnrwi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F5_TUNBUF, afbldF5_.in.tunbufi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F5_TNRMO, afbldF5_.out.tnrmo[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F5_TNRSO, afbldF0_.out.tnrso[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F5_TNRWO, afbldF5_.out.tnrwo[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F5_IMG3O, afbldF5_.out.img3o[i]->get() });

	namedFrames.push_back({ Dump::Id::AFBLD_F6_IMGI_D1, afbldF6_.in.imgi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F6_VIPI, afbldF6_.in.vipi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F6_TNRSI, afbldF6_.in.tnrsi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F6_TUNBUF, afbldF6_.in.tunbufi[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F6_TNRSO, afbldF0_.out.tnrso[i]->get() });
	namedFrames.push_back({ Dump::Id::AFBLD_F6_IMG4O, afbldF6_.out.img4o[i]->get() });

	tune(internalRequestId, internalRequestId + order[2], namedFrames, true);
}

void OnDeviceTuner::writeStillCaptureDebugMetadata(
	ControlList &out, mtk::hal3a::v1_0::mtk_3a_result *result,
	std::map<int, int> mfnrExifData)
{
	if (!enabled_) {
		return;
	}
	const unsigned int mfnrDebug = 5;
	const unsigned int idx3ADebug = 6;
	const unsigned int idxIspDebug = 7;

	bool mfnrNDD = false;
	DEBUG_CAM_INFO_T debug_cam_info;
	DEBUG_MF_INFO_T *debug_mf_info = &debug_cam_info.rDbgMFInfo;
	if (mfnrExifData.size() != 0) {
		mfnrNDD = true;
		uint8_t *app5Dest = reinterpret_cast<uint8_t *>(&debug_cam_info);
		std::memcpy(app5Dest,
			    reinterpret_cast<uint8_t *>(const_cast<void *>(NSCam::Custom::sDbgExifBufInfo_cam.header_context)),
			    sizeof(debug_cam_info.hdr));

		for (const auto &item : mfnrExifData) {
			const int index = item.first;
			const int value = item.second;
			debug_mf_info->Tag[index].u4FieldID = (0x1000000 | index);
			debug_mf_info->Tag[index].u4FieldValue = value;
		}
	}

	std::array<uint16_t, 16> jpegAppSegmentLength{ 0 };
	if (mfnrNDD)
		jpegAppSegmentLength[mfnrDebug] = sizeof(DEBUG_CAM_INFO_T);
	jpegAppSegmentLength[idx3ADebug] = sizeof(AAA_DEBUG_INFO1_T);
	jpegAppSegmentLength[idxIspDebug] = sizeof(AAA_DEBUG_INFO2_T);
	out.set(controls::JpegApplicationSegmentLength, jpegAppSegmentLength);

	size_t totalSize = sizeof(AAA_DEBUG_INFO1_T) +
			   sizeof(AAA_DEBUG_INFO2_T);
	if (mfnrNDD)
		totalSize += sizeof(DEBUG_CAM_INFO_T);

	std::vector<uint8_t> jpegAppSegmentContent(totalSize);
	if (mfnrNDD) {
		uint8_t *app5Src = reinterpret_cast<uint8_t *>(&debug_cam_info);
		std::memcpy(jpegAppSegmentContent.data(), app5Src,
			    jpegAppSegmentLength[mfnrDebug]);
	}

	uint8_t *app6Src = reinterpret_cast<uint8_t *>(
		&result->debug_3a_info);
	uint8_t *app6Dest = jpegAppSegmentContent.data() +
			    jpegAppSegmentLength[mfnrDebug];
	std::memcpy(app6Dest, app6Src,
		    jpegAppSegmentLength[idx3ADebug]);

	uint8_t *app7Src = reinterpret_cast<uint8_t *>(
		&result->debug_isp_info);
	uint8_t *app7Dest = jpegAppSegmentContent.data() +
			    jpegAppSegmentLength[mfnrDebug] +
			    jpegAppSegmentLength[idx3ADebug];
	std::memcpy(app7Dest, app7Src, jpegAppSegmentLength[idxIspDebug]);

	out.set(controls::JpegApplicationSegmentContent, jpegAppSegmentContent);
}

void OnDeviceTuner::writeLogScenarioRecorder(
	uint32_t requestId, uint32_t frameNumber,
	EStage_T stage, std::string logMessage)
{
	if (!enabled_) {
		return;
	}

	if (isIpa_) {
		ImagiqAdapter::configureScenarioRecorder(
			requestId, frameNumber, sessionTimestamp_, false,
			isStillCaptureRequest(requestId), Feature::NUM,
			logMessage, stage, sensorId_);
	} else {
		LOG(MtkISP7, Error) << "writeLogScenarioRecorder should be called in ipa";
	}
}
} // namespace libcamera

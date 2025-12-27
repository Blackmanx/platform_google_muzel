/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * on_device_tuner.h - MtkISP7 On Device Tuner module.
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <libcamera/controls.h>
#include <libcamera/request.h>

#include "libcamera/internal/info_frame.h"
#include "libcamera/internal/mailbox.h"

#include "mtkcam-core/include/mtkcam-core/aaahal/aaa_hal/aaa_hal_def.h"
#include "mtkcam-halif/utils/metadata/1.x/IMetadata.h"
#include "pipeline/mtkisp7/odt/camsys_driver_debug.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/dump.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/imagiq_adapter.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/feature.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/static_strings.h"
#include "pipeline/mtkisp7/odt/imgsys_driver_debug.h"
#include "platform/mtkisp7/halisp/IspControls.h"
#include "platform/mtkisp7/halisp/TuningParam.h"
#include "platform/mtkisp7/single_device_helper.h"
#include "tuning_mapping/cam_idx_struct_ext_pub.h"

namespace libcamera {

struct CaptureFrames;
struct MeFrames;
struct MeBFrames;
struct TrFrames;
struct Dip1Frames;
struct Dip2Frames;
struct XtrFrames;
struct LpnrDipFrames;
struct AaaIspExchange;
struct BfbldFrames;
struct McdsF1Frames;
struct BfmeFrames;
struct SwmeFrames;
struct DsFrames;
struct DsVbiFrames;
struct BssFrames;
struct MsbldFrames;
struct AfbldFrames;

class IPADelegate;

class OnDeviceTuner
{
public:
	void initialize(bool isIpa);
	void configure(const std::string &sensorId, unsigned int camsysIndex,
		       int sessionTimestamp = 0,
		       IPADelegate *ipa = nullptr);

	int getSessionTimestamp() { return sessionTimestamp_; }

	void notifyRequestBegin(int requestNumber);
	void notifyRequestEnd(int requestNumber);

	void notifyExportBegin(
		const uint32_t exportBegin,
		const uint32_t exportEnd);
	void notifyImportBegin(
		const uint32_t importBegin,
		const uint32_t importEnd);

	// Tuning tools need to know if there is a still capture in
	// the request or not.
	void notifyVideoOnly(int requestNumber);
	void notifyStillCapture(int baseRequestNumber, int frameNumber);
	bool isStillCaptureRequest(int requestNumber);

	// P1 Camsys
	void tuneCamsys(uint32_t internalRequestId, CaptureFrames &frames);
	void fillCamsysDebugFrame(uint32_t internalRequestId,
				  SharedMailBox<InfoFrame> debugMailBox);
	bool isCamsysDebugFrameEnabled();

	// HAL ISP
	bool tuneCamsysHalIsp(
		uint32_t internalRequestId,
		mtk::isphal::v1_0::TuningParamP1 &tuningParam,
		mtk::isphal::v1_0::ReturnParamP1 &tuningResult,
		mtk::hal3a::v1_0::mtk_3a_result &mtk3AResult,
		Feature feature, bool highIsoMode);
	void tuneImgsysHalIsp(
		uint32_t internalRequestId,
		uint32_t frameNumber,
		mtk::isphal::v1_0::TuningParamDip &tuningParam,
		mtk::isphal::v1_0::ReturnParamDip &tuningResult,
		mtk::hal3a::v1_0::mtk_3a_result &mtk3AResult,
		EStage_T stage, Feature feature);
	void tuneExif(uint32_t internalRequestId,
		      uint32_t frameNumber,
		      const mtk::isphal::v1_0::ExifInfo3A &exif3a,
		      const mtk::isphal::v1_0::ExifInfoP2 &exifIsp,
		      EStage_T stage, Feature feature);

	// 3A
	void tune3ARequest(
		uint32_t internalRequestId,
		mtk::hal3a::v1_0::mtk_3a_request &r3aRequest,
		Feature feature);
	void tune3AState(uint32_t internalRequestId,
			 FrameBuffer *statistics0,
			 mtk::hal3a::v1_0::mtk_3a_result *mtk3AResult);

	// P2 Imgsys driver
	// Todo: in V4L2 mode, there is only one stage,
	// multiple stages per request is in only in single device mode.
	// Once the single device mode code removed, then this function
	// should only accept one stage.
	void tuneImgsysMetadata(
		uint32_t internalRequestId,
		uint32_t frameNumber,
		int layer,
		const std::vector<PEU_Stage> &stages,
		const InfoFrame &metaFrame,
		int mediaRequestFd);
	void tuneImgsysDriver(int internalRequestId,
			      int mediaRequestFd,
			      std::vector<PEU_Stage> stages);

	// MCNR
	void tuneMeA(uint32_t internalRequestId, MeFrames &frames);
	void tuneMeMM(uint32_t internalRequestId, SharedMailBox<InfoFrame> tuning);
	void tuneMeB(uint32_t internalRequestId, MeBFrames &frames);
	void tuneTr(uint32_t internalRequestId, TrFrames &frames);
	void tuneDip1(uint32_t internalRequestId, Dip1Frames &frames);
	void tuneDip2(
		Request *request, uint32_t internalRequestId, Dip2Frames &frames,
		FrameBuffer *videoOut1, FrameBuffer *videoOut2);

	// LPNR
	void tuneXtr(uint32_t internalRequestId, XtrFrames &frames);
	void tuneLpnrDip(Request *request, uint32_t internalRequestId, LpnrDipFrames &frames,
			 std::vector<SharedMailBox<InfoFrame>> reci,
			 std::vector<SharedMailBox<InfoFrame>> dipImg3o,
			 FrameBuffer *still1Output,
			 FrameBuffer *still2Output);
	bool isLowIsoLpnrEnforced();

	bool isEnabled() { return enabled_; }
	// MFNR
	void tuneBss(uint32_t internalRequestId, BssFrames &frames, int frameCount);
	void tuneBfbld(uint32_t internalRequestId, BfbldFrames &frames, std::vector<int> order);
	void tuneMcdsF1(uint32_t internalRequestId, McdsF1Frames &frames, std::vector<int> order);
	void tuneBfme(uint32_t internalRequestId, BfmeFrames &frames, std::vector<int> order);
	void tuneSwme(uint32_t internalRequestId, SwmeFrames &frames, std::vector<int> order);
	void tuneDs(uint32_t internalRequestId, DsFrames &frames, std::vector<int> order);
	void tuneDsVbi(uint32_t internalRequestId, DsVbiFrames &ds_vbi_v2, DsVbiFrames &ds_vbi_v5, std::vector<int> order);
	void tuneMsbld(
		uint32_t internalRequestId, MsbldFrames msbldF0_, MsbldFrames msbldF1_, MsbldFrames msbldF2_,
		MsbldFrames msbldF3_, MsbldFrames msbldF4_, MsbldFrames msbldF5_, MsbldFrames msbldF6_, std::vector<int> order,
		int msbldIdx);
	void tuneAfbld(
		Request *request, uint32_t internalRequestId,
		AfbldFrames afbldF0_, AfbldFrames afbldF1_, AfbldFrames afbldF2_,
		AfbldFrames afbldF3_, AfbldFrames afbldF4_, AfbldFrames afbldF5_, AfbldFrames afbldF6_,
		std::vector<int> order, FrameBuffer *still1Output, FrameBuffer *still2Output);

	// Still capture only
	void writeStillCaptureDebugMetadata(
		ControlList &out, mtk::hal3a::v1_0::mtk_3a_result *result,
		std::map<int, int> mfnrExifData);

	void writeLogScenarioRecorder(
		uint32_t requestId, uint32_t frameNumber,
		EStage_T stage, std::string logMessage);

private:
	static bool isStillCaptureFeature(Feature feature);

	struct NamedFrame {
		Dump::Id id;
		const InfoFrame &frame;
		Stage stage = Stage::Default;
	};
	struct NamedPointer {
		Dump::Id id;
		uint8_t *ptr;
		size_t size;
	};

	std::vector<ImagiqAdapter::ExportResult> batchExport(
		const std::vector<Dump> &dumps);
	void batchImport(const std::vector<Dump> &dumps);
	void batchPrepareReimport(
		const std::vector<ImagiqAdapter::ExportResult> &exportResults);
	InfoFrame getFrameInfoFromRequest(
		Request *request, FrameBuffer *buffer);
	NSCam::IMetadata *getMtkMetadata(int requestNumber);
	bool isImgsysCaptureStage(PEU_Stage stage);
	void loadTuneRequest(int requestNumber);
	bool parseHalIspNdd(
		uint32_t internalRequestId,
		uint32_t frameNumber,
		mtk::isphal::v1_0::NddInfo &ndd,
		Feature feature);
	int prepareNewExportDirectory();
	bool shouldExportDumpNow(uint32_t requestNumber);
	bool shouldImportDumpNow(uint32_t requestNumber);
	void tune(uint32_t requestNumber,
		  std::vector<NamedFrame> namedFrames,
		  bool forceDump = false);
	void tune(uint32_t requestNumber,
		  uint32_t frameNumber,
		  std::vector<NamedFrame> namedFrames,
		  bool forceDump = false);
	void tune(uint32_t requestNumber,
		  std::vector<NamedPointer> namedPointers,
		  bool forceDump = false);

	bool isIpa_;
	IPADelegate *ipa_;

	bool enabled_;
	bool enforceLowIsoLpnr_;
	bool enableCamsysDebugFrame_;
	int sessionTimestamp_;
	std::string sensorId_;

	int prevStartedRequestNum_;
	int prevEndedRequestNum_;
	uint32_t exportBegin_;
	uint32_t exportEnd_;
	uint32_t importBegin_;
	uint32_t importEnd_;
	std::filesystem::path currentExportPath_;

	std::map<Dump::Id, Dump::Config> dumpConfig_;

	// These metadata must live through the request.
	std::map<int, std::unique_ptr<NSCam::IMetadata>>
		mtkMetadata_;

	std::unique_ptr<CamsysDebug> camsysDebug_;
	ImgsysDebug imgsysDebug_;

	std::unordered_map<uint32_t, uint32_t> stillCaptureFrames_;
};

} // namespace libcamera

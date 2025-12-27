/*
 * Copyright (C) 2024, Google Inc.
 *
 * bss.h - MtkISP7 ImgSys Device BSS
 */
#pragma once

#include <memory>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "libcamera/internal/mapped_framebuffer.h"

#include "libcamera/geometry.h"
#include "mtkcam-interfaces/utils/odt/IOnDeviceTuning.h"
#include "pipeline/mtkisp7/imgsys/const.h"
#include "platform/mtkisp7/halisp/ITuningDataProvider.h"
#include "platform/mtkisp7/mtkcam-core/libcamera_ext/lib/libBssWrapper/MTKBssHeader/IMTKBss.h"

namespace libcamera {

struct BssFramesBuffers {
	struct {
		MappedFrameBuffer *bssParamInfo;
		MappedFrameBuffer *bssDataGInfo;
		MappedFrameBuffer *bssVerInfo;
		std::shared_ptr<mtk::isphal::v1::isp_bss_Param> db_param;
		MappedFrameBuffer *bssTuningInfo;

		std::vector<MappedFrameBuffer *> bssFdMainInfo;
		std::vector<MappedFrameBuffer *> imgi;
		std::vector<FrameBuffer *> imgiBuffers;
		std::vector<MappedFrameBuffer *> bssFdInfo;
		std::vector<MappedFrameBuffer *> bssFaceInfo;
		std::vector<MappedFrameBuffer *> bssPosInfo;
	} in;

	struct {
		MappedFrameBuffer *bssOutDataInfo;
	} out;
};

class BssWrapper
{
public:
	BssWrapper(int sensorIndex);
	~BssWrapper();
	MRESULT bssInit(Size camsysYuvSize);
	std::vector<int> doBss(int frameNum,
			       BssFramesBuffers &bssFramesBuffers);
	std::map<MINT32, MINT32> getExifData() { return mExifData; }

private:
	MRESULT bssMain(IBSS_PROC_ENUM ProcId, void *pParaIn, void *pParaOut);
	MRESULT bssFeatureCtrl(IBSS_FTCTRL_ENUM FcId, void *pParaIn, void *pParaOut);
	void *Parser_MfbllIn(void *pParaIn);
	void *Parser_MfbllOut(void *pParaOut);
	// void Parser_MfbllIn_Done(void *pParaIn, void *pParaParseIn);
	void Parser_MfbllOut_Done(void *pParaOut, void *pParaParseOut);
	void *Parser_ParaIn(IBSS_FTCTRL_ENUM FcId, void *pParaIn);
	void *Parser_ParaOut(IBSS_FTCTRL_ENUM FcId, void *pParaOut);
	void Parser_ParaIn_Done(IBSS_FTCTRL_ENUM FcId,
				void *pParaIn,
				void *pParaParseIn);
	void Parser_ParaOut_Done(IBSS_FTCTRL_ENUM FcId,
				 void *pParaOut,
				 void *pParaParseOut);
	void *Parser_BSSIn(void *pParaIn);
	void *Parser_BSSOut(void *pParaOut);
	void Parser_BSSIn_Done(void *pParaIn, void *pParaParseIn);
	void Parser_BSSOut_Done(void *pParaOut, void *pParaParseOut);

	MBOOL getForceBss(void *param_addr, size_t param_size);
	void updateBssProcInfo(IBSS_PARAM_STRUCT *bss_param,
			       MINT32 frameNum,
			       Size srcSize,
			       std::shared_ptr<mtk::isphal::v1::isp_bss_Param> dbParam);
	MBOOL appendBSSInput(std::vector<MappedFrameBuffer *> &p1YuvMappedFrameBuffer,

			     IBSS_INPUT_DATA_G_IPC &bss_input);
	MVOID updateBssIOInfo(IBSS_INPUT_DATA_G_IPC &bss_input);

	MVOID collectPreBSSExifData(IBSS_PARAM_STRUCT *param);
	MVOID collectPostBSSExifData(std::vector<MINT32> &vNewIndex,
				     IBSS_OUTPUT_DATA &bss_output);

	void *m_pBssDrv;
	std::shared_ptr<NSCam::TuningUtils::IOdtUtils> mOdtUtils;
	std::shared_ptr<mtk::isphal::v1::isp_bss_Param> mDbParam;

	ZipOutData mZipData;
	uint32_t sensorIndex_;
	Size camsysYuvSize_;

	bool mEnableBSSOrdering = true;

	IPass_BSS_WB_STRUCT m_BSSWBParse;
	IPass_BSS_PARAM_STRUCT m_BSSParaParse;
	IPass_BSS_CONFIG_ZIP_IN_STRUCT m_BSSZipInParse;
	IPass_BSS_PROC_ZIP_STRUCT m_BSSZipProcParse;
	IPass_BSS_CONVERT_STRUCT m_BSSCovertParaParse;
	IPass_BSS_CONFIG_ZIP_OUT_STRUCT m_BSSZipOutParse;
	IPass_BSS_INPUT_DATA_G m_BSSInputParse;
	IPass_BSS_OUTPUT_DATA m_BSSOutParse;

	std::map<MINT32, MINT32> mExifData;
};

} // namespace libcamera

/*
 * Copyright (C) 2024, Google Inc.
 *
 * swme.h - MtkISP7 ImgSys Device Softeware Motion Estimate
 */

#pragma once
#include "libcamera/internal/info_frame.h"
#include "libcamera/internal/mapped_framebuffer.h"

#include "pipeline/mtkisp7/imgsys/const.h"
#include "platform/mtkisp7/halisp/ITuningDataProvider.h"
#include "platform/mtkisp7/mtkcam-core/libcamera/mt8188/include/libmfnr/MTKMfbll.h"
#include "platform/mtkisp7/mtkcam-core/libcamera_ext/lib/libMfbllWrapper/MTKMfbllHeader/IMTKMfbll.h"
#include "platform/mtkisp7/mtkcam-core/libcamera_ext/lib/libMfbllWrapper/MTKMfbllHeader/include/EMfbll.h"

namespace libcamera {

struct SwmeFramesBuffers {
	struct {
		std::vector<uint8_t> *workbuf;

		MappedFrameBuffer *base_buf;
		MappedFrameBuffer *ref_buf;
		MappedFrameBuffer *bss_buf;
		MappedFrameBuffer *tuningInfo;

		std::shared_ptr<mtk::isphal::v1::isp_swme_Param> db_param;
	} in;
	struct {
		MappedFrameBuffer *conf_map;
		FrameBuffer *conf_map_buffer;
		MappedFrameBuffer *warpping_map;
		FrameBuffer *warpping_map_buffer;
		MappedFrameBuffer *mcmv;
		FrameBuffer *mcmv_buffer;
		MappedFrameBuffer *paramOutInfo;
	} out;
};

class SwmeWrapper
{
public:
	SwmeWrapper();
	~SwmeWrapper();
	MRESULT init();
	void reset();
	static void prepareParam(
		IMFBLL_SET_PROC_INFO_STRUCT_IPC &param,
		std::optional<MtkCameraFaceMetadata> &faceMetadata,
		SwmeFramesBuffers swmeFramesBuffers,
		Size frame_size,
		Size mc_size,
		int index);
	static void prepareOutParam(
		IMFBLL_PROC1_OUT_STRUCT_IPC *paramOut,
		SwmeFramesBuffers swmeFramesBuffers);
	MRESULT swmeMain(IMFBLL_PROC_ENUM ProcId, void *pParaIn, void *pParaOut);
	MRESULT featureCtrl(IMFBLL_FTCTRL_ENUM FcId, void *pParaIn, void *pParaOut);

	void setMotionEstimationResolution(const Size &size)
	{
		m_widthMe = size.width;
		m_heightMe = size.height;
	}

	Size getAlgorithmWorkBufferSize()
	{
		return Size{ m_WorkingBufInfo.Ext_mem_size, 1 };
	}

	Size getWarppingMapSize()
	{
		return Size{ m_WorkingBufInfo.WpeMap_width, m_WorkingBufInfo.WpeMap_height };
	}
	Size getConfMapSize()
	{
		return Size{ m_WorkingBufInfo.CofMap_width, m_WorkingBufInfo.CofMap_height };
	}

private:
	void *Parser_MfbllIn(void *pParaIn);
	void *Parser_MfbllOut(void *pParaOut);
	void Parser_MfbllOut_Done(void *pParaOut, void *pParaParseOut);
	void *Parser_ParaIn(IMFBLL_FTCTRL_ENUM FcId, void *pParaIn);
	void *Parser_ParaOut(IMFBLL_FTCTRL_ENUM FcId, void *pParaOut);
	void Parser_ParaIn_Done(IMFBLL_FTCTRL_ENUM FcId, void *pParaIn, void *pParaParseIn);
	void Parser_ParaOut_Done(IMFBLL_FTCTRL_ENUM FcId, void *pParaOut, void *pParaParseOut);

	MTKMfbll *m_pMfbllDrv;
	int m_widthMe;
	int m_heightMe;
	IPass_MFBLL_GET_PROC_INFO_STRUCT m_MfbllGetProc;
	IPass_MFBLL_SET_PROC_INFO_STRUCT m_MfbllSetProc;
	IPass_MFBLL_PROC1_OUT_STRUCT m_MfbllProcOut;

	IMFBLL_GET_PROC_INFO_STRUCT m_WorkingBufInfo;
};

} /* namespace libcamera */

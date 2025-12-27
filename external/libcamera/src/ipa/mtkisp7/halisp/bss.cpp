/*
 * Copyright (C) 2024, Google Inc.
 *
 * bss.cpp - MtkISP7 ImgSys Device BSS
 */
#include "bss.h"

#include <map>
#include <string>

#include <libcamera/base/log.h>

#include "libcamera/framebuffer.h"
#include "mtkcam-chrom/custom/common/hal/inc/custom/debug_exif/cam/dbg_cam_mf_param18.h"
#include "mtkcam-halif/def/UITypes.h"
#include "mtkcam-interfaces/def/ImageFormat.h"
#include "pipeline/mtkisp7/imgsys/const.h"
#include "pipeline/mtkisp7/imgsys/mfnr.h"
#include "platform/mtkisp7/mtkcam-chrom/custom/mt8188/hal/inc/debug_exif/cam/dbg_cam_param.h"
#include "platform/mtkisp7/mtkcam-core/libcamera/mt8188/include/libmfnr/MTKBss.h"
#include "platform/mtkisp7/mtkcam-core/libcamera_ext/lib/libBssWrapper/MTKBssHeader/EMTKBss.h"
#include "sensor/sensor_info.h"

#define MFLL_MF_TAG_VERSION 18
#define MFLLBSS_FILE_PATH_LEN_MAX 512
#define MFLLBSS_DUMP_PATH "/data/vendor/camera_dump/"
#define MFLLBSS_DUMP_FD_FILENAME "fd-data.log"
#define MFLLBSS_DUMP_BSS_PARAM_FILENAME "bss-param.bin"
#define MFLLBSS_DUMP_BSS_IN_FILENAME "bss-in.bin"
#define MFLLBSS_DUMP_BSS_OUT_FILENAME "bss-out.bin"
namespace libcamera {
LOG_DECLARE_CATEGORY(MtkISP7)

static const int MF_BSS_ROI_PERCENTAGE = 95;
static const int MF_BSS_ON = 1;

BssWrapper::BssWrapper(int sensorIndex)
	: sensorIndex_(sensorIndex)
{
	DRVBssObject_s Pass_DRVBssObject_s = DRV_BSS_OBJ_SW;
	m_pBssDrv =
		(void *)MTKBss::createInstance((DrvBssObject_e)Pass_DRVBssObject_s);
	mOdtUtils = NSCam::TuningUtils::IOdtUtils::getInstance(sensorIndex);
}
BssWrapper::~BssWrapper()
{
	if (m_pBssDrv) {
		MTKBss *pMTKBSS = (MTKBss *)m_pBssDrv;
		pMTKBSS->destroyInstance();
		pMTKBSS = nullptr;
	}
}
MRESULT BssWrapper::bssInit(Size camsysYuvSize)
{
	camsysYuvSize_ = camsysYuvSize;

	MRESULT ErrCode = S_BSS_OK;
	//LOG(MtkISP7, Info) << "bssInit";
	MTKBss *pMTKBSS = (MTKBss *)m_pBssDrv;
	ErrCode = pMTKBSS->BssInit(NULL, NULL);
	return ErrCode;
}

MRESULT BssWrapper::bssFeatureCtrl(IBSS_FTCTRL_ENUM FcId,
				   void *pParaIn,
				   void *pParaOut)
{
	//LOG(MtkISP7, Info) << "bssFeatureCtrl FcId = " << FcId;
	MRESULT ErrCode = S_BSS_OK;
	BSS_FTCTRL_ENUM Pass_BSS_FTCTRL_ENUM;
	switch (FcId) {
	case IBSS_FTCTRL_GET_WB_SIZE: {
		Pass_BSS_FTCTRL_ENUM = BSS_FTCTRL_GET_WB_SIZE;
	} break;
	case IBSS_FTCTRL_SET_WB_SIZE: {
		Pass_BSS_FTCTRL_ENUM = BSS_FTCTRL_SET_WB_SIZE;
	} break;
	case IBSS_FTCTRL_SET_PROC_INFO: {
		Pass_BSS_FTCTRL_ENUM = BSS_FTCTRL_SET_PROC_INFO;
	} break;
	case IBSS_FTCTRL_CONFIG_ZIP: {
		Pass_BSS_FTCTRL_ENUM = BSS_FTCTRL_CONFIG_ZIP;
	} break;
	case IBSS_FTCTRL_PROC_ZIP: {
		Pass_BSS_FTCTRL_ENUM = BSS_FTCTRL_PROC_ZIP;
	} break;
	case IBSS_FTCTRL_CONVERT_I422_YUY2: {
		Pass_BSS_FTCTRL_ENUM = BSS_FTCTRL_CONVERT_I422_YUY2;
	} break;
	case IBSS_FTCTRL_GET_VERSION: {
		Pass_BSS_FTCTRL_ENUM = BSS_FTCTRL_GET_VERSION;
	} break;
	case IBSS_FTCTRL_MAX: {
		Pass_BSS_FTCTRL_ENUM = BSS_FTCTRL_MAX;
	} break;
	}
	void *pIn = Parser_ParaIn(FcId, pParaIn);
	void *pOut = Parser_ParaOut(FcId, pParaOut);
	MTKBss *pMTKBSS = (MTKBss *)m_pBssDrv;
	ErrCode = pMTKBSS->BssFeatureCtrl(Pass_BSS_FTCTRL_ENUM, pIn, pOut);
	Parser_ParaIn_Done(FcId, pIn, pParaIn);
	Parser_ParaOut_Done(FcId, pOut, pParaOut);
	return ErrCode;
}

MRESULT BssWrapper::bssMain(IBSS_PROC_ENUM ProcId,
			    void *pParaIn,
			    void *pParaOut)
{
	MRESULT ErrCode = S_BSS_OK;
	BSS_PROC_ENUM Pass_BSS_PROC_ENUM;
	switch (ProcId) {
	case IBSS_PROC1: {
		Pass_BSS_PROC_ENUM = BSS_PROC1;
	} break;
	case IBSS_PROC2: {
		Pass_BSS_PROC_ENUM = BSS_PROC2;
	} break;
	case IBSS_PROC3: {
		Pass_BSS_PROC_ENUM = BSS_PROC3;
	} break;
	case IBSS_UNKNOWN_PROC: {
		Pass_BSS_PROC_ENUM = BSS_UNKNOWN_PROC;
	} break;
	}
	void *pBSSInParse = Parser_BSSIn(pParaIn);
	void *pBSSOutParse = Parser_BSSOut(pParaOut);

	MTKBss *pMTKBSS = (MTKBss *)m_pBssDrv;
	ErrCode = pMTKBSS->BssMain(Pass_BSS_PROC_ENUM, pBSSInParse, pBSSOutParse);

	Parser_BSSIn_Done(pBSSInParse, pParaIn);
	Parser_BSSOut_Done(pBSSOutParse, pParaOut);
	return ErrCode;
}

void *BssWrapper::Parser_ParaIn(IBSS_FTCTRL_ENUM FcId, void *pParaIn)
{
	//LOG(MtkISP7, Info) << "Parser_ParaIn FcId = " << FcId;
	switch (FcId) {
	case IBSS_FTCTRL_GET_WB_SIZE:
	case IBSS_FTCTRL_SET_WB_SIZE: {
		IBSS_WB_STRUCT *pBSSWB = (IBSS_WB_STRUCT *)pParaIn;
		IPASS_BSS_PROC_ENUM Pass_BSS_PROC_ENUM;
		switch (pBSSWB->rProcId) {
		case IBSS_PROC1: {
			Pass_BSS_PROC_ENUM = IPASS_BSS_PROC1;
		} break;
		case IBSS_PROC2: {
			Pass_BSS_PROC_ENUM = IPASS_BSS_PROC2;
		} break;
		case IBSS_PROC3: {
			Pass_BSS_PROC_ENUM = IPASS_BSS_PROC3;
		} break;
		case IBSS_UNKNOWN_PROC: {
			Pass_BSS_PROC_ENUM = IPASS_BSS_UNKNOWN_PROC;
		} break;
		}
		m_BSSWBParse.rProcId = Pass_BSS_PROC_ENUM;
		m_BSSWBParse.u4Width = pBSSWB->u4Width;
		m_BSSWBParse.u4Height = pBSSWB->u4Height;

		m_BSSWBParse.u4FrameNum = pBSSWB->u4FrameNum;
		m_BSSWBParse.u4WKSize = pBSSWB->u4WKSize;
		m_BSSWBParse.pu1BW = pBSSWB->pu1BW;
		return &m_BSSWBParse;
	} break;
	case IBSS_FTCTRL_SET_PROC_INFO: {
		IBSS_PARAM_STRUCT *pBSSPara = (IBSS_PARAM_STRUCT *)pParaIn;
		m_BSSParaParse.BSS_ON = pBSSPara->BSS_ON;
		m_BSSParaParse.BSS_ROI_WIDTH = pBSSPara->BSS_ROI_WIDTH;
		m_BSSParaParse.BSS_ROI_HEIGHT = pBSSPara->BSS_ROI_HEIGHT;
		m_BSSParaParse.BSS_ROI_X0 = pBSSPara->BSS_ROI_X0;
		m_BSSParaParse.BSS_ROI_Y0 = pBSSPara->BSS_ROI_Y0;
		m_BSSParaParse.pBSSNvram = pBSSPara->pBSSNvram;
		m_BSSParaParse.BSS_FRAME_NUM = pBSSPara->BSS_FRAME_NUM;
		m_BSSParaParse.BSS_GAIN_TH0 = pBSSPara->BSS_GAIN_TH0;
		m_BSSParaParse.BSS_GAIN_TH1 = pBSSPara->BSS_GAIN_TH1;
		m_BSSParaParse.BSS_MIN_ISP_GAIN = pBSSPara->BSS_MIN_ISP_GAIN;
		m_BSSParaParse.BSS_LCSO_SIZE = pBSSPara->BSS_LCSO_SIZE;
		m_BSSParaParse.BSS_FD_TH0 = pBSSPara->BSS_FD_TH0;
		m_BSSParaParse.BSS_FD_TH1 = pBSSPara->BSS_FD_TH1;
		m_BSSParaParse.BSS_AEVC_EN = pBSSPara->BSS_AEVC_EN;
		m_BSSParaParse.BSS_AEVC_DCNT = pBSSPara->BSS_AEVC_DCNT;
		return &m_BSSParaParse;
	} break;
	case IBSS_FTCTRL_CONFIG_ZIP: {
		IBSS_CONFIG_ZIP_IN_STRUCT *pBSSZipIn =
			(IBSS_CONFIG_ZIP_IN_STRUCT *)pParaIn;

		m_BSSZipInParse.u4Width = pBSSZipIn->u4Width;
		m_BSSZipInParse.u4Height = pBSSZipIn->u4Height;

		IPASS_BSS_PROC_TYPE Pass_BSS_PROC_TYPE;
		switch (pBSSZipIn->eType) {
		case IBSS_TYPE_PACK_RAW10: {
			Pass_BSS_PROC_TYPE = IPASS_BSS_TYPE_PACK_RAW10;
		} break;
		case IBSS_TYPE_PACK_Y10: {
			Pass_BSS_PROC_TYPE = IPASS_BSS_TYPE_PACK_Y10;
		} break;
		case IBSS_TYPE_UNPACK_NV21: {
			Pass_BSS_PROC_TYPE = IPASS_BSS_TYPE_UNPACK_NV21;
		} break;
		case IBSS_UNKNOWN_TYPE: {
			Pass_BSS_PROC_TYPE = IPASS_BSS_UNKNOWN_TYPE;
		} break;
		}
		m_BSSZipInParse.eType = Pass_BSS_PROC_TYPE;
		m_BSSZipInParse.u4ROI_X0 = pBSSZipIn->u4ROI_X0;
		m_BSSZipInParse.u4ROI_Y0 = pBSSZipIn->u4ROI_Y0;
		m_BSSZipInParse.u4ROI_WIDTH = pBSSZipIn->u4ROI_WIDTH;
		m_BSSZipInParse.u4ROI_HEIGHT = pBSSZipIn->u4ROI_HEIGHT;
		m_BSSZipInParse.pBSSNvram = pBSSZipIn->pBSSNvram;
		return &m_BSSZipInParse;
	} break;
	case IBSS_FTCTRL_PROC_ZIP: {
		IBSS_PROC_ZIP_STRUCT *pBSSZipProc = (IBSS_PROC_ZIP_STRUCT *)pParaIn;
		m_BSSZipProcParse.pu8InImg = pBSSZipProc->pu8InImg;
		m_BSSZipProcParse.u4ImgInStride = pBSSZipProc->u4ImgInStride;
		m_BSSZipProcParse.u4ImgInSize = pBSSZipProc->u4ImgInSize;
		m_BSSZipProcParse.u4ImgInBayerOrder = pBSSZipProc->u4ImgInBayerOrder;
		m_BSSZipProcParse.pu8OutImg = pBSSZipProc->pu8OutImg;
		m_BSSZipProcParse.u4ImgOutSize = pBSSZipProc->u4ImgOutSize;
		m_BSSZipProcParse.i4GmvX = pBSSZipProc->i4GmvX;
		m_BSSZipProcParse.i4GmvY = pBSSZipProc->i4GmvY;
		return &m_BSSZipProcParse;
	} break;
	case IBSS_FTCTRL_CONVERT_I422_YUY2: {
		IBSS_CONVERT_STRUCT *pBSSCovertPara = (IBSS_CONVERT_STRUCT *)pParaIn;
		m_BSSCovertParaParse.pu1Y = pBSSCovertPara->pu1Y;
		m_BSSCovertParaParse.pu1U = pBSSCovertPara->pu1U;
		m_BSSCovertParaParse.pu1V = pBSSCovertPara->pu1V;
		m_BSSCovertParaParse.pu1Out = pBSSCovertPara->pu1Out;
		m_BSSCovertParaParse.u4Width = pBSSCovertPara->u4Width;
		m_BSSCovertParaParse.u4Height = pBSSCovertPara->u4Height;
		m_BSSCovertParaParse.u4Core = pBSSCovertPara->u4Core;
		return &m_BSSCovertParaParse;
	} break;
	case IBSS_FTCTRL_GET_VERSION: {
		return pParaIn;
	} break;
	default:
		LOG(MtkISP7, Error) << "[Parser_ParaIn] Feature Ctrl error: " << FcId;
		break;
	}
	return pParaIn;
}

void *BssWrapper::Parser_ParaOut(IBSS_FTCTRL_ENUM FcId, void *pParaOut)
{
	//LOG(MtkISP7, Info) << "Parser_ParaOut FcId = " << FcId;
	switch (FcId) {
	case IBSS_FTCTRL_CONFIG_ZIP: {
		IBSS_CONFIG_ZIP_OUT_STRUCT *pBSSZipOut =
			(IBSS_CONFIG_ZIP_OUT_STRUCT *)pParaOut;
		m_BSSZipOutParse.u4ZipBufferSize = pBSSZipOut->u4ZipBufferSize;
		return &m_BSSZipOutParse;
	}
	case IBSS_FTCTRL_GET_VERSION: {
		return pParaOut;
	} break;
	default:
		LOG(MtkISP7, Error) << "[Parser_ParaOut] Feature Ctrl error: " << FcId;
		break;
	}
	return pParaOut;
}

void BssWrapper::Parser_ParaIn_Done(IBSS_FTCTRL_ENUM FcId,
				    void *pParaIn,
				    void *pParaParseIn)
{
	switch (FcId) {
	case IBSS_FTCTRL_GET_WB_SIZE:
	case IBSS_FTCTRL_SET_WB_SIZE: {
		IPass_BSS_WB_STRUCT *pBSSWB = (IPass_BSS_WB_STRUCT *)pParaIn;
		IBSS_WB_STRUCT *pBSSWBParse = (IBSS_WB_STRUCT *)pParaParseIn;
		pBSSWBParse->u4Width = pBSSWB->u4Width;
		pBSSWBParse->u4Height = pBSSWB->u4Height;
		pBSSWBParse->u4FrameNum = pBSSWB->u4FrameNum;
		pBSSWBParse->u4WKSize = pBSSWB->u4WKSize;
		pBSSWBParse->pu1BW = pBSSWB->pu1BW;
	} break;
	case IBSS_FTCTRL_SET_PROC_INFO: {
		IPass_BSS_PARAM_STRUCT *pBSSPara = (IPass_BSS_PARAM_STRUCT *)pParaIn;
		IBSS_PARAM_STRUCT *pBSSParaParse = (IBSS_PARAM_STRUCT *)pParaParseIn;
		pBSSParaParse->BSS_ON = pBSSPara->BSS_ON;
		pBSSParaParse->BSS_ROI_WIDTH = pBSSPara->BSS_ROI_WIDTH;
		pBSSParaParse->BSS_ROI_HEIGHT = pBSSPara->BSS_ROI_HEIGHT;
		pBSSParaParse->BSS_ROI_X0 = pBSSPara->BSS_ROI_X0;
		pBSSParaParse->BSS_ROI_Y0 = pBSSPara->BSS_ROI_Y0;
		pBSSParaParse->pBSSNvram = pBSSPara->pBSSNvram;
		pBSSParaParse->BSS_FRAME_NUM = pBSSPara->BSS_FRAME_NUM;
		pBSSParaParse->BSS_GAIN_TH0 = pBSSPara->BSS_GAIN_TH0;
		pBSSParaParse->BSS_GAIN_TH1 = pBSSPara->BSS_GAIN_TH1;
		pBSSParaParse->BSS_MIN_ISP_GAIN = pBSSPara->BSS_MIN_ISP_GAIN;
		pBSSParaParse->BSS_LCSO_SIZE = pBSSPara->BSS_LCSO_SIZE;
		pBSSParaParse->BSS_FD_TH0 = pBSSPara->BSS_FD_TH0;
		pBSSParaParse->BSS_FD_TH1 = pBSSPara->BSS_FD_TH1;
		pBSSParaParse->BSS_AEVC_EN = pBSSPara->BSS_AEVC_EN;
		pBSSParaParse->BSS_AEVC_DCNT = pBSSPara->BSS_AEVC_DCNT;
	} break;
	case IBSS_FTCTRL_CONFIG_ZIP: {
		IPass_BSS_CONFIG_ZIP_IN_STRUCT *pBSSZipIn =
			(IPass_BSS_CONFIG_ZIP_IN_STRUCT *)pParaIn;
		IBSS_CONFIG_ZIP_IN_STRUCT *pBSSZipInParse =
			(IBSS_CONFIG_ZIP_IN_STRUCT *)pParaParseIn;
		pBSSZipInParse->u4Width = pBSSZipIn->u4Width;
		pBSSZipInParse->u4Height = pBSSZipIn->u4Height;
		pBSSZipInParse->u4ROI_X0 = pBSSZipIn->u4ROI_X0;
		pBSSZipInParse->u4ROI_Y0 = pBSSZipIn->u4ROI_Y0;
		pBSSZipInParse->u4ROI_WIDTH = pBSSZipIn->u4ROI_WIDTH;
		pBSSZipInParse->pBSSNvram = pBSSZipIn->pBSSNvram;
	} break;
	case IBSS_FTCTRL_PROC_ZIP: {
		IPass_BSS_PROC_ZIP_STRUCT *pBSSZipProc =
			(IPass_BSS_PROC_ZIP_STRUCT *)pParaIn;
		IBSS_PROC_ZIP_STRUCT *pBSSZipProcParse =
			(IBSS_PROC_ZIP_STRUCT *)pParaParseIn;

		pBSSZipProcParse->pu8InImg = pBSSZipProc->pu8InImg;
		pBSSZipProcParse->u4ImgInStride = pBSSZipProc->u4ImgInStride;
		pBSSZipProcParse->u4ImgInSize = pBSSZipProc->u4ImgInSize;
		pBSSZipProcParse->u4ImgInBayerOrder = pBSSZipProc->u4ImgInBayerOrder;
		pBSSZipProcParse->pu8OutImg = pBSSZipProc->pu8OutImg;
		pBSSZipProcParse->u4ImgOutSize = pBSSZipProc->u4ImgOutSize;
		pBSSZipProcParse->i4GmvX = pBSSZipProc->i4GmvX;
		pBSSZipProcParse->i4GmvY = pBSSZipProc->i4GmvY;
	} break;
	case IBSS_FTCTRL_CONVERT_I422_YUY2: {
		IPass_BSS_CONVERT_STRUCT *pBSSCovertPara =
			(IPass_BSS_CONVERT_STRUCT *)pParaIn;
		IBSS_CONVERT_STRUCT *pBSSCovertParaParse =
			(IBSS_CONVERT_STRUCT *)pParaParseIn;

		pBSSCovertParaParse->pu1Y = pBSSCovertPara->pu1Y;
		pBSSCovertParaParse->pu1U = pBSSCovertPara->pu1U;
		pBSSCovertParaParse->pu1V = pBSSCovertPara->pu1V;
		pBSSCovertParaParse->pu1Out = pBSSCovertPara->pu1Out;
		pBSSCovertParaParse->u4Width = pBSSCovertPara->u4Width;
		pBSSCovertParaParse->u4Height = pBSSCovertPara->u4Height;
		pBSSCovertParaParse->u4Core = pBSSCovertPara->u4Core;
	} break;
	case IBSS_FTCTRL_GET_VERSION: {
	} break;
	default:
		LOG(MtkISP7, Error) << "[Parser_ParaIn_Done] Feature Ctrl error:" << FcId;
		break;
	}
	return;
}

void BssWrapper::Parser_ParaOut_Done(IBSS_FTCTRL_ENUM FcId,
				     void *pParaOut,
				     void *pParaParseOut)
{
	switch (FcId) {
	case IBSS_FTCTRL_CONFIG_ZIP: {
		IPass_BSS_CONFIG_ZIP_OUT_STRUCT *pBSSZipOut =
			(IPass_BSS_CONFIG_ZIP_OUT_STRUCT *)pParaOut;
		IBSS_CONFIG_ZIP_OUT_STRUCT *pBSSZipOutParse =
			(IBSS_CONFIG_ZIP_OUT_STRUCT *)pParaParseOut;
		pBSSZipOutParse->u4ZipBufferSize = pBSSZipOut->u4ZipBufferSize;
	} break;
	case IBSS_FTCTRL_GET_VERSION: {
	} break;
	default:
		LOG(MtkISP7, Error) << "[Parser_ParaOut_Done] Feature Ctrl error: "
				    << FcId;
		break;
	}
	return;
}

void *BssWrapper::Parser_BSSIn(void *pParaIn)
{
	IBSS_INPUT_DATA_G *pBSSInput = (IBSS_INPUT_DATA_G *)pParaIn;
	m_BSSInputParse.Bitnum = pBSSInput->Bitnum;
	m_BSSInputParse.BayerOrder = pBSSInput->BayerOrder;
	m_BSSInputParse.Stride = pBSSInput->Stride;
	m_BSSInputParse.inWidth = pBSSInput->inWidth;
	m_BSSInputParse.inHeight = pBSSInput->inHeight;
	m_BSSInputParse.fdWidth = pBSSInput->fdWidth;
	m_BSSInputParse.fdHeight = pBSSInput->fdHeight;
	IPASS_BSS_PROC_TYPE Pass_BSS_PROC_TYPE;
	switch (pBSSInput->eType) {
	case IBSS_TYPE_PACK_RAW10: {
		Pass_BSS_PROC_TYPE = IPASS_BSS_TYPE_PACK_RAW10;
	} break;
	case IBSS_TYPE_PACK_Y10: {
		Pass_BSS_PROC_TYPE = IPASS_BSS_TYPE_PACK_Y10;
	} break;
	case IBSS_TYPE_UNPACK_NV21: {
		Pass_BSS_PROC_TYPE = IPASS_BSS_TYPE_UNPACK_NV21;
	} break;
	case IBSS_UNKNOWN_TYPE: {
		Pass_BSS_PROC_TYPE = IPASS_BSS_UNKNOWN_TYPE;
	} break;
	}
	m_BSSInputParse.eType = Pass_BSS_PROC_TYPE;
	m_BSSInputParse.InputZip = pBSSInput->InputZip;

	for (int i = 0; i < MAX_FRAME_NUM; i++) {
		m_BSSInputParse.apbyBssInImg[i] = pBSSInput->apbyBssInImg[i];
		m_BSSInputParse.gmv[i].x = pBSSInput->gmv[i].x;
		m_BSSInputParse.gmv[i].y = pBSSInput->gmv[i].y;

		m_BSSInputParse.Face[i] = (IBssFaceMetadata *)pBSSInput->Face[i];
		m_BSSInputParse.u4AGain[i] = pBSSInput->u4AGain[i];
		m_BSSInputParse.u4DGain[i] = pBSSInput->u4DGain[i];
		m_BSSInputParse.u4ExpT[i] = pBSSInput->u4ExpT[i];
		m_BSSInputParse.u2Lcso[i] = pBSSInput->u2Lcso[i];
		m_BSSInputParse.act_hist_buf[i] = pBSSInput->act_hist_buf[i];
	}
	m_BSSInputParse.prGyroInfo = (IBSS_GYRO_INFO *)pBSSInput->prGyroInfo;
	m_BSSInputParse.u4GyroNum = pBSSInput->u4GyroNum;
	m_BSSInputParse.u4GyroIntervalMS = pBSSInput->u4GyroIntervalMS;
	m_BSSInputParse.bss_iso = pBSSInput->bss_iso;
	return &m_BSSInputParse;
}

void *BssWrapper::Parser_BSSOut(void *pParaOut)
{
	IBSS_OUTPUT_DATA *pBSSOut = (IBSS_OUTPUT_DATA *)pParaOut;

	for (int i = 0; i < MAX_FRAME_NUM; i++) {
		m_BSSOutParse.originalOrder[i] = pBSSOut->originalOrder[i];
		m_BSSOutParse.gmv[i].x = pBSSOut->gmv[i].x;
		m_BSSOutParse.gmv[i].y = pBSSOut->gmv[i].y;
		m_BSSOutParse.SharpScore[i] = pBSSOut->SharpScore[i];
		m_BSSOutParse.adj1_score[i] = pBSSOut->adj1_score[i];
		m_BSSOutParse.adj2_score[i] = pBSSOut->adj2_score[i];
		m_BSSOutParse.adj3_score[i] = pBSSOut->adj3_score[i];
		m_BSSOutParse.final_score[i] = pBSSOut->final_score[i];
		m_BSSOutParse.ACTSScore[i] = pBSSOut->ACTSScore[i];
		m_BSSOutParse.BlendingScore[i] = pBSSOut->BlendingScore[i];
		m_BSSOutParse.AvgPxLvl[i] = pBSSOut->AvgPxLvl[i];
		m_BSSOutParse.u4DGain[i] = pBSSOut->u4DGain[i];
		m_BSSOutParse.u2Lcso[i] = pBSSOut->u2Lcso[i];
	}
	m_BSSOutParse.i4SkipFrmCnt = pBSSOut->i4SkipFrmCnt;
	return &m_BSSOutParse;
}

void BssWrapper::Parser_BSSIn_Done(void *pParaIn, void *pParaParseIn)
{
	IPass_BSS_INPUT_DATA_G *pBSSInput = (IPass_BSS_INPUT_DATA_G *)pParaIn;
	IBSS_INPUT_DATA_G *pBSSInputParse = (IBSS_INPUT_DATA_G *)pParaParseIn;
	pBSSInputParse->Bitnum = pBSSInput->Bitnum;
	pBSSInputParse->BayerOrder = pBSSInput->BayerOrder;
	pBSSInputParse->Stride = pBSSInput->Stride;
	pBSSInputParse->inWidth = pBSSInput->inWidth;
	pBSSInputParse->inHeight = pBSSInput->inHeight;
	pBSSInputParse->fdWidth = pBSSInput->fdWidth;
	pBSSInputParse->fdHeight = pBSSInput->fdHeight;
	pBSSInputParse->InputZip = pBSSInput->InputZip;
	for (int i = 0; i < MAX_FRAME_NUM; i++) {
		pBSSInputParse->apbyBssInImg[i] = pBSSInput->apbyBssInImg[i];
		pBSSInputParse->gmv[i].x = pBSSInput->gmv[i].x;
		pBSSInputParse->gmv[i].y = pBSSInput->gmv[i].y;
		pBSSInputParse->Face[i] = (IBssFaceMetadata *)pBSSInput->Face[i];
		pBSSInputParse->u4AGain[i] = pBSSInput->u4AGain[i];
		pBSSInputParse->u4DGain[i] = pBSSInput->u4DGain[i];
		pBSSInputParse->u4ExpT[i] = pBSSInput->u4ExpT[i];
		pBSSInputParse->u2Lcso[i] = pBSSInput->u2Lcso[i];
		pBSSInputParse->act_hist_buf[i] = pBSSInput->act_hist_buf[i];
	}
	pBSSInputParse->prGyroInfo = (IBSS_GYRO_INFO *)pBSSInput->prGyroInfo;
	pBSSInputParse->u4GyroNum = pBSSInput->u4GyroNum;
	pBSSInputParse->u4GyroIntervalMS = pBSSInput->u4GyroIntervalMS;
	pBSSInputParse->bss_iso = pBSSInput->bss_iso;
}

void BssWrapper::Parser_BSSOut_Done(void *pParaOut, void *pParaParseOut)
{
	IPass_BSS_OUTPUT_DATA *pBSSOut = (IPass_BSS_OUTPUT_DATA *)pParaOut;
	IBSS_OUTPUT_DATA *pBSSOutParse = (IBSS_OUTPUT_DATA *)pParaParseOut;
	for (int i = 0; i < MAX_FRAME_NUM; i++) {
		pBSSOutParse->originalOrder[i] = pBSSOut->originalOrder[i];
		pBSSOutParse->gmv[i].x = pBSSOut->gmv[i].x;
		pBSSOutParse->gmv[i].y = pBSSOut->gmv[i].y;
		pBSSOutParse->SharpScore[i] = pBSSOut->SharpScore[i];
		pBSSOutParse->adj1_score[i] = pBSSOut->adj1_score[i];
		pBSSOutParse->adj2_score[i] = pBSSOut->adj2_score[i];
		pBSSOutParse->adj3_score[i] = pBSSOut->adj3_score[i];
		pBSSOutParse->final_score[i] = pBSSOut->final_score[i];
		pBSSOutParse->ACTSScore[i] = pBSSOut->ACTSScore[i];
		pBSSOutParse->BlendingScore[i] = pBSSOut->BlendingScore[i];
		pBSSOutParse->AvgPxLvl[i] = pBSSOut->AvgPxLvl[i];
		pBSSOutParse->u4DGain[i] = pBSSOut->u4DGain[i];
		pBSSOutParse->u2Lcso[i] = pBSSOut->u2Lcso[i];
	}
	pBSSOutParse->i4SkipFrmCnt = pBSSOut->i4SkipFrmCnt;
}

void BssWrapper::updateBssProcInfo(IBSS_PARAM_STRUCT *bss_param,
				   MINT32 frameNum,
				   Size srcSize,
				   std::shared_ptr<mtk::isphal::v1::isp_bss_Param> dbParam)
{
	MINT32 roiPercentage = MF_BSS_ROI_PERCENTAGE;
	MINT32 w = (srcSize.width * roiPercentage + 5) / 100;
	MINT32 h = (srcSize.height * roiPercentage + 5) / 100;
	MINT32 x = (srcSize.width - w) / 2;
	MINT32 y = (srcSize.height - h) / 2;

#define MAKE_TAG(prefix, tag, id) prefix##tag##id
#define MAKE_TUPLE(tag, id) std::make_tuple(#tag, id)
#define DECLARE_BSS_ENUM_MAP() \
	std::map<std::tuple<std::string, int>, MUINT32> enumMap
#define BUILD_BSS_ENUM_MAP(tag)                                                            \
	do {                                                                               \
		if (enumMap[MAKE_TUPLE(tag, -1)] == 1)                                     \
			break;                                                             \
		enumMap[MAKE_TUPLE(tag, -1)] = 1;                                          \
		enumMap[MAKE_TUPLE(tag, 0)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _00); \
		enumMap[MAKE_TUPLE(tag, 1)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _01); \
		enumMap[MAKE_TUPLE(tag, 2)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _02); \
		enumMap[MAKE_TUPLE(tag, 3)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _03); \
		enumMap[MAKE_TUPLE(tag, 4)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _04); \
		enumMap[MAKE_TUPLE(tag, 5)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _05); \
		enumMap[MAKE_TUPLE(tag, 6)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _06); \
		enumMap[MAKE_TUPLE(tag, 7)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _07); \
	} while (0)
#define SET_CUST_MFLL_BSS(tag, idx, value)                      \
	do {                                                    \
		BUILD_BSS_ENUM_MAP(tag);                        \
		enumMap[MAKE_TUPLE(tag, idx)] = (MUINT32)value; \
	} while (0)
#define GET_CUST_MFLL_BSS(tag)                                 \
	[&, this]() {                                          \
		BUILD_BSS_ENUM_MAP(tag);                       \
		return enumMap[MAKE_TUPLE(tag, mSensorIndex)]; \
	}()

	DECLARE_BSS_ENUM_MAP();

	mDbParam = dbParam;

	/* ODT BSS_TUNING */
	if (mOdtUtils && mOdtUtils->is_enable() && mDbParam != nullptr) {
		/*
loadBinaryFromOdt(reinterpret_cast<void*>(mDbParam.get()),
          sizeof(isp_bss_Param), EStage_T::EStage_BSS,
          eModule::kBSS_TUNING, "BSS_TUNING");
*/
	} else {
		LOG(MtkISP7, Info) << "OdtUtils is not enable or mDbParam is nullptr "
				   << (mDbParam == nullptr);
	}
	bss_param->pBSSNvram = mDbParam.get();
	LOG(MtkISP7, Info) << "OdtUtils is not enable or mDbParam is nullptr "
			   << (mDbParam == nullptr);

	//LOG(MtkISP7, Info) << "pBSSNvram:" << reinterpret_cast<void *>(bss_param->pBSSNvram);
	// using BSS_PARAM_STRUCT default value if not set.

	bss_param->BSS_ON = MF_BSS_ON;
	bss_param->BSS_ROI_WIDTH = w;
	bss_param->BSS_ROI_HEIGHT = h;
	bss_param->BSS_ROI_X0 = x;
	bss_param->BSS_ROI_Y0 = y;

	bss_param->BSS_FRAME_NUM = frameNum;

	/*
  if (MY_UNLIKELY(getForceBss(reinterpret_cast<void *>(&p),
                              sizeof(IBSS_PARAM_STRUCT)))) {
          MY_LOGI("@@@BSS %s: force set BSS param as manual setting",
  __FUNCTION__);
  }
  */

	/* ODT IBSS_PARAM_STRUCT */
	/*
  if (mOdtUtils && mOdtUtils->is_enable()) {
          loadBinaryFromOdt(reinterpret_cast<void *>(&p),
  sizeof(IBSS_PARAM_STRUCT), EStage_T::EStage_BSS, eModule::kBSS_PARAM,
  "BSS_PARAM"); bss_param->pBSSNvram = mDbParam.get();
  }
  */

	LOG(MtkISP7, Info) << "======= updateBssProcInfo start ======";

	LOG(MtkISP7, Info) << "ON(" << (MINT32)bss_param->BSS_ON << ") VER("
			   << ") ROI("
			   << bss_param->BSS_ROI_X0 << "," << bss_param->BSS_ROI_Y0 << ", "
			   << bss_param->BSS_ROI_WIDTH << "x" << bss_param->BSS_ROI_HEIGHT << ")";

	LOG(MtkISP7, Info) << "FRAME_NUM(" << bss_param->BSS_FRAME_NUM << ")";
	LOG(MtkISP7, Info) << "GAIN0(" << bss_param->BSS_GAIN_TH0 << ") GAIN1("
			   << bss_param->BSS_GAIN_TH1 << ") MIN_ISP_GAIN("
			   << bss_param->BSS_MIN_ISP_GAIN << ") LCSO_SIZE(" << bss_param->BSS_LCSO_SIZE
			   << ")";

	LOG(MtkISP7, Info) << "AEVC: EN(" << (MINT32)bss_param->BSS_AEVC_EN << ")";

	LOG(MtkISP7, Info) << "======= updateBssProcInfo end ======";
}
MBOOL BssWrapper::appendBSSInput(
	std::vector<MappedFrameBuffer *> &p1YuvMappedFrameBuffer,
	IBSS_INPUT_DATA_G_IPC &bss_input)
{
	for (auto idx = 0; idx < (int)p1YuvMappedFrameBuffer.size(); idx++) {
		bss_input.apbyBssInImg[idx] = reinterpret_cast<MUINT8 *>(
			p1YuvMappedFrameBuffer[idx]->planes()[0].data());
	}
	return true;
}

MVOID BssWrapper::updateBssIOInfo(IBSS_INPUT_DATA_G_IPC &bss_input)
{
	memset(&bss_input, 0, sizeof(bss_input));

	std::shared_ptr<SensorInfo> sensor_info =
		SensorInfo::getInstance(sensorIndex_);

	if (sensor_info == NULL) {
		LOG(MtkISP7, Error) << "get sensor std::vector failed";
		return;
	} else {
		std::array<mtk::hal3a::SensorStaticInfo, kMaxSensorCnt>
			nscam_sensor_static_info_array;
		sensor_info->get_sensor_static_info(&nscam_sensor_static_info_array);

		NSCam::SensorStaticInfo sensorStaticInfo =
			nscam_sensor_static_info_array[sensorIndex_].info;

		bss_input.BayerOrder = sensorStaticInfo.sensorFormatOrder;
		bss_input.Bitnum = [&]() -> MUINT32 {
			switch (sensorStaticInfo.rawSensorBit) {
			case NSCam::RAW_SENSOR_8BIT:
				return 8;
			case NSCam::RAW_SENSOR_10BIT:
				return 10;
			case NSCam::RAW_SENSOR_12BIT:
				return 12;
			case NSCam::RAW_SENSOR_14BIT:
				return 14;
			default:
				LOG(MtkISP7, Error) << "get sensor raw bitnum failed";
				return 0xFF;
			}
		}();
	}

	bss_input.Stride = mZipData.imgStride[0];
	bss_input.inWidth = mZipData.imgWidth;
	bss_input.inHeight = mZipData.imgHeight;

	LOG(MtkISP7, Info) << "BayerOrder:" << bss_input.BayerOrder
			   << ", Bitnum:" << bss_input.Bitnum
			   << ", Stride:" << bss_input.BayerOrder
			   << ", Size:" << bss_input.inWidth << "x" << bss_input.inHeight;
}

MVOID BssWrapper::collectPreBSSExifData(IBSS_PARAM_STRUCT *bss_param)
{
#if (MFLL_MF_TAG_VERSION > 0)
	mtk::isphal::v1::isp_bss_Param *pBssDB = reinterpret_cast<mtk::isphal::v1::isp_bss_Param *>(bss_param->pBSSNvram);
	LOG(MtkISP7, Info) << "pBssDB: " << reinterpret_cast<void *>(pBssDB);
#define SET_EXIF_BSS(tag, value)                                              \
	do {                                                                  \
		mExifData[(MINT32)__namespace_mf(MFLL_MF_TAG_VERSION)::tag] = \
			(MINT32)value;                                        \
	} while (0)

	SET_EXIF_BSS(MF_TAG_BSS_ON, bss_param->BSS_ON);
	SET_EXIF_BSS(MF_TAG_BSS_ROI_WIDTH, bss_param->BSS_ROI_WIDTH);
	SET_EXIF_BSS(MF_TAG_BSS_ROI_HEIGHT, bss_param->BSS_ROI_HEIGHT);
	SET_EXIF_BSS(MF_TAG_BSS_ROI_X0, bss_param->BSS_ROI_X0);
	SET_EXIF_BSS(MF_TAG_BSS_ROI_Y0, bss_param->BSS_ROI_Y0);
	SET_EXIF_BSS(MF_TAG_BSS_VER, pBssDB->bss_ver);
	SET_EXIF_BSS(MF_TAG_BSS_SCALE_FACTOR, pBssDB->scale_factor);
	SET_EXIF_BSS(MF_TAG_BSS_CLIP_TH0, pBssDB->clip_th0);
	SET_EXIF_BSS(MF_TAG_BSS_CLIP_TH1, pBssDB->clip_th1);
	SET_EXIF_BSS(MF_TAG_BSS_CLIP_TH2, pBssDB->clip_th2);
	SET_EXIF_BSS(MF_TAG_BSS_CLIP_TH3, pBssDB->clip_th3);
	SET_EXIF_BSS(MF_TAG_BSS_ZERO, pBssDB->zero_gmv);
	SET_EXIF_BSS(MF_TAG_BSS_ADF_TH, pBssDB->adf_th);
	SET_EXIF_BSS(MF_TAG_BSS_SDF_TH, pBssDB->sdf_th);
	SET_EXIF_BSS(MF_TAG_BSS_YPF_EN, pBssDB->ypf_en);
	SET_EXIF_BSS(MF_TAG_BSS_YPF_FAC, pBssDB->ypf_fac);
	SET_EXIF_BSS(MF_TAG_BSS_YPF_ADJTH, pBssDB->ypf_adj_th);
	SET_EXIF_BSS(MF_TAG_BSS_YPF_DFMED0, pBssDB->ypf_dfmed0);
	SET_EXIF_BSS(MF_TAG_BSS_YPF_DFMED1, pBssDB->ypf_dfmed1);
	SET_EXIF_BSS(MF_TAG_BSS_YPF_TH0, pBssDB->ypf_th0);
	SET_EXIF_BSS(MF_TAG_BSS_YPF_TH1, pBssDB->ypf_th1);
	SET_EXIF_BSS(MF_TAG_BSS_YPF_TH2, pBssDB->ypf_th2);
	SET_EXIF_BSS(MF_TAG_BSS_YPF_TH3, pBssDB->ypf_th3);
	SET_EXIF_BSS(MF_TAG_BSS_YPF_TH4, pBssDB->ypf_th4);
	SET_EXIF_BSS(MF_TAG_BSS_YPF_TH5, pBssDB->ypf_th5);
	SET_EXIF_BSS(MF_TAG_BSS_YPF_TH6, pBssDB->ypf_th6);
	SET_EXIF_BSS(MF_TAG_BSS_YPF_TH7, pBssDB->ypf_th7);
	SET_EXIF_BSS(MF_TAG_BSS_FD_EN, pBssDB->fd_en);
	SET_EXIF_BSS(MF_TAG_BSS_FD_FAC, pBssDB->fd_fac);
	SET_EXIF_BSS(MF_TAG_BSS_FD_FNUM, pBssDB->fd_fnum);
	SET_EXIF_BSS(MF_TAG_BSS_EYE_EN, pBssDB->eye_en);
	SET_EXIF_BSS(MF_TAG_BSS_EYE_CFTH, pBssDB->eye_cfth);
	SET_EXIF_BSS(MF_TAG_BSS_EYE_RATIO0, pBssDB->eye_ratio0);
	SET_EXIF_BSS(MF_TAG_BSS_EYE_RATIO1, pBssDB->eye_ratio1);
	SET_EXIF_BSS(MF_TAG_BSS_EYE_FAC, pBssDB->eye_fac);
	SET_EXIF_BSS(MF_TAG_BSS_FACECVTH, pBssDB->FaceCVTh);
	SET_EXIF_BSS(MF_TAG_BSS_GRADTHL, pBssDB->GradThL);
	SET_EXIF_BSS(MF_TAG_BSS_GRADTHH, pBssDB->GradThH);
	SET_EXIF_BSS(MF_TAG_BSS_FACEAREATHL0, pBssDB->FaceAreaThL0);
	SET_EXIF_BSS(MF_TAG_BSS_FACEAREATHL1, pBssDB->FaceAreaThL1);
	SET_EXIF_BSS(MF_TAG_BSS_FACEAREATHH0, pBssDB->FaceAreaThH0);
	SET_EXIF_BSS(MF_TAG_BSS_FACEAREATHH1, pBssDB->FaceAreaThH1);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH0, pBssDB->APLDeltaTh0);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH1, pBssDB->APLDeltaTh1);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH2, pBssDB->APLDeltaTh2);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH3, pBssDB->APLDeltaTh3);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH4, pBssDB->APLDeltaTh4);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH5, pBssDB->APLDeltaTh5);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH6, pBssDB->APLDeltaTh6);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH7, pBssDB->APLDeltaTh7);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH8, pBssDB->APLDeltaTh8);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH9, pBssDB->APLDeltaTh9);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH10, pBssDB->APLDeltaTh10);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH11, pBssDB->APLDeltaTh11);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH12, pBssDB->APLDeltaTh12);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH13, pBssDB->APLDeltaTh13);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH14, pBssDB->APLDeltaTh14);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH15, pBssDB->APLDeltaTh15);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH16, pBssDB->APLDeltaTh16);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH17, pBssDB->APLDeltaTh17);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH18, pBssDB->APLDeltaTh18);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH19, pBssDB->APLDeltaTh19);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH20, pBssDB->APLDeltaTh20);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH21, pBssDB->APLDeltaTh21);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH22, pBssDB->APLDeltaTh22);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH23, pBssDB->APLDeltaTh23);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH24, pBssDB->APLDeltaTh24);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH25, pBssDB->APLDeltaTh25);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH26, pBssDB->APLDeltaTh26);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH27, pBssDB->APLDeltaTh27);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH28, pBssDB->APLDeltaTh28);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH29, pBssDB->APLDeltaTh29);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH30, pBssDB->APLDeltaTh30);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH31, pBssDB->APLDeltaTh31);
	SET_EXIF_BSS(MF_TAG_BSS_APLDELTATH32, pBssDB->APLDeltaTh32);
	SET_EXIF_BSS(MF_TAG_BSS_GRADRATIOTH0, pBssDB->GradRatioTh0);
	SET_EXIF_BSS(MF_TAG_BSS_GRADRATIOTH1, pBssDB->GradRatioTh1);
	SET_EXIF_BSS(MF_TAG_BSS_GRADRATIOTH2, pBssDB->GradRatioTh2);
	SET_EXIF_BSS(MF_TAG_BSS_GRADRATIOTH3, pBssDB->GradRatioTh3);
	SET_EXIF_BSS(MF_TAG_BSS_GRADRATIOTH4, pBssDB->GradRatioTh4);
	SET_EXIF_BSS(MF_TAG_BSS_GRADRATIOTH5, pBssDB->GradRatioTh5);
	SET_EXIF_BSS(MF_TAG_BSS_GRADRATIOTH6, pBssDB->GradRatioTh6);
	SET_EXIF_BSS(MF_TAG_BSS_GRADRATIOTH7, pBssDB->GradRatioTh7);
	SET_EXIF_BSS(MF_TAG_BSS_EYEDISTTHL, pBssDB->EyeDistThL);
	SET_EXIF_BSS(MF_TAG_BSS_EYEDISTTHH, pBssDB->EyeDistThH);
	SET_EXIF_BSS(MF_TAG_BSS_EYEMINWEIGHT, pBssDB->EyeMinWeight);
	SET_EXIF_BSS(MF_TAG_BSS_ACTS_CLIP_TH0, pBssDB->MF_BSS_ACTS_CLIP_TH0);
	SET_EXIF_BSS(MF_TAG_BSS_ACTS_CLIP_TH1, pBssDB->MF_BSS_ACTS_CLIP_TH1);
	SET_EXIF_BSS(MF_TAG_BSS_ISO_THH, pBssDB->MF_BSS_BLEND_ISO_ThH);
	SET_EXIF_BSS(MF_TAG_BSS_ISO_THL, pBssDB->MF_BSS_BLEND_ISO_ThL);
	SET_EXIF_BSS(MF_TAG_BSS_SHARP_MIN_WEIGHT, pBssDB->MF_BSS_Sharp_min_weight);
	SET_EXIF_BSS(MF_TAG_BSS_AI_SHUTTER_MODE_EN, pBssDB->AI_SHUTTER_MODE);
	SET_EXIF_BSS(MF_TAG_BSS_AI_SHUTTER_WEIGHT0, pBssDB->SCORE_WEIGHT0);
	SET_EXIF_BSS(MF_TAG_BSS_AI_SHUTTER_WEIGHT1, pBssDB->SCORE_WEIGHT1);
	SET_EXIF_BSS(MF_TAG_BSS_AI_SHUTTER_WEIGHT2, pBssDB->SCORE_WEIGHT2);
	SET_EXIF_BSS(MF_TAG_BSS_AI_SHUTTER_WEIGHT3, pBssDB->SCORE_WEIGHT3);
	SET_EXIF_BSS(MF_TAG_BSS_AI_SHUTTER_WEIGHT4, pBssDB->SCORE_WEIGHT4);
	SET_EXIF_BSS(MF_TAG_BSS_AI_SHUTTER_WEIGHT5, pBssDB->SCORE_WEIGHT5);
	SET_EXIF_BSS(MF_TAG_BSS_AI_SHUTTER_WEIGHT6, pBssDB->SCORE_WEIGHT6);
	SET_EXIF_BSS(MF_TAG_BSS_AI_SHUTTER_WEIGHT7, pBssDB->SCORE_WEIGHT7);
	SET_EXIF_BSS(MF_TAG_BSS_FRAME_NUM, bss_param->BSS_FRAME_NUM);
	SET_EXIF_BSS(MF_TAG_BSS_GAIN_TH0, bss_param->BSS_GAIN_TH0);
	SET_EXIF_BSS(MF_TAG_BSS_GAIN_TH1, bss_param->BSS_GAIN_TH1);
	SET_EXIF_BSS(MF_TAG_BSS_MIN_ISP_GAIN, bss_param->BSS_MIN_ISP_GAIN);
	SET_EXIF_BSS(MF_TAG_BSS_LCSO_SIZE, bss_param->BSS_LCSO_SIZE);
	SET_EXIF_BSS(MF_TAG_BSS_FD_TH0, bss_param->BSS_FD_TH0);
	SET_EXIF_BSS(MF_TAG_BSS_FD_TH1, bss_param->BSS_FD_TH1);
	SET_EXIF_BSS(MF_TAG_BSS_AEVC_EN, bss_param->BSS_AEVC_EN);
	SET_EXIF_BSS(MF_TAG_BSS_AEVC_DCNT, bss_param->BSS_AEVC_DCNT);
	SET_EXIF_BSS(MF_TAG_PROC_TYPE, BSS_TYPE_PACK_Y10);

#endif
}

MVOID BssWrapper::collectPostBSSExifData(std::vector<MINT32> &vNewIndex,
					 IBSS_OUTPUT_DATA &bss_output)
{
	(void)vNewIndex;
	(void)bss_output;

	/* bss result score */
	for (size_t i = 0; i < vNewIndex.size(); i++) {
		LOG(MtkISP7, Info) << "SharpScore[" << i
				   << "]  = " << bss_output.SharpScore[i] << ", adj1_score["
				   << i << "]  = " << bss_output.adj1_score[i]
				   << ", adj2_score[" << i
				   << "]  = " << bss_output.adj2_score[i] << ", adj3_score["
				   << i << "]  = " << bss_output.adj3_score[i]
				   << ", final_score[" << i
				   << "]  = " << bss_output.final_score[i];
	}
}

std::vector<int> BssWrapper::doBss(int frameNum, BssFramesBuffers &bssFramesBuffers)
{
	std::shared_ptr<mtk::isphal::v1::isp_bss_Param> dbParam = bssFramesBuffers.in.db_param;
	std::vector<int> BSSOrder;

	std::vector<int> doBssIndex;
	for (auto i = 0; i < frameNum; i++) {
		doBssIndex.push_back(i);
	}
	IBSS_WB_STRUCT workingBufferInfo;
	std::unique_ptr<MUINT8[]> bss_working_buffer;

	FrameBuffer *mainFrame = bssFramesBuffers.in.imgiBuffers[0];

	mZipData.imgWidth = camsysYuvSize_.width;
	mZipData.imgHeight = camsysYuvSize_.height;
	mZipData.imgFormat = NSCam::eImgFmt_MTK_YUV_P010;
	mZipData.imgStride[0] = mainFrame->planes()[0].stride;
	LOG(MtkISP7, Info) << "mZipData size: " << mZipData.imgWidth << "x"
			   << mZipData.imgHeight << ", total size: ["
			   << mZipData.imgSize[0] << ", " << mZipData.imgSize[1]
			   << ", " << mZipData.imgSize[2] << "]";

	LOG(MtkISP7, Info) << "format: " << mZipData.imgFormat << ", stride: ["
			   << mZipData.imgStride[0] << ", " << mZipData.imgStride[1]
			   << ", " << mZipData.imgStride[2] << "]";

	workingBufferInfo.rProcId = IBSS_PROC3;
	workingBufferInfo.u4Width = mZipData.imgWidth;
	workingBufferInfo.u4Height = mZipData.imgHeight;
	workingBufferInfo.u4FrameNum = frameNum;
	workingBufferInfo.u4WKSize = 0; // it will return working buffer require size
	workingBufferInfo.pu1BW = nullptr; // assign working buffer latrer.

	LOG(MtkISP7, Info) << "IBSS_FTCTRL_GET_WB_SIZE";
	auto b = bssFeatureCtrl(IBSS_FTCTRL_GET_WB_SIZE,
				reinterpret_cast<void *>(&workingBufferInfo), NULL);
	if (b != S_BSS_OK) {
		LOG(MtkISP7, Error) << "get working buffer size from MTKBss failed: " << b;
		return BSSOrder;
	}
	if (workingBufferInfo.u4WKSize <= 0) {
		LOG(MtkISP7, Error) << "unexpected bss working buffer size: "
				    << workingBufferInfo.u4WKSize;
		return BSSOrder;
	}
	bss_working_buffer =
		std::unique_ptr<MUINT8[]>(new MUINT8[workingBufferInfo.u4WKSize]{ 0 });
	workingBufferInfo.pu1BW =
		bss_working_buffer.get(); // assign working buffer for bss algo.
	LOG(MtkISP7, Info) << "rProcId    = " << workingBufferInfo.rProcId
			   << ", u4Width    = " << workingBufferInfo.u4Width
			   << ", u4Height   = " << workingBufferInfo.u4Height
			   << ", u4FrameNum = " << workingBufferInfo.u4FrameNum
			   << ", u4WKSize   = " << workingBufferInfo.u4WKSize
			   << ", pu1BW      = " << workingBufferInfo.pu1BW;

	LOG(MtkISP7, Info) << "IBSS_FTCTRL_SET_WB_SIZE";
	b = bssFeatureCtrl(IBSS_FTCTRL_SET_WB_SIZE,
			   reinterpret_cast<void *>(&workingBufferInfo), NULL);
	if (b != S_BSS_OK) {
		LOG(MtkISP7, Error) << "set working buffer to MTKBss failed, size = "
				    << workingBufferInfo.u4WKSize << ", b = " << b;
	}

	IBSS_PARAM_STRUCT bssParam;
	IBSS_PARAM_STRUCT *bss_param = reinterpret_cast<IBSS_PARAM_STRUCT *>(bssFramesBuffers.in.bssParamInfo->planes()[0].data());
	memcpy(bss_param, &bssParam, sizeof(IBSS_PARAM_STRUCT));
	updateBssProcInfo(
		bss_param, frameNum,
		Size{ (unsigned int)mZipData.imgWidth, (unsigned int)mZipData.imgHeight },
		dbParam);

	b = bssFeatureCtrl(IBSS_FTCTRL_SET_PROC_INFO,
			   reinterpret_cast<void *>(bss_param), NULL);
	if (b != S_BSS_OK) {
		LOG(MtkISP7, Error) << "Set info to MTKBss failed " << b;
	}

	IBSS_INPUT_DATA_G_IPC bssInData;
	IBSS_OUTPUT_DATA bssOutData;
	vector<MTKBSSFDInfo> bssFdData;

	memset(&bssInData, 0, sizeof(bssInData));
	memset(&bssOutData, 0, sizeof(bssOutData));

	updateBssIOInfo(bssInData);

	for (auto i = 0; i < (int)bssFramesBuffers.in.imgiBuffers.size(); i++) {
		// TODO: check with Han-lin: why is this needed?
		DmaSyncer syncer(bssFramesBuffers.in.imgiBuffers[i]->planes()[0].fd.get());
	}

	appendBSSInput(bssFramesBuffers.in.imgi, bssInData);

	if (NSCam::isHalRawFormat((NSCam::EImageFormat)mZipData.imgFormat)) {
		bssInData.eType = IBSS_TYPE_PACK_RAW10;
	} else if (mZipData.imgFormat == NSCam::EImageFormat::eImgFmt_NV21) {
		bssInData.eType = IBSS_TYPE_UNPACK_NV21;
	} else {
		bssInData.eType = IBSS_TYPE_PACK_Y10;
	}
	bssInData.InputZip = 0;
	LOG(MtkISP7, Info) << "bssInData.eType = " << bssInData.eType
			   << ", bssInData.InputZip = " << bssInData.InputZip;

	collectPreBSSExifData(bss_param);
	b = bssMain(IBSS_PROC3, &bssInData, &bssOutData);

	// dump bss input info to text file for bss simulation
	// dumpBssInputData2File(pMainRequest, bss_param, bssInData, bssOutData,
	// doBssRequests, pIBss);
	if (b != S_BSS_OK) {
		LOG(MtkISP7, Error) << "MTKBss::Main returns failed " << (int)b;
	}

	std::vector<MINT32> vNewOrdering;
	MUINT32 order = 0;
	MUINT32 bssOrder_group1 = 0;
	for (size_t i = 0, bss_idx = 0; i < (unsigned long)frameNum; i++) {
		MINT32 newOrder =
			(mEnableBSSOrdering == 0) ? bss_idx : bssOutData.originalOrder[bss_idx];
		vNewOrdering.push_back(newOrder);
		order = (mEnableBSSOrdering == 0)
				? i
				: doBssIndex[bssOutData.originalOrder[bss_idx]];
		if (bss_idx == 0) {
			LOG(MtkISP7, Info) << "set doBssRequests["
					   << bssOutData.originalOrder[bss_idx] << "] as Golden";
		}
		bss_idx++;

		BSSOrder.push_back(order);
		LOG(MtkISP7, Info) << "bssOrder " << i << " -> " << order;
	}

	for (auto o : BSSOrder) {
		bssOrder_group1 = (bssOrder_group1 >> 4) | (o << 12);
	}
	mExifData[MF_TAG_BSS_ORDER_GROUP_1] = bssOrder_group1;
	mExifData[MF_TAG_BSS_ORDER_GROUP_2] = 0;
	IBSS_INPUT_DATA_G *bss_dataG = reinterpret_cast<IBSS_INPUT_DATA_G *>(bssFramesBuffers.in.bssDataGInfo->planes()[0].data());
	memcpy(bss_dataG, &bssInData, sizeof(IBSS_INPUT_DATA_G));

	IBSS_OUTPUT_DATA *bss_outData = reinterpret_cast<IBSS_OUTPUT_DATA *>(bssFramesBuffers.out.bssOutDataInfo->planes()[0].data());
	memcpy(bss_outData, &bssOutData, sizeof(IBSS_OUTPUT_DATA));

	collectPostBSSExifData(vNewOrdering, bssOutData);

	IPASS_BSS_VerInfo verInfo;
	bssFeatureCtrl(IBSS_FTCTRL_GET_VERSION, NULL, &verInfo);
	char ver[15] = { '.' };
	size_t offset = 0;
	memcpy(&ver[offset], verInfo.rMainVer, strlen(verInfo.rMainVer));
	offset += strlen(verInfo.rMainVer);
	ver[offset] = ',';
	offset++;
	memcpy(&ver[offset], verInfo.rPatchVer, strlen(verInfo.rPatchVer));
	offset += strlen(verInfo.rPatchVer);
	ver[offset] = ',';
	offset++;
	memcpy(&ver[offset], verInfo.rSubVer, strlen(verInfo.rSubVer));
	offset += strlen(verInfo.rSubVer);
	ver[offset] = '\0';

	IPASS_BSS_VerInfo *bss_VerInfo = reinterpret_cast<IPASS_BSS_VerInfo *>(bssFramesBuffers.in.bssVerInfo->planes()[0].data());
	memcpy(bss_VerInfo, (void *)(&ver), strlen(ver));

	for (int i = 0; i < kInputRawCount; i++) {
		IBssFaceMetadata *bss_fd = reinterpret_cast<IBssFaceMetadata *>(bssFramesBuffers.in.bssFdInfo[i]->planes()[0].data());
		IBssFace *bss_face = reinterpret_cast<IBssFace *>(bssFramesBuffers.in.bssFaceInfo[i]->planes()[0].data());
		IBssFaceInfo *bss_pos = reinterpret_cast<IBssFaceInfo *>(bssFramesBuffers.in.bssPosInfo[i]->planes()[0].data());
		if (bssInData.Face[i] != nullptr) {
			memcpy(bss_fd, reinterpret_cast<void *>(bssInData.Face[i]), sizeof(IBssFaceMetadata));
			int numberOfFaces = bssInData.Face[i]->number_of_faces;
			if (bssInData.Face[i]->faces != nullptr) {
				memcpy(bss_face, reinterpret_cast<void *>(bssInData.Face[i]->faces), sizeof(IBssFace) * numberOfFaces);
			} else {
				LOG(MtkISP7, Info) << "bssInData.Face[" << i << "].faces is null";
				IBssFace dummy_faces[15];
				memcpy(bss_face, reinterpret_cast<void *>(dummy_faces), sizeof(IBssFace) * 15);
			}
			if (bssInData.Face[i]->posInfo != nullptr) {
				memcpy(bss_pos, reinterpret_cast<void *>(bssInData.Face[i]->posInfo), sizeof(IBssFaceInfo) * numberOfFaces);
			} else {
				LOG(MtkISP7, Info) << "bssInData.Face[" << i << "].posInfo is null";
				IBssFaceInfo dummy_posInfo[15];
				memcpy(bss_pos, reinterpret_cast<void *>(dummy_posInfo), sizeof(IBssFaceInfo) * 15);
			}
		} else {
			LOG(MtkISP7, Info) << "bssInData.Face[" << i << "] is null";
			IBssFaceMetadata dummy_facedata;
			memcpy(bss_fd, reinterpret_cast<void *>(&dummy_facedata), sizeof(IBssFaceMetadata));
			IBssFace dummy_faces[15];
			memcpy(bss_face, reinterpret_cast<void *>(dummy_faces), sizeof(IBssFace) * 15);
			IBssFaceInfo dummy_posInfo[15];
			memcpy(bss_pos, reinterpret_cast<void *>(dummy_posInfo), sizeof(IBssFaceInfo) * 15);
		}
	}

	return BSSOrder;
}

} // namespace libcamera

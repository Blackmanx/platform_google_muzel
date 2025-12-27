/*
 * Copyright (C) 2024, Google Inc.
 *
 * swme.cpp - MtkISP7 ImgSys Device Softeware Motion Estimate
 */

#include "swme.h"

#include <libcamera/base/log.h>

#include "bss.h"

namespace libcamera {
LOG_DECLARE_CATEGORY(MtkISP7)

SwmeWrapper::SwmeWrapper()
	: m_widthMe(0),
	  m_heightMe(0)
{
	DRVMfbllObject_s Pass_DRVMfbllObject_s = DRV_MFBLL_OBJ_SW;
	m_pMfbllDrv = MTKMfbll::createInstance(Pass_DRVMfbllObject_s);
}

SwmeWrapper::~SwmeWrapper()
{
	if (m_pMfbllDrv) {
		m_pMfbllDrv->destroyInstance();
		m_pMfbllDrv = nullptr;
	}
}

MRESULT SwmeWrapper::init()
{
	IMFBLL_INIT_PARAM_STRUCT initParam;
	initParam.Proc1_imgW = static_cast<MUINT16>(m_widthMe); // image width
	initParam.Proc1_imgH = static_cast<MUINT16>(m_heightMe); // image height
	initParam.core_num = static_cast<MUINT32>(8);
	//TODO, check if Proc1_DSUS_mode is always 0
	initParam.Proc1_DSUS_mode = 0;

	LOG(MtkISP7, Info) << "Proc1_imgW: " << initParam.Proc1_imgH
			   << ", Proc1_imgH: " << initParam.Proc1_imgW
			   << ", core_num: " << initParam.core_num
			   << ", Proc1_DSUS_mode: " << initParam.Proc1_DSUS_mode;

	MRESULT ErrCode = S_MFBLL_OK;
	ErrCode = m_pMfbllDrv->MfbllInit(&initParam, NULL);
	if (ErrCode != S_MFBLL_OK) {
		LOG(MtkISP7, Info) << "MfbllInit error!!,  ErrCode = " << ErrCode;
	}
	//Prepare working buffer
	IMFBLL_SET_PROC_INFO_STRUCT_IPC param;
	featureCtrl(IMFBLL_FTCTRL_GET_PROC_INFO,
		    &param, // no need
		    &m_WorkingBufInfo);
	LOG(MtkISP7, Info) << "working buffer size = " << m_WorkingBufInfo.Ext_mem_size
			   << ", conf map size = " << m_WorkingBufInfo.CofMap_width << "x" << m_WorkingBufInfo.CofMap_height
			   << ", motion vec size = " << m_WorkingBufInfo.MV_width << "x" << m_WorkingBufInfo.MV_height
			   << ", warp map size = " << m_WorkingBufInfo.WpeMap_width << "x" << m_WorkingBufInfo.WpeMap_height;
	return ErrCode;
}

void SwmeWrapper::reset()
{
}

MRESULT SwmeWrapper::swmeMain(IMFBLL_PROC_ENUM ProcId, void *pParaIn, void *pParaOut)
{
	MRESULT ErrCode = S_MFBLL_OK;
	MFBLL_PROC_ENUM Pass_MFBLL_PROC_ENUM;
	switch (ProcId) {
	case IMFBLL_PROC1: {
		Pass_MFBLL_PROC_ENUM = MFBLL_PROC1;
	} break;
	case IMFBLL_UNKNOWN_PROC: {
		Pass_MFBLL_PROC_ENUM = MFBLL_UNKNOWN_PROC;
	} break;
	}
	void *pMfbllInParse = Parser_MfbllIn(pParaIn);
	void *pMfbllOutParse = Parser_MfbllOut(pParaOut);
	ErrCode = m_pMfbllDrv->MfbllMain(Pass_MFBLL_PROC_ENUM, pMfbllInParse, pMfbllOutParse);
	if (ErrCode != S_MFBLL_OK) {
		LOG(MtkISP7, Error) << "MfbllMain error!!,  ErrCode = " << ErrCode;
	}
	//Parser_MfbllIn_Done(pMfbllInParse, pParaIn);
	Parser_MfbllOut_Done(pMfbllOutParse, pParaOut);
	return ErrCode;
}

MRESULT SwmeWrapper::featureCtrl(IMFBLL_FTCTRL_ENUM FcId, void *pParaIn, void *pParaOut)
{
	MRESULT ErrCode = S_MFBLL_OK;
	MFBLL_FTCTRL_ENUM Pass_MFBLL_FTCTRL_ENUM;
	switch (FcId) {
	case IMFBLL_FTCTRL_GET_PROC_INFO: {
		Pass_MFBLL_FTCTRL_ENUM = MFBLL_FTCTRL_GET_PROC_INFO;
	} break;
	case IMFBLL_FTCTRL_SET_PROC_INFO: {
		Pass_MFBLL_FTCTRL_ENUM = MFBLL_FTCTRL_SET_PROC_INFO;
	} break;
	case IMFBLL_FTCTRL_GET_VERSION: {
		Pass_MFBLL_FTCTRL_ENUM = MFBLL_FTCTRL_GET_VERSION;
	} break;
	case IMFBLL_FTCTRL_CAL_GYRO_MV: {
		Pass_MFBLL_FTCTRL_ENUM = MFBLL_FTCTRL_CAL_GYRO_MV;
	} break;
	case IMFBLL_FTCTRL_MAX: {
		Pass_MFBLL_FTCTRL_ENUM = MFBLL_FTCTRL_MAX;
	} break;
	}
	void *pIn = Parser_ParaIn(FcId, pParaIn);
	void *pOut = Parser_ParaOut(FcId, pParaOut);
	IMFBLL_GET_PROC_INFO_STRUCT *debug_out = &m_WorkingBufInfo;
	ErrCode = m_pMfbllDrv->MfbllFeatureCtrl(Pass_MFBLL_FTCTRL_ENUM, pIn, pOut);
	if (ErrCode != S_MFBLL_OK) {
		LOG(MtkISP7, Error) << "MfbllFeatureCtrl error!!,  ErrCode = " << ErrCode;
	}
	Parser_ParaIn_Done(FcId, pIn, pParaIn);
	Parser_ParaOut_Done(FcId, pOut, pParaOut);

	LOG(MtkISP7, Info) << "Ext_mem_size: " << debug_out->Ext_mem_size
			   << ", CofMap_width: " << debug_out->CofMap_width
			   << ", CofMap_height: " << debug_out->CofMap_height
			   << ", MV_width: " << debug_out->MV_width
			   << ", MV_height: " << debug_out->MV_height
			   << ", WpeMap_width: " << debug_out->WpeMap_width
			   << ", WpeMap_height: " << debug_out->WpeMap_height
			   << ", ErrCode = " << ErrCode;
	return ErrCode;
}

void *SwmeWrapper::Parser_MfbllIn(void *pParaIn)
{
	return pParaIn;
}

void *SwmeWrapper::Parser_MfbllOut(void *pParaOut)
{
	IMFBLL_PROC1_OUT_STRUCT *pMfbllOut = (IMFBLL_PROC1_OUT_STRUCT *)pParaOut;
	m_MfbllProcOut.pu1ConfMap = pMfbllOut->pu1ConfMap;
	m_MfbllProcOut.u4MapSize = pMfbllOut->u4MapSize;
	m_MfbllProcOut.pu1MV = pMfbllOut->pu1MV;
	m_MfbllProcOut.pi4WpeMapX = pMfbllOut->pi4WpeMapX;
	m_MfbllProcOut.pi4WpeMapY = pMfbllOut->pi4WpeMapY;
	m_MfbllProcOut.u4WpeMapSize = pMfbllOut->u4WpeMapSize;
	m_MfbllProcOut.u4MVSize = pMfbllOut->u4MVSize;
	return &m_MfbllProcOut;
}

/*
void SwmeWrapper::Parser_MfbllIn_Done(void* pParaIn, void* pParaParseIn)
{

    return;
}
*/
void SwmeWrapper::Parser_MfbllOut_Done(void *pParaOut, void *pParaParseOut)
{
	IPass_MFBLL_PROC1_OUT_STRUCT *pMfbllOut = (IPass_MFBLL_PROC1_OUT_STRUCT *)pParaOut;
	IMFBLL_PROC1_OUT_STRUCT *pMfbllParse = (IMFBLL_PROC1_OUT_STRUCT *)pParaParseOut;
	pMfbllParse->pu1ConfMap = pMfbllOut->pu1ConfMap;
	pMfbllParse->u4MapSize = pMfbllOut->u4MapSize;
	pMfbllParse->pu1MV = pMfbllOut->pu1MV;
	pMfbllParse->pi4WpeMapX = pMfbllOut->pi4WpeMapX;
	pMfbllParse->pi4WpeMapY = pMfbllOut->pi4WpeMapY;
	pMfbllParse->u4WpeMapSize = pMfbllOut->u4WpeMapSize;
	pMfbllParse->u4MVSize = pMfbllOut->u4MVSize;
}

void *SwmeWrapper::Parser_ParaIn(IMFBLL_FTCTRL_ENUM FcId, void *pParaIn)
{
	switch (FcId) {
	case IMFBLL_FTCTRL_GET_PROC_INFO:
	case IMFBLL_FTCTRL_SET_PROC_INFO:
	case IMFBLL_FTCTRL_CAL_GYRO_MV: {
		IMFBLL_SET_PROC_INFO_STRUCT *pMfbllSetProcInfo = (IMFBLL_SET_PROC_INFO_STRUCT *)pParaIn;
		m_MfbllSetProc.workbuf_addr = pMfbllSetProcInfo->workbuf_addr;
		m_MfbllSetProc.buf_size = pMfbllSetProcInfo->buf_size;
		m_MfbllSetProc.Proc1_base = pMfbllSetProcInfo->Proc1_base;
		m_MfbllSetProc.Proc1_ref = pMfbllSetProcInfo->Proc1_ref;
		m_MfbllSetProc.Proc1_width = pMfbllSetProcInfo->Proc1_width;
		m_MfbllSetProc.Proc1_height = pMfbllSetProcInfo->Proc1_height;
		m_MfbllSetProc.Proc1_ME_top_mode = pMfbllSetProcInfo->Proc1_ME_top_mode;
		m_MfbllSetProc.Proc1_ME_force_mode = pMfbllSetProcInfo->Proc1_ME_force_mode;
		m_MfbllSetProc.Proc1_HG_info_en = pMfbllSetProcInfo->Proc1_HG_info_en;
		m_MfbllSetProc.Proc1_me_wpe_image_width = pMfbllSetProcInfo->Proc1_me_wpe_image_width;
		m_MfbllSetProc.Proc1_me_wpe_image_height = pMfbllSetProcInfo->Proc1_me_wpe_image_height;
		m_MfbllSetProc.Proc1_me_wpe_np1_mode = pMfbllSetProcInfo->Proc1_me_wpe_np1_mode;
		m_MfbllSetProc.Proc1_me_wpe_stride = pMfbllSetProcInfo->Proc1_me_wpe_stride;

		m_MfbllSetProc.Proc1_FD_image_width = pMfbllSetProcInfo->Proc1_FD_image_width;
		m_MfbllSetProc.Proc1_FD_image_height = pMfbllSetProcInfo->Proc1_FD_image_height;
		m_MfbllSetProc.Proc1_Bst_FDROI_X0 = pMfbllSetProcInfo->Proc1_Bst_FDROI_X0;
		m_MfbllSetProc.Proc1_Bst_FDROI_Y0 = pMfbllSetProcInfo->Proc1_Bst_FDROI_Y0;
		m_MfbllSetProc.Proc1_Bst_FDROI_X1 = pMfbllSetProcInfo->Proc1_Bst_FDROI_X1;
		m_MfbllSetProc.Proc1_Bst_FDROI_Y1 = pMfbllSetProcInfo->Proc1_Bst_FDROI_Y1;
		m_MfbllSetProc.Proc1_Ref_FDROI_X0 = pMfbllSetProcInfo->Proc1_Ref_FDROI_X0;
		m_MfbllSetProc.Proc1_Ref_FDROI_Y0 = pMfbllSetProcInfo->Proc1_Ref_FDROI_Y0;
		m_MfbllSetProc.Proc1_Ref_FDROI_X1 = pMfbllSetProcInfo->Proc1_Ref_FDROI_X1;
		m_MfbllSetProc.Proc1_Ref_FDROI_Y1 = pMfbllSetProcInfo->Proc1_Ref_FDROI_Y1;
		m_MfbllSetProc.Proc1_Bst_LEYE_X0 = pMfbllSetProcInfo->Proc1_Bst_LEYE_X0;
		m_MfbllSetProc.Proc1_Bst_LEYE_X1 = pMfbllSetProcInfo->Proc1_Bst_LEYE_X1;
		m_MfbllSetProc.Proc1_Bst_LEYE_Y0 = pMfbllSetProcInfo->Proc1_Bst_LEYE_Y0;
		m_MfbllSetProc.Proc1_Bst_LEYE_Y1 = pMfbllSetProcInfo->Proc1_Bst_LEYE_Y1;
		m_MfbllSetProc.Proc1_Bst_LEYE_UX = pMfbllSetProcInfo->Proc1_Bst_LEYE_UX;
		m_MfbllSetProc.Proc1_Bst_LEYE_UY = pMfbllSetProcInfo->Proc1_Bst_LEYE_UY;
		m_MfbllSetProc.Proc1_Bst_LEYE_DX = pMfbllSetProcInfo->Proc1_Bst_LEYE_DX;
		m_MfbllSetProc.Proc1_Bst_LEYE_DY = pMfbllSetProcInfo->Proc1_Bst_LEYE_DY;
		m_MfbllSetProc.Proc1_Bst_REYE_X0 = pMfbllSetProcInfo->Proc1_Bst_REYE_X0;
		m_MfbllSetProc.Proc1_Bst_REYE_X1 = pMfbllSetProcInfo->Proc1_Bst_REYE_X1;
		m_MfbllSetProc.Proc1_Bst_REYE_Y0 = pMfbllSetProcInfo->Proc1_Bst_REYE_Y0;
		m_MfbllSetProc.Proc1_Bst_REYE_Y1 = pMfbllSetProcInfo->Proc1_Bst_REYE_Y1;
		m_MfbllSetProc.Proc1_Bst_REYE_UX = pMfbllSetProcInfo->Proc1_Bst_REYE_UX;
		m_MfbllSetProc.Proc1_Bst_REYE_UY = pMfbllSetProcInfo->Proc1_Bst_REYE_UY;
		m_MfbllSetProc.Proc1_Bst_REYE_DX = pMfbllSetProcInfo->Proc1_Bst_REYE_DX;
		m_MfbllSetProc.Proc1_Bst_REYE_DY = pMfbllSetProcInfo->Proc1_Bst_REYE_DY;
		m_MfbllSetProc.Proc1_Ref_LEYE_X0 = pMfbllSetProcInfo->Proc1_Ref_LEYE_X0;
		m_MfbllSetProc.Proc1_Ref_LEYE_X1 = pMfbllSetProcInfo->Proc1_Ref_LEYE_X1;
		m_MfbllSetProc.Proc1_Ref_LEYE_Y0 = pMfbllSetProcInfo->Proc1_Ref_LEYE_Y0;
		m_MfbllSetProc.Proc1_Ref_LEYE_Y1 = pMfbllSetProcInfo->Proc1_Ref_LEYE_Y1;
		m_MfbllSetProc.Proc1_Ref_LEYE_UX = pMfbllSetProcInfo->Proc1_Ref_LEYE_UX;
		m_MfbllSetProc.Proc1_Ref_LEYE_UY = pMfbllSetProcInfo->Proc1_Ref_LEYE_UY;
		m_MfbllSetProc.Proc1_Ref_LEYE_DX = pMfbllSetProcInfo->Proc1_Ref_LEYE_DX;
		m_MfbllSetProc.Proc1_Ref_LEYE_DY = pMfbllSetProcInfo->Proc1_Ref_LEYE_DY;
		m_MfbllSetProc.Proc1_Ref_REYE_X0 = pMfbllSetProcInfo->Proc1_Ref_REYE_X0;
		m_MfbllSetProc.Proc1_Ref_REYE_X1 = pMfbllSetProcInfo->Proc1_Ref_REYE_X1;
		m_MfbllSetProc.Proc1_Ref_REYE_Y0 = pMfbllSetProcInfo->Proc1_Ref_REYE_Y0;
		m_MfbllSetProc.Proc1_Ref_REYE_Y1 = pMfbllSetProcInfo->Proc1_Ref_REYE_Y1;
		m_MfbllSetProc.Proc1_Ref_REYE_UX = pMfbllSetProcInfo->Proc1_Ref_REYE_UX;
		m_MfbllSetProc.Proc1_Ref_REYE_UY = pMfbllSetProcInfo->Proc1_Ref_REYE_UY;
		m_MfbllSetProc.Proc1_Ref_REYE_DX = pMfbllSetProcInfo->Proc1_Ref_REYE_DX;
		m_MfbllSetProc.Proc1_Ref_REYE_DY = pMfbllSetProcInfo->Proc1_Ref_REYE_DY;
		m_MfbllSetProc.Proc1_base_ae_dgn_gain = pMfbllSetProcInfo->Proc1_base_ae_dgn_gain;
		m_MfbllSetProc.Proc1_base_ae_exposure_time = pMfbllSetProcInfo->Proc1_base_ae_exposure_time;
		m_MfbllSetProc.Proc1_base_ae_isp_gain = pMfbllSetProcInfo->Proc1_base_ae_isp_gain;
		m_MfbllSetProc.Proc1_base_ae_sensor_gain = pMfbllSetProcInfo->Proc1_base_ae_sensor_gain;
		m_MfbllSetProc.Proc1_ref_ae_dgn_gain = pMfbllSetProcInfo->Proc1_ref_ae_dgn_gain;
		m_MfbllSetProc.Proc1_ref_ae_exposure_time = pMfbllSetProcInfo->Proc1_ref_ae_exposure_time;
		m_MfbllSetProc.Proc1_ref_ae_isp_gain = pMfbllSetProcInfo->Proc1_ref_ae_isp_gain;
		m_MfbllSetProc.Proc1_ref_ae_sensor_gain = pMfbllSetProcInfo->Proc1_ref_ae_sensor_gain;
		IPASS_PROC_IMAGE_FORMAT Pass_PROC_IMAGE_FORMAT;
		switch (pMfbllSetProcInfo->Proc1_ImgFmt) {
		case IPROC1_FMT_YV16: {
			Pass_PROC_IMAGE_FORMAT = IPASS_PROC1_FMT_YV16;
		} break;
		case IPROC1_FMT_YUY2: {
			Pass_PROC_IMAGE_FORMAT = IPASS_PROC1_FMT_YUY2;
		} break;
		case IPROC1_FMT_NV12: {
			Pass_PROC_IMAGE_FORMAT = IPASS_PROC1_FMT_NV12;
		} break;
		case IPROC1_FMT_Y: {
			Pass_PROC_IMAGE_FORMAT = IPASS_PROC1_FMT_Y;
		} break;
		case IPROC1_FMT_Y16bit: {
			Pass_PROC_IMAGE_FORMAT = IPASS_PROC1_FMT_Y16bit;
		} break;
		case IPROC1_FMT_MAX: {
			Pass_PROC_IMAGE_FORMAT = IPASS_PROC1_FMT_MAX;
		}
		}
		m_MfbllSetProc.Proc1_ImgFmt = Pass_PROC_IMAGE_FORMAT;

		m_MfbllSetProc.iBssOrgScore_base = pMfbllSetProcInfo->iBssOrgScore_base;
		m_MfbllSetProc.iBssOrgScore_ref = pMfbllSetProcInfo->iBssOrgScore_ref;
		m_MfbllSetProc.Proc_idx = pMfbllSetProcInfo->Proc_idx;
		m_MfbllSetProc.pSWMENvram = pMfbllSetProcInfo->pSWMENvram;
		m_MfbllSetProc.HGInfo = pMfbllSetProcInfo->HGInfo;
		m_MfbllSetProc.GyroMVInfo = pMfbllSetProcInfo->GyroMVInfo;
		return &m_MfbllSetProc;

	} break;
	case IMFBLL_FTCTRL_GET_VERSION: {
		return pParaIn;
	} break;
	default:
		//LOG(MtkISP7, Error) << "[Parser_ParaIn] Feature Ctrl error" << FcId;
		break;
	}
	return pParaIn;
}
void *SwmeWrapper::Parser_ParaOut(IMFBLL_FTCTRL_ENUM FcId, void *pParaOut)
{
	switch (FcId) {
	case IMFBLL_FTCTRL_GET_PROC_INFO: {
		IMFBLL_GET_PROC_INFO_STRUCT *pMfbllGetProcInfo = (IMFBLL_GET_PROC_INFO_STRUCT *)pParaOut;

		m_MfbllGetProc.Ext_mem_size = pMfbllGetProcInfo->Ext_mem_size;
		m_MfbllGetProc.CofMap_width = pMfbllGetProcInfo->CofMap_width;
		m_MfbllGetProc.CofMap_height = pMfbllGetProcInfo->CofMap_height;
		m_MfbllGetProc.MV_width = pMfbllGetProcInfo->MV_width;
		m_MfbllGetProc.MV_height = pMfbllGetProcInfo->MV_height;
		m_MfbllGetProc.WpeMap_width = pMfbllGetProcInfo->WpeMap_width;
		m_MfbllGetProc.WpeMap_height = pMfbllGetProcInfo->WpeMap_height;
		return &m_MfbllGetProc;
	} break;
	case IMFBLL_FTCTRL_SET_PROC_INFO:
	case IMFBLL_FTCTRL_CAL_GYRO_MV:
	case IMFBLL_FTCTRL_GET_VERSION: {
		return pParaOut;
	} break;
	default:
		//LOGE("@@@ MfllCore/Memc [Parser_ParaOut] Feature Ctrl error: %d \n", FcId);
		break;
	}
	return pParaOut;
}

void SwmeWrapper::Parser_ParaIn_Done(IMFBLL_FTCTRL_ENUM FcId, void *pParaIn, void *pParaParseIn)
{
	switch (FcId) {
	case IMFBLL_FTCTRL_GET_PROC_INFO:
	case IMFBLL_FTCTRL_SET_PROC_INFO:
	case IMFBLL_FTCTRL_CAL_GYRO_MV: {
		IPass_MFBLL_SET_PROC_INFO_STRUCT *pMfbllSetProc = (IPass_MFBLL_SET_PROC_INFO_STRUCT *)pParaIn;
		IMFBLL_SET_PROC_INFO_STRUCT *pMfbllSetProcParse = (IMFBLL_SET_PROC_INFO_STRUCT *)pParaParseIn;

		pMfbllSetProcParse->workbuf_addr = pMfbllSetProc->workbuf_addr;
		pMfbllSetProcParse->buf_size = pMfbllSetProc->buf_size;
		pMfbllSetProcParse->Proc1_base = pMfbllSetProc->Proc1_base;
		pMfbllSetProcParse->Proc1_ref = pMfbllSetProc->Proc1_ref;
		pMfbllSetProcParse->Proc1_width = pMfbllSetProc->Proc1_width;
		pMfbllSetProcParse->Proc1_height = pMfbllSetProc->Proc1_height;
		pMfbllSetProcParse->Proc1_ME_top_mode = pMfbllSetProc->Proc1_ME_top_mode;
		pMfbllSetProcParse->Proc1_ME_force_mode = pMfbllSetProc->Proc1_ME_force_mode;
		pMfbllSetProcParse->Proc1_HG_info_en = pMfbllSetProc->Proc1_HG_info_en;
		pMfbllSetProcParse->Proc1_me_wpe_image_width = pMfbllSetProc->Proc1_me_wpe_image_width;
		pMfbllSetProcParse->Proc1_me_wpe_image_height = pMfbllSetProc->Proc1_me_wpe_image_height;
		pMfbllSetProcParse->Proc1_me_wpe_np1_mode = pMfbllSetProc->Proc1_me_wpe_np1_mode;
		pMfbllSetProcParse->Proc1_me_wpe_stride = pMfbllSetProc->Proc1_me_wpe_stride;
		pMfbllSetProcParse->Proc1_FD_image_width = pMfbllSetProc->Proc1_FD_image_width;
		pMfbllSetProcParse->Proc1_FD_image_height = pMfbllSetProc->Proc1_FD_image_height;
		pMfbllSetProcParse->Proc1_Bst_FDROI_X0 = pMfbllSetProc->Proc1_Bst_FDROI_X0;
		pMfbllSetProcParse->Proc1_Bst_FDROI_Y0 = pMfbllSetProc->Proc1_Bst_FDROI_Y0;
		pMfbllSetProcParse->Proc1_Bst_FDROI_X1 = pMfbllSetProc->Proc1_Bst_FDROI_X1;
		pMfbllSetProcParse->Proc1_Bst_FDROI_Y1 = pMfbllSetProc->Proc1_Bst_FDROI_Y1;
		pMfbllSetProcParse->Proc1_Ref_FDROI_X0 = pMfbllSetProc->Proc1_Ref_FDROI_X0;
		pMfbllSetProcParse->Proc1_Ref_FDROI_Y0 = pMfbllSetProc->Proc1_Ref_FDROI_Y0;
		pMfbllSetProcParse->Proc1_Ref_FDROI_X1 = pMfbllSetProc->Proc1_Ref_FDROI_X1;
		pMfbllSetProcParse->Proc1_Ref_FDROI_Y1 = pMfbllSetProc->Proc1_Ref_FDROI_Y1;
		pMfbllSetProcParse->Proc1_Bst_LEYE_X0 = pMfbllSetProc->Proc1_Bst_LEYE_X0;
		pMfbllSetProcParse->Proc1_Bst_LEYE_X1 = pMfbllSetProc->Proc1_Bst_LEYE_X1;
		pMfbllSetProcParse->Proc1_Bst_LEYE_Y0 = pMfbllSetProc->Proc1_Bst_LEYE_Y0;
		pMfbllSetProcParse->Proc1_Bst_LEYE_Y1 = pMfbllSetProc->Proc1_Bst_LEYE_Y1;
		pMfbllSetProcParse->Proc1_Bst_LEYE_UX = pMfbllSetProc->Proc1_Bst_LEYE_UX;
		pMfbllSetProcParse->Proc1_Bst_LEYE_UY = pMfbllSetProc->Proc1_Bst_LEYE_UY;
		pMfbllSetProcParse->Proc1_Bst_LEYE_DX = pMfbllSetProc->Proc1_Bst_LEYE_DX;
		pMfbllSetProcParse->Proc1_Bst_LEYE_DY = pMfbllSetProc->Proc1_Bst_LEYE_DY;
		pMfbllSetProcParse->Proc1_Bst_REYE_X0 = pMfbllSetProc->Proc1_Bst_REYE_X0;
		pMfbllSetProcParse->Proc1_Bst_REYE_X1 = pMfbllSetProc->Proc1_Bst_REYE_X1;
		pMfbllSetProcParse->Proc1_Bst_REYE_Y0 = pMfbllSetProc->Proc1_Bst_REYE_Y0;
		pMfbllSetProcParse->Proc1_Bst_REYE_Y1 = pMfbllSetProc->Proc1_Bst_REYE_Y1;
		pMfbllSetProcParse->Proc1_Bst_REYE_UX = pMfbllSetProc->Proc1_Bst_REYE_UX;
		pMfbllSetProcParse->Proc1_Bst_REYE_UY = pMfbllSetProc->Proc1_Bst_REYE_UY;
		pMfbllSetProcParse->Proc1_Bst_REYE_DX = pMfbllSetProc->Proc1_Bst_REYE_DX;
		pMfbllSetProcParse->Proc1_Bst_REYE_DY = pMfbllSetProc->Proc1_Bst_REYE_DY;
		pMfbllSetProcParse->Proc1_Ref_LEYE_X0 = pMfbllSetProc->Proc1_Ref_LEYE_X0;
		pMfbllSetProcParse->Proc1_Ref_LEYE_X1 = pMfbllSetProc->Proc1_Ref_LEYE_X1;
		pMfbllSetProcParse->Proc1_Ref_LEYE_Y0 = pMfbllSetProc->Proc1_Ref_LEYE_Y0;
		pMfbllSetProcParse->Proc1_Ref_LEYE_Y1 = pMfbllSetProc->Proc1_Ref_LEYE_Y1;
		pMfbllSetProcParse->Proc1_Ref_LEYE_UX = pMfbllSetProc->Proc1_Ref_LEYE_UX;
		pMfbllSetProcParse->Proc1_Ref_LEYE_UY = pMfbllSetProc->Proc1_Ref_LEYE_UY;
		pMfbllSetProcParse->Proc1_Ref_LEYE_DX = pMfbllSetProc->Proc1_Ref_LEYE_DX;
		pMfbllSetProcParse->Proc1_Ref_LEYE_DY = pMfbllSetProc->Proc1_Ref_LEYE_DY;
		pMfbllSetProcParse->Proc1_Ref_REYE_X0 = pMfbllSetProc->Proc1_Ref_REYE_X0;
		pMfbllSetProcParse->Proc1_Ref_REYE_X1 = pMfbllSetProc->Proc1_Ref_REYE_X1;
		pMfbllSetProcParse->Proc1_Ref_REYE_Y0 = pMfbllSetProc->Proc1_Ref_REYE_Y0;
		pMfbllSetProcParse->Proc1_Ref_REYE_Y1 = pMfbllSetProc->Proc1_Ref_REYE_Y1;
		pMfbllSetProcParse->Proc1_Ref_REYE_UX = pMfbllSetProc->Proc1_Ref_REYE_UX;
		pMfbllSetProcParse->Proc1_Ref_REYE_UY = pMfbllSetProc->Proc1_Ref_REYE_UY;
		pMfbllSetProcParse->Proc1_Ref_REYE_DX = pMfbllSetProc->Proc1_Ref_REYE_DX;
		pMfbllSetProcParse->Proc1_Ref_REYE_DY = pMfbllSetProc->Proc1_Ref_REYE_DY;
		pMfbllSetProcParse->Proc1_base_ae_dgn_gain = pMfbllSetProc->Proc1_base_ae_dgn_gain;
		pMfbllSetProcParse->Proc1_base_ae_exposure_time = pMfbllSetProc->Proc1_base_ae_exposure_time;
		pMfbllSetProcParse->Proc1_base_ae_isp_gain = pMfbllSetProc->Proc1_base_ae_isp_gain;
		pMfbllSetProcParse->Proc1_base_ae_sensor_gain = pMfbllSetProc->Proc1_base_ae_sensor_gain;
		pMfbllSetProcParse->Proc1_ref_ae_dgn_gain = pMfbllSetProc->Proc1_ref_ae_dgn_gain;
		pMfbllSetProcParse->Proc1_ref_ae_exposure_time = pMfbllSetProc->Proc1_ref_ae_exposure_time;
		pMfbllSetProcParse->Proc1_ref_ae_isp_gain = pMfbllSetProc->Proc1_ref_ae_isp_gain;
		pMfbllSetProcParse->Proc1_ref_ae_sensor_gain = pMfbllSetProc->Proc1_ref_ae_sensor_gain;
		pMfbllSetProcParse->iBssOrgScore_base = pMfbllSetProc->iBssOrgScore_base;
		pMfbllSetProcParse->iBssOrgScore_ref = pMfbllSetProc->iBssOrgScore_ref;
		pMfbllSetProcParse->Proc_idx = pMfbllSetProc->Proc_idx;
		pMfbllSetProcParse->pSWMENvram = pMfbllSetProc->pSWMENvram;
		pMfbllSetProcParse->HGInfo = pMfbllSetProc->HGInfo;
		pMfbllSetProcParse->GyroMVInfo = pMfbllSetProc->GyroMVInfo;
	} break;
	case IMFBLL_FTCTRL_GET_VERSION: {
	} break;
	default:
		//LOGE("@@@ MfllCore/Memc [Parser_ParaIn_Done] Feature Ctrl error: %d \n", FcId);
		break;
	}
	return;
}
void SwmeWrapper::Parser_ParaOut_Done(IMFBLL_FTCTRL_ENUM FcId, void *pParaOut, void *pParaParseOut)
{
	switch (FcId) {
	case IMFBLL_FTCTRL_GET_PROC_INFO: {
		IPass_MFBLL_GET_PROC_INFO_STRUCT *pMfbllGetProc = (IPass_MFBLL_GET_PROC_INFO_STRUCT *)pParaOut;
		IMFBLL_GET_PROC_INFO_STRUCT *pMfbllGetProcParse = (IMFBLL_GET_PROC_INFO_STRUCT *)pParaParseOut;

		pMfbllGetProcParse->Ext_mem_size = pMfbllGetProc->Ext_mem_size;
		pMfbllGetProcParse->CofMap_width = pMfbllGetProc->CofMap_width;
		pMfbllGetProcParse->CofMap_height = pMfbllGetProc->CofMap_height;
		pMfbllGetProcParse->MV_width = pMfbllGetProc->MV_width;
		pMfbllGetProcParse->MV_height = pMfbllGetProc->MV_height;
		pMfbllGetProcParse->WpeMap_width = pMfbllGetProc->WpeMap_width;
		pMfbllGetProcParse->WpeMap_height = pMfbllGetProc->WpeMap_height;
	} break;
	case IMFBLL_FTCTRL_SET_PROC_INFO:
	case IMFBLL_FTCTRL_CAL_GYRO_MV: {
		return;
	} break;
	case IMFBLL_FTCTRL_GET_VERSION: {
		IPASS_MFBLL_VerInfo *pMfbllVerInfo = (IPASS_MFBLL_VerInfo *)pParaOut;
		IMFBLL_VerInfo *pMfbllVerInfoParse = (IMFBLL_VerInfo *)pParaParseOut;

		for (int i = 0; i < 5; i++) {
			pMfbllVerInfoParse->rMainVer[i] = pMfbllVerInfo->rMainVer[i];
			pMfbllVerInfoParse->rSubVer[i] = pMfbllVerInfo->rSubVer[i];
			pMfbllVerInfoParse->rPatchVer[i] = pMfbllVerInfo->rPatchVer[i];
		}
	} break;
	default:
		//LOGE("@@@ MfllCore/Memc [Parser_ParaOut_Done] Feature Ctrl error: %d \n", FcId);
		break;
	}
	return;
}

// static
void SwmeWrapper::prepareParam(
	IMFBLL_SET_PROC_INFO_STRUCT_IPC &param,
	std::optional<MtkCameraFaceMetadata> &faceMetadata,
	SwmeFramesBuffers swmeFramesBuffers,
	Size frame_size,
	Size mc_size,
	int index)
{
	param.workbuf_addr =
		reinterpret_cast<MUINT8 *>(swmeFramesBuffers.in.workbuf->data());
	param.buf_size = swmeFramesBuffers.in.workbuf->size();
	param.Proc1_base = swmeFramesBuffers.in.base_buf->planes()[0].data();
	param.Proc1_ref = swmeFramesBuffers.in.ref_buf->planes()[0].data();

	param.Proc1_width = mc_size.width;
	param.Proc1_height = mc_size.height;

	param.Proc1_ImgFmt = IPROC1_FMT_Y;
	auto isHighResolution = [](Size size) {
		return (size.width >= 6532 || size.height >= 4898);
	};
	param.Proc1_me_wpe_image_width = frame_size.width;
	param.Proc1_me_wpe_image_height = frame_size.height;
	param.Proc1_me_wpe_np1_mode = isHighResolution(frame_size);
	param.Proc1_me_wpe_stride = swmeFramesBuffers.out.warpping_map_buffer->planes()[0].stride;
	swmeFramesBuffers.in.db_param->ME_RSV_4_1 = 700;
	param.pSWMENvram = swmeFramesBuffers.in.db_param.get();
	param.Proc1_ImgFmt = IPROC1_FMT_Y;
	param.Proc_idx = index;

	IBSS_OUTPUT_DATA *bssOut = reinterpret_cast<IBSS_OUTPUT_DATA *>(swmeFramesBuffers.in.bss_buf->planes()[0].data());
	auto bstIdx = bssOut->originalOrder[0];
	auto refIdx = bssOut->originalOrder[index + 1];
	param.iBssOrgScore_base = (MUINT32)bssOut->final_score[bstIdx];
	param.iBssOrgScore_ref = (MUINT32)bssOut->final_score[refIdx];
	LOG(MtkISP7, Info) << "workbuf_addr: " << static_cast<void *>(param.workbuf_addr)
			   << ", buf_size: " << param.buf_size
			   << ", Proc1_base: " << static_cast<void *>(param.Proc1_base)
			   << ", Proc1_ref: " << static_cast<void *>(param.Proc1_ref)
			   << ", Proc1_width: " << param.Proc1_width
			   << ", Proc1_height: " << param.Proc1_height
			   << ", ME_RSV_4_1: " << swmeFramesBuffers.in.db_param->ME_RSV_4_1
			   << ", bstIdx:" << bstIdx
			   << ", refIdx:" << refIdx
			   << ", iBssOrgScore_base: " << param.iBssOrgScore_base
			   << ", iBssOrgScore_ref:" << param.iBssOrgScore_ref
			   << ", Proc_idx: " << (int)param.Proc_idx;
	char buf[1024];
	sprintf(buf, "me wpe image size %dx%d stride:%d, wpe_np1_mode:%d",
		param.Proc1_me_wpe_image_width, param.Proc1_me_wpe_image_height,
		param.Proc1_me_wpe_stride, param.Proc1_me_wpe_np1_mode);
	;
	LOG(MtkISP7, Info) << buf;

	if (faceMetadata != std::nullopt) {
		param.Proc1_Bst_FDROI_X0 = faceMetadata->faces[0].rect[0];
		param.Proc1_Bst_FDROI_Y0 = faceMetadata->faces[0].rect[1];
		param.Proc1_Bst_FDROI_X1 = faceMetadata->faces[0].rect[2];
		param.Proc1_Bst_FDROI_Y1 = faceMetadata->faces[0].rect[3];
		param.Proc1_Ref_FDROI_X0 = faceMetadata->faces[0].rect[0];
		param.Proc1_Ref_FDROI_Y0 = faceMetadata->faces[0].rect[1];
		param.Proc1_Ref_FDROI_X1 = faceMetadata->faces[0].rect[2];
		param.Proc1_Ref_FDROI_Y1 = faceMetadata->faces[0].rect[3];

		param.Proc1_Bst_LEYE_X0 = faceMetadata->leyex0[0];
		param.Proc1_Bst_LEYE_X1 = faceMetadata->leyex1[0];
		param.Proc1_Bst_LEYE_Y0 = faceMetadata->leyey0[0];
		param.Proc1_Bst_LEYE_Y1 = faceMetadata->leyey1[0];
		param.Proc1_Bst_LEYE_UX = faceMetadata->leyeux[0];
		param.Proc1_Bst_LEYE_UY = faceMetadata->leyeuy[0];
		param.Proc1_Bst_LEYE_DX = faceMetadata->leyedx[0];
		param.Proc1_Bst_LEYE_DY = faceMetadata->leyedy[0];
		param.Proc1_Bst_REYE_X0 = faceMetadata->reyex0[0];
		param.Proc1_Bst_REYE_X1 = faceMetadata->reyex1[0];
		param.Proc1_Bst_REYE_Y0 = faceMetadata->reyey0[0];
		param.Proc1_Bst_REYE_Y1 = faceMetadata->reyey1[0];
		param.Proc1_Bst_REYE_UX = faceMetadata->reyeux[0];
		param.Proc1_Bst_REYE_UY = faceMetadata->reyeuy[0];
		param.Proc1_Bst_REYE_DX = faceMetadata->reyedx[0];
		param.Proc1_Bst_REYE_DY = faceMetadata->reyedy[0];

		param.Proc1_Ref_LEYE_X0 = faceMetadata->leyex0[0];
		param.Proc1_Ref_LEYE_X1 = faceMetadata->leyex1[0];
		param.Proc1_Ref_LEYE_Y0 = faceMetadata->leyey0[0];
		param.Proc1_Ref_LEYE_Y1 = faceMetadata->leyey1[0];
		param.Proc1_Ref_LEYE_UX = faceMetadata->leyeux[0];
		param.Proc1_Ref_LEYE_UY = faceMetadata->leyeuy[0];
		param.Proc1_Ref_LEYE_DX = faceMetadata->leyedx[0];
		param.Proc1_Ref_LEYE_DY = faceMetadata->leyedy[0];
		param.Proc1_Ref_REYE_X0 = faceMetadata->reyex0[0];
		param.Proc1_Ref_REYE_X1 = faceMetadata->reyex1[0];
		param.Proc1_Ref_REYE_Y0 = faceMetadata->reyey0[0];
		param.Proc1_Ref_REYE_Y1 = faceMetadata->reyey1[0];
		param.Proc1_Ref_REYE_UX = faceMetadata->reyeux[0];
		param.Proc1_Ref_REYE_UY = faceMetadata->reyeuy[0];
		param.Proc1_Ref_REYE_DX = faceMetadata->reyedx[0];
		param.Proc1_Ref_REYE_DY = faceMetadata->reyedy[0];
	}

	sprintf(buf, "Bst_FDROI X0=%d, Y0=%d, X1=%d, Y1=%d",
		param.Proc1_Bst_FDROI_X0, param.Proc1_Bst_FDROI_Y0,
		param.Proc1_Bst_FDROI_X1, param.Proc1_Bst_FDROI_Y1);
	LOG(MtkISP7, Info) << buf;

	sprintf(buf, "Ref_FDROI X0=%d, Y0=%d, X1=%d, Y1=%d",
		param.Proc1_Ref_FDROI_X0, param.Proc1_Ref_FDROI_Y0,
		param.Proc1_Ref_FDROI_X1, param.Proc1_Ref_FDROI_Y1);
	LOG(MtkISP7, Info) << buf;

	sprintf(buf,
		"Bst_LEYE X0=%d, X1=%d, Y0=%d, Y1=%d, UX=%d, UY=%d, XD=%d, DY=%d",
		param.Proc1_Bst_LEYE_X0, param.Proc1_Bst_LEYE_X1, param.Proc1_Bst_LEYE_Y0, param.Proc1_Bst_LEYE_Y1,
		param.Proc1_Bst_LEYE_UX, param.Proc1_Bst_LEYE_UY, param.Proc1_Bst_LEYE_DX, param.Proc1_Bst_LEYE_DY);
	LOG(MtkISP7, Info) << buf;
	sprintf(buf,
		"Bst_REYE X0=%d, X1=%d, Y0=%d, Y1=%d, UX=%d, UY=%d, XD=%d, DY=%d",
		param.Proc1_Bst_REYE_X0, param.Proc1_Bst_REYE_X1, param.Proc1_Bst_REYE_Y0, param.Proc1_Bst_REYE_Y1,
		param.Proc1_Bst_REYE_UX, param.Proc1_Bst_REYE_UY, param.Proc1_Bst_REYE_DX, param.Proc1_Bst_REYE_DY);
	LOG(MtkISP7, Info) << buf;
	sprintf(buf,
		"Ref_LEYE X0=%d, X1=%d, Y0=%d, Y1=%d, UX=%d, UY=%d, XD=%d, DY=%d",
		param.Proc1_Ref_LEYE_X0, param.Proc1_Ref_LEYE_X1, param.Proc1_Ref_LEYE_Y0, param.Proc1_Ref_LEYE_Y1,
		param.Proc1_Ref_LEYE_UX, param.Proc1_Ref_LEYE_UY, param.Proc1_Ref_LEYE_DX, param.Proc1_Ref_LEYE_DY);
	LOG(MtkISP7, Info) << buf;
	sprintf(buf,
		"Ref_REYE X0=%d, X1=%d, Y0=%d, Y1=%d, UX=%d, UY=%d, XD=%d, DY=%d",
		param.Proc1_Ref_REYE_X0, param.Proc1_Ref_REYE_X1, param.Proc1_Ref_REYE_Y0, param.Proc1_Ref_REYE_Y1,
		param.Proc1_Ref_REYE_UX, param.Proc1_Ref_REYE_UY, param.Proc1_Ref_REYE_DX, param.Proc1_Ref_REYE_DY);
	LOG(MtkISP7, Info) << buf;

	LOG(MtkISP7, Info) << "SWME Proc1_ImgFmt: " << param.Proc1_ImgFmt
			   << ", SWME Proc_idx: " << param.Proc_idx
			   << ", mfnr_.pSWMENvram addr = " << static_cast<void *>(param.pSWMENvram);
}

// static
void SwmeWrapper::prepareOutParam(
	IMFBLL_PROC1_OUT_STRUCT_IPC *paramOut,
	SwmeFramesBuffers swmeFramesBuffers)
{
	paramOut->pu1ConfMap = swmeFramesBuffers.out.conf_map->planes()[0].data();
	paramOut->u4MapSize = swmeFramesBuffers.out.conf_map_buffer->planes()[0].length;
	paramOut->pi4WpeMapX = static_cast<MINT32 *>(reinterpret_cast<void *>(swmeFramesBuffers.out.warpping_map->planes()[0].data()));
	paramOut->pi4WpeMapY = static_cast<MINT32 *>(reinterpret_cast<void *>(swmeFramesBuffers.out.warpping_map->planes()[1].data()));
	paramOut->u4WpeMapSize += swmeFramesBuffers.out.warpping_map_buffer->planes()[0].length;
	paramOut->pu1MV = static_cast<MUINT8 *>(swmeFramesBuffers.out.mcmv->planes()[0].data());
	paramOut->u4MVSize = swmeFramesBuffers.out.mcmv_buffer->planes()[0].length;
	LOG(MtkISP7, Info) << "pu1ConfMap: " << static_cast<void *>(paramOut->pu1ConfMap)
			   << ", pi4WpeMapX: " << static_cast<void *>(paramOut->pi4WpeMapX)
			   << ", pi4WpeMapY: " << static_cast<void *>(paramOut->pi4WpeMapY)
			   << ", pu1MV: " << static_cast<void *>(paramOut->pu1MV)
			   << ", u4MapSize: " << paramOut->u4MapSize
			   << ", u4WpeMapSize: " << paramOut->u4WpeMapSize
			   << ", u4MVSize: " << paramOut->u4MVSize;
}

} /* namespace libcamera */

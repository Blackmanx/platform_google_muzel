/*
 * Copyright (C) 2024, Google Inc.
 *
 * const.h - Constants of ImgSys.
 */

#pragma once

#include <cstring>

#include "libcamera/internal/info_frame.h"

#include "libmfnr/MTKBssType.h"
#include "mtkcam-core/libcamera_ext/lib/libBssWrapper/MTKBssHeader/EBss.h"
#include "mtkcam-core/libcamera_ext/lib/libBssWrapper/MTKBssHeader/IMTKBss.h"
#include "mtkcam-core/libcamera_ext/lib/libMfbllWrapper/MTKMfbllHeader/IMTKMfbll.h"
#include "mtkcam-core/libcamera_ext/lib/libMfbllWrapper/MTKMfbllHeader/include/EMfbll.h"
#include "mtkcam-interfaces/utils/hw/faces.h"

#include "MediaTypes.h"

namespace libcamera {

struct MfnrInput {
	SharedMailBox<InfoFrame> raw;
	SharedMailBox<InfoFrame> yuvo1;
};

constexpr int kInputRawCount = 4;

struct ZipOutData {
	MINT32 imgWidth;
	MINT32 imgHeight;
	MINT32 imgFormat;
	MINT32 imgPlaneCount;
	MINT32 imgStride[3];
	MINT32 imgSize[3];
	MINT32 frameIndex; // MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX
	ZipOutData()
		: imgWidth{},
		  imgHeight{},
		  imgFormat{},
		  imgPlaneCount{},
		  imgStride{},
		  imgSize{},
		  frameIndex{} {}
};

typedef enum IBSS_PROC_ENUM {
	IBSS_PROC1 = 0,
	IBSS_PROC2,
	IBSS_PROC3,
	IBSS_UNKNOWN_PROC,
} IBSS_PROC_ENUM;

struct IBSS_WB_STRUCT {
	IBSS_PROC_ENUM rProcId; // [User SET]process mode
	MUINT32 u4Width; // [User SET] frame width
	MUINT32 u4Height; // [User SET] frame height
	MUINT32 u4FrameNum; // [User SET] frame number
	MUINT32 u4WKSize; // [return value] working buffer require size
	MUINT8 *pu1BW; // [User SET] frame number

	// constructor
	IBSS_WB_STRUCT() { ::memset(this, 0x0, sizeof(struct IBSS_WB_STRUCT)); }
};

struct IBSS_PARAM_STRUCT {
	MUINT8 BSS_ON;
	MUINT32 BSS_ROI_WIDTH;
	MUINT32 BSS_ROI_HEIGHT;
	MUINT32 BSS_ROI_X0;
	MUINT32 BSS_ROI_Y0;
	void *pBSSNvram;
	MUINT32 BSS_FRAME_NUM;
	MUINT32 BSS_GAIN_TH0;
	MUINT32 BSS_GAIN_TH1;
	MUINT32 BSS_MIN_ISP_GAIN;
	MUINT32 BSS_LCSO_SIZE;
	MUINT8 BSS_FD_TH0;
	MUINT8 BSS_FD_TH1;
	MUINT8 BSS_AEVC_EN;
	MUINT16 BSS_AEVC_DCNT;

	// constructor
	IBSS_PARAM_STRUCT()
		: BSS_ON(1),
		  BSS_FRAME_NUM(4),
		  BSS_GAIN_TH0(853),
		  BSS_GAIN_TH1(1229),
		  BSS_MIN_ISP_GAIN(546),
		  BSS_LCSO_SIZE(147456),
		  BSS_FD_TH0(5),
		  BSS_FD_TH1(40),
		  BSS_AEVC_EN(0),
		  BSS_AEVC_DCNT(512) {}
};

struct IBSS_CONFIG_ZIP_IN_STRUCT {
	MUINT32 u4Width; // [User SET] frame width
	MUINT32 u4Height; // [User SET] frame height
	IBSS_PROC_TYPE eType;
	MUINT32 u4ROI_X0;
	MUINT32 u4ROI_Y0;
	MUINT32 u4ROI_WIDTH;
	MUINT32 u4ROI_HEIGHT;
	MUINT32 u4SCALE_FACTOR;
	void *pBSSNvram;

	// constructor
	IBSS_CONFIG_ZIP_IN_STRUCT()
	{
		::memset(this, 0x0, sizeof(struct IBSS_CONFIG_ZIP_IN_STRUCT));
	}
};

struct IBSS_PROC_ZIP_STRUCT {
	MUINT8 *pu8InImg;
	MUINT32 u4ImgInStride;
	MUINT32 u4ImgInSize;
	MUINT32 u4ImgInBayerOrder; // for raw input
	MUINT8 *pu8OutImg;
	MUINT32 u4ImgOutSize;

	MINT32 i4GmvX;
	MINT32 i4GmvY;

	// constructor
	IBSS_PROC_ZIP_STRUCT()
	{
		::memset(this, 0x0, sizeof(struct IBSS_PROC_ZIP_STRUCT));
	}
};

struct IBSS_CONVERT_STRUCT {
	MUINT8 *pu1Y;
	MUINT8 *pu1U;
	MUINT8 *pu1V;
	MUINT8 *pu1Out;
	MINT32 u4Width;
	MINT32 u4Height;
	MINT32 u4Core;

	// constructor
	IBSS_CONVERT_STRUCT()
	{
		::memset(this, 0x0, sizeof(struct IBSS_CONVERT_STRUCT));
	}
};

struct IBSS_CONFIG_ZIP_OUT_STRUCT {
	MUINT32 u4ZipBufferSize; // [return value] zip out buffer require size

	// constructor
	IBSS_CONFIG_ZIP_OUT_STRUCT()
	{
		::memset(this, 0x0, sizeof(struct IBSS_CONFIG_ZIP_OUT_STRUCT));
	}
};

struct IBSS_INPUT_DATA_G {
	MUINT32 Bitnum;
	MUINT32 BayerOrder;
	MUINT32 Stride;

	MUINT32 inWidth;
	MUINT32 inHeight;
	MUINT32 fdWidth;
	MUINT32 fdHeight;

	IBSS_PROC_TYPE eType;
	MBOOL InputZip;

#ifdef D_BSS_FORCE_BEST_IDX
	MUINT32 i4BSFEN; // force enable
	MUINT32 i4BSFBI; // force best index
#endif

	MUINT8 *apbyBssInImg[MAX_FRAME_NUM];

	IGmv gmv[MAX_FRAME_NUM];
	IBssFaceMetadata *Face[MAX_FRAME_NUM];
	MUINT32 u4AGain[MAX_FRAME_NUM];
	MUINT32 u4DGain[MAX_FRAME_NUM];
	MUINT32 u4ExpT[MAX_FRAME_NUM];
	MUINT16 *u2Lcso[MAX_FRAME_NUM];
	MUINT32 *act_hist_buf[MAX_FRAME_NUM];

	IBSS_GYRO_INFO *prGyroInfo;
	MUINT32 u4GyroNum;
	MUINT32 u4GyroIntervalMS;
	MINT32 bss_iso;

	// constructor
	IBSS_INPUT_DATA_G() { ::memset(this, 0x0, sizeof(struct IBSS_INPUT_DATA_G)); }
};

struct IBSS_INPUT_DATA_G_IPC : public IBSS_INPUT_DATA_G {
	void *apbyBssInImgBufPtr[MAX_FRAME_NUM];
	// constructor
	IBSS_INPUT_DATA_G_IPC()
	{
		::memset(this, 0x0, sizeof(struct IBSS_INPUT_DATA_G_IPC));
	}
};

struct IBSS_OUTPUT_DATA {
	MUINT32 originalOrder[MAX_FRAME_NUM];
	IGmv gmv[MAX_FRAME_NUM];
	long long SharpScore[MAX_FRAME_NUM];
	long long adj1_score[MAX_FRAME_NUM];
	long long adj2_score[MAX_FRAME_NUM];
	long long adj3_score[MAX_FRAME_NUM];
	long long final_score[MAX_FRAME_NUM];
	long long ACTSScore[MAX_FRAME_NUM];
	long long BlendingScore[MAX_FRAME_NUM];
	MINT32 AvgPxLvl[MAX_FRAME_NUM];
	MUINT32 i4SkipFrmCnt;
	MUINT32 u4DGain[MAX_FRAME_NUM];
	MUINT16 *u2Lcso[MAX_FRAME_NUM];

	// constructor
	IBSS_OUTPUT_DATA() { ::memset(this, 0x0, sizeof(struct IBSS_OUTPUT_DATA)); }
};

struct MTKFDContainerInfo {
	MtkCameraFaceMetadata facedata;
	MINT32 sensorId = -1;
	MTKFDContainerInfo() { memset(&facedata, 0, sizeof(MtkCameraFaceMetadata)); }
	~MTKFDContainerInfo() {}

	void clone(MTKFDContainerInfo *cloneInfo)
	{
		if (cloneInfo)
			*cloneInfo = *this;
	}
};
#define FD_DATATYPE MTKFDContainerInfo

struct MTKBSSFDInfo {
	IBssFaceMetadata facedata;
	IBssFace faces[15];
	IBssFaceInfo posInfo[15];
	MTKBSSFDInfo()
		: facedata{}, faces{}, posInfo{} {}
	~MTKBSSFDInfo() {}
};

struct IMFBLL_SET_PROC_INFO_STRUCT {
	MUINT8 *workbuf_addr;
	MUINT32 buf_size;
	MUINT8 *Proc1_base;
	MUINT8 *Proc1_ref;
	MUINT32 Proc1_width;
	MUINT32 Proc1_height;
	MUINT32 Proc1_ME_top_mode;
	MUINT32 Proc1_ME_force_mode;
	//0x30
	MUINT32 Proc1_HG_info_en;

	MUINT32 Proc1_me_wpe_image_width;
	MUINT32 Proc1_me_wpe_image_height;
	MUINT32 Proc1_me_wpe_np1_mode;
	//0x40
	MUINT32 Proc1_me_wpe_stride;
	MUINT32 Proc1_FD_image_width;
	MUINT32 Proc1_FD_image_height;

	MUINT32 Proc1_Bst_FDROI_X0;
	//0x50
	MUINT32 Proc1_Bst_FDROI_Y0;
	MUINT32 Proc1_Bst_FDROI_X1;
	MUINT32 Proc1_Bst_FDROI_Y1;
	MUINT32 Proc1_Ref_FDROI_X0;
	//0x60
	MUINT32 Proc1_Ref_FDROI_Y0;
	MUINT32 Proc1_Ref_FDROI_X1;
	MUINT32 Proc1_Ref_FDROI_Y1;

	MUINT32 Proc1_Bst_LEYE_X0;
	//0x70
	MUINT32 Proc1_Bst_LEYE_X1;
	MUINT32 Proc1_Bst_LEYE_Y0;
	MUINT32 Proc1_Bst_LEYE_Y1;
	MUINT32 Proc1_Bst_LEYE_UX;
	//0x80
	MUINT32 Proc1_Bst_LEYE_UY;
	MUINT32 Proc1_Bst_LEYE_DX;
	MUINT32 Proc1_Bst_LEYE_DY;
	MUINT32 Proc1_Bst_REYE_X0;
	//0x90
	MUINT32 Proc1_Bst_REYE_X1;
	MUINT32 Proc1_Bst_REYE_Y0;
	MUINT32 Proc1_Bst_REYE_Y1;
	MUINT32 Proc1_Bst_REYE_UX;
	//0xA0
	MUINT32 Proc1_Bst_REYE_UY;
	MUINT32 Proc1_Bst_REYE_DX;
	MUINT32 Proc1_Bst_REYE_DY;

	MUINT32 Proc1_Ref_LEYE_X0;
	//0xB0
	MUINT32 Proc1_Ref_LEYE_X1;
	MUINT32 Proc1_Ref_LEYE_Y0;
	MUINT32 Proc1_Ref_LEYE_Y1;
	MUINT32 Proc1_Ref_LEYE_UX;
	//0xC0
	MUINT32 Proc1_Ref_LEYE_UY;
	MUINT32 Proc1_Ref_LEYE_DX;
	MUINT32 Proc1_Ref_LEYE_DY;
	MUINT32 Proc1_Ref_REYE_X0;
	//0xD0
	MUINT32 Proc1_Ref_REYE_X1;
	MUINT32 Proc1_Ref_REYE_Y0;
	MUINT32 Proc1_Ref_REYE_Y1;
	MUINT32 Proc1_Ref_REYE_UX;
	//0xE0
	MUINT32 Proc1_Ref_REYE_UY;
	MUINT32 Proc1_Ref_REYE_DX;
	MUINT32 Proc1_Ref_REYE_DY;

	MUINT32 Proc1_base_ae_dgn_gain;
	//0xF0
	MUINT32 Proc1_base_ae_exposure_time;
	MUINT32 Proc1_base_ae_isp_gain;
	MUINT32 Proc1_base_ae_sensor_gain;
	MUINT32 Proc1_ref_ae_dgn_gain;
	//0x100
	MUINT32 Proc1_ref_ae_exposure_time;
	MUINT32 Proc1_ref_ae_isp_gain;
	MUINT32 Proc1_ref_ae_sensor_gain;

	IPROC_IMAGE_FORMAT Proc1_ImgFmt;
	//0x110
	MUINT32 iBssOrgScore_base;
	MUINT32 iBssOrgScore_ref;

	MINT8 Proc_idx; //debug

	void *pSWMENvram;

	IPass_HG_Info HGInfo;
	IPass_GYRO_MV_INFO GyroMVInfo;

#ifdef FOR_SIM

	MINT32 i4ForceMapMode;
	MINT32 i4ForceMvMode;
	MINT32 i4ForceMvXVal;
	MINT32 i4ForceMvYVal;
	MINT32 i4ForceMvBkXSt;
	MINT32 i4ForceMvBkXEd;
	MINT32 i4ForceMvBkYSt;
	MINT32 i4ForceMvBkYEd;

	MINT32 i4MVPTestModeA;
	MINT32 i4MVPTestModeB;
	MINT32 i4MVPTestModeC;
	MINT32 i4MVPTestModeD;
	MINT32 i4MVPTestModeE;

	MINT32 i4MEBlkLogPosEn;
	MINT32 i4MEBlkLogPosPx;
	MINT32 i4MEBlkLogPosPy;
	MINT32 i4MEBlkLogPosLR;
	MINT32 i4MEBlkLogPosUD;

#endif

	// constructor
	IMFBLL_SET_PROC_INFO_STRUCT()
	{
		memset(this, 0x0, sizeof(struct IMFBLL_SET_PROC_INFO_STRUCT));
	}
};

typedef struct
{
	MUINT32 Ext_mem_size;
	MUINT32 CofMap_width;
	MUINT32 CofMap_height;
	MUINT32 MV_width;
	MUINT32 MV_height;
	MUINT32 WpeMap_width;
	MUINT32 WpeMap_height;

} IMFBLL_GET_PROC_INFO_STRUCT;

struct IMFBLL_VerInfo {
	char rMainVer[5];
	char rSubVer[5];
	char rPatchVer[5];
};

typedef struct IMFBLL_PROC1_OUT_STRUCT {
	MUINT8 *pu1ConfMap;
	MUINT32 u4MapSize;
	MUINT8 *pu1MV;
	MINT32 *pi4WpeMapX;
	MINT32 *pi4WpeMapY;
	MUINT32 u4WpeMapSize;
	MUINT32 u4MVSize;
} IMFBLL_PROC1_OUT_STRUCT;

struct IMFBLL_INIT_PARAM_STRUCT {
	MUINT16 Proc1_imgW;
	MUINT16 Proc1_imgH;
	MUINT32 core_num;
	MBOOL Proc1_DSUS_mode; // 0: 1/4, 1: 1/2
#ifdef FOR_SIM
	char achDmpPath[512];
#endif

	// constructor
	IMFBLL_INIT_PARAM_STRUCT()
		: core_num(4)
	{
	}
};

struct IMFBLL_SET_PROC_INFO_STRUCT_IPC : public IMFBLL_SET_PROC_INFO_STRUCT {
	void *workbuf_ptr;
	void *Proc1_base_ptr;
	void *Proc1_ref_ptr;

	// constructor
	IMFBLL_SET_PROC_INFO_STRUCT_IPC()
	{
		memset(this, 0x0, sizeof(struct IMFBLL_SET_PROC_INFO_STRUCT_IPC));
	}
};

typedef struct IMFBLL_PROC1_OUT_STRUCT_IPC : IMFBLL_PROC1_OUT_STRUCT {
	void *pConfMapPtr;
	void *pMVPtr;
	void *pWpeMapPtr;
} IMFBLL_PROC1_OUT_STRUCT_IPC, *P_IMFBLL_PROC1_OUT_STRUCT_IPC;

} /* namespace libcamera */

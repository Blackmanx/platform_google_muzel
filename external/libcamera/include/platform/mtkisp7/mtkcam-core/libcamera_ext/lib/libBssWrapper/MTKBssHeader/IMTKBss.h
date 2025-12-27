/*
 * Copyright (C) 2022 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _IMTK_BSS_H
#define _IMTK_BSS_H

#include "EBss.h"
#include "EMTKBss.h"
#include "IBssErrCode.h"
#include "IBssFace.h"
#include "IBssType.h"
#define MAX_FRAME_NUM 10
//#define MAX_FACE_NUM 5

struct IBSS_GYRO_INFO {
	MFLOAT fX;
	MFLOAT fY;
	MFLOAT fZ;

	// constructor
	IBSS_GYRO_INFO()
		: fX(0), fY(0), fZ(0)
	{
	}
};

struct IGmv {
	MINT32 x;
	MINT32 y;

	// constructor
	IGmv()
		: x(0), y(0)
	{
	}
};

struct IPass_BSS_WB_STRUCT {
	IPASS_BSS_PROC_ENUM rProcId; // [User SET]process mode
	MUINT32 u4Width; // [User SET] frame width
	MUINT32 u4Height; // [User SET] frame height
	MUINT32 u4FrameNum; // [User SET] frame number
	MUINT32 u4WKSize; // [return value] working buffer require size
	MUINT8 *pu1BW; // [User SET] frame number

	// constructor
	IPass_BSS_WB_STRUCT()
	{
		::memset(this, 0x0, sizeof(struct IPass_BSS_WB_STRUCT));
	}
};

struct IPass_BSS_PARAM_STRUCT {
	MUINT8 BSS_ON;
#if 0
  MUINT8  BSS_VER;
#endif
	MUINT32 BSS_ROI_WIDTH;
	MUINT32 BSS_ROI_HEIGHT;
	MUINT32 BSS_ROI_X0;
	MUINT32 BSS_ROI_Y0;
#if 0
  MUINT32 BSS_SCALE_FACTOR;
#endif
	void *pBSSNvram;

#if 0
  MUINT32 BSS_CLIP_TH0;
  MUINT32 BSS_CLIP_TH1;
  MUINT32 BSS_CLIP_TH2;
  MUINT32 BSS_CLIP_TH3;
  MUINT32 BSS_ZERO;
#endif
	MUINT32 BSS_FRAME_NUM;
#if 0
  MUINT32 BSS_ADF_TH;
  MUINT32 BSS_SDF_TH;
#endif
	MUINT32 BSS_GAIN_TH0;
	MUINT32 BSS_GAIN_TH1;
	MUINT32 BSS_MIN_ISP_GAIN;
	MUINT32 BSS_LCSO_SIZE;
#if 0
  MUINT8  BSS_YPF_EN;
  MUINT8  BSS_YPF_FAC;
  MUINT16 BSS_YPF_ADJTH;
  MUINT16 BSS_YPF_DFMED0;
  MUINT16 BSS_YPF_DFMED1;
  MUINT8  BSS_YPF_TH0;
  MUINT8  BSS_YPF_TH1;
  MUINT8  BSS_YPF_TH2;
  MUINT8  BSS_YPF_TH3;
  MUINT8  BSS_YPF_TH4;
  MUINT8  BSS_YPF_TH5;
  MUINT8  BSS_YPF_TH6;
  MUINT8  BSS_YPF_TH7;

  MUINT8  BSS_FD_EN;
  MUINT8  BSS_FD_FAC;
  MUINT8  BSS_FD_FNUM;
#endif
	MUINT8 BSS_FD_TH0;
	MUINT8 BSS_FD_TH1;
#if 0
  MUINT8  BSS_EYE_EN;
  MUINT8  BSS_EYE_CFTH;
  MUINT8  BSS_EYE_RATIO0;
  MUINT8  BSS_EYE_RATIO1;
  MUINT8  BSS_EYE_FAC;
#endif
	MUINT8 BSS_AEVC_EN;
	MUINT16 BSS_AEVC_DCNT;

#if 0
  // BSS 3.0
  MUINT8  BSS_FaceCVTh;       // range: 0 ~ 100
  MUINT8  BSS_GradThL;        // range: 0 ~ 255
  MUINT8  BSS_GradThH;        // range: 0 ~ 255
  MUINT16 BSS_FaceAreaThL[2]; // range: 0 ~ 65535
  MUINT16 BSS_FaceAreaThH[2]; // range: 0 ~ 65535
  MUINT16 BSS_APLDeltaTh[33]; // range: 0 ~ 4095
  MUINT16 BSS_GradRatioTh[8]; // range: 0 ~ 10000
  MUINT8  BSS_EyeDistThL;     // range: 0 ~ 255
  MUINT8  BSS_EyeDistThH;     // range: 0 ~ 255
  MUINT16 BSS_EyeMinWeight;   // range: 0 ~ 10000

  // BSS 3.5
  MUINT32 BSS_ACTS_CLIP_TH0;
  MUINT32 BSS_ACTS_CLIP_TH1;
  MINT32 BSS_ISO_ThH;
  MINT32 BSS_ISO_ThL;
  MUINT16 BSS_Sharp_min_weight;

  //AI shutter mode
  MUINT8  ai_shutter_mode_en;
  MUINT16  ai_shutter_weight[10];
#endif

	// constructor
	IPass_BSS_PARAM_STRUCT()
		: BSS_ON(1),
#if 0
      BSS_VER(3),
      BSS_SCALE_FACTOR(8),
      BSS_CLIP_TH0(10), BSS_CLIP_TH1(20), BSS_CLIP_TH2(10), BSS_CLIP_TH3(20),
      BSS_ZERO(12),
#endif
		  BSS_ROI_WIDTH(0),
		  BSS_ROI_HEIGHT(0),
		  BSS_ROI_X0(0),
		  BSS_ROI_Y0(0),
		  pBSSNvram(0),
		  BSS_FRAME_NUM(4),
#if 0
      BSS_ADF_TH(18), BSS_SDF_TH(80),
#endif
		  BSS_GAIN_TH0(853), BSS_GAIN_TH1(1229),
		  BSS_MIN_ISP_GAIN(546), BSS_LCSO_SIZE(147456),

#if 0
      BSS_YPF_EN(1), BSS_YPF_FAC(50), BSS_YPF_ADJTH(12),
      BSS_YPF_DFMED0(20), BSS_YPF_DFMED1(32),
      BSS_YPF_TH0(102), BSS_YPF_TH1(104), BSS_YPF_TH2(98), BSS_YPF_TH3(96),
      BSS_YPF_TH4(96), BSS_YPF_TH5(96), BSS_YPF_TH6(96), BSS_YPF_TH7(96),
#endif

#if 0
      BSS_FD_EN(0), BSS_FD_FAC(2), BSS_FD_FNUM(1),
#endif
		  BSS_FD_TH0(5), BSS_FD_TH1(40),
#if 0
      BSS_EYE_EN(0), BSS_EYE_CFTH(60), BSS_EYE_RATIO0(75), BSS_EYE_RATIO1(55), BSS_EYE_FAC(50),
#endif
		  BSS_AEVC_EN(0), BSS_AEVC_DCNT(512)

#if 0
      BSS_FaceCVTh(50), BSS_GradThL(4), BSS_GradThH(58),
      BSS_EyeDistThL(7), BSS_EyeDistThH(11), BSS_EyeMinWeight(9700),
      BSS_ACTS_CLIP_TH0(10), BSS_ACTS_CLIP_TH1(30), BSS_ISO_ThH(0),
      BSS_ISO_ThL(0), BSS_Sharp_min_weight(0), ai_shutter_mode_en(0)
#endif
	{
#if 0
      BSS_FaceAreaThL[0] = 256;
      BSS_FaceAreaThL[1] = 256;
      BSS_FaceAreaThH[0] = 16384;
      BSS_FaceAreaThH[1] = 32768;
      BSS_APLDeltaTh[0]  = 70;  // 0
      BSS_APLDeltaTh[1]  = 75;  // 128
      BSS_APLDeltaTh[2]  = 80;  // 256
      BSS_APLDeltaTh[3]  = 85;  // 384
      BSS_APLDeltaTh[4]  = 90;  // 512
      BSS_APLDeltaTh[5]  = 95;  // 640
      BSS_APLDeltaTh[6]  = 100; // 768
      BSS_APLDeltaTh[7]  = 105; // 896
      BSS_APLDeltaTh[8]  = 110; // 1024
      BSS_APLDeltaTh[9]  = 120; // 1152
      BSS_APLDeltaTh[10] = 130; // 1280
      BSS_APLDeltaTh[11] = 140; // 1408
      BSS_APLDeltaTh[12] = 150; // 1536
      BSS_APLDeltaTh[13] = 160; // 1664
      BSS_APLDeltaTh[14] = 170; // 1792
      BSS_APLDeltaTh[15] = 180; // 1920
      BSS_APLDeltaTh[16] = 190; // 2048
      BSS_APLDeltaTh[17] = 200; // 2176
      BSS_APLDeltaTh[18] = 200; // 2304
      BSS_APLDeltaTh[19] = 200; // 2432
      BSS_APLDeltaTh[20] = 200; // 2560
      BSS_APLDeltaTh[21] = 200; // 2688
      BSS_APLDeltaTh[22] = 200; // 2816
      BSS_APLDeltaTh[23] = 200; // 2944
      BSS_APLDeltaTh[24] = 200; // 3072
      BSS_APLDeltaTh[25] = 200; // 3200
      BSS_APLDeltaTh[26] = 200; // 3328
      BSS_APLDeltaTh[27] = 200; // 3456
      BSS_APLDeltaTh[28] = 200; // 3584
      BSS_APLDeltaTh[29] = 200; // 3712
      BSS_APLDeltaTh[30] = 200; // 3840
      BSS_APLDeltaTh[31] = 200; // 3968
      BSS_APLDeltaTh[32] = 200; // 4095
      BSS_GradRatioTh[0] = 1500;
      BSS_GradRatioTh[1] = 2000;
      BSS_GradRatioTh[2] = 2500;
      BSS_GradRatioTh[3] = 3000;
      BSS_GradRatioTh[4] = 4000;
      BSS_GradRatioTh[5] = 6000;
      BSS_GradRatioTh[6] = 8000;
      BSS_GradRatioTh[7] = 10000;
      ai_shutter_weight[0] = 10000;
      ai_shutter_weight[1] = 10000;
      ai_shutter_weight[2] = 10000;
      ai_shutter_weight[3] = 10000;
      ai_shutter_weight[4] = 10000;
      ai_shutter_weight[5] = 10000;
      ai_shutter_weight[6] = 10000;
      ai_shutter_weight[7] = 10000;
#endif
	}
};

struct IPass_BSS_CONFIG_ZIP_IN_STRUCT {
	MUINT32 u4Width; // [User SET] frame width
	MUINT32 u4Height; // [User SET] frame height
	IPASS_BSS_PROC_TYPE eType;
	MUINT32 u4ROI_X0;
	MUINT32 u4ROI_Y0;
	MUINT32 u4ROI_WIDTH;
	MUINT32 u4ROI_HEIGHT;
	void *pBSSNvram;

	// constructor
	IPass_BSS_CONFIG_ZIP_IN_STRUCT()
	{
		::memset(this, 0x0, sizeof(struct IPass_BSS_CONFIG_ZIP_IN_STRUCT));
	}
};

struct IPass_BSS_PROC_ZIP_STRUCT {
	MUINT8 *pu8InImg;
	MUINT32 u4ImgInStride;
	MUINT32 u4ImgInSize;
	MUINT32 u4ImgInBayerOrder; // for raw input
	MUINT8 *pu8OutImg;
	MUINT32 u4ImgOutSize;

	MINT32 i4GmvX;
	MINT32 i4GmvY;

	// constructor
	IPass_BSS_PROC_ZIP_STRUCT()
	{
		::memset(this, 0x0, sizeof(struct IPass_BSS_PROC_ZIP_STRUCT));
	}
};

struct IPass_BSS_PROC_ZIP_STRUCT_IPC : public IPass_BSS_PROC_ZIP_STRUCT {
	void *pInImgBufPtr;
	void *pOutImgBufPtr;

	// constructor
	IPass_BSS_PROC_ZIP_STRUCT_IPC()
	{
		::memset(this, 0x0, sizeof(struct IPass_BSS_PROC_ZIP_STRUCT_IPC));
	}
};

struct IPass_BSS_CONVERT_STRUCT {
	MUINT8 *pu1Y;
	MUINT8 *pu1U;
	MUINT8 *pu1V;
	MUINT8 *pu1Out;
	MINT32 u4Width;
	MINT32 u4Height;
	MINT32 u4Core;

	// constructor
	IPass_BSS_CONVERT_STRUCT()
	{
		::memset(this, 0x0, sizeof(struct IPass_BSS_CONVERT_STRUCT));
	}
};

struct IPass_BSS_CONFIG_ZIP_OUT_STRUCT {
	MUINT32 u4ZipBufferSize; // [return value] zip out buffer require size

	// constructor
	IPass_BSS_CONFIG_ZIP_OUT_STRUCT()
	{
		::memset(this, 0x0, sizeof(struct IPass_BSS_CONFIG_ZIP_OUT_STRUCT));
	}
};

struct IPass_BSS_OUTPUT_DATA {
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
	IPass_BSS_OUTPUT_DATA()
	{
		::memset(this, 0x0, sizeof(struct IPass_BSS_OUTPUT_DATA));
	}
};

struct IPass_BSS_INPUT_DATA_G {
	MUINT32 Bitnum;
	MUINT32 BayerOrder;
	MUINT32 Stride;

	MUINT32 inWidth;
	MUINT32 inHeight;
	MUINT32 fdWidth;
	MUINT32 fdHeight;

	IPASS_BSS_PROC_TYPE eType;
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
	IPass_BSS_INPUT_DATA_G()
	{
		::memset(this, 0x0, sizeof(IPass_BSS_INPUT_DATA_G));
	}
};

struct IPass_BSS_INPUT_DATA_G_IPC : public IPass_BSS_INPUT_DATA_G {
	void *apbyBssInImgBufPtr[MAX_FRAME_NUM];

	// constructor
	IPass_BSS_INPUT_DATA_G_IPC()
	{
		::memset(this, 0x0, sizeof(IPass_BSS_INPUT_DATA_G_IPC));
	}
};

struct IPASS_BSS_VerInfo {
	char rMainVer[5];
	char rSubVer[5];
	char rPatchVer[5];

	// constructor
	IPASS_BSS_VerInfo()
	{
		::memset(this, 0x0, sizeof(struct IPASS_BSS_VerInfo));
	}
}; // get Version by FeatureCtrl(GET_VERSION)

#endif

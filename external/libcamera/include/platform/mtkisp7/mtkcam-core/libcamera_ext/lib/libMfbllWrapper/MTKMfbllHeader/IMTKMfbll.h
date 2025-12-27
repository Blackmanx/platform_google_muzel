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

#ifndef _IMTK_MFBLL_H
#define _IMTK_MFBLL_H


#define PROC1_DN_RATIO 4
#define MV2WPE_EN

#include "include/IMTKMfbllType.h"
#include "IMTKMfbllErrCode.h"
#include "EMTKMfbll.h"
#include "include/EMfbll.h"

/*****************************************************************************
  MFBLL INIT
******************************************************************************/
struct IPass_MFBLL_INIT_PARAM_STRUCT
{
    MUINT16 Proc1_imgW;
    MUINT16 Proc1_imgH;
    MUINT32 core_num;
    MBOOL Proc1_DSUS_mode;   // 0: 1/4, 1: 1/2
#ifdef FOR_SIM
    char achDmpPath[512];
#endif

    // constructor
    IPass_MFBLL_INIT_PARAM_STRUCT()
    : Proc1_imgW(0), Proc1_imgH(0), core_num(4), Proc1_DSUS_mode(0)
    {}
};

// homographic information
struct IPass_HG_Info
{
    MINT32 i4ImageWidth;	// image width/height which homographic calculated on
    MINT32 i4ImageHeight;

    MINT32 i4M1VLD;	// homographic model 1 valid (for background area)
    MINT32 i4M2VLD;	// homographic model 2 valid (for face area)

    MINT32 i4M1FLAG;	// homographic model 1 valid (for background area)
    MINT32 i4M2FLAG;	// homographic model 2 valid (for face area)

    MINT32 i4M1X0; // model 1 window, left/top X poisition
    MINT32 i4M1Y0; // model 1 window, left/top Y poisition
    MINT32 i4M1X1; // model 1 window, right/down X poisition
    MINT32 i4M1Y1; // model 1 window, right/down Y poisition

    MINT32 i4M2X0; // model 2 window, left/top X poisition
    MINT32 i4M2Y0; // model 2 window, left/top Y poisition
    MINT32 i4M2X1; // model 2 window, right/down X poisition
    MINT32 i4M2Y1; // model 2 window, right/down Y poisition

    MINT32 i4M1Conf;  // model 1 confidence level (for background area)
    MINT32 i4M1Coef0; // model 1 coefficient 0 (10 bit signed interger, 22 bit fractional, range -511~+511)
    MINT32 i4M1Coef1;
    MINT32 i4M1Coef2;
    MINT32 i4M1Coef3;
    MINT32 i4M1Coef4;
    MINT32 i4M1Coef5;
    MINT32 i4M1Coef6;
    MINT32 i4M1Coef7;

    MINT32 i4M2Conf;  // model 2 confidence level (for face area)
    MINT32 i4M2Coef0; // model 2 coefficient 0
    MINT32 i4M2Coef1;
    MINT32 i4M2Coef2;
    MINT32 i4M2Coef3;
    MINT32 i4M2Coef4;
    MINT32 i4M2Coef5;
    MINT32 i4M2Coef6;
    MINT32 i4M2Coef7;

};

typedef struct IPass_MFBLL_PROC1_OUT_STRUCT
{
    MUINT8 *pu1ConfMap;
    MUINT32 u4MapSize;
    MUINT8 *pu1MV;
    MINT32 *pi4WpeMapX;
    MINT32 *pi4WpeMapY;
    MUINT32 u4WpeMapSize;
    MUINT32 u4MVSize;

    // constructor
    IPass_MFBLL_PROC1_OUT_STRUCT()
    {
        memset(this, 0x0, sizeof(struct IPass_MFBLL_PROC1_OUT_STRUCT));
    }
    
} IPass_MFBLL_PROC1_OUT_STRUCT, *P_IPASS_MFBLL_PROC1_OUT_STRUCT;

typedef struct IPass_MFBLL_PROC1_OUT_STRUCT_IPC : public IPass_MFBLL_PROC1_OUT_STRUCT
{
    void   *pConfMapPtr;
    void   *pMVPtr;
    void   *pWpeMapPtr;

    // constructor
    IPass_MFBLL_PROC1_OUT_STRUCT_IPC()
    {
        memset(this, 0x0, sizeof(struct IPass_MFBLL_PROC1_OUT_STRUCT_IPC));
    }
} IPass_MFBLL_PROC1_OUT_STRUCT_IPC, *P_IPass_MFBLL_PROC1_OUT_STRUCT_IPC;

typedef struct IPass_MFBLL_GET_PROC_INFO_STRUCT
{
    MUINT32 Ext_mem_size;
    MUINT32 CofMap_width;
    MUINT32 CofMap_height;
    MUINT32 MV_width;
    MUINT32 MV_height;
    MUINT32 WpeMap_width;
    MUINT32 WpeMap_height;

    // constructor
    IPass_MFBLL_GET_PROC_INFO_STRUCT()
    {
        memset(this, 0x0, sizeof(struct IPass_MFBLL_GET_PROC_INFO_STRUCT));
    }

} IPass_MFBLL_GET_PROC_INFO_STRUCT, *P_IPass_MFBLL_GET_PROC_INFO_STRUCT;

struct IPass_GYRO_MV_INFO
{
    MUINT8 *mv;   //MAX_MV_NUM * 4 (TBD:double buffer?)
    MINT8 base_idx;
    MINT8 ref_idx;
    MINT8 mv_height;
    MINT8 mv_width;
    MINT8 frame_num;
};

struct IPass_MFBLL_SET_PROC_INFO_STRUCT
{
    MUINT8  *workbuf_addr;
    MUINT32  buf_size;
    MUINT8  *Proc1_base;
    MUINT8  *Proc1_ref;
    MUINT32  Proc1_width;
    MUINT32  Proc1_height;
    MUINT32  Proc1_ME_top_mode;
    MUINT32  Proc1_ME_force_mode;
    MUINT32  Proc1_HG_info_en;

    MUINT32  Proc1_me_wpe_image_width;
    MUINT32  Proc1_me_wpe_image_height;
    MUINT32  Proc1_me_wpe_np1_mode;
    MUINT32  Proc1_me_wpe_stride;
    MUINT32  Proc1_FD_image_width;
    MUINT32  Proc1_FD_image_height;

    MUINT32  Proc1_Bst_FDROI_X0;
    MUINT32  Proc1_Bst_FDROI_Y0;
    MUINT32  Proc1_Bst_FDROI_X1;
    MUINT32  Proc1_Bst_FDROI_Y1;
    MUINT32  Proc1_Ref_FDROI_X0;
    MUINT32  Proc1_Ref_FDROI_Y0;
    MUINT32  Proc1_Ref_FDROI_X1;
    MUINT32  Proc1_Ref_FDROI_Y1;

    MUINT32  Proc1_Bst_LEYE_X0;
    MUINT32  Proc1_Bst_LEYE_X1;
    MUINT32  Proc1_Bst_LEYE_Y0;
    MUINT32  Proc1_Bst_LEYE_Y1;
    MUINT32  Proc1_Bst_LEYE_UX;
    MUINT32  Proc1_Bst_LEYE_UY;
    MUINT32  Proc1_Bst_LEYE_DX;
    MUINT32  Proc1_Bst_LEYE_DY;
    MUINT32  Proc1_Bst_REYE_X0;
    MUINT32  Proc1_Bst_REYE_X1;
    MUINT32  Proc1_Bst_REYE_Y0;
    MUINT32  Proc1_Bst_REYE_Y1;
    MUINT32  Proc1_Bst_REYE_UX;
    MUINT32  Proc1_Bst_REYE_UY;
    MUINT32  Proc1_Bst_REYE_DX;
    MUINT32  Proc1_Bst_REYE_DY;

    MUINT32  Proc1_Ref_LEYE_X0;
    MUINT32  Proc1_Ref_LEYE_X1;
    MUINT32  Proc1_Ref_LEYE_Y0;
    MUINT32  Proc1_Ref_LEYE_Y1;
    MUINT32  Proc1_Ref_LEYE_UX;
    MUINT32  Proc1_Ref_LEYE_UY;
    MUINT32  Proc1_Ref_LEYE_DX;
    MUINT32  Proc1_Ref_LEYE_DY;
    MUINT32  Proc1_Ref_REYE_X0;
    MUINT32  Proc1_Ref_REYE_X1;
    MUINT32  Proc1_Ref_REYE_Y0;
    MUINT32  Proc1_Ref_REYE_Y1;
    MUINT32  Proc1_Ref_REYE_UX;
    MUINT32  Proc1_Ref_REYE_UY;
    MUINT32  Proc1_Ref_REYE_DX;
    MUINT32  Proc1_Ref_REYE_DY;

    MUINT32  Proc1_base_ae_dgn_gain;
    MUINT32  Proc1_base_ae_exposure_time;
    MUINT32  Proc1_base_ae_isp_gain;
    MUINT32  Proc1_base_ae_sensor_gain;
    MUINT32  Proc1_ref_ae_dgn_gain;
    MUINT32  Proc1_ref_ae_exposure_time;
    MUINT32  Proc1_ref_ae_isp_gain;
    MUINT32  Proc1_ref_ae_sensor_gain;

    IPASS_PROC_IMAGE_FORMAT Proc1_ImgFmt;

    MUINT32 iBssOrgScore_base;
    MUINT32 iBssOrgScore_ref;

    MINT8   Proc_idx;  //debug

    void* pSWMENvram;

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
    IPass_MFBLL_SET_PROC_INFO_STRUCT()
    {
        memset(this, 0x0, sizeof(struct IPass_MFBLL_SET_PROC_INFO_STRUCT));
    }
};
typedef IPass_MFBLL_SET_PROC_INFO_STRUCT *P_IPass_MFBLL_SET_PROC_INFO_STRUCT;

struct IPass_MFBLL_SET_PROC_INFO_STRUCT_IPC : public IPass_MFBLL_SET_PROC_INFO_STRUCT
{
    void    *workbuf_ptr;
    void    *Proc1_base_ptr;
    void    *Proc1_ref_ptr;
    // constructor
    IPass_MFBLL_SET_PROC_INFO_STRUCT_IPC()
    {
        memset(this, 0x0, sizeof(struct IPass_MFBLL_SET_PROC_INFO_STRUCT_IPC));
    }
};
typedef IPass_MFBLL_SET_PROC_INFO_STRUCT_IPC *P_IPass_MFBLL_SET_PROC_INFO_STRUCT_IPC;


struct IPASS_MFBLL_VerInfo
{
    char rMainVer[5];
    char rSubVer[5];
    char rPatchVer[5];
} ;


#endif
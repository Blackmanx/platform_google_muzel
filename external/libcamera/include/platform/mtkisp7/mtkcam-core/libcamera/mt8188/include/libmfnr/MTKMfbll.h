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



#ifndef _MTK_MFBLL_H
#define _MTK_MFBLL_H

#include <string.h>

#include "MTKMfbllType.h"
#include "MTKMfbllErrCode.h"

//#define FOR_SIM
//#define KEEP_PROC2
#define PROC1_DN_RATIO 4
#define MV2WPE_EN

//#define CONF_PADDING

typedef enum DRVMfbllObject_s
{
    DRV_MFBLL_OBJ_NONE = 0,
    DRV_MFBLL_OBJ_SW,
    DRV_MFBLL_OBJ_UNKNOWN = 0xFF,
} DrvMfbllObject_e;

/*****************************************************************************
  Main Module
******************************************************************************/
typedef enum MFBLL_PROC_ENUM
{
    MFBLL_PROC1 = 0,
#ifdef KEEP_PROC2
    MFBLL_PROC2,
#endif
    MFBLL_UNKNOWN_PROC,
} MFBLL_PROC_ENUM;

typedef enum MFBLL_FTCTRL_ENUM
{
    MFBLL_FTCTRL_GET_PROC_INFO,
    MFBLL_FTCTRL_SET_PROC_INFO,
    MFBLL_FTCTRL_GET_VERSION,        // feature id to get Version
    MFBLL_FTCTRL_CAL_GYRO_MV,
    MFBLL_FTCTRL_MAX
} MFBLL_FTCTRL_ENUM;

/*****************************************************************************
  MFBLL INIT
******************************************************************************/
struct MFBLL_INIT_PARAM_STRUCT
{
    MUINT16 Proc1_imgW;
    MUINT16 Proc1_imgH;
    MUINT32 core_num;
    MBOOL Proc1_DSUS_mode;   // 0: 1/4, 1: 1/2
#ifdef FOR_SIM
    char achDmpPath[512];
#endif

    // constructor
    MFBLL_INIT_PARAM_STRUCT()
    : core_num(4)
    {}
};

/*****************************************************************************
  PROC1 PART (Type definition should be exactly same as that in core Proc1)
******************************************************************************/

// homographic information
struct HG_Info
{
    MINT32 i4ImageWidth;// image width/height which homographic calculated on
    MINT32 i4ImageHeight;

    MINT32 i4M1VLD; // homographic model 1 valid (for background area)
    MINT32 i4M2VLD; // homographic model 2 valid (for face area)

    MINT32 i4M1FLAG;// homographic model 1 valid (for background area)
    MINT32 i4M2FLAG;// homographic model 2 valid (for face area)

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
struct GYRO_MV_INFO
{
    MUINT8 *mv;   //MAX_MV_NUM * 4 (TBD:double buffer?)
    MINT8 base_idx;
    MINT8 ref_idx;
    MINT8 mv_height;
    MINT8 mv_width;
    MINT8 frame_num;
};

typedef enum PROC_IMAGE_FORMAT
{
    PROC1_FMT_YV16 = 0, // 422 : 3 plane , Y..U..V..
    PROC1_FMT_YUY2, // 422 : YUV YUV ...
    PROC1_FMT_NV12, // 420 : 2 plane , Y... UVUV..
    PROC1_FMT_Y,
    PROC1_FMT_Y16bit,
    PROC1_FMT_MAX      // maximum image format enum
} PROC_IMAGE_FORMAT;

typedef struct MFBLL_PROC1_OUT
{
    MUINT8 *pu1ConfMap;
    MUINT32 u4MapSize;
    MUINT8 *pu1MV;
    MINT32	*pi4WpeMapX;
    MINT32	*pi4WpeMapY;
    MUINT32 u4WpeMapSize;
    MUINT32 u4MVSize;
} MFBLL_PROC1_OUT_STRUCT, *P_MFBLL_PROC1_OUT_STRUCT;

#ifdef KEEP_PROC2
typedef struct
{
    PROC_IMAGE_FORMAT ImgFmt;

    MUINT8 *pbyInImg;
    MUINT32  i4InWidth;
    MUINT32  i4InHeight;

    MUINT8 *pbyOuImg;
    MUINT32  i4OuWidth;
    MUINT32  i4OuHeight;
} MFBLL_PROC2_OUT_STRUCT,*P_MFBLL_PROC2_OUT_STRUCT;
#endif

typedef struct
{
    MUINT32 Ext_mem_size;
    MUINT32 CofMap_width;
    MUINT32 CofMap_height;
    MUINT32 MV_width;
    MUINT32 MV_height;
    MUINT32 WpeMap_width;
    MUINT32 WpeMap_height;

} MFBLL_GET_PROC_INFO_STRUCT, *P_MFBLL_GET_PROC_INFO_STRUCT;

struct MFBLL_SET_PROC_INFO_STRUCT
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

    MUINT32	 Proc1_me_wpe_image_width;
    MUINT32	 Proc1_me_wpe_image_height;
    MUINT32	 Proc1_me_wpe_np1_mode;

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

    PROC_IMAGE_FORMAT Proc1_ImgFmt;

    MUINT32 iBssOrgScore_base;
    MUINT32 iBssOrgScore_ref;

    MINT8   Proc_idx;  //debug

    void* pSWMENvram;

    HG_Info HGInfo;

    GYRO_MV_INFO GyroMVInfo;
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
    MFBLL_SET_PROC_INFO_STRUCT()
    {
        memset(this, 0x0, sizeof(struct MFBLL_SET_PROC_INFO_STRUCT));
    }
};
typedef MFBLL_SET_PROC_INFO_STRUCT*  P_MFBLL_SET_PROC_INFO_STRUCT;


struct MFBLL_VerInfo
{
    char rMainVer[5];
    char rSubVer[5];
    char rPatchVer[5];
} ;                      // get Version by FeatureCtrl(GET_VERSION)

/*******************************************************************************
*
********************************************************************************/
class MTKMfbll
{
public:
    static MTKMfbll* createInstance(DrvMfbllObject_e eobject);
    virtual void   destroyInstance() = 0;
    virtual ~MTKMfbll(){}
    virtual MRESULT MfbllInit(void* pParaIn, void* pParaOut);
    virtual MRESULT MfbllReset(void);
    virtual MRESULT MfbllMain(MFBLL_PROC_ENUM ProcId, void* pParaIn, void* pParaOut);
    virtual MRESULT MfbllFeatureCtrl(MFBLL_FTCTRL_ENUM FcId, void* pParaIn, void* pParaOut);
private:

};

class AppMfbllTmp : public MTKMfbll
{
public:

    static MTKMfbll* getInstance();
    virtual void destroyInstance();

    AppMfbllTmp() {}
    virtual ~AppMfbllTmp() {}
};
#endif


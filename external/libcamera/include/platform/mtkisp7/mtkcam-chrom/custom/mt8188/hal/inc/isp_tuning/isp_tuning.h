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


#ifndef _ISP_TUNING_H_
#define _ISP_TUNING_H_

#include "isp_tuning_sensor.h"
#include "tuning_mapping/cam_idx_struct_ext_pub.h"

#define ISP_STANDARD_CCM_ACC (9)

#define ISP_STANDARD_GGM_ENDVAR (1023)

#define ISP_STANDARD_C2G_CNV_00 (512)   // 512
#define ISP_STANDARD_C2G_CNV_01 (0)     // 0
#define ISP_STANDARD_C2G_CNV_02 (718)   // 718
#define ISP_STANDARD_C2G_CNV_10 (512)   // 512
#define ISP_STANDARD_C2G_CNV_11 (1872)  // -176
#define ISP_STANDARD_C2G_CNV_12 (1682)  // -366
#define ISP_STANDARD_C2G_CNV_20 (512)   // 512
#define ISP_STANDARD_C2G_CNV_21 (907)   // 907
#define ISP_STANDARD_C2G_CNV_22 (0)     // 0

#define ISP_STANDARD_G2C_CNV_00 (153)   // 153
#define ISP_STANDARD_G2C_CNV_01 (301)   // 301
#define ISP_STANDARD_G2C_CNV_02 (58)    // 58
#define ISP_STANDARD_G2C_CNV_10 (1962)  // -86
#define ISP_STANDARD_G2C_CNV_11 (1878)  // -170
#define ISP_STANDARD_G2C_CNV_12 (256)   // 256
#define ISP_STANDARD_G2C_CNV_20 (256)   // 256
#define ISP_STANDARD_G2C_CNV_21 (1834)  // -214
#define ISP_STANDARD_G2C_CNV_22 (2006)  // -42

#define ISP_HDR10P_C2G_CNV_00 (512)   // 512
#define ISP_HDR10P_C2G_CNV_01 (0)     // 0
#define ISP_HDR10P_C2G_CNV_02 (755)   // 755
#define ISP_HDR10P_C2G_CNV_10 (512)   // 512
#define ISP_HDR10P_C2G_CNV_11 (1964)  // -84
#define ISP_HDR10P_C2G_CNV_12 (1755)  // -293
#define ISP_HDR10P_C2G_CNV_20 (512)   // 512
#define ISP_HDR10P_C2G_CNV_21 (963)   // 963
#define ISP_HDR10P_C2G_CNV_22 (0)     // 0

#define ISP_HDR10P_G2C_CNV_00 (135)   // 135
#define ISP_HDR10P_G2C_CNV_01 (347)   // 347
#define ISP_HDR10P_G2C_CNV_02 (30)    // 30
#define ISP_HDR10P_G2C_CNV_10 (1977)  // -71
#define ISP_HDR10P_G2C_CNV_11 (1863)  // -185
#define ISP_HDR10P_G2C_CNV_12 (256)   // 256
#define ISP_HDR10P_G2C_CNV_20 (256)   // 256
#define ISP_HDR10P_G2C_CNV_21 (1813)  // -235
#define ISP_HDR10P_G2C_CNV_22 (2027)  // -21

namespace NSIspTuning
{


/*******************************************************************************
*
*******************************************************************************/
typedef enum MERROR_ENUM
{
    MERR_OK         = 0,
    MERR_UNKNOWN    = 0x80000000, // Unknown error
    MERR_UNSUPPORT,
    MERR_BAD_PARAM,
    MERR_BAD_CTRL_CODE,
    MERR_BAD_FORMAT,
    MERR_BAD_ISP_DRV,
    MERR_BAD_NVRAM_DRV,
    MERR_BAD_SENSOR_DRV,
    MERR_BAD_SYSRAM_DRV,
    MERR_SET_ISP_REG,
    MERR_NO_MEM,
    MERR_NO_SYSRAM_MEM,
    MERR_NO_RESOURCE,
    MERR_CUSTOM_DEFAULT_INDEX_NOT_FOUND,
    MERR_CUSTOM_NOT_READY,
    MERR_PREPARE_HW,
    MERR_APPLY_TO_HW,
    MERR_CUSTOM_ISO_ENV_ERR,
    MERR_CUSTOM_CT_ENV_ERR
} MERROR_ENUM_T;

/*******************************************************************************
* Operation Mode
*******************************************************************************/
typedef enum
{
    EOperMode_Normal    = 0,
    EOperMode_PureRaw,
    EOperMode_Meta,
    EOperMode_EM,
    EOperMpde_Factory
} EOperMode_T;

typedef enum
{
    ERawType_Proc = 0,
    ERawType_Pure = 1,
    ERawType_PartialProc_afterBPC = 2
}   ERawType_T;

typedef enum
{
    ERaw2Yuv      = 0,
    EYuv2Yuv      = 1,
    ERaw2Yuv_MONO = 5  //without CamInfo
}   EP2IN_FMT_T;

typedef enum
{
    ENormalUpdate      = 0,
    EPartKeep          = 1,
    EAllKeep           = 2,
    ELPCNR_8Bit_Pass1  = 3,
    ELPCNR_8Bit_Pass2  = 4,
    EContinuousShots   = 5,
    ELPCNR_10Bit_Pass1 = 6,
    ELPCNR_10Bit_Pass2 = 7,
    EIdenditySetting   = 8,
}   EP2UPDATE_MODE;

typedef enum
{
    EHWHDRType_None = 0,
    EHWHDRType_mStream = 1,
    EHWHDRType_Stagger2 = 2,
    EHWHDRType_Stagger3 = 3
}   EHWHDRType_T;

}   //  NSIspTuning

#endif //  _ISP_TUNING_H_


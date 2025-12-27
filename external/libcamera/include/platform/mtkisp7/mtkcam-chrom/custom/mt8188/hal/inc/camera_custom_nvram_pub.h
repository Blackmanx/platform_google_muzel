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


#ifndef _CAMERA_CUSTOM_NVRAM_PUB_H_
#define _CAMERA_CUSTOM_NVRAM_PUB_H_

#include <stddef.h>
#include "MediaTypes.h"
#include "CFG_Camera_File_Max_Size.h"
#ifndef MTK_STEREO_KERNEL_NVRAM_LENGTH
//May defined in algo header
#define MTK_STEREO_KERNEL_NVRAM_LENGTH (8400) // refer to the same define in MtkStereoKernel.h
#endif

/*******************************************************************************
*
********************************************************************************/
typedef enum
{
    CAMERA_DATA_TYPE_START=0,
    CAMERA_NVRAM_DATA_ISP = CAMERA_DATA_TYPE_START,
    CAMERA_NVRAM_DATA_CAC,
    CAMERA_NVRAM_DATA_GEOMETRY,
    CAMERA_NVRAM_DATA_FOV,
    CAMERA_NVRAM_DATA_FEATURE,
    CAMERA_NVRAM_VERSION,
    CAMERA_DATA_TYPE_NUM
} CAMERA_DATA_TYPE_ENUM;

typedef enum
{
    AE_CUSTOM_TRANSFORM_START = 0,
    AE_CUSTOM_TRANSFORM_BINSUM = AE_CUSTOM_TRANSFORM_START,
    AE_CUSTOM_TRANSFORM_ATR,
    AE_CUSTOM_TRANSFORM_BINSUM_AIHDR,
    AE_CUSTOM_TRANSFORM_SENSORLIMIT, // for stagger
    AE_CUSTOM_TRANSFORM_NUM
} AE_CUSTOM_TRANSFORM_ENUM;

#define GIS_MAXSUPPORTED_SMODE (20)

typedef struct NVRAM_CAMERA_FEATURE_GIS_STRUCT_t
{
    MUINT32 gis_defWidth;
    MUINT32 gis_defHeight;
    MUINT32 gis_defCrop;
    double   gis_defParameter1[6]; //tRS, Bias x, y, z, FL, Toffset,
    double   gis_defParameter2[6]; //tRS, Bias x, y, z, FL, Toffset,
    double   gis_defParameter3[6]; //tRS, Bias x, y, z, FL, Toffset,
    double   gis_deftRS[GIS_MAXSUPPORTED_SMODE]; //tRS by Sensor mode
    double   gis_deftRerserved1[GIS_MAXSUPPORTED_SMODE]; //tRS by Sensor mode
    double   gis_deftRerserved2[GIS_MAXSUPPORTED_SMODE]; //tRS by Sensor mode
} NVRAM_CAMERA_FEATURE_GIS_STRUCT, *PNVRAM_CAMERA_FEATURE_GIS_STRUCT;

typedef struct
{
    char msg_ver[500];
} NVRAM_CAMERA_FEATURE_VERSION_T, *PNVRAM_CAMERA_FEATURE_VERSION_T;

typedef struct NVRAM_CAMERA_FEATURE_STRUCT_t
{
    NVRAM_CAMERA_FEATURE_GIS_STRUCT         gis;
    NVRAM_CAMERA_FEATURE_VERSION_T          VER_FOR_TUNING;

} NVRAM_CAMERA_FEATURE_STRUCT, *PNVRAM_CAMERA_FEATURE_STRUCT;

typedef struct // maximum 2048 bytes
{
    // MFLL
    MUINT8  capture_M;       //capture frame number, default=6, range=2~8, step=1
    MUINT8  blend_N;         //blending frame number, default=6, range=2~8, step=1, blend_frame_number< capture_frame_number

    MUINT32 memc_bad_mv_range;          //default=255
    MUINT32 memc_bad_mv_rate_th;        //default=12707

    MUINT32 me_noise_lv; //noise level for ME algorithm. The value can equal to ISO speed for IMX519 sensor.
    MUINT32 conf_noise_lv; //noise level for ME confidence caculation

    //DSDN related settings
    MUINT8 dsdn_ratio; //The scaling ratio of DSDN (1~16, default 16). dsdn_ratio/16. If dsdn_ratio=16, dsdn is disabled.
    MUINT8 memc_dsus_mode; //The mode of DSDN (called DSUS in ME).

    //AE variation compensation
    MUINT8 aevc_ae_en; //Enable AE variation compensation. 0:disable, 1:enable. default = 1
    MUINT8 aevc_lcso_en; //Enable AE variation compensation for lcso table. 0:disable, 1:enable. default = 1

    //Post refine
    MUINT8 post_refine_en; //Enable post refine. default = 1.
    MUINT8 post_me_refine_en; //Enable post refine parameter by me output. default =1. (if post_refine_en=0, don't care this variable)

    //Global drop frm
    MUINT16 post_me_refine_mv_ratio; // parameter for me output index calculation. range=0~256, default = 128;
    MUINT16 post_me_refine_face_ratio_ThL; // range: 0 ~ 10000, default = 1000
    MUINT16 post_me_refine_face_ratio_ThH; // range: 0 ~ 10000, default = 3200
    MUINT16 post_me_refine_full_ratio_ThL; // range: 0 ~ 10000, default = 1000
    MUINT16 post_me_refine_full_ratio_ThH; // range: 0 ~ 10000, default = 3200
    MUINT16 post_me_refine_edge_ccl_Th[2]; // range: 0 ~ 65535, default = {600, 600}
    MUINT8 post_me_refine_edge_cclnum_Th;  // range: 0 ~   255, default = 3
    MUINT16 post_me_refine_edge_FDAreaThL; // range: 0 ~ 65535, default = 0
    MUINT16 post_me_refine_edge_FDAreaThH; // range: 0 ~ 65535, default = 16384

    //Local decrease
    MUINT8  lcl_deconf_en; //Enable local confidence fallback. 0:disable, 1:enable. default = 0
    MUINT32 lcl_deconf_noise_lv;    //parameter for confidence decrease calculation.
    MUINT16 lcl_deconf_bg_bss_ratio;//parameter for confidence decrease calculation. range=0~256, default = 256;
    MUINT16 lcl_deconf_fd_bss_ratio;//parameter for confidence decrease calculation. range=0~256, default = 243;
    MUINT8 lcl_deconf_dltvar_en;   //parameter for confidence decrease calculation. default = 0

    // reserved space
    MUINT8 ext_setting; // use extension settings of reserved block. disable = 0. default = 0, 1: parameter add to post_refine_int

    ///////////////////////////////////////////////////////
    //extension version 1 Start
    //##### Eye
    MUINT32  Blink_eye_en;
    MUINT32  Blink_eye_ThL;
    MUINT32  Blink_eye_ThH;
    MUINT32  Blink_eye_qstep;
    MUINT32  post_refine_int;
    MUINT32  me_large_mv_thd;
    MUINT32  me_large_mv_SAD_thd;
    MUINT32  me_large_mv_ratio;
    MUINT32  me_large_mv_txtr_en;
    MUINT32  me_large_mv_txtr_wei;
    MUINT32  me_large_mv_txtr_thd;
    //extension version 1 End
    ///////////////////////////////////////////////////////

    // reserved space
    MUINT32 reserved[102];

} NVRAM_CAMERA_FEATURE_MFLL_STRUCT, *PNVRAM_CAMERA_FEATURE_MFLL_STRUCT;

static_assert( sizeof(NVRAM_CAMERA_FEATURE_MFLL_STRUCT) <= 2048,
        "NVRAM_CAMERA_FEATURE_MFLL_STRUCT is greater 2048 bytes, please make sure " \
        "it's smaller 2048 bytes");

typedef struct
{
    MINT32 iso_th;
    MINT32 ext_setting;
    MINT32 reserved;
} NVRAM_CAMERA_FEATURE_MFNR_THRES_STRUCT, *PNVRAM_CAMERA_FEATURE_MFNR_THRES_STRUCT;


typedef union
{
    struct
    {
        MUINT32 StereoData[MTK_STEREO_KERNEL_NVRAM_LENGTH];
        float DepthAfData[ (MAXIMUM_NVRAM_CAMERA_GEOMETRY_FILE_SIZE - MTK_STEREO_KERNEL_NVRAM_LENGTH * sizeof(MUINT32) ) / sizeof(float) ];
    } StereoNvramData;

    UINT8   Data[MAXIMUM_NVRAM_CAMERA_GEOMETRY_FILE_SIZE];
}NVRAM_CAMERA_GEOMETRY_STRUCT;

typedef struct NVRAM_CAMERA_FOV_STRUCT_t
{
    UINT8   Data[MAXIMUM_NVRAM_CAMERA_FOV_FILE_SIZE];
} NVRAM_CAMERA_FOV_STRUCT, *PNVRAM_CAMERA_FOV_STRUCT;

static_assert(
        sizeof(NVRAM_CAMERA_FOV_STRUCT) <= MAXIMUM_NVRAM_CAMERA_FOV_FILE_SIZE,
        "nvram fov size is not enough"
        );

#endif // _CAMERA_CUSTOM_NVRAM_PUB_H_


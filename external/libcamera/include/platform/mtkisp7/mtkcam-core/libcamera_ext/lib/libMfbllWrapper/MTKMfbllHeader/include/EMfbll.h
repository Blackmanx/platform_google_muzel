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

#ifndef _E_MFBLL_H
#define _E_MFBLL_H


typedef enum MFBLL_FEATURE_ENUM
{
    None = 0,
    MFNR,
    AIS2,
} MFBLL_FEATURE_ENUM;


typedef enum IDRVMfbllObject_s
{
    IDRV_MFBLL_OBJ_NONE = 0,
    IDRV_MFBLL_OBJ_SW,
    IDRV_MFBLL_OBJ_UNKNOWN = 0xFF,
} IDrvMfbllObject_e;

typedef enum IMFBLL_PROC_ENUM
{
    IMFBLL_PROC1 = 0,
#ifdef KEEP_PROC2
    IMFBLL_PROC2,
#endif
    IMFBLL_UNKNOWN_PROC,
} IMFBLL_PROC_ENUM;

typedef enum IMFBLL_FTCTRL_ENUM
{
    IMFBLL_FTCTRL_GET_PROC_INFO,
    IMFBLL_FTCTRL_SET_PROC_INFO,
    IMFBLL_FTCTRL_GET_VERSION,        // feature id to get Version
    IMFBLL_FTCTRL_CAL_GYRO_MV,
    IMFBLL_FTCTRL_MAX
} IMFBLL_FTCTRL_ENUM;

typedef enum IPROC_IMAGE_FORMAT
{
    IPROC1_FMT_YV16 = 0, // 422 : 3 plane , Y..U..V..
    IPROC1_FMT_YUY2, // 422 : YUV YUV ...
    IPROC1_FMT_NV12, // 420 : 2 plane , Y... UVUV..
    IPROC1_FMT_Y,
    IPROC1_FMT_Y16bit,
    IPROC1_FMT_MAX      // maximum image format enum
} IPROC_IMAGE_FORMAT;
#endif
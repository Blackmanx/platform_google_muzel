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

#ifndef _E_BSS_H
#define _E_BSS_H
/*
typedef enum BSS_FEATURE_ENUM
{
    None = 0,
    MFNR,
    AIS2,
} BSS_FEATURE_ENUM;
*/
typedef enum IBSS_PROC_ENUM
{
    IBSS_PROC1 = 0,
    IBSS_PROC2,
    IBSS_PROC3,
    IBSS_UNKNOWN_PROC,
} IBSS_PROC_ENUM;

typedef enum IDRVBssObject_s
{
    IDRV_BSS_OBJ_NONE = 0,
    IDRV_BSS_OBJ_SW,
    IDRV_BSS_OBJ_UNKNOWN = 0xFF,
} IDrvBssObject_e;

typedef enum IBSS_PROC_TYPE
{
    IBSS_TYPE_PACK_RAW10 = 0,
    IBSS_TYPE_PACK_Y10,
    IBSS_TYPE_UNPACK_NV21,
    IBSS_UNKNOWN_TYPE
} IBSS_PROC_TYPE;

typedef enum IBSS_FTCTRL_ENUM
{
  IBSS_FTCTRL_GET_WB_SIZE,
  IBSS_FTCTRL_SET_WB_SIZE,
  IBSS_FTCTRL_SET_PROC_INFO,
  IBSS_FTCTRL_CONFIG_ZIP,
  IBSS_FTCTRL_PROC_ZIP,
  IBSS_FTCTRL_CONVERT_I422_YUY2,
  IBSS_FTCTRL_GET_VERSION,        // feature id to get Version
  IBSS_FTCTRL_MAX
} IBSS_FTCTRL_ENUM;
#endif
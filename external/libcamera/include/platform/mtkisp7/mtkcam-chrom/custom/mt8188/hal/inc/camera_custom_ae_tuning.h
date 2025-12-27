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


#ifndef _CAMERA_CUSTOM_AE_TUNING_H_
#define _CAMERA_CUSTOM_AE_TUNING_H_

#include <stddef.h>
#include "MediaTypes.h"
#include "camera_custom_AEPlinetable.h"
#include "CFG_Camera_File_Max_Size.h"
//#include "ae_feature.h"

// temp define module ID and LensID
#define CAMERA_MODULE_ID 0
#define CAMERA_LENS_ID 0

typedef struct {
    MUINT32 u4AECustomVersion;
} strAEParam;

typedef struct {
    strAEParam CustomAEInfo;
} strAECustomParam;

typedef struct
{
    strAECustomParam *pCustomParam;
}AE_CUST_PARAM_T;

#define AE_DEBUG_DATA_CUST_SIZE (61440) // 60k

typedef struct
{
    MUINT32 u4Size; // using size
    MUINT8 data[AE_DEBUG_DATA_CUST_SIZE];
} AE_DEBUG_DATA_CUST_T;

#endif

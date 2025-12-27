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

#ifndef AE_CUSTOM_TRANSFORM_H_
#define AE_CUSTOM_TRANSFORM_H_

#include "MediaTypes.h"
#include "camera_custom_ae.h"

VOID transShutterPriority(MUINT32 u4Shutter, const VOID* const pNVRAMData, VOID* const pData);
VOID transISOPriority(MUINT32 u4ISO, const VOID* const pNVRAMData, VOID* const pData);
VOID transSuperNightShot(AE_CUST_Super_Night_Param_T* a_AeCustParam, const VOID* const pNVRAMData, VOID* const pData);
VOID transCustomAESetting(VOID* const pCustomAESettingData);

#endif


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
#ifndef __AAA_YUV_TUNING_CUSTOM_H__
#define __AAA_YUV_TUNING_CUSTOM_H__

namespace NSYuvTuning
{
    MUINT32     custom_GetFlashlightGain10X(MINT32 i4SensorDevId);
    MUINT32     custom_BurstFlashlightGain10X(MINT32 i4SensorDevId);
    MDOUBLE     custom_GetYuvFlashlightThreshold(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvFlashlightFrameCnt(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvFlashlightDuty(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvFlashlightStep(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvFlashlightHighCurrentDuty(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvFlashlightHighCurrentTimeout(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvAfLampSupport(MINT32 i4SensorDevId);
    MINT32      custom_GetYuvPreflashAF(MINT32 i4SensorDevId);
}

#endif //__AAA_YUV_TUNING_CUSTOM_H__


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
#ifndef _CAMERA_CUSTOM_MVHDR3EXPO_H_
#define _CAMERA_CUSTOM_MVHDR3EXPO_H_

#include "camera_custom_types.h"  // For MUINT*/MINT*/MVOID/MBOOL type definitions.
#include "camera_custom_ivhdr.h"

#define MVHDR3EXPO_WIDTH  8
#define MVHDR3EXPO_HEIGHT 6

MUINT32 getMVHDR3ExpoBufSize();
MVOID decodeMVHDR3ExpoStatistic( MINT32 i4SensorDev,
                                 MVOID *pAEYDataPointer, MVOID *pAEHistDataPointer, MVOID *pEmbDataPointer, // input data
                                 MVOID *pOutputDataPointer, MUINT32 &u4MVHDRRatio_x100); // output data
MVOID getMVHDR3Expo_AEInfo(const MINT32 i4Ratio, MINT32 &i4SEDeltaEVx100);

#endif // _CAMERA_CUSTOM_MVHDR3EXPO_H_


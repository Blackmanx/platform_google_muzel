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


#ifndef _CAMERA_CUSTOM_AE_SYNC_NVRAM_H_
#define _CAMERA_CUSTOM_AE_SYNC_NVRAM_H_

#define AESYNC_DENOISE_MAPPING_TABLE_MAX 30
#define AESYNC_EVOFFSET_ARRAY_MAX 5
#define AESYNC_REVERSE_ARRAY_MAX 30
/*typedef enum
{
    AESYNC_DENOISE_BMDN = 0,
    AESYNC_DENOISE_MFNR,
    AESYNC_DENOISE_MAX
}AESYNC_DENOISE_ENUM;*/


//--------------------------------------------------------------------------------------------------------------------------------------------------------------
typedef struct
{
    MBOOL bGainRegression;
    // 0: cwv; 1: avgY
    MUINT16 u2RegressionType;
    MUINT16 u2AlignMode;
    MINT32 i4EvOffset[AESYNC_EVOFFSET_ARRAY_MAX];
    MUINT32 u4RGB2YCoef[3];
    MUINT32 u4FixSyncGain;
    MUINT32 pDeltaBvToRatioArray[AESYNC_DENOISE_MAX][AESYNC_DENOISE_MAPPING_TABLE_MAX];
    MINT32 pReserved[AESYNC_REVERSE_ARRAY_MAX];
} AESYNC_NVRAM_T;

#endif
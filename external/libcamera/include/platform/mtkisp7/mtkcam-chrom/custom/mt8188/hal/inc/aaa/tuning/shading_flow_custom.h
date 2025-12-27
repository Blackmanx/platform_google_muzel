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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
#ifndef _SHADING_FLOW_CUSTOM_H_
#define _SHADING_FLOW_CUSTOM_H_

#include "camera_custom_types.h"
//#include <ae_param.h>

// TODO : Need to think how to fix
#ifndef AE_BLOCK_NO
#define AE_BLOCK_NO  5
#endif

typedef struct
{
    MUINT32 u4ShadingCCT;
    // AWB info for ISP tuning
    MINT32  i4AWBCCT; // CCT
    // Flash info for ISP tuning
    MBOOL   isFlash; //0: no flash, 1: image with flash
    // AE info for ISP tuning
    MUINT32 u4RealISOValue;
    MUINT32 pu4AEBlock[AE_BLOCK_NO][AE_BLOCK_NO];
    MBOOL   bEnableRAFastConverge;
    MUINT32 u4MgrCWValue;
    MBOOL   TgCtrlRight;
    MINT32  i4deltaIndex;
    MINT32  u4AEFinerEVIdxBase;
    MBOOL   bAEStable;
    MUINT32 u4AvgWValue;
    MBOOL   bAELock;
    MBOOL   bAELimiter;
    MINT32  i4AEComp;
    //for ratio mapping
    const MINT32* isoTbl;
    const MINT32* rtoTbl;
    MBOOL   bRatioSmoothEn;
} LSC_INPUT_INFO_T;

#endif

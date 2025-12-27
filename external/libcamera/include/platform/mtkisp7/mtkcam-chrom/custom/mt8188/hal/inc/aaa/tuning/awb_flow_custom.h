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
#ifndef _AWB_FLOW_CUSTOM_H_
#define _AWB_FLOW_CUSTOM_H_

#include "camera_custom_types.h"
#include "camera_custom_nvram_pub.h"


// Update information for ISP used
struct AWB_ISP_INFO_T {
    MINT32 i4CCT; // CCT
    AWB_GAIN_T rAwbGainPref; //add for ISP 6.0
    AWB_GAIN_T rAwbGainNoPref; //AWB gain without preference
    MINT32 i4SatRatio; //add for ISP 6.0

    //for ISP tuning
    AWB_GAIN_T rCurrentAWBGain; // Current AWB gain
    MINT32 i4FluorescentIndex; // Fluorescent index

    //for ResultPool
    AWB_GAIN_T rPregain1; // AA pregain1
    MINT32 rCscCCM[9]; // AA CCM

    //for paramctrl_per_frame
    AWB_GAIN_T rRPG; //RPG
    AWB_GAIN_T rPGN; //PGN
    MBOOL bAWBLock;  //CCM

    //for AI3A
    MINT32 rGGMtable[192];
};

struct AWB_STAT_INFO_T {
    //For AE
    MINT32 i4StatEnable;

    MINT32 i4WindowSizeX;
    MINT32 i4WindowSizeY;

    MINT32 i4LowThresholdR;  // Error pixel low threshold of R (8-bit)
    MINT32 i4LowThresholdG;  // Error pixel low threshold of G (8-bit)
    MINT32 i4LowThresholdB;  // Error pixel lowthreshold of B (8-bit)
    MINT32 i4HighThresholdR; // Error pixel high threshold of R (8-bit)
    MINT32 i4HighThresholdG; // Error pixel high threshold of G (8-bit)
    MINT32 i4HighThresholdB; // Error pixel high threshold of B (8-bit)

    MINT32 i4LinearOutputEn; // Output format select 1: Linear mode, 0: non-linear mode

    //For AI info
    MINT32 i4AAOStride;
    MINT32 i4crop_x;      //AAO config origin X
    MINT32 i4crop_y;      //AAO config origin Y
    MINT32 i4crop_width;  //AAO width
    MINT32 i4crop_height; //AAO height
    MINT32 i4AISEG_enable;
    MINT32 i4AISEG_ISOthr;
    MINT32 i4AISEG_COUNTthr;
    MINT32 i4AIE2E_enable;
    MINT32 i4AIE2E_ISOthr;
    MINT32 i4AIE2E_COUNTthr;
    MINT32 i4MasterDev;

    //For AIGA
    MINT32 i4GAconfig_enable;
    MINT32 i4GAconfig_param[27];
};

#endif

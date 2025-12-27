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


#ifndef _CAMERA_CUSTOM_3A_NVRAM_H_
#define _CAMERA_CUSTOM_3A_NVRAM_H_

#include <stddef.h>
#include "MediaTypes.h"
#include "camera_custom_AEPlinetable.h"
#include "CFG_Camera_File_Max_Size.h"

/*******************************************************************************
* AF
********************************************************************************/

// Camera Scenario
typedef enum
{
    CAM_SCENARIO_PREVIEW = 0,  // PREVIEW
    CAM_SCENARIO_VIDEO,        // VIDEO
    CAM_SCENARIO_CAPTURE,      // CAPTURE
    CAM_SCENARIO_CUSTOM1,      // HDR
    CAM_SCENARIO_CUSTOM2,      // AUTO HDR
    CAM_SCENARIO_CUSTOM3,      // VT
    CAM_SCENARIO_CUSTOM4,      // STEREO
    CAM_SCENARIO_NUM
} CAM_SCENARIO_T;

typedef struct CAMERA_PDTBL_STRUCT_t
{
    MUINT32 bpci_xsize;
    MUINT32 bpci_ysize;
    MUINT32 pdo_xsize;
    MUINT32 pdo_ysize;
    MUINT8* bpci_array;

} CAMERA_PD_TBL_STRUCT, *PCAMERA_PD_TBL_STRUCT;

typedef struct CAMERA_BPCI_STRUCT_t
{
    CAMERA_PD_TBL_STRUCT PDE_TBL;
    CAMERA_PD_TBL_STRUCT PDC_FULL_TBL;
    CAMERA_PD_TBL_STRUCT PDC_BIN_TBL;

} CAMERA_BPCI_STRUCT, *PCAMERA_BPCI_STRUCT;

#endif // _CAMERA_CUSTOM_3A_NVRAM_H_


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

#ifndef _CAM_IDX_STRUCT_MAPPING_H_
#define _CAM_IDX_STRUCT_MAPPING_H_

#include <map>

namespace NSIspTuning{
typedef struct _CAM_IDX_QRY_COMB_ISP7
{
    struct {
        EAction_T eAction;
        EApp_T eApp;
        EAppSize_T eAppSize;
        ECameraModule_T eCameraModule;
        ECustom_T eCustom;
        ECustomFeature_T eCustomFeature;
        EFPS_T eFPS;
        EFeature_T eFeature;
        EFlash_T eFlash;
        EFlashDevice_T eFlashDevice;
        ELatency_T eLatency;
        EProject_T eProject;
        ESensor_T eSensor;
        ESensorFeature_T eSensorFeature;
        ESensorMode_T eSensorMode;
        EStandard_T eStandard;
        EYUVSize_T eYUVSize;
        EZoom_T eZoom;
        ECT_T eCT;
        EDR_T eDR;
        EFaceDetection_T eFaceDetection;
        EISO_T eISO[NVRAM_ISP_REGS_ISO_GROUP_NUM];
        ELV_T eLV;
        ERatio_T eRatio;
        EStage_T eStage;
        EToneGain_T eToneGain;
        ETripod_T eTripod;
    };
    MUINT32 query[EDim_NUM+NVRAM_ISP_REGS_ISO_GROUP_NUM-1];
    _CAM_IDX_QRY_COMB_ISP7() {
      memset(this, 0, sizeof(_CAM_IDX_QRY_COMB_ISP7));
    }
} CAM_IDX_QRY_COMB_ISP7;

}
#endif // _CAM_IDX_STRUCT_MAPPING_H_

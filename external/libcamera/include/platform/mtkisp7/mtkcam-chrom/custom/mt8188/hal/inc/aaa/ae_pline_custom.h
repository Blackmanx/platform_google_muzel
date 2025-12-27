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

#ifndef AE_MANUAL_PLINE_CUSTOM_H_
#define AE_MANUAL_PLINE_CUSTOM_H_

#include <ae_feature.h>
#include <camera_custom_nvram.h>
#include <camera_custom_AEPlinetable.h>
#include "camera_custom_types.h"
#include "ae_setting.h"
#include <stdio.h>

#define CUST_AE_MP_PARAM MFALSE

enum EAEManualPline_T
{
    EAEManualPline_Default           = 0x0000,
    EAEManualPline_ADBCtrol,

    // feature manual pline
    EAEManualPline_EISRecord1         = 0x0010,
    EAEManualPline_EISRecord2,
    EAEManualPline_SM120FPS,
    EAEManualPline_SM240FPS,
    EAEManualPline_SM480FPS,
    EAEManualPline_AIS1Capture,
    EAEManualPline_AIS2Capture,
    EAEManualPline_MFHRCapture,
    EAEManualPline_BMDNCapture,
    EAEManualPline_ShtterISOPriority,
    EAEManualPline_MStreamVhdr,
    EAEManualPline_StaggerVhdr,
    EAEManualPline_Vhdr60FPS,
    EAEManualPline_Video,
    EAEManualPline_Video60FPS,

    // customer manual pline
    EAEManualPline_Custom1           = 0x0100,
    EAEManualPline_Custom2,
    EAEManualPline_Custom3,
    EAEManualPline_Custom4,
    EAEManualPline_Custom5,

    EAEManualPline_Num
};

typedef struct MPParam{
    int MinFps;
    int MaxFps;
    int SensorMode;
    int SceneMode;
    int FixISOSpeed;
    long FixShutter;
    MPParam(): MinFps(0), MaxFps(0), SensorMode(0), SceneMode(0), FixISOSpeed(0), FixShutter(0){}
    MPParam(int minFps, int maxFps, int sensorMode, int sceneMode, int isoMode,long shutter)
    : MinFps(minFps)
    , MaxFps(maxFps)
    , SensorMode(sensorMode)
    , SceneMode(sceneMode)
    , FixISOSpeed(isoMode)
    , FixShutter(shutter)
    {}
} ManualPlineParam;

void getAEManualPline(const ManualPlineParam& manualPlineParam, ae_pline_anchor_tbl& CurrentPline);
void getAEManualPline(const EAEManualPline_T& e_AEManualPline, ae_pline_anchor_tbl& CurrentPline);

#endif /* AE_MANUAL_PLINE_CUSTOM_H_ */


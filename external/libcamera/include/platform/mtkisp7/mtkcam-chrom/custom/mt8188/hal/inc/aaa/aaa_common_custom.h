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

#ifndef AAA_COMMON_CUSTOM_H_
#define AAA_COMMON_CUSTOM_H_

#include "camera_custom_nvram.h"    //CAM_SCENARIO_T;
#include <camera_custom_isp_nvram.h>
#include "custom/aaa/ae_param.h"               //AE_MODE;
#include <isp_tuning.h>
#include <vector>

// define for flash cap dummy frame
#define CUST_FLASH_DUMMY_CNT_BEFORE 2
#define CUST_FLASH_DUMMY_CNT_AFTER 3
//

MBOOL CUST_ENABLE_PRECAPTURE_AF(void);
MBOOL CUST_PRECAPTURE_AF_AFTER_PREFLASH(void);
MBOOL CUST_ENABLE_VIDEO_AUTO_FLASH(void);
MBOOL CUST_CAF_WINDOW_FOLLOW_TAF_WINDOW(void);
MBOOL CUST_ONE_SHOT_AE_BEFORE_TAF(void);
MBOOL CUST_SKIP_ONE_SHOT_AE_FOR_TAF(void);
MBOOL CUST_ENABLE_TOUCH_AE(void);
MBOOL CUST_ENABLE_FACE_AE(void);
MBOOL CUST_ENABLE_FACE_AWB(void);
MBOOL CUST_LOCK_AE_DURING_CAF(void);
MBOOL CUST_ENABLE_VIDEO_DYNAMIC_FRAME_RATE(void);
MBOOL CUST_ENABLE_FLASH_DURING_TOUCH(void);
MUINT32 CUST_FACE_AWB_CLEAR_COUNT(void);
MUINT32 CUST_GET_SYNC3A_AESTABLE_MAGIC(void);
MINT32 CUST_GET_SKIP_PRECAP_FLASH_FRAME_COUNT(void);
MBOOL CUST_LENS_COVER_COUNT(MINT32 i4LvMaster, MINT32 i4LvSlave);


struct ScenarioParam{
    NSIspTuning::EIspProfile_T eIspProfile;
    unsigned char CaptureIntent;
    unsigned char HdrMode;
    int Sync2AMode;
    int TargetMode;
    unsigned int SensorMode;

    ScenarioParam(): eIspProfile(NSIspTuning::EIspProfile_Preview), CaptureIntent(1), HdrMode(0), Sync2AMode(0), TargetMode(0), SensorMode(0){}
    ScenarioParam(NSIspTuning::EIspProfile_T IspProfile, unsigned char cap, unsigned char hdr, int sync2a, int target, unsigned int sensor)
    : eIspProfile(IspProfile)
    , CaptureIntent(cap)
    , HdrMode(hdr)
    , Sync2AMode(sync2a)
    , TargetMode(target)
    , SensorMode(sensor)
    {}
};

unsigned int Scenario4AE(const ScenarioParam&);
unsigned int Scenario4AWB(const ScenarioParam&);
unsigned int Scenario4AF(const ScenarioParam&);

std::vector<int> getShortExpFrame(void);

void cust_initSpecialLongExpOnOff(const MINT32 &i4AEEffectiveFrame);
void cust_setSpecialLongExpOnOff(const MINT64 &i8ExposureTime);
MBOOL cust_getIsSpecialLongExpOn();

#endif /* AAA_COMMON_CUSTOM_H_ */


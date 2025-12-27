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
#ifndef _DUALZOOM_CUSTOM_H_
#define _DUALZOOM_CUSTOM_H_

// define cam ID
#define DUALZOOM_WIDE_CAM_ID (0)
#define DUALZOOM_TELE_CAM_ID (2)
#define DUALZOOM_FOV_MAX_FPS (30)

// define dual zoom parameters
#define DUALZOOM_FOV_APPLIED_CAM (DUALZOOM_WIDE_CAM_ID)
#define DUALZOOM_FOV_MARGIN  (20) // 6% margin base on sensor size
#define DUALZOOM_FOV_MARGIN_COMBINE_EIS  (3) // 3% margin base on sensor size for combine EIS
#define DUALZOOM_FOV_MARGIN_PIXEL (384)
#define DUALZOOM_START_FOV_ZOOM_RATIO  (150) // 2.0x start do FOV
#define DUALZOOM_SWICH_CAM_ZOOM_RATIO  (200) // 2.0x switch camera
#define DUALZOOM_WAIT_STABLE_COUNT (120)      // wait count to set background camera go to lowpower
#define DUALZOOM_WAIT_LOW_POWER_COUNT (8)    // wait count to change state to lowpower state
#define DUALZOOM_WAIT_CAM_STANDBY_TO_ACT (8)   // wait count to change state(standby) to active state
#define DUALZOOM_WAIT_CAM_LOWFPS_TO_ACT  (0)   // wait count to change state(low fps) to active state
#define DUALZOOM_WIDE_STANDY_EN          (1)   // wide got to standby while equal to 1

// 3A policy tuning of dual zoom
#define DUALZOOM_AF_DAC_LOW_THRESHOLD (550)
#define DUALZOOM_AF_DAC_HIGH_THRESHOLD (600)
#define DUALZOOM_AE_LV_LOW_THRESHOLD (20)
#define DUALZOOM_AE_LV_HIGH_THRESHOLD (30)
#define DUALZOOM_AE_ISO_LOW_THRESHOLD (600)
#define DUALZOOM_AE_ISO_HIGH_THRESHOLD (1200)
#define DUALZOOM_AE_LV_DIFFERENCE (30)

// fov online calibration
#define DUALZOOM_FOV_ONLINE_ISO_MAX (2400)
#define DUALZOOM_FOV_ONLINE_EXPTIME_MAX (30000) // unit: us
#define DUALZOOM_FOV_ONLINE_DAC_WIDE_MAX (15)  // unit: 0.1%
#define DUALZOOM_FOV_ONLINE_DAC_TELE_MAX (20)  // unit: 0.1%
#define DUALZOOM_FOV_ONLINE_TEMP_MAX (100)     // unit: degree

#endif /* _DUALZOOM_CUSTOM_H_ */


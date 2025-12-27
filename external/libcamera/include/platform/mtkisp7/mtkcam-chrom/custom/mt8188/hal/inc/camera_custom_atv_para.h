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
#ifndef _CAMERA_CUSTOM_ATV_PARA_H_
#define _CAMERA_CUSTOM_ATV_PARA_H_
//
//
namespace NSCamCustom
{

/*******************************************************************************
* ATV disp delay time
*******************************************************************************/

#define ATV_MODE_NTSC 30000
#define ATV_MODE_PAL  25000

#ifdef MTK_MT5192
//unit: us
#define ATV_MODE_NTSC_DELAY 5000
#define ATV_MODE_PAL_DELAY  10000

#else 
#ifdef MTK_MT5193
//unit: us
#define ATV_MODE_NTSC_DELAY 18000
#define ATV_MODE_PAL_DELAY  26000
#else
//unit: us
#define ATV_MODE_NTSC_DELAY 0
#define ATV_MODE_PAL_DELAY  0
#endif

#endif

// 0 meas data pin is 2~9;  1 means data pin is 0~7
#define ATV_INPUT_DATA_FORMAT 1


};  //NSCamCustom
#endif  //  _CAMERA_CUSTOM_IF_


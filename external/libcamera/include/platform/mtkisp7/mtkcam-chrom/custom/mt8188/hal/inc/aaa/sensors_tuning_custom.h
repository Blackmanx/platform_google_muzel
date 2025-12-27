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

#ifndef __SENSORS_TUNING_CUSTOM_H__
#define __SENSORS_TUNING_CUSTOM_H__

#include <vector>

#include <mtkcam-interfaces/utils/sys/sensor_type.h>

#define STK37600_ALS_NAME "stk37600 als_rear"
#define STK37600_FLICKER_NAME "stk37600 flicker_rear"
#define STK37600_CCT_NAME "stk37600 cct_rear"
#define TCS3408_ALS_NAME "tcs3408 als_rear"
#define TCS3408_FLICKER_NAME "tcs3408 flicker_rear"
#define TCS3408_CCT_NAME "tcs3408 cct_rear"
#define TCS3701_ALS_NAME "tcs3701 LIGHT"
#define MN29005_ALS_NAME "mn29005 rear_als"
#define OPLUS_AI_SHUTTER_NAME "ai_shutter"

#define DEFAULT_SENSOR_ID 0x0000
#define STK37600_SENSOR_ID 0x037600
#define TCS3408_SENSOR_ID 0x3408
#define TCS3701_SENSOR_ID 0x3701
#define MN29005_SENSOR_ID 0x029005

using namespace NSCam::Utils;
using namespace std;

template <MINT32 SensorID>
MVOID* getALSTuningParameters();
template <MINT32 SensorID>
MVOID* getFlickerTuningParameters();
template <MINT32 SensorID>
MVOID* getColorTuningParameters();

MVOID* custom_getSensorsTuningParameters(eSensorType type, MINT32 ID);
MINT32 custom_mapSensorNameToSensorID(char* name);

vector<eSensorType> custom_getSensorsListByCameraSensorDev(int sensorDev);
#endif /* __SENSORS_TUNING_CUSTOM_H__*/


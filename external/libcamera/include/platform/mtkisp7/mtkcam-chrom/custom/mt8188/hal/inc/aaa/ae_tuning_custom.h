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

#ifndef _AE_TUNING_CUSTOM_H
#define _AE_TUNING_CUSTOM_H

#include <isp_tuning.h>
//#include "ae_param.h"
#include "camera_custom_ae_tuning.h"
#include "kd_imgsensor.h"

#define AE_CYCLE_NUM (3)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//         P U B L I C    F U N C T I O N    D E C L A R A T I O N              //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
/*
template <ESensorDev_T eSensorDev>
MBOOL isAEEnabled();

template <ESensorDev_T eSensorDev>
AE_PARAM_TEMP_T const& getAEParam();

template <ESensorDev_T eSensorDev>
AE_PARAM_TEMP_T const& getHDRAEParam();

template <ESensorDev_T eSensorDev>
AE_PARAM_TEMP_T const& getAUTOHDRAEParam();

template <ESensorDev_T eSensorDev>
AE_PARAM_TEMP_T const& getVTAEParam();

template <ESensorDev_T eSensorDev, CAM_SCENARIO_T eCamScenario>
AE_PARAM_TEMP_T const& getAEParamData();

template <ESensorDev_T eSensorDev>
const MINT32* getAEActiveCycle();

template <ESensorDev_T eSensorDev>
MINT32 getAECycleNum();
*/

// New Custom AE params
template <MINT32 i4SensorDevID, MINT32 i4ModuleID, MINT32 i4LensID>
AE_CUST_PARAM_T const& getCustomAEPreParam();

template <MINT32 i4SensorDevID, MINT32 i4ModuleID, MINT32 i4LensID>
AE_CUST_PARAM_T const& getCustomAEVdoParam();

template <MINT32 i4SensorDevID, MINT32 i4ModuleID, MINT32 i4LensID>
AE_CUST_PARAM_T const& getCustomAECapParam();

template <MINT32 i4SensorDevID, MINT32 i4ModuleID, MINT32 i4LensID, CAM_SCENARIO_T eCamScenario>
AE_CUST_PARAM_T const& getCustomAEParamData();

AE_CUST_PARAM_T const& getAEModuleParamData(CAM_SCENARIO_T eCamScenario, MINT32 i4SensorDevID, MINT32 i4ModuleID, MINT32 i4LensID);

#endif


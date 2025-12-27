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

#ifndef _AWB_TUNING_CUSTOM_H_
#define _AWB_TUNING_CUSTOM_H_

#include <isp_tuning/isp_tuning.h>
#include "awb_feature.h"
#include "awb_param.h"

#include "camera_custom_awb_tuning.h"
#include "kd_imgsensor.h"

template <ESensorDev_T eSensorDev>
MBOOL isAWBEnabled();

template <ESensorDev_T eSensorDev>
awb_param_t const& getAWBParam();

template <ESensorDev_T eSensorDev>
awbsync_custom_param const& getawbsync_custom_param();

//template <ESensorDev_T eSensorDev>
//AWB_STAT_PARAM_T const& getAWBStatParam();

template <MINT32 i4SensorDevID, MINT32 i4ModuleID, MINT32 i4LensID>
AWB_CUST_PARAM_T const& getCustomAWBPreParam();
template <MINT32 i4SensorDevID, MINT32 i4ModuleID, MINT32 i4LensID>
AWB_CUST_PARAM_T const& getCustomAWBVdoParam();
template <MINT32 i4SensorDevID, MINT32 i4ModuleID, MINT32 i4LensID>
AWB_CUST_PARAM_T const& getCustomAWBCapParam();
template <MINT32 i4SensorDevID, MINT32 i4ModuleID, MINT32 i4LensID, CAM_SCENARIO_T eCamScenario>
AWB_CUST_PARAM_T const& getCustomAWBParamData();
AWB_CUST_PARAM_T const& getAWBModuleParamData(CAM_SCENARIO_T eCamScenario, MINT32 i4SensorDevID, MINT32 i4ModuleID, MINT32 i4LensID);

#endif


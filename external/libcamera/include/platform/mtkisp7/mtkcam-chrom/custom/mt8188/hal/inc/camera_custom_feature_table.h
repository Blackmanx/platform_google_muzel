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


#ifndef _CAMERA_CUSTOM_FEATURE_TABLE_HEADER_H_
#define _CAMERA_CUSTOM_FEATURE_TABLE_HEADER_H_

// mtkcam header file
#ifndef MTKCAM_TINYMW_SUPPORT
#include <mtkcam3/3rdparty/core/scenario_type.h>
#include <mtkcam3/3rdparty/customer/customer_feature_type.h>
#include <mtkcam3/3rdparty/mtk/mtk_feature_type.h>
#else
#include <mtkcam-interfaces/thirdparty/core/scenario_type.h>
#include <mtkcam-interfaces/thirdparty/customer/customer_feature_type.h>
#include <mtkcam-interfaces/thirdparty/mtk/mtk_feature_type.h>
#endif
#include <map>
#include <vector>
#include <unordered_map>

using namespace NSCam::v3::pipeline::policy::scenariomgr;

// For Camera HAL server
extern const std::vector<std::unordered_map<int32_t, ScenarioFeatures>>  gCustomerScenarioFeaturesMaps;
extern const std::vector<std::unordered_map<int32_t, ScenarioFeatures>>  gCustomerScenarioFeaturesMapsPhyMaster;
extern const std::vector<std::unordered_map<int32_t, ScenarioFeatures>>  gCustomerScenarioFeaturesMapsPhySlave;

// For ISP HIDL only
extern const std::vector<std::unordered_map<int32_t, ScenarioFeatures>>  gCustomerIspHidlScenarioFeaturesMaps;

#endif /* _CAMERA_CUSTOM_FEATURE_TABLE_HEADER_H_ */


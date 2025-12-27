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

#ifndef INCLUDE_MTKCAM_INTERFACES_DEF_SENSORMAP_H_
#define INCLUDE_MTKCAM_INTERFACES_DEF_SENSORMAP_H_

#include <unordered_map>
//
namespace NSCam {

/**
 *  It is used for storing data of sensor.
 */
using SensorId_T = uint32_t;
template<typename Value_T>
using SensorMap = std::unordered_map<uint32_t, Value_T>;

}   // namespace NSCam

#endif  // INCLUDE_MTKCAM_INTERFACES_DEF_SENSORMAP_H_


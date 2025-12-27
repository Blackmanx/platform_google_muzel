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
#ifndef INCLUDE_MTKCAM_INTERFACES_UTILS_SCENARIORECORDER_SCENARIORECORDERDEF_H_
#define INCLUDE_MTKCAM_INTERFACES_UTILS_SCENARIORECORDER_SCENARIORECORDERDEF_H_

#include <mtkcam-interfaces/utils/metadata/IMetadata.h>
#include <cstdint>
#include <string>

namespace NSCam {
namespace TuningUtils {
namespace scenariorecorder {

enum DecisionType {
  DECISION_UNKNOW = 0,
  DECISION_ATMS = 1,
  DECISION_TOP_CONTROL = 2,
  DECISION_NORMAL_DATA_DUMP = 3,
  DECISION_AE_EXPOSURE_PARAM = 4,
  DECISION_FEATURE = 5,
  DECISION_ISP_FLOW = 6,
};

struct DebugSerialNumInfo {
  int32_t uniquekey;  // Must
  int32_t reqNum;  // Must
  int32_t frameNum;  // Option
  int32_t magicNum;  // Option

  DebugSerialNumInfo(void) noexcept
    : uniquekey(-1),
    reqNum(-1),
    frameNum(-1),
    magicNum(-1)
    {}
  DebugSerialNumInfo(const DebugSerialNumInfo& other) = default;
  DebugSerialNumInfo& operator=(const DebugSerialNumInfo& other) = default;
};

struct DecisionParam {
  DebugSerialNumInfo dbgNumInfo;
  int32_t sensorId;
  DecisionType decisionType;
  uint32_t moduleId;  // Define ULOG ModuleIdEnum

  DecisionParam(void) noexcept
      : sensorId(-1),
        decisionType(DECISION_UNKNOW),
        moduleId(0) {}
};

struct ResultParam {
  DebugSerialNumInfo dbgNumInfo;
  int32_t sensorId;
  DecisionType decisionType;
  int32_t stageId;    // EProfileMappingStages
  uint32_t moduleId;  // Define ULOG ModuleIdEnum
  bool writeToHeadline;

  ResultParam(void) noexcept
      : sensorId(-1),
        decisionType(DECISION_UNKNOW),
        stageId(-1),
        moduleId(-1),
        writeToHeadline(false) {}
};

struct UserStaticInfo {
  int32_t sensorId = 0;
  uint32_t moduleId = 0;  // Define ULOG ModuleIdEnum
};

struct DecisionInput {
  UserStaticInfo staticInfo = {};
  // Control parameters
  int32_t magicNum = -1;  // Option
  DecisionType decisionType = DECISION_UNKNOW;
};

struct ExecResultInput {
  UserStaticInfo staticInfo = {};
  // Control parameters
  int32_t magicNum = -1;  // Option
  DecisionType decisionType = DECISION_UNKNOW;
  int32_t stageId = -1;
  bool writeToHeadline = false;
};

}  // namespace scenariorecorder
}  // namespace TuningUtils
}  // namespace NSCam

#endif  // INCLUDE_MTKCAM_INTERFACES_UTILS_SCENARIORECORDER_SCENARIORECORDERDEF_H_

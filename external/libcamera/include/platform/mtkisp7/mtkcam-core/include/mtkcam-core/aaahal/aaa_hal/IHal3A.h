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
#ifndef INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_IHAL3A_H_
#define INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_IHAL3A_H_

#include <cstdint>

#include "mtkcam-core/aaahal/aaa_hal/aaa_hal_def.h"

namespace mtk {
namespace hal3a {

struct ResultEvent {
  // Flag
  bool force_update = false;
  bool camsys_change = false;
  // Data
  mtk_camsys_info camsys_info = {};
};
namespace v1_0 {

class IHal3A {
 public:
  virtual ~IHal3A() {}

 public:
  // Life cycle
  virtual int32_t Init(const mtk_3a_init& init) = 0;
  virtual int32_t Config(const mtk_3a_config& config) = 0;
  virtual int32_t Start(const mtk_3a_start& start) = 0;
  virtual int32_t Stop(const mtk_3a_stop& stop) = 0;
  virtual int32_t Uninit(const mtk_3a_uninit& uninit) = 0;
  // Per frame
  virtual int32_t SetParam(const mtk_3a_param& param) = 0;
  virtual int32_t DoCalculation(const mtk_3a_request& request) = 0;
  virtual int32_t GetResult(mtk_3a_result& result) = 0;
  // AF per frame
  virtual int32_t SetParamAF(const mtk_3a_param& param) = 0;
  virtual int32_t DoCalculationAF(const mtk_af_request& request) = 0;
  virtual int32_t GetResultAF(mtk_lens_result& result) = 0;

  // On-Demand API
  virtual int32_t GetHwInitialSetting(const mtk_3a_config& config,
                                      mtk_hw_initial_setting& result) = 0;
  virtual int32_t GetResultOfCamsysChange(const mtk_camsys_info& info,
                                  mtk_hal3a_setting* p_hal3a_setting) = 0;
  virtual int32_t GetResultForceUpdate(mtk_3a_result& result) = 0;
  // multi-cam sync module to update data
  virtual int32_t Set2aDataToLastPool() = 0;
  virtual int32_t Set2aDataToPool(const mtk_3a_request& request) = 0;

  // For CCT
  virtual int32_t Enable3ASetParam(const bool enable) = 0;
  virtual int32_t GetSensorStaticInfo(NSCam::SensorStaticInfo* info) = 0;
};
}  // namespace v1_0

namespace v2_0 {
class IHal3A {
 public:
  virtual ~IHal3A() {}

 public:
  // Non per frame
  virtual int32_t Config(const mtk_3a_config& config) = 0;
};
}  // namespace v2_0

class IHal3A : public v1_0::IHal3A, v2_0::IHal3A {
 public:
  virtual ~IHal3A() {}

 public:
  static IHal3A* GetInstance(const uint32_t idx);
  static int32_t GetStaticInfo(const uint32_t idx,
                              mtk_hal3a_static_info& info);
  using v1_0::IHal3A::Init;
  using v1_0::IHal3A::Config;
  using v1_0::IHal3A::Start;
  using v1_0::IHal3A::Stop;
  using v1_0::IHal3A::Uninit;
  using v1_0::IHal3A::SetParam;
  using v1_0::IHal3A::DoCalculation;
  using v1_0::IHal3A::GetResult;
  using v1_0::IHal3A::SetParamAF;
  using v1_0::IHal3A::DoCalculationAF;
  using v1_0::IHal3A::GetResultAF;
  using v1_0::IHal3A::GetHwInitialSetting;
  using v1_0::IHal3A::GetResultOfCamsysChange;
  using v1_0::IHal3A::GetResultForceUpdate;
  using v1_0::IHal3A::Enable3ASetParam;
  using v1_0::IHal3A::GetSensorStaticInfo;

  using v2_0::IHal3A::Config;
};

}       // namespace hal3a
}       // namespace mtk
#endif  // INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_IHAL3A_H_

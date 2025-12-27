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

/**
 * @file IHal3ACb.h
 * @brief Declarations of Abstraction of 3A Hal Callback Class and Top Data
 * Structures
 */

#ifndef AAA_AAAHAL_INCLUDE_AAA_HAL_PRIVATE_H_
#define AAA_AAAHAL_INCLUDE_AAA_HAL_PRIVATE_H_

#include <mtkcam-core/aaahal/aaa_hal/aaa_hal_def.h>
#include "Hal3AFrameInfo.h"

// std lib
#include <stdint.h>
#include <memory>

using mtk::hal3a::MtkFlashType::kNoFlash;
using mtk::hal3a::MtkFlashStatus::kStillOff;
using mtk::hal3a::MtkSttBufFlashState::kFlashOff;

namespace NS3Av3 {
#define SENSOR_IDX_MAX (20)
template <typename T>
struct INST_T {
  std::once_flag onceFlag;
  std::unique_ptr<T> instance;
};
}   // namespace NS3Av3

namespace mtk {
namespace hal3a {
namespace v1_0 {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//              customize structure
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//              private structure
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct mtk_task3a_request {
  mtk_3a_request* p_3a_request;
  mtk_af_request* p_af_request;
  mtk_3a_param* p_3a_param;
  uint64_t request_id;
  uint64_t stats_number;
  bool is_hqc;
};

struct mtk_task3a_result {
  mtk_3a_result* p_3a_result;
  mtk_lens_result* p_af_result;
};

struct mtk_thread_request {
  mtk_task3a_request task3a_request;
  mtk_task3a_result task3a_result;
  std::shared_ptr<mtk_3a_frame_info> p_frame_info;
};

}       // namespace v1_0
}       // namespace hal3a
}       // namespace mtk
#endif  // AAA_AAAHAL_INCLUDE_AAA_HAL_PRIVATE_H_

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
#ifndef INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_CTRL_IHAL3ALISTENER_H_
#define INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_CTRL_IHAL3ALISTENER_H_

#include <memory>
#include <vector>
#include <map>

// Harvey: don't know why this is needed. Commented it out.
// #include "mtkcam-core/aaahal/aaa_hal_ctrl/hal3a_stt_info.h"

namespace mtk {
namespace hal3a {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IHal3AListener {
 public:
  enum E_NOTIFY_ID_T { eID_BEGIN = 0, eID_NUM };

 public:
  virtual ~IHal3AListener() {}

 public:
  virtual void notifySettingReady(uint64_t request_id) = 0;
  virtual void notifySttBufDone(
      const std::map<uint64_t, uint32_t>& request_id_map) = 0;
  virtual void notify(E_NOTIFY_ID_T e_id,
                      intptr_t i4_arg1,
                      intptr_t i4_arg2,
                      intptr_t i4_arg3) = 0;
};

}       // namespace hal3a
}       // namespace mtk
#endif  // INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_CTRL_IHAL3ALISTENER_H_

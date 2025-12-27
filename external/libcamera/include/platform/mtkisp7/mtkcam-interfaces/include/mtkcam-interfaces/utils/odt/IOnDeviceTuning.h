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
#ifndef INCLUDE_MTKCAM_INTERFACES_UTILS_ODT_IONDEVICETUNING_H_
#define INCLUDE_MTKCAM_INTERFACES_UTILS_ODT_IONDEVICETUNING_H_

// std lib
#include <memory>

#include "mtkcam-interfaces/utils/ndd/ndd_autogen_def.h"

namespace NSCam {
namespace TuningUtils {

class IOdtUtils {
 public:
  static std::shared_ptr<IOdtUtils> getInstance(uint32_t sensor_idx);
  virtual ~IOdtUtils() {}

 public:
   /**
   * @brief top control for readback function
   */
  virtual bool is_enable() = 0;

  /**
   * @brief top control for module's readback function
   * @param [in] module is the name of module
   *        [in] stage is defined by ATMS
   *        [in] layer is defined for multi-run pass2
   *        [in] action is defined for multi-path pass2,
   *             caller has to give it to enumeration EAction_T.
   */
  virtual bool is_enable(eModule module,
                         int32_t stage,
                         int32_t layer = -1,
                         int32_t action = -1) = 0;

  /**   * @brief top control by per-frame for module's readback function
  * @param [in] module is the name of module
  *        [in] frame_id is frame id of pipeline framework
  *        [in] stage is defined by ATMS
  *        [in] layer is defined for multi-run pass2
  *        [in] action is defined for multi-path pass2,
  *             caller has to give it to enumeration EAction_T.
  */
  virtual bool is_enable(uint32_t frame_id,
                         eModule module,
                         int32_t stage,
                         int32_t layer = -1,
                         int32_t action = -1) = 0;

  /**
   * @brief notify preview start
   */
  virtual void stream_on() = 0;

  /**
   * @brief notify preview stop
   */
  virtual void stream_off() = 0;

  /**
   * @brief notify begin timing of frame with frame id
   * @param [in] frame_id is frame id of pipeline framework
   */
  virtual void frame_begin(uint32_t frame_id) = 0;

  /**
   * @brief notify the end timing of frame with frame id
   * @param [in] frame_id is frame id of pipeline framework
   */
  virtual void frame_end(uint32_t frame_id) = 0;
};

}  // namespace TuningUtils
}  // namespace NSCam
#endif  // INCLUDE_MTKCAM_INTERFACES_UTILS_ODT_IONDEVICETUNING_H_

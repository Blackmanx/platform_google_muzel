/*
 * Copyright (C) 2020 MediaTek Inc.
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

#ifndef INCLUDE_MTKCAM_CORE_HW_CAMSYS_PIPE_MGR_METADATA_H_
#define INCLUDE_MTKCAM_CORE_HW_CAMSYS_PIPE_MGR_METADATA_H_

#include <mtkcam-core/aaahal/ae_mgr/ae_setting.h>

namespace NSCam {
namespace camsys {
namespace pipemgr {

/**
 * Sensor test pattern data
 *
 * Config together with test_pattern and all RGGB values are
 * valid when >= 0.
 *
 */
struct mtk_sensor_test_pattern_data {
  bool valid = false;
  int32_t Channel_R = -1;
  int32_t Channel_Gr = -1;
  int32_t Channel_Gb = -1;
  int32_t Channel_B = -1;
};

/**
 * Sensor driver control setting
 *
 * @exposure: gain/shutter control set, the vector order complies with sensor
 *              when multiple exposure is required.
 *            it is possible to have 7 or more exposure numbers and the order
 *              is from longer to shorter like: LLLE,LLE,LE,NE,SE,SSE,SSSE.
 * @frame_rate: sensor maximum frame rate
 * @flicker_adjustment: flicker adjustment switch
 * @sensor_scenario: sensor scenario default -1 means no scenario assigned
 * @extend_frame_length: use when doing seamless switch, act only when > 0
 * @test_pattern: sensor test pattern, valid when >= 0
 * @test_pattern_data: sensor test pattern color data
 * @hint_only: true for hint camsys pipeline only, false(default) then set
 *            to sensor
 */
struct mtk_sensor_driver_control {
  ae_exposure_setting_table exposure;
  uint32_t frame_rate = 0;
  bool flicker_adjustment = false;
  int32_t sensor_scenario = -1;
  int32_t extend_frame_length = 0;
  int32_t test_pattern = -1;
  bool fus_only = false;
  struct mtk_sensor_test_pattern_data test_pattern_data;
  bool hint_only = false;
};

struct mtk_flash_driver_control {
  uint32_t device_id = 0;
  uint32_t enable = 0;
  uint32_t mode = 0;
  uint32_t pulse_number = 0;
  uint32_t offset = 0;
  uint32_t high_width = 0;
  uint32_t low_width = 0;
};

struct mtk_cam_front_end_setting {
  struct mtk_sensor_driver_control sensor_setting;
  struct mtk_flash_driver_control flash_setting;
};

struct MetaFormat {
  // statistic DMA
  uint32_t dataformat;
  uint32_t buffersize;
};

}  // namespace pipemgr
}  // namespace camsys
}  // namespace NSCam

#endif  // INCLUDE_MTKCAM_CORE_HW_CAMSYS_PIPE_MGR_METADATA_H_

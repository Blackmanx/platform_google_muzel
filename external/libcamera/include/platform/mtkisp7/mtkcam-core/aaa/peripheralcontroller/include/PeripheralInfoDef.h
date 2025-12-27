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
#ifndef AAA_PERIPHERALCONTROLLER_INCLUDE_PERIPHERALINFODEF_H_
#define AAA_PERIPHERALCONTROLLER_INCLUDE_PERIPHERALINFODEF_H_

// C
#include <stdint.h>

// C++
#include <array>
#include <memory>
#include <vector>

// Other
#include "mtkcam-interfaces/hw/sensor/IHalSensor.h"

static const uint32_t kMaxSensorCnt = 10;
static const uint32_t kMaxGainTableSize = 1025;

namespace mtk {
namespace hal3a {

struct GyroMvResult {
  std::shared_ptr<std::vector<uint8_t>> mv;
  uint32_t mv_width;
  uint32_t mv_height;
  uint32_t img_w;
  uint32_t img_h;
};

struct OisMvResult {
  uint64_t frame_timestamp;
  std::shared_ptr<std::vector<double>> ois_position_x;
  std::shared_ptr<std::vector<double>> ois_position_y;
};

struct SensorStaticInfo {
  uint32_t index;
  uint32_t dev_id;
  uint32_t sensor_id;
  uint32_t module_id;
  uint32_t orientation;
  NSCam::SensorStaticInfo info;
};

enum SensorOrientation {
  kRear = 0,
  kFront,
};

struct BySensorModeInfo {
  // SENSOR_CMD_GET_BINNING_TYPE
  uint32_t bin_sum_ratio = 1;
  //  SENSOR_CMD_GET_GAIN_RANGE_BY_SCENARIO
  uint32_t min_gain;
  uint32_t max_gain;
  //  SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY
  bool is_sensor_mode_support_pd;
  //  SENSOR_CMD_GET_SENSOR_PDAF_INFO
  SET_PD_BLOCK_INFO_T pd_blk_info;
  //  SENSOR_CMD_GET_SENSOR_VC_INFO2
  NSCam::SensorVCInfo2 vc_info_2;
  //  SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO
  SensorCropWinInfo crop_info;
  //
  uint32_t min_time_ns;
  uint32_t line_time;
  uint32_t shutter_step;
  uint32_t fine_integ_line;
  // SENSOR_CMD_GET_STAGGER_MAX_EXP_TIME
  uint32_t max_me_time_us;
  uint32_t max_se_time_us;
  // SENSOR_CMD_GET_EXPOSURE_MARGIN_BY_SCENARIO
  uint32_t exp_margin;
  // SENSOR_CMD_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO
  uint32_t pixel_clock;
  // SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM_BY_SCENARIO
  uint32_t line_length;
  //  SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER
  uint32_t rolling_shutter;
};

struct SensorInitialDynamicInfo {
  //  SENSOR_CMD_GET_BASE_GAIN_ISO_AND_STEP
  uint32_t gain_iSO;
  uint32_t gain_step_unit;
  uint32_t gain_type = 16;
  //  SENSOR_CMD_GET_ANA_GAIN_TABLE
  uint32_t real_table_size;
  std::array<uint32_t, kMaxGainTableSize> gain_table;
  // info query from all sensor mode
  std::array<BySensorModeInfo, SENSOR_SCENARIO_ID_MAX>
      by_sensor_mode_info;
};

struct SensorConfigDynamicInfo {};

struct SensorPerframeDynamicInfo {
  NSCam::SensorDynamicInfo info;
  uint32_t temperature_value;
  //  SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM
  uint32_t period;
  //  SENSOR_CMD_GET_SENSOR_PDAF_REG_SETTING
  uint32_t pd_reg_size;
  uint16_t* p_pd_register;
  //  SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM
  uint32_t pixels_in_line;
};

}       // namespace hal3a
}       // namespace mtk
#endif  // AAA_PERIPHERALCONTROLLER_INCLUDE_PERIPHERALINFODEF_H_

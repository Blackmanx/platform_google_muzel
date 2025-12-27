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
#ifndef AAA_AAAHAL_INCLUDE_PUBLIC_AAA_HAL_COMMON_H_
#define AAA_AAAHAL_INCLUDE_PUBLIC_AAA_HAL_COMMON_H_

// C
#include <stdint.h>

// C++
#include <array>

// Other
#include "mtkcam-interfaces/hw/sensor/IHalSensor.h"
#include "isp_tuning/isp_tuning.h"

static const uint32_t kMax3ARegionsCount = 9;

namespace mtk {
namespace hal3a {

enum Mtk3ABitMode {
  kBitMode_10Bit = 0,
  kBitMode_12Bit,
  kBitMode_14Bit,
  kBitMode_16Bit
};

struct ori {
  int32_t sensor_orientation;
  uint8_t facing;
};

struct mtk_3a_area {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
  int32_t weight;
  mtk_3a_area() : left(0), top(0), right(0), bottom(0), weight(0) {}
};

struct mtk_3a_regions {
  mtk_3a_area areas[kMax3ARegionsCount];
  uint32_t count;
};

struct mtk_3a_exif {
  int32_t f_number;            // Format: F2.8 = 28
  int32_t focal_length;        // Format: FL 3.5 = 350
  int32_t awb_mode;            // White balance mode
  int32_t light_source;        // Light Source mode
  int32_t exposure_program;    // Exposure Program
  int32_t scene_cap_type;      // Scene Capture Type
  int32_t flashlight_time_us;  // Strobe on/off
  int32_t ae_meter_mode;       // Exposure metering mode
  int32_t ae_exp_bias;         // Exposure index*10
  int32_t exposure_time;       // Exposure time
  int32_t iso_speed;           // ISO value
  int32_t ae_brightness_value;
};

struct shadingConfig_T {
  MUINT32 AAOstrideSize;
  MUINT32 AAOBlockW;
  MUINT32 AAOBlockH;

  MUINT32 u4HBinWidth;
  MUINT32 u4HBinHeight;

  MINT32 i4BinWidth;
  MINT32 i4BinHight;
};

}       // namespace hal3a
}       // namespace mtk
#endif  // AAA_AAAHAL_INCLUDE_PUBLIC_AAA_HAL_COMMON_H_

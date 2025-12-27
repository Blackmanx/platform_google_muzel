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
#ifndef INCLUDE_MTKCAM_INTERFACES_AAAHAL_AAA_HAL_CTRL_ISYNC3ADATA_H_
#define INCLUDE_MTKCAM_INTERFACES_AAAHAL_AAA_HAL_CTRL_ISYNC3ADATA_H_

#define SYNC_FOV_ALIGN_SENSOR_NUM 10

#include <vector>

namespace mtk {
namespace hal3a {
/*****************************************************************************
 * Sync3A Enum
 *****************************************************************************/
enum Mtk3ASync2AMode {
  kSYNC2A_MODE_OFF = 0,
  kSYNC2A_MODE_SYNC = 1,   // by frame
  kSYNC2A_MODE_ALIGN = 2,  // align master data
};

enum Mtk3ASyncAfMode {
  kSYNCAF_MODE_IDLE = 0,
  kSYNCAF_MODE_ON = 1,
  kSYNCAF_MODE_OFF = 2,
};
enum Mtk3AHwFrmSyncMode {
  kHW_FRM_SYNC_MODE_IDLE = 0,
  kHW_FRM_SYNC_MODE_ON = 1,
  kHW_FRM_SYNC_MODE_OFF = 2,
};
enum Mtk3ASync3ANotify {
  kSYNC3A_NOTIFY_NONE = 0,
  kSYNC3A_NOTIFY_SWITCH_ON = 1,
};
enum Mtk3AMultiCamFeatureMode {
  kMULTI_CAM_FEATURE_MODE_ZOOM = 0,
  kMULTI_CAM_FEATURE_MODE_VSDOF,
  kMULTI_CAM_FEATURE_MODE_DENOISE,
};

enum Mtk3ASync3AScenario {
  kSYNC3A_SCENARIO_OFFLINE_SYNC = (1<<0),
  kSYNC3A_SCENARIO_ONLINE_SYNC = (1<<1),
  kSYNC3A_SCENARIO_SUBSAMPLE_SYNC_INFO = (1<<2),
  kSYNC3A_SCENARIO_LAGGING_LAUNCH = (1<<3),
  kSYNC3A_SCENARIO_RESUME = (1<<4),
  kSYNC3A_SCENARIO_LOW_TO_HIGH_FPS = (1<<5)
};

/*****************************************************************************
 * SAT FOV Alignment Parameter
 *****************************************************************************/

typedef struct {
  uint32_t master_idx;
  uint32_t slave_idx;

  // Prview alignment result (For AE/AWB Sync)
  uint32_t master_PV_left;
  uint32_t master_PV_top;
  uint32_t master_PV_right;
  uint32_t master_PV_bottom;
  uint32_t master_sensor_width;
  uint32_t master_sensor_height;
  int32_t master_mag_num;

  uint32_t slave_PV_left;
  uint32_t slave_PV_top;
  uint32_t slave_PV_right;
  uint32_t slave_PV_bottom;
  uint32_t slave_sensor_width;
  uint32_t slave_sensor_height;
  int32_t slave_mag_num;

  // ROI alignment result (For AF Sync)
  uint32_t ROI_left;
  uint32_t ROI_top;
  uint32_t ROI_right;
  uint32_t ROI_bottom;

  // 3rd Party info(TBD)
  bool en_third_party;    // Is 3rd Party FOV Alignment
  int32_t zoom_ratio;     // 3rd Party Zoom Ratio
}fov_align_result;

typedef struct {
  std::vector<fov_align_result*> fov_result;
}fov_align_output;

typedef struct {
    int32_t thr_l;
    int32_t thr_h;
}thr_low_high;

typedef struct {
    thr_low_high dist_thr;
    thr_low_high fov_ratio;
}sat_sensor_calibration_info;

typedef struct {
    thr_low_high dist_thr;
    thr_low_high dispa_shiftx;
    thr_low_high dispa_shifty;
    thr_low_high dispa_zoomrat;
}sat_sensor_calibration_disparity_shift;

typedef struct {
    sat_sensor_calibration_info cali_info[SYNC_FOV_ALIGN_SENSOR_NUM];
    sat_sensor_calibration_disparity_shift
      cali_shift[SYNC_FOV_ALIGN_SENSOR_NUM][SYNC_FOV_ALIGN_SENSOR_NUM];
}sat_fov_align_param;

}       // namespace hal3a
}       // namespace mtk
#endif  // INCLUDE_MTKCAM_INTERFACES_AAAHAL_AAA_HAL_CTRL_ISYNC3ADATA_H_

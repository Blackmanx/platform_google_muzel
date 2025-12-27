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
 * @file Sync3ADataDef.h
 * @brief Declarations of sync3A data define
 */
#ifndef AAA_AAAHAL_SOURCE_HAL3A_SYNC3A_WRAPPER_SYNC3ADATADEF_H_
#define AAA_AAAHAL_SOURCE_HAL3A_SYNC3A_WRAPPER_SYNC3ADATADEF_H_

// feature define
#include <mtkcam-interfaces/aaahal/aaa_hal_ctrl/ISync3AData.h>

// std lib
#include <time.h>
#include <array>
#include <mutex>
#include <unordered_map>
#include <vector>

/* (unit:us) */
#define GETTIMESTAMP(time)                                                \
  {                                                                       \
    struct timespec t;                                                    \
    uint64_t timestamp;                                                   \
                                                                          \
    t.tv_sec = t.tv_nsec = 0;                                             \
    timestamp = 0;                                                        \
    clock_gettime(CLOCK_MONOTONIC, &t);                                   \
    timestamp = (uint64_t)((t.tv_sec) * 1000000000LL + t.tv_nsec) / 1000; \
    time = timestamp;                                                     \
  }

namespace mtk {
namespace hal3a {

enum Mtk3ASync3ASupport {
  kSYNC3A_SUPPORT_AE = (1 << 0),
  kSYNC3A_SUPPORT_AWB = (1 << 1),
  kSYNC3A_SUPPORT_AF = (1 << 2)
};

enum Mtk3ASync3ACondition {
  kSYNC3A_CONDITION_NONE = 0,
  kSYNC3A_CONDITION_WAIT = 1,
  kSYNC3A_CONDITION_POST = 2,
  kSYNC3A_CONDITION_FINISH = 3
};

/************ config info ************/
// need p1node provide
// save queue by sensor idx(EX: master/slave1/slave2...)
struct mtk_sync3a_config {
  // Be define by sensor. EX: SENSOR_RAW_MONO or SENSOR_RAW_Bayer
  uint32_t sensor_type;
  // is support af feature
  uint32_t af_support;
  // multi cam feature mode
  uint32_t feature_mode;

  uint32_t Cam_physical_numbers;
  bool is_subsample_mode;

  mtk_sync3a_config()
      : sensor_type(0),
        af_support(true),
        feature_mode(kMULTI_CAM_FEATURE_MODE_ZOOM),
        Cam_physical_numbers(0),
        is_subsample_mode(0) {}
};

/************ Control info ************/
// save queue by sensor idx(EX: master/slave1/slave2...)
struct mtk_sync3a_control_info {
  uint32_t master_idx;
  uint32_t previous_master_idx;
  uint32_t previous_awb_master_idx;
  uint32_t awb_master_idx;
  std::vector<uint32_t> v_slave_idx;
  uint32_t request_number;
  uint32_t sync2a_mode;
  uint32_t syncaf_mode;
  uint32_t synchw_mode;
  uint32_t sync_mode_count;
  // other info
  int32_t zoom_ratio;
  int32_t low_fps;
  std::vector<fov_align_result> fov_result;
  int32_t sync3a_scenario;
  uint64_t time_stamp;

  mtk_sync3a_control_info()
      : master_idx(0),
        previous_master_idx(0),
        previous_awb_master_idx(0),
        awb_master_idx(0),
        request_number(0),
        sync2a_mode(0),
        syncaf_mode(0),
        synchw_mode(0),
        sync_mode_count(0),
        zoom_ratio(0),
        low_fps(0),
        sync3a_scenario(kSYNC3A_SCENARIO_ONLINE_SYNC),
        time_stamp(0) {
    v_slave_idx.clear();
    v_slave_idx.shrink_to_fit();
    fov_result.clear();
    fov_result.shrink_to_fit();
  }

  void clear_slave_idx_vector() {
    v_slave_idx.clear();
    v_slave_idx.shrink_to_fit();
  }

  void clear_fov_result_vector() {
    fov_result.clear();
    fov_result.shrink_to_fit();
  }

  void clear_vector() {
    clear_slave_idx_vector();
    clear_fov_result_vector();
  }
};

struct mtk_sync3a_control_info_queue {
  // save queue by frame number
  std::unordered_map<uint32_t, mtk_sync3a_control_info> map_sync3a_control_info;
};

/************ Sync status ************/
// save queue by frame number
struct mtk_sync3a_sync_status {
  uint32_t request_number;
  // count sync number for sync mode
  uint32_t waiting_count;
  Mtk3ASync3ACondition sync3a_Condition;
  uint64_t time_stamp;

  mtk_sync3a_sync_status()
      : request_number(0),
        waiting_count(0),
        sync3a_Condition(kSYNC3A_CONDITION_NONE),
        time_stamp(0) {}
};

/************ control sync algo data ************/
struct mtk_sync3a_ctrl_algo_info {
  uint32_t opt;
  bool bIsOfflineSync;
  int32_t sync3a_scenario;

  mtk_sync3a_ctrl_algo_info()
      : opt(0),
        bIsOfflineSync(0),
        sync3a_scenario(kSYNC3A_SCENARIO_ONLINE_SYNC) {}
};

}       // namespace hal3a
}       // namespace mtk
#endif  // AAA_AAAHAL_SOURCE_HAL3A_SYNC3A_WRAPPER_SYNC3ADATADEF_H_

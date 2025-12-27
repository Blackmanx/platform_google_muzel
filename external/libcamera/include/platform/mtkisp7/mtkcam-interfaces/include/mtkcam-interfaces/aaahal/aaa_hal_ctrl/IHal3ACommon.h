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
 * @file IHal3ACommon.h
 * @brief Declarations perframe metadata input for both 3a and isp
 * Structures
 */

#ifndef INCLUDE_MTKCAM_INTERFACES_AAAHAL_AAA_HAL_CTRL_IHAL3ACOMMON_H_
#define INCLUDE_MTKCAM_INTERFACES_AAAHAL_AAA_HAL_CTRL_IHAL3ACOMMON_H_

#include "mtkcam-interfaces/utils/metadata/IMetadata.h"
#include "mtkcam-interfaces/utils/metadata/client/mtk_metadata_tag.h"
#include "mtkcam-interfaces/def/ImageFormat.h"

namespace mtk {
namespace hal3a {

enum MtkAeExpMode {
  kExpModeVVLE = 0,
  kExpModeVLE,
  kExpModeLLE,
  kExpModeLE,
  kExpModeNE,
  kExpModeME,
  kExpModeSE,
  kExpModeSSE,
  kExpModeVSE,
  kExpModeVVSE,
  kExpModeMax
};
struct mtk_ae_exposure_setting {
  int32_t mode;
  int32_t ev;
  uint64_t exposure_ns;
  uint32_t exposure_line;
  uint32_t afe_gain;
  uint32_t isp_gain;
  uint32_t iris;
  uint32_t iso;
  uint32_t iso_base;
};
struct mtk_ae_exposure_setting_table{
  uint32_t cnt;
  mtk_ae_exposure_setting table[kExpModeMax];
};
struct mtk_hal3a_ae_info {
  bool is_ae_stable;
  bool is_ae_back_lit;
  int16_t ae_face_diff_index;
  int32_t ae_lv_x10;
  int32_t ae_touch_ev_diff;
  int32_t ae_ev_bar_ev_diff;
  uint32_t ae_dgn_gain;
  uint32_t ae_exposure_time;
  uint32_t ae_isp_gain;
  uint32_t ae_sensor_gain;
  uint32_t ae_iso;
  // HDR Reconfig info
  uint32_t full_ratio;
  uint32_t dr_ratio;
  int32_t bv_value;
  uint32_t converge_state;
  uint32_t bin_sum_ratio;
  bool flicker_active;
  int32_t flicker_50hz_score;
  int32_t flicker_60hz_score;
  int32_t flicker_result;
  int32_t de_flicker;
  int32_t ae_flicker_mode;
  int32_t ae_seamless_smooth_result;
  uint32_t next_le_expo;
  uint32_t next_le_real_iso;
  uint32_t next_me_expo;
  uint32_t next_me_real_iso;
  uint32_t next_se_expo;
  uint32_t next_se_real_iso;
  uint32_t next_see_expo;
  uint32_t next_vse_real_iso;
  uint32_t ae_bv_x10;
  int32_t *seam_tun_para;
  MUINT32 seam_tun_para_len;
  //
  mtk_ae_exposure_setting_table ae_exp_table;
  bool ai_shutter_exist_motion;
  int32_t real_light_value_x10;
};
struct mtk_awb_gain {
  int32_t r_x128;
  int32_t b_x128;
  int32_t r_d65_x128;
  int32_t b_d65_x128;
  int32_t r_cwf_x128;
  int32_t b_cwf_x128;
};
struct mtk_hal3a_awb_info {
  bool is_awb_stable;
  mtk_awb_gain gain;
};
struct mtk_hal3a_af_info {
  bool is_af_support;
  int32_t macro_to_inf_ratio;
  int32_t dac_inf;
  int32_t dac_macro;
  int32_t damping_time;
  int32_t readout_time_us;
  int32_t init_dac;
  int32_t af_table_start;
  int32_t af_table_end;
  int32_t stt_num;
  int32_t set_count;  // 0~2
  int32_t dac_info_from[2];
  int32_t dac_info_to[2];
  int32_t dac_info_percent[2];
  //
  int32_t ref_dac_min;
  int32_t ref_dac_max;
  int32_t ref_cur_dac;
  int32_t ref_posture_dac;
  int32_t ref_is_af_stable;
  int32_t ref_roi_left;
  int32_t ref_roi_top;
  int32_t ref_roi_right;
  int32_t ref_roi_bottom;
  int32_t ref_roi_type;
  //
  int32_t focus_range_min;
  int32_t focus_range_max;
  int32_t zoom_position;
  int32_t zoom_range_min;
  int32_t zoom_range_max;
};

struct mtk_hal3a_pd_info {
  uint32_t map[8];
  uint32_t map_width;
  uint32_t map_height;
  int32_t active_x0;
  int32_t active_y0;
  int32_t active_x1;
  int32_t active_y1;
};

#define NUMBER_OF_AI_FLASH_FRAME 4
#define MAX_SIZE_OF_FLASH_AAO 32*24

struct mtk_hal3a_flash_info {
  int32_t fg_y[NUMBER_OF_AI_FLASH_FRAME];  // MTK_FLASH_FG_Y
  int32_t bg_y[NUMBER_OF_AI_FLASH_FRAME];  // MTK_FLASH_BG_Y
  int32_t nf_map[MAX_SIZE_OF_FLASH_AAO];   // MTK_FLASH_NF_MAP
  int32_t pf_map[MAX_SIZE_OF_FLASH_AAO];   // MTK_FLASH_PF_MAP
};

#define AIS2O_MAX_SIZE (128)
typedef struct ais2o_raw_data
{
    uint32_t size;
    uint8_t data[AIS2O_MAX_SIZE];
} ais2o_raw_data;

struct mtk_hal3a_ai3a_info {
  ais2o_raw_data ais2o;
};

struct mtk_hal3a_info {
  mtk_hal3a_ae_info ae_info = {};
  mtk_hal3a_awb_info awb_info = {};
  mtk_hal3a_af_info af_info = {};
  mtk_hal3a_flash_info flash_info = {};
  mtk_hal3a_ai3a_info ai3a_info = {};
};

struct mtk_hal3a_static_policy {
  bool is_hsf;
};

static const uint32_t base_buf_depth = 2;
struct mtk_hal3a_comm_static_info {
  uint32_t meta_0_buf_depth = base_buf_depth;  // aao
  uint32_t meta_1_buf_depth = base_buf_depth;  // afo
  uint32_t ai3a_buf_depth = base_buf_depth;
  uint32_t camsv_buf_depth = base_buf_depth;
};
struct mtk_hal3a_ae_static_info {};
struct mtk_hal3a_awb_static_info {};
struct mtk_hal3a_af_static_info {};
struct mtk_hal3a_ai3a_static_info {
  int32_t ai3a_enable;
  NSCam::EImageFormat ai3ao_format;
  int32_t ai3ao_depth;
  int32_t aiseg_width;
  int32_t aiseg_height;
  int32_t aiseg_stride_size;
};
struct mtk_hal3a_static_info {
  // For user defined input
  mtk_hal3a_static_policy policy = {};
  // Mgr output
  mtk_hal3a_comm_static_info comm_static_info = {};
  mtk_hal3a_ae_static_info ae_static_info = {};
  mtk_hal3a_awb_static_info awb_static_info = {};
  mtk_hal3a_af_static_info af_static_info = {};
  mtk_hal3a_ai3a_static_info ai3a_static_info = {};
};

namespace v1_0 {

struct mtk_hal3a_metaset {
  bool dummy;
  bool skip_exposure_setting;
  uint64_t dummy_generated_by_request_id = 0xFFFFFFFFFFFFFFFF;
  NSCam::IMetadata appMeta;
  NSCam::IMetadata halMeta;

  mtk_hal3a_metaset() : dummy(0), skip_exposure_setting(false) {}

  mtk_hal3a_metaset(bool _dummy,
                    NSCam::IMetadata _appMeta,
                    NSCam::IMetadata _halMeta)
      : dummy(_dummy), skip_exposure_setting(false), appMeta(_appMeta),
        halMeta(_halMeta) {}

  mtk_hal3a_metaset(NSCam::IMetadata appMeta, NSCam::IMetadata halMeta)
      : mtk_hal3a_metaset(false, appMeta, halMeta) {}
};

}   // namespace v1_0

}       // namespace hal3a
}       // namespace mtk
#endif  // INCLUDE_MTKCAM_INTERFACES_AAAHAL_AAA_HAL_CTRL_IHAL3ACOMMON_H_

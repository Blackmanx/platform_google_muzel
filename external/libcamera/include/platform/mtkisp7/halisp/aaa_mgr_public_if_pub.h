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
#ifndef AAA_COMMON_INCLUDE_AAA_MGR_PUBLIC_IF_H_
#define AAA_COMMON_INCLUDE_AAA_MGR_PUBLIC_IF_H_

struct lsc_sl2_info_t {
  uint32_t i4CenterX;
  uint32_t i4CenterY;
  uint32_t i4R0;
  uint32_t i4R1;
  uint32_t i4R2;
  uint32_t i4Gain0;
  uint32_t i4Gain1;
  uint32_t i4Gain2;
  uint32_t i4Gain3;
  uint32_t i4Gain4;
  uint32_t i4SetZero;
};

struct lsc_isp_info_t {
  bool lscOnOff;
  bool tsfOnOff;
  lsc_sl2_info_t sl2_info;
  uint32_t tsfs_gain;
  uint8_t tsfo[14400];
  uint32_t lscApiMode;
  uint32_t flash_type;
  uint32_t flash_status;
  uint32_t buf_flash_status;
  uint32_t u4AvgY;
  uint32_t u4LuxValue_x10000;
  bool bAELock;
  int32_t focus_range_mac;
  int32_t focus_range_inf;
  int32_t zoom_range_tele;
  int32_t zoom_range_wide;
  int32_t focus_position;
  int32_t zoom_position;
  int32_t gyroData[3];
  uint32_t resultGainTable[1156];
};

//--- AWB ---//
struct awb_static_info {};
struct awb_pre_config_info_t {
  int32_t window_num_x;
  int32_t window_num_y;
};
struct awb_gain_t {
  int32_t i4R;  // R gain
  int32_t i4G;  // G gain
  int32_t i4B;  // B gain
};
struct awb_isp_info_t {
  int32_t cct;                // CCT
  awb_gain_t awbgain_nopref;  // AWB gain without preference
  int32_t sat_ratio;          // add for ISP 6.0
  // for ISP tuning
  awb_gain_t current_awbgain;  // Current AWB gain
  int32_t fluorescent_index;   // Fluorescent index
  // for ResultPool
  awb_gain_t pregain1;  // AA pregain1
  int32_t csc_ccm[9];   // AA CCM
  // for paramctrl_per_frame
  awb_gain_t rpg;    // RPG
  awb_gain_t pgn;    // PGN
  int32_t awb_lock;  // CCM
  // for AI3A
  int32_t debug_info[39];
};

#endif  // AAA_COMMON_INCLUDE_AAA_MGR_PUBLIC_IF_H_

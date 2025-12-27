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
#ifndef AAA_IPC_HAL3A_INC_HAL3APARAM_V1_0_H_
#define AAA_IPC_HAL3A_INC_HAL3APARAM_V1_0_H_

// Commented out by Google.
// #include <Hal3AParam.h>

#include <IPCCommon.h>
#include <aaa_hal/IHal3A.h>
#include "mtkcam-interfaces/aaahal/aaa_hal_ctrl/IHal3ACommon.h"
#include <mtkcam-interfaces/utils/hw/faces.h>
#include <peripheraldriver/lens/vcm_drv.h>
// #ifndef MTKCAM_ISP7_DEV
// #include <kernel_header/mtk_cam_metabuf.h>
// #else
#include <mtk_cam_metabuf.h>
// #endif
#include <stddef.h>
#include <algorithm>
#include <vector>
using mtk::hal3a::v1_0::IrisInformation;
using NSIspTuning::ILscTable;
namespace mtk {
namespace ipc {
namespace hal3a_v1_0 {

#ifdef HAL_3A_VERSION_V_1_0
#endif

#define MTK_SHADING_MAP_INFO_MAX_NUM 32
#define MTK_AREA_INFO_MAX_NUM 32
#define MTK_SENINF_CONFIG_INFO_MAX_NUM 8
#define MTK_SENSOR_VC_INFO_MAX_NUM 8
#define MTK_AF_ASSIST_PD_REG_MAX_SIZE 1024 /* bytes */
#define MTK_AF_ASSIST_PD_REG_MAX_NUM \
  (MTK_AF_ASSIST_PD_REG_MAX_SIZE / sizeof(uint16_t))
#define MTK_AI3A_RESULT_REQUEST_ID_MAX_NUM 6
#define MTK_AE_EXP_LEVEL_NUM 10
#define MTK_IRIS_DELTA_EV_LUT_SIZE_X 100
#define MTK_IRIS_DELTA_EV_LUT_SIZE_Y 100
#define MTK_AE_SEAMLESS_TUNING_PARA_SIZE 64
#define MAX_ALS_DATA_SIZE 5
#define MAX_AE_CUSTOM_METERING_TABLE_SIZE 256  /* 16x16 */
#define MTK_MAX_PLINE_ANCHOR 30
#define MTK_AE_CLUSIVE_ROI_SIZE 12288  /* 128x96 */
#define MTK_MANUAL_AREA_ROI_SIZE 5

using ShmMemInfo = NSCam::ipc::wrapper::ShmMemInfo;
using ShmMemBase = NSCam::ipc::wrapper::ShmMemBase;

struct Buffer {
  int fd;
  int32_t handle;

  void set(const int32_t inHandle, const int inFd);
  void get(int32_t& outHandle);
};

struct mtk_3a_fast_switch_param {
  int32_t sensor_mode;
  NSCam::MSize tg_size;
  NSCam::MSize full_tg_size;
  int32_t ae_target_mode_next;
  int32_t ae_valid_exp_next;
  uint32_t ae_sensor_mode_next;
  bool is_seamless;
  uint8_t seam_policy;
};

struct mtk_hal3a_config {
  uint32_t subsample_count;
  uint32_t request_count;
  uint32_t sensor_mode;
  uint32_t sensor_dev;
  uint32_t sensor_id;
  uint32_t sensor_tg_width;
  uint32_t sensor_tg_height;
  uint32_t bit_mode;

  mtk_hal3a_config& operator=(const mtk::hal3a::v1_0::mtk_hal3a_config& config);
  void copyTo(mtk::hal3a::v1_0::mtk_hal3a_config& config);
};

struct SensorDynamicInfo {
  uint32_t TgInfo;     // TG_NONE,TG_1,...
  uint32_t pixelMode;  // ONE_PIXEL_MODE, TWO_PIXEL_MODE, FOUR_PIXEL_,MODE
  uint32_t
      TgVR1Info;  // CAM_TG_1, CAM_TG_2, CAM_TG_3, CAM_TG_4, CAM_TG_5, CAM_TG_6
  uint32_t TgVR2Info;
  uint32_t TgCLKInfo;  // Unit : Khz
  uint32_t HDRInfo;
  uint32_t PDAFInfo;
  uint32_t HDRPixelMode;  // ONE_PIXEL_MODE, TWO_PIXEL_MODE, FOUR_PIXEL_,MODE
  uint32_t PDAFPixelMode;
  uint32_t CamInfo[NSCam::HDR_DATA_MAX_NUM];
  uint32_t PixelMode[NSCam::HDR_DATA_MAX_NUM];

  NSCam::SeninfConfigInfo config_infos[MTK_SENINF_CONFIG_INFO_MAX_NUM];
  uint32_t config_infos_size;

  SensorDynamicInfo& operator=(const NSCam::SensorDynamicInfo& info);
  void copyTo(NSCam::SensorDynamicInfo& info);
};

struct SensorVCInfo2 {
  MUINT16 VC_Num;
  MUINT16 VC_PixelNum;
  MUINT16 ModeSelect;    // 0: auto mode, 1:direct mode
  MUINT16 EXPO_Ratio;    // 1/1, 1/2, 1/4, 1/8
  MUINT16 ODValue;       // OD Vaule
  MUINT16 RG_STATSMODE;  // STATS divistion mdoe 0: 16x16, 1:8x8, 2:4x4, 3:1x1
  SINGLE_VC_INFO2 vcInfo2s[MTK_SENSOR_VC_INFO_MAX_NUM];

  uint32_t vcInfo2sSize;

  SensorVCInfo2& operator=(const NSCam::SensorVCInfo2& info);
  void copyTo(NSCam::SensorVCInfo2& info);
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
  //  Sensor VC info, SENSOR_CMD_GET_SENSOR_VC_INFO
  NSCam::SensorVCInfo vc_info;
  //  SENSOR_CMD_GET_SENSOR_VC_INFO2
  SensorVCInfo2 vc_info_2;
  //  SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO
  SensorCropWinInfo crop_info;
  //
  uint32_t min_time_ns;
  uint32_t line_time;
  uint32_t shutter_step;
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
  uint32_t tline;
  uint32_t vsize;

  BySensorModeInfo& operator=(const mtk::hal3a::BySensorModeInfo& info);
  void copyTo(mtk::hal3a::BySensorModeInfo& info);
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

  SensorInitialDynamicInfo& operator=(
      const mtk::hal3a::SensorInitialDynamicInfo& info);
  void copyTo(mtk::hal3a::SensorInitialDynamicInfo& info);
};

struct SensorPerframeDynamicInfo {
  SensorDynamicInfo info;
  uint32_t pixel_clk_freq;
  uint32_t period;
  uint32_t pd_reg_size;
  uint16_t p_pd_register[MTK_AF_ASSIST_PD_REG_MAX_NUM];
  uint32_t pixels_in_line;

  bool pd_buf_valid;

  SensorPerframeDynamicInfo();
  SensorPerframeDynamicInfo& operator=(
      const mtk::hal3a::SensorPerframeDynamicInfo& sensorInfo);
  void copyTo(mtk::hal3a::SensorPerframeDynamicInfo& sensorInfo);
};

struct SensorStaticInfoMap {
  uint32_t sensor_idx;
  mtk::hal3a::SensorStaticInfo sensor_info;
};

struct mtk_3a_init {
  std::array<mtk::hal3a::SensorStaticInfo, kMaxSensorCnt>
      sensor_static_info_array;
  SensorInitialDynamicInfo sensor_init_dynamic_info;
  /************ sensor info  ************/
  uint32_t gyro_mv_size;
  uint32_t gyro_num_size;
  bool is_vcm_support;
  bool is_ircut_support;
  bool is_ozoom_support;
  bool flash_hw_support;
  strobe_drv_cap flash_capability;
  CAM_CAL_DATA_STRUCT cal_data;
  CAM_CAL_2A_DATA_STRUCT cal_aa;
  CAM_CAL_LSC_DATA_STRUCT cal_lsc;
  CAM_CAL_PDAF_DATA_STRUCT cal_pdaf;

  mtk_3a_init& operator=(const mtk::hal3a::v1_0::mtk_3a_init& init);
  void copyTo(mtk::hal3a::v1_0::mtk_3a_init& init);
};

struct mtk_3a_config {
  std::array<mtk::hal3a::SensorStaticInfo, kMaxSensorCnt> sensor_static_info_array;
  SensorInitialDynamicInfo sensor_init_dynamic_info;
  mtk::ipc::hal3a_v1_0::mtk_hal3a_config control_config;
  uint32_t u4MIN_SENSITIVITY;
  // V4L2 param
  /************ Common ************/
  uint32_t sensor_idx;
  uint32_t sensor_mode;
  bool is_subsample_mode;
  uint8_t capture_intent = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
  int64_t capture_feature;
  /************ Tuning Info ************/
  int64_t tuning_feature;
  int64_t sensor_feature;
  int32_t custom_feature;
  int64_t tuning_feature_cap;
  int32_t custom_feature_cap;
  int32_t custom_00;
  int32_t zoom_ratio = 100;
  uint32_t target_size_w;
  uint32_t target_size_h;
  /************ sensor info  ************/
  mtk::hal3a::SensorConfigDynamicInfo sensor_config_dynamic_info;
  SensorPerframeDynamicInfo sensor_perframe_dynamic_info;
  mtk::hal3a::ori orientation;
  /************ sensor info  ************/
  bool sub_flash_enable;
  uint32_t tg_width;
  uint32_t tg_height;
  float fno;
  float focal_length;
  /************ sync3a info  ************/
  int32_t feature_mode;
  /************ ae info  ************/
  int32_t ae_target_mode;
  int32_t ae_min_fps = 5000;
  int32_t ae_max_fps = 30000;
  int32_t ae_valid_exp;
  /************ feature  ************/
  int32_t multiexp_hdr_mode;
  /************ ltm info  ************/
  int32_t isp_fus_num;

  // Additional
  bool bMinSensitivityValid;

  mtk_3a_config& operator=(const mtk::hal3a::v1_0::mtk_3a_config& config);
  void copyTo(mtk::hal3a::v1_0::mtk_3a_config& config);
};

struct Hal3A_Param_IrisInformation {
  /* bit0: 1->move, 0->stop;
   * bit1: 1->arrival, 0->unarrival;
   * bit16~31: current position;
   */
  int32_t iris_status;
  int32_t iris_position;
  int32_t previous_iris_position;
  int64_t moving_timestamp;  // Unit : us
  int64_t previous_moving_timestamp;
  int32_t p_DeltaEV_LUT[MTK_IRIS_DELTA_EV_LUT_SIZE_X]
                       [MTK_IRIS_DELTA_EV_LUT_SIZE_Y];
  float fn_cali;

  // Additional
  bool bDeltaEvLutValid;

  Hal3A_Param_IrisInformation();
  Hal3A_Param_IrisInformation& operator=(const IrisInformation& iris_info);
  void copyTo(IrisInformation& iris_info);
};

struct Hal3A_Param_AEClusiveRoi {
  uint32_t handle;
  uint32_t tableSize;
  int32_t* pTableBuf;

  void setShmInfo(const ShmMemInfo& shmInfo);
  Hal3A_Param_AEClusiveRoi& operator=(
      const std::vector<int32_t>& vecAeClusiveRoi);
  void copyTo(std::vector<int32_t>& vecAeClusiveRoi);
};

struct Hal3A_Param_AEManualareaRoi {
  uint32_t handle;
  uint32_t tableSize;
  int32_t* pTableBuf;

  void setShmInfo(const ShmMemInfo& shmInfo);
  Hal3A_Param_AEManualareaRoi& operator=(
      const std::vector<int32_t>& vecAeManualareaRoi);
  void copyTo(std::vector<int32_t>& vecAeManualareaRoi);
};

struct mtk_3a_param {
  uint64_t request_id;
  uint32_t active_items;
  bool updated;
  // --------- app metadata --------
  // common tag
  uint8_t control_mode;
  uint8_t scene_mode;
  uint8_t capture_intent;
  uint8_t inflight_capture;
  // ae tag
  uint8_t ae_lock;
  uint8_t ae_mode;
  uint8_t ae_precap_trigger;
  uint8_t ae_anti_banding_mode;
  int64_t sensor_frame_duration;
  int64_t sensor_exposure;
  int32_t sensor_sensitivity;
  int32_t ae_exp_index;
  float ae_exp_step;
  int32_t ae_min_fps;
  int32_t ae_max_fps;
  mtk::hal3a::mtk_3a_regions ae_region;
  uint8_t black_level_lock;
  // awb tag
  uint8_t awb_lock;
  uint8_t awb_mode;
  uint8_t color_correct_mode;
  std::array<float, kMaxColorGainsCount> color_correct_gain = {0};
  std::array<float, kMaxColorMatsCount> color_correct_mat = {0};

  // af tag
  uint8_t af_mode;
  uint8_t af_trigger;
  float af_focus_distance;
  int32_t af_zoom_ratio;
  int32_t af_zoom_stop;
  mtk::hal3a::mtk_3a_regions af_region;
  // flash
  uint8_t strobe_mode;
  mtk::hal3a::MtkFlashType flash_type;
  // shading
  uint8_t shading_mode;
  uint8_t shadingmap_mode;
  uint8_t tonemap_mode;
  bool lock_ratio;
  // feature
  uint8_t face_detect_mode;
  int32_t face_detect_force;
  mtk_3a_fast_switch_param fast_switch_param;
  // vednor
  int32_t ae_custom_pline_mode;
  int32_t ae_manual_pline_idx;
  int32_t ae_iso_speed_mode;
  uint8_t ae_meter_mode;
  uint8_t ae_convergence_speed;
  int32_t ae_custom_metering_table_mode;
  int32_t ae_clusive_roi_mode;
  std::array<uint8_t, MAX_AE_CUSTOM_METERING_TABLE_SIZE> ae_custom_metering_table;
  std::array<int32_t, MTK_MAX_PLINE_ANCHOR> ae_pline_anchor;
  std::array<int32_t, MTK_AE_CLUSIVE_ROI_SIZE> ae_clusive_roi;
  std::array<int32_t, MTK_MANUAL_AREA_ROI_SIZE> ae_manualarea_roi;
  int32_t flash_cali_en;
  int32_t awb_convergence_speed;
  int32_t awb_warmstart_enable;
  // --------- hal metadata --------
  // common tag
  bool repeat_tag;
  bool get_exif;
  bool is_center_region;
  int32_t imgo_type;
  int32_t app_mode;
  int32_t zoom_ratio;
  uint32_t target_size_w;
  uint32_t target_size_h;
  mtk::hal3a::mtk_3a_area prv_crop_region;
  mtk::hal3a::mtk_3a_area prv_crop_normalize_region;
  int32_t low_fps;
  // ae tag
  int32_t ae_target_mode;
  std::array<int32_t, MTK_AE_EXP_LEVEL_NUM> ae_exp_level;
  mtk::hal3a::v1_0::mtk_3a_custom_param custom_param;
  int32_t ae_valid_exp;
  int32_t denoise_mode;
  // awb tag
  uint8_t mwb_cct;
  // af tag
  bool pause_af;
  bool gyro_valid;
  NSCam::Utils::SensorData gyro_data;
  bool acce_valid;
  NSCam::Utils::SensorData acce_data;
  bool light_valid;
  NSCam::Utils::SensorData light_data;
  bool als_all_valid[2];
  std::array<NSCam::Utils::SensorData, MAX_ALS_DATA_SIZE> als_data_all[2];
  bool flicker_valid[2];
  NSCam::Utils::SensorData flicker_data[2];
  bool color_valid[2];
  NSCam::Utils::SensorData color_data[2];
  bool is_stagger;
  bool is_mstream;
  // flash
  strobe_drv_info flash_info;
  bool is_flash_force_off;
  // ltm
  int32_t isp_fus_num;
  // feature
  int32_t multiexp_hdr_mode;
  uint8_t hdr_mode;
  uint8_t face_num;
  bool is_fd_ready;
  bool is_fd_enable;
  MtkCameraFaceMetadata faces;
  // sync3a
  int32_t sync2a_mode;
  uint32_t master_idx;
  // --------- other param --------
  Hal3A_Param_IrisInformation iris_info;

  // Additional
  uint32_t ae_exp_level_size;
  size_t ae_custom_metering_table_size;
  size_t ae_pline_anchor_size;
  size_t ae_clusive_roi_size;
  size_t ae_manualarea_roi_size;
  size_t als_data_all_size[2];

  mtk_3a_param& operator=(const mtk::hal3a::v1_0::mtk_3a_param& param);
  void copyTo(mtk::hal3a::v1_0::mtk_3a_param& param);
};

struct mtk_3a_param_ae_metering_table {
  uint32_t tableSize;
  uint8_t* pTableBuf;

  mtk_3a_param_ae_metering_table& operator=(
      const std::vector<uint8_t>& vecAeMeteringTable);
  void copyTo(std::vector<uint8_t>& vecAeMeteringTable);
};

struct mtk_stt_buf {
  mtk_cam_uapi_meta_raw_stats_0 buf;
  int32_t fd;

  int32_t handle;

  mtk_stt_buf();
  mtk_stt_buf& operator=(const mtk::hal3a::v1_0::mtk_stt_buf& stt_buf);
  void copyTo(mtk::hal3a::v1_0::mtk_stt_buf& stt_buf, bool bOverrideSetting);
};

struct mtk_afo_buf {
  mtk_cam_uapi_meta_raw_stats_1* buf;
  int32_t fd;

  int32_t handle;

  // Additional
  bool buf_valid;

  mtk_afo_buf();
  mtk_afo_buf& operator=(const mtk::hal3a::v1_0::mtk_afo_buf& afo_buf);
  void copyTo(mtk::hal3a::v1_0::mtk_afo_buf& afo_buf, bool bOverrideSetting);
};

struct mtk_camsv_buf {
  mtk_cam_uapi_meta_camsv_stats_0 buf;
  int32_t fd;
  int32_t node_id;
  int32_t channel_id;
  int32_t feature_type;

  int32_t handle;

  mtk_camsv_buf();
  mtk_camsv_buf& operator=(const mtk::hal3a::v1_0::mtk_camsv_buf& camsv_buf);
  void copyTo(mtk::hal3a::v1_0::mtk_camsv_buf& camsv_buf,
              bool bOverrideSetting);
};

struct mtk_mraw_buf {
  const mtk_cam_uapi_meta_mraw_stats_0* buf;
  int32_t fd;
  int32_t node_id;
  int32_t channel_id;
  int32_t feature_type;

  int32_t handle;

  mtk_mraw_buf();
  mtk_mraw_buf& operator=(const mtk::hal3a::v1_0::mtk_mraw_buf& mraw_buf);
  void copyTo(mtk::hal3a::v1_0::mtk_mraw_buf& mraw_buf,
              bool bOverrideSetting);
};

struct mtk_ai3ao_buf {
  const void* buf;
  int32_t fd;

  int32_t handle;

  mtk_ai3ao_buf();
  mtk_ai3ao_buf& operator=(const mtk::hal3a::v1_0::mtk_ai3ao_buf& ai3ao_buf);
  void copyTo(mtk::hal3a::v1_0::mtk_ai3ao_buf& ai3ao_buf);
};

struct mtk_af_request {
  /********* Scenario ********/
  mutable mtk::hal3a::Mtk3AScenario scenario;  // preview, capture, postcap

  /********* Common **********/
  mtk::hal3a::v1_0::mtk_buf_info buf_info;
  mtk::ipc::hal3a_v1_0::mtk_afo_buf afo_buf;
  //
  mtk::hal3a::MtkFlashStatus flash_status;
  mtk::hal3a::MtkFlashType flash_type;
  SensorPerframeDynamicInfo sensor_perframe_dynamic_info;  /* non-POD */
  NSCam::TuningUtils::NddData ndd_data;
  NSCam::TuningUtils::eCategory ndd_category;

  /************ AE ***********/
  /************ AWB ***********/
  /************ AF ***********/
  // uint32_t cmd_trigger_af;
  // af data
  VcmFocusInformation focus_info;
  OpticalZoomInformation ozoom_info;

  /************ AFAsst ***********/
  TofInfo tof_info;
  /************ Flash ***********/
  /************ Shading ***********/


  mtk_af_request& operator=(const mtk::hal3a::v1_0::mtk_af_request& request);
  void copyTo(mtk::hal3a::v1_0::mtk_af_request& request);
};

struct mtk_3a_request {
  /********* Scenario ********/
  mtk::hal3a::Mtk3AScenario scenario;  // preview, capture, postcap

  /********* Common **********/
  mtk::hal3a::v1_0::mtk_buf_info buf_info;
  mtk::ipc::hal3a_v1_0::mtk_stt_buf stt_buf;
  mtk_ai3ao_buf ai3ao_buf;
  mtk_camsv_buf multi_camsv_buf[MAX_VC_INFO_CNT];
  mtk_mraw_buf multi_mraw_buf[MAX_VC_INFO_CNT];
  mtk::hal3a::v1_0::custom_aao_buf custom_aao;
  mtk::hal3a::v1_0::mtk_me_buf_info me_buf_info;
  mtk::hal3a::v1_0::mtk_me_result me_result;
  mtk::hal3a::GyroMvResult gyromv_result;
  mtk::hal3a::OisMvResult oismv_result;

  mtk::hal3a::MtkFlashStatus flash_status;
  mtk::hal3a::MtkFlashType previous_flash_type;
  mtk::hal3a::MtkFlashType flash_type;
  SensorPerframeDynamicInfo sensor_perframe_dynamic_info;
  NSCam::TuningUtils::NddData ndd_data;
  NSCam::TuningUtils::eCategory ndd_category;

  /************ AE ***********/
  /************ AWB ***********/
  /************ AF ***********/
  uint32_t cmd_trigger_af;
  VcmFocusInformation focus_info;
  OpticalZoomInformation ozoom_info;

  /************ AFAsst ***********/
  TofInfo tof_info;

  /************ Flash ***********/
  /************ Shading ***********/

  /************ Shared memory (used for IPC) ***********/

  mtk_3a_request& operator=(const mtk::hal3a::v1_0::mtk_3a_request& request);
  void copyTo(mtk::hal3a::v1_0::mtk_3a_request& request,
              bool bOverrideStt,
              bool bLinkStt);
};

struct area_info {
  int32_t buf[MTK_AREA_INFO_MAX_NUM];
  uint32_t size;

  void copyFrom(const std::vector<int32_t>& area);
  void copyTo(std::vector<int32_t>& area);
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
  int32_t seam_tun_para[MTK_AE_SEAMLESS_TUNING_PARA_SIZE];
  uint32_t seam_tun_para_len;

  //
  mtk::hal3a::mtk_ae_exposure_setting_table ae_exp_table;
  bool ai_shutter_exist_motion;
  int32_t real_light_value_x10;

  mtk_hal3a_ae_info& operator=(const mtk::hal3a::mtk_hal3a_ae_info& result);
  void copyTo(mtk::hal3a::mtk_hal3a_ae_info& result);
};

struct mtk_ae_result {
  bool skip_ae_calc;
  bool ae_stable;
  bool sensor_flicker_adjustment;
  uint8_t ae_state;
  uint8_t flicker_state;
  int16_t ae_face_diff_index;
  int32_t auto_hdr_result;
  int32_t sensor_sensitivity;
  int32_t sensor_gain;
  int32_t isp_gain;
  int32_t lux_index;
  int32_t sensor_frame_rate;
  int32_t ae_real_lv_x10;
  int32_t ev_comp;
  int32_t ev_index;
  int64_t sensor_exposure_time;
  int64_t sensor_frame_duration;
  int64_t sensor_exposure_line_count;
  ae_exposure_setting_table ae_exp_table;
  bool aai_enable;
  std::array<uint8_t, MTK_CAM_UAPI_ROI_MAP_BLK_NUM> aai_map;
  area_info area;
  uint32_t iris_target_step;
  mtk_hal3a_ae_info perframe_output;
  // TODO: implement it.
  void* p_ae_alg_data;
  int32_t ae_alg_data_size;
  // SAT
  bool bDisplayInvalid;

  mtk_ae_result& operator=(const mtk::hal3a::v1_0::mtk_ae_result& result);
  void copyTo(mtk::hal3a::v1_0::mtk_ae_result& result);
};

struct mtk_awb_result {
  bool awb_stable;
  uint8_t awb_state;
  std::array<int32_t, kNumAwbGains> awb_gain;
  int32_t awb_gain_scale_uint;
  int32_t awb_rgain_x128;
  int32_t awb_bgain_x128;
  int32_t awb_rgain_d65_x128;
  int32_t awb_bgain_d65_x128;
  int32_t awb_rgain_cwf_x128;
  int32_t awb_bgain_cwf_x128;
  int32_t awb_color_temperature;
  int32_t awb_stt_num;
  std::array<int32_t, kNumAwbRange> awb_available_range;
  std::array<float, kMaxColorGainsCount> color_correct_gain;
  area_info area;

  mtk_awb_result& operator=(const mtk::hal3a::v1_0::mtk_awb_result& result);
  void copyTo(mtk::hal3a::v1_0::mtk_awb_result& result);
};

struct mtk_af_result {
  uint8_t af_state;
  uint8_t lens_state;
  bool is_af_focused;
  bool is_focus_finish;

  // lens
  float lens_focus_distance;
  int32_t lens_zoom_ratio;
  int32_t lens_zoom_stop;
  std::array<float, kNumLensFocusRange> lens_focus_range;
  int32_t lens_position;
  OpticalZoomParameter ozoom_param;
  area_info area;

  // mtk_hal3a_af_info for MW User
  mtk::hal3a::mtk_hal3a_af_info perframe_output;

  mtk_af_result& operator=(const mtk::hal3a::v1_0::mtk_af_result& result);
  void copyTo(mtk::hal3a::v1_0::mtk_af_result& result);
};

struct shading_map_info {
  uint8_t buf[MTK_SHADING_MAP_INFO_MAX_NUM];
  uint32_t size;
};

struct mtk_shading_result {
  uint32_t table_offset;
  uint32_t table_size;
  std::array<uint8_t,
             (sizeof(ILscTable::Config) + sizeof(ILscTable::RsvdData) +
              MTK_CAM_LSCI_TABLE_SIZE)>
      lsc_data;

  mtk_shading_result& operator=(
      const mtk::hal3a::v1_0::mtk_shading_result& result);
  void copyTo(mtk::hal3a::v1_0::mtk_shading_result& result);
};

struct mtk_af_assist_result {
  uint32_t pd_reg_size;
  uint16_t p_pd_register[MTK_AF_ASSIST_PD_REG_MAX_NUM];
  mtk::hal3a::mtk_hal3a_pd_info pd_perframe_output;

  bool pd_buf_valid;

  mtk_af_assist_result();
  mtk_af_assist_result& operator=(
      const mtk::hal3a::v1_0::mtk_af_assist_result& af_assist);
  void copyTo(mtk::hal3a::v1_0::mtk_af_assist_result& af_assist);
};

struct mtk_ai3a_result {
  uint64_t v_finished_request_id[MTK_AI3A_RESULT_REQUEST_ID_MAX_NUM];
  uint32_t v_finished_request_id_size;

  // TODO: Implement the followings.
  void* sa_buf;
  int32_t sa_dumpsize;
  void* ga_buf;
  int32_t ga_bufsize;
  void* fa_buf;
  int32_t fa_dumpsize;

  NSCam::MRect crop_region;
  mtk::hal3a::mtk_hal3a_ai3a_info perframe_output;


  mtk_ai3a_result();
  mtk_ai3a_result& operator=(const mtk::hal3a::v1_0::mtk_ai3a_result& result);
  void copyTo(mtk::hal3a::v1_0::mtk_ai3a_result& result);
};

struct mtk_tone_result {
  uint8_t* p_ltm_alg_data;
  int32_t p_ltm_alg_data_handle;
  int32_t ltm_alg_data_size;

  uint8_t* p_me_tcy_in_workbuf_data;
  int32_t me_tcy_in_workbuf_data_size;
  int32_t p_me_tcy_in_workbuf_data_handle;

  uint8_t* p_me_tcy_fst_o_data;
  int32_t me_tcy_fst_o_data_size;
  int32_t p_me_tcy_fst_o_data_handle;

  void setLtmDataShmInfo(const ShmMemInfo& shmInfo);
  void setTCYInShmInfo(const ShmMemInfo& shmInfo);
  void setTCYFstShmInfo(const ShmMemInfo& shmInfo);
};

struct mtk_3a_result {
  uint64_t request_id;
  mtk_ae_result ae_result;
  mtk_awb_result awb_result;
  mtk_af_result af_result;
  mtk_af_assist_result afasst_result;
  mtk::hal3a::v1_0::mtk_flash_result flash_result;
  mtk::hal3a::v1_0::mtk_flk_result flk_result;
  mtk::hal3a::v1_0::mtk_shading_result shading_result;
  mtk::hal3a::v1_0::mtk_ir_result ir_result;
  mtk_ai3a_result ai3a_result;
  mtk_tone_result tone_result;
  mtk::hal3a::v1_0::mtk_pdp_result pdp_result;
  mtk::hal3a::v1_0::mtk_sensor_result sensor_result;

  // TODO: Implement it.
  // mtk_cac_result cac_result;

  mtk_cam_uapi_meta_raw_stats_cfg raw_meta;
  mtk::hal3a::mtk_3a_exif standard_exif;
  AAA_DEBUG_INFO1_T debug_3a_info;
  AAA_DEBUG_INFO2_T debug_isp_info;

  mtk_3a_result& operator=(const mtk::hal3a::v1_0::mtk_3a_result& result);
  void copyTo(mtk::hal3a::v1_0::mtk_3a_result* result);
};

struct mtk_hal3a_setting {
  mtk_cam_uapi_meta_raw_stats_cfg raw_meta;
#if SUPPORT_META_MRAW == 1
  mtk_cam_uapi_meta_mraw_stats_cfg mraw_meta[VC_MAX_NUM];
#endif
  mtk_cam_front_end_setting driver_setting;
  NSCam::MRect ai3ao_crop;
  uint64_t request_id;

  // Additional
  bool b_raw_meta_valid;

  mtk_hal3a_setting& operator=(const mtk::hal3a::v1_0::mtk_hal3a_setting& setting);
  void copyTo(mtk::hal3a::v1_0::mtk_hal3a_setting* setting);
};

//=====================================================================================

/*
 * Hal3A Shared Memory
 */
struct Hal3A_Init : public ShmMemBase {
  mtk_3a_init init;

  Hal3A_Init& operator=(const mtk::hal3a::v1_0::mtk_3a_init& init);
  void copyTo(mtk::hal3a::v1_0::mtk_3a_init& init);
};

struct Hal3A_GetHwInitialSetting : public ShmMemBase {
  mtk::ipc::hal3a_v1_0::mtk_3a_config config;
  mtk::hal3a::v1_0::mtk_hw_initial_setting result;
};

struct Hal3A_Config : public ShmMemBase {
  mtk::ipc::hal3a_v1_0::mtk_3a_config config;
};

struct Hal3A_Start : public ShmMemBase {
  mtk::hal3a::v1_0::mtk_3a_start start;
};

struct Hal3A_Stop : public ShmMemBase {
  mtk::hal3a::v1_0::mtk_3a_stop stop;
};

struct Hal3A_Uninit : public ShmMemBase {
  mtk::hal3a::v1_0::mtk_3a_uninit uninit;
};

struct Hal3A_Param : public ShmMemBase {
  mtk::ipc::hal3a_v1_0::mtk_3a_param param;
};

struct Hal3A_DoCalculation : public ShmMemBase {
  mtk::ipc::hal3a_v1_0::mtk_3a_request request;
};

struct Hal3A_DoCalculationAF : public ShmMemBase {
  mtk_af_request request;
};

struct Hal3A_AAOBuf : public ShmMemBase {};

struct Hal3A_GetResult : public ShmMemBase {
  mtk::ipc::hal3a_v1_0::mtk_3a_result result;
};

struct Hal3A_GetResultAF : public ShmMemBase {
  mtk::hal3a::v1_0::mtk_lens_result result;

  void copyTo(mtk::hal3a::v1_0::mtk_lens_result& result);
};

struct Hal3A_Enable3ASetParam : public ShmMemBase {
  bool enable;
};

struct Hal3A_GetSensorStaticInfo : public ShmMemBase {
  NSCam::SensorStaticInfo sensor_info;

  Hal3A_GetSensorStaticInfo& operator=(const NSCam::SensorStaticInfo* info);
  void copyTo(NSCam::SensorStaticInfo* info);
};

struct Hal3A_Set2aDataToLastPool : public ShmMemBase {
};

struct Hal3A_Set2aDataToPool : public ShmMemBase {
  mtk::ipc::hal3a_v1_0::mtk_3a_request request;
};

struct Hal3A_GetResultOfCamsysChange : public ShmMemBase {
  mtk::hal3a::mtk_camsys_info info;
  mtk_hal3a_setting setting;

  Hal3A_GetResultOfCamsysChange& operator=(const mtk::hal3a::mtk_camsys_info& info);
  void copyTo(mtk::hal3a::v1_0::mtk_hal3a_setting* setting);
};

struct Hal3A_GetResultForceUpdate : public ShmMemBase {
  mtk::ipc::hal3a_v1_0::mtk_3a_result result;
};


}  // namespace hal3a_v1_0
}  // namespace ipc
}  // namespace mtk

#endif  // AAA_IPC_HAL3A_INC_HAL3APARAM_V1_0_H_

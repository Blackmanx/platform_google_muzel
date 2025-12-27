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
#ifndef INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_AAA_HAL_DEF_H_
#define INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_AAA_HAL_DEF_H_

#include <mtkcam-core/aaahal/aaa_hal_ctrl/aaa_hal_ctrl_def.h>
#include <aaahal/source/Hal3A/sync3a_wrapper/Sync3ADataDef.h>
#include <array>
#include <vector>

// #ifndef MTKCAM_ISP7_DEV
// #include "kernel_header/mtk_cam_metabuf.h"
// #else
#include <mtk_cam_metabuf.h>
// #endif
#include "mtkcam-interfaces/utils/hw/faces.h"
// #include "mtkcam-interfaces/aaahal/aaa_hal_ctrl/IHal3AAssist.h"
#include "mtkcam-interfaces/aaahal/aaa_hal_ctrl/IHal3ACommon.h"
#include "mtkcam-interfaces/aaahal/aaa_hal_ctrl/ISync3AData.h"
#include "mtkcam-interfaces/hw/mem/cam_cal_drv.h"
#include "mtkcam-interfaces/utils/sys/sensor_type.h"
#include "mtkcam-interfaces/utils/ndd/ndd_autogen_def.h"
#include "mtkcam-interfaces/feature/objcetTracking/IObjTrackingInfo.h"
#include "mtkcam-interfaces/feature/stereo/hal/stereo_aaa_info_provider.h"
#include "peripheraldriver/lens/vcm_drv.h"
#include "peripheraldriver/ozoom/ozoom_drv.h"
#include "peripheraldriver/strobe/strobe_hal.h"
#include "peripheraldriver/tof/tof_drv.h"
#include "public/aaa_hal_common.h"
#include "mtkcam-core/aaahal/ae_mgr/ae_setting.h"
#include "debug_exif/aaa/dbg_aaa_param.h"
#include "mtkcam-core/aaa/include/lsc/ILscTable.h"
#include "mtkcam-core/hw/camsys/pipe_mgr/MetaData.h"

static const uint32_t kMaxColorGainsCount = 4;
static const uint32_t kMaxColorMatsCount = 9;
static const uint32_t kNumAwbGains = 3;
static const uint32_t kNumAwbRange = 2;
static const uint32_t kNumLensFocusRange = 2;
static const uint32_t kNumFocusArea = 2;

namespace mtk {
namespace hal3a {

enum Mtk3AScenario {
  kPreview = 0,
  kPreCapture,
  kCaptureP1,
  kCaptureP2,
  kTouchAE,
  kAFNormal,
  kAFTrigger
};

enum MtkFlashType {
  kNoFlash = 0,
  kPreFlash,
  kMainFlash,
  kMainFlashByUser,
  kSlowSyncMainFlash,
  kAiFlashMainFlash,
  kTorchFlash,
  kPanelFlash,
  kCalibrationFlash,
};

enum MtkFlashStatus { kStillOff = 0, kStillOn, kChangeToOff, kChangeToOn };

enum MtkSttBufFlashState { kFlashOff = 0, kFlashPartial, kFlashFired };

enum Mtk3AExtCtrl { kGetExtInfo = 0, kSetExtInfo, kMtk3AExtCtrlEnd };

namespace Mtk3AActiveItem {
static const uint32_t kAE = (1 << 0);
static const uint32_t kAWB = (1 << 1);
static const uint32_t kAF = (1 << 2);
static const uint32_t kFlash = (1 << 3);
static const uint32_t kFlicker = (1 << 4);
static const uint32_t kShading = (1 << 5);
}  // namespace Mtk3AActiveItem

struct Mtk3AOTInfo {
  bool is_valid;
  ObjectTrackingInfo data;
};
struct Mtk3AGFInfo {
  bool is_valid;
  StereoHAL::GF_INFO_TO_AF data;
};

namespace v1_0 {
struct mtk_3a_custom_param {
  uint32_t id;
  bool gain_align;
  uint32_t target_gain_x1024;
  uint32_t ev0_isp_gain_x1024;
  uint32_t ev0_sensor_gain_x1024;
  uint32_t ev0_exptime_us;
  uint32_t exptime_limit_us;
  uint32_t gain_limit_x1024;
  uint32_t frame_index;
  uint32_t end_frame_index;
};

struct mtk_3a_init {
  std::array<SensorStaticInfo, kMaxSensorCnt> sensor_static_info_array;
  mtk::hal3a::SensorInitialDynamicInfo sensor_init_dynamic_info;
  uint32_t mock_mode;
  /************ sensor info  ************/
  uint32_t gyro_mv_size;
  uint32_t gyro_num_size;
  bool is_vcm_support;
  bool is_ircut_support;
  bool is_ozoom_support;
  bool is_iris_support;
  bool flash_hw_support;
  strobe_drv_cap flash_capability;
  CAM_CAL_DATA_STRUCT cal_data;
  CAM_CAL_2A_DATA_STRUCT cal_aa;
  CAM_CAL_LSC_DATA_STRUCT cal_lsc;
  CAM_CAL_PDAF_DATA_STRUCT cal_pdaf;
};
struct mtk_3a_uninit {};
struct mtk_3a_start {};
struct mtk_3a_stop {};

struct mtk_3a_fast_switch_param {
  int32_t sensor_mode;
  NSCam::MSize tg_size;
  NSCam::MSize full_tg_size;
  int32_t ae_target_mode_next;
  int32_t ae_valid_exp_next;
  uint32_t ae_sensor_mode_next;
  bool is_seamless;
  uint8_t seam_policy;

  mtk_3a_fast_switch_param()
    : sensor_mode(0)
    , tg_size()
    , full_tg_size()
    , ae_target_mode_next(0)
    , ae_valid_exp_next(0)
    , ae_sensor_mode_next(0)
    , is_seamless(false)
    , seam_policy(0)
    {}
};

struct mtk_manual_ev_control {
    int32_t frame_count;
    int32_t frame_index;
    float ev_step;
    float ev_begin;
    uint8_t ae_lock;

    mtk_manual_ev_control()
        : frame_count(0)
        , frame_index(0)
        , ev_step(0)
        , ev_begin(0)
        , ae_lock(0)
    {}
};

struct mtk_sensor_test_patten_data {
    bool valid;
    int32_t Channel_R;
    int32_t Channel_Gr;
    int32_t Channel_Gb;
    int32_t Channel_B;

    mtk_sensor_test_patten_data()
        : valid(false)
        , Channel_R(0)
        , Channel_Gr(0)
        , Channel_Gb(0)
        , Channel_B(0)
    {}
};
/*****************************************************************************
 * Config Input
 *****************************************************************************/
/*
 * Mediatek AE statistics (AAHO).
 *  @stat_width: source width of the statistics
 *  @state_height: source height of the statistics
 *  @ae_stat_r: r channel stat of AE statistics
 */
struct IrisInformation {
  int32_t iris_status;
  int32_t iris_position;
  int32_t previous_iris_position;
  int64_t moving_timestamp;
  int64_t previous_moving_timestamp;
  int32_t (*p_DeltaEV_LUT)[100];
  float fn_cali;
};
struct mtk_3a_config {
  std::array<SensorStaticInfo, kMaxSensorCnt> sensor_static_info_array;
  mtk::hal3a::SensorInitialDynamicInfo sensor_init_dynamic_info;
  mtk_hal3a_config control_config;
  NSCam::IMetadata static_meta;
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
  mtk::hal3a::SensorPerframeDynamicInfo sensor_perframe_dynamic_info;
  ori orientation;
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
  int32_t ae_sensor_min_fps = 0;
  int32_t ae_sensor_max_fps = 0;
  int32_t ae_valid_exp;
  /************ feature  ************/
  int32_t multiexp_hdr_mode;
  bool aov_enable;
  /************ ltm info  ************/
  int32_t isp_fus_num;
  /************ Iris info  ************/
  IrisInformation iris_info;
  /************ Others  ************/
  uint8_t face_num;
  MtkFlashType flash_type;
  /************ Peripheral sensors info  ************/
  string als_sensor_name[2];
  string flicker_sensor_name[2];
  string color_sensor_name[2];
};

/****************************************************************************
 * Perframe Set Param
 *****************************************************************************/
struct mtk_3a_param {
  uint64_t request_id;
  uint32_t active_items;
  bool updated;
  bool is_dummy_request;
  // --------- app metadata --------
  // common tag
  uint8_t control_mode = MTK_CONTROL_MODE_AUTO;
  uint8_t scene_mode;
  uint8_t capture_intent = MTK_CONTROL_CAPTURE_INTENT_PREVIEW;
  uint8_t inflight_capture;
  // ae tag
  uint8_t ae_lock;
  uint8_t ae_mode = 1;
  uint8_t ae_precap_trigger;
  uint8_t ae_anti_banding_mode;
  int64_t sensor_frame_duration;
  int64_t sensor_exposure;
  int32_t sensor_sensitivity;
  int32_t ae_exp_index;
  float ae_exp_step = 5;
  int32_t ae_min_fps = 5000;
  int32_t ae_max_fps = 30000;
  mtk_3a_regions ae_region = {};
  uint8_t black_level_lock;
  bool set_converge;
  // awb tag
  uint8_t awb_lock;
  uint8_t awb_mode = 1;
  uint8_t color_correct_mode = MTK_COLOR_CORRECTION_MODE_FAST;
  std::array<float, kMaxColorGainsCount> color_correct_gain = {0};
  std::array<float, kMaxColorMatsCount> color_correct_mat = {0};
  bool awb_default_pregain1 = false;
  // af tag
  uint8_t af_mode = 5;
  uint8_t af_trigger;
  float af_focus_distance = -1;
  int32_t af_zoom_ratio = -1;
  int32_t af_zoom_stop;
  mtk_3a_regions af_region = {};
  uint8_t lens_ois_mode = MTK_LENS_OPTICAL_STABILIZATION_MODE_OFF;
  bool af_notify_timeout = false;
  // flash
  uint8_t strobe_mode;
  MtkFlashType flash_type = kNoFlash;
  // shading
  uint8_t shading_mode = MTK_SHADING_MODE_FAST;
  uint8_t shadingmap_mode = MTK_STATISTICS_LENS_SHADING_MAP_MODE_OFF;
  uint8_t tonemap_mode = MTK_TONEMAP_MODE_FAST;
  bool lock_ratio;
  bool lock_shading = false;
  // sesnor
  int32_t sensor_test_patten_mode = -1;
  mtk_sensor_test_patten_data sensor_test_patten_data;
  int32_t prolong_frame_length = 0;
  // feature
  uint8_t face_detect_mode;
  int32_t face_detect_force;
  // vendor
  int32_t ae_custom_pline_mode;
  int32_t ae_manual_pline_idx;
  int32_t ae_iso_speed_mode;
  uint8_t ae_meter_mode;
  int32_t ae_convergence_speed = 0;
  int32_t ae_custom_metering_table_mode =
          MTK_3A_FEATURE_AE_CUSTOM_METERING_TABLE_OFF;
  int32_t ae_clusive_roi_mode =
          MTK_3A_FEATURE_AE_CLUSIVE_ROI_OFF;
  std::vector<uint8_t> ae_custom_metering_table;
  std::vector<int32_t> ae_pline_anchor;
  std::vector<int32_t> ae_clusive_roi;
  std::vector<int32_t> ae_manualarea_roi;
  int32_t flash_cali_en = 0;
  int32_t awb_convergence_speed;
  int32_t awb_warmstart_enable;
  // --------- hal metadata --------
  // common tag
  bool repeat_tag;
  bool get_exif;
  bool is_center_region;
  int32_t imgo_type;
  int32_t app_mode;
  int32_t zoom_ratio = 100;
  uint32_t target_size_w;
  uint32_t target_size_h;
  mtk_3a_area prv_crop_region = {};
  mtk_3a_area prv_crop_normalize_region = {};
  int32_t low_fps = MTK_MULTICAM_LOW_FPS_OFF;
  uint8_t remosaic_enable = 0;
  // ae tag
  int32_t ae_target_mode;
  std::vector<int32_t> ae_exp_level;
  mtk_3a_custom_param custom_param = {};
  int32_t ae_valid_exp;
  int32_t denoise_mode;
  int32_t ae_sensor_min_fps = 0;
  int32_t ae_sensor_max_fps = 0;
  int32_t ae_hal_exp_index = 0;
  mtk_manual_ev_control manual_ev_ctrl = {};
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
  std::vector<NSCam::Utils::SensorData> als_data_all[2];
  bool flicker_valid[2];
  NSCam::Utils::SensorData flicker_data[2];
  bool color_valid[2];
  NSCam::Utils::SensorData color_data[2];
  bool is_stagger;
  bool is_mstream;
  // flash
  strobe_drv_info flash_info = {};
  bool is_flash_force_off;
  bool flash_full_cali_en = false;
  bool flash_fast_cali_en = false;
  // ltm
  int32_t isp_fus_num;
  // feature
  int32_t subsample_sync_info;  // for HAL3A modules subsampling
  int32_t multiexp_hdr_mode;
  uint8_t hdr_mode;
  mtk_3a_fast_switch_param fast_switch_param;
  uint8_t face_num;
  bool is_fd_ready;
  bool is_fd_enable;
  MtkCameraFaceMetadata faces = {};
  Mtk3AOTInfo ot_info;
  Mtk3AGFInfo gf_info;
  // Stereo Feature
  mtk_sync3a_control_info r_sync3a_control_info;
  /************ Tuning Info ************/
  int64_t tuning_feature;
  int64_t sensor_feature;
  int32_t custom_feature;
  int64_t tuning_feature_cap;
  int32_t custom_feature_cap;
  int32_t custom_00;
  // sync3a
  int32_t  sync2a_mode = kSYNC2A_MODE_OFF;
  uint32_t master_idx;
  uint32_t awb_master_idx;
  // --------- other param --------
  IrisInformation iris_info;
};

/*****************************************************************************
 * Perframe Input
 *****************************************************************************/
struct mtk_buf_info {
  uint64_t request_id;
  uint64_t driver_frame_id = 0;
  uint32_t stt_count;
  uint64_t sof_timestamp;
  MtkSttBufFlashState flash_state;
  // ai3ao info
  int32_t crop_rect_x;
  int32_t crop_rect_y;
  int32_t crop_rect_width;
  int32_t crop_rect_height;
  // algo setting, will be update by Hal3AImp
  mutable mtk_buf_ae_info ae_stat_info;
  mutable std::array<uint8_t, MTK_CAM_UAPI_ROI_MAP_BLK_NUM> aai_map;
};

struct mtk_stt_buf {
  const mtk_cam_uapi_meta_raw_stats_0* buf;
  int32_t fd;
};

struct mtk_afo_buf {
  const mtk_cam_uapi_meta_raw_stats_1* buf;
  int32_t fd;
};

struct mtk_ai3ao_buf {
  const void* buf;
  int32_t fd;
};

struct mtk_camsv_buf {
  const mtk_cam_uapi_meta_camsv_stats_0* buf;
  int32_t fd;
  int32_t node_id;
  int32_t channel_id;
  int32_t feature_type;
};

struct mtk_mraw_buf {
#if SUPPORT_META_MRAW == 1
  const mtk_cam_uapi_meta_mraw_stats_0* buf;
#endif
  int32_t fd;
  int32_t node_id;
  int32_t channel_id;
  int32_t feature_type;
};

struct custom_aao_buf {
  void* buf;
  uint32_t buf_size;
};

struct mtk_af_request {
  /********* Scenario ********/
  mutable Mtk3AScenario scenario;  // preview, capture, postcap

  /********* Common **********/
  mtk_buf_info buf_info;
  mtk_afo_buf afo_buf;
  //
  MtkFlashStatus flash_status;
  MtkFlashType flash_type;
  mtk::hal3a::SensorPerframeDynamicInfo sensor_perframe_dynamic_info;
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
};

struct mtk_me_buf_info {
  void* p_buf;            // me buffer address
  int32_t fd;            // me buffer fd
  uint32_t buf_size;    // me buffer size
  uint32_t buf_strides;  // me buffer strides
  uint32_t data_width;  // me data width
  uint32_t data_height;  // me data height
};

struct mtk_me_result {
  bool is_valid;         // is me result valid or not
  uint32_t mag_num;     // magic number
  uint64_t time_stamp;  // timestamp
  mtk_me_buf_info mei_L1;    // layer 1 input Y buffer
  mtk_me_buf_info mv_L0;     // layer 0 mv buffer
  mtk_me_buf_info mv_L1;     // layer 1 mv buffer
  mtk_me_buf_info conf;     // confidence
  mtk_me_buf_info fmb_L0;    // layer 0 fmb buffer
};

struct mtk_3a_request {
  /********* Scenario ********/
  mutable Mtk3AScenario scenario;  // preview, capture, postcap

  /********* Common **********/
  mtk_buf_info buf_info;
  mtk_stt_buf stt_buf;
  mtk_ai3ao_buf ai3ao_buf;
  mtk_camsv_buf multi_camsv_buf[MAX_VC_INFO_CNT];
  mtk_mraw_buf multi_mraw_buf[MAX_VC_INFO_CNT];
  custom_aao_buf custom_aao;
  mtk_me_buf_info me_buf_info;
  mtk_me_result me_result;
  mtk::hal3a::GyroMvResult gyromv_result;
  mtk::hal3a::OisMvResult oismv_result;

  MtkFlashStatus flash_status;
  MtkFlashType previous_flash_type;
  MtkFlashType flash_type;
  mtk::hal3a::SensorPerframeDynamicInfo sensor_perframe_dynamic_info;
  NSCam::TuningUtils::NddData ndd_data;
  NSCam::TuningUtils::eCategory ndd_category;

  // af data
  VcmFocusInformation focus_info;
  OpticalZoomInformation ozoom_info;

  /************ AFAsst ***********/
  TofInfo tof_info;
  /************ AE ***********/
  /************ AWB ***********/
  /************ AF ***********/
  // af data

  /************ AFAsst ***********/
  /************ Flash ***********/
  /************ Shading ***********/
};

struct mtk_3a_ext_request {};
struct mtk_3a_ext_result {};

/*****************************************************************************
 * Perframe Output
 *****************************************************************************/
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
  uint64_t padding_sensor_line = 0;
  uint64_t padding_sensor_exp = 0;
  ae_exposure_setting_table ae_exp_table;
  //
  bool aai_enable;
  std::array<uint8_t, MTK_CAM_UAPI_ROI_MAP_BLK_NUM> aai_map;
  std::vector<int32_t> area;
  uint32_t iris_target_step;
  // mtk_hal3a_ae_info for MW User
  mtk_hal3a_ae_info perframe_output;
  void* p_ae_alg_data;
  int32_t ae_alg_data_size;
  // SAT
  bool bDisplayInvalid;
};

struct mtk_flash_result {
  uint8_t sub_flash_state;
  uint8_t algo_version;
  int32_t panel_rgb;
  int32_t hardwareChannelEn;
  bool main_flash;      // TODO(Muse) : isMainFlash
  bool pre_flash;       // TODO(Muse) : isPreFlash
  StrobeHalInfo drv_setting;
  strobe_duty_setting duty_setting[STROBE_HAL_SCENARIO_NUM];
  mtk_hal3a_flash_info perframe_output;
  bool cali_flash_en = 0;
  int32_t flash_cali_state = 0;
  int32_t flash_cali_result = 0;
  NSCam::camsys::pipemgr::mtk_flash_driver_control camsys_flash_setting;
  bool flash_bv_trigger;
  uint32_t padding_sensor_line = 0;
  uint64_t padding_sensor_exp = 0;
};

struct mtk_flk_result {};

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
  std::vector<int32_t> area;
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
  std::vector<int32_t> area;

  // mtk_hal3a_af_info for MW User
  mtk_hal3a_af_info perframe_output;
};

struct mtk_lens_result {
  int32_t lens_position;
  bool is_focus_finish;
  OpticalZoomParameter ozoom_param;
};

struct mtk_af_assist_result {
  uint32_t pd_reg_size;
  uint16_t* p_pd_register;
  mtk_hal3a_pd_info pd_perframe_output;
};

struct mtk_shading_result {
  uint32_t lsc_width;
  uint32_t lsc_height;
  uint32_t table_offset;
  uint32_t table_size;
  std::array<uint8_t, (sizeof(NSIspTuning::ILscTable::Config) +
    sizeof(NSIspTuning::ILscTable::RsvdData) + MTK_CAM_LSCI_TABLE_SIZE)> lsc_data;
};

struct mtk_ir_result {
  uint32_t ir_status;
};

struct mtk_ai3a_result {
  std::vector<uint64_t> v_finished_request_id;
  void* sa_buf;
  int32_t sa_dumpsize;
  void* ga_buf;
  int32_t ga_bufsize;
  void* fa_buf;
  int32_t fa_dumpsize;
  NSCam::MRect crop_region;
  mtk_hal3a_ai3a_info perframe_output;
};

struct mtk_tone_result {
  void* p_ltm_alg_data;
  int32_t ltm_alg_data_size;
  void* p_me_tcy_in_workbuf_data;
  int32_t me_tcy_in_workbuf_data_size;
  void* p_me_tcy_fst_o_data;
  int32_t me_tcy_fst_o_data_size;
};

#define MTK_PDP_HW_NUM 4
struct mtk_pdp_result {
  uint32_t used_num;
  uint32_t vc_feature[MTK_PDP_HW_NUM];
#if SUPPORT_META_MRAW == 1
  mtk_cam_uapi_meta_mraw_stats_cfg mraw_meta[MTK_PDP_HW_NUM];
#endif
};

struct mtk_sensor_result {
  uint8_t flicker_status;
  uint8_t flicker_confidence;
  uint16_t flicker_frequency;
  uint8_t flicker_ac_dc_ratio_x100;
};

struct mtk_cac_result {
  void* p_cac_table;
  int32_t cac_table_size;
};

struct mtk_3a_result {
  uint64_t request_id;
  mtk_ae_result ae_result;
  mtk_awb_result awb_result;
  mtk_af_result af_result;
  mtk_af_assist_result afasst_result;
  mtk_flash_result flash_result;
  mtk_flk_result flk_result;
  mtk_shading_result shading_result;
  mtk_ir_result ir_result;
  mtk_ai3a_result ai3a_result;
  mtk_tone_result tone_result;
  mtk_pdp_result pdp_result;
  mtk_sensor_result sensor_result;
  mtk_cac_result cac_result;
  mtk_cam_uapi_meta_raw_stats_cfg raw_meta;
  mtk_3a_exif standard_exif;
  AAA_DEBUG_INFO1_T debug_3a_info;
  AAA_DEBUG_INFO2_T debug_isp_info;
};

struct mtk_result_event {
  // Flag
  bool is_force_update = false;
  bool is_camsys_change = false;
  // Data
  mtk_camsys_info camsys_info = {};
};

struct mtk_hw_initial_setting {
  ae_exposure_setting_table ae_exp_table;
  int32_t sensor_frame_rate;
};

}   // namespace v1_0

namespace v2_0 {
struct mtk_3a_custom_param : v1_0::mtk_3a_custom_param {};
struct mtk_3a_start : v1_0::mtk_3a_start {};
struct mtk_3a_stop : v1_0::mtk_3a_stop {};
struct mtk_3a_config : v1_0::mtk_3a_config {};
struct mtk_3a_param : v1_0::mtk_3a_param {};
struct mtk_3a_request : v1_0::mtk_3a_request {};
struct mtk_3a_result : v1_0::mtk_3a_result {};
struct mtk_3a_ext_request : v1_0::mtk_3a_ext_request {};
struct mtk_3a_ext_result : v1_0::mtk_3a_ext_result {};
}   // namespace v2_0

}       // namespace hal3a
}       // namespace mtk
#endif  // INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_AAA_HAL_DEF_H_

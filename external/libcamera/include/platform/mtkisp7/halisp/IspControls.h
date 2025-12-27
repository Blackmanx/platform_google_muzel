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

#ifndef AAA_ISPHAL_INCLUDE_V2_0_ISPHAL_ISPCONTROLS_H_
#define AAA_ISPHAL_INCLUDE_V2_0_ISPHAL_ISPCONTROLS_H_

// public interfaces
#include <mtkcam-interfaces/isphal/Buffer.h>      // mtk::isphal::Buffer
#include <mtkcam-interfaces/isphal/Predefines.h>  // MTK_ISPHAL_ALIGN_DEFAULT

#include <platform/mtkisp7/mtkcam-interfaces/include/mtkcam-interfaces/utils/ndd/ndd_autogen_def.h>

// core interfaces
#include "IspControlBasic.h"

// private interfaces
#include "utils/ImageFormats.h"
#include "utils/Size.h"

#include "platform/mtkisp7/mtkcam-chrom/custom/mt8188/hal/inc/isp_tuning/isp_tuning.h"
#include <platform/mtkisp7/mtkcam-chrom/custom/mt8188/hal/inc/tuning_mapping/cam_idx_struct_ext_pub.h>
#include <platform/mtkisp7/mtkcam-chrom/custom/mt8188/hal/inc/camera_custom_isp_nvram_pub.h>
#include <platform/mtkisp7/mtkcam-chrom/custom/mt8188/hal/inc/isp_tuning/ver1/isp_tuning_cam_info_pub.h>
#include <platform/mtkisp7/mtkcam-chrom/custom/mt8188/hal/inc/tuning_mapping/cam_idx_struct_mapping_pub.h>

#include <platform/mtkisp7/mtkcam-interfaces/mtkcam-halif/include/mtkcam-halif/utils/metadata/1.x/IMetadata.h>
// 3a interfaces
#include "aaa_mgr_public_if_pub.h"

// ae info
#include "ae_mgr/ae_flow_params.h"

// ScenarioRecorder
//#include <mtkcam-interfaces/utils/ScenarioRecorder/IScenarioRecorder.h>
#include "ScenarioRecorder/ScenarioRecorderDef.h"

// NDD
//#include <mtkcam-interfaces/utils/ndd/INdd.h>
using NSCam::TuningUtils::eCategory;
using NSCam::TuningUtils::eModule;
//using NSCam::TuningUtils::INdd;
//using NSCam::TuningUtils::NddCommon;
using NSCam::TuningUtils::NddData;

// #include "MappingInfo.h"

// Custom
//#include <isp_tuning/isp_tuning.h>
//#include <isp_tuning/cam_info_pub.h>

#include <bitset>   // std::bitset
#include <cstdint>  // uint32_t, int32_t
#include <mutex>
#include <array>

namespace NSCam {
	class IMetadata;
}

namespace mtk {
namespace isphal {
namespace v1_0 {

/**
 * Configuration for ISP HAL while configuring.
 */
struct IspConfigContrl {
  mtk::isphal::Size sensor_size;
};

/**
 * ImageDescritor describes the information that the related image.
 *  @width: The width of the image in pixels.
 *  @height: The height of the image in pixels.
 *  @format: The image format of the image, where the format should be the one
 *      that ISP hal knows.
 */
struct ImageDescriptor {
  size_t width;
  size_t height;
  mtk::isphal::ImageFormat format;

  // temp raw descriptor
  int32_t p2_in_img_fmg;

  ImageDescriptor()
      : width(0),
        height(0),
        format(kImageFormatUndef),
        p2_in_img_fmg(0) {}
};

// NDD data
struct NddInfo{
  NddData ndd_data;
  eCategory ndd_category;
};

enum P1YuvPath {
  kP1FdPath = 0x0001,
  kP1DepthPath = 0x0010,
  kP1PreviewYuv = 0x0100,
};

/**
 * Control mode hint, gives to default value
 * kControlModeOff(0).
 *  @kControlModeOff: Disable DIP DCE for cts_TEST pass
 *  @kControlModeOn
 */
enum ControlMode {
  kControlModeOff = 0,
  kControlModeOn,
};

/**
 * Edge mode. gives to default value
 * kEdgeModeOn(0).
 *  @kEdgeModeOn: Enabel edge enhancement to ISP
 *  @kEdgeModeOff:Disable edge enhancement to ISP
 */
enum EdgeMode {
  kEdgeModeOn = 0,
  kEdgeModeOff,
};

/**
 * Noise reduction mode. gives to default value
 * kNoiseReductionModeOn(0).
 *  @kNoiseReductionModeOn: Enabel noise reduction to ISP
 *  @kNoiseReductionModeOff:Disable noise reduction to ISP
 */
enum NoiseReductionMode {
  kNoiseReductionModeOn = 0,
  kNoiseReductionModeOff,
};

/**
 * Color correction mode. gives to default value
 * kColorCorrectionModeAuto(0).
 *  @kColorCorrectionModeAuto:   Set color colorection matrix by tuning.
 *  @kColorCorrectionModeManual: Use the user color colorection matrix
 */
enum ColorCorrectionMode {
  kColorCorrectionModeAuto = 0,
  kColorCorrectionModeManual,
};

/**
 * Color correction transform.
 *  manual color correction matrix if ColorCorrectionMode is 1
 *  @mat: 3 x 3 matrix
 *   _                _
 *  | mat0  mat1  mat2 |
 *  | mat3  mat4  mat5 |
 *  |_mat6  mat7  mat8_|
 */
struct ColorCorrectionTransform {
  float mat[9];
};

/**
 * Sensor test pattern mode. gives to default value
 * kSensorTestPatternModeOff(0).
 *  @kSensorTestPatternModeOff: Set color colorection matrix by tuning.
 *  @kSensorTestPatternModeOn: Use the user color colorection matrix
 */
enum SensorTestPatternMode {
  kSensorTestPatternModeOff = 0,
  kSensorTestPatternModeOn,
};

/**
 * Tone Map mode. gives to default value
 * kToneMapModeAuto(0).
 *  @kToneMapModeAuto:  Set tone map curve by tuning.
 *  @kToneMapModeMaual: Use the user tone map curve.
 */
enum ToneMapMode {
  kToneMapModeAuto = 0,
  kToneMapModeMaual,
};

/**
 * Size of toneMap curve in byte
 * Only Support linear toneMap curve
 */
enum { ISP_TONE_MAP_CURVE_SIZE = 101 };

/**
 * Tone map curve.
 *  manual tone map curve if ToneMapMode is 1
 * red:     red channel for range 0~1, control points (x, y)
 * green: green channel for range 0~1, control points (x, y)
 * blue:   blue channel for range 0~1, control points (x, y)
 */
struct ToneMapCurve {
  int32_t red_Cnt;
  float red_X[ISP_TONE_MAP_CURVE_SIZE];
  float red_Y[ISP_TONE_MAP_CURVE_SIZE];
  int32_t green_Cnt;
  float green_X[ISP_TONE_MAP_CURVE_SIZE];
  float green_Y[ISP_TONE_MAP_CURVE_SIZE];
  int32_t blue_Cnt;
  float blue_X[ISP_TONE_MAP_CURVE_SIZE];
  float blue_Y[ISP_TONE_MAP_CURVE_SIZE];
} MTK_ISPHAL_ALIGN_DEFAULT;

/**
 * Tuning update mode.
 *  @kTuningUpdModeDefault: Default mode, usually caller gives this.
 *  @kTuningUpdModePartialKeep: keep existed parameters but some parts will be
 * updated;
 *  @kTuningUpdModeAllKeep: keep all existed parameters (force mode)
 *  @kTuningUpdModeLPCNR1: Special case for LPCNR Pass1
 *  @kTuningUpdModeLPCNR2: Special case for LPCNR Pass2
 */
enum TuningUpdateMode {
  kTuningUpdModeDefault = 0,
  kTuningUpdModePartialKeep = 1,
  kTuningUpdModeAllKeep = 2,
  kTuningUpdModeLPCNR8Bit_1 = 3,
  kTuningUpdModeLPCNR8Bit_2 = 4,
  kTuningUpdModeCShots = 5,
  kTuningUpdModeLPCNR10Bit_1 = 6,
  kTuningUpdModeLPCNR10Bit_2 = 7,
  kTuningUpdModeIdenditySetting = 8,
  kTuningUpdModeDSDNSetting = 9,
};

/**
 * TNRFWConfig
 * TO-DO add comment for this structure
 */
struct TNRFWConfig {
  int32_t scaleIndex;
  int32_t totalScaleNo;
  int32_t frameIndex;
  int32_t frameTotal;
  int32_t fallbackRate;
  int32_t bInkMode;
} MTK_ISPHAL_ALIGN_DEFAULT;

/**
  * Scenario Recorder.
  */
struct scenarioRecordParam {
  int32_t enable;
  NSCam::IMetadata* pHalMeta;
  NSCam::TuningUtils::scenariorecorder::DecisionInput decision_param;
  NSCam::TuningUtils::scenariorecorder::ExecResultInput result_param;
};

struct DUAL_ISP_P1_SYNC_INFO_T {
  bool bSync2AMode;
  bool bSlave_P1;
};

// ISP control enum
enum kISPCtrl_HalIsp_T {
  kISPCtrl_HalIsp_GetP2TuningInfo = 0,
  kISPCtrl_HalIsp_GetLCEGain,
  kISPCtrl_HalIsp_GetAINRParam,
  kISPCtrl_HalIsp_GetMsfTuning_With_Luma,
  kISPCtrl_HalIsp_GetMaxRrzRatio,
  kISPCtrl_HalIsp_SetDynamicBypass,
  kISPCtrl_HalIsp_SetDynamicCCM,
  kISPCtrl_HalIsp_GetDynamicCCM,
  kISPCtrl_HalIsp_PutCCM,
  kISPCtrl_HalIsp_GetCCM,
  kISPCtrl_HalIsp_SetCCTOBCEnable,
  kISPCtrl_HalIsp_IsCCTOBCEnable,
  kISPCtrl_HalIsp_SetCCTBPCEnable,
  kISPCtrl_HalIsp_IsCCTBPCEnable,
  kISPCtrl_HalIsp_SetCCTDMEnable,
  kISPCtrl_HalIsp_IsCCTDMEnable,
  kISPCtrl_HalIsp_SetCCTLDNREnable,
  kISPCtrl_HalIsp_IsCCTLDNREnable,
  kISPCtrl_HalIsp_SetCCTCCMEnable,
  kISPCtrl_HalIsp_IsCCTCCMEnable,
  kISPCtrl_HalIsp_SetCCTYNREnable,
  kISPCtrl_HalIsp_IsCCTYNREnable,
  kISPCtrl_HalIsp_SetCCTGGMEnable,
  kISPCtrl_HalIsp_IsCCTGGMEnable,
  kISPCtrl_HalIsp_SetCCTEEEnable,
  kISPCtrl_HalIsp_IsCCTEEEnable,
  kISPCtrl_HalIsp_SetCCTCNREnable,
  kISPCtrl_HalIsp_IsCCTCNREnable,
  kISPCtrl_HalIsp_SetCCTCOLOREnable,
  kISPCtrl_HalIsp_IsCCTCOLOREnable,
  kISPCtrl_HalIsp_SetCCTLPCNREnable,
  kISPCtrl_HalIsp_IsCCTLPCNREnable,
};

enum ISP_ISO_TYPE {
  kAfterFUS,
  kBeforeFUS_R1,
  kBeforeFUS_R2,
  kBeforeFUS_R3,
  kISO_TYPE_NUM
};

struct ISP_ISO_INFO_T {
  bool valid;
  uint32_t isp_iso_value;
  uint32_t isp_iso_idx[NVRAM_ISP_REGS_ISO_GROUP_NUM];
  uint32_t isp_iso_idx_l[NVRAM_ISP_REGS_ISO_GROUP_NUM];
  uint32_t isp_iso_idx_u[NVRAM_ISP_REGS_ISO_GROUP_NUM];
};

#define CCM_MATRIX_NUM 9

struct ccm_matrix_t {
  int32_t element[CCM_MATRIX_NUM];  // only for matrix
};

typedef enum {
    EISP_HWHDRType_None = 0,
    EISP_HWHDRType_Stagger = 1,
    EISP_HWHDRType_mStreamSENE = 2,
    EISP_HWHDRType_mStreamNESE = 3
}   EISP_HWHDRType_T;

/**
 * Per-frame control for ISP HAL.
 *  @param p1_yuv_port; this number indict direct yuv port on
 *      mtk hw diagram
 *  @param iso_value Real ISO information to hint ISP hal to generate the
 *      related settings.
 *  @param bYUV_after_rrz; a flag to decide that process yuv on rrzo
 */
struct IspPerframeControl {
  uint64_t u8Id;     // magic num
  uint64_t aaaId;    // 3a setting id

  uint32_t ISP_3A_result_id;

  // HQC info for exif
  bool is_enable_HQC;
  uint32_t HQC_3A_stt_id;

  uint64_t user_id;  // user id

  bool keep_tuning;  // old fgNeedKeepP1
  int32_t i4_sensor_id;

  bool is_camsys_reprocess;

  int32_t app_iso_value;
  int32_t i4ZoomRatio_x100;
  bool fgFDEnable;

  bool qrm_enable;
  bool qbpc_enable;

  bool mock_camsys;
  bool hdr10_enable;

  int32_t multi_frame_bss_index;
  mtk::isphal::Buffer sensor_gyro_status;
  int32_t mvWidth;
  int32_t mvHeight;

  NSIspTuning::CAM_IDX_QRY_COMB_ISP7 rMapping_Info;
  NSIspTuning::CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO rMapping_Info_with_sys_info;

  ISP_ISO_INFO_T isp_iso_info[kISO_TYPE_NUM];

  int32_t frame_rate;

  ImageDescriptor srcimg_descriptor;

  // 3A information
  ae_isp_info_t ae_info;
  awb_isp_info_t awb_info;

  // p1 status config
  uint32_t p1_yuv_port;
  bool bYUV_after_rrz;

  struct NddInfo rNdd_info;

  // mode controls
  ControlMode control_mode;
  EdgeMode edge_mode;
  ColorCorrectionMode color_correction_mode;
  ColorCorrectionTransform color_correction_transform;
  SensorTestPatternMode sensor_test_pattern_mode;
  ToneMapMode tone_map_mode;
  ToneMapCurve tone_map_curve;


  NSIspTuning::EOperMode_T oper_mode;

  bool bSlave;

  // dual sync
  DUAL_ISP_P1_SYNC_INFO_T rP1SyncInfo;

  struct {
    mtk::isphal::Size sTGout;

    // predict final target size
    mtk::isphal::Size targetSize;
  } rCropRzInfo;

  /**
   * HLR info.
   */
  struct {
    bool hlr_en;
  } hlr_info;

  // fd info
  CAMERA_TUNING_FD_INFO_T rFdInfo;

  // ISP_ALG_VER
  struct {
    uint16_t sCCM;
  } isp_alg_ver;

  // obc offset
  int32_t obc_ofst[4];

  // obc gain
  uint32_t obc_gain;

  // dgn gain
  int32_t dgn_gain;

  /**
   * TNC info.
   *  @is_calculate_tnc: flag to check if gtm_curve valid.
   *  @gtm_curve: 17 point gtm curve in uint32_t.
   */
  struct _tnc_info {
    bool isTNCValid;
    int32_t gtm_curve[17];
  } tnc_info;

  /**
   * GGM info.
   *  @is_r1_en: flag to check if ggm_r1 enabled.
   *  @is_t1_en: flag to check if ggm_t1 enabled.
   *  @is_ggm_info_valid: flag to check if ggm_info valid.
   *  @gma_type: gma type info for TNC.
   *  @ggm_xxx: 256 point gamma curve in uint32_t.
   *  @iggm_xxx: 256 point inverse-gamma curve in uint32_t.
   *  @tggm_xxx: 256 point gamma curve in uint32_t for TGGM in TNC.
   *  @tiggm_xxx: 256 point inverse-gamma curve in uint32_t for TIGGM in TNC.
   *  @is_ai3a_en: flag to check if AI-3A flow enabled.
   *  @ggm_r3_lut: 256 point gamma curve in uint32_t from AI-3A for GGM_R3.
   */
  struct _ggm_info {
    enum { COUNT = 256 };
    bool is_r1_en;
    bool is_t1_en;
    bool is_ggm_info_valid;
    uint32_t gma_type;
    uint32_t ggm_lut[COUNT];
    uint32_t ggm_end_var;
    uint32_t iggm_lut[COUNT];
    uint32_t iggm_end_var;
    uint32_t iggm_extreme_end_var;
    uint32_t tggm_lut[COUNT];
    uint32_t tggm_end_var;
    uint32_t tiggm_lut[COUNT];
    uint32_t tiggm_end_var;
    uint32_t tiggm_extreme_end_var;
    // AI-3A
    bool is_ai3a_en;
    uint32_t ggm_r3_lut[COUNT];
  } ggm_info;

  /**
   * LTM info.
   */
  struct _ltm_info {
    enum { COUNT = 27, COUNT_CURVE = 1728, COUNT_CLP = 108 };
    bool ltm_valid;
    uint32_t p1_ltm_data[COUNT];
    uint32_t p1_ltm_curve[COUNT_CURVE];
    uint32_t p1_ltm_clp[COUNT_CLP];
    uint32_t p2_ltm_data[COUNT];
    uint32_t p2_ltm_curve[COUNT_CURVE];
    uint32_t p2_ltm_clp[COUNT_CLP];
    bool ltm_en;
  } ltm_info;

  /**
   * CCM info.
   */

  struct _ccm_info {
    bool is_p1_valid;
    ccm_matrix_t matrix;  // only for matrix
  } ccm_info;

  struct scenarioRecordParam sr_para;

  /**
   * Shading table ISP info.
   *  @ref_width: The width of the image which is the given shading table
   *      generated from.
   *  @ref_height: The height of the image which generates the given shading
   *      table.
   *  @data_valid: The data in this struct is valid or not.
   *  @data: shading table including configuration.
   *  @lsc_datasize: datasize of shading table only.
   */
  struct ShadingIspInfo {
    size_t ref_width;
    size_t ref_height;
    bool data_valid;
    bool p1_shading_enable;
    bool p2_shading_enable;
  } shd_info;

  interpolation_sys_info int_sys_info;

  // module enable dependency (move here for exif)
  bool enable_g2c_d1;
  bool enable_ynr_d1;
  bool enable_cnr_d1;
  bool enable_rpg;
  bool enable_lce;

  int32_t i4ExtSharpnessMode;
  int32_t i4ExtContrastMode;
  int32_t i4ExtSaturationMode;
  int32_t i4ExtBrightnessMode;
  int32_t i4ExtNR2DMode;
  int32_t i4ExtNR3DMode;
  int32_t i4ExtGgmMode;
  int32_t i4ExtLtmMode;
  struct _fw_me_tcy_info {
    enum { COUNT = 82 };
    int32_t stt_req_id;
    int32_t isp_request_num;
    bool fw_tcy_en;
    uint32_t fw_tcy_fst[COUNT];
    bool isPreIspCalculated;
  } fw_me_tcy_info;

  struct _hwhdr_info {
    uint32_t i4fus_num;
    EISP_HWHDRType_T hdr_type;
  } hwhdr_info;

  bool aov_mode;
  bool aov_partial_config;

  // drzs8t crop info (for calculating SLK by P2 driver)
  struct _drzs8t_crop_info {
    bool is_valid;
    NSCam::MRect crop_region;
    NSCam::MSize dst_size;
  } drzs8t_crop_info;

  // youvo down scale info (for calculating DRZH2N by P1 driver)
  struct _yuvo_ds_mode_info {
    int32_t yuvo_r2_ds;
    int32_t yuvo_r4_ds;
  } yuvo_ds_mode_info;

  std::array<int32_t, 64> tnr_tcy_curve;

  IspPerframeControl()
      : u8Id(0),
        aaaId(0),
        ISP_3A_result_id(0),
        is_enable_HQC(false),
        HQC_3A_stt_id(0),
        user_id(0),
        i4_sensor_id(0),
        is_camsys_reprocess(false),
        i4ZoomRatio_x100(100),
        qrm_enable(false),
        qbpc_enable(false),
        mock_camsys(false),
        hdr10_enable(false),
        multi_frame_bss_index(0),
        frame_rate(0),
        p1_yuv_port(0),
        bYUV_after_rrz(true),
        control_mode(kControlModeOn),
        edge_mode(kEdgeModeOn),
        color_correction_mode(kColorCorrectionModeAuto),
        sensor_test_pattern_mode(kSensorTestPatternModeOff),
        tone_map_mode(kToneMapModeAuto),
        oper_mode(NSIspTuning::EOperMode_Normal),
        bSlave(false),
        obc_gain(512),
        dgn_gain(0),
        i4ExtSharpnessMode(0),
        i4ExtContrastMode(0),
        i4ExtSaturationMode(0),
        i4ExtBrightnessMode(0),
        i4ExtNR2DMode(0),
        i4ExtNR3DMode(0),
        i4ExtGgmMode(0),
        i4ExtLtmMode(0),
        aov_mode(false),
        aov_partial_config(false),
        tnr_tcy_curve{} {
    memset(&color_correction_transform, 0, sizeof(ColorCorrectionTransform));
    memset(&tone_map_curve, 0, sizeof(ToneMapCurve));
    memset(&rNdd_info, 0, sizeof(rNdd_info));
    memset(&rFdInfo, 0, sizeof(CAMERA_TUNING_FD_INFO_T));
    memset(&(obc_ofst[0]), 0, sizeof(int32_t) * 4);
    memset(&ggm_info, 0, sizeof(ggm_info));
    memset(&ltm_info, 0, sizeof(ltm_info));
    memset(&ccm_info, 0, sizeof(ccm_info));
    memset(&fw_me_tcy_info, 0, sizeof(fw_me_tcy_info));
    memset(&rCropRzInfo, 0, sizeof(rCropRzInfo));
    memset(&hlr_info, 0, sizeof(hlr_info));
    memset(&sr_para, 0, sizeof(sr_para));
    memset(&shd_info, 0, sizeof(shd_info));
    memset(&hwhdr_info, 0, sizeof(hwhdr_info));
    memset(&tnc_info, 0, sizeof(tnc_info));
    memset(&yuvo_ds_mode_info, 0, sizeof(yuvo_ds_mode_info));
    memset(&int_sys_info, 0, sizeof(interpolation_sys_info));
    // default mapping info
    rMapping_Info.eFeature = NSIspTuning::EFeature_Preview;
    rMapping_Info.eStage = NSIspTuning::EStage_P1;
    rMapping_Info.eCustomFeature = NSIspTuning::ECustomFeature_OFF;
    rMapping_Info.eSensorFeature = NSIspTuning::ESensorFeature_OFF;
  }
} MTK_ISPHAL_ALIGN_DEFAULT;

/**
 * Per-frame control for ISP HAL especially size greater than 10k
 *  @lsc_info: shading info from 3a
 */

struct IspReadOnlyControl {
  lsc_isp_info_t lsc_info;

  /**
   * CAC info.
   *  @cac_t1_reg: register settings of cac_t1.
   *  @cac_t1_param: software settings of cac_t1.
   */
  struct _cac_info {
    enum { COUNT_REG = 8, COUNT_PARAM = 12503 };
    bool cac_enable;
    uint32_t cac_t1_reg[COUNT_REG];
    uint32_t cac_t1_param[COUNT_PARAM];
  } cac_info;

  IspReadOnlyControl() {
    memset(&cac_info, 0, sizeof(cac_info));
  }
} MTK_ISPHAL_ALIGN_DEFAULT;

/**
 * Tuning Paramter for DIP.
 *  @param sequence_num The sequnce number of the.
 *  @param is_need_dump_exif Describes that ISP hal shall dump EXIF informatio
 *      or not.
 *
 *  @param rCropRzInfo; rrz info about in/out/crop size and region
 **/
struct IspImgSysControl {
  bool is_capture;

  int32_t sequence_num;
  uint8_t is_need_dump_exif;

  // start of mapping info

  uint32_t stage;
  uint32_t action;

  bool mock_imgsys;

  // end of mapping info

  ImageDescriptor srcimg_descriptor;

  TuningUpdateMode tuing_update_mode;

  bool    bypass_nr;
  NoiseReductionMode nr_mode;

  struct {
    // fw me part
    mtk::isphal::Size sMEL0out;
    mtk::isphal::Size sGyroMv;

    // imgsys input size
    mtk::isphal::Size imgsys_in_size;

    // before warping
    mtk::isphal::Rectangle rBefore_Warp_Crop;
    mtk::isphal::Size rBefore_Warp_Size;
  } rCropRzInfo;

  /**
   * HLR info.
   */
  struct {
    int32_t hlr_p2_fake_ratio;
  } hlr_info;

  int32_t ResizeYUV;

  // TNCS
  struct {
    bool bValid;
    mtk::isphal::Rectangle tncs_in_cropinfo;
    mtk::isphal::Size target_tnc_size;
  } tncs_info;

  // TNC
  struct {
    bool bValid;
    mtk::isphal::Rectangle tnc_in_cropinfo;
  } tnc_roi;

  WrappingParam rWrappingInfo;

  // TNR info
  TNRFWConfig tnr_fw_config;

  // msyuv
  int ds_mode;
  int total_frame_num;

  // for TuningDataProvider::readData<IspGroupType_COLOR> (move here for exif)
  int32_t custom_color_is_capture;
  int32_t smoothColor_FirstTimeBoot;  // m_bSmoothColor_FirstTimeBoot

  // copy from camsys
  NSIspTuning::CAM_IDX_QRY_COMB_ISP7 rMapping_Info;
  NSIspTuning::CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO rMapping_Info_with_sys_info;

  struct NddInfo rNdd_info;
  struct scenarioRecordParam sr_para;

  // fd info
  CAMERA_TUNING_FD_INFO_T rFdInfo_afterWarp;

  // p2_input_crop_info
  struct {
    int32_t enable;
    NSCam::MRect crop_region;
    NSCam::MSize resize_size;
  } imgsys_input_crop_info;


  IspImgSysControl()
    : is_capture(false),
      mock_imgsys(false),
      tuing_update_mode(kTuningUpdModeDefault),
      bypass_nr(false),
      nr_mode(kNoiseReductionModeOn),
      ds_mode(0),
      total_frame_num(1),
      custom_color_is_capture(false),
      smoothColor_FirstTimeBoot(0),
      imgsys_input_crop_info{} {
    memset(&rCropRzInfo, 0, sizeof(rCropRzInfo));
    memset(&hlr_info, 0, sizeof(hlr_info));
    memset(&tncs_info, 0, sizeof(tncs_info));
    memset(&tnc_roi, 0, sizeof(tnc_roi));
    memset(&rWrappingInfo, 0, sizeof(WrappingParam));
    memset(&tnr_fw_config, 0, sizeof(TNRFWConfig));
    memset(&rNdd_info, 0, sizeof(rNdd_info));
    memset(&sr_para, 0, sizeof(sr_para));
    memset(&rFdInfo_afterWarp, 0, sizeof(CAMERA_TUNING_FD_INFO_T));
  }
} MTK_ISPHAL_ALIGN_DEFAULT;

}  // namespace v1_0
}  // namespace isphal
}  // namespace mtk

#endif  // AAA_ISPHAL_INCLUDE_V2_0_ISPHAL_ISPCONTROLS_H_

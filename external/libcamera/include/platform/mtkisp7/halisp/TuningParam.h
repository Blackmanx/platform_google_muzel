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

#ifndef AAA_ISPHAL_INCLUDE_V2_0_ISPHAL_TUNINGPARAM_H_
#define AAA_ISPHAL_INCLUDE_V2_0_ISPHAL_TUNINGPARAM_H_

#include <mtkcam-interfaces/isphal/Buffer.h>  // mtk::isphal::Buffer
#include <mtkcam-interfaces/isphal/IspTuningMeta.h>

#include <mtkcam-interfaces/isphal/Predefines.h>
#include "utils/Size.h"
#include "IspControls.h"

// debug dump
//#include <mtkcam-interfaces/utils/TuningUtils/FileDumpNamingRule.h>

#include <cstdint>  // uint32_t, int32_t
#include <bitset>   // std::bitset
#include <vector>   // std::vector
#include <string>   // std::string

namespace mtk {
namespace isphal {
namespace v1_0 {


/**
 * Mediatek ISP supports frontal binning function, if caller provided this
 * information, the ISP hal may provide much better IQ ISP setting.
 */
enum FrontalBinInfo {
    kFrontalBinUnknown = 0,
    kFrontalBinEnabled,
    kFrontalBinDisabled,
};

/**
 * Catprue mode hint, if the request is not capture, gives to default value
 * kCaptureModeNone(0).
 *  @kCaptureModeNode: None capture mode.
 *  @kCaptureModeNormal: Normal case capture.
 */
enum CaptureMode {
    kCaptureModeNone = 0,
    kCaptureModeNormal,
};

static_assert(sizeof(float) == 4, "size of float is supposed to be 4 bytes");

struct HalIspInputSetup {
  // app meta part
  uint8_t tone_map_mode;
  uint8_t edge_mode;
  uint8_t nr_mode;
};

/**
 * Mediatek ISP driver exif info buffer, maximum size is ISP_EXIF_SIZE.
 */
enum { ISP_EXIF_SIZE = 65404 };
struct ExifInfo {
    bool valid;
    uint8_t padding[3];
    uint8_t data[ISP_EXIF_SIZE];
} MTK_ISPHAL_ALIGN_DEFAULT;

struct ExifInfoP2 {
    bool valid;
    uint32_t size;
    uint8_t* data;
} MTK_ISPHAL_ALIGN_DEFAULT;

struct ExifInfo3A {
    uint32_t size;
    const uint8_t* data;
} MTK_ISPHAL_ALIGN_DEFAULT;

/**
 * Control Tuning Paramters for Mediatek P1 driver.
 *  @par
 *  @param magic_num The identify ID.
 *  @param is_need_exif; a flag indicate that need to dump exif
 *  @param capture_mode; indicate this frame is a capture frame
 *  @param cam_info; The given cam_info setup by ihalisp
 *      as the input of getCamSysMetaTuning, and its also the result of p1
 *  // TODO:
 */
struct TuningParamP1 {
    uint64_t magic_num;
    uint64_t aaa_magic_num;
    int32_t  subsample_count;
    uint8_t  is_need_exif;

    IspTuningCamsysControl tuning_control;

    mtk::isphal::v1_0::CaptureMode capture_mode;
    mtk::isphal::v1_0::IspPerframeControl* cam_info; // todo, mostly updated by isphal
    mtk::isphal::v1_0::IspReadOnlyControl* cam_info_3a; // todo, mostly updated by isphal

    // params for p1 reprocessing
    const uint8_t* shading_table;
    uint32_t shading_table_size;

    const uint8_t* pModulesCtrl;
    uint32_t modules_ctrl_size;

    // debug string for ndd
    string debugPreFix;

    // TODO(MTK): Yang check the purpose of this member
    mtk::isphal::v1_0::FrontalBinInfo frontal_bin_info;
    // Need to set p1 3a tuning, and get hal result ?

    const IspTuningStatisticsP1 *tuning_stat; // todo
} MTK_ISPHAL_ALIGN_DEFAULT;


/**
 * The output data of ISP Hal
 *
 *  @param stt_bufs Statistic buffers for output, if the feature was disabled
 *      (or not support), the related buffer would be nullptr.
 *  @param isp_func_refine The ISP function refine set which allows user to
 *      refine the enable of the ISP function(s).
 *  @param exif The EXIF, maybe not updated if the given configuration says
 *      there's no need to update EXIF.
 *  @param tuning_data The result to P1 ISP hardware, see struct IspTuningBuffer
 *      for more information.
 *  @color_correction_transform a transform matrix used for color correction currently
 *  @cam_info: pipeline config carry to DIP
 */
struct ReturnParamP1 {
    mtk::isphal::IspTuningBufferP1* tuning_data;
    mtk::isphal::v1_0::ExifInfo exif;
    mtk::isphal::v1_0::ColorCorrectionTransform color_correction_transform;
    uint64_t meta_size;
} MTK_ISPHAL_ALIGN_DEFAULT;

/**
 * ISP special bit setting for AI-NR or customer's flow.
 *
 *  @enable Flag to check if is special bit.
 *  @hlr_bit The bit for hlr.
 *  @ltm_bit The bit for ltm.
 */
struct SpecialBitInfo {
    bool enable;
    uint8_t hlr_bit;
    uint8_t ltm_bit;
    SpecialBitInfo()
      : enable(false), hlr_bit(12), ltm_bit(12) {}
} MTK_ISPHAL_ALIGN_DEFAULT;

/**
 * Tuning Paramter for DIP.
 *
 *  @param magic_num The identify ID.
 *  @param is_need_exif Describes that ISP hal shall fill up EXIF informatio
 *      or not.
 *  @param capture_mode; indicate this frame is a capture frame

 *  @param cam_info; The given cam_info setup by ihalisp
 *      as the input of getImgSysMetaTuning
 */
struct TuningParamDip {
    bool    is_batch;
    uint8_t is_need_exif;

    mtk::isphal::v1_0::CaptureMode capture_mode;

    mtk::isphal::v1_0::HalIspInputSetup camsys_history;

    mtk::isphal::v1_0::IspPerframeControl cam_info; // where?
    const mtk::isphal::v1_0::IspReadOnlyControl* cam_info_3a; // where?
    const uint8_t* shading_table;
    uint32_t shading_table_size;

    const uint8_t* pModulesCtrl;
    uint32_t modules_ctrl_size;

    std::vector<mtk::isphal::v1_0::IspImgSysControl> imgsys_info; // where?
    std::vector<const IspTuningStatisticsP2*> tuning_stat; // where?

    mtk::isphal::v1_0::ExifInfo3A exif_3a;

    mtk::isphal::v1_0::SpecialBitInfo bit_info;

    TuningParamDip()
      : is_batch(0),
        is_need_exif(0),
        capture_mode(kCaptureModeNone) {
    }
} MTK_ISPHAL_ALIGN_DEFAULT;  // struct TuningParam


/**
 * The output data of ISP Hal
 *  @param tuning_data The result to DIP ISP hardware, see struct IspTuningBuffer for
 *          more information.
 *  @param exif The EXIF, maybe not updated if the given configuration says
 *      there's no need to update EXIF.
 *  @param stt_bufs Statistic buffers for output, if the feature was disabled
 *      (or not support), the related buffer would be nullptr.
 *  @param tone_map_curve; To report current tone map curve for android meta
 */
struct ReturnParamDip {
    std::vector<mtk::isphal::IspTuningBufferP2 *> tuning_data;
    mtk::isphal::v1_0::ExifInfoP2 exif;
    mtk::isphal::v1_0::ToneMapCurve tone_map_curve;
    uint64_t meta_size;
} MTK_ISPHAL_ALIGN_DEFAULT;

}  // namespace v1_0
}  // namespace isphal
}  // namespace mtk

#endif  // AAA_ISPHAL_INCLUDE_V2_0_ISPHAL_TUNINGPARAM_H_


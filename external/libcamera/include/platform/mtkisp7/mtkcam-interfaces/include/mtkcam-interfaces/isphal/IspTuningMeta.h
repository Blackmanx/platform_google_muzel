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

#ifndef INCLUDE_MTKCAM_INTERFACES_ISPHAL_ISPTUNINGMETA_H_
#define INCLUDE_MTKCAM_INTERFACES_ISPHAL_ISPTUNINGMETA_H_

#include <mtkcam-interfaces/isphal/Buffer.h>      // mtk::isphal::Buffer
#include <mtkcam-interfaces/isphal/ImageBuffer.h>  // mtk::isphal::ImageBuffer
#include <mtkcam-interfaces/isphal/Predefines.h>  // MTK_ISPHAL_ALIGN_DEFAULT


#include <cstdint>  // uint32_t, int32_t
#include <bitset>   // std::bitset
#include <unordered_map>
#include <map>
#include <vector>
#include <utility>      // std::pair, std::make_pair
#include <tuple>        // std::tuple


namespace mtk {
namespace isphal {
/*
 * Describes an
 *  extension buffer type
 */
enum kISPExtBuf {
  kISPExtBif_Begin = 0,

  // ISP
  kISPExtBif_DualSyncInfo = 0x0001,
  kISPExtBif_CcmSetting,
  kISPExtBif_LtmSetting,
  kISPExtBif_TncSetting,
  kISPExtBif_AwbSetting,
  kISPExtBif_IN_HWME_STAT_0,    // Remove after UNPACK STT done
  kISPExtBif_IN_HWME_STAT_1,    // Remove after UNPACK STT done
  kISPExtBif_IN_HWME_STAT_FST_MD0,
  kISPExtBif_IN_HWME_STAT_FMB_MD0,
  kISPExtBif_IN_HWME_STAT_FST_MD1,
  kISPExtBif_IN_HWME_MODE_0_TUN_BUF,
  kISPExtBif_IN_FWME_FST,
  kISPExtBif_IN_FWMM_MMG_FBFST,
  kISPExtBif_IN_FWMM_MMG_RST,
  kISPExtBif_IN_WPE_BW_INFO,
  kISPExtBif_IN_GYRO_MV,
  kISPExtBif_IN_WARP_MAP_X,
  kISPExtBif_IN_WARP_MAP_Y,
  kISPExtBif_OUT_FWME_FST,
  kISPExtBif_OUT_FWMM_MMG_FBFST,
  kISPExtBif_OUT_FWMM_MMG_RST,
  kISPExtBif_OUT_FWMM_MIL,
  kISPCtrl_Num
};

/*
 * Describes a
 *  report status
 */
enum kISPStatus {
  kISPStatus_Begin = 0,
  kISPStatus_FUS_ON = 0x0001,
  kISPStatus_Num
};

/*
 * Describes a struct for kISPExtBif_LtmSetting
 * in reserved of IspTuningStatisticsP2.
 * caller has responsibility to prepare it.
 *  @LTMCurve first 32 point of curve.
 *  @LTMClp last point of curve.
 *  @remian other registers of LTM.
 */
struct UserSettingLtm {
  uint32_t LTMCurve[1728];
  uint32_t LTMClp[108];
  uint32_t LTM_CLIP_TH_SW;
  uint32_t LTM_CLIP_TH_HW;
  uint32_t LTM_MAP_LOG_EN;
  uint32_t LTM_WGT_LOG_EN;
  uint32_t LTM_NONTRAN_MAP_TYPE;
  uint32_t LTM_TRAN_MAP_TYPE;
  uint32_t LTM_TRAN_WGT_TYPE;
  uint32_t LTM_TRAN_WGT;
  uint32_t LTM_RANGE_SCL;
  uint32_t LTM_GAIN_TH;
  uint32_t LTM_BLK_X_NUM;
  uint32_t LTM_BLK_Y_NUM;
} MTK_ISPHAL_ALIGN_DEFAULT;

struct UserSettingTnc {
/* info for shutter2.0 */
  void* aishutter2_info = nullptr;
  uint32_t aishutter2_info_size = 0; /* invalid if 0 */
} MTK_ISPHAL_ALIGN_DEFAULT;

struct UserSettingAwb {
/* info for shutter2.0 */
  uint32_t gain_r;
  uint32_t gain_g;
  uint32_t gain_b;
  uint32_t debug_info[39];
} MTK_ISPHAL_ALIGN_DEFAULT;

/*
 * Describes a size,
 * caller has responsibility to prepare it.
 *  @width of the region.
 *  @height of the region.
 */
struct size {
  uint32_t width;
  uint32_t height;
};

/*
 * Describes a crop region,
 * caller has responsibility to prepare it.
 *  @offset_x x direction offset.
 *  @offset_y y direction offset.
 *  @region_w region width.
 *  @region_h region height.
 */
struct CropRegion {
  uint32_t offset_x;
  uint32_t offset_y;
  size     region;
} MTK_ISPHAL_ALIGN_DEFAULT;

/*
 * Describes a image info and port name,
 *  @ImageBuffer: provide info about image contain.
 *  @uint32_t:: provide port name about this image.
 *    ref. to header of camsys driver
 *      - mtkcam-interfaces
 *        isp/6s/include/mtkcam-interfaces/
 *        hw/camsys/pipe_mgr/PipeMgrDefs.h
 *      - enum NodeId
 *    ref. to header of imgsys driver
 *      - mtkcam-core/
 *        include/mtkcam-core/hw/imgstream/ImgPortDef.h
 *      - enum IMG_PORT
 */
typedef std::pair<mtk::isphal::ImageBuffer,
                  uint32_t> ImageDescriptorPair;

/*
 * Describes a tuning buffer,
 *  caller has responsibility to prepare it.
 *  @camsys_statistics Statistics from p1.
 *  @reserved extension buffers
 *  @note that the size of buffer is in bytes,
 *  *** which is diff with previous platform ***
 */
struct IspTuningStatisticsP1 {
  mtk::isphal::Buffer camsys_statistics;

  std::unordered_map<mtk::isphal::kISPExtBuf,
    mtk::isphal::Buffer> reserved;
} MTK_ISPHAL_ALIGN_DEFAULT;

/*
 * Describes a control method for camsys,
 * caller has responsibility to prepare it.
 *  @fgForce: prepare a setting for dummy frame
 *  @fgDue:   prepare a setting which is the same as previous frame
 */
struct IspTuningCamsysControl {
  bool fgForce = 0;
  bool fgDue   = 0;
} MTK_ISPHAL_ALIGN_DEFAULT;

/*
 * Describes a tuning buffer,
 * caller has responsibility to prepare it.
 *  @param [in] out_image: output image of imgsys driver
                see "ImageDescriptorPair" for more info
 *  @param [out] out_image_rzinfo:
 *               resize info and crop info
 *               about this output of image
 *  @param [out] mux_select_info:
 *               output map for the selection result
 *               <port id, select result>
 *               See ImageDescriptorPair for port id
 *               ref. to header of camsys driver
                   - mtkcam-interfaces
 *                   isp/7/include/mtkcam-interfaces/
 *                   hw/camsys/pipe_mgr/PipeMgrDefs.h
 *                 - enum MainStreamPathControl
 *  @param [out] p1_meta_buffer:
 *               output config for imgsys driver
 */
struct IspTuningBufferP1 {
  // image descriptor, kIspInPortInfo
  mtk::isphal::ImageDescriptorPair in_image;

  std::vector<mtk::isphal::ImageDescriptorPair> out_image;

  // Resize info, crop info
  std::vector<std::pair<mtk::isphal::size,
      mtk::isphal::CropRegion>> out_image_rzinfo;

  std::map<uint32_t, uint32_t> mux_select_info;

  std::unordered_map<mtk::isphal::kISPStatus, uint32_t>
    isp_status;

  mtk::isphal::Buffer p1_meta_buffer;
} MTK_ISPHAL_ALIGN_DEFAULT;

/*
 * Describes a mode
 *  of update in isp hal
 */
enum kIspUpdateMode {
  kIspUpdateModeAuto,
  kIspUpdateModeSkip,
  kIspUpdateModeIdentity,
  kIspUpdateModeWithoutHistory
};

/*
 * Describes a control method,
 * caller has responsibility to prepare it.
 *  @feature_profile .
 *  @stage
 *  @action e.g. preview/video... value defined at:
 *    "vendor/mediatek/proprietary/custom/<platform>/hal/inc/tuning_mapping/cam_idx_struct_ext.h"
 *  @update_mode ref to kIspUpdateMode
 */
struct IspTuningControl {
  uint32_t stage  = 0;
  uint32_t action = 0;
  bool     mock   = false;
  kIspUpdateMode update_mode = kIspUpdateModeAuto;
} MTK_ISPHAL_ALIGN_DEFAULT;

/*
 * Describes a tuning buffer,
 *  caller has responsibility to prepare it.
 *  @camsys_statistics Statistics from p1.
 *  @imgsys_statistics Statistics from p2.
 *  @reserved extension buffers
 *  @note that the size of buffer is in bytes,
 *  *** which is diff with previous platform ***
 */
struct IspTuningStatisticsP2 {
  mtk::isphal::Buffer camsys_statistics;
  mtk::isphal::Buffer imgsys_statistics;
  mtk::isphal::Buffer imgsys_hist_buffer;
  std::unordered_map<mtk::isphal::kISPExtBuf,
      mtk::isphal::Buffer> reserved;
} MTK_ISPHAL_ALIGN_DEFAULT;

/*
 * Describes a control mode
 *  for PQ dip which would be updated in isp hal
 */
enum kIspPQControlMode {
  kIspPQControlModeAuto,
  kIspPQControlModeOff,
  kIspPQControlModeCnt
};

/*
 * Describes a PQ-DIP control infomation,
 * caller has responsibility to prepare it.
 *  @CropSize .
 *  @OutSize .
 *  @serial_id tuning serial id.
 *  @ctrl : froce control.
 *  @ndd_layer: .
 *  @active_tcc: tcc enable hint.
 */
struct PQInfo {
  size CropSize;
  size OutSize;
  uint32_t serial_id;
  kIspPQControlMode ctrl;
  int ndd_layer = -1;
  bool active_tcc = false;
};

/*
 * kIspWPEType: Kinds of WPE usage
 * Describes a control mode
 *  for param of Warping engine which would be updated in isp hal
 */
enum kIspWPEType {
  kWPE_EIS = 0,  // WPE_E1A
  kWPE_TNR,      // WPE_E1B
  kWPE_LITE,     // WPE_E1C
  kWPE_Num
};

/*
 * Describes a WPE control infomation,
 * caller has responsibility to prepare it.
 *  @buf_id: type of wpe map such as eis, tnr, lite
 *  @is_motion: is motion type param
 */
struct WPEInfo {
  kIspWPEType buf_id;  // WPE 0=A, 1=B, 2=C
  bool is_motion;
};

/*
 * Describes a output tuning buffer config,
 *  caller has responsibility to prepare it.
 *  @param [in] in_image: input image of imgsys driver
                see "ImageDescriptorPair" for more info
 *  @param [in] out_image: output image of imgsys driver
                see "ImageDescriptorPair" for more info
 *  @param [in] pq_info: see "PQInfo" for more info.
 *  @param [in] wpe_info: see "WPEInfo" for more info.
 *  @param [out] mux_select_info:
 *               output map for the selection result
 *               <port id, select result>
 *               See ImageDescriptorPair for port id
 *               ref. to header of camsys driver
 *                 - mtkcam-core
 *                   /include/mtkcam-core/hw/imgstream/IImgStreamDef.h
 *                 - enum IMG_OUTPUT_SEL_ENUM or IMG_INPUT_SEL_ENUM
 *  @param [out] p2_meta_buffer:
 *               output config for imgsys driver
 */
struct IspTuningBufferP2 {
  mtk::isphal::ImageDescriptorPair in_image;

  std::vector<mtk::isphal::ImageDescriptorPair> out_image;

  std::vector<PQInfo> pq_info;

  std::vector<WPEInfo> wpe_info;

  std::map<uint32_t, uint32_t> mux_select_info;

  mtk::isphal::Buffer p2_meta_buffer;
} MTK_ISPHAL_ALIGN_DEFAULT;


}   // namespace isphal
}   // namespace mtk

#endif  // INCLUDE_MTKCAM_INTERFACES_ISPHAL_ISPTUNINGMETA_H_


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

#ifndef INCLUDE_MTKCAM_INTERFACES_ISPHAL_IHALISPADAPTER_H_
#define INCLUDE_MTKCAM_INTERFACES_ISPHAL_IHALISPADAPTER_H_

// For meta data
#include <mtkcam-interfaces/utils/metadata/IMetadata.h>

// For meta data set
#include <mtkcam-interfaces/aaahal/aaa_hal_ctrl/IHal3ACommon.h>
#include <mtkcam-interfaces/isphal/IspTuningMeta.h>
#include <mtkcam-interfaces/hw/external/ExternalIspDefs.h>  // NSCam::external::IspDataResult
#include <mtkcam-interfaces/hw/camsys/P1_dma.h>

// For MRect/MSize
#include <mtkcam-interfaces/def/common.h>

// std c++
#include <vector>
#include <memory>
#include <tuple>

// These eum would be deprecated in isp 7.0
enum kISPCtrl_T {
  kISPCtrl_Begin = 0,
  // ISP

  kISPCtrl_GetIspGamma = 0x0001,
  kISPCtrl_ValidatePass1,
  kISPCtrl_SetIspProfile,
  kISPCtrl_GetOBOffset,
  kISPCtrl_GetRwbInfo,
  kISPCtrl_SetOperMode,
  kISPCtrl_GetOperMode,
  kISPCtrl_GetMfbSize,
  kISPCtrl_GetMfbTblSize,
  kISPCtrl_SetLcsoParam,
  kISPCtrl_GetLtmCurve,
  kISPCtrl_GetP2TuningInfo,
  kISPCtrl_GetMssTuningInfo,
  kISPCtrl_NotifyCCU,
  kISPCtrl_WTSwitch,
  kISPCtrl_GetLCEGain,
  kISPCtrl_GetAINRParam,
  kISPCtrl_SetFdSource,
  kISPCtrl_NotifyP1CQDone,
  kISPCtrl_GetMsfTuning_With_Luma,
  kISPCtrl_GetMaxRrzRatio,
  kISPCtrl_GetPureRaw,
  kISPCtrl_GetHDRISO,
  kISPCtrl_Num
};

using mtk::hal3a::v1_0::mtk_hal3a_metaset;

struct mtk_isp_config {
  // V4L2 param
  /************ Common ************/
  uint32_t sensor_dev;
  uint32_t sensor_idx;
  /************ sensor info  ************/
  uint32_t tg_width;
  uint32_t tg_height;

  uint32_t sub_sample_count;
  uint32_t direct_yuv_path;
  bool yuv_after_rrz;
  NSCam::IMetadata _appMeta;
  NSCam::IMetadata _halMeta;
};

struct mtk_halisp_metaset {
  bool dummy;
  NSCam::IMetadata* appMeta;
  NSCam::IMetadata* halMeta;

  mtk_halisp_metaset() : dummy(0), appMeta(NULL), halMeta(NULL) {}

  mtk_halisp_metaset(bool _dummy,
                     NSCam::IMetadata* _appMeta,
                     NSCam::IMetadata* _halMeta)
      : dummy(_dummy), appMeta(_appMeta), halMeta(_halMeta) {}

  mtk_halisp_metaset(NSCam::IMetadata* appMeta, NSCam::IMetadata* halMeta)
      : mtk_halisp_metaset(false, appMeta, halMeta) {}
};

struct mtk_imgsys_halisp_config {
  mtk_halisp_metaset control_imgsys;
  mtk::isphal::IspTuningControl tuning_control;
  mtk::isphal::IspTuningStatisticsP2 tuning_statistics;
  mtk::isphal::IspTuningBufferP2 tuning_data;
};

struct mtk_isp_buf_info {
  size_t lceso_size = 0;
  size_t lcesho_size = 0;
  size_t dceso_size = 0;
  size_t camsys_stat_size = 0;
  size_t camsys_meta_size = 0;
  size_t camsys_meta_version = 0;
  size_t imgsys_stat_size = 0;
  size_t imgsys_hist_buf_size = 0;
  size_t imgsys_meta_size = 0;
  size_t imgsys_meta_version = 0;
  size_t hwme_stat_fst_size = 0;
  size_t hwme_stat_fmb_size = 0;
  size_t hwme_stat_lmi_size = 0;
  size_t fwme_fst_meta_size = 0;
  size_t fwmm_mmg_fbfst_meta_size = 0;
  size_t fwmm_mmg_rst_meta_size = 0;
  size_t fwmm_mil_meta_size = 0;
};

struct mtk_frame_info {
  std::vector<std::tuple<int, NSCam::MRect, NSCam::MSize>> portCropInfo;
  // Data from external ISP
  std::shared_ptr<NSCam::external::IspDataResult> pIspDataResult;
};

struct mtk_yuvo_info {
  // reference from mtkcam-interfaces/hw/camsys/P1_dma.h
  int32_t port;
  int32_t ds_mode;
};

namespace mtk {
namespace ispcf {

class IHalISPAdapter {
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //  Interfaces.
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 public:
  /**
   * Create the latest IHalISPAdapter instance.
   *  @param[in] sensor_dev Sensor device unique ID.
   *  @param[in] sensor index of the given |sensor_dev|.
   *  @return The instance of IHalIsp.
   */
  static std::shared_ptr<mtk::ispcf::IHalISPAdapter>
    createInstance(size_t sensor_idx, uint64_t userId = 0);

  /**
   * @brief init ISP
   * @param [in] strUser: user name in char
   * @return
   * - bool value of TRUE/FALSE.
   */
  virtual bool init4CamSys(const char* strUser, bool mock = false) = 0;

  /**
   * @brief uninit ISP
   * @param [in] strUser: user name in char
   * @return
   * - bool value of TRUE/FALSE.
   */
  virtual bool uninit4Camsys(const char* strUser) = 0;

  /**
   * @brief config ISP setting
   * @param [in] rConfigInfo ISP setting of mtk_isp_config
   * @return
   * - bool value of TRUE/FALSE.
   */
  virtual bool config4Camsys(const mtk_isp_config& rConfigInfo) = 0;

  /**
   * @brief set sensor mode
   * @param [in] i4SensorMode
   * @return
   * - NULL
   */
  virtual void setSensorMode(int32_t i4SensorMode) = 0;

  /**
   * @Prepare ISP settings for Mediatek P1 ISP driver according to the
   * configurations of |requestQ|.
   * @param [in] requestQ: p1 request metadata
   * @param [in] fgForce: dummy request
   * @param [out] resultMeta: result of p1 metadata
   * @return
   * - bool value of TRUE/FALSE.
   */
  virtual bool getCamSysMetaTuning(
      uint64_t frmId, uint64_t aaaFrmId,
      const std::vector<mtk_hal3a_metaset*>& requestQ,
      const mtk::isphal::IspTuningCamsysControl& tuning_control,
      mtk::isphal::IspTuningBufferP1& tuning_data) = 0;

  // this function will be deprecated
  virtual bool getCamSysMetaTuning(
      uint64_t frmId,
      const std::vector<mtk_hal3a_metaset*>& requestQ,
      mtk::isphal::IspTuningBufferP1& tuning_data,
      bool const fgForce = 0) = 0;

  virtual bool getCamSysMetaTuning(
      uint64_t frmId,
      const std::vector<mtk_hal3a_metaset*>& requestQ,
      const mtk::isphal::IspTuningStatisticsP1& tuning_statistics,
      const isphal::IspTuningBufferP1& tuning_data,
      bool const fgForce = 0) = 0;

  /**
   * @Tell hardware setting to isphal and print debug msg
   * configurations of |requestQ|.
   * @param [in] requestQ: p1 request metadata
   * @param [in] fgForce: dummy request
   * @param [out] resultMeta: result of p1 metadata
   * @return
   * - bool value of TRUE/FALSE.
   */
  virtual bool notifyCamSysMetaTuning(
      uint64_t frmId, uint64_t aaaFrmId,
      const std::vector<mtk_hal3a_metaset*>& requestQ,
      const mtk::isphal::IspTuningCamsysControl& tuning_control,
      mtk::isphal::IspTuningBufferP1& tuning_data) = 0;

  /**
   * @Prepare ISP settings for Mediatek P1 ISP driver according to the given
   * configurations of |control|.
   * @param [in] control_from_history: p1 request metadata from history
   * @param [in] control_per_stage: p1 request metadata for each stage
   * @param [in] tuning_control: Import setting corresponding to control_per_stage
   * @param [out] tuning_data: tuning buffer for pass-1 setting
   * @param [out] resultMeta: result of metadata
   * @return
   * - bool value of TRUE/FALSE.
   */
  virtual bool getCamSysReprocMetaTuning(
      const mtk_halisp_metaset& control_from_history,
      const mtk_halisp_metaset& control_per_stage,
      const mtk::isphal::IspTuningControl& tuning_control,
      mtk::isphal::IspTuningBufferP1& tuning_data,
      mtk_halisp_metaset* pResult) = 0;

  /**
   * @brief get ISP setting
   * @param [in] frmId: want to get the result of setting with frame id
   * @param [out] result: output metadata for p1
   * @return
   * - bool value of TRUE/FALSE.
   */
  virtual bool getResultByFrameID(uint64_t frmId,
      const mtk_frame_info& frame_info,
      mtk_hal3a_metaset* result) = 0;

  /**
   * @brief get ISP setting
   * @param [in] frmId: want to get the result of setting with frame id
   * @param [in] IspTuningBufferP1: ref tuning meta and rrz info
   * @param [out] result: output metadata for p1
   * @return
   * - bool value of TRUE/FALSE.
   */
  virtual bool getResultByFrameID(uint64_t frmId,
                              const mtk::isphal::IspTuningBufferP1& tuning_data,
                              mtk_hal3a_metaset* result) = 0;

  /**
   * @Prepare ISP settings for Mediatek P2 ISP driver according to the given
   * configurations of |control|.
   * @param [in] control: p2 request metadata
   * @param [out] pTuningBuf: tuning buffer for pass-2 setting
   * @param [out] resultMeta: result of p2 metadata
   * @return
   * - bool value of TRUE/FALSE.
   */
  virtual bool getImgSysMetaTuning(
      const mtk_halisp_metaset& control,
      const mtk::isphal::IspTuningStatisticsP2& tuning_statistics,
      const mtk::isphal::IspTuningBufferP2& tuning_data,
      mtk_halisp_metaset* pResult) = 0;

  /**
   * @Prepare ISP settings for Mediatek P2 ISP driver according to the given
   * configurations of |control|.
   * @param [in] control_from_history: pipeline meta with pipeline frame
   * @param [in] control_imgsys: per-stage control meta for imgsys
   * @param [in] tuning_control: control parameter with brief struct
   * @param [in] tuning_statistics: hw statistic or sw working buffer
   * @param [out] pTuningBuf: tuning buffer for pass-2 setting
   * @param [out] resultMeta: result of p2 metadata
   * @return
   * - bool value of TRUE/FALSE.
   */
  virtual bool getImgSysMetaTuning(
      const mtk_halisp_metaset& control_from_history,
      const mtk_halisp_metaset& control_per_stage,
      const mtk::isphal::IspTuningControl& tuning_control,
      const mtk::isphal::IspTuningStatisticsP2& tuning_statistics,
      mtk::isphal::IspTuningBufferP2& tuning_data,
      mtk_halisp_metaset* pResult) = 0;

  /**
   * @Prepare ISP settings for Mediatek P2 ISP driver according to the given
   * configurations of |control|.
   * @param [in] control_from_history: pipeline meta with pipeline frame
   * @param [in/out] tuning_config_set:
   *    see struct "mtk_imgsys_halisp_config"
   *        which is similar to the argument no.2-5 of getImgSysMetaTuning
   * @param [out] resultMeta: result of p2 metadata
   * @return
   * - bool value of TRUE/FALSE.
   */
  virtual bool batchImgSysMetaTuning(
      const mtk_halisp_metaset& control_from_history,
      std::vector<mtk_imgsys_halisp_config>&
        tuning_config_set,
      mtk_halisp_metaset* resultMeta) = 0;

  /**
   * @brief Set FD info to isp
   * @param [in] prFaces: MtkCameraFaceMetadata for setting isp
   * @param [in] FdEN: Tag to check if FDEnable
   * @return
   * - bool value of TRUE/FALSE.
   */
  virtual bool setFDInfo(void* prFaces, bool FdEN = false) = 0;

  /**
   * @brief Query current isp buf information
   * @param [out] bufferInfo
   * @return
   * - bool value of TRUE/FALSE.
   */
  virtual bool queryISPBufferInfo(mtk_isp_buf_info* bufferInfo) = 0;

  /**
   * @brief standard api to Send isp control
   * @param [in] eISPCtrl: control code
   * @param [in] iArg1: input1
   * @param [in] sizeArg1: actual size of arg1
   * @param [in] iArg2: input2
   * @param [in] sizeArg2: actual size of arg2
   * @return 0 for ok, otherwise checks POSIX error code. Basically, if the
   *         control is not support, returns -ENOTSUP.
   */
  virtual int sendIspCtrl(kISPCtrl_T eISPCtrl,
                          intptr_t iArg1,
                          uint32_t sizeArg1,
                          intptr_t iArg2,
                          uint32_t sizeArg2) = 0;

 protected:
  IHalISPAdapter() = default;

  /**
   * Copy and move an instance were forbidden since the IHalISPAdapter was
   * always managed by std::unique_ptr<T>
   */
  IHalISPAdapter(const IHalISPAdapter&) = delete;
  IHalISPAdapter(IHalISPAdapter&&) = delete;

 public:
  virtual ~IHalISPAdapter() = default;
};

}       // namespace ispcf
}       // namespace mtk
#endif  // INCLUDE_MTKCAM_INTERFACES_ISPHAL_IHALISPADAPTER_H_

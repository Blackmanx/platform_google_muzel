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

#ifndef AAA_ISPHAL_SRC_INCLUDE_PLUGIN_IHALISPPLUGIN_H_
#define AAA_ISPHAL_SRC_INCLUDE_PLUGIN_IHALISPPLUGIN_H_

// For meta data
#include <mtkcam-interfaces/utils/metadata/IMetadata.h>

// For meta data set
//#include <isphal/TuningParam.h>
#include <mtkcam-interfaces/aaahal/aaa_hal_ctrl/IHal3ACommon.h>
#include <mtkcam-interfaces/isphal/IHalISPAdapter.h>
#include <mtkcam-interfaces/isphal/IspTuningMeta.h>

// std c++
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace mtk {
namespace ispcf {

enum kISPPLUGIN_BUF_T {
  kISPPLUGIN_UNKNOWN_BUF = 0,
  kISPPLUGIN_LTM_BUF,
  kISPPLUGIN_BIT_INFO
};

typedef struct {
  mtk::isphal::v1_0::IspPerframeControl* cam_info;
  mtk::isphal::v1_0::IspReadOnlyControl* cam_info_3a;
} ISP_CAMERA_INFO;

class IHalIspPlugin {
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //  Interfaces.
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 public:
  virtual bool isMatchPluginCamSys(
      const std::tuple<int, int, int, int>& method,
      const mtk_hal3a_metaset& control_from_camsys) = 0;

  virtual bool isMatchPluginCamSysReproc(
      const std::tuple<int, int, int, int>& method,
      const mtk_halisp_metaset& control_from_camsys,
      const mtk_halisp_metaset& control_imgsys,
      const mtk::isphal::IspTuningControl& tuning_control) = 0;

  virtual bool isMatchPluginImgSys(
      const std::tuple<int, int, int, int>& method,
      const mtk_halisp_metaset& control_from_camsys,
      const mtk_halisp_metaset& control_imgsys,
      const mtk::isphal::IspTuningControl& tuning_control) = 0;

  virtual bool doImgSysPreProc(
      const mtk_halisp_metaset& control_from_camsys,
      const mtk_halisp_metaset& control_imgsys,
      const mtk::isphal::IspTuningControl& tuning_control,
      const mtk::isphal::IspTuningStatisticsP2& tuning_statistics,
      const mtk::isphal::IspTuningBufferP2& tuning_data,
      std::pair<std::string,
                std::vector<std::pair<kISPPLUGIN_BUF_T, mtk::isphal::Buffer>>>*
          result) = 0;

  virtual bool doImgSysPostProc(
      const mtk_halisp_metaset& control_from_camsys,
      const mtk_halisp_metaset& control_imgsys,
      const mtk::isphal::IspTuningControl& tuning_control,
      const mtk::isphal::IspTuningStatisticsP2& tuning_statistics,
      const mtk::isphal::IspTuningBufferP2& tuning_data,
      mtk::isphal::v1_0::ExifInfoP2& exif) = 0;

  virtual bool updateCamsysResult(uint64_t frmId,
                                  const mtk_frame_info& frame_info,
                                  ISP_CAMERA_INFO& ispCameraInfo,
                                  mtk_hal3a_metaset* result) = 0;

  virtual bool doCamSysPostProc(
      uint64_t frmId,
      const std::vector<mtk::hal3a::v1_0::mtk_hal3a_metaset*>& requestQ,
      const isphal::IspTuningCamsysControl& tuning_control,
      isphal::IspTuningBufferP1& tuning_data) = 0;

  virtual bool doCamSysReprocPostProc(
      const mtk_halisp_metaset& control_from_camsys,
      const mtk_halisp_metaset& control_imgsys,
      const mtk::isphal::IspTuningControl& tuning_control,
      mtk::isphal::IspTuningBufferP1& tuning_data) = 0;

 protected:
  IHalIspPlugin() = default;

  /**
   * Copy and move an instance were forbidden since the IHalIspPlugin was
   * always managed by std::unique_ptr<T>
   */
  IHalIspPlugin(const IHalIspPlugin&) = delete;
  IHalIspPlugin(IHalIspPlugin&&) = delete;

 public:
  virtual ~IHalIspPlugin() = default;
};

class IHalIspPluginFactory {
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //  Interfaces.
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 public:
  virtual std::shared_ptr<IHalIspPlugin>
    createPlugin(int32_t sensor_dev, int32_t sensor_idx) = 0;

 protected:
  IHalIspPluginFactory() = default;

 public:
  virtual ~IHalIspPluginFactory() = default;
};


}      // namespace ispcf
}      // namespace mtk
#endif  // AAA_ISPHAL_SRC_INCLUDE_PLUGIN_IHALISPPLUGIN_H_

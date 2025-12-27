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

#ifndef AAA_ISPHAL_SRC_INCLUDE_PLUGIN_IPLUGINNOTIFIER_H_
#define AAA_ISPHAL_SRC_INCLUDE_PLUGIN_IPLUGINNOTIFIER_H_

// For meta data
#include <mtkcam-interfaces/utils/metadata/IMetadata.h>

// For meta data set
#include <mtkcam-interfaces/aaahal/aaa_hal_ctrl/IHal3ACommon.h>
#include <mtkcam-interfaces/isphal/IspTuningMeta.h>
#include <mtkcam-interfaces/isphal/IHalISPAdapter.h>
#include "IHalIspPlugin.h"

// std c++
#include <vector>
#include <memory>
#include <string>
#include <utility>

namespace mtk {
namespace ispcf {

class IPluginNotifier {
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  //  Interfaces.
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 public:
  /**
   * Create the latest IPluginNotifier instance.
   *  @param[in] sensor_dev Sensor device unique ID.
   *  @param[in] sensor index of the given |sensor_dev|.
   *  @return The instance of IHalIsp.
   */
  static IPluginNotifier* createInstance(size_t sensor_dev, size_t sensor_idx);
  static bool registerPluginFactory(IHalIspPluginFactory* plugin);

  virtual bool doAllImgSysPreproc(
    const mtk_halisp_metaset& control_from_camsys,
    const mtk_halisp_metaset& control_imgsys,
    const mtk::isphal::IspTuningControl& tuning_control,
    const mtk::isphal::IspTuningStatisticsP2& tuning_statistics,
    const mtk::isphal::IspTuningBufferP2& tuning_data,
    std::vector<std::pair<std::string,
      std::vector<
        std::pair<kISPPLUGIN_BUF_T, mtk::isphal::Buffer>>>>& result) = 0;

  virtual bool doAllImgSysPostProc(
    const mtk_halisp_metaset& control_from_camsys,
    const mtk_halisp_metaset& control_imgsys,
    const mtk::isphal::IspTuningControl& tuning_control,
    const mtk::isphal::IspTuningStatisticsP2& tuning_statistics,
    const mtk::isphal::IspTuningBufferP2& tuning_data,
    mtk::isphal::v1_0::ExifInfoP2& exif) = 0;

  virtual bool updateAllCamsysResult(
    uint64_t frmId,
    const mtk_frame_info& frame_info,
    ISP_CAMERA_INFO& ispCameraInfo,
    mtk_hal3a_metaset* result) = 0;

  virtual bool doAllCamSysPostProc(
    uint64_t frmId,
    const std::vector<mtk::hal3a::v1_0::mtk_hal3a_metaset*>& requestQ,
    const isphal::IspTuningCamsysControl& tuning_control,
    isphal::IspTuningBufferP1& tuning_data) = 0;

  virtual bool doAllCamSysReprocPostProc(
    const mtk_halisp_metaset& control_from_camsys,
    const mtk_halisp_metaset& control_imgsys,
    const mtk::isphal::IspTuningControl& tuning_control,
    mtk::isphal::IspTuningBufferP1& tuning_data) = 0;

 protected:
  IPluginNotifier() = default;

  /**
   * Copy and move an instance were forbidden since the IPluginNotifier was
   * always managed by std::unique_ptr<T>
   */
  IPluginNotifier(const IPluginNotifier&) = delete;
  IPluginNotifier(IPluginNotifier&&) = delete;

 public:
  virtual ~IPluginNotifier() = default;
};

}      // namespace ispcf
}      // namespace mtk
#endif  // AAA_ISPHAL_SRC_INCLUDE_PLUGIN_IPLUGINNOTIFIER_H_

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

#ifndef AAA_ISPHAL_INCLUDE_V2_IHALISP_H_
#define AAA_ISPHAL_INCLUDE_V2_IHALISP_H_

#include "IHalIspFactory.h"     // IHalIspFactory
#include "utils/VersionInfo.h"  // mtk::isphal::Version
#include "TuningParam.h"

#include "platform/mtkisp7/mtkcam-interfaces/include/mtkcam-interfaces/isphal/IHalISPAdapter.h"

#include <memory>  // std::shared_ptr

//struct mtk_isp_buf_info {
  //size_t lceso_size = 0;
  //size_t lcesho_size = 0;
  //size_t dceso_size = 0;
  //size_t camsys_stat_size = 0;
  //size_t camsys_meta_size = 0;
  //size_t camsys_meta_version = 0;
  //size_t imgsys_stat_size = 0;
  //size_t imgsys_hist_buf_size = 0;
  //size_t imgsys_meta_size = 0;
  //size_t imgsys_meta_version = 0;
  //size_t hwme_stat_fst_size = 0;
  //size_t hwme_stat_fmb_size = 0;
  //size_t hwme_stat_lmi_size = 0;
  //size_t fwme_fst_meta_size = 0;
  //size_t fwmm_mmg_fbfst_meta_size = 0;
  //size_t fwmm_mmg_rst_meta_size = 0;
  //size_t fwmm_mil_meta_size = 0;
//};


namespace mtk {
namespace isphal {
namespace v1 {
/**
 * IHalISP is a reentrant and thread-safe. Caller has no need to deal with data
 * racing problems and memory barriers.
 * Creating IHalIsp directly has been forbidden, caller must invoke
 * IHalIsp::createInstance() to create an instance.
 */
class IHalIsp : public mtk::isphal::IHalIspFactory {
 public:
  /**
   * Create the latest IHalISP instance.
   *  @param[in] sensor_dev Sensor device unique ID.
   *  @param[in] sensor index of the given |sensor_dev|.
   *  @return The instance of IHalIsp.
   */
  static std::shared_ptr<v1::IHalIsp> createInstance(size_t sensor_dev,
                                                     size_t sensor_idx,
                                                     uint64_t user_id);

 public:  // v1.0
  /**
   * Prepare ISP settings for Mediatek P1 ISP driver according to the
   * configurations of |tuning_param|.
   *  @param tuning_param The configurations that how ISP hal to prepare
   *      P1 ISP settings.
   *  @param p_return The P1 ISP settings and other outputs.
   *  @return Returns 0 for ok, otherwise checks GNU error code.
   */
  virtual int getCamSysMetaTuning(
      mtk::isphal::v1_0::TuningParamP1* tuning_param,
      mtk::isphal::v1_0::ReturnParamP1* p_return) = 0;

  /**
   * Trigger dump in camsys
   * configurations of |tuning_param|.
   *  @param tuning_param The configurations that how ISP hal to prepare
   *      P1 ISP settings.
   *  @param p_return The P1 ISP settings and other outputs.
   *  @return Returns 0 for ok, otherwise checks GNU error code.
   */
  virtual int dump4CamSysModule(
      mtk::isphal::v1_0::TuningParamP1* tuning_param,
      mtk::isphal::v1_0::ReturnParamP1* p_return) = 0;

  /**
   * Report status of camsys
   * configurations of |tuning_param|.
   *  @param tuning_param The configurations that how ISP hal to prepare
   *      P1 ISP settings.
   *  @param p_return The P1 ISP settings and other outputs.
   *  @return Returns 0 for ok, otherwise checks GNU error code.
   */
  virtual int reportStatus4CamSys(
      mtk::isphal::v1_0::TuningParamP1* tuning_param,
      mtk::isphal::v1_0::ReturnParamP1* p_return) = 0;

  /**
   * Prepare ISP settings for Mediatek P2 ISP driver according to the given
   * configurations of |tuning_param|.
   *  @return Returns 0 for ok, otherwise checks GNU error code.
   */
  virtual int getImgSysMetaTuning(
      mtk::isphal::v1_0::TuningParamDip* tuning_param,
      mtk::isphal::v1_0::ReturnParamDip* p_retrun) = 0;

  /**
   * Receive ISP controls from Mediatek middleware and do corresponding handle
   *  @param [in] eISPCtrl: control code
   *  @param [in] iArg1: input1
   *  @param [in] iArg2: input2
   *  @return 0 for ok, otherwise checks POSIX error code. Basically, if the
   *         control is not support, returns -ENOTSUP.
   */
  virtual int sendIspCtrl(mtk::isphal::v1_0::kISPCtrl_HalIsp_T eISPCtrl,
                          intptr_t iArg1,
                          uint32_t sizeArg1,
                          intptr_t iArg2,
                          uint32_t sizeArg2) = 0;

  virtual bool queryISPBufferInfo(mtk_isp_buf_info* bufferInfo) = 0;

 protected:
  IHalIsp() = default;

  /**
   * Copy and move an instance were forbidden since the IHalIsp was always
   * managed by std::unique_ptr<T>
   */
  IHalIsp(const IHalIsp&) = delete;
  IHalIsp(IHalIsp&&) = delete;

 public:
  virtual ~IHalIsp() = default;
};  // class HalIspImp

}      // namespace v1
}      // namespace isphal
}      // namespace mtk
#endif  // AAA_ISPHAL_INCLUDE_V2_IHALISP_H_

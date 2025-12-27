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
#ifndef INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_CTRL_AAA_HAL_CTRL_DEF_H_
#define INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_CTRL_AAA_HAL_CTRL_DEF_H_

#include <string>

#include "mtkcam-interfaces/utils/metadata/IMetadata.h"
#include "mtkcam-core/hw/camsys/pipe_mgr/MetaData.h"
#include "public/aaa_hal_common.h"

#include "mtkcam-interfaces/utils/metadata/IMetadata.h"
#include "mtkcam-interfaces/utils/metadata/client/mtk_metadata_tag.h"
#include "mtkcam-interfaces/utils/metadata/hal/mtk_platform_metadata_tag.h"
#include "mtkcam-interfaces/aaahal/aaa_hal_ctrl/IHal3ACommon.h"
#include "mtkcam-core/hw/camsys/pipe_mgr/MetaData.h"
#include "public/aaa_hal_common.h"
#include "peripheralcontroller/include/IPeripheralController.h"
#include "mtkcam-core/aaahal/aaa_hal_ctrl/IHal3AListener.h"
#include "mtkcam-core/aaahal/aaa_hal_ctrl/hal3a_stt_info.h"

using NSCam::camsys::pipemgr::mtk_cam_front_end_setting;

namespace mtk {
namespace hal3a {

struct mtk_camsys_info {
  struct img_size_after_frz {
    uint32_t width;
    uint32_t height;
  } size_after_frz;
  uint32_t pixel_mode;
};

namespace v1_0 {
struct mtk_hal3a_initial_info {
  struct sensor_info {
    NSCam::camsys::pipemgr::mtk_sensor_driver_control ctrl;
  } sensor;
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
  IHal3AListener* p_3a_listener;
  NSCam::IMetadata appMeta;
  NSCam::IMetadata halMeta;

  mtk_hal3a_config()
      : subsample_count(1),
        request_count(1),
        sensor_mode(0),
        sensor_dev(0),
        sensor_id(0),
        sensor_tg_width(4000),
        sensor_tg_height(3000),
        bit_mode(kBitMode_12Bit),
        p_3a_listener(0) {}
};

// mtk_hal3a_metaset move to mtkcam-interfaces
// include/mtkcam-interfaces/aaahal/aaa_hal_ctrl/IHal3AComon.h

struct mtk_hal3a_sof_info {
  uint32_t sof_id;
};

struct mtk_hal3a_setting {
  mtk_cam_uapi_meta_raw_stats_cfg* raw_meta;
#if SUPPORT_META_MRAW == 1
  mtk_cam_uapi_meta_mraw_stats_cfg* mraw_meta[VC_MAX_NUM];
#endif
  mtk_cam_front_end_setting* driver_setting;
  NSCam::MRect ai3ao_crop;
  uint64_t request_id;
};

struct mtk_hal3a_debug {
  std::string str = "";
};

}   // namespace v1_0

namespace v2_0 {
struct mtk_hal3a_config : v1_0::mtk_hal3a_config {};
struct mtk_hal3a_metaset : v1_0::mtk_hal3a_metaset {};
struct mtk_hal3a_setting : v1_0::mtk_hal3a_setting {};
struct mtk_hal3a_sof_info : v1_0::mtk_hal3a_sof_info {};
}   // namespace v2_0

}       // namespace hal3a
}       // namespace mtk
#endif  // INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_CTRL_AAA_HAL_CTRL_DEF_H_

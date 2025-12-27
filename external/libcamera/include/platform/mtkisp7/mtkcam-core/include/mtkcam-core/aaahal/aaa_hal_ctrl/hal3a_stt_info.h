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

#ifndef INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_CTRL_HAL3A_STT_INFO_H_
#define INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_CTRL_HAL3A_STT_INFO_H_

// #ifndef MTKCAM_ISP7_DEV
// #include "kernel_header/mtk_cam_metabuf.h"
// #else
#include <mtk_cam_metabuf.h>
// #endif

namespace mtk {
namespace hal3a {

namespace Mtk3ASttTypeId {
const uint32_t kAAO = (1 << 0);
const uint32_t kAFO = (1 << 1);
const uint32_t kAi3aO = (1 << 2);
const uint32_t kCamsv = (1 << 3);
const uint32_t kMraw = (1 << 4);
}  // namespace Mtk3ASttTypeId

namespace v1_0 {
struct mtk_hal3a_stt_info {
  uint64_t sof_timestamp;
  struct active_flag {
    uint64_t request_id;  // The id of control request which was actived on stt
    uint32_t stt_active_items;  // The set of stt typeid which were actived.
  } flag;
  struct aao_buf {
    const mtk_cam_uapi_meta_raw_stats_0* buf;
    int32_t fd;
  } aao;
  struct afo_buf {
    const mtk_cam_uapi_meta_raw_stats_1* buf;
    int32_t fd;
  } afo;
  struct ai3ao_buf {
    void* buf;
    int32_t fd;
    // For ai3ao_t in aaa/common/include/aaa_mgr_public_if.h
    int32_t crop_rect_x;
    int32_t crop_rect_y;
    int32_t crop_rect_width;
    int32_t crop_rect_height;
    //
  } ai3ao;
  struct camsv_buf {
    const mtk_cam_uapi_meta_camsv_stats_0* buf;
    int32_t fd;
    int32_t node_id;
    int32_t channel_id;
    int32_t feature_type;
  };
  camsv_buf multi_camsv[MAX_VC_INFO_CNT];
  struct mraw_buf {
#if SUPPORT_META_MRAW == 1
    const mtk_cam_uapi_meta_mraw_stats_0* buf;
#endif
    int32_t fd;
    int32_t node_id;
    int32_t channel_id;
    int32_t feature_type;
  };
  mraw_buf multi_mraw[MAX_VC_INFO_CNT];
  bool is_reliable = true;
};
}  // namespace v1_0

}       // namespace hal3a
}       // namespace mtk
#endif  // INCLUDE_MTKCAM_CORE_AAAHAL_AAA_HAL_CTRL_HAL3A_STT_INFO_H_

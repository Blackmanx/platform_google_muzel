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

#ifndef INCLUDE_MTKCAM_INTERFACES_HW_CAMSYS_P1_DMA_H_
#define INCLUDE_MTKCAM_INTERFACES_HW_CAMSYS_P1_DMA_H_

#include <cstdint>
#include "mtkcam-interfaces/utils/metadata/client/mtk_metadata_tag.h"

namespace NSCam {
namespace camsys {
namespace p1dma {

/**
 * List abstract P1 dma node id for MW/feature users.
 * including high level definitions and low level definitions.
 * High level means specific purpose usage like fd, AI3A, me.
 * Low level means hardware physical naming which is for super user
 * who wants to control detailed hw dma port by its own and high level
 * naming cannot cover it.
 *
 * |   8 bits |   8 bits |
 * | DMA type |   index  |
 */
enum : uint16_t {
  // high level
  FULL_RAW     = MTK_HALCORE_STREAM_SOURCE_FULL_RAW,
  FD           = MTK_HALCORE_STREAM_SOURCE_FD,
  ME           = MTK_HALCORE_STREAM_SOURCE_ME,
  DEPTH        = MTK_HALCORE_STREAM_SOURCE_DEPTH,
  FE           = MTK_HALCORE_STREAM_SOURCE_FE,
  AI_TRACKING  = MTK_HALCORE_STREAM_SOURCE_AI_TRACKING,

  // low level
  YUV          = MTK_HALCORE_STREAM_SOURCE_YUV,      // No R0 (index=0)
  YUV_R1       = MTK_HALCORE_STREAM_SOURCE_YUV_R1,   // index=1
  YUV_R2       = MTK_HALCORE_STREAM_SOURCE_YUV_R2,
  YUV_R3       = MTK_HALCORE_STREAM_SOURCE_YUV_R3,
  YUV_R4       = MTK_HALCORE_STREAM_SOURCE_YUV_R4,
  YUV_R5       = MTK_HALCORE_STREAM_SOURCE_YUV_R5,
  CAMSV,
  RAWI,

  // low level - isp6
  STT          = MTK_HALCORE_STREAM_SOURCE_STT,      // statistics
};


/**
 * Convert a given p1dma to the index of it.
 *
 * For example,
 *    NSCam::camsys::p1dma::toIndex(NSCam::camsys::p1dma::YUV_R5) == 5
 */
static inline uint16_t toIndex(uint16_t id) { return (id & 0x00ff); }

/**
 * Convert a given p1dma to the type of it.
 *
 * For example,
 *    NSCam::camsys::p1dma::toType(NSCam::camsys::p1dma::YUV_R5) ==
 *    NSCam::camsys::p1dma::YUV
 */
static inline uint16_t toType (uint16_t id) { return (id & 0xff00); }


}   // namespace p1dma
}   // namespace camsys
}   // namespace NSCam

#endif  // INCLUDE_MTKCAM_INTERFACES_HW_CAMSYS_P1_DMA_H_

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


#ifndef INCLUDE_MTKCAM_INTERFACES_FEATURE_STEREO_HAL_STEREO_AAA_INFO_PROVIDER_H_
#define INCLUDE_MTKCAM_INTERFACES_FEATURE_STEREO_HAL_STEREO_AAA_INFO_PROVIDER_H_

#include <memory>

#include "mtkcam-interfaces/def/common.h"
// Commented out by Google.
// #include "mtkcam-interfaces/feature/stereo/hal/stereo_common.h"
#include "mtkcam-interfaces/utils/metadata/IMetadata.h"
#include "mtkcam-interfaces/aaahal/aaa_hal_ctrl/IHal3ACommon.h"

namespace StereoHAL {

// GF -> AF
struct GF_INFO_TO_AF {
  MBOOL enable = false;
  MUINT32 dac  = 0;
  MUINT32 conf = 0;
  NSCam::MPoint topLeft;      // top-left in TG domain
  NSCam::MPoint rightBottom;  // right-bottom in TG domain
};

// AF -> GF
typedef MUINT32 PD_LABEL_T;
struct AF_INFO_TO_VSDOF {
  int32_t dac_min      = 0;  // possible minimum dac
  int32_t dac_max      = 0;  // possible maximum dac
  int32_t cur_dac      = 0;  // current af position
  int32_t posture_dac  = 0;  // af position after posture compensate
  int32_t is_af_stable = 0;  // is af search done
  int32_t roi_left     = 0;  // af roi start_x
  int32_t roi_top      = 0;  // af roi start_y
  int32_t roi_right    = 0;  // af roi end_x
  int32_t roi_bottom   = 0;  // af roi end_y
  int32_t roi_type     = 0;  // af roi type (center / face / …)

  // For Bokeh AF
  NSCam::MSize pdCoverage;   // TrackRangeW, TrackRangeH
  NSCam::MSize pdGridNum;
  PD_LABEL_T pdLable[8];  // Count will be pdGridNum.x*pdGridNum.y
  const size_t pdLableSize = sizeof(pdLable) / sizeof(pdLable[0]);
  static_assert(sizeof(pdLable) == sizeof(mtk::hal3a::mtk_hal3a_pd_info::map),
                "pdLable size mismatch");

  AF_INFO_TO_VSDOF() = default;

  AF_INFO_TO_VSDOF(const AF_INFO_TO_VSDOF &rhs) {
    dac_min      = rhs.dac_min;
    dac_max      = rhs.dac_max;
    cur_dac      = rhs.cur_dac;
    posture_dac  = rhs.posture_dac;
    is_af_stable = rhs.is_af_stable;
    roi_left     = rhs.roi_left;
    roi_top      = rhs.roi_top;
    roi_right    = rhs.roi_right;
    roi_bottom   = rhs.roi_bottom;
    roi_type     = rhs.roi_type;
    pdCoverage   = rhs.pdCoverage;
    pdGridNum    = rhs.pdGridNum;
    ::memcpy(pdLable, rhs.pdLable, sizeof(pdLable));
  }

  AF_INFO_TO_VSDOF(AF_INFO_TO_VSDOF &&rhs) {
    dac_min      = rhs.dac_min;
    dac_max      = rhs.dac_max;
    cur_dac      = rhs.cur_dac;
    posture_dac  = rhs.posture_dac;
    is_af_stable = rhs.is_af_stable;
    roi_left     = rhs.roi_left;
    roi_top      = rhs.roi_top;
    roi_right    = rhs.roi_right;
    roi_bottom   = rhs.roi_bottom;
    roi_type     = rhs.roi_type;
    pdCoverage   = rhs.pdCoverage;
    pdGridNum    = rhs.pdGridNum;
    ::memcpy(pdLable, rhs.pdLable, sizeof(pdLable));
  }

  ~AF_INFO_TO_VSDOF() {}
};

struct StereoAFStaticInfo {
  bool is_af          = false;
  MUINT16 af_dac_min  = 0;
  MUINT16 af_dac_max  = 0;
  MUINT16 af_dist_mcr = 0;
  MUINT16 af_dist_inf = 0;
};

struct AFInfoQueryParam {
  // Must set to query
  int sensorId;
  int frameNumber;

  // Set to transfer af ROI to the domain
  NSCam::MSize targetDomainSize;

  AFInfoQueryParam(int id, int frame) {
    sensorId    = id;
    frameNumber = frame;
  }

  AFInfoQueryParam(int id, int frame, NSCam::MSize size) {
    sensorId         = id;
    frameNumber      = frame;
    targetDomainSize = size;
  }
};

typedef int32_t FRAME_NUM_T;
class IStereo3AInfoProvider {
 public:
  static std::shared_ptr<IStereo3AInfoProvider> getInstance();

  virtual bool setMetadata(int sensorId, FRAME_NUM_T frameNumber,
                           NSCam::IMetadata *meta) = 0;

  virtual void initAFStaticInfo(int sensorId,
                                const StereoAFStaticInfo &afInfo) = 0;
  virtual const StereoAFStaticInfo *getAFStaticInfo(int sensorId) const = 0;

  virtual const AF_INFO_TO_VSDOF *getAFInfo(AFInfoQueryParam param) const = 0;

  virtual void reset() = 0;

 protected:
  IStereo3AInfoProvider() = default;
  virtual ~IStereo3AInfoProvider() {}
};

}   // namespace StereoHAL

#endif  // INCLUDE_MTKCAM_INTERFACES_FEATURE_STEREO_HAL_STEREO_AAA_INFO_PROVIDER_H_

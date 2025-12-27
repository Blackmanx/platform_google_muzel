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

#ifndef INCLUDE_MTKCAM_CORE_ISPHAL_ISPCONTROLBASIC_H_
#define INCLUDE_MTKCAM_CORE_ISPHAL_ISPCONTROLBASIC_H_

#include <cstdint>  // uint32_t, int32_t

namespace mtk {
namespace isphal {

/**
 * Setup EIS warpping info for imgsys driver
 */
struct WrappingParam {
    uint8_t  CRT_EN;
    uint32_t CRT_IN_WD;
    uint32_t CRT_IN_HT;
    uint32_t CRT_CROP_WD;
    uint32_t CRT_CROP_HT;
    uint32_t CRT_POINT_X0;
    uint32_t CRT_POINT_Y0;
    uint32_t CRT_POINT_X1;
    uint32_t CRT_POINT_Y1;
    uint32_t CRT_POINT_X2;
    uint32_t CRT_POINT_Y2;
    uint32_t CRT_POINT_X3;
    uint32_t CRT_POINT_Y3;
};

struct CAMERA_TUNING_FD_INFO_T {
  int32_t YUVsts[15][5];  // face statistic data, only first five face valid
  int8_t fld_GenderLabel[15];
  int32_t fld_GenderInfo[15];
  int32_t fld_rip[15];
  int32_t fld_rop[15];
  int32_t rect[15][4];
  int32_t GenderNum;
  int32_t LandmarkNum;
  int32_t Face_Leye[15][4];
  int32_t Face_Reye[15][4];
  int32_t Landmark_CV[15];
  int32_t FaceNum;
  int32_t tcy_index;
  uint32_t tcy_uv_gain;
  uint32_t tcy_y_curve[32];
  int32_t FD_source;
  int32_t FD_magicNo;
};

/**
 * Input config for MSYUV
 */
enum msyuv_dsMode {
    kmsyuv_dsMode2,
    kmsyuv_dsMode4,
    kmsyuv_dsMode42,
};

}  // namespace isphal
}  // namespace mtk

#endif  // INCLUDE_MTKCAM_CORE_ISPHAL_ISPCONTROLBASIC_H_

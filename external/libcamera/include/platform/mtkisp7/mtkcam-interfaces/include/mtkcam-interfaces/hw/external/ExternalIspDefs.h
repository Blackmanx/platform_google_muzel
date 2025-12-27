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

#ifndef INCLUDE_MTKCAM_INTERFACES_HW_EXTERNAL_EXTERNALISPDEFS_H_
#define INCLUDE_MTKCAM_INTERFACES_HW_EXTERNAL_EXTERNALISPDEFS_H_

namespace NSCam {
namespace external {

static constexpr int LSC_LUT_TABLE_SIZE_MAX = 19;

struct LscResult {
  // actual width of lut table, max is (19) current is 17
  uint32_t    mTabWidth;
  // actual height of lut table, max is (15) current is 13
  uint32_t    mTabHeight;
  // the count of lsc table, default: 4
  uint16_t    mTabNum;
  // R, size is tabWidth * tabHeight
  uint16_t    mTab0[LSC_LUT_TABLE_SIZE_MAX];
  // GR
  uint16_t    mTab1[LSC_LUT_TABLE_SIZE_MAX];
  // GB
  uint16_t    mTab2[LSC_LUT_TABLE_SIZE_MAX];
  // B
  uint16_t    mTab3[LSC_LUT_TABLE_SIZE_MAX];
};

struct BlcResult {
  uint8_t ob;
  uint8_t cropBlc;
  uint8_t dgBlc;
  uint8_t hdrBlc;
  uint8_t wbBlc;
  uint8_t lscBlc;
};

struct IspDataResult {
  bool lscEnable;
  bool obEnable;
  bool bpcEnable;
  bool isYuvFmt;
  LscResult mLscRes;
  BlcResult mBlcRes;
  int32_t ispCcm[9];
  uint32_t gammaTbl[129];
};

}  // namespace external
}  // namespace NSCam

#endif  // INCLUDE_MTKCAM_INTERFACES_HW_EXTERNAL_EXTERNALISPDEFS_H_

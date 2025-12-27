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

#ifndef AAA_INCLUDE_LSC_ILSCTABLE_H_
#define AAA_INCLUDE_LSC_ILSCTABLE_H_

#include <vector>

namespace NSIspTuning {
class ILscTable {
 public:
  typedef enum { HWTBL = 0, GAIN_FIXED = 1, GAIN_FLOAT = 2 } TBL_TYPE_T;

  typedef enum {
    BAYER_B = 0,
    BAYER_GB = 1,
    BAYER_GR = 2,
    BAYER_R = 3
  } TBL_BAYER_T;

  struct ConfigBlk {
    ConfigBlk()
        : i4BlkX(0),
          i4BlkY(0),
          i4BlkW(0),
          i4BlkH(0),
          i4BlkFirstW(0),
          i4BlkFirstH(0),
          i4BlkLastW(0),
          i4BlkLastH(0) {}

    ConfigBlk(int32_t i4ImgWd,
              int32_t i4ImgHt,
              int32_t i4GridX,
              int32_t i4GridY) {
      i4BlkX = i4GridX - 1;
      i4BlkY = i4GridY - 1;
      i4BlkW = (i4ImgWd) / (2 * i4BlkX);
      i4BlkH = (i4ImgHt) / (2 * i4BlkY);
      i4BlkFirstW = (i4ImgWd / 2 - i4BlkW * (i4BlkX - 2)) / 2;
      i4BlkFirstH = (i4ImgHt / 2 - i4BlkH * (i4BlkY - 2)) / 2;
      i4BlkLastW = i4ImgWd / 2 - i4BlkFirstW - i4BlkW * (i4BlkX - 2);
      i4BlkLastH = i4ImgHt / 2 - i4BlkFirstH - i4BlkH * (i4BlkY - 2);
    }

    ConfigBlk(int32_t _i4BlkX,
              int32_t _i4BlkY,
              int32_t _i4BlkW,
              int32_t _i4BlkH,
              int32_t _i4BlkLastW,
              int32_t _i4BlkLastH)
        : i4BlkX(_i4BlkX),
          i4BlkY(_i4BlkY),
          i4BlkW(_i4BlkW),
          i4BlkH(_i4BlkH),
          i4BlkLastW(_i4BlkLastW),
          i4BlkLastH(_i4BlkLastH) {}

    int32_t i4BlkX;
    int32_t i4BlkY;
    int32_t i4BlkW;
    int32_t i4BlkH;
    int32_t i4BlkFirstW;
    int32_t i4BlkFirstH;
    int32_t i4BlkLastW;
    int32_t i4BlkLastH;
  };

  struct Config {
    Config() : i4ImgWd(0), i4ImgHt(0), i4GridX(0), i4GridY(0), rCfgBlk() {}

    int32_t i4ImgWd;
    int32_t i4ImgHt;
    int32_t i4GridX;
    int32_t i4GridY;
    ConfigBlk rCfgBlk;
  };

  struct TransformCfg_T {
    TransformCfg_T(uint32_t _u4ResizeW,
                   uint32_t _u4ResizeH,
                   uint32_t _u4GridX,
                   uint32_t _u4GridY,
                   uint32_t _u4X,
                   uint32_t _u4Y,
                   uint32_t _u4W,
                   uint32_t _u4H)
        : u4ResizeW(_u4ResizeW),
          u4ResizeH(_u4ResizeH),
          u4GridX(_u4GridX),
          u4GridY(_u4GridY),
          u4X(_u4X),
          u4Y(_u4Y),
          u4W(_u4W),
          u4H(_u4H) {}
    TransformCfg_T()
        : u4ResizeW(0),
          u4ResizeH(0),
          u4GridX(0),
          u4GridY(0),
          u4X(0),
          u4Y(0),
          u4W(0),
          u4H(0) {}
    uint32_t u4ResizeW;
    uint32_t u4ResizeH;
    uint32_t u4GridX;
    uint32_t u4GridY;
    uint32_t u4X;
    uint32_t u4Y;
    uint32_t u4W;
    uint32_t u4H;
  };

  struct RsvdData {
    explicit RsvdData(uint32_t _u4HwRto) : u4HwRto(_u4HwRto) {}
    RsvdData() : u4HwRto(32) {}
    uint32_t u4HwRto;
  };
};
}   // namespace NSIspTuning

#endif  // AAA_INCLUDE_LSC_ILSCTABLE_H_

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

#ifndef AAA_ISPHAL_INCLUDE_ISPHAL_UTILS_IMAGEFORMATS_H_
#define AAA_ISPHAL_INCLUDE_ISPHAL_UTILS_IMAGEFORMATS_H_

namespace mtk {
namespace isphal {

enum RawType {
    kRawTypeRRZO = 0,
    kRawTypeIMGOPure,
    kRawTypeIMGOProcessed,
};

enum ImageFormat {
    kImageFormatUndef,
    kImageFormatMtkRawBayer,
    kImageFormatMtkRawMono,
    kImageFormatMtkDirectYuv,  // note p1 yuv for mediatek hardware
    kImageFormatMtkYuvReProc,  // note p2 yuv for mediatek hardware
    kImageFormatMtkRawIR
};

}  // namespace isphal
}  // namespace mtk
#endif  // AAA_ISPHAL_INCLUDE_ISPHAL_UTILS_IMAGEFORMATS_H_


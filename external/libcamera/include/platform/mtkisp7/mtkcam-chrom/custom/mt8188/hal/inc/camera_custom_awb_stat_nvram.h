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

#ifndef AAA_COMMON_AWB_MGR_CAMERA_CUSTOM_AWB_STAT_NVRAM_H_
#define AAA_COMMON_AWB_MGR_CAMERA_CUSTOM_AWB_STAT_NVRAM_H_

#include "MediaTypes.h"

#define NVRAM_CUSTOM_AWB_STAT_REVISION 9181001

#define BEGIN_OF_AWB_STAT

// AWB statistics parameter
typedef struct {
  // Number of AWB windows
  MINT32 i4WindowNumX;  // Number of horizontal AWB windows
  MINT32 i4WindowNumY;  // Number of vertical AWB windows

  // Thresholds
  MINT32 i4LowThresholdR;   // Low threshold of R
  MINT32 i4LowThresholdG;   // Low threshold of G
  MINT32 i4LowThresholdB;   // Low threshold of B
  MINT32 i4HighThresholdR;  // High threshold of R
  MINT32 i4HighThresholdG;  // High threshold of G
  MINT32 i4HighThresholdB;  // High threshold of B

  MINT32 i4LightSrcLowThresholdR;   // Low threshold of R for light source
                                    // estimation
  MINT32 i4LightSrcLowThresholdG;   // Low threshold of G for light source
                                    // estimation
  MINT32 i4LightSrcLowThresholdB;   // Low threshold of B for light source
                                    // estimation
  MINT32 i4LightSrcHighThresholdR;  // High threshold of R for light source
                                    // estimation
  MINT32 i4LightSrcHighThresholdG;  // High threshold of G for light source
                                    // estimation
  MINT32 i4LightSrcHighThresholdB;  // High threshold of B for light source
                                    // estimation

  // Pre-gain maximum limit clipping
  MINT32 i4PreGainLimitR;  // Maximum limit clipping for R color
  MINT32 i4PreGainLimitG;  // Maximum limit clipping for G color
  MINT32 i4PreGainLimitB;  // Maximum limit clipping for B color

  // AWB error threshold
  MINT32 i4ErrorThreshold;  // Programmable threshold for the allowed total
                            // over-exposured and under-exposered pixels in one
                            // main stat window

  // AWB error count shift bits
  MINT32 i4ErrorShiftBits;  // Programmable error count shift bits: 0 ~ 7
                            // Note: AWB statistics provide 4-bits error count
                            // output only

  // AWB error pixel ratio
  MINT32 i4ErrorRatio;  // Programmable error pixel count by AWB window size
                        // (base : 256)

  // AWB motion error pixel ratio
  MINT32 i4MoErrorRatio;  // Programmable motion error pixel count by AWB window
                          // size (base : 256)

  // AWB output mode select flag
  MINT32 i4StatMode;  // 1: Linear mode, 0: Non-linear mode
} AWB_STAT_NVRAM_T;
#endif  // AAA_COMMON_AWB_MGR_CAMERA_CUSTOM_AWB_STAT_NVRAM_H_

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

#ifndef FEATURE_COMMON_FACEEFFECT_FACEDETECTION_FD_TUNING_TUNINGPARA_H_
#define FEATURE_COMMON_FACEEFFECT_FACEDETECTION_FD_TUNING_TUNINGPARA_H_

#include "libfdft_lib/MTKDetectionType.h"

struct tuning_para {
  MUINT32 FDThreshold;   // default 256, suggest range: 100~400 bigger is harder
  MUINT32 DisLimit;      // default 4, suggest range: 1 ~ 4
  MUINT32 DecreaseStep;  // default 48, suggest range: 0 ~ 384
  MUINT32 FDMINSZ;
  MUINT32 DelayThreshold;  // default 20, under this goes to median reliability,
                           // above goes high
  MUINT32 DelayCount;  // default 2, for median reliability face, should have
                       // deteced in continous frame
  MUINT32 MajorFaceDecision;  // default 1, 0: Size fist.  1: Center first.   2:
                              // Size first perframe. 3: Center first per frame
  MUINT8 OTBndOverlap;        // default 8, suggest range: 6 ~ 9
  MUINT32 OTRatio;            // default 960, suggest range: 400~1200
  MUINT32 OTds;               // default 2, suggest range: 1~2
  MUINT32 OTtype;             // default 1, suggest range: 0~1
  MUINT32 SmoothLevel;        // default 8, suggest range: 0~16
  MUINT32 Momentum;           // default 2819,
  MUINT32 SmoothLevelUI;      // default 8, suggest range: 0~16
  MUINT32 MomentumUI;         // default 2818
  MUINT32 MaxTrackCount;      // default 10, suggest range: 0~120
  MUINT8 SilentModeFDSkipNum;  // default 0, suggest range: 0~2
  MUINT8 HistUpdateSkipNum;    // default 1, suggest range: 0~5
  MUINT32 FDSkipStep;          // default 2, suggest range: 2~6
  MUINT32 FDRectify;  // default 10000000 means disable and 0 means disable as
                      // well. suggest range: 5~10
  MUINT32 FDRefresh;  // default 3, suggest range: 30~120
};

static struct tuning_para tuning_35 = {
    256,   // FDThreshold;
    4,     // DisLimit;
    384,   // DecreaseStep;
    0,     // FDMINSZ;
    127,   // DelayThreshold;
    3,     // DelayCount;
    1,     // MajorFaceDecision;
    8,     // OTBndOverlap;
    1088,  // OTRatio;
    2,     // OTds;
    1,     // OTtype;
    8,     // SmoothLevel;
    0,     // Momentum;
    0,     // SmoothLevelUI;
    0,     // MomentumUI;
    10,    // MaxTrackCount;
    0,     // SilentModeFDSkipNum;
    1,     // HistUpdateSkipNum;
    2,     // FDSkipStep;
    10,    // FDRectify;
    3      // FDRefresh;
};

static struct tuning_para tuning_40 = {
    200,   // FDThreshold;
    4,     // DisLimit;
    192,   // DecreaseStep;
    0,     // FDMINSZ;
    50,    // DelayThreshold;
    2,     // DelayCount;
    14,    // MajorFaceDecision;
    8,     // OTBndOverlap;
    1088,  // OTRatio;
    2,     // OTds;
    1,     // OTtype;
    8,     // SmoothLevel;
    0,     // Momentum;
    0,     // SmoothLevelUI;
    0,     // MomentumUI;
    10,    // MaxTrackCount;
    0,     // SilentModeFDSkipNum;
    1,     // HistUpdateSkipNum;
    2,     // FDSkipStep;
    10,    // FDRectify;
    3      // FDRefresh;
};

static struct tuning_para tuning_50 = {
    293,   // FDThreshold;
    1,     // DisLimit;
    48,    // DecreaseStep;
    0,     // FDMINSZ;
    293,   // DelayThreshold;
    2,     // DelayCount;
    1,     // MajorFaceDecision;
    8,     // OTBndOverlap;
    800,   // OTRatio;
    2,     // OTds;
    1,     // OTtype;
    8,     // SmoothLevel;
    2821,  // Momentum;
    0,     // SmoothLevelUI;
    0,     // MomentumUI;
    10,    // MaxTrackCount;
    0,     // SilentModeFDSkipNum;
    1,     // HistUpdateSkipNum;
    2,     // FDSkipStep;
    10,    // FDRectify;
    3      // FDRefresh;
};

static struct tuning_para tuning_51 = {
    232,   // FDThreshold;
    1,     // DisLimit;
    20,    // DecreaseStep;
    0,     // FDMINSZ;
    295,   // DelayThreshold;
    2,     // DelayCount;
    14,    // MajorFaceDecision;
    8,     // OTBndOverlap;
    800,   // OTRatio;
    2,     // OTds;
    1,     // OTtype;
    8,     // SmoothLevel;
    2819,  // Momentum;
    8,     // SmoothLevelUI;
    2818,  // MomentumUI;
    6,     // MaxTrackCount;
    0,     // SilentModeFDSkipNum;
    1,     // HistUpdateSkipNum;
    2,     // FDSkipStep;
    10,    // FDRectify;
    3      // FDRefresh;
};

static struct tuning_para tuning_52 = {
    283,   // FDThreshold;
    1,     // DisLimit;
    40,    // DecreaseStep;
    0,     // FDMINSZ;
    295,   // DelayThreshold;
    2,     // DelayCount;
    14,    // MajorFaceDecision;
    8,     // OTBndOverlap;
    800,   // OTRatio;
    2,     // OTds;
    1,     // OTtype;
    8,     // SmoothLevel;
    2819,  // Momentum;
    8,     // SmoothLevelUI;
    2818,  // MomentumUI;
    10,    // MaxTrackCount;
    0,     // SilentModeFDSkipNum;
    1,     // HistUpdateSkipNum;
    2,     // FDSkipStep;
    10,    // FDRectify;
    1      // FDRefresh;
};

static struct tuning_para tuning_53 = {
    283,   // FDThreshold;
    1,     // DisLimit;
    40,    // DecreaseStep;
    0,     // FDMINSZ;
    295,   // DelayThreshold;
    2,     // DelayCount;
    14,    // MajorFaceDecision;
    8,     // OTBndOverlap;
    800,   // OTRatio;
    2,     // OTds;
    1,     // OTtype;
    8,     // SmoothLevel;
    2819,  // Momentum;
    8,     // SmoothLevelUI;
    2818,  // MomentumUI;
    10,    // MaxTrackCount;
    0,     // SilentModeFDSkipNum;
    1,     // HistUpdateSkipNum;
    2,     // FDSkipStep;
    10,    // FDRectify;
    1      // FDRefresh;
};

static struct tuning_para tuning_531 = {
    250,   // FDThreshold;
    1,     // DisLimit;
    20,    // DecreaseStep;
    0,     // FDMINSZ;
    265,   // DelayThreshold;
    2,     // DelayCount;
    14,    // MajorFaceDecision;
    8,     // OTBndOverlap;
    800,   // OTRatio;
    2,     // OTds;
    1,     // OTtype;
    8,     // SmoothLevel;
    2819,  // Momentum;
    8,     // SmoothLevelUI;
    2818,  // MomentumUI;
    10,    // MaxTrackCount;
    0,     // SilentModeFDSkipNum;
    1,     // HistUpdateSkipNum;
    2,     // FDSkipStep;
    10,    // FDRectify;
    1      // FDRefresh;
};

#endif  // FEATURE_COMMON_FACEEFFECT_FACEDETECTION_FD_TUNING_TUNINGPARA_H_

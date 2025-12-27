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

#ifndef HW_IMGSTREAM_INC_DRV_COMMON_7_1_PQDIP_META_H_
#define HW_IMGSTREAM_INC_DRV_COMMON_7_1_PQDIP_META_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/**
 * @brief HW ID
 */
typedef enum PQDIPHWID {
  PQDIP_HW_A = 0, /*!< Select PQDIP HW A */
  PQDIP_HW_B,     /*!< Select PQDIP HW B */
  PQDIP_HW_MAX
} PQDIP_HW_ID;

/******************************************************************************
 * @struct IMG_PRroFile_Enum
 *
 * @brief Img_ProFile_Enum. copy from
 *"mtkcam\include\mtkcam\hw\imgstream\IImgStreamDef.h"
 ******************************************************************************/
enum IMG_PROFILE_ENUM {
  IMG_PROFILE_DEFAULT,
  IMG_PROFILE_JPEG = IMG_PROFILE_DEFAULT,
  IMG_PROFILE_FULL_BT601 = IMG_PROFILE_JPEG,
  IMG_PROFILE_BT601,  // Limited range
  IMG_PROFILE_BT709,
  IMG_PROFILE_BT2020,      // not support for output
  IMG_PROFILE_FULL_BT709,  // not support for output
  IMG_PROFILE_FULL_BT2020  // not support for output
};

struct pqportinfo {
  unsigned int mWdmaoPQIdx;
  uint64_t mWdmaoUserString;
  unsigned int mWdmaoBypassCrop;  // 0: refine crop, 1: bypass crop
  unsigned int mWrotoPQIdx;
  uint64_t mWrotoUserString;
  unsigned int mWrotoBypassCrop;  // 0: refine crop, 1: bypass crop
};

struct slk_pqdip_ctrl_t {
  uint32_t PQ_CROP_EN;
  uint32_t PQ_CROP_X;
  uint32_t PQ_CROP_Y;
  uint32_t PQ_CROP_WD;
  uint32_t PQ_CROP_HT;
  uint32_t PQ_OUT_WD;
  uint32_t PQ_OUT_HT;
};

/**
 * @brief ctrl meta usage for pqdip driver
 */
typedef struct pqdip_ctrl {
  PQDIP_HW_ID HWID; /*!< 0:PQDIP_HW_A, 1:PQDIP_HW_B */
  enum IMG_PROFILE_ENUM inProfile;
  // PQDIP_HW_A
  enum IMG_PROFILE_ENUM outProfile_a;
  // PQDIP_HW_B
  enum IMG_PROFILE_ENUM outProfile_b;
  struct slk_pqdip_ctrl_t slk_ctrl[PQDIP_HW_MAX];
  struct pqportinfo pqidxinfo;
} PQDIP_CTRL_META;

#endif  // HW_IMGSTREAM_INC_DRV_COMMON_7_1_PQDIP_META_H_

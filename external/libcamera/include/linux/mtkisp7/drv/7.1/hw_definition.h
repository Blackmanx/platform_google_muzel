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

#ifndef HW_IMGSTREAM_INC_DRV_COMMON_7_1_HW_DEFINITION_H_
#define HW_IMGSTREAM_INC_DRV_COMMON_7_1_HW_DEFINITION_H_

#include "dip_meta.h"
#include "me_meta.h"
#include "me_mv_stats.h"
#include "pqdip_meta.h"
#include <stdio.h>
#include <stdlib.h>
#include "traw_meta.h"
#include "wpe_meta.h"
#include "adl_meta.h"
#include "ufbc_meta.h"

/**
 * enum ModuleID
 *
 * Definition about HW Module (module based)
 * Only module defined before tdr max need to do tdr calculation
 */
typedef enum ModuleID {
  MODULE_WPE = 0,
  MODULE_ADL = 1,
  MODULE_TRAW = 2,
  MODULE_DIP = 3,
  MODULE_PQDIP = 4,
  MODULE_TDR_MAX = 5,
  MODULE_ME = MODULE_TDR_MAX,
  MODULE_MAX,
} ModuleID;

/**
 * enum HWID
 *
 * Definition about HW Set (HW based, it could be multiple hw set for a single
 * module) Only hw set defined before tdr max need to do tdr calculation and
 * direct link operation Note. max length of direct link table should be
 * HW_TDR_MAX
 */
typedef enum HWID {
  HW_WPE_EIS = 0,
  HW_WPE_TNR,
  HW_WPE_LITE,
  HW_ADL_A,
  HW_ADL_B,
  HW_TRAW,
  HW_LTRAW,
  HW_XTRAW,
  HW_DIP,
  HW_PQDIP_A,
  HW_PQDIP_B,
  HW_TDR_MAX,
  HW_ME = HW_TDR_MAX,
  HW_MAX,
} HWID;

/**********************************************************************
 * enum SecType : Buffer access permission
 *
 * Bus fabric conforms to ARM TrustZone and data protection to avoid
 * a slave can be accessed by one or more unexpected masters.
 *
 * ARM creates multiple exception state, each of which owns
 * a specific Exception Level (EL), from 1 to 3, respectively.
 * The increased exception level indicates a higher level of execution
 * privilege.
 *
 * Data protection is enforced by a MPU (Memory Protection Unit) and
 * a set of access permission between each master (domain)
 * and memory space (region). MPU ensures that only the domain with the right
 * access permission can read and/or write a corresponding region.
 *
 * Based on the aforementioned security fundamentals, we define buffer
 * access permission and memory type thereof.
 */

enum SecType {
  /**
   * Readable/Writable from EL0, EL1, EL2, Secure-EL0, Secure-EL1, EL3
   */
  mem_normal = 0,

  /**
   * Readable/Writable from EL2, Secure-EL0, Secure-EL1, EL3
   * Reference:
   * vendor/mediatek/proprietary/hardware/gralloc_extra/include/gralloc1_mtk_defs.h
   * GRALLOC1_USAGE_PROT, GRALLOC1_USAGE_PROT_*
   */
  mem_protected = 1,
  /**
   * Readable/Writable from Secure-EL0, Secure-EL1, EL3
   * Reference:
   * vendor/mediatek/proprietary/hardware/gralloc_extra/include/gralloc1_mtk_defs.h
   * GRALLOC1_USAGE_SECURE_CAMERA
   */
  mem_secure = 2
};

/**
 *  @struct dltb_t
 *  @brief information for a HW(refer HWID) in direct link table
 *
 *  @var on         indicating the hw is on or not
 *  @var src_fmt    src image format in direct link path, for module to update
 * path selection
 *  @var src_wd     src image width in direct link path, for module to update
 * tpipe structure
 *  @var src_ht     src image height in direct link path, for module to update
 * tpipe structure
 */
struct dltb_t {
  int on;
  unsigned int src_fmt;
  unsigned int src_wd;
  unsigned int src_ht;
};

/**
 *  @struct mvframeinfo_t
 *  @brief common me frame information for wpe/traw/dip resizer
 *
 *  @var meF0Width           width of meF0
 *  @var meF0Height          height of meF0
 *  @var meL0Width           width of meL0
 *  @var meL0Height          height of meL0
 *  @var meConfScaleRatio    ratio of MEL0FRMW and Conf_W
 */
struct mvframeinfo_t {
  unsigned int meF0Width;
  unsigned int meF0Height;
  unsigned int meL0Width;
  unsigned int meL0Height;
  unsigned int meConfScaleRatio;
};

/**
 *  @struct p_img4o_crop_info
 *  @brief common me frame information for wpe/dip resizer
 *
 *  @var p_img4o_crop_x         crop x start point of img4o
 *  @var p_img4o_crop_y         crop y start point of img4o
 *  @var p_mg4o_crop_w         crop width of img4o
 *  @var p_img4o_crop_h         crop height of img4o
 *  @var tnrwo_scale_ratio    ratio of tnrwo
 */
struct p_img4o_crop_info {
  int p_img4o_crop_x;
  int p_img4o_crop_y;
  int p_img4o_crop_w;
  int p_img4o_crop_h;
  int tnrwo_scale_ratio;
};

/**
 * enum job_thre_pr
 *
 * Definition about priority for jobs/tasks from mw
 */
enum job_thre_pr {
  job_thre_pr_high = 0,
  job_thre_pr_middle = 1,
  job_thre_pr_low = 2,
};

#endif  // HW_IMGSTREAM_INC_DRV_COMMON_7_1_HW_DEFINITION_H_

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

#ifndef HW_IMGSTREAM_INC_DRV_COMMON_7_1_MT8188_TUNING_HELPER_H_
#define HW_IMGSTREAM_INC_DRV_COMMON_7_1_MT8188_TUNING_HELPER_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "linux/mtkisp7/drv/7.1/hw_definition.h"
#include "linux/mtkisp7/drv/7.1/mt8188/mtk_img_metabuf.h"

enum PQDIPEnum {
  ENUMPQDIP_A = 0,  // 0x1
  ENUMPQDIP_B,
  ENUMPQDIP_MAX
};

struct slk_me_ctrl_t {
  uint32_t IN_WD;
  uint32_t IN_HT;
};

struct MwCtrlParams {
  struct slk_pqdip_ctrl_t
      pqdip_ctrl_slk[ENUMPQDIP_MAX];  // [0] PQ_DIP A, [1] pq_DIP B
  struct slk_me_ctrl_t me_ctrl_slk;
  int me_on;
  int mRequestNo;
  uint64_t frm_owner;
};

/********************************************************************
 *  Tuning Register Helper functions for tuning meta buffer
 *******************************************************************/
class TuningHelper {
 public:
  TuningHelper();
  ~TuningHelper();
  void init();
  void do_helper(struct mtk_img_uapi_meta_raw_stats_cfg* metai,
                 struct dltb_t dl_table[HW_TDR_MAX],
                 struct MwCtrlParams* ctrli);

 private:
  int dbgEnable;
  int moduleLSCEnable;
  int moduleLTMEnable;
  int moduleTNCEnable;
  int moduleMEEnable;
  int moduleDMEnable;
  int moduleSNRSEnable;
  int moduleTNREnable;
  int moduleSNREnable;
  int moduleCNREnable;
  int moduleEEEnable;
  int moduleTDSHAPAEnable;
  int moduleTDSHAPBEnable;
  int moduleRegDump;

  struct timespec dohelper_st;
  struct timespec dohelper_et;
  struct timespec lsc_st;
  struct timespec lsc_et;
  struct timespec ltm_st;
  struct timespec ltm_et;
  struct timespec tnc_st;
  struct timespec tnc_et;
  struct timespec me_st;
  struct timespec me_et;
  struct timespec dm_st;
  struct timespec dm_et;
  struct timespec dipallslk_st;
  struct timespec dipallslk_et;
  struct timespec tdshapa_st;
  struct timespec tdshapa_et;
  struct timespec tdshapb_st;
  struct timespec tdshapb_et;
};

#endif  // HW_IMGSTREAM_INC_DRV_COMMON_7_1_MT8188_TUNING_HELPER_H_

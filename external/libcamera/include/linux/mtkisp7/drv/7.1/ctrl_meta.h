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

#ifndef HW_IMGSTREAM_INC_DRV_COMMON_7_1_CTRL_META_H_
#define HW_IMGSTREAM_INC_DRV_COMMON_7_1_CTRL_META_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#include "hw_definition.h"

#define STOKEN_LISTLEN 16

enum token_type_e {
  token_set = 0,
  token_wait = 1
};

/**
 *  @struct stoken_t
 *  @brief sw token structure
 *
 *  @var type         token type: set or wait
 *  @var token_key    sw token key, would be translated to gce token id
 */
struct stoken_t {
  enum token_type_e type;
  int token_key; /*stating from 1*/
};
/**
 *  @struct common_ctrl
 *  @brief common part in ctrl meta structure
 *
 *  @var needDump           indicating need to dump ndd or not
 *  @var dumpHint           dump hint set from tuning
 *  @var timestamp          special time tag for matching dumped ndd data
 *  @var frame_no           Frame number in a specific enqueued request from mw 
 *  @var request_fd         Enqueued request fd in v4l2 kernel framework
 *  @var unique_key         Enqueued request number from mw  
 *  @var frm_owner          indicating the enqued sub-frame belongs to which user
 *  @var is_timeshared      the enqued sub-frame is time shared or not
 *  @var priority           the job priority about the sub-frame, refer enum job_thre_pr
 *  @var is_secureFrm       the sub-frame is secure type or not
 *  @var is_early_cb        the sub-frame needs early cb or not
 *  @var fps                requested fps
 *  @var dl_table           direct link table information in the sub-frame
 *  @var mvinfo             motion vector information in the sub-frame
 *  @var is_lowlatency      the sub-frame is low-latency usage or not
 *  @var syncid             specific sync-id for cam & img sync under low-latency case
 *  @var sync_next          need to sync with next frames in same package or not
 *  @var sync_prev          need to sync with previous frames in same package or not
 *  @var stage              debug message. indicate which sw stage the sib-frame in package is
 *  @var nddfp              ndd dump file path
 */
struct common_ctrl {
  int needDump; /* need bit-true dump or not */
  /*TuningUtils::FILE_DUMP_NAMING_HINT dumpHint;*/ /* isp bit-true fields */
  unsigned int timestamp;
  /*above is for bit-true dump*/
  int frame_no;
  int request_fd;
  int unique_key;
  uint64_t frm_owner;
  int is_timeshared; /* 0: normal, 1: vss */
  int priority;      /*multiple priority usage*/
  bool is_secureFrm;
  bool is_early_cb;
  unsigned int fps;
  struct dltb_t dl_table[HW_TDR_MAX]; /*direct link info table*/
  struct mvframeinfo_t mvinfo;        /*motion vector frame info from me*/
  struct p_img4o_crop_info img4o_crp;
  bool is_lowlatency;
  bool sync_next;
  bool sync_prev;
  int syncid;
  int stage;
  char nddfp[256];
  struct timeval enquetime;
  bool stoken_en;
  int stoken_num;
  struct stoken_t stoken[STOKEN_LISTLEN];
};

struct ctrl_meta_t {
  struct common_ctrl common;
  struct wpe_ctrl wpe_mdata[HW_ADL_A];
  struct traw_ctrl traw_mdata;
  struct dip_ctrl dip_mdata;
  struct pqdip_ctrl pqdip_mdata;
  struct me_ctrl me_meta;
  struct adl_ctrl adl_meta;
  YUFO_META_INFO wpe_ufo_meta;
  YUFO_META_INFO dip_ufo_meta;
};

#endif  // HW_IMGSTREAM_INC_DRV_COMMON_7_1_CTRL_META_H_

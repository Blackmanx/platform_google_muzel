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

#ifndef INCLUDE_MTKCAM_INTERFACES_DEF_PRIORITYDEFS_H_
#define INCLUDE_MTKCAM_INTERFACES_DEF_PRIORITYDEFS_H_
/******************************************************************************
 *  Priority Definitions.
 ******************************************************************************/
#include <stdint.h>
#include <sys/types.h>

/******************************************************************************
 *  Nice value (SCHED_OTHER)
 ******************************************************************************/
enum {
  NICE_PRIORITY_NORMAL = 0,
  NICE_PRIORITY_FOREGROUND = -2,
  //
  NICE_CAMERA_PASS1 = NICE_PRIORITY_FOREGROUND,
  NICE_CAMERA_PASS2 = NICE_PRIORITY_FOREGROUND,
  NICE_CAMERA_SM_PASS2 = NICE_PRIORITY_NORMAL - 8,
  //
  //
  // Lomo Jni for Matrix Menu of Effect
  NICE_CAMERA_LOMO = NICE_PRIORITY_NORMAL - 8,
  //
  //  3A-related
  NICE_CAMERA_3A_MAIN = (NICE_PRIORITY_NORMAL - 7),
  NICE_CAMERA_AE = (NICE_PRIORITY_NORMAL - 8),
  NICE_CAMERA_AF = (NICE_PRIORITY_NORMAL - 19),
  NICE_CAMERA_AWB = (NICE_PRIORITY_NORMAL - 8),
  NICE_CAMERA_FLK = (NICE_PRIORITY_NORMAL - 3),
  NICE_CAMERA_TSF = (NICE_PRIORITY_NORMAL - 8),
  NICE_CAMERA_STT_AF = (NICE_PRIORITY_NORMAL - 19),
  NICE_CAMERA_STT = (NICE_PRIORITY_NORMAL - 8),
  NICE_CAMERA_CCU = (NICE_PRIORITY_NORMAL - 4),
  NICE_CAMERA_AE_Start = (NICE_PRIORITY_NORMAL - 7),
  NICE_CAMERA_AF_Start = (NICE_PRIORITY_NORMAL - 7),
  NICE_CAMERA_ISP_BPCI = (NICE_PRIORITY_NORMAL - 7),
  NICE_CAMERA_CONFIG_STTPIPE = (NICE_PRIORITY_NORMAL - 7),
  NICE_CAMERA_ResultPool = (NICE_PRIORITY_NORMAL - 4),
  //
  // Pipeline-related
  NICE_CAMERA_PIPELINE_P1NODE = (NICE_PRIORITY_NORMAL - 4),
  //
  NICE_CAMERA_P1_ENQUE = (NICE_PRIORITY_NORMAL - 20),
  NICE_CAMERA_P1_DEQUE = (NICE_PRIORITY_NORMAL - 16),
  //
  NICE_CAMERA_ISP_ENQUE = (NICE_PRIORITY_NORMAL - 20),
  NICE_CAMERA_ISP_DEQUE = (NICE_PRIORITY_NORMAL - 20),
  //
  NICE_CAMERA_PIPEMGR_BASE = NICE_PRIORITY_FOREGROUND,
};

/******************************************************************************
 *
 ******************************************************************************/
#endif  // INCLUDE_MTKCAM_INTERFACES_DEF_PRIORITYDEFS_H_

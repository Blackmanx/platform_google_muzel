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

/********************************************************************************************
 * LEGAL DISCLAIMER
 *
 * (Header of MediaTek Software/Firmware Release or Documentation)
 *
 * BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
 *AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN
 *"AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 *EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER
 *DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF
 *ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE
 *MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
 *WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR
 *ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO CONFORM TO
 *A PARTICULAR STANDARD OR OPEN FORUM.
 *
 * BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 *LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT
 *MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR
 *REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK
 *FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH
 *THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS
 *PRINCIPLES.
 ************************************************************************************************/
/**
 * @file ae_setting.h
 * @brief AE mgr output setting
 */
#ifndef INCLUDE_MTKCAM_CORE_AAAHAL_AE_MGR_AE_SETTING_H_
#define INCLUDE_MTKCAM_CORE_AAAHAL_AE_MGR_AE_SETTING_H_

#define MAX_ANCHOR (30)

typedef enum AE_EXP_SETTING_MODE_T {
  AE_EXP_MODE_UNKNOWN_T = -1,
  AE_EXP_MODE_VVLE_T = 0,
  AE_EXP_MODE_VLE_T,
  AE_EXP_MODE_LLE_T,
  AE_EXP_MODE_LE_T,
  AE_EXP_MODE_NE_T,
  AE_EXP_MODE_ME_T,
  AE_EXP_MODE_SE_T,
  AE_EXP_MODE_SSE_T,
  AE_EXP_MODE_VSE_T,
  AE_EXP_MODE_VVSE_T,
  AE_EXP_MODE_MAX_T
} AE_EXP_MODE_T;

typedef struct {
  int32_t mode;
  int32_t ev;
  uint64_t exposure_ns;
  uint64_t exposure_line;
  uint32_t afe_gain;
  uint32_t isp_gain;
  uint32_t iris;
  uint32_t iso;
  uint32_t iso_base;
  uint32_t gainRA_x100;
} ae_exposure_setting;

typedef struct {
  uint32_t cnt;
  ae_exposure_setting table[AE_EXP_MODE_MAX_T];
} ae_exposure_setting_table;

typedef struct {
  int32_t mode;
  int32_t ratio;
  uint32_t dcg_gain;
  int32_t reserved[10];
} ae_calculation_setting;

typedef struct {
  uint32_t cnt;
  int32_t full_ratio;
  ae_calculation_setting table[AE_EXP_MODE_MAX_T];
} ae_calculation_setting_table;

typedef struct {
  uint64_t exp_max_ns;
  uint64_t exp_min_ns;
  uint32_t iso_ratio_max;
  uint32_t iso_ratio_min;
  int32_t reserved[10];
} ae_pline_anchor_point;

typedef struct {
  uint32_t cnt;
  ae_pline_anchor_point table[MAX_ANCHOR];
} ae_pline_anchor_table;

typedef struct {
  int32_t bZoomChange;
  int32_t u4XOffset;
  int32_t u4YOffset;
  uint32_t u4XWidth;
  uint32_t u4YHeight;
} AEZOOM_WINDOW_T;

typedef struct {
  uint32_t u4XLow;
  uint32_t u4XHigh;
  uint32_t u4YLow;
  uint32_t u4YHigh;
} AEAAO_WINDOW_T;



struct mtk_buf_ae_info {
  ae_exposure_setting_table ae_exp_table;
  ae_calculation_setting_table ae_calc_table;
  int32_t ev_compensate;
  int32_t ev_index;
  bool aai_enable;
  AEZOOM_WINDOW_T ae_zoom_win;
  AEAAO_WINDOW_T ae_aao_win;
};

#endif  // INCLUDE_MTKCAM_CORE_AAAHAL_AE_MGR_AE_SETTING_H_

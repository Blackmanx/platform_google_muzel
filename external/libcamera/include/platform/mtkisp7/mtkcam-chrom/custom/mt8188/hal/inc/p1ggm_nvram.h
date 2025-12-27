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

#ifndef _P1GGM_NVRAM_H_
#define _P1GGM_NVRAM_H_

#define P1GGM_TBL_SIZE                      (256)
#define P1GGM_TBL_NUM                       (2)
#define P1GGM_CONTRAST_WEIGHTING_TBL_NUM    (11)
#define P1GGM_LV_WEIGHTING_TBL_NUM          (20)
#define P1GGM_HIST_BIN                      (256)
#define P1GGM_NVRAM_START
typedef enum {
    eP1GGM_RETURN_NONE = 0x0,
    eP1GGM_RETURN_SUCCESS = 0x1,
    eP1GGM_ALLOCATE_BUFF_FAIL = 0x2,
    eP1GGM_NOT_SUPPORT = 0x4
}eP1GGM_RETURN_CODE;

typedef struct
{
    int enc_nvram_p1ggm[P1GGM_TBL_NUM][P1GGM_TBL_SIZE];
} p1ggm_curve_param_t;

typedef struct {
    int contrast_wt_tbl[P1GGM_CONTRAST_WEIGHTING_TBL_NUM];
    int lv_wt_tbl[P1GGM_LV_WEIGHTING_TBL_NUM];
} p1ggm_tuning_lut_t;

typedef struct {
    int enable;
    int wait_ae_stable;
    int speed;     // 0 ~ 10
} p1ggm_tuning_smoot_t;

typedef struct {
    int enable;
} p1ggm_tuning_flare_t;

typedef struct {
    int mode;  // 0: Fixed Gamma  1: Dynamic Gamma
    int low_contrast_thr;
    p1ggm_tuning_lut_t tuning_lut;
    p1ggm_tuning_smoot_t tuning_smooth;
    p1ggm_tuning_flare_t tuning_flare;
} p1ggm_tuning_param_t;

typedef struct {
    int lv;
    int ae_stable;
    int flare_offset;
    int ev_delta_index;
    int ev_ratio;
    int backlight_prob;
    int night_prob;
    int histogram[P1GGM_HIST_BIN];
} p1ggm_runtime_info_t;

typedef struct {
    p1ggm_curve_param_t curve_param;
    p1ggm_tuning_param_t tuning_param;
} P1GGM_NVRAM_T;

#endif


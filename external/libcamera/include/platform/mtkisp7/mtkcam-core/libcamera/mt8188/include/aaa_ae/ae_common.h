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

#ifndef __AE_COMMON_H__
#define __AE_COMMON_H__
#include <stdint.h>

/*** Define ***/
#define AE_BLOCK_SUB_MAX_SIZE (5)
#define AE_BLOCK_MAX_SIZE (15)
#define AE_AAO_MAX_HEIGHT (90)
#define AE_AAO_MAX_WIDTH (120)
#define AE_HIST_MAX_BIN (1024)

/*** Struct ***/
typedef struct
{
    uint32_t u4XWidth;
    uint32_t u4YHeight;
} AE_SIZE;

typedef struct
{
    uint32_t u4XLow;
    uint32_t u4XHi;
    uint32_t u4YLow;
    uint32_t u4YHi;
    uint32_t u4Weight;
} AE_AAO_FULL_WIN;

typedef struct
{
    uint32_t u4XOffset;
    uint32_t u4XWidth;
    uint32_t u4YOffset;
    uint32_t u4YHeight;
} AE_AAO_ZOOM_WIN;

typedef struct
{
    uint32_t stat_width;
    uint32_t stat_height;
    uint16_t aao_r[AE_AAO_MAX_WIDTH*AE_AAO_MAX_HEIGHT];
    uint16_t aao_g[AE_AAO_MAX_WIDTH*AE_AAO_MAX_HEIGHT];
    uint16_t aao_b[AE_AAO_MAX_WIDTH*AE_AAO_MAX_HEIGHT];
    uint16_t aao_y_le[AE_AAO_MAX_WIDTH*AE_AAO_MAX_HEIGHT];
    uint16_t aao_y_se[AE_AAO_MAX_WIDTH*AE_AAO_MAX_HEIGHT];
    uint32_t hist_r_le[AE_HIST_MAX_BIN];
    uint32_t hist_g_le[AE_HIST_MAX_BIN];
    uint32_t hist_b_le[AE_HIST_MAX_BIN];
    uint32_t hist_rgb_le[AE_HIST_MAX_BIN];
    uint32_t hist_y_le[AE_HIST_MAX_BIN];
    uint32_t hist_central_y_le[AE_HIST_MAX_BIN];
    uint32_t hist_r_se[AE_HIST_MAX_BIN];
    uint32_t hist_g_se[AE_HIST_MAX_BIN];
    uint32_t hist_b_se[AE_HIST_MAX_BIN];
    uint32_t hist_rgb_se[AE_HIST_MAX_BIN];
    uint32_t hist_y_se[AE_HIST_MAX_BIN];
    uint32_t hist_central_y_se[AE_HIST_MAX_BIN];
} AE_STAT_RAW_DATA;

typedef struct
{
    // AE Block
    uint32_t block_sub_y_le[AE_BLOCK_SUB_MAX_SIZE][AE_BLOCK_SUB_MAX_SIZE];
    uint32_t block_sub_y_se[AE_BLOCK_SUB_MAX_SIZE][AE_BLOCK_SUB_MAX_SIZE];
    uint32_t block_y_le[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t block_y_se[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t block_r[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t block_g[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t block_b[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];

    /* wide_tele le ae block */
    uint32_t wide_block_y[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t wide_block_r[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t wide_block_g[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t wide_block_b[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t tele_block_y[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t tele_block_r[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t tele_block_g[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t tele_block_b[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];

    uint32_t total_cnt_y_le;
    uint32_t total_cnt_y_se;

    // AE Over Exposure
    uint32_t oe_ratio_y;
    uint32_t oe_ratio_r;
    uint32_t oe_ratio_g;
    uint32_t oe_ratio_b;
    uint32_t block_oe_cnt_y[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t block_oe_cnt_r[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t block_oe_cnt_g[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];
    uint32_t block_oe_cnt_b[AE_BLOCK_MAX_SIZE][AE_BLOCK_MAX_SIZE];

    uint32_t oe_sum_y_se;
    uint32_t oe_cnt_y_se;

    // Linear Block
    uint32_t block_sort_y_le[AE_BLOCK_MAX_SIZE * AE_BLOCK_MAX_SIZE];
    uint32_t block_sort_y_se[AE_BLOCK_MAX_SIZE * AE_BLOCK_MAX_SIZE];

} AE_STAT_BLOCK_DATA;

typedef struct
{
    // request number
    uint32_t req_num;
    uint32_t key_num;

    AE_AAO_FULL_WIN aao_full_win;
    AE_AAO_ZOOM_WIN aao_zoom_win;

    /* wide telel window  */
    AE_AAO_FULL_WIN aao_wide_win;
    AE_AAO_FULL_WIN aao_tele_win;

    uint32_t cwv_y_le;          // center weighting value
    uint32_t cwv_y_se;          // center weighting value
    uint32_t avg_y_le;          // average weighting value
    uint32_t avg_y_se;          // average weighting value
    uint32_t central_y_le;      // Y value of central block
    uint32_t cwv_linear_y_le;   // center weighting value of linear AAO
    uint32_t block_weight_tbl[AE_BLOCK_SUB_MAX_SIZE][AE_BLOCK_SUB_MAX_SIZE];    //AE weighting table

    uint32_t hist_cnt_r_le;
    uint32_t hist_cnt_g_le;
    uint32_t hist_cnt_b_le;
    uint32_t hist_cnt_rgb_le;
    uint32_t hist_cnt_y_le;
    uint32_t hist_cnt_central_y_le;

    uint32_t hist_cnt_r_se;
    uint32_t hist_cnt_g_se;
    uint32_t hist_cnt_b_se;
    uint32_t hist_cnt_rgb_se;
    uint32_t hist_cnt_y_se;
    uint32_t hist_cnt_central_y_se;

    // color info
    uint32_t scene_sum_r;
    uint32_t scene_sum_g;
    uint32_t scene_sum_b;
    uint32_t color_distance;

    // ev comp
    uint32_t ae_ev_comp;
} AE_STAT_CALC_DATA;

#endif // __AE_STREAM_CORE__

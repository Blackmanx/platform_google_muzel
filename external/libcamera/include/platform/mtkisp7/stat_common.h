// SPDX-License-Identifier: MediaTekProprietary
#ifndef __STAT_COMMON_H__
#define __STAT_COMMON_H__
#include <stdint.h>

/*** Define ***/
#define AAO_MAX_HEIGHT (128)
#define AAO_MAX_WIDTH (128)
#define AAHO_MAX_BINS (1024)
#define TSFO_MAX_SIZE (14408)
#define LTMSO_MAX_SIZE (59952)
#define AIS2O_MAX_SIZE (128)
#define HW_MAX_SIZE (60)
#define AE_ZOOM_STEP_SIZE (10)

/*** Struct ***/
typedef struct aao_raw_data
{
    uint8_t stat_width;
    uint8_t stat_height;
    uint8_t avg_bitdepth;
    uint8_t dgn_bitdepth;
    uint8_t stat_multi_en; // 1: enable; 0: disable
    uint8_t hdr_mode;      // 1: HDR mode enable; 0: HDR mode disable from HWAA status
    uint16_t hdr_ratio;    // HDR_1x = 1000, Max vaule is 64000
    uint32_t r_avg_le[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint32_t g_avg_le[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint32_t b_avg_le[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint32_t r_sum_le[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint32_t g_sum_le[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint32_t b_sum_le[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint16_t r_cnt_le[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint16_t g_cnt_le[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint16_t b_cnt_le[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint16_t err_cnt_le[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint8_t res_le[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];

    uint32_t r_avg_se[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint32_t g_avg_se[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint32_t b_avg_se[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint32_t r_sum_se[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint32_t g_sum_se[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint32_t b_sum_se[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint16_t r_cnt_se[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint16_t g_cnt_se[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint16_t b_cnt_se[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint16_t err_cnt_se[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
    uint8_t res_se[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];

    uint32_t aao_y_le[AAO_MAX_WIDTH*AAO_MAX_HEIGHT]; // Le Y/ Full Y
    uint32_t aao_y_se[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];
} aao_raw_data;

typedef struct aaoy_raw_data
{
    uint8_t rgbw_en; // 1: enable; 0: disable
    uint32_t aao_y[AAO_MAX_WIDTH*AAO_MAX_HEIGHT]; // Full Y
} aaoy_raw_data;

typedef struct aaho_raw_data
{
    uint32_t hist_r_le[AAHO_MAX_BINS];
    uint32_t hist_g_le[AAHO_MAX_BINS];
    uint32_t hist_b_le[AAHO_MAX_BINS];
    uint32_t hist_rgb_le[AAHO_MAX_BINS];
    uint32_t hist_y_le[AAHO_MAX_BINS];
    uint32_t hist_central_y_le[AAHO_MAX_BINS];

    uint32_t hist_r_se[AAHO_MAX_BINS];
    uint32_t hist_g_se[AAHO_MAX_BINS];
    uint32_t hist_b_se[AAHO_MAX_BINS];
    uint32_t hist_rgb_se[AAHO_MAX_BINS];
    uint32_t hist_y_se[AAHO_MAX_BINS];
    uint32_t hist_central_y_se[AAHO_MAX_BINS];

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
} aaho_raw_data;

typedef struct aaho_map_data
{
    uint32_t aai_width;
    uint32_t aai_height;
    uint8_t aai_mode;
    uint8_t aai_map[AAO_MAX_WIDTH*AAO_MAX_HEIGHT];

    uint32_t hist_map_0_y_le[AAHO_MAX_BINS];
    uint32_t hist_map_1_y_le[AAHO_MAX_BINS];
    uint32_t hist_map_2_y_le[AAHO_MAX_BINS];
    uint32_t hist_map_3_y_le[AAHO_MAX_BINS];

    uint32_t hist_map_0_y_se[AAHO_MAX_BINS];
    uint32_t hist_map_1_y_se[AAHO_MAX_BINS];
    uint32_t hist_map_2_y_se[AAHO_MAX_BINS];
    uint32_t hist_map_3_y_se[AAHO_MAX_BINS];

    uint32_t hist_cnt_map_0_y_le;
    uint32_t hist_cnt_map_1_y_le;
    uint32_t hist_cnt_map_2_y_le;
    uint32_t hist_cnt_map_3_y_le;

    uint32_t hist_cnt_map_0_y_se;
    uint32_t hist_cnt_map_1_y_se;
    uint32_t hist_cnt_map_2_y_se;
    uint32_t hist_cnt_map_3_y_se;
} aaho_map_data;

typedef struct ltmso_raw_data
{
    uint32_t size;
    uint8_t data[LTMSO_MAX_SIZE];
} ltmso_raw_data;

typedef struct tsfo_raw_data
{
    uint32_t size;
    uint8_t data[TSFO_MAX_SIZE];
} tsfo_raw_data;

typedef struct ais2o_raw_data
{
    uint32_t size;
    uint8_t data[AIS2O_MAX_SIZE];
} ais2o_raw_data;

typedef struct hw_raw_data
{
    uint64_t r_avg_sum;
    uint64_t gr_avg_sum;
    uint64_t gb_avg_sum;
    uint64_t b_avg_sum;
    int32_t full_hdr_ratio;
    uint8_t data[HW_MAX_SIZE];
} hw_raw_data;

typedef struct aaa_stat_raw_data
{
    // request number
    uint32_t req_num;
    uint32_t key_num;

    aao_raw_data aao;
    aaho_raw_data aaho;
    aaho_map_data aaho_map;
    ltmso_raw_data ltmso;
    tsfo_raw_data tsfo;
    ais2o_raw_data ais2o;
    hw_raw_data hw;
    aaho_raw_data aaho_w;
    aaoy_raw_data aaoy_w;
} aaa_stat_raw_data;

#endif // __STAT_COMMON_H__

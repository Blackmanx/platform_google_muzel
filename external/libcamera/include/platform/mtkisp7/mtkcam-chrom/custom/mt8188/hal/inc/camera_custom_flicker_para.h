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


#ifndef _CAMERA_CUSTOM_FLICKER_PARA_H_
#define _CAMERA_CUSTOM_FLICKER_PARA_H_

#include <stdint.h>

typedef struct
{
    int32_t m;
    int32_t b_l;
    int32_t b_r;
    int32_t offset;
} FLICKER_CUST_STATISTICS;

typedef struct
{
    int64_t pclk;
    int32_t line_length;
    int32_t column_length;
} FLICEKR_SENSOR_SETTING;

typedef struct
{
    int32_t flickerFreq[9];
    int32_t flickerGradThreshold;
    int32_t flickerSearchRange;
    int32_t minPastFrames;
    int32_t maxPastFrames;
    FLICKER_CUST_STATISTICS EV50_L50;
    FLICKER_CUST_STATISTICS EV50_L60;
    FLICKER_CUST_STATISTICS EV60_L50;
    FLICKER_CUST_STATISTICS EV60_L60;
    int32_t EV50_thresholds[2];
    int32_t EV60_thresholds[2];
    int32_t freq_feature_index[2];
    FLICEKR_SENSOR_SETTING sensor_setting;
    int8_t is_auto_gen;
} FLICKER_CUST_PARA;

#endif // #ifndef _CAMERA_CUSTOM_FLICKER_PARA_H_



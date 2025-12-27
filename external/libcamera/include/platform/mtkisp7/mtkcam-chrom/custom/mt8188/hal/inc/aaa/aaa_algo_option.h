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

/**
 * @file aaa_algo_option.h
 * @brief 3A algorithm option
 */

#ifndef _AAA_ALGO_OPTION_H_
#define _AAA_ALGO_OPTION_H_

namespace NS3A
{

typedef enum
{
    EAAAOpt_MTK    = 0,
    EAAAOpt_OpenSource,
    EAAAOpt_NUM
} EAAAOpt_T;

#define USE_OPEN_SOURCE_AE       (0) // 0: use MTK AE algorithm; 1: use open source AE algorithm
#define USE_OPEN_SOURCE_AF       (0) // 0: use MTK AF algorithm; 1: use open source AF algorithm
#define USE_OPEN_SOURCE_AWB      (0) // 0: use MTK AWB algorithm; 1: use open source AWB algorithm
#define USE_OPEN_SOURCE_FLASH_AE (0) // 0: use MTK flash AE algorithm; 1: use open source flash AE algorithm

}; // namespace NS3A

#endif


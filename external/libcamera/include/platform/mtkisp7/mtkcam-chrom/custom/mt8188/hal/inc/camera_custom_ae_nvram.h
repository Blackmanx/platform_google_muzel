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


#ifndef _CAMERA_CUSTOM_AE_NVRAM_H_
#define _CAMERA_CUSTOM_AE_NVRAM_H_

#include <unordered_map>
#include "aaa_ae/ae_param.h"
#include "aaa_ae/ae_nvram.h"

#ifndef MBOOL
typedef int MBOOL;
#endif
#ifndef MINT8
typedef signed char         MINT8;
#endif
#ifndef MINT16
typedef signed short         MINT16;
#endif
#ifndef MINT32
typedef signed int         MINT32;
#endif
#ifndef MTRUE
#define MTRUE  1
#endif
#ifndef MFALSE
#define MFALSE 0
#endif

#define NVRAM_CUSTOM_AE_REVISION (8513004)
#define AE_TG_BLOCK_NO_X 12
#define AE_TG_BLOCK_NO_Y 9
#define AE_TG_BLOCK_TOTAL_SIZE (AE_TG_BLOCK_NO_X*AE_TG_BLOCK_NO_Y)

#endif

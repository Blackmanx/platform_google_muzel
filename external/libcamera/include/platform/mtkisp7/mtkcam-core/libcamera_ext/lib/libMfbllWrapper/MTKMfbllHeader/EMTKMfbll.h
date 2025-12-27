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

#ifndef _E_MTKMFBLL_H
#define _E_MTKMFBLL_H


typedef enum IPASS_PROC_IMAGE_FORMAT
{
    IPASS_PROC1_FMT_YV16 = 0, // 422 : 3 plane , Y..U..V..
    IPASS_PROC1_FMT_YUY2, // 422 : YUV YUV ...
    IPASS_PROC1_FMT_NV12, // 420 : 2 plane , Y... UVUV..
    IPASS_PROC1_FMT_Y,
    IPASS_PROC1_FMT_Y16bit,
    IPASS_PROC1_FMT_MAX      // maximum image format enum
} IPASS_PROC_IMAGE_FORMAT;
#endif
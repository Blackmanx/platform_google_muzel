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

#ifndef _E_MTK_BSS_H
#define _E_MTK_BSS_H

typedef enum IPASS_BSS_PROC_TYPE
{
    IPASS_BSS_TYPE_PACK_RAW10 = 0,
    IPASS_BSS_TYPE_PACK_Y10,
    IPASS_BSS_TYPE_UNPACK_NV21,
    IPASS_BSS_UNKNOWN_TYPE
} IPASS_BSS_PROC_TYPE;


typedef enum IPASS_BSS_PROC_ENUM
{
    IPASS_BSS_PROC1 = 0,
    IPASS_BSS_PROC2,
    IPASS_BSS_PROC3,
    IPASS_BSS_UNKNOWN_PROC,
} IPASS_BSS_PROC_ENUM;

#endif
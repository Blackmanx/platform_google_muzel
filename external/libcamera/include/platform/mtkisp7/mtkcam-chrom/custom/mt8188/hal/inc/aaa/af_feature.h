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

#ifndef _LIB3A_AF_FEATURE_H
#define _LIB3A_AF_FEATURE_H

// AF command ID: 0x2000 ~
typedef enum
{
    LIB3A_AF_CMD_ID_SET_AF_MODE  = 0x2000,     // Set AF Mode
    LIB3A_AF_CMD_ID_SET_AF_METER = 0x2001,     // Set AF Meter

} LIB3A_AF_CMD_ID_T;

// AF meter definition
typedef enum
{
    LIB3A_AF_METER_SPOT = 0,      // Spot Window
    LIB3A_AF_METER_MATRIX,                // Matrix Window
    LIB3A_AF_METER_FD,                 // FD Window
    LIB3A_AF_METER_CONTI,         // for AFC

    LIB3A_AF_METER_NUM,
    LIB3A_AF_METER_MIN = LIB3A_AF_METER_SPOT,
    LIB3A_AF_METER_MAX = LIB3A_AF_METER_CONTI

} LIB3A_AF_METER_T;

typedef enum
{
    FEATURE_PDAF_UNSUPPORT = 0,
    FEATURE_PDAF_SUPPORT_BNR_PDO = 1,
    FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL = 2,
    FEATURE_PDAF_SUPPORT_PBN_PDO = 4,
    FEATURE_PDAF_SUPPORT_LEGACY = 5,
    FEATURE_PDAF_SUPPORT_PDP = 6,
} FEATURE_PDAF_STATUS;

#endif

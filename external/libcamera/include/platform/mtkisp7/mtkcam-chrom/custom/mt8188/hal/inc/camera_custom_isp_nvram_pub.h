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


#ifndef _CAMERA_CUSTOM_NVRAM_ISP_H_
#define _CAMERA_CUSTOM_NVRAM_ISP_H_

#include "MediaTypes.h"

namespace NSIspTuning {

typedef MUINT32 FIELD;

/*******************************************************************************
* ISP NVRAM parameter
********************************************************************************/
#define NVRAM_ISP_REGS_ISO_GROUP_NUM    (10)

/*
*   CCM NVRAM Param
*/
#define ISP_NVRAM_DYNAMIC_CCM_NUM                       (4)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CCM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
  FIELD CCM_CNV_00 : 13; /*  0..12, 0x00001fff */
  FIELD rsv_13 : 3;      /* 13..15, 0x0000e000 */
  FIELD CCM_CNV_01 : 13; /* 16..28, 0x1fff0000 */
  FIELD rsv_29 : 3;      /* 29..31, 0xe0000000 */
} ISP_CCM_CNV_1_T;

typedef union {
  ISP_CCM_CNV_1_T bits;
  MUINT32 val;
} ISP_NVRAM_CCM_CNV_1;

typedef struct {
  FIELD CCM_CNV_02 : 13; /*  0..12, 0x00001fff */
  FIELD rsv_13 : 19;     /* 13..31, 0xffffe000 */
} ISP_CCM_CNV_2_T;

typedef union {
  ISP_CCM_CNV_2_T bits;
  MUINT32 val;
} ISP_NVRAM_CCM_CNV_2;

typedef struct {
  FIELD CCM_CNV_10 : 13; /*  0..12, 0x00001fff */
  FIELD rsv_13 : 3;      /* 13..15, 0x0000e000 */
  FIELD CCM_CNV_11 : 13; /* 16..28, 0x1fff0000 */
  FIELD rsv_29 : 3;      /* 29..31, 0xe0000000 */
} ISP_CCM_CNV_3_T;

typedef union {
  ISP_CCM_CNV_3_T bits;
  MUINT32 val;
} ISP_NVRAM_CCM_CNV_3;

typedef struct {
  FIELD CCM_CNV_12 : 13; /*  0..12, 0x00001fff */
  FIELD rsv_13 : 19;     /* 13..31, 0xffffe000 */
} ISP_CCM_CNV_4_T;

typedef union {
  ISP_CCM_CNV_4_T bits;
  MUINT32 val;
} ISP_NVRAM_CCM_CNV_4;

typedef struct {
  FIELD CCM_CNV_20 : 13; /*  0..12, 0x00001fff */
  FIELD rsv_13 : 3;      /* 13..15, 0x0000e000 */
  FIELD CCM_CNV_21 : 13; /* 16..28, 0x1fff0000 */
  FIELD rsv_29 : 3;      /* 29..31, 0xe0000000 */
} ISP_CCM_CNV_5_T;

typedef union {
  ISP_CCM_CNV_5_T bits;
  MUINT32 val;
} ISP_NVRAM_CCM_CNV_5;

typedef struct {
  FIELD CCM_CNV_22 : 13; /*  0..12, 0x00001fff */
  FIELD rsv_13 : 19;     /* 13..31, 0xffffe000 */
} ISP_CCM_CNV_6_T;

typedef union {
  ISP_CCM_CNV_6_T bits;
  MUINT32 val;
} ISP_NVRAM_CCM_CNV_6;

typedef union {
  struct {
    ISP_NVRAM_CCM_CNV_1 cnv_1;
    ISP_NVRAM_CCM_CNV_2 cnv_2;
    ISP_NVRAM_CCM_CNV_3 cnv_3;
    ISP_NVRAM_CCM_CNV_4 cnv_4;
    ISP_NVRAM_CCM_CNV_5 cnv_5;
    ISP_NVRAM_CCM_CNV_6 cnv_6;
  };
  enum { COUNT = 6 };
  MUINT32 set[COUNT];
} ISP_NVRAM_CCM_T;

}   // namespace NSIspTuning

using namespace NSIspTuning;
// AWB gain
#ifndef AWBGAINT
#define AWBGAINT
typedef struct
{
	int32_t i4R; // R gain
	int32_t i4G; // G gain
	int32_t i4B; // B gain
} AWB_GAIN_T;
#endif
typedef struct
{
    struct COLOR_T
    {
        MUINT32                   COLOR_Method;
    } COLOR;
    struct CCM_T
    {
        ISP_NVRAM_CCM_T           dynamic_CCM[ISP_NVRAM_DYNAMIC_CCM_NUM];
        AWB_GAIN_T                dynamic_CCM_AWBGain[ISP_NVRAM_DYNAMIC_CCM_NUM];
    } CCM;

} ISP_NVRAM_COLOR_COMM_T, *PISP_NVRAM_COLOR_COMM_T;

typedef struct
{
    ISP_NVRAM_COLOR_COMM_T        COMM;
} ISP_NVRAM_COLOR_TABLE_STRUCT, *PISP_NVRAM_COLOR_TABLE_STRUCT;

typedef struct
{
    // y = ax + b
    double a;
    double b;
} CoefLinear_T;

typedef struct
{
    CoefLinear_T S;
    CoefLinear_T O;
} DngNoiseProile_T;

typedef struct
{
    MINT32 i4RefereceIlluminant1;
    MINT32 i4RefereceIlluminant2;
    DngNoiseProile_T rNoiseProfile[4];
} ISP_NVRAM_DNG_METADATA_T, *PISP_NVRAM_DNG_METADATA_T;

typedef union
{
    struct  {
        MUINT32                         Version;
        MUINT32                         SensorId;    // ID of sensor module
        ISP_NVRAM_COLOR_TABLE_STRUCT    ISPColorTbl;
        ISP_NVRAM_DNG_METADATA_T        DngMetadata;
    };
} NVRAM_CAMERA_ISP_PARAM_STRUCT, *PNVRAM_CAMERA_ISP_PARAM_STRUCT;
#endif


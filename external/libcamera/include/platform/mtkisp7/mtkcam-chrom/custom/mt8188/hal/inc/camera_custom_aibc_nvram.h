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


#ifndef _CAMERA_CUSTOM_NVRAM_AIBC_H_
#define _CAMERA_CUSTOM_NVRAM_AIBC_H_

#include <stddef.h>
#include "MediaTypes.h"
#include "CFG_Camera_File_Max_Size.h"
#include "tuning_mapping/cam_idx_struct_ext.h"
#include "camera_custom_tuning_size.h"

using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Pre_Process
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  AIISP_EV_Sel_Size                                       : 32;
} FEATURE_AIISP_EV_Sel_Size_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Size_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Size_T;

typedef struct {
    FIELD  AIISP_EV_Sel_SE_Policy                                  : 32;
} FEATURE_AIISP_EV_Sel_SE_Policy_T;

typedef union {
    FEATURE_AIISP_EV_Sel_SE_Policy_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_SE_Policy_T;

typedef struct {
    FIELD  AIISP_EV_Sel_LE_Policy                                  : 32;
} FEATURE_AIISP_EV_Sel_LE_Policy_T;

typedef union {
    FEATURE_AIISP_EV_Sel_LE_Policy_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_LE_Policy_T;

typedef struct {
    FIELD  AIISP_EV_Sel_SSE_Policy                                 : 32;
} FEATURE_AIISP_EV_Sel_SSE_Policy_T;

typedef union {
    FEATURE_AIISP_EV_Sel_SSE_Policy_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_SSE_Policy_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Seq0                                       : 32;
} FEATURE_AIISP_EV_Sel_Seq0_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Seq0_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Seq0_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Seq1                                       : 32;
} FEATURE_AIISP_EV_Sel_Seq1_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Seq1_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Seq1_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Seq2                                       : 32;
} FEATURE_AIISP_EV_Sel_Seq2_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Seq2_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Seq2_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Seq3                                       : 32;
} FEATURE_AIISP_EV_Sel_Seq3_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Seq3_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Seq3_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Seq4                                       : 32;
} FEATURE_AIISP_EV_Sel_Seq4_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Seq4_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Seq4_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Seq5                                       : 32;
} FEATURE_AIISP_EV_Sel_Seq5_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Seq5_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Seq5_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Seq6                                       : 32;
} FEATURE_AIISP_EV_Sel_Seq6_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Seq6_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Seq6_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Seq7                                       : 32;
} FEATURE_AIISP_EV_Sel_Seq7_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Seq7_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Seq7_T;

typedef struct {
    FIELD  AIISP_EV_Sel_BSS                                        : 32;
} FEATURE_AIISP_EV_Sel_BSS_T;

typedef union {
    FEATURE_AIISP_EV_Sel_BSS_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_BSS_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_SE_LV00                              : 32;
} FEATURE_AIISP_EV_Sel_Table_SE_LV00_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_SE_LV00_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV00_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_SE_LV01                              : 32;
} FEATURE_AIISP_EV_Sel_Table_SE_LV01_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_SE_LV01_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV01_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_SE_LV02                              : 32;
} FEATURE_AIISP_EV_Sel_Table_SE_LV02_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_SE_LV02_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV02_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_SE_LV03                              : 32;
} FEATURE_AIISP_EV_Sel_Table_SE_LV03_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_SE_LV03_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV03_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_SE_LV04                              : 32;
} FEATURE_AIISP_EV_Sel_Table_SE_LV04_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_SE_LV04_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV04_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_SE_LV05                              : 32;
} FEATURE_AIISP_EV_Sel_Table_SE_LV05_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_SE_LV05_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV05_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_SE_LV06                              : 32;
} FEATURE_AIISP_EV_Sel_Table_SE_LV06_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_SE_LV06_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV06_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_SE_LV07                              : 32;
} FEATURE_AIISP_EV_Sel_Table_SE_LV07_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_SE_LV07_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV07_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_SE_LV08                              : 32;
} FEATURE_AIISP_EV_Sel_Table_SE_LV08_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_SE_LV08_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV08_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_SE_LV09                              : 32;
} FEATURE_AIISP_EV_Sel_Table_SE_LV09_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_SE_LV09_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV09_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_SE_LV10                              : 32;
} FEATURE_AIISP_EV_Sel_Table_SE_LV10_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_SE_LV10_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV10_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_SE_LV11                              : 32;
} FEATURE_AIISP_EV_Sel_Table_SE_LV11_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_SE_LV11_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV11_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_LE_LV00                              : 32;
} FEATURE_AIISP_EV_Sel_Table_LE_LV00_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_LE_LV00_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV00_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_LE_LV01                              : 32;
} FEATURE_AIISP_EV_Sel_Table_LE_LV01_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_LE_LV01_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV01_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_LE_LV02                              : 32;
} FEATURE_AIISP_EV_Sel_Table_LE_LV02_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_LE_LV02_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV02_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_LE_LV03                              : 32;
} FEATURE_AIISP_EV_Sel_Table_LE_LV03_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_LE_LV03_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV03_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_LE_LV04                              : 32;
} FEATURE_AIISP_EV_Sel_Table_LE_LV04_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_LE_LV04_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV04_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_LE_LV05                              : 32;
} FEATURE_AIISP_EV_Sel_Table_LE_LV05_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_LE_LV05_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV05_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_LE_LV06                              : 32;
} FEATURE_AIISP_EV_Sel_Table_LE_LV06_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_LE_LV06_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV06_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_LE_LV07                              : 32;
} FEATURE_AIISP_EV_Sel_Table_LE_LV07_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_LE_LV07_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV07_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_LE_LV08                              : 32;
} FEATURE_AIISP_EV_Sel_Table_LE_LV08_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_LE_LV08_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV08_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_LE_LV09                              : 32;
} FEATURE_AIISP_EV_Sel_Table_LE_LV09_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_LE_LV09_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV09_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_LE_LV10                              : 32;
} FEATURE_AIISP_EV_Sel_Table_LE_LV10_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_LE_LV10_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV10_T;

typedef struct {
    FIELD  AIISP_EV_Sel_Table_LE_LV11                              : 32;
} FEATURE_AIISP_EV_Sel_Table_LE_LV11_T;

typedef union {
    FEATURE_AIISP_EV_Sel_Table_LE_LV11_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV11_T;

typedef struct {
    FIELD  AIISP_Part1_Start                                       : 32;
} FEATURE_AIISP_EV_Part1_Start_T;

typedef union {
    FEATURE_AIISP_EV_Part1_Start_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Part1_Start_T;

typedef struct {
    FIELD  AIISP_Part1_End                                         : 32;
} FEATURE_AIISP_EV_Part1_End_T;

typedef union {
    FEATURE_AIISP_EV_Part1_End_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Part1_End_T;

typedef struct {
    FIELD  AIISP_Part2_Start                                       : 32;
} FEATURE_AIISP_EV_Part2_Start_T;

typedef union {
    FEATURE_AIISP_EV_Part2_Start_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Part2_Start_T;

typedef struct {
    FIELD  AIISP_Part2_End                                         : 32;
} FEATURE_AIISP_EV_Part2_End_T;

typedef union {
    FEATURE_AIISP_EV_Part2_End_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Part2_End_T;

typedef struct {
    FIELD  AIISP_Part3_Start                                       : 32;
} FEATURE_AIISP_EV_Part3_Start_T;

typedef union {
    FEATURE_AIISP_EV_Part3_Start_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Part3_Start_T;

typedef struct {
    FIELD  AIISP_Part3_End                                         : 32;
} FEATURE_AIISP_EV_Part3_End_T;

typedef union {
    FEATURE_AIISP_EV_Part3_End_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_Part3_End_T;

typedef struct {
    FIELD  AIISP_MStream_Start                                     : 32;
} FEATURE_AIISP_EV_MStream_Start_T;

typedef union {
    FEATURE_AIISP_EV_MStream_Start_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_MStream_Start_T;

typedef struct {
    FIELD  AIISP_MStream_End                                       : 32;
} FEATURE_AIISP_EV_MStream_End_T;

typedef union {
    FEATURE_AIISP_EV_MStream_End_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_MStream_End_T;

typedef struct {
    FIELD  AIISP_FEFM_RSZ_Ratio                                    : 32;
} FEATURE_AIISP_EV_FEFM_RSZ_Ratio_T;

typedef union {
    FEATURE_AIISP_EV_FEFM_RSZ_Ratio_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_FEFM_RSZ_Ratio_T;

typedef struct {
    FIELD  AIISP_ToneMappingStatus                                 : 32;
} FEATURE_AIISP_EV_ToneMappingStatus_T;

typedef union {
    FEATURE_AIISP_EV_ToneMappingStatus_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_EV_ToneMappingStatus_T;

typedef union {
    struct {
        FEATURE_NVRAM_AIISP_EV_Sel_Size_T              sel_size;
        FEATURE_NVRAM_AIISP_EV_Sel_SE_Policy_T         sel_se_policy;
        FEATURE_NVRAM_AIISP_EV_Sel_LE_Policy_T         sel_le_policy;
        FEATURE_NVRAM_AIISP_EV_Sel_SSE_Policy_T        sel_sse_policy;
        FEATURE_NVRAM_AIISP_EV_Sel_Seq0_T              sel_seq0;
        FEATURE_NVRAM_AIISP_EV_Sel_Seq1_T              sel_seq1;
        FEATURE_NVRAM_AIISP_EV_Sel_Seq2_T              sel_seq2;
        FEATURE_NVRAM_AIISP_EV_Sel_Seq3_T              sel_seq3;
        FEATURE_NVRAM_AIISP_EV_Sel_Seq4_T              sel_seq4;
        FEATURE_NVRAM_AIISP_EV_Sel_Seq5_T              sel_seq5;
        FEATURE_NVRAM_AIISP_EV_Sel_Seq6_T              sel_seq6;
        FEATURE_NVRAM_AIISP_EV_Sel_Seq7_T              sel_seq7;
        FEATURE_NVRAM_AIISP_EV_Sel_BSS_T               sel_bss;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV00_T     sel_table_se_lv00;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV01_T     sel_table_se_lv01;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV02_T     sel_table_se_lv02;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV03_T     sel_table_se_lv03;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV04_T     sel_table_se_lv04;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV05_T     sel_table_se_lv05;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV06_T     sel_table_se_lv06;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV07_T     sel_table_se_lv07;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV08_T     sel_table_se_lv08;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV09_T     sel_table_se_lv09;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV10_T     sel_table_se_lv10;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_SE_LV11_T     sel_table_se_lv11;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV00_T     sel_table_le_lv00;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV01_T     sel_table_le_lv01;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV02_T     sel_table_le_lv02;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV03_T     sel_table_le_lv03;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV04_T     sel_table_le_lv04;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV05_T     sel_table_le_lv05;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV06_T     sel_table_le_lv06;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV07_T     sel_table_le_lv07;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV08_T     sel_table_le_lv08;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV09_T     sel_table_le_lv09;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV10_T     sel_table_le_lv10;
        FEATURE_NVRAM_AIISP_EV_Sel_Table_LE_LV11_T     sel_table_le_lv11;
        FEATURE_NVRAM_AIISP_EV_Part1_Start_T           part1_start;
        FEATURE_NVRAM_AIISP_EV_Part1_End_T             part1_end;
        FEATURE_NVRAM_AIISP_EV_Part2_Start_T           part2_start;
        FEATURE_NVRAM_AIISP_EV_Part2_End_T             part2_end;
        FEATURE_NVRAM_AIISP_EV_Part3_Start_T           part3_start;
        FEATURE_NVRAM_AIISP_EV_Part3_End_T             part3_end;
        FEATURE_NVRAM_AIISP_EV_MStream_Start_T         mstream_start;
        FEATURE_NVRAM_AIISP_EV_MStream_End_T           mstream_end;
        FEATURE_NVRAM_AIISP_EV_FEFM_RSZ_Ratio_T        fefm_rsz_ratio;
        FEATURE_NVRAM_AIISP_EV_ToneMappingStatus_T     tonemappingstatus;
    };
    enum { COUNT = 47 };
    MUINT32 set[COUNT];
} FEATURE_NVRAM_AIISP_EV_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Pre_Process
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  AIISP_SWME_Enable                                       : 32;
} FEATURE_AIISP_LV_Pre_SWME_Enable_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Enable_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Enable_T;

typedef struct {
    FIELD  AIISP_SWME_SSE_Enable                                   : 32;
} FEATURE_AIISP_LV_Pre_SWME_SSE_Enable_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_SSE_Enable_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_SSE_Enable_T;

typedef struct {
    FIELD  AIISP_SWME_Reg00                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg00_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg00_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg00_T;

typedef struct {
    FIELD  AIISP_SWME_Reg01                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg01_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg01_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg01_T;

typedef struct {
    FIELD  AIISP_SWME_Reg02                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg02_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg02_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg02_T;

typedef struct {
    FIELD  AIISP_SWME_Reg03                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg03_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg03_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg03_T;

typedef struct {
    FIELD  AIISP_SWME_Reg04                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg04_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg04_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg04_T;

typedef struct {
    FIELD  AIISP_SWME_Reg05                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg05_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg05_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg05_T;

typedef struct {
    FIELD  AIISP_SWME_Reg06                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg06_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg06_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg06_T;

typedef struct {
    FIELD  AIISP_SWME_Reg07                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg07_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg07_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg07_T;

typedef struct {
    FIELD  AIISP_SWME_Reg08                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg08_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg08_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg08_T;

typedef struct {
    FIELD  AIISP_SWME_Reg09                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg09_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg09_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg09_T;

typedef struct {
    FIELD  AIISP_SWME_Reg10                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg10_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg10_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg10_T;

typedef struct {
    FIELD  AIISP_SWME_Reg11                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg11_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg11_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg11_T;

typedef struct {
    FIELD  AIISP_SWME_Reg12                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg12_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg12_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg12_T;

typedef struct {
    FIELD  AIISP_SWME_Reg13                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg13_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg13_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg13_T;

typedef struct {
    FIELD  AIISP_SWME_Reg14                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg14_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg14_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg14_T;

typedef struct {
    FIELD  AIISP_SWME_Reg15                                        : 32;
} FEATURE_AIISP_LV_Pre_SWME_Reg15_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SWME_Reg15_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg15_T;

typedef struct {
    FIELD  AIISP_ISO_ATMs_Pre_Idx                                  : 32;
} FEATURE_AIISP_LV_Pre_ISO_ATMs_Pre_Idx_T;

typedef union {
    FEATURE_AIISP_LV_Pre_ISO_ATMs_Pre_Idx_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_ISO_ATMs_Pre_Idx_T;

typedef struct {
    FIELD  AIISP_ISO_ATMs_APU_Idx                                  : 32;
} FEATURE_AIISP_LV_Pre_ISO_ATMs_APU_Idx_T;

typedef union {
    FEATURE_AIISP_LV_Pre_ISO_ATMs_APU_Idx_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_ISO_ATMs_APU_Idx_T;

typedef struct {
    FIELD  AIISP_ISO_ATMs_Post_Idx                                 : 32;
} FEATURE_AIISP_LV_Pre_ISO_ATMs_Post_Idx_T;

typedef union {
    FEATURE_AIISP_LV_Pre_ISO_ATMs_Post_Idx_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_ISO_ATMs_Post_Idx_T;

typedef struct {
    FIELD  AIISP_ISO_JPEG_Idx                                      : 32;
} FEATURE_AIISP_LV_Pre_ISO_JPEG_Idx_T;

typedef union {
    FEATURE_AIISP_LV_Pre_ISO_JPEG_Idx_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_ISO_JPEG_Idx_T;

typedef struct {
    FIELD  AIISP_SE_FaceFallBack_Ratio                             : 32;
} FEATURE_AIISP_LV_Pre_SE_FaceFallBack_Ratio_T;

typedef union {
    FEATURE_AIISP_LV_Pre_SE_FaceFallBack_Ratio_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_SE_FaceFallBack_Ratio_T;

typedef struct {
    FIELD  AIISP_NE_FaceFallBack_Ratio                             : 32;
} FEATURE_AIISP_LV_Pre_NE_FaceFallBack_Ratio_T;

typedef union {
    FEATURE_AIISP_LV_Pre_NE_FaceFallBack_Ratio_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_NE_FaceFallBack_Ratio_T;

typedef struct {
    FIELD  AIISP_LE_FaceFallBack_Ratio                             : 32;
} FEATURE_AIISP_LV_Pre_LE_FaceFallBack_Ratio_T;

typedef union {
    FEATURE_AIISP_LV_Pre_LE_FaceFallBack_Ratio_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_LV_Pre_LE_FaceFallBack_Ratio_T;

typedef union {
    struct {
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Enable_T           swme_enable;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_SSE_Enable_T       swme_sse_enable;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg00_T            swme_reg00;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg01_T            swme_reg01;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg02_T            swme_reg02;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg03_T            swme_reg03;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg04_T            swme_reg04;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg05_T            swme_reg05;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg06_T            swme_reg06;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg07_T            swme_reg07;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg08_T            swme_reg08;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg09_T            swme_reg09;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg10_T            swme_reg10;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg11_T            swme_reg11;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg12_T            swme_reg12;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg13_T            swme_reg13;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg14_T            swme_reg14;
        FEATURE_NVRAM_AIISP_LV_Pre_SWME_Reg15_T            swme_reg15;
        FEATURE_NVRAM_AIISP_LV_Pre_ISO_ATMs_Pre_Idx_T      iso_atms_pre_idx;
        FEATURE_NVRAM_AIISP_LV_Pre_ISO_ATMs_APU_Idx_T      iso_atms_apu_idx;
        FEATURE_NVRAM_AIISP_LV_Pre_ISO_ATMs_Post_Idx_T     iso_atms_post_idx;
        FEATURE_NVRAM_AIISP_LV_Pre_ISO_JPEG_Idx_T          iso_jpeg_idx;
        FEATURE_NVRAM_AIISP_LV_Pre_SE_FaceFallBack_Ratio_T se_facefallback_ratio;
        FEATURE_NVRAM_AIISP_LV_Pre_NE_FaceFallBack_Ratio_T ne_facefallback_ratio;
        FEATURE_NVRAM_AIISP_LV_Pre_LE_FaceFallBack_Ratio_T le_facefallback_ratio;
    };
    enum { COUNT = 25 };
    MUINT32 set[COUNT];
} FEATURE_NVRAM_AIISP_LV_Pre_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Pre_Process
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  AIISP_FEFM_ToneAlign00                                  : 32;
} FEATURE_AIISP_ISO_FEFM_ToneAlign00_T;

typedef union {
    FEATURE_AIISP_ISO_FEFM_ToneAlign00_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign00_T;

typedef struct {
    FIELD  AIISP_FEFM_ToneAlign01                                  : 32;
} FEATURE_AIISP_ISO_FEFM_ToneAlign01_T;

typedef union {
    FEATURE_AIISP_ISO_FEFM_ToneAlign01_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign01_T;

typedef struct {
    FIELD  AIISP_FEFM_ToneAlign02                                  : 32;
} FEATURE_AIISP_ISO_FEFM_ToneAlign02_T;

typedef union {
    FEATURE_AIISP_ISO_FEFM_ToneAlign02_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign02_T;

typedef struct {
    FIELD  AIISP_FEFM_ToneAlign03                                  : 32;
} FEATURE_AIISP_ISO_FEFM_ToneAlign03_T;

typedef union {
    FEATURE_AIISP_ISO_FEFM_ToneAlign03_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign03_T;

typedef struct {
    FIELD  AIISP_FEFM_ToneAlign04                                  : 32;
} FEATURE_AIISP_ISO_FEFM_ToneAlign04_T;

typedef union {
    FEATURE_AIISP_ISO_FEFM_ToneAlign04_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign04_T;

typedef struct {
    FIELD  AIISP_FEFM_ToneAlign05                                  : 32;
} FEATURE_AIISP_ISO_FEFM_ToneAlign05_T;

typedef union {
    FEATURE_AIISP_ISO_FEFM_ToneAlign05_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign05_T;

typedef struct {
    FIELD  AIISP_FEFM_ToneAlign06                                  : 32;
} FEATURE_AIISP_ISO_FEFM_ToneAlign06_T;

typedef union {
    FEATURE_AIISP_ISO_FEFM_ToneAlign06_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign06_T;

typedef struct {
    FIELD  AIISP_FEFM_ToneAlign07                                  : 32;
} FEATURE_AIISP_ISO_FEFM_ToneAlign07_T;

typedef union {
    FEATURE_AIISP_ISO_FEFM_ToneAlign07_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign07_T;

typedef union {
    struct {
        FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign00_T           tonealign00;
        FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign01_T           tonealign01;
        FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign02_T           tonealign02;
        FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign03_T           tonealign03;
        FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign04_T           tonealign04;
        FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign05_T           tonealign05;
        FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign06_T           tonealign06;
        FEATURE_NVRAM_AIISP_ISO_FEFM_ToneAlign07_T           tonealign07;
    };
    enum { COUNT = 8 };
    MUINT32 set[COUNT];
} FEATURE_NVRAM_AIISP_ISO_FEFM_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Pre_Process
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  AIISP_SWME_MMap_Enable                                  : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Enable_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Enable_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Enable_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg00                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg00_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg00_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg00_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg01                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg01_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg01_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg01_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg02                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg02_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg02_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg02_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg03                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg03_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg03_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg03_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg04                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg04_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg04_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg04_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg05                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg05_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg05_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg05_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg06                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg06_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg06_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg06_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg07                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg07_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg07_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg07_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg08                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg08_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg08_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg08_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg09                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg09_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg09_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg09_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg10                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg10_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg10_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg10_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg11                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg11_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg11_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg11_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg12                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg12_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg12_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg12_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg13                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg13_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg13_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg13_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg14                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg14_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg14_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg14_T;

typedef struct {
    FIELD  AIISP_SWME_MMap_Reg15                                   : 32;
} FEATURE_AIISP_ISO_SWME_MMap_Reg15_T;

typedef union {
    FEATURE_AIISP_ISO_SWME_MMap_Reg15_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg15_T;

typedef union {
    struct {
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Enable_T           mmap_enable;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg00_T            mmap_reg00;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg01_T            mmap_reg01;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg02_T            mmap_reg02;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg03_T            mmap_reg03;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg04_T            mmap_reg04;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg05_T            mmap_reg05;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg06_T            mmap_reg06;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg07_T            mmap_reg07;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg08_T            mmap_reg08;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg09_T            mmap_reg09;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg10_T            mmap_reg10;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg11_T            mmap_reg11;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg12_T            mmap_reg12;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg13_T            mmap_reg13;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg14_T            mmap_reg14;
        FEATURE_NVRAM_AIISP_ISO_SWME_MMap_Reg15_T            mmap_reg15;
    };
    enum { COUNT = 17 };
    MUINT32 set[COUNT];
} FEATURE_NVRAM_AIISP_ISO_SWME_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Model
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  AIISP_APU_Part1_Model                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_Model_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_Model_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_Model_T;

typedef struct {
    FIELD  AIISP_APU_Part1_chi_R                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_chi_R_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_chi_R_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_chi_R_T;

typedef struct {
    FIELD  AIISP_APU_Part1_chi_GR                                  : 32;
} FEATURE_AIISP_ISO_APU_Part1_chi_GR_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_chi_GR_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_chi_GR_T;

typedef struct {
    FIELD  AIISP_APU_Part1_chi_GB                                  : 32;
} FEATURE_AIISP_ISO_APU_Part1_chi_GB_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_chi_GB_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_chi_GB_T;

typedef struct {
    FIELD  AIISP_APU_Part1_chi_B                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_chi_B_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_chi_B_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_chi_B_T;

typedef struct {
    FIELD  AIISP_APU_Part1_std_R                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_std_R_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_std_R_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_std_R_T;

typedef struct {
    FIELD  AIISP_APU_Part1_std_GR                                  : 32;
} FEATURE_AIISP_ISO_APU_Part1_std_GR_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_std_GR_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_std_GR_T;

typedef struct {
    FIELD  AIISP_APU_Part1_std_GB                                  : 32;
} FEATURE_AIISP_ISO_APU_Part1_std_GB_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_std_GB_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_std_GB_T;

typedef struct {
    FIELD  AIISP_APU_Part1_std_B                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_std_B_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_std_B_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_std_B_T;

typedef struct {
    FIELD  AIISP_APU_Part1_blend_R                                 : 32;
} FEATURE_AIISP_ISO_APU_Part1_blend_R_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_blend_R_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_blend_R_T;

typedef struct {
    FIELD  AIISP_APU_Part1_blend_GR                                : 32;
} FEATURE_AIISP_ISO_APU_Part1_blend_GR_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_blend_GR_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_blend_GR_T;

typedef struct {
    FIELD  AIISP_APU_Part1_blend_GB                                : 32;
} FEATURE_AIISP_ISO_APU_Part1_blend_GB_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_blend_GB_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_blend_GB_T;

typedef struct {
    FIELD  AIISP_APU_Part1_blend_B                                 : 32;
} FEATURE_AIISP_ISO_APU_Part1_blend_B_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_blend_B_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_blend_B_T;

typedef struct {
    FIELD  AIISP_APU_Part1_Lambda                                  : 32;
} FEATURE_AIISP_ISO_APU_Part1_Lambda_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_Lambda_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_Lambda_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV00                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV00_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV00_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV00_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV01                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV01_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV01_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV01_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV02                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV02_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV02_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV02_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV03                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV03_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV03_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV03_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV04                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV04_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV04_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV04_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV05                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV05_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV05_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV05_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV06                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV06_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV06_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV06_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV07                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV07_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV07_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV07_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV08                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV08_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV08_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV08_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV09                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV09_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV09_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV09_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV10                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV10_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV10_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV10_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV11                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV11_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV11_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV11_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV12                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV12_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV12_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV12_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV13                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV13_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV13_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV13_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV14                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV14_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV14_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV14_T;

typedef struct {
    FIELD  AIISP_APU_Part1_RSV15                                   : 32;
} FEATURE_AIISP_ISO_APU_Part1_RSV15_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part1_RSV15_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV15_T;

typedef union {
    struct {
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_Model_T                 model;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_chi_R_T                 chi_r;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_chi_GR_T                chi_gr;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_chi_GB_T                chi_gb;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_chi_B_T                 chi_b;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_std_R_T                 std_r;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_std_GR_T                std_gr;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_std_GB_T                std_gb;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_std_B_T                 std_b;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_blend_R_T               blend_r;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_blend_GR_T              blend_gr;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_blend_GB_T              blend_gb;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_blend_B_T               blend_b;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_Lambda_T                lambda;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV00_T                 rsv00;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV01_T                 rsv01;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV02_T                 rsv02;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV03_T                 rsv03;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV04_T                 rsv04;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV05_T                 rsv05;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV06_T                 rsv06;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV07_T                 rsv07;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV08_T                 rsv08;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV09_T                 rsv09;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV10_T                 rsv10;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV11_T                 rsv11;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV12_T                 rsv12;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV13_T                 rsv13;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV14_T                 rsv14;
        FEATURE_NVRAM_AIISP_ISO_APU_Part1_RSV15_T                 rsv15;
    };
    enum { COUNT = 30 };
    MUINT32 set[COUNT];
} FEATURE_NVRAM_AIISP_ISO_APU_Part1_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Model
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  AIISP_APU_Part2_Model                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_Model_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_Model_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_Model_T;

typedef struct {
    FIELD  AIISP_APU_Part2_chi_R                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_chi_R_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_chi_R_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_chi_R_T;

typedef struct {
    FIELD  AIISP_APU_Part2_chi_GR                                  : 32;
} FEATURE_AIISP_ISO_APU_Part2_chi_GR_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_chi_GR_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_chi_GR_T;

typedef struct {
    FIELD  AIISP_APU_Part2_chi_GB                                  : 32;
} FEATURE_AIISP_ISO_APU_Part2_chi_GB_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_chi_GB_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_chi_GB_T;

typedef struct {
    FIELD  AIISP_APU_Part2_chi_B                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_chi_B_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_chi_B_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_chi_B_T;

typedef struct {
    FIELD  AIISP_APU_Part2_std_R                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_std_R_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_std_R_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_std_R_T;

typedef struct {
    FIELD  AIISP_APU_Part2_std_GR                                  : 32;
} FEATURE_AIISP_ISO_APU_Part2_std_GR_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_std_GR_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_std_GR_T;

typedef struct {
    FIELD  AIISP_APU_Part2_std_GB                                  : 32;
} FEATURE_AIISP_ISO_APU_Part2_std_GB_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_std_GB_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_std_GB_T;

typedef struct {
    FIELD  AIISP_APU_Part2_std_B                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_std_B_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_std_B_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_std_B_T;

typedef struct {
    FIELD  AIISP_APU_Part2_blend_R                                 : 32;
} FEATURE_AIISP_ISO_APU_Part2_blend_R_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_blend_R_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_blend_R_T;

typedef struct {
    FIELD  AIISP_APU_Part2_blend_GR                                : 32;
} FEATURE_AIISP_ISO_APU_Part2_blend_GR_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_blend_GR_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_blend_GR_T;

typedef struct {
    FIELD  AIISP_APU_Part2_blend_GB                                : 32;
} FEATURE_AIISP_ISO_APU_Part2_blend_GB_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_blend_GB_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_blend_GB_T;

typedef struct {
    FIELD  AIISP_APU_Part2_blend_B                                 : 32;
} FEATURE_AIISP_ISO_APU_Part2_blend_B_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_blend_B_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_blend_B_T;

typedef struct {
    FIELD  AIISP_APU_Part2_Lambda                                  : 32;
} FEATURE_AIISP_ISO_APU_Part2_Lambda_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_Lambda_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_Lambda_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV00                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV00_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV00_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV00_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV01                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV01_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV01_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV01_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV02                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV02_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV02_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV02_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV03                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV03_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV03_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV03_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV04                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV04_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV04_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV04_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV05                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV05_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV05_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV05_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV06                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV06_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV06_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV06_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV07                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV07_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV07_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV07_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV08                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV08_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV08_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV08_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV09                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV09_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV09_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV09_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV10                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV10_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV10_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV10_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV11                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV11_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV11_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV11_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV12                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV12_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV12_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV12_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV13                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV13_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV13_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV13_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV14                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV14_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV14_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV14_T;

typedef struct {
    FIELD  AIISP_APU_Part2_RSV15                                   : 32;
} FEATURE_AIISP_ISO_APU_Part2_RSV15_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part2_RSV15_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV15_T;

typedef union {
    struct {
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_Model_T                 model;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_chi_R_T                 chi_r;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_chi_GR_T                chi_gr;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_chi_GB_T                chi_gb;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_chi_B_T                 chi_b;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_std_R_T                 std_r;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_std_GR_T                std_gr;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_std_GB_T                std_gb;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_std_B_T                 std_b;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_blend_R_T               blend_r;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_blend_GR_T              blend_gr;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_blend_GB_T              blend_gb;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_blend_B_T               blend_b;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_Lambda_T                lambda;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV00_T                 rsv00;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV01_T                 rsv01;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV02_T                 rsv02;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV03_T                 rsv03;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV04_T                 rsv04;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV05_T                 rsv05;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV06_T                 rsv06;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV07_T                 rsv07;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV08_T                 rsv08;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV09_T                 rsv09;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV10_T                 rsv10;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV11_T                 rsv11;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV12_T                 rsv12;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV13_T                 rsv13;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV14_T                 rsv14;
        FEATURE_NVRAM_AIISP_ISO_APU_Part2_RSV15_T                 rsv15;
    };
    enum { COUNT = 30 };
    MUINT32 set[COUNT];
} FEATURE_NVRAM_AIISP_ISO_APU_Part2_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Model
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  AIISP_APU_Part3_Model                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_Model_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_Model_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_Model_T;

typedef struct {
    FIELD  AIISP_APU_Part3_chi_R                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_chi_R_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_chi_R_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_chi_R_T;

typedef struct {
    FIELD  AIISP_APU_Part3_chi_GR                                  : 32;
} FEATURE_AIISP_ISO_APU_Part3_chi_GR_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_chi_GR_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_chi_GR_T;

typedef struct {
    FIELD  AIISP_APU_Part3_chi_GB                                  : 32;
} FEATURE_AIISP_ISO_APU_Part3_chi_GB_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_chi_GB_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_chi_GB_T;

typedef struct {
    FIELD  AIISP_APU_Part3_chi_B                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_chi_B_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_chi_B_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_chi_B_T;

typedef struct {
    FIELD  AIISP_APU_Part3_std_R                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_std_R_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_std_R_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_std_R_T;

typedef struct {
    FIELD  AIISP_APU_Part3_std_GR                                  : 32;
} FEATURE_AIISP_ISO_APU_Part3_std_GR_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_std_GR_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_std_GR_T;

typedef struct {
    FIELD  AIISP_APU_Part3_std_GB                                  : 32;
} FEATURE_AIISP_ISO_APU_Part3_std_GB_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_std_GB_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_std_GB_T;

typedef struct {
    FIELD  AIISP_APU_Part3_std_B                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_std_B_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_std_B_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_std_B_T;

typedef struct {
    FIELD  AIISP_APU_Part3_blend_R                                 : 32;
} FEATURE_AIISP_ISO_APU_Part3_blend_R_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_blend_R_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_blend_R_T;

typedef struct {
    FIELD  AIISP_APU_Part3_blend_GR                                : 32;
} FEATURE_AIISP_ISO_APU_Part3_blend_GR_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_blend_GR_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_blend_GR_T;

typedef struct {
    FIELD  AIISP_APU_Part3_blend_GB                                : 32;
} FEATURE_AIISP_ISO_APU_Part3_blend_GB_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_blend_GB_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_blend_GB_T;

typedef struct {
    FIELD  AIISP_APU_Part3_blend_B                                 : 32;
} FEATURE_AIISP_ISO_APU_Part3_blend_B_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_blend_B_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_blend_B_T;

typedef struct {
    FIELD  AIISP_APU_Part3_Lambda                                  : 32;
} FEATURE_AIISP_ISO_APU_Part3_Lambda_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_Lambda_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_Lambda_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV00                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV00_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV00_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV00_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV01                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV01_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV01_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV01_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV02                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV02_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV02_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV02_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV03                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV03_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV03_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV03_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV04                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV04_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV04_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV04_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV05                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV05_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV05_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV05_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV06                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV06_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV06_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV06_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV07                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV07_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV07_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV07_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV08                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV08_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV08_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV08_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV09                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV09_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV09_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV09_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV10                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV10_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV10_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV10_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV11                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV11_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV11_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV11_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV12                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV12_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV12_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV12_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV13                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV13_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV13_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV13_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV14                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV14_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV14_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV14_T;

typedef struct {
    FIELD  AIISP_APU_Part3_RSV15                                   : 32;
} FEATURE_AIISP_ISO_APU_Part3_RSV15_T;

typedef union {
    FEATURE_AIISP_ISO_APU_Part3_RSV15_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV15_T;

typedef union {
    struct {
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_Model_T                 model;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_chi_R_T                 chi_r;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_chi_GR_T                chi_gr;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_chi_GB_T                chi_gb;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_chi_B_T                 chi_b;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_std_R_T                 std_r;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_std_GR_T                std_gr;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_std_GB_T                std_gb;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_std_B_T                 std_b;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_blend_R_T               blend_r;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_blend_GR_T              blend_gr;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_blend_GB_T              blend_gb;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_blend_B_T               blend_b;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_Lambda_T                lambda;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV00_T                 rsv00;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV01_T                 rsv01;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV02_T                 rsv02;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV03_T                 rsv03;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV04_T                 rsv04;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV05_T                 rsv05;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV06_T                 rsv06;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV07_T                 rsv07;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV08_T                 rsv08;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV09_T                 rsv09;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV10_T                 rsv10;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV11_T                 rsv11;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV12_T                 rsv12;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV13_T                 rsv13;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV14_T                 rsv14;
        FEATURE_NVRAM_AIISP_ISO_APU_Part3_RSV15_T                 rsv15;
    };
    enum { COUNT = 30 };
    MUINT32 set[COUNT];
} FEATURE_NVRAM_AIISP_ISO_APU_Part3_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Model
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  AIISP_PostSW_OB_Enable                                  : 32;
} FEATURE_AIISP_ISO_PostSW_OB_Enable_T;

typedef union {
    FEATURE_AIISP_ISO_PostSW_OB_Enable_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_PostSW_OB_Enable_T;

typedef struct {
    FIELD  AIISP_PostSW_OB_R                                       : 32;
} FEATURE_AIISP_ISO_PostSW_OB_R_T;

typedef union {
    FEATURE_AIISP_ISO_PostSW_OB_R_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_PostSW_OB_R_T;

typedef struct {
    FIELD  AIISP_PostSW_OB_GR                                      : 32;
} FEATURE_AIISP_ISO_PostSW_OB_GR_T;

typedef union {
    FEATURE_AIISP_ISO_PostSW_OB_GR_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_PostSW_OB_GR_T;

typedef struct {
    FIELD  AIISP_PostSW_OB_GB                                      : 32;
} FEATURE_AIISP_ISO_PostSW_OB_GB_T;

typedef union {
    FEATURE_AIISP_ISO_PostSW_OB_GB_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_PostSW_OB_GB_T;

typedef struct {
    FIELD  AIISP_PostSW_OB_B                                       : 32;
} FEATURE_AIISP_ISO_PostSW_OB_B_T;

typedef union {
    FEATURE_AIISP_ISO_PostSW_OB_B_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_PostSW_OB_B_T;

typedef struct {
    FIELD  AIISP_PostSW_DGN_Enable                                 : 32;
} FEATURE_AIISP_ISO_PostSW_DGN_Enable_T;

typedef union {
    FEATURE_AIISP_ISO_PostSW_DGN_Enable_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_PostSW_DGN_Enable_T;

typedef struct {
    FIELD  AIISP_PostSW_DGN_R                                      : 32;
} FEATURE_AIISP_ISO_PostSW_DGN_R_T;

typedef union {
    FEATURE_AIISP_ISO_PostSW_DGN_R_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_PostSW_DGN_R_T;

typedef struct {
    FIELD  AIISP_PostSW_DGN_GR                                     : 32;
} FEATURE_AIISP_ISO_PostSW_DGN_GR_T;

typedef union {
    FEATURE_AIISP_ISO_PostSW_DGN_GR_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_PostSW_DGN_GR_T;

typedef struct {
    FIELD  AIISP_PostSW_DGN_GB                                     : 32;
} FEATURE_AIISP_ISO_PostSW_DGN_GB_T;

typedef union {
    FEATURE_AIISP_ISO_PostSW_DGN_GB_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_PostSW_DGN_GB_T;

typedef struct {
    FIELD  AIISP_PostSW_DGN_B                                      : 32;
} FEATURE_AIISP_ISO_PostSW_DGN_B_T;

typedef union {
    FEATURE_AIISP_ISO_PostSW_DGN_B_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_PostSW_DGN_B_T;

typedef union {
    struct {
        FEATURE_NVRAM_AIISP_ISO_PostSW_OB_Enable_T             ob_enable;
        FEATURE_NVRAM_AIISP_ISO_PostSW_OB_R_T                  ob_r;
        FEATURE_NVRAM_AIISP_ISO_PostSW_OB_GR_T                 ob_gr;
        FEATURE_NVRAM_AIISP_ISO_PostSW_OB_GB_T                 ob_gb;
        FEATURE_NVRAM_AIISP_ISO_PostSW_OB_B_T                  ob_b;
        FEATURE_NVRAM_AIISP_ISO_PostSW_DGN_Enable_T            dgn_enable;
        FEATURE_NVRAM_AIISP_ISO_PostSW_DGN_R_T                 dgn_r;
        FEATURE_NVRAM_AIISP_ISO_PostSW_DGN_GR_T                dgn_gr;
        FEATURE_NVRAM_AIISP_ISO_PostSW_DGN_GB_T                dgn_gb;
        FEATURE_NVRAM_AIISP_ISO_PostSW_DGN_B_T                 dgn_b;
    };
    enum { COUNT = 10 };
    MUINT32 set[COUNT];
} FEATURE_NVRAM_AIISP_ISO_PostSW_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DRC
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg00                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg00_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg00_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg00_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg01                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg01_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg01_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg01_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg02                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg02_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg02_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg02_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg03                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg03_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg03_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg03_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg04                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg04_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg04_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg04_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg05                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg05_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg05_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg05_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg06                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg06_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg06_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg06_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg07                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg07_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg07_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg07_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg08                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg08_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg08_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg08_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg09                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg09_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg09_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg09_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg10                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg10_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg10_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg10_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg11                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg11_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg11_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg11_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg12                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg12_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg12_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg12_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg13                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg13_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg13_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg13_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg14                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg14_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg14_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg14_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg15                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg15_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg15_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg15_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg16                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg16_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg16_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg16_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg17                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg17_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg17_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg17_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg18                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg18_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg18_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg18_T;

typedef struct {
    FIELD  AIISP_DRC_Statistics_Reg19                              : 32;
} FEATURE_AIISP_ISO_DRC_Statistics_Reg19_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Statistics_Reg19_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg19_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg00                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg00_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg00_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg00_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg01                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg01_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg01_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg01_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg02                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg02_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg02_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg02_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg03                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg03_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg03_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg03_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg04                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg04_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg04_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg04_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg05                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg05_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg05_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg05_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg06                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg06_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg06_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg06_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg07                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg07_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg07_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg07_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg08                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg08_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg08_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg08_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg09                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg09_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg09_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg09_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg10                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg10_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg10_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg10_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg11                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg11_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg11_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg11_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg12                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg12_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg12_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg12_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg13                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg13_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg13_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg13_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg14                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg14_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg14_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg14_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg15                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg15_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg15_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg15_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg16                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg16_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg16_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg16_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg17                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg17_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg17_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg17_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg18                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg18_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg18_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg18_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg19                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg19_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg19_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg19_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg20                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg20_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg20_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg20_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg21                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg21_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg21_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg21_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg22                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg22_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg22_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg22_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg23                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg23_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg23_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg23_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg24                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg24_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg24_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg24_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg25                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg25_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg25_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg25_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg26                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg26_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg26_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg26_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg27                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg27_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg27_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg27_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg28                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg28_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg28_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg28_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg29                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg29_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg29_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg29_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg30                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg30_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg30_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg30_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg31                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg31_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg31_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg31_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg32                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg32_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg32_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg32_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg33                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg33_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg33_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg33_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg34                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg34_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg34_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg34_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg35                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg35_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg35_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg35_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg36                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg36_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg36_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg36_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg37                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg37_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg37_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg37_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg38                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg38_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg38_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg38_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg39                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg39_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg39_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg39_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg40                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg40_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg40_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg40_T;

typedef struct {
    FIELD  AIISP_DRC_Cal_Reg41                                     : 32;
} FEATURE_AIISP_ISO_DRC_Cal_Reg41_T;

typedef union {
    FEATURE_AIISP_ISO_DRC_Cal_Reg41_T bits;
    MUINT32 val;
} FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg41_T;

typedef union {
    struct {
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg00_T      statistics_reg00;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg01_T      statistics_reg01;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg02_T      statistics_reg02;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg03_T      statistics_reg03;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg04_T      statistics_reg04;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg05_T      statistics_reg05;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg06_T      statistics_reg06;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg07_T      statistics_reg07;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg08_T      statistics_reg08;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg09_T      statistics_reg09;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg10_T      statistics_reg10;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg11_T      statistics_reg11;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg12_T      statistics_reg12;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg13_T      statistics_reg13;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg14_T      statistics_reg14;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg15_T      statistics_reg15;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg16_T      statistics_reg16;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg17_T      statistics_reg17;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg18_T      statistics_reg18;
        FEATURE_NVRAM_AIISP_ISO_DRC_Statistics_Reg19_T      statistics_reg19;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg00_T             cal_reg00;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg01_T             cal_reg01;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg02_T             cal_reg02;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg03_T             cal_reg03;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg04_T             cal_reg04;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg05_T             cal_reg05;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg06_T             cal_reg06;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg07_T             cal_reg07;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg08_T             cal_reg08;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg09_T             cal_reg09;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg10_T             cal_reg10;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg11_T             cal_reg11;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg12_T             cal_reg12;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg13_T             cal_reg13;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg14_T             cal_reg14;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg15_T             cal_reg15;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg16_T             cal_reg16;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg17_T             cal_reg17;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg18_T             cal_reg18;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg19_T             cal_reg19;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg20_T             cal_reg20;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg21_T             cal_reg21;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg22_T             cal_reg22;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg23_T             cal_reg23;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg24_T             cal_reg24;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg25_T             cal_reg25;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg26_T             cal_reg26;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg27_T             cal_reg27;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg28_T             cal_reg28;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg29_T             cal_reg29;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg30_T             cal_reg30;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg31_T             cal_reg31;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg32_T             cal_reg32;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg33_T             cal_reg33;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg34_T             cal_reg34;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg35_T             cal_reg35;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg36_T             cal_reg36;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg37_T             cal_reg37;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg38_T             cal_reg38;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg39_T             cal_reg39;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg40_T             cal_reg40;
        FEATURE_NVRAM_AIISP_ISO_DRC_Cal_Reg41_T             cal_reg41;
    };
    enum { COUNT = 62 };
    MUINT32 set[COUNT];
} FEATURE_NVRAM_AIISP_ISO_DRC_T;

#endif // _CAMERA_CUSTOM_NVRAM_H_


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

#ifndef HW_IMGSTREAM_INC_DRV_COMMON_7_0_TRAW_META_H_
#define HW_IMGSTREAM_INC_DRV_COMMON_7_0_TRAW_META_H_

#include <stdio.h>
#include <stdlib.h>


/************************************************************************
 * Enum Define
 ************************************************************************/
/**
 * @brief YUV420 select
 */
typedef enum TrawScenarioTag {
  TRAW_SCENARIO_NORMAL, /*!< Normal flow */
  TRAW_SCENARIO_FE = 2, /*!< FE flow */
} TRAW_SCENARIO_TAG;

/**
 * @brief HW ID
 */
typedef enum TrawHWID {
  TRAW_HW_TRAW,  /*!< Select TRAW HW */
  TRAW_HW_LTRAW, /*!< Select LTRAW HW */
  TRAW_HW_XTRAW, /*!< Select XTRAW HW */
} TRAW_HW_ID;

/**
 * @brief Crp dump select
 */
typedef enum TrawCrpDumpSelect {
  TRAW_CRP_LTM, /*!< CRP_LTM */
  TRAW_CRP_HLR, /*!< CRP_HLR */
  TRAW_CRP_LSC, /*!< CRP_LSC */
  TRAW_CRP_DGN, /*!< CRP_DGN */
  TRAW_CRP_CRNR /*!< CRP_CRNR */
} TRAW_CRP_DUMP_SELECT;

/**
 * @brief Timgo dump select
 */
typedef enum TrawTimgoDumpSelect {
  TRAW_TIMGO_CRP, /*!< CRP_T1 path */
  TRAW_TIMGO_CCM  /*!< CCM_T1 path */
} TRAW_TIMGO_DUMP_SELECT;

/**
 * @brief Prc raw type select
 */
typedef enum TrawPrcRawTypeSelect {
  TRAW_PRC_RAW_TYPE_NONE, /*!< None */
  TRAW_PRC_RAW_TYPE_20B,  /*!< DGN input*/
  TRAW_PRC_RAW_TYPE_16B   /*!< LSC input */
} TRAW_PRC_RAW_TYPE_SELECT;

/**
 * @brief Resizer ratio
 */
typedef enum TrawResizeRatio {
    TRAW_RESIZE_ANYRATIO,    /*!< Any ratio */
    TRAW_RESIZE_DOWN4,       /*!< Down4 */
    TRAW_RESIZE_DOWN2,       /*!< Down2 */
    TRAW_RESIZE_DOWN42       /*!< Down42 */
}TRAW_RESIZE_RATIO;

/*************************************************************************
 * Structure Define
 *************************************************************************/
/**
 * @brief FE info
 */
typedef struct TrawFEInfo {
  unsigned int DSCR_SBIT; /*!< DSCR_SBIT */
  unsigned int TH_C;      /*!< TH_C */
  unsigned int TH_G;      /*!< TH_G */
  unsigned int FLT_EN;    /*!< FLT_EN */
  unsigned int PARAM;     /*!< PARAM */
  unsigned int MODE;      /*!< MODE */
  unsigned int YIDX;      /*!< YIDX */
  unsigned int XIDX;      /*!< XIDX */
  unsigned int START_X;   /*!< START_X */
  unsigned int START_Y;   /*!< START_Y */
  unsigned int IN_HT;     /*!< IN_HT */
  unsigned int IN_WD;     /*!< IN_WD */
} TRAW_FE_INFO;

/**
 * @brief Resizer info
 */
typedef struct TrawResizerInfo {
  unsigned char DRZH2NT2_APL_EN; /*!< DRZH2NT2_APL_EN */
} TRAW_RESIZER_INFO;

/**
 * @brief FE srz
 */
typedef struct TrawFESrz {
  unsigned int SrzId;             /*!< Srz id */
  TRAW_RESIZE_RATIO RszRatio;     /*!< Resize ratio */
  unsigned int InWidth;           /*!< In width */
  unsigned int InHeight;          /*!< In height */
  unsigned int OutWidth;          /*!< Out width */
  unsigned int OutHeight;         /*!< Out height */
  unsigned int CropX;             /*!< Crop X */
  unsigned int CropY;             /*!< Crop Y */
  unsigned int CropFloatX;        /*!< Crop floatX */
  unsigned int CropFloatY;        /*!< Crop floatY */
  unsigned int CropWidth;         /*!< Crop width */
  unsigned int CropHeight;        /*!< Crop height */
} TRAW_FE_SRZ;

/**
 * @brief ctrl meta usage for traw driver
 */
typedef struct traw_ctrl {
  TRAW_SCENARIO_TAG StreamTag;         /*!< Stream tag */
  TRAW_HW_ID HWID;                     /*!< 0:TRAW, 1:LTRAW */
  TRAW_TIMGO_DUMP_SELECT TimgoDumpSel; /*!< Timgo dump select */
  TRAW_CRP_DUMP_SELECT CrpDumpSel;     /*!< Crp dump select */
  TRAW_PRC_RAW_TYPE_SELECT PrcRawType; /*!< Processed raw type select */
  TRAW_FE_INFO FEInfo;                 /*!< FE info */
  TRAW_RESIZER_INFO ResizerInfo;       /*!< Resizer info */
  TRAW_FE_SRZ FESrz;                   /*!< FE Resizer info */
} TRAW_CTRL_META;

#endif  // HW_IMGSTREAM_INC_DRV_COMMON_7_0_TRAW_META_H_

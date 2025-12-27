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

#ifndef HW_IMGSTREAM_INC_DRV_COMMON_7_1_WPE_META_H_
#define HW_IMGSTREAM_INC_DRV_COMMON_7_1_WPE_META_H_

#include <stdio.h>
#include <stdlib.h>

/*******************************************************************************
 * Enum Define
 ********************************************************************************/
/**
 * @enum WpeMode
 * @brief
 */
typedef enum WpeMode {
  EW_HW_DEFAULT = 0,
  EW_MODE_3rdParty = (1UL << 0),
  EW_HW_EIS  = (1UL << 1),
  EW_HW_TNR  = (1UL << 2),
  EW_HW_LITE  = (1UL << 3),
} WPE_MODE;

/**
 * @enum RgbMode
 * @brief wpe process rgb as uv422, 2 pass: 1st pass for odd line; 2nd pass for
 * even line
 */
typedef enum RgbMode {
  EW_RGB_ODD_LINE = 0,  // for RG
  EW_RGB_EVEN_LINE,     // for GB
  EW_TOTAL_RGB_MODE
} WPE_RGB_MODE;

/**
 * @enum ExtraFeatureIndex
 * @brief extra featue index
 */
typedef enum ExtraFeatureIndex {
  EW_NONE = 0,
  EW_PSP_BORDER_COLOR = (1UL << 0),  // user defined psp border / isp7.0 only
  EW_MVMAP = (1UL << 1),             // map to WPE is mv map
  EW_DEBUG_WPEO = (1UL << 2),        // dl, dump as dl content
  EW_IROI = (1UL << 3),              // input ROI
  EW_VGENIN_OFST = (1UL << 4),       // vgen_in ofst
  EW_TOTAL_INDEX
} WPE_EXTRA_FEATURE_IDX;

/**
 * @enum PspTblSel
 * @brief PSP coefficient table select
 */
typedef enum PspTblSel {
  EW_PSP_TB_DEFAULT,  // default
  //
  EW_PSP_TB_SEL_ISP_1,  // ISP generation based table1
  EW_PSP_TB_SEL_ISP_2,  // ISP generation based table2
  EW_PSP_TB_SEL_ISP_3,  // ISP generation based table3
  //
  EW_PSP_TB_SEL_USER,
  EW_PSP_TB_SEL_0,  // bi-cubic
  EW_PSP_TB_SEL_1,  // bi-linear
  EW_PSP_TB_SEL_MAX
} WPE_PSP_TBL_SEL;

/*******************************************************************************
 * Structure Define
 ********************************************************************************/
struct drv_wpe_info_t {};

struct WPE_CrpInfo {
  unsigned int x_start_point;
  unsigned int x_end_point;
  unsigned int y_start_point;
  unsigned int y_end_point;
};

struct WPE_SzInfo {
  unsigned int wd;
  unsigned int ht;
};

struct WPE_CrpOfstInfo {
  unsigned int x_start;  // tile align
  unsigned int hr_int_ofst;
  unsigned int hr_sub_ofst;
  unsigned int y_start;  // tile align
  unsigned int vt_int_ofst;
  unsigned int vt_sub_ofst;
  unsigned int wd;
  unsigned int ht;
};

/**
 * @struct wpe_ctrl
 * @brief ctrl meta usage for wpe driver
 */
struct wpe_ctrl {
  WPE_MODE wpe_mode;            // Direct link mode
  struct WPE_CrpInfo vgen_out;  // VGEN out

  /* PSP, ISP7.0 only */
  WPE_PSP_TBL_SEL tbl_sel_v;  // isp7, default: bi-cubic
  WPE_PSP_TBL_SEL tbl_sel_h;  // isp7, default: bi-cubic

  /* Extra feature */
  unsigned int extra_feature_idx;  // enum WPE_EXTRA_FEATURE_IDX
  WPE_RGB_MODE rgb_mode;           // for format=bayer/rgb]
  struct WPE_CrpOfstInfo vgen_in;  // EW_VGENIN_OFST, full in, ref. partial

  /* for extra_feature_idx|=EWPE_PSP_BORDER_COLOR, <ISP7:8bits;>=ISP7:12bits */
  unsigned int psp_border_color_y;
  unsigned int psp_border_color_u;
  unsigned int psp_border_color_v;
};

#endif  // HW_IMGSTREAM_INC_DRV_COMMON_7_1_WPE_META_H_

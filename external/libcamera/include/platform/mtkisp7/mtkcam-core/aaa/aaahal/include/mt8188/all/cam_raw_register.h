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

#ifndef AAA_AAAHAL_INCLUDE_PRIVATE_MT8188_ALL_CAM_RAW_REGISTER_H_
#define AAA_AAAHAL_INCLUDE_PRIVATE_MT8188_ALL_CAM_RAW_REGISTER_H_

// Usage example: To print value of "MTK_CAM_BPC_BPC_FUNC_CON_BPC_BPC_LUT_BIT_EXTEND_EN" in "val"
// > printf("%x", GET_MTK_CAM(val, MTK_CAM_BPC_BPC_FUNC_CON_BPC_BPC_LUT_BIT_EXTEND_EN));
#define GET_MTK_CAM(val, field) ((val & field##_MASK) >> field##_SHIFT)
// Usage example: To set "val_of_bpc_lut_bit_extend_en" to bits of "MTK_CAM_BPC_BPC_FUNC_CON_BPC_BPC_LUT_BIT_EXTEND_EN" in "val"
// > val = SET_MTK_CAM(val, MTK_CAM_BPC_BPC_FUNC_CON_BPC_BPC_LUT_BIT_EXTEND_EN, val_of_bpc_lut_bit_extend_en);
#define SET_MTK_CAM(val, field, set_val) ((val & ~field##_MASK) | ((set_val << field##_SHIFT) & field##_MASK))

/**
 * Bit Feild of BPC_FUNC_CON: BPC_EN
 * MTK_CAM_BPC_FUNC_CON_BPC_EN: [31, 31]
 * Enable/disable for BPC Correction
 * 1'd1: enable the function
 * 1'd0: disable the function
 */
#define MTK_CAM_BPC_FUNC_CON_BPC_EN_MASK   0x80000000
#define MTK_CAM_BPC_FUNC_CON_BPC_EN_SHIFT  31

/**
 * Bit Feild of BPC_FUNC_CON: BPC_CT_EN
 * MTK_CAM_BPC_FUNC_CON_BPC_CT_EN: [30, 30]
 * Enable/disable for Cross-Talk compensation
 * 1'd1: enable
 * 1'd0: disable
 */
#define MTK_CAM_BPC_FUNC_CON_BPC_CT_EN_MASK   0x40000000
#define MTK_CAM_BPC_FUNC_CON_BPC_CT_EN_SHIFT  30

/**
 * Bit Feild of BPC_FUNC_CON: BPC_PDC_EN
 * MTK_CAM_BPC_FUNC_CON_BPC_PDC_EN: [29, 29]
 * Enable/disable for PDC correction
 * 1'd1: enable
 * 1'd0: disable
 */
#define MTK_CAM_BPC_FUNC_CON_BPC_PDC_EN_MASK   0x20000000
#define MTK_CAM_BPC_FUNC_CON_BPC_PDC_EN_SHIFT  29

/**
 * Bit Feild of BPC_FUNC_CON: BPC_LUT_EN
 * MTK_CAM_BPC_FUNC_CON_BPC_LUT_EN: [28, 28]
 * Enable table lookup
 * 1'd1:  enable BPC with default table mode
 * 1'd0:  disable BPC with default table mode
 */
#define MTK_CAM_BPC_FUNC_CON_BPC_LUT_EN_MASK   0x10000000
#define MTK_CAM_BPC_FUNC_CON_BPC_LUT_EN_SHIFT  28

/**
 * Bit Feild of BPC_FUNC_CON: BPC_LUT_BIT_EXTEND_EN
 * MTK_CAM_BPC_FUNC_CON_BPC_LUT_BIT_EXTEND_EN: [0, 0]
 * Enable table 24 bits mode
 * 1'd1: Table format to be 24 bits
 * 1'd0: @ the original format, tbale to be 16 bits mode
 */
#define MTK_CAM_BPC_FUNC_CON_BPC_LUT_BIT_EXTEND_EN_MASK   0x00000001
#define MTK_CAM_BPC_FUNC_CON_BPC_LUT_BIT_EXTEND_EN_SHIFT  0

/**
 * Bit Feild of CCM_CNV_1: CCM_CNV_01
 * MTK_CAM_CCM_CNV_1_CCM_CNV_01: [16, 28]
 * matrix 0,1 coefficient
 */
#define MTK_CAM_CCM_CNV_1_CCM_CNV_01_MASK   0x1fff0000
#define MTK_CAM_CCM_CNV_1_CCM_CNV_01_SHIFT  16

/**
 * Bit Feild of CCM_CNV_1: CCM_CNV_00
 * MTK_CAM_CCM_CNV_1_CCM_CNV_00: [0, 12]
 * matrix 0,0 coefficient
 */
#define MTK_CAM_CCM_CNV_1_CCM_CNV_00_MASK   0x00001fff
#define MTK_CAM_CCM_CNV_1_CCM_CNV_00_SHIFT  0

/**
 * Bit Feild of CCM_CNV_2: CCM_CNV_02
 * MTK_CAM_CCM_CNV_2_CCM_CNV_02: [0, 12]
 * matrix 0,2 coefficient
 */
#define MTK_CAM_CCM_CNV_2_CCM_CNV_02_MASK   0x00001fff
#define MTK_CAM_CCM_CNV_2_CCM_CNV_02_SHIFT  0

/**
 * Bit Feild of CCM_CNV_3: CCM_CNV_11
 * MTK_CAM_CCM_CNV_3_CCM_CNV_11: [16, 28]
 * matrix 1,1 coefficient
 */
#define MTK_CAM_CCM_CNV_3_CCM_CNV_11_MASK   0x1fff0000
#define MTK_CAM_CCM_CNV_3_CCM_CNV_11_SHIFT  16

/**
 * Bit Feild of CCM_CNV_3: CCM_CNV_10
 * MTK_CAM_CCM_CNV_3_CCM_CNV_10: [0, 12]
 * matrix 1,0 coefficient
 */
#define MTK_CAM_CCM_CNV_3_CCM_CNV_10_MASK   0x00001fff
#define MTK_CAM_CCM_CNV_3_CCM_CNV_10_SHIFT  0

/**
 * Bit Feild of CCM_CNV_4: CCM_CNV_12
 * MTK_CAM_CCM_CNV_4_CCM_CNV_12: [0, 12]
 * matrix 1,2 coefficient
 */
#define MTK_CAM_CCM_CNV_4_CCM_CNV_12_MASK   0x00001fff
#define MTK_CAM_CCM_CNV_4_CCM_CNV_12_SHIFT  0

/**
 * Bit Feild of CCM_CNV_5: CCM_CNV_21
 * MTK_CAM_CCM_CNV_5_CCM_CNV_21: [16, 28]
 * matrix 2,1 coefficient
 */
#define MTK_CAM_CCM_CNV_5_CCM_CNV_21_MASK   0x1fff0000
#define MTK_CAM_CCM_CNV_5_CCM_CNV_21_SHIFT  16

/**
 * Bit Feild of CCM_CNV_5: CCM_CNV_20
 * MTK_CAM_CCM_CNV_5_CCM_CNV_20: [0, 12]
 * matrix 2,0 coefficient
 */
#define MTK_CAM_CCM_CNV_5_CCM_CNV_20_MASK   0x00001fff
#define MTK_CAM_CCM_CNV_5_CCM_CNV_20_SHIFT  0

/**
 * Bit Feild of CCM_CNV_6: CCM_CNV_22
 * MTK_CAM_CCM_CNV_6_CCM_CNV_22: [0, 12]
 * matrix 2,2 coefficient
 */
#define MTK_CAM_CCM_CNV_6_CCM_CNV_22_MASK   0x00001fff
#define MTK_CAM_CCM_CNV_6_CCM_CNV_22_SHIFT  0

/**
 * Bit Feild of DM_INTP_NAT: DM_L0_OFST
 * MTK_CAM_DM_INTP_NAT_DM_L0_OFST: [12, 19]
 * luma blending LUT
 * 0 ~ 255
 */
#define MTK_CAM_DM_INTP_NAT_DM_L0_OFST_MASK   0x000ff000
#define MTK_CAM_DM_INTP_NAT_DM_L0_OFST_SHIFT  12

/**
 * Bit Feild of DM_SL_CTL: DM_SL_Y1
 * MTK_CAM_DM_SL_CTL_DM_SL_Y1: [14, 21]
 * shading link modulation LUT
 * 0 ~ 255
 */
#define MTK_CAM_DM_SL_CTL_DM_SL_Y1_MASK   0x003fc000
#define MTK_CAM_DM_SL_CTL_DM_SL_Y1_SHIFT  14

/**
 * Bit Feild of DM_SL_CTL: DM_SL_Y2
 * MTK_CAM_DM_SL_CTL_DM_SL_Y2: [6, 13]
 * shading link modulation LUT
 * 0 ~ 255
 */
#define MTK_CAM_DM_SL_CTL_DM_SL_Y2_MASK   0x00003fc0
#define MTK_CAM_DM_SL_CTL_DM_SL_Y2_SHIFT  6

/**
 * Bit Feild of DM_SL_CTL: DM_SL_EN
 * MTK_CAM_DM_SL_CTL_DM_SL_EN: [0, 0]
 * shading link enable
 * 0: disable SL
 * 1: enable SL
 */
#define MTK_CAM_DM_SL_CTL_DM_SL_EN_MASK   0x00000001
#define MTK_CAM_DM_SL_CTL_DM_SL_EN_SHIFT  0

/**
 * Bit Feild of DM_NR_STR: DM_N0_STR
 * MTK_CAM_DM_NR_STR_DM_N0_STR: [10, 14]
 * noise reduction strength
 * 0 ~ 16
 */
#define MTK_CAM_DM_NR_STR_DM_N0_STR_MASK   0x00007c00
#define MTK_CAM_DM_NR_STR_DM_N0_STR_SHIFT  10

/**
 * Bit Feild of DM_NR_ACT: DM_N0_OFST
 * MTK_CAM_DM_NR_ACT_DM_N0_OFST: [24, 31]
 * noise reduction activity LUT
 * 0 ~ 255
 */
#define MTK_CAM_DM_NR_ACT_DM_N0_OFST_MASK   0xff000000
#define MTK_CAM_DM_NR_ACT_DM_N0_OFST_SHIFT  24

/**
 * Bit Feild of DM_HF_STR: DM_HA_STR
 * MTK_CAM_DM_HF_STR_DM_HA_STR: [27, 31]
 * overall high frequency strength
 * 0 ~ 31
 */
#define MTK_CAM_DM_HF_STR_DM_HA_STR_MASK   0xf8000000
#define MTK_CAM_DM_HF_STR_DM_HA_STR_SHIFT  27

/**
 * Bit Feild of DM_HF_STR: DM_H1_GN
 * MTK_CAM_DM_HF_STR_DM_H1_GN: [22, 26]
 * individual high frequency strength
 * 0 ~ 31
 */
#define MTK_CAM_DM_HF_STR_DM_H1_GN_MASK   0x07c00000
#define MTK_CAM_DM_HF_STR_DM_H1_GN_SHIFT  22

/**
 * Bit Feild of DM_HF_STR: DM_H2_GN
 * MTK_CAM_DM_HF_STR_DM_H2_GN: [17, 21]
 * individual high frequency strength
 * 0 ~ 31
 */
#define MTK_CAM_DM_HF_STR_DM_H2_GN_MASK   0x003e0000
#define MTK_CAM_DM_HF_STR_DM_H2_GN_SHIFT  17

/**
 * Bit Feild of DM_HF_STR: DM_H3_GN
 * MTK_CAM_DM_HF_STR_DM_H3_GN: [12, 16]
 * individual high frequency strength
 * 0 ~ 31
 */
#define MTK_CAM_DM_HF_STR_DM_H3_GN_MASK   0x0001f000
#define MTK_CAM_DM_HF_STR_DM_H3_GN_SHIFT  12

/**
 * Bit Feild of DM_HF_ACT1: DM_H1_LWB
 * MTK_CAM_DM_HF_ACT1_DM_H1_LWB: [24, 31]
 * high frequency activity LUT
 * 0 ~ 255
 */
#define MTK_CAM_DM_HF_ACT1_DM_H1_LWB_MASK   0xff000000
#define MTK_CAM_DM_HF_ACT1_DM_H1_LWB_SHIFT  24

/**
 * Bit Feild of DM_HF_ACT1: DM_H1_UPB
 * MTK_CAM_DM_HF_ACT1_DM_H1_UPB: [16, 23]
 * high frequency activity LUT
 * 0 ~ 255
 */
#define MTK_CAM_DM_HF_ACT1_DM_H1_UPB_MASK   0x00ff0000
#define MTK_CAM_DM_HF_ACT1_DM_H1_UPB_SHIFT  16

/**
 * Bit Feild of DM_HF_ACT1: DM_H2_LWB
 * MTK_CAM_DM_HF_ACT1_DM_H2_LWB: [8, 15]
 * high frequency activity LUT
 * 0 ~ 255
 */
#define MTK_CAM_DM_HF_ACT1_DM_H2_LWB_MASK   0x0000ff00
#define MTK_CAM_DM_HF_ACT1_DM_H2_LWB_SHIFT  8

/**
 * Bit Feild of DM_HF_ACT1: DM_H2_UPB
 * MTK_CAM_DM_HF_ACT1_DM_H2_UPB: [0, 7]
 * high frequency activity LUT
 * 0 ~ 255
 */
#define MTK_CAM_DM_HF_ACT1_DM_H2_UPB_MASK   0x000000ff
#define MTK_CAM_DM_HF_ACT1_DM_H2_UPB_SHIFT  0

/**
 * Bit Feild of DM_HF_ACT2: DM_H3_LWB
 * MTK_CAM_DM_HF_ACT2_DM_H3_LWB: [16, 23]
 * high frequency activity LUT
 * 0 ~ 255
 */
#define MTK_CAM_DM_HF_ACT2_DM_H3_LWB_MASK   0x00ff0000
#define MTK_CAM_DM_HF_ACT2_DM_H3_LWB_SHIFT  16

/**
 * Bit Feild of DM_HF_ACT2: DM_H3_UPB
 * MTK_CAM_DM_HF_ACT2_DM_H3_UPB: [8, 15]
 * high frequency activity LUT
 * 0 ~ 255
 */
#define MTK_CAM_DM_HF_ACT2_DM_H3_UPB_MASK   0x0000ff00
#define MTK_CAM_DM_HF_ACT2_DM_H3_UPB_SHIFT  8

/**
 * Bit Feild of DM_CLIP: DM_OV_TH
 * MTK_CAM_DM_CLIP_DM_OV_TH: [16, 23]
 * over/undershoot brightness LUT
 * 0 ~ 255
 */
#define MTK_CAM_DM_CLIP_DM_OV_TH_MASK   0x00ff0000
#define MTK_CAM_DM_CLIP_DM_OV_TH_SHIFT  16

/**
 * Bit Feild of DM_CLIP: DM_UN_TH
 * MTK_CAM_DM_CLIP_DM_UN_TH: [8, 15]
 * over/undershoot brightness LUT
 * 0 ~ 255
 */
#define MTK_CAM_DM_CLIP_DM_UN_TH_MASK   0x0000ff00
#define MTK_CAM_DM_CLIP_DM_UN_TH_SHIFT  8

/**
 * Bit Feild of DM_CLIP: DM_CLIP_TH
 * MTK_CAM_DM_CLIP_DM_CLIP_TH: [0, 7]
 * over/undershoot activity LUT
 * 0 ~ 255
 */
#define MTK_CAM_DM_CLIP_DM_CLIP_TH_MASK   0x000000ff
#define MTK_CAM_DM_CLIP_DM_CLIP_TH_SHIFT  0

/**
 * Bit Feild of DM_EE: DM_HNEG_GN
 * MTK_CAM_DM_EE_DM_HNEG_GN: [5, 9]
 * edge enhancement negative gain
 * 0~16
 */
#define MTK_CAM_DM_EE_DM_HNEG_GN_MASK   0x000003e0
#define MTK_CAM_DM_EE_DM_HNEG_GN_SHIFT  5

/**
 * Bit Feild of DM_EE: DM_HPOS_GN
 * MTK_CAM_DM_EE_DM_HPOS_GN: [0, 4]
 * edge enhancement positive gain
 * 0~16
 */
#define MTK_CAM_DM_EE_DM_HPOS_GN_MASK   0x0000001f
#define MTK_CAM_DM_EE_DM_HPOS_GN_SHIFT  0

/**
 * Bit Feild of G2C_CONV_0A: G2C_CNV_01
 * MTK_CAM_G2C_CONV_0A_G2C_CNV_01: [16, 26]
 * DIP RGB 2 YUV Matrix 0,1 coefficient in Q1.1.9
 */
#define MTK_CAM_G2C_CONV_0A_G2C_CNV_01_MASK   0x07ff0000
#define MTK_CAM_G2C_CONV_0A_G2C_CNV_01_SHIFT  16

/**
 * Bit Feild of G2C_CONV_0A: G2C_CNV_00
 * MTK_CAM_G2C_CONV_0A_G2C_CNV_00: [0, 10]
 * DIP RGB 2 YUV Matrix 0,0 coefficient in Q1.1.9
 */
#define MTK_CAM_G2C_CONV_0A_G2C_CNV_00_MASK   0x000007ff
#define MTK_CAM_G2C_CONV_0A_G2C_CNV_00_SHIFT  0

/**
 * Bit Feild of G2C_CONV_0B: G2C_Y_OFST
 * MTK_CAM_G2C_CONV_0B_G2C_Y_OFST: [16, 30]
 * Y offset. Q1.10.0 (mobile) or Q1.14.0 (non-mobile)
 */
#define MTK_CAM_G2C_CONV_0B_G2C_Y_OFST_MASK   0x7fff0000
#define MTK_CAM_G2C_CONV_0B_G2C_Y_OFST_SHIFT  16

/**
 * Bit Feild of G2C_CONV_0B: G2C_CNV_02
 * MTK_CAM_G2C_CONV_0B_G2C_CNV_02: [0, 10]
 * DIP RGB 2 YUV Matrix 0,2 coefficient in Q1.1.9
 */
#define MTK_CAM_G2C_CONV_0B_G2C_CNV_02_MASK   0x000007ff
#define MTK_CAM_G2C_CONV_0B_G2C_CNV_02_SHIFT  0

/**
 * Bit Feild of G2C_CONV_1A: G2C_CNV_11
 * MTK_CAM_G2C_CONV_1A_G2C_CNV_11: [16, 26]
 * DIP RGB 2 YUV Matrix 1,1 coefficient in Q1.1.9
 */
#define MTK_CAM_G2C_CONV_1A_G2C_CNV_11_MASK   0x07ff0000
#define MTK_CAM_G2C_CONV_1A_G2C_CNV_11_SHIFT  16

/**
 * Bit Feild of G2C_CONV_1A: G2C_CNV_10
 * MTK_CAM_G2C_CONV_1A_G2C_CNV_10: [0, 10]
 * DIP RGB 2 YUV Matrix 1,0 coefficient in Q1.1.9
 */
#define MTK_CAM_G2C_CONV_1A_G2C_CNV_10_MASK   0x000007ff
#define MTK_CAM_G2C_CONV_1A_G2C_CNV_10_SHIFT  0

/**
 * Bit Feild of G2C_CONV_1B: G2C_U_OFST
 * MTK_CAM_G2C_CONV_1B_G2C_U_OFST: [16, 29]
 * U offset. Q1.9.0 (mobile) or Q1.13.0 (non-mobile)
 */
#define MTK_CAM_G2C_CONV_1B_G2C_U_OFST_MASK   0x3fff0000
#define MTK_CAM_G2C_CONV_1B_G2C_U_OFST_SHIFT  16

/**
 * Bit Feild of G2C_CONV_1B: G2C_CNV_12
 * MTK_CAM_G2C_CONV_1B_G2C_CNV_12: [0, 10]
 * DIP RGB 2 YUV Matrix 1,2 coefficient in Q1.1.9
 */
#define MTK_CAM_G2C_CONV_1B_G2C_CNV_12_MASK   0x000007ff
#define MTK_CAM_G2C_CONV_1B_G2C_CNV_12_SHIFT  0

/**
 * Bit Feild of G2C_CONV_2A: G2C_CNV_21
 * MTK_CAM_G2C_CONV_2A_G2C_CNV_21: [16, 26]
 * DIP RGB 2 YUV Matrix 2,1 coefficient in Q1.1.9
 */
#define MTK_CAM_G2C_CONV_2A_G2C_CNV_21_MASK   0x07ff0000
#define MTK_CAM_G2C_CONV_2A_G2C_CNV_21_SHIFT  16

/**
 * Bit Feild of G2C_CONV_2A: G2C_CNV_20
 * MTK_CAM_G2C_CONV_2A_G2C_CNV_20: [0, 10]
 * DIP RGB 2 YUV Matrix 2,0 coefficient in Q1.1.9
 */
#define MTK_CAM_G2C_CONV_2A_G2C_CNV_20_MASK   0x000007ff
#define MTK_CAM_G2C_CONV_2A_G2C_CNV_20_SHIFT  0

/**
 * Bit Feild of G2C_CONV_2B: G2C_V_OFST
 * MTK_CAM_G2C_CONV_2B_G2C_V_OFST: [16, 29]
 * V offset. Q1.9.0 (mobile) or Q1.13.0 (non-mobile)
 */
#define MTK_CAM_G2C_CONV_2B_G2C_V_OFST_MASK   0x3fff0000
#define MTK_CAM_G2C_CONV_2B_G2C_V_OFST_SHIFT  16

/**
 * Bit Feild of G2C_CONV_2B: G2C_CNV_22
 * MTK_CAM_G2C_CONV_2B_G2C_CNV_22: [0, 10]
 * DIP RGB 2 YUV Matrix 2,2 coefficient in Q1.1.9
 */
#define MTK_CAM_G2C_CONV_2B_G2C_CNV_22_MASK   0x000007ff
#define MTK_CAM_G2C_CONV_2B_G2C_CNV_22_SHIFT  0

/**
 * Bit Feild of GGM_LUT: GGM_LUT
 * MTK_CAM_GGM_LUT_GGM_LUT: [0, 9]
 * Gamma table entry
 * Do NOT read/write this control register when GGM is enabled (ISP pipeline processing is on-going) or output data of GGM will be gated
 */
#define MTK_CAM_GGM_LUT_GGM_LUT_MASK   0x000003ff
#define MTK_CAM_GGM_LUT_GGM_LUT_SHIFT  0

/**
 * Bit Feild of GGM_CTRL: GGM_LNR
 * MTK_CAM_GGM_CTRL_GGM_LNR: [0, 0]
 * Enable linear output
 */
#define MTK_CAM_GGM_CTRL_GGM_LNR_MASK   0x00000001
#define MTK_CAM_GGM_CTRL_GGM_LNR_SHIFT  0

/**
 * Bit Feild of GGM_CTRL: GGM_END_VAR
 * MTK_CAM_GGM_CTRL_GGM_END_VAR: [1, 10]
 * end point value
 */
#define MTK_CAM_GGM_CTRL_GGM_END_VAR_MASK   0x000007fe
#define MTK_CAM_GGM_CTRL_GGM_END_VAR_SHIFT  1

/**
 * Bit Feild of GGM_CTRL: GGM_RMP_VAR
 * MTK_CAM_GGM_CTRL_GGM_RMP_VAR: [16, 20]
 * 5-bit: can support mapping to 14-bit output, (RMP_VAR+out limiter)/1024
 */
#define MTK_CAM_GGM_CTRL_GGM_RMP_VAR_MASK   0x001f0000
#define MTK_CAM_GGM_CTRL_GGM_RMP_VAR_SHIFT  16

/**
 * Bit Feild of LSC_RATIO: LSC_RA00
 * MTK_CAM_LSC_RATIO_LSC_RA00: [0, 5]
 * Shading ratio
 */
#define MTK_CAM_LSC_RATIO_LSC_RA00_MASK   0x0000003f
#define MTK_CAM_LSC_RATIO_LSC_RA00_SHIFT  0

/**
 * Bit Feild of LTMS_MAX_DIV: LTMS_CLIP_TH_ALPHA_BASE
 * MTK_CAM_LTMS_MAX_DIV_LTMS_CLIP_TH_ALPHA_BASE: [0, 9]
 * Divider for Maxvalue
 */
#define MTK_CAM_LTMS_MAX_DIV_LTMS_CLIP_TH_ALPHA_BASE_MASK   0x000003ff
#define MTK_CAM_LTMS_MAX_DIV_LTMS_CLIP_TH_ALPHA_BASE_SHIFT  0

/**
 * Bit Feild of LTMS_MAX_DIV: LTMS_CLIP_TH_ALPHA_BASE_SHIFT_BIT
 * MTK_CAM_LTMS_MAX_DIV_LTMS_CLIP_TH_ALPHA_BASE_SHIFT_BIT: [16, 20]
 * Divider for Maxvalue
 */
#define MTK_CAM_LTMS_MAX_DIV_LTMS_CLIP_TH_ALPHA_BASE_SHIFT_BIT_MASK   0x001f0000
#define MTK_CAM_LTMS_MAX_DIV_LTMS_CLIP_TH_ALPHA_BASE_SHIFT_BIT_SHIFT  16

/**
 * Bit Feild of LTMS_BLKHIST_LB: LTMS_BLKHIST_LB
 * MTK_CAM_LTMS_BLKHIST_LB_LTMS_BLKHIST_LB: [0, 17]
 * block histogram low bound,
 * BLKHIST_UB>=BLKHIST_MB>=BLKHIST_LB
 */
#define MTK_CAM_LTMS_BLKHIST_LB_LTMS_BLKHIST_LB_MASK   0x0003ffff
#define MTK_CAM_LTMS_BLKHIST_LB_LTMS_BLKHIST_LB_SHIFT  0

/**
 * Bit Feild of LTMS_BLKHIST_MB: LTMS_BLKHIST_MB
 * MTK_CAM_LTMS_BLKHIST_MB_LTMS_BLKHIST_MB: [0, 17]
 * block histogram middle bound,
 * BLKHIST_UB>=BLKHIST_MB>=BLKHIST_LB
 */
#define MTK_CAM_LTMS_BLKHIST_MB_LTMS_BLKHIST_MB_MASK   0x0003ffff
#define MTK_CAM_LTMS_BLKHIST_MB_LTMS_BLKHIST_MB_SHIFT  0

/**
 * Bit Feild of LTMS_BLKHIST_UB: LTMS_BLKHIST_UB
 * MTK_CAM_LTMS_BLKHIST_UB_LTMS_BLKHIST_UB: [0, 17]
 * block histogram up bound,
 * BLKHIST_UB>=BLKHIST_MB>=BLKHIST_LB
 */
#define MTK_CAM_LTMS_BLKHIST_UB_LTMS_BLKHIST_UB_MASK   0x0003ffff
#define MTK_CAM_LTMS_BLKHIST_UB_LTMS_BLKHIST_UB_SHIFT  0

/**
 * Bit Feild of LTMS_BLKHIST_INT: LTMS_BLKHIST_INT1
 * MTK_CAM_LTMS_BLKHIST_INT_LTMS_BLKHIST_INT1: [0, 13]
 * block histogram interval 1
 */
#define MTK_CAM_LTMS_BLKHIST_INT_LTMS_BLKHIST_INT1_MASK   0x00003fff
#define MTK_CAM_LTMS_BLKHIST_INT_LTMS_BLKHIST_INT1_SHIFT  0

/**
 * Bit Feild of LTMS_BLKHIST_INT: LTMS_BLKHIST_INT2
 * MTK_CAM_LTMS_BLKHIST_INT_LTMS_BLKHIST_INT2: [16, 29]
 * block histogram interval 2
 */
#define MTK_CAM_LTMS_BLKHIST_INT_LTMS_BLKHIST_INT2_MASK   0x3fff0000
#define MTK_CAM_LTMS_BLKHIST_INT_LTMS_BLKHIST_INT2_SHIFT  16

/**
 * Bit Feild of LTMS_CLIP_TH_CAL: LTMS_CLP_HLTHD
 * MTK_CAM_LTMS_CLIP_TH_CAL_LTMS_CLP_HLTHD: [0, 10]
 * control percentage of histogram to calculate clip_th, 10-bits precision.
 */
#define MTK_CAM_LTMS_CLIP_TH_CAL_LTMS_CLP_HLTHD_MASK   0x000007ff
#define MTK_CAM_LTMS_CLIP_TH_CAL_LTMS_CLP_HLTHD_SHIFT  0

/**
 * Bit Feild of LTMS_CLIP_TH_CAL: LTMS_CLP_STARTBIN
 * MTK_CAM_LTMS_CLIP_TH_CAL_LTMS_CLP_STARTBIN: [16, 23]
 * start bin of histogram.
 */
#define MTK_CAM_LTMS_CLIP_TH_CAL_LTMS_CLP_STARTBIN_MASK   0x00ff0000
#define MTK_CAM_LTMS_CLIP_TH_CAL_LTMS_CLP_STARTBIN_SHIFT  16

/**
 * Bit Feild of LTMS_CLIP_TH_LB: LTMS_CLP_LB
 * MTK_CAM_LTMS_CLIP_TH_LB_LTMS_CLP_LB: [0, 21]
 * Low bound of clip threshold output.
 */
#define MTK_CAM_LTMS_CLIP_TH_LB_LTMS_CLP_LB_MASK   0x003fffff
#define MTK_CAM_LTMS_CLIP_TH_LB_LTMS_CLP_LB_SHIFT  0

/**
 * Bit Feild of LTMS_CLIP_TH_HB: LTMS_CLP_HB
 * MTK_CAM_LTMS_CLIP_TH_HB_LTMS_CLP_HB: [0, 21]
 * High bound of clip threshold output.
 */
#define MTK_CAM_LTMS_CLIP_TH_HB_LTMS_CLP_HB_MASK   0x003fffff
#define MTK_CAM_LTMS_CLIP_TH_HB_LTMS_CLP_HB_SHIFT  0

/**
 * Bit Feild of LTMS_GLBHIST_INT: LTMS_GLBHIST_INT
 * MTK_CAM_LTMS_GLBHIST_INT_LTMS_GLBHIST_INT: [0, 14]
 * Interval of global histogram
 */
#define MTK_CAM_LTMS_GLBHIST_INT_LTMS_GLBHIST_INT_MASK   0x00007fff
#define MTK_CAM_LTMS_GLBHIST_INT_LTMS_GLBHIST_INT_SHIFT  0

/**
 * Bit Feild of LTMTC_CURVE: LTMTC_TONECURVE_LUT_L
 * MTK_CAM_LTMTC_CURVE_LTMTC_TONECURVE_LUT_L: [0, 13]
 * SRAM_PING_PONG
 * [u8.6-bits]x12x9
 */
#define MTK_CAM_LTMTC_CURVE_LTMTC_TONECURVE_LUT_L_MASK   0x00003fff
#define MTK_CAM_LTMTC_CURVE_LTMTC_TONECURVE_LUT_L_SHIFT  0

/**
 * Bit Feild of LTMTC_CURVE: LTMTC_TONECURVE_LUT_H
 * MTK_CAM_LTMTC_CURVE_LTMTC_TONECURVE_LUT_H: [16, 29]
 * SRAM_PING_PONG
 * [u8.6-bits]x12x9
 */
#define MTK_CAM_LTMTC_CURVE_LTMTC_TONECURVE_LUT_H_MASK   0x3fff0000
#define MTK_CAM_LTMTC_CURVE_LTMTC_TONECURVE_LUT_H_SHIFT  16

/**
 * Bit Feild of LTMTC_CLP: LTMTC_TONECURVE_CLP
 * MTK_CAM_LTMTC_CLP_LTMTC_TONECURVE_CLP: [0, 23]
 * LTM block CT
 */
#define MTK_CAM_LTMTC_CLP_LTMTC_TONECURVE_CLP_MASK   0x00ffffff
#define MTK_CAM_LTMTC_CLP_LTMTC_TONECURVE_CLP_SHIFT  0

/**
 * Bit Feild of LTM_CTRL: LTM_GAMMA_EN
 * MTK_CAM_LTM_CTRL_LTM_GAMMA_EN: [4, 4]
 * Enable gamma domain
 */
#define MTK_CAM_LTM_CTRL_LTM_GAMMA_EN_MASK   0x00000010
#define MTK_CAM_LTM_CTRL_LTM_GAMMA_EN_SHIFT  4

/**
 * Bit Feild of LTM_CTRL: LTM_CURVE_CP_MODE
 * MTK_CAM_LTM_CTRL_LTM_CURVE_CP_MODE: [5, 5]
 * Mode of curve control point, [0]: 33 fixed cp, [1]: 16 XY cp
 */
#define MTK_CAM_LTM_CTRL_LTM_CURVE_CP_MODE_MASK   0x00000020
#define MTK_CAM_LTM_CTRL_LTM_CURVE_CP_MODE_SHIFT  5

/**
 * Bit Feild of LTM_BLK_NUM: LTM_BLK_X_NUM
 * MTK_CAM_LTM_BLK_NUM_LTM_BLK_X_NUM: [0, 4]
 * block X number supports 2~12
 */
#define MTK_CAM_LTM_BLK_NUM_LTM_BLK_X_NUM_MASK   0x0000001f
#define MTK_CAM_LTM_BLK_NUM_LTM_BLK_X_NUM_SHIFT  0

/**
 * Bit Feild of LTM_BLK_NUM: LTM_BLK_Y_NUM
 * MTK_CAM_LTM_BLK_NUM_LTM_BLK_Y_NUM: [8, 12]
 * block Y number supports 2~9
 */
#define MTK_CAM_LTM_BLK_NUM_LTM_BLK_Y_NUM_MASK   0x00001f00
#define MTK_CAM_LTM_BLK_NUM_LTM_BLK_Y_NUM_SHIFT  8

/**
 * Bit Feild of LTM_MAX_DIV: LTM_CLIP_TH_ALPHA_BASE
 * MTK_CAM_LTM_MAX_DIV_LTM_CLIP_TH_ALPHA_BASE: [0, 9]
 * Divider for Maxvalue
 */
#define MTK_CAM_LTM_MAX_DIV_LTM_CLIP_TH_ALPHA_BASE_MASK   0x000003ff
#define MTK_CAM_LTM_MAX_DIV_LTM_CLIP_TH_ALPHA_BASE_SHIFT  0

/**
 * Bit Feild of LTM_MAX_DIV: LTM_CLIP_TH_ALPHA_BASE_SHIFT_BIT
 * MTK_CAM_LTM_MAX_DIV_LTM_CLIP_TH_ALPHA_BASE_SHIFT_BIT: [16, 20]
 * Divider for Maxvalue
 */
#define MTK_CAM_LTM_MAX_DIV_LTM_CLIP_TH_ALPHA_BASE_SHIFT_BIT_MASK   0x001f0000
#define MTK_CAM_LTM_MAX_DIV_LTM_CLIP_TH_ALPHA_BASE_SHIFT_BIT_SHIFT  16

/**
 * Bit Feild of LTM_CLIP: LTM_GAIN_TH
 * MTK_CAM_LTM_CLIP_LTM_GAIN_TH: [0, 5]
 * Threshold to clip output gain
 */
#define MTK_CAM_LTM_CLIP_LTM_GAIN_TH_MASK   0x0000003f
#define MTK_CAM_LTM_CLIP_LTM_GAIN_TH_SHIFT  0

/**
 * Bit Feild of LTM_CFG: LTM_ENGINE_EN
 * MTK_CAM_LTM_CFG_LTM_ENGINE_EN: [0, 0]
 * None
 */
#define MTK_CAM_LTM_CFG_LTM_ENGINE_EN_MASK   0x00000001
#define MTK_CAM_LTM_CFG_LTM_ENGINE_EN_SHIFT  0

/**
 * Bit Feild of LTM_CFG: LTM_CG_DISABLE
 * MTK_CAM_LTM_CFG_LTM_CG_DISABLE: [4, 4]
 * None
 */
#define MTK_CAM_LTM_CFG_LTM_CG_DISABLE_MASK   0x00000010
#define MTK_CAM_LTM_CFG_LTM_CG_DISABLE_SHIFT  4

/**
 * Bit Feild of LTM_CFG: LTM_CHKSUM_EN
 * MTK_CAM_LTM_CFG_LTM_CHKSUM_EN: [28, 28]
 * None
 */
#define MTK_CAM_LTM_CFG_LTM_CHKSUM_EN_MASK   0x10000000
#define MTK_CAM_LTM_CFG_LTM_CHKSUM_EN_SHIFT  28

/**
 * Bit Feild of LTM_CFG: LTM_CHKSUM_SEL
 * MTK_CAM_LTM_CFG_LTM_CHKSUM_SEL: [29, 30]
 * None
 */
#define MTK_CAM_LTM_CFG_LTM_CHKSUM_SEL_MASK   0x60000000
#define MTK_CAM_LTM_CFG_LTM_CHKSUM_SEL_SHIFT  29

/**
 * Bit Feild of LTM_CLIP_TH: LTM_CLIP_TH
 * MTK_CAM_LTM_CLIP_TH_LTM_CLIP_TH: [0, 23]
 * clipping threshold, enabled if #define LTM_USE_PREVIOUS_MAXVALUE=1
 */
#define MTK_CAM_LTM_CLIP_TH_LTM_CLIP_TH_MASK   0x00ffffff
#define MTK_CAM_LTM_CLIP_TH_LTM_CLIP_TH_SHIFT  0

/**
 * Bit Feild of LTM_CLIP_TH: LTM_WGT_BSH
 * MTK_CAM_LTM_CLIP_TH_LTM_WGT_BSH: [24, 27]
 * rightward bit shift for weighting data
 */
#define MTK_CAM_LTM_CLIP_TH_LTM_WGT_BSH_MASK   0x0f000000
#define MTK_CAM_LTM_CLIP_TH_LTM_WGT_BSH_SHIFT  24

/**
 * Bit Feild of LTM_GAIN_MAP: LTM_MAP_LOG_EN
 * MTK_CAM_LTM_GAIN_MAP_LTM_MAP_LOG_EN: [0, 0]
 * switch for map log
 */
#define MTK_CAM_LTM_GAIN_MAP_LTM_MAP_LOG_EN_MASK   0x00000001
#define MTK_CAM_LTM_GAIN_MAP_LTM_MAP_LOG_EN_SHIFT  0

/**
 * Bit Feild of LTM_GAIN_MAP: LTM_WGT_LOG_EN
 * MTK_CAM_LTM_GAIN_MAP_LTM_WGT_LOG_EN: [1, 1]
 * switch for weight log
 */
#define MTK_CAM_LTM_GAIN_MAP_LTM_WGT_LOG_EN_MASK   0x00000002
#define MTK_CAM_LTM_GAIN_MAP_LTM_WGT_LOG_EN_SHIFT  1

/**
 * Bit Feild of LTM_GAIN_MAP: LTM_NONTRAN_MAP_TYPE
 * MTK_CAM_LTM_GAIN_MAP_LTM_NONTRAN_MAP_TYPE: [4, 7]
 * type of nontran map
 */
#define MTK_CAM_LTM_GAIN_MAP_LTM_NONTRAN_MAP_TYPE_MASK   0x000000f0
#define MTK_CAM_LTM_GAIN_MAP_LTM_NONTRAN_MAP_TYPE_SHIFT  4

/**
 * Bit Feild of LTM_GAIN_MAP: LTM_TRAN_MAP_TYPE
 * MTK_CAM_LTM_GAIN_MAP_LTM_TRAN_MAP_TYPE: [8, 11]
 * type of tran map
 */
#define MTK_CAM_LTM_GAIN_MAP_LTM_TRAN_MAP_TYPE_MASK   0x00000f00
#define MTK_CAM_LTM_GAIN_MAP_LTM_TRAN_MAP_TYPE_SHIFT  8

/**
 * Bit Feild of LTM_GAIN_MAP: LTM_TRAN_WGT_TYPE
 * MTK_CAM_LTM_GAIN_MAP_LTM_TRAN_WGT_TYPE: [12, 13]
 * type of tran weight
 */
#define MTK_CAM_LTM_GAIN_MAP_LTM_TRAN_WGT_TYPE_MASK   0x00003000
#define MTK_CAM_LTM_GAIN_MAP_LTM_TRAN_WGT_TYPE_SHIFT  12

/**
 * Bit Feild of LTM_GAIN_MAP: LTM_TRAN_WGT
 * MTK_CAM_LTM_GAIN_MAP_LTM_TRAN_WGT: [16, 20]
 * static tran weight
 */
#define MTK_CAM_LTM_GAIN_MAP_LTM_TRAN_WGT_MASK   0x001f0000
#define MTK_CAM_LTM_GAIN_MAP_LTM_TRAN_WGT_SHIFT  16

/**
 * Bit Feild of LTM_GAIN_MAP: LTM_RANGE_SCL
 * MTK_CAM_LTM_GAIN_MAP_LTM_RANGE_SCL: [24, 29]
 * scale of maxTran
 */
#define MTK_CAM_LTM_GAIN_MAP_LTM_RANGE_SCL_MASK   0x3f000000
#define MTK_CAM_LTM_GAIN_MAP_LTM_RANGE_SCL_SHIFT  24

/**
 * Bit Feild of LTM_CVNODE_GRP0: LTM_CVNODE_0
 * MTK_CAM_LTM_CVNODE_GRP0_LTM_CVNODE_0: [0, 11]
 * cvnode-0 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP0_LTM_CVNODE_0_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP0_LTM_CVNODE_0_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP0: LTM_CVNODE_1
 * MTK_CAM_LTM_CVNODE_GRP0_LTM_CVNODE_1: [16, 27]
 * cvnode-1 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP0_LTM_CVNODE_1_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP0_LTM_CVNODE_1_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP1: LTM_CVNODE_2
 * MTK_CAM_LTM_CVNODE_GRP1_LTM_CVNODE_2: [0, 11]
 * cvnode-2 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP1_LTM_CVNODE_2_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP1_LTM_CVNODE_2_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP1: LTM_CVNODE_3
 * MTK_CAM_LTM_CVNODE_GRP1_LTM_CVNODE_3: [16, 27]
 * cvnode-3 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP1_LTM_CVNODE_3_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP1_LTM_CVNODE_3_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP2: LTM_CVNODE_4
 * MTK_CAM_LTM_CVNODE_GRP2_LTM_CVNODE_4: [0, 11]
 * cvnode-4 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP2_LTM_CVNODE_4_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP2_LTM_CVNODE_4_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP2: LTM_CVNODE_5
 * MTK_CAM_LTM_CVNODE_GRP2_LTM_CVNODE_5: [16, 27]
 * cvnode-5 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP2_LTM_CVNODE_5_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP2_LTM_CVNODE_5_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP3: LTM_CVNODE_6
 * MTK_CAM_LTM_CVNODE_GRP3_LTM_CVNODE_6: [0, 11]
 * cvnode-6 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP3_LTM_CVNODE_6_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP3_LTM_CVNODE_6_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP3: LTM_CVNODE_7
 * MTK_CAM_LTM_CVNODE_GRP3_LTM_CVNODE_7: [16, 27]
 * cvnode-7 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP3_LTM_CVNODE_7_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP3_LTM_CVNODE_7_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP4: LTM_CVNODE_8
 * MTK_CAM_LTM_CVNODE_GRP4_LTM_CVNODE_8: [0, 11]
 * cvnode-8 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP4_LTM_CVNODE_8_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP4_LTM_CVNODE_8_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP4: LTM_CVNODE_9
 * MTK_CAM_LTM_CVNODE_GRP4_LTM_CVNODE_9: [16, 27]
 * cvnode-9 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP4_LTM_CVNODE_9_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP4_LTM_CVNODE_9_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP5: LTM_CVNODE_10
 * MTK_CAM_LTM_CVNODE_GRP5_LTM_CVNODE_10: [0, 11]
 * cvnode-10 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP5_LTM_CVNODE_10_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP5_LTM_CVNODE_10_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP5: LTM_CVNODE_11
 * MTK_CAM_LTM_CVNODE_GRP5_LTM_CVNODE_11: [16, 27]
 * cvnode-11 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP5_LTM_CVNODE_11_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP5_LTM_CVNODE_11_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP6: LTM_CVNODE_12
 * MTK_CAM_LTM_CVNODE_GRP6_LTM_CVNODE_12: [0, 11]
 * cvnode-12 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP6_LTM_CVNODE_12_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP6_LTM_CVNODE_12_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP6: LTM_CVNODE_13
 * MTK_CAM_LTM_CVNODE_GRP6_LTM_CVNODE_13: [16, 27]
 * cvnode-13 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP6_LTM_CVNODE_13_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP6_LTM_CVNODE_13_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP7: LTM_CVNODE_14
 * MTK_CAM_LTM_CVNODE_GRP7_LTM_CVNODE_14: [0, 11]
 * cvnode-14 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP7_LTM_CVNODE_14_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP7_LTM_CVNODE_14_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP7: LTM_CVNODE_15
 * MTK_CAM_LTM_CVNODE_GRP7_LTM_CVNODE_15: [16, 27]
 * cvnode-15 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP7_LTM_CVNODE_15_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP7_LTM_CVNODE_15_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP8: LTM_CVNODE_16
 * MTK_CAM_LTM_CVNODE_GRP8_LTM_CVNODE_16: [0, 11]
 * cvnode-16 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP8_LTM_CVNODE_16_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP8_LTM_CVNODE_16_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP8: LTM_CVNODE_17
 * MTK_CAM_LTM_CVNODE_GRP8_LTM_CVNODE_17: [16, 27]
 * cvnode-17 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP8_LTM_CVNODE_17_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP8_LTM_CVNODE_17_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP9: LTM_CVNODE_18
 * MTK_CAM_LTM_CVNODE_GRP9_LTM_CVNODE_18: [0, 11]
 * cvnode-18 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP9_LTM_CVNODE_18_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP9_LTM_CVNODE_18_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP9: LTM_CVNODE_19
 * MTK_CAM_LTM_CVNODE_GRP9_LTM_CVNODE_19: [16, 27]
 * cvnode-19 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP9_LTM_CVNODE_19_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP9_LTM_CVNODE_19_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP10: LTM_CVNODE_20
 * MTK_CAM_LTM_CVNODE_GRP10_LTM_CVNODE_20: [0, 11]
 * cvnode-20 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP10_LTM_CVNODE_20_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP10_LTM_CVNODE_20_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP10: LTM_CVNODE_21
 * MTK_CAM_LTM_CVNODE_GRP10_LTM_CVNODE_21: [16, 27]
 * cvnode-21 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP10_LTM_CVNODE_21_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP10_LTM_CVNODE_21_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP11: LTM_CVNODE_22
 * MTK_CAM_LTM_CVNODE_GRP11_LTM_CVNODE_22: [0, 11]
 * cvnode-22 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP11_LTM_CVNODE_22_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP11_LTM_CVNODE_22_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP11: LTM_CVNODE_23
 * MTK_CAM_LTM_CVNODE_GRP11_LTM_CVNODE_23: [16, 27]
 * cvnode-23 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP11_LTM_CVNODE_23_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP11_LTM_CVNODE_23_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP12: LTM_CVNODE_24
 * MTK_CAM_LTM_CVNODE_GRP12_LTM_CVNODE_24: [0, 11]
 * cvnode-24 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP12_LTM_CVNODE_24_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP12_LTM_CVNODE_24_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP12: LTM_CVNODE_25
 * MTK_CAM_LTM_CVNODE_GRP12_LTM_CVNODE_25: [16, 27]
 * cvnode-25 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP12_LTM_CVNODE_25_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP12_LTM_CVNODE_25_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP13: LTM_CVNODE_26
 * MTK_CAM_LTM_CVNODE_GRP13_LTM_CVNODE_26: [0, 11]
 * cvnode-26 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP13_LTM_CVNODE_26_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP13_LTM_CVNODE_26_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP13: LTM_CVNODE_27
 * MTK_CAM_LTM_CVNODE_GRP13_LTM_CVNODE_27: [16, 27]
 * cvnode-27 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP13_LTM_CVNODE_27_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP13_LTM_CVNODE_27_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP14: LTM_CVNODE_28
 * MTK_CAM_LTM_CVNODE_GRP14_LTM_CVNODE_28: [0, 11]
 * cvnode-28 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP14_LTM_CVNODE_28_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP14_LTM_CVNODE_28_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP14: LTM_CVNODE_29
 * MTK_CAM_LTM_CVNODE_GRP14_LTM_CVNODE_29: [16, 27]
 * cvnode-29 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP14_LTM_CVNODE_29_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP14_LTM_CVNODE_29_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP15: LTM_CVNODE_30
 * MTK_CAM_LTM_CVNODE_GRP15_LTM_CVNODE_30: [0, 11]
 * cvnode-30 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP15_LTM_CVNODE_30_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP15_LTM_CVNODE_30_SHIFT  0

/**
 * Bit Feild of LTM_CVNODE_GRP15: LTM_CVNODE_31
 * MTK_CAM_LTM_CVNODE_GRP15_LTM_CVNODE_31: [16, 27]
 * cvnode-31 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP15_LTM_CVNODE_31_MASK   0x0fff0000
#define MTK_CAM_LTM_CVNODE_GRP15_LTM_CVNODE_31_SHIFT  16

/**
 * Bit Feild of LTM_CVNODE_GRP16: LTM_CVNODE_32
 * MTK_CAM_LTM_CVNODE_GRP16_LTM_CVNODE_32: [0, 11]
 * cvnode-32 for gain map
 */
#define MTK_CAM_LTM_CVNODE_GRP16_LTM_CVNODE_32_MASK   0x00000fff
#define MTK_CAM_LTM_CVNODE_GRP16_LTM_CVNODE_32_SHIFT  0

/**
 * Bit Feild of LTM_OUT_STR: LTM_OUT_STR
 * MTK_CAM_LTM_OUT_STR_LTM_OUT_STR: [0, 4]
 * output strength
 */
#define MTK_CAM_LTM_OUT_STR_LTM_OUT_STR_MASK   0x0000001f
#define MTK_CAM_LTM_OUT_STR_LTM_OUT_STR_SHIFT  0

/**
 * Bit Feild of LTM_ACT_WIN_X: LTM_ACT_WIN_X_START
 * MTK_CAM_LTM_ACT_WIN_X_LTM_ACT_WIN_X_START: [0, 15]
 * Horizontal setting for active window of AE debug
 */
#define MTK_CAM_LTM_ACT_WIN_X_LTM_ACT_WIN_X_START_MASK   0x0000ffff
#define MTK_CAM_LTM_ACT_WIN_X_LTM_ACT_WIN_X_START_SHIFT  0

/**
 * Bit Feild of LTM_ACT_WIN_X: LTM_ACT_WIN_X_END
 * MTK_CAM_LTM_ACT_WIN_X_LTM_ACT_WIN_X_END: [16, 31]
 * Horizontal setting for active window of AE debug
 */
#define MTK_CAM_LTM_ACT_WIN_X_LTM_ACT_WIN_X_END_MASK   0xffff0000
#define MTK_CAM_LTM_ACT_WIN_X_LTM_ACT_WIN_X_END_SHIFT  16

/**
 * Bit Feild of LTM_ACT_WIN_Y: LTM_ACT_WIN_Y_START
 * MTK_CAM_LTM_ACT_WIN_Y_LTM_ACT_WIN_Y_START: [0, 15]
 * Vertical setting for active window of AE debug
 */
#define MTK_CAM_LTM_ACT_WIN_Y_LTM_ACT_WIN_Y_START_MASK   0x0000ffff
#define MTK_CAM_LTM_ACT_WIN_Y_LTM_ACT_WIN_Y_START_SHIFT  0

/**
 * Bit Feild of LTM_ACT_WIN_Y: LTM_ACT_WIN_Y_END
 * MTK_CAM_LTM_ACT_WIN_Y_LTM_ACT_WIN_Y_END: [16, 31]
 * Vertical setting for active window of AE debug
 */
#define MTK_CAM_LTM_ACT_WIN_Y_LTM_ACT_WIN_Y_END_MASK   0xffff0000
#define MTK_CAM_LTM_ACT_WIN_Y_LTM_ACT_WIN_Y_END_SHIFT  16

/**
 * Bit Feild of OBC_DBS: OBC_DBS_RATIO
 * MTK_CAM_OBC_DBS_OBC_DBS_RATIO: [0, 4]
 * Ratio of "bias" being eliminated
 */
#define MTK_CAM_OBC_DBS_OBC_DBS_RATIO_MASK   0x0000001f
#define MTK_CAM_OBC_DBS_OBC_DBS_RATIO_SHIFT  0

/**
 * Bit Feild of OBC_DBS: OBC_POSTTUNE_EN
 * MTK_CAM_OBC_DBS_OBC_POSTTUNE_EN: [8, 8]
 * Enable gray-blending and LUT-subtraction processing
 */
#define MTK_CAM_OBC_DBS_OBC_POSTTUNE_EN_MASK   0x00000100
#define MTK_CAM_OBC_DBS_OBC_POSTTUNE_EN_SHIFT  8

/**
 * Bit Feild of OBC_GRAY_BLD_0: OBC_LUMA_MODE
 * MTK_CAM_OBC_GRAY_BLD_0_OBC_LUMA_MODE: [0, 0]
 * Selection between max mode or mean mode for luma computation
 */
#define MTK_CAM_OBC_GRAY_BLD_0_OBC_LUMA_MODE_MASK   0x00000001
#define MTK_CAM_OBC_GRAY_BLD_0_OBC_LUMA_MODE_SHIFT  0

/**
 * Bit Feild of OBC_GRAY_BLD_0: OBC_GRAY_MODE
 * MTK_CAM_OBC_GRAY_BLD_0_OBC_GRAY_MODE: [1, 2]
 * Method of gray value computation
 */
#define MTK_CAM_OBC_GRAY_BLD_0_OBC_GRAY_MODE_MASK   0x00000006
#define MTK_CAM_OBC_GRAY_BLD_0_OBC_GRAY_MODE_SHIFT  1

/**
 * Bit Feild of OBC_GRAY_BLD_0: OBC_NORM_BIT
 * MTK_CAM_OBC_GRAY_BLD_0_OBC_NORM_BIT: [3, 7]
 * Data scale to be normalized to 12-bit
 */
#define MTK_CAM_OBC_GRAY_BLD_0_OBC_NORM_BIT_MASK   0x000000f8
#define MTK_CAM_OBC_GRAY_BLD_0_OBC_NORM_BIT_SHIFT  3

/**
 * Bit Feild of OBC_GRAY_BLD_1: OBC_BLD_MXRT
 * MTK_CAM_OBC_GRAY_BLD_1_OBC_BLD_MXRT: [0, 7]
 * (normal and LE)Maximum weight for gray blending
 */
#define MTK_CAM_OBC_GRAY_BLD_1_OBC_BLD_MXRT_MASK   0x000000ff
#define MTK_CAM_OBC_GRAY_BLD_1_OBC_BLD_MXRT_SHIFT  0

/**
 * Bit Feild of OBC_GRAY_BLD_1: OBC_BLD_LOW
 * MTK_CAM_OBC_GRAY_BLD_1_OBC_BLD_LOW: [8, 19]
 * (normal and LE)Luma level below which the gray value is bleneded with the specified maximum weight.
 */
#define MTK_CAM_OBC_GRAY_BLD_1_OBC_BLD_LOW_MASK   0x000fff00
#define MTK_CAM_OBC_GRAY_BLD_1_OBC_BLD_LOW_SHIFT  8

/**
 * Bit Feild of OBC_GRAY_BLD_1: OBC_BLD_SLP
 * MTK_CAM_OBC_GRAY_BLD_1_OBC_BLD_SLP: [20, 31]
 * (normal and LE)Slope of the blending ratio curve between zero and maximum weight.
 */
#define MTK_CAM_OBC_GRAY_BLD_1_OBC_BLD_SLP_MASK   0xfff00000
#define MTK_CAM_OBC_GRAY_BLD_1_OBC_BLD_SLP_SHIFT  20

/**
 * Bit Feild of OBC_WBG_RB: OBC_PGN_R
 * MTK_CAM_OBC_WBG_RB_OBC_PGN_R: [0, 12]
 * WB gain for R channel
 */
#define MTK_CAM_OBC_WBG_RB_OBC_PGN_R_MASK   0x00001fff
#define MTK_CAM_OBC_WBG_RB_OBC_PGN_R_SHIFT  0

/**
 * Bit Feild of OBC_WBG_RB: OBC_PGN_B
 * MTK_CAM_OBC_WBG_RB_OBC_PGN_B: [16, 28]
 * WB gain for R channel
 */
#define MTK_CAM_OBC_WBG_RB_OBC_PGN_B_MASK   0x1fff0000
#define MTK_CAM_OBC_WBG_RB_OBC_PGN_B_SHIFT  16

/**
 * Bit Feild of OBC_WBG_G: OBC_PGN_G
 * MTK_CAM_OBC_WBG_G_OBC_PGN_G: [0, 12]
 * WB gain for G channel
 */
#define MTK_CAM_OBC_WBG_G_OBC_PGN_G_MASK   0x00001fff
#define MTK_CAM_OBC_WBG_G_OBC_PGN_G_SHIFT  0

/**
 * Bit Feild of OBC_WBIG_RB: OBC_IVGN_R
 * MTK_CAM_OBC_WBIG_RB_OBC_IVGN_R: [0, 9]
 * Inverse WB gain for R channel
 */
#define MTK_CAM_OBC_WBIG_RB_OBC_IVGN_R_MASK   0x000003ff
#define MTK_CAM_OBC_WBIG_RB_OBC_IVGN_R_SHIFT  0

/**
 * Bit Feild of OBC_WBIG_RB: OBC_IVGN_B
 * MTK_CAM_OBC_WBIG_RB_OBC_IVGN_B: [16, 25]
 * Inverse WB gain for B channel
 */
#define MTK_CAM_OBC_WBIG_RB_OBC_IVGN_B_MASK   0x03ff0000
#define MTK_CAM_OBC_WBIG_RB_OBC_IVGN_B_SHIFT  16

/**
 * Bit Feild of OBC_WBIG_G: OBC_IVGN_G
 * MTK_CAM_OBC_WBIG_G_OBC_IVGN_G: [0, 9]
 * Inverse WB gain for G channel
 */
#define MTK_CAM_OBC_WBIG_G_OBC_IVGN_G_MASK   0x000003ff
#define MTK_CAM_OBC_WBIG_G_OBC_IVGN_G_SHIFT  0

/**
 * Bit Feild of OBC_OBG_RB: OBC_GAIN_R
 * MTK_CAM_OBC_OBG_RB_OBC_GAIN_R: [0, 11]
 * OB gain for R channel
 */
#define MTK_CAM_OBC_OBG_RB_OBC_GAIN_R_MASK   0x00000fff
#define MTK_CAM_OBC_OBG_RB_OBC_GAIN_R_SHIFT  0

/**
 * Bit Feild of OBC_OBG_RB: OBC_GAIN_B
 * MTK_CAM_OBC_OBG_RB_OBC_GAIN_B: [16, 27]
 * OB gain for B channel
 */
#define MTK_CAM_OBC_OBG_RB_OBC_GAIN_B_MASK   0x0fff0000
#define MTK_CAM_OBC_OBG_RB_OBC_GAIN_B_SHIFT  16

/**
 * Bit Feild of OBC_OBG_G: OBC_GAIN_GR
 * MTK_CAM_OBC_OBG_G_OBC_GAIN_GR: [0, 11]
 * OB gain for Gr channel
 */
#define MTK_CAM_OBC_OBG_G_OBC_GAIN_GR_MASK   0x00000fff
#define MTK_CAM_OBC_OBG_G_OBC_GAIN_GR_SHIFT  0

/**
 * Bit Feild of OBC_OBG_G: OBC_GAIN_GB
 * MTK_CAM_OBC_OBG_G_OBC_GAIN_GB: [16, 27]
 * OB gain for Gb channel
 */
#define MTK_CAM_OBC_OBG_G_OBC_GAIN_GB_MASK   0x0fff0000
#define MTK_CAM_OBC_OBG_G_OBC_GAIN_GB_SHIFT  16

/**
 * Bit Feild of OBC_OFFSET_R: OBC_OFST_R
 * MTK_CAM_OBC_OFFSET_R_OBC_OFST_R: [0, 21]
 * OB offset for R channel
 */
#define MTK_CAM_OBC_OFFSET_R_OBC_OFST_R_MASK   0x003fffff
#define MTK_CAM_OBC_OFFSET_R_OBC_OFST_R_SHIFT  0

/**
 * Bit Feild of OBC_OFFSET_GR: OBC_OFST_GR
 * MTK_CAM_OBC_OFFSET_GR_OBC_OFST_GR: [0, 21]
 * OB offset for Gr channel
 */
#define MTK_CAM_OBC_OFFSET_GR_OBC_OFST_GR_MASK   0x003fffff
#define MTK_CAM_OBC_OFFSET_GR_OBC_OFST_GR_SHIFT  0

/**
 * Bit Feild of OBC_OFFSET_GB: OBC_OFST_GB
 * MTK_CAM_OBC_OFFSET_GB_OBC_OFST_GB: [0, 21]
 * OB offset for Gb channel
 */
#define MTK_CAM_OBC_OFFSET_GB_OBC_OFST_GB_MASK   0x003fffff
#define MTK_CAM_OBC_OFFSET_GB_OBC_OFST_GB_SHIFT  0

/**
 * Bit Feild of OBC_OFFSET_B: OBC_OFST_B
 * MTK_CAM_OBC_OFFSET_B_OBC_OFST_B: [0, 21]
 * OB offset for B channel
 */
#define MTK_CAM_OBC_OFFSET_B_OBC_OFST_B_MASK   0x003fffff
#define MTK_CAM_OBC_OFFSET_B_OBC_OFST_B_SHIFT  0

/**
 * Bit Feild of TSFS_DGAIN: TSFS_REGEN_Y_EN
 * MTK_CAM_TSFS_DGAIN_TSFS_REGEN_Y_EN: [0, 0]
 * Digital gain control
 */
#define MTK_CAM_TSFS_DGAIN_TSFS_REGEN_Y_EN_MASK   0x00000001
#define MTK_CAM_TSFS_DGAIN_TSFS_REGEN_Y_EN_SHIFT  0

/**
 * Bit Feild of TSFS_DGAIN: TSFS_GAIN
 * MTK_CAM_TSFS_DGAIN_TSFS_GAIN: [1, 16]
 * Digital gain
 */
#define MTK_CAM_TSFS_DGAIN_TSFS_GAIN_MASK   0x0001fffe
#define MTK_CAM_TSFS_DGAIN_TSFS_GAIN_SHIFT  1


#endif  // HW_IMGSTREAM_INC_DRV_COMMON_7_1_MT8188_CAM_RAW_REGISTER_H_

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

#ifndef AAA_ISPHAL_SRC_INCLUDE_ISPHALIMP_V2_ITUNINGDATAPROVIDER_H_
#define AAA_ISPHAL_SRC_INCLUDE_ISPHALIMP_V2_ITUNINGDATAPROVIDER_H_

// #include <ispblocks/IspBlockControls.h>       // IspBlockControl
// #include <mtkcam-core/utils/mapping_mgr/idx_cache.h>  // NSIspTuning::IdxCache
#include <memory> // std::shared_ptr
#include <type_traits> // std::is_same

#include <tuning_mapping/cam_idx_struct_ext_pub.h>

#include "mtkcam-chrom/custom/mt8188/hal/inc/isp_tuning/ver1/isp_tuning_cam_info_pub.h"

namespace mtk {
namespace isphal {
namespace v1 {

typedef struct isp_lpnrthres_Param {
	uint32_t LPNR_ISO_LOW_TH : 16;
	uint32_t LPNR_ISO_HIGH_TH : 16;
	uint32_t LPNR_RSV0 : 16;
	uint32_t LPNR_RSV1 : 16;
} isp_lpnrthres_Param;

typedef struct isp_mfnrthres_Param {
	uint32_t iso_th : 14;
	uint32_t MFNR_THRES_THRES1_rsv_14 : 18;

} isp_mfnrthres_Param;

typedef struct isp_bss_Param {
	uint32_t bss_ver : 2;
	uint32_t bss_ver_rsv_2 : 30;
	uint32_t scale_factor : 8;
	uint32_t scale_factor_rsv_8 : 24;
	uint32_t clip_th0 : 8;
	uint32_t clip_th0_rsv_8 : 24;
	uint32_t clip_th1 : 8;
	uint32_t clip_th1_rsv_8 : 24;
	uint32_t clip_th2 : 8;
	uint32_t clip_th2_rsv_8 : 24;
	uint32_t clip_th3 : 8;
	uint32_t clip_th3_rsv_8 : 24;
	uint32_t zero_gmv : 8;
	uint32_t zero_gmv_rsv_8 : 24;
	uint32_t adf_th : 8;
	uint32_t adf_th_rsv_8 : 24;
	uint32_t sdf_th : 8;
	uint32_t sdf_th_rsv_8 : 24;
	uint32_t ypf_en : 1;
	uint32_t ypf_en_rsv_1 : 31;
	uint32_t ypf_fac : 8;
	uint32_t ypf_fac_rsv_8 : 24;
	uint32_t ypf_adj_th : 8;
	uint32_t ypf_adj_th_rsv_8 : 24;
	uint32_t ypf_dfmed0 : 8;
	uint32_t ypf_dfmed0_rsv_8 : 24;
	uint32_t ypf_dfmed1 : 8;
	uint32_t ypf_dfmed1_rsv_8 : 24;
	uint32_t ypf_th0 : 8;
	uint32_t ypf_th0_rsv_8 : 24;
	uint32_t ypf_th1 : 8;
	uint32_t ypf_th1_rsv_8 : 24;
	uint32_t ypf_th2 : 8;
	uint32_t ypf_th2_rsv_8 : 24;
	uint32_t ypf_th3 : 8;
	uint32_t ypf_th3_rsv_8 : 24;
	uint32_t ypf_th4 : 8;
	uint32_t ypf_th4_rsv_8 : 24;
	uint32_t ypf_th5 : 8;
	uint32_t ypf_th5_rsv_8 : 24;
	uint32_t ypf_th6 : 8;
	uint32_t ypf_th6_rsv_8 : 24;
	uint32_t ypf_th7 : 8;
	uint32_t ypf_th7_rsv_8 : 24;
	uint32_t fd_en : 1;
	uint32_t fd_en_rsv_1 : 31;
	uint32_t fd_fac : 8;
	uint32_t fd_fac_rsv_8 : 24;
	uint32_t fd_fnum : 4;
	uint32_t fd_fnum_rsv_4 : 28;
	uint32_t eye_en : 1;
	uint32_t eye_en_rsv_1 : 31;
	uint32_t eye_cfth : 8;
	uint32_t eye_cfth_rsv_8 : 24;
	uint32_t eye_ratio0 : 8;
	uint32_t eye_ratio0_rsv_8 : 24;
	uint32_t eye_ratio1 : 8;
	uint32_t eye_ratio1_rsv_8 : 24;
	uint32_t eye_fac : 8;
	uint32_t eye_fac_rsv_8 : 24;
	uint32_t FaceCVTh : 7;
	uint32_t bss_faceCVTh_rsv_7 : 25;
	uint32_t GradThL : 8;
	uint32_t bss_grad_thL_rsv_8 : 24;
	uint32_t GradThH : 8;
	uint32_t bss_grad_thH_rsv_8 : 24;
	uint32_t FaceAreaThL0 : 16;
	uint32_t faceareaThL0_rsv_16 : 16;
	uint32_t FaceAreaThL1 : 16;
	uint32_t faceareaThL1_rsv_16 : 16;
	uint32_t FaceAreaThH0 : 16;
	uint32_t faceareaThH0_rsv_16 : 16;
	uint32_t FaceAreaThH1 : 16;
	uint32_t faceareaThH1_rsv_16 : 16;
	uint32_t APLDeltaTh0 : 12;
	uint32_t apldeltaTh0_rsv_12 : 20;
	uint32_t APLDeltaTh1 : 12;
	uint32_t apldeltaTh1_rsv_12 : 20;
	uint32_t APLDeltaTh2 : 12;
	uint32_t apldeltaTh2_rsv_12 : 20;
	uint32_t APLDeltaTh3 : 12;
	uint32_t apldeltaTh3_rsv_12 : 20;
	uint32_t APLDeltaTh4 : 12;
	uint32_t apldeltaTh4_rsv_12 : 20;
	uint32_t APLDeltaTh5 : 12;
	uint32_t apldeltaTh5_rsv_12 : 20;
	uint32_t APLDeltaTh6 : 12;
	uint32_t apldeltaTh6_rsv_12 : 20;
	uint32_t APLDeltaTh7 : 12;
	uint32_t apldeltaTh7_rsv_12 : 20;
	uint32_t APLDeltaTh8 : 12;
	uint32_t apldeltaTh8_rsv_12 : 20;
	uint32_t APLDeltaTh9 : 12;
	uint32_t apldeltaTh9_rsv_12 : 20;
	uint32_t APLDeltaTh10 : 12;
	uint32_t apldeltaTh10_rsv_12 : 20;
	uint32_t APLDeltaTh11 : 12;
	uint32_t apldeltaTh11_rsv_12 : 20;
	uint32_t APLDeltaTh12 : 12;
	uint32_t apldeltaTh12_rsv_12 : 20;
	uint32_t APLDeltaTh13 : 12;
	uint32_t apldeltaTh13_rsv_12 : 20;
	uint32_t APLDeltaTh14 : 12;
	uint32_t apldeltaTh14_rsv_12 : 20;
	uint32_t APLDeltaTh15 : 12;
	uint32_t apldeltaTh15_rsv_12 : 20;
	uint32_t APLDeltaTh16 : 12;
	uint32_t apldeltaTh16_rsv_12 : 20;
	uint32_t APLDeltaTh17 : 12;
	uint32_t apldeltaTh17_rsv_12 : 20;
	uint32_t APLDeltaTh18 : 12;
	uint32_t apldeltaTh18_rsv_12 : 20;
	uint32_t APLDeltaTh19 : 12;
	uint32_t apldeltaTh19_rsv_12 : 20;
	uint32_t APLDeltaTh20 : 12;
	uint32_t apldeltaTh20_rsv_12 : 20;
	uint32_t APLDeltaTh21 : 12;
	uint32_t apldeltaTh21_rsv_12 : 20;
	uint32_t APLDeltaTh22 : 12;
	uint32_t apldeltaTh22_rsv_12 : 20;
	uint32_t APLDeltaTh23 : 12;
	uint32_t apldeltaTh23_rsv_12 : 20;
	uint32_t APLDeltaTh24 : 12;
	uint32_t apldeltaTh24_rsv_12 : 20;
	uint32_t APLDeltaTh25 : 12;
	uint32_t apldeltaTh25_rsv_12 : 20;
	uint32_t APLDeltaTh26 : 12;
	uint32_t apldeltaTh26_rsv_12 : 20;
	uint32_t APLDeltaTh27 : 12;
	uint32_t apldeltaTh27_rsv_12 : 20;
	uint32_t APLDeltaTh28 : 12;
	uint32_t apldeltaTh28_rsv_12 : 20;
	uint32_t APLDeltaTh29 : 12;
	uint32_t apldeltaTh29_rsv_12 : 20;
	uint32_t APLDeltaTh30 : 12;
	uint32_t apldeltaTh30_rsv_12 : 20;
	uint32_t APLDeltaTh31 : 12;
	uint32_t apldeltaTh31_rsv_12 : 20;
	uint32_t APLDeltaTh32 : 12;
	uint32_t apldeltaTh32_rsv_12 : 20;
	uint32_t GradRatioTh0 : 15;
	uint32_t gradRatioTh0_rsv_15 : 17;
	uint32_t GradRatioTh1 : 15;
	uint32_t gradRatioTh1_rsv_15 : 17;
	uint32_t GradRatioTh2 : 15;
	uint32_t gradRatioTh2_rsv_15 : 17;
	uint32_t GradRatioTh3 : 15;
	uint32_t gradRatioTh3_rsv_15 : 17;
	uint32_t GradRatioTh4 : 15;
	uint32_t gradRatioTh4_rsv_15 : 17;
	uint32_t GradRatioTh5 : 15;
	uint32_t gradRatioTh5_rsv_15 : 17;
	uint32_t GradRatioTh6 : 15;
	uint32_t gradRatioTh6_rsv_15 : 17;
	uint32_t GradRatioTh7 : 15;
	uint32_t gradRatioTh7_rsv_15 : 17;
	uint32_t EyeDistThL : 8;
	uint32_t eyedistThL_rsv_8 : 24;
	uint32_t EyeDistThH : 8;
	uint32_t eyedistThH_rsv_8 : 24;
	uint32_t EyeMinWeight : 15;
	uint32_t eyeminweight_rsv_15 : 17;
	uint32_t MF_BSS_ACTS_CLIP_TH0 : 6;
	uint32_t acts_clip_th0_rsv_6 : 26;
	uint32_t MF_BSS_ACTS_CLIP_TH1 : 6;
	uint32_t acts_clip_th1_rsv_6 : 26;
	uint32_t MF_BSS_BLEND_ISO_ThH : 14;
	uint32_t blend_iso_thH_rsv_14 : 18;
	uint32_t MF_BSS_BLEND_ISO_ThL : 14;
	uint32_t blend_iso_thL_rsv_14 : 18;
	uint32_t MF_BSS_Sharp_min_weight : 9;
	uint32_t sharpness_min_weight_rsv_9 : 23;
	uint32_t AI_SHUTTER_MODE : 1;
	uint32_t ai_shutter_mode_rsv_1 : 31;
	uint32_t SCORE_WEIGHT0 : 15;
	uint32_t score_weight0_rsv_15 : 17;
	uint32_t SCORE_WEIGHT1 : 15;
	uint32_t score_weight1_rsv_15 : 17;
	uint32_t SCORE_WEIGHT2 : 15;
	uint32_t score_weight2_rsv_15 : 17;
	uint32_t SCORE_WEIGHT3 : 15;
	uint32_t score_weight3_rsv_15 : 17;
	uint32_t SCORE_WEIGHT4 : 15;
	uint32_t score_weight4_rsv_15 : 17;
	uint32_t SCORE_WEIGHT5 : 15;
	uint32_t score_weight5_rsv_15 : 17;
	uint32_t SCORE_WEIGHT6 : 15;
	uint32_t score_weight6_rsv_15 : 17;
	uint32_t SCORE_WEIGHT7 : 15;
	uint32_t score_weight7_rsv_15 : 17;

} isp_bss_Param;

typedef struct isp_swme_Param {
	uint32_t ME_NOISE_LEVEL : 8;
	uint32_t ME_MPME_SRH_RNG : 8;
	uint32_t ME_MPME_SAD_COR : 8;
	uint32_t ME_MPME_VAR_COR : 8;
	uint32_t ME_MPME_GMV_RTO_TH1 : 8;
	uint32_t ME_MPME_GMV_CNT_TH1 : 8;
	uint32_t ME_MPME_GMV_RTO_TH2 : 8;
	uint32_t ME_MPME_GMV_CNT_TH2 : 8;
	uint32_t ME_MPME_GMV_RTO_TH3 : 8;
	uint32_t ME_MPME_GMV_CNT_TH3 : 8;
	uint32_t ME_MPME_GMV_2_rsv_16 : 16;
	uint32_t ME_MPME_CAND_ZERO_PNLTY : 8;
	uint32_t ME_MPME_CAND_GMV_PNLTY : 8;
	uint32_t ME_MPME_CAND_TMPR_PNLTY : 8;
	uint32_t ME_MPME_CAND_1_rsv_24 : 8;
	uint32_t ME_MPME_CAND_RAND_INT_PNLTY : 8;
	uint32_t ME_MPME_CAND_RAND_SUB_PNLTY : 8;
	uint32_t ME_MPME_CAND_2_rsv_16 : 16;
	uint32_t ME_MPME_CAND_BG_GMV_PNLTY_GAIN : 8;
	uint32_t ME_MPME_CAND_BG_GMV_PNLTY_CLIP : 8;
	uint32_t ME_MPME_CAND_FG_GMV_PNLTY_GAIN : 8;
	uint32_t ME_MPME_CAND_FG_GMV_PNLTY_CLIP : 8;
	uint32_t ME_MPME_CAND_INT_SMTH_PNLTY_VAR_H_TH : 8;
	uint32_t ME_MPME_CAND_INT_SMTH_PNLTY_VAR_V_TH : 8;
	uint32_t ME_MPME_CAND_INT_SMTH_PNLTY_GAIN_MAX : 8;
	uint32_t ME_MPME_CAND_INT_SMTH_PNLTY_CLIP : 8;
	uint32_t ME_MPME_CAND_SUB_SMTH_PNLTY_GAIN : 8;
	uint32_t ME_MPME_CAND_SUB_SMTH_PNLTY_CLIP : 8;
	uint32_t ME_MPME_CAND_5_rsv_16 : 16;
	uint32_t ME_MVEP_BLK_WD : 8;
	uint32_t ME_MVEP_GENERAL_rsv_8 : 24;
	uint32_t ME_MVEP_MVBLD_MODE : 2;
	uint32_t ME_MVEP_MVBLD_rsv_2 : 6;
	uint32_t ME_MVEP_MVBLD_ERR_OFST : 8;
	uint32_t ME_MVEP_MVBLD_rsv_16 : 16;
	uint32_t ME_MVEP_CJDET_TH1_A : 8;
	uint32_t ME_MVEP_CJDET_TH2_A : 8;
	uint32_t ME_MVEP_CJDET_MAX_A : 8;
	uint32_t ME_MVEP_CJDET_TH1_B : 8;
	uint32_t ME_MVEP_CJDET_TH2_B : 8;
	uint32_t ME_MVEP_CJDET_MAX_B : 8;
	uint32_t ME_MVEP_CJDET_TH1_C : 8;
	uint32_t ME_MVEP_CJDET_TH2_C : 8;
	uint32_t ME_MVEP_CJDET_MAX_C : 8;
	uint32_t ME_MVEP_CJDET_3_rsv_8 : 24;
	uint32_t ME_MVEP_RVDET_TH1_A : 8;
	uint32_t ME_MVEP_RVDET_TH2_A : 8;
	uint32_t ME_MVEP_RVDET_MAX_A : 8;
	uint32_t ME_MVEP_RVDET_MIN_A : 8;
	uint32_t ME_MVEP_RVDET_TH1_B : 8;
	uint32_t ME_MVEP_RVDET_TH2_B : 8;
	uint32_t ME_MVEP_RVDET_MAX_B : 8;
	uint32_t ME_MVEP_RVDET_MIN_B : 8;
	uint32_t ME_MVEP_RVDET_TH1_C : 8;
	uint32_t ME_MVEP_RVDET_TH2_C : 8;
	uint32_t ME_MVEP_RVDET_MAX_C : 8;
	uint32_t ME_MVEP_RVDET_MIN_C : 8;
	uint32_t ME_MVEP_SKDET_TH1_A : 8;
	uint32_t ME_MVEP_SKDET_TH2_A : 8;
	uint32_t ME_MVEP_SKDET_MAX_A : 8;
	uint32_t ME_MVEP_SKDET_TH1_B : 8;
	uint32_t ME_MVEP_SKDET_TH2_B : 8;
	uint32_t ME_MVEP_SKDET_MAX_B : 8;
	uint32_t ME_MVEP_SKDET_TH1_C : 8;
	uint32_t ME_MVEP_SKDET_TH2_C : 8;
	uint32_t ME_MVEP_SKDET_MAX_C : 8;
	uint32_t ME_MVEP_SKDET_3_rsv_8 : 24;
	uint32_t ME_CONF_MVIDX_DX_TH : 8;
	uint32_t ME_CONF_MVIDX_DY_TH : 8;
	uint32_t ME_CONF_MVIDX_DIV : 8;
	uint32_t ME_CONF_MVIDX_COR : 8;
	uint32_t ME_CONF_MVIDX_MAX : 8;
	uint32_t ME_CONF_MVIDX_2_rsv_8 : 24;
	uint32_t ME_CONF_SADIDX_RS : 8;
	uint32_t ME_CONF_SADIDX_COR : 8;
	uint32_t ME_CONF_SADIDX_MAX : 8;
	uint32_t ME_CONF_SADIDX_1_rsv_24 : 8;
	uint32_t ME_CONF_VARIDX_RS : 8;
	uint32_t ME_CONF_VARIDX_COR : 8;
	uint32_t ME_CONF_VARIDX_MAX : 8;
	uint32_t ME_CONF_VARIDX_1_rsv_24 : 8;
	uint32_t ME_CONF_DCIDX_RS : 8;
	uint32_t ME_CONF_DCIDX_COR : 8;
	uint32_t ME_CONF_DCIDX_MAX : 8;
	uint32_t ME_CONF_DCIDX_1_rsv_24 : 8;
	uint32_t ME_CONF_FLT_OUTLIER_IDX_TH : 8;
	uint32_t ME_CONF_FLT_SMTH_DT_MAX1 : 8;
	uint32_t ME_CONF_FLT_SMTH_DT_MAX2 : 8;
	uint32_t ME_CONF_FLT_rsv_24 : 8;
	uint32_t ME_CONF_ISM_MAX : 8;
	uint32_t ME_CONF_ISM_TH1 : 8;
	uint32_t ME_CONF_ISM_TH2 : 8;
	uint32_t ME_CONF_ISM_rsv_24 : 8;
	uint32_t ME_CONF_MVIDX_SMCTP_DC_COR : 8;
	uint32_t ME_CONF_MVIDX_SMCTP_DC_RS : 8;
	uint32_t ME_CONF_MVIDX_SMCTP_DC_MAX : 8;
	uint32_t ME_CONF_MVIDX_SMCTP_DC_GAIN : 8;
	uint32_t ME_CONF_MVIDX_SMCTP_VAR_TH1 : 8;
	uint32_t ME_CONF_MVIDX_SMCTP_VAR_TH2 : 8;
	uint32_t ME_CONF_MVIDX_SMCTP_2_rsv_16 : 16;
	uint32_t ME_CONF_CTIDX_SML_TH1 : 8;
	uint32_t ME_CONF_CTIDX_rsv_8 : 24;
	uint32_t ME_CONF_MVEP_EN : 8;
	uint32_t ME_CONF_MVEP_CONF_OFST : 8;
	uint32_t ME_CONF_MVEP_VAR_WT_RS : 8;
	uint32_t ME_CONF_MVEP_1_rsv_24 : 8;
	uint32_t ME_CONF_MVEP_VAR_WT_X1 : 8;
	uint32_t ME_CONF_MVEP_VAR_WT_X2 : 8;
	uint32_t ME_CONF_MVEP_VAR_WT_Y1 : 8;
	uint32_t ME_CONF_MVEP_VAR_WT_Y2 : 8;
	uint32_t ME_CONF_MVEP_DC_WT_X1 : 8;
	uint32_t ME_CONF_MVEP_DC_WT_X2 : 8;
	uint32_t ME_CONF_MVEP_DC_WT_Y1 : 8;
	uint32_t ME_CONF_MVEP_DC_WT_Y2 : 8;
	uint32_t ME_CONF_MVEP_LMC_X1 : 8;
	uint32_t ME_CONF_MVEP_LMC_X2 : 8;
	uint32_t ME_CONF_MVEP_LMC_X3 : 8;
	uint32_t ME_CONF_MVEP_LMC_Y1 : 8;
	uint32_t ME_CONF_MVEP_LMC_Y2 : 8;
	uint32_t ME_CONF_MVEP_LMC_Y3 : 8;
	uint32_t ME_CONF_MVEP_LMC_2_rsv_16 : 16;
	uint32_t ME_SDGN_X1 : 8;
	uint32_t ME_SDGN_X2 : 8;
	uint32_t ME_SDGN_X3 : 8;
	uint32_t ME_SDGN_X4 : 8;
	uint32_t ME_SDGN_Y1 : 8;
	uint32_t ME_SDGN_Y2 : 8;
	uint32_t ME_SDGN_Y3 : 8;
	uint32_t ME_SDGN_Y4 : 8;
	uint32_t ME_SDGN_EN_A : 1;
	uint32_t ME_SDGN_EN_B : 1;
	uint32_t ME_SDGN_EN_C : 1;
	uint32_t ME_SDGN_EN_D : 1;
	uint32_t ME_SDGN_3_rsv_4 : 28;
	uint32_t ME_LCL_DECONF_EN : 1;
	uint32_t ME_LCL_DECONF_DLTVAR_EN : 1;
	uint32_t ME_LCL_DECONF_1_rsv_2 : 30;
	uint32_t ME_LCL_DECONF_BG_BSS_RATIO : 16;
	uint32_t ME_LCL_DECONF_FD_BSS_RATIO : 16;
	uint32_t ME_LARGE_MV_TXTR_EN : 1;
	uint32_t ME_LARGE_MV_1_rsv_1 : 15;
	uint32_t ME_LARGE_MV_THD : 16;
	uint32_t ME_LARGE_MV_SAD_THD : 16;
	uint32_t ME_LARGE_MV_RATIO : 16;
	uint32_t ME_LARGE_MV_TXTR_WEI : 16;
	uint32_t ME_LARGE_MV_TXTR_THD : 16;
	uint32_t ME_MPME_HG_rsv_0 : 16;
	uint32_t ME_MPME_HG_M1_CONF_TH : 8;
	uint32_t ME_MPME_HG_M2_CONF_TH : 8;
	uint32_t ME_MPME_TXTLVL_NOISE_TH_H : 8;
	uint32_t ME_MPME_TXTLVL_NOISE_TH_V : 8;
	uint32_t ME_MPME_TXTLVL_rsv_16 : 16;
	uint32_t ME_MPME_TOTAL_PNLTY_GAIN : 8;
	uint32_t ME_MPME_NEW_GMV_PNLTY_MODE : 8;
	uint32_t ME_MPME_GMV_PNLTY_GAIN_M1_A : 8;
	uint32_t ME_MPME_GMV_PNLTY_CLIP_M1_A : 8;
	uint32_t ME_MPME_GMV_PNLTY_MVDF_TH_M1_B : 8;
	uint32_t ME_MPME_GMV_PNLTY_GAIN_M1_B : 8;
	uint32_t ME_MPME_GMV_PNLTY_CLIP_M1_B : 8;
	uint32_t ME_MPME_GMV_PNLTY_MVDF_TH_C1_B : 8;
	uint32_t ME_MPME_GMV_PNLTY_GAIN_C1_B : 8;
	uint32_t ME_MPME_GMV_PNLTY_CLIP_C1_B : 8;
	uint32_t ME_MPME_GMV_PNLTY_MVDF_TH_M2_B : 8;
	uint32_t ME_MPME_GMV_PNLTY_GAIN_M2_B : 8;
	uint32_t ME_MPME_GMV_PNLTY_CLIP_M2_B : 8;
	uint32_t ME_MPME_GMV_PNLTY_MVDF_TH_C2_B : 8;
	uint32_t ME_MPME_GMV_PNLTY_GAIN_C2_B : 8;
	uint32_t ME_MPME_GMV_PNLTY_CLIP_C2_B : 8;
	uint32_t ME_MPME_NEW_SMTH_PNLTY_MODE : 8;
	uint32_t ME_MPME_SMTH_PNLTY_TXTLVL_TH0 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_TXTLVL_TH1 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_1_rsv_24 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_MVGAIN_HIGH_RAND : 8;
	uint32_t ME_MPME_SMTH_PNLTY_MVGAIN_LOW_RAND : 8;
	uint32_t ME_MPME_SMTH_PNLTY_CLIP_RAND : 8;
	uint32_t ME_MPME_SMTH_PNLTY_MVGAIN_HIGH_TEMP : 8;
	uint32_t ME_MPME_SMTH_PNLTY_MVGAIN_LOW_TEMP : 8;
	uint32_t ME_MPME_SMTH_PNLTY_CLIP_TEMP : 8;
	uint32_t ME_MPME_SMTH_PNLTY_MVGAIN_HIGH_GMV_M1 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_MVGAIN_LOW_GMV_M1 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_CLIP_GMV_M1 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_MVGAIN_HIGH_GMV_C1 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_MVGAIN_LOW_GMV_C1 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_CLIP_GMV_C1 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_MVGAIN_HIGH_GMV_M2 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_MVGAIN_LOW_GMV_M2 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_CLIP_GMV_M2 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_MVGAIN_HIGH_GMV_C2 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_MVGAIN_LOW_GMV_C2 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_CLIP_GMV_C2 : 8;
	uint32_t ME_MPME_SMTH_PNLTY_6_rsv_16 : 16;
	uint32_t ME_MPME_GMV_CLST_MODE : 8;
	uint32_t ME_MPME_GMV_CLST_TXTLVL_TH : 8;
	uint32_t ME_MPME_GMV_CLST_MVDF_TH : 8;
	uint32_t ME_MPME_GMV_CLST_1_rsv_24 : 8;
	uint32_t ME_MPME_GMV_CLST_B_RTO_TH : 8;
	uint32_t ME_MPME_GMV_CLST_B_CNT_TH : 8;
	uint32_t ME_MPME_GMV_CLST_F_RTO_TH : 8;
	uint32_t ME_MPME_GMV_CLST_F_CNT_TH : 8;
	uint32_t ME_RSV_0_0 : 16;
	uint32_t ME_RSV_0_1 : 16;
	uint32_t ME_RSV_1_0 : 16;
	uint32_t ME_RSV_1_1 : 16;
	uint32_t ME_RSV_2_0 : 16;
	uint32_t ME_RSV_2_1 : 16;
	uint32_t ME_RSV_3_0 : 16;
	uint32_t ME_RSV_3_1 : 16;
	uint32_t ME_RSV_4_0 : 16;
	uint32_t ME_RSV_4_1 : 16;
	uint32_t ME_RSV_5_0 : 16;
	uint32_t ME_RSV_5_1 : 16;
	uint32_t ME_RSV_6_0 : 16;
	uint32_t ME_RSV_6_1 : 16;
	uint32_t ME_RSV_7_0 : 16;
	uint32_t ME_RSV_7_1 : 16;
	uint32_t ME_RSV_8_0 : 16;
	uint32_t ME_RSV_8_1 : 16;
	uint32_t ME_RSV_9_0 : 16;
	uint32_t ME_RSV_9_1 : 16;
	uint32_t ME_RSV_10_0 : 16;
	uint32_t ME_RSV_10_1 : 16;
	uint32_t ME_RSV_11_0 : 16;
	uint32_t ME_RSV_11_1 : 16;
	uint32_t ME_RSV_12_0 : 16;
	uint32_t ME_RSV_12_1 : 16;
	uint32_t ME_RSV_13_0 : 16;
	uint32_t ME_RSV_13_1 : 16;
	uint32_t ME_RSV_14_0 : 16;
	uint32_t ME_RSV_14_1 : 16;
	uint32_t ME_VERSION : 16;
	uint32_t ME_VERSION_rsv_16 : 16;
} isp_swme_Param;
/**
 * ITuningDataProvider is an interface class but basically TuningDataProvider is
 * a version based implementations, this base class is for represent the
 * instance of version based instance. Caller should invoke
 * TuningDataProviderTypeHelper<Version>::Type to cast it when
 * TuningDataProviderTypeHelper<Version>::Valid is true.
 */
class ITuningDataProvider
{
public:
	/**
   * @V0: TuningDataProvider version 1.0
   * @V1: TuningDataProvider version 1.1
   * ...
   */
	enum Version : int {
		V0 = 0,
		V1,
		V2,
		V3,
	};

	/**
  * Feature control enum for TuningProvider
  */
	enum kTuningProviderCmd_T {
		kTuningProviderCmd_getNvram_Feature_interpolation,
		kTuningProviderCmd_CalculateMsf_with_luma_mean,
		kTuningProviderCmd_MaxNum,
	};

	// ISP group based ID, describes all SubGroupId. If derived ISP block is not
	// built in all pipeline, do not inherit this base class.
	struct IspGroupTypeBase {
		enum SubGroupId {
			SubGroupId_Undefined = 0,
			SubGroupId_R1,
			SubGroupId_R2,
			SubGroupId_R3,
			SubGroupId_R4,
			SubGroupId_R5,
			SubGroupId_R6,
			SubGroupId_R7,
			SubGroupId_R8,
			SubGroupId_T1,
			SubGroupId_T2,
			SubGroupId_T3,
			SubGroupId_T4,
			SubGroupId_T5,
			SubGroupId_T6,
			SubGroupId_T7,
			SubGroupId_T8,
			SubGroupId_T9,
			SubGroupId_D1,
			SubGroupId_D2,
			SubGroupId_P1A,
			SubGroupId_P1B,
			SubGroupId_E1A,
			SubGroupId_E1B,
			SubGroupId_E1C,
			SubGroupId_Mraw,
			SubGroupId_Mfb,
			SubGroupId_Mss,
		};

		constexpr inline static size_t subGroupId2Idx(SubGroupId gid)
		{
			if (gid == SubGroupId_Undefined)
				return SIZE_MAX;
			return gid - 1;
		}
	};

	static std::shared_ptr<mtk::isphal::v1::ITuningDataProvider> createInstance(
		size_t sensor_index,
		size_t sensor_dev_id,
		uint64_t user_id = 0);

public:
	/**
   * Query version info
   */
	virtual Version getVersion() const = 0;

	virtual bool readDataForFeature(void *p_out,
					int data_size,
					NSIspTuning::EModuleDB_T atms_module,
					const NSIspTuning::CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO &qry_with_sys_info) = 0;

	virtual bool readDataForFeature(void *p_out,
					int data_size,
					NSIspTuning::EModuleDB_T atms_module,
					const NSIspTuning::CAM_IDX_QRY_COMB_ISP7 &qry) = 0;

	virtual void getLatestMappingInfo(
		NSIspTuning::CAM_IDX_QRY_COMB_ISP7 &output) = 0;

	virtual void getLatestMappingInfo(
		NSIspTuning::CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO &output) = 0;

	virtual void releaseSensorTuningDB(
		size_t sensor_index,
		size_t sensor_dev_id) = 0;

public:
	virtual ~ITuningDataProvider() = default;

protected:
	ITuningDataProvider() = default;
};

/**
 * Type helper, undefined type will be 'void'.
 *  @tparam Version of ITuningDataProvider.
 */
template<int V>
struct TuningDataProviderTypeHelper {
	typedef void Type;
	enum { Valid = false };
};

} // namespace v1
} // namespace isphal
} // namespace mtk

namespace mtk {
namespace isphal {
namespace v1_0 {

/**
 * Tuning Data Provider is a class to provide the processed data based on the
 * given ISP Group Type "IspGroupType".
 */
class TuningDataProvider : public v1::ITuningDataProvider
{
public:
	static std::shared_ptr<mtk::isphal::v1::ITuningDataProvider> createInstance(
		size_t sensor_index,
		size_t sensor_dev_id,
		uint64_t user_id = 0);
	TuningDataProvider(size_t sensor_index,
			   size_t sensor_dev_id,
			   uint64_t user_id);

	virtual ~TuningDataProvider();

public: // Version
	enum : int { Ver = v1::ITuningDataProvider::V0 };

public: // re-implementations of ITuningDataProvider
	v1::ITuningDataProvider::Version getVersion() const override
	{
		return v1::ITuningDataProvider::V0;
	}

public:
	bool readDataForFeature(
		void *p_out,
		int data_size,
		EModuleDB_T atms_module,
		const NSIspTuning::CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO &qry_with_sys_info)
		override;

	bool readDataForFeature(void *p_out,
				int data_size,
				EModuleDB_T atms_module,
				const NSIspTuning::CAM_IDX_QRY_COMB_ISP7 &qry)
		override;

	void getLatestMappingInfo(CAM_IDX_QRY_COMB_ISP7 &output) override;
	void getLatestMappingInfo(CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO &output)
		override;
	void releaseSensorTuningDB(size_t sensor_index, size_t sensor_dev_id) override;

private:
	size_t m_sensorid; // current sensor id (not sensor index)
	size_t m_sensor_idx;
	int32_t m_debugEnable;
	int32_t m_i4DbCheckLogEn;
	ALL_ISP_INTERVAL m_all_interval;
	mutable std::mutex m_Lock;
	std::string path;

private:
};
} // namespace v1_0
} // namespace isphal
} // namespace mtk

#endif // AAA_ISPHAL_SRC_INCLUDE_ISPHALIMP_V2_ITUNINGDATAPROVIDER_H_

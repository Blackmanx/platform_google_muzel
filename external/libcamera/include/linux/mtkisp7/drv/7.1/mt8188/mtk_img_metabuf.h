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

#ifndef HW_IMGSTREAM_INC_DRV_COMMON_7_1_MT8188_MTK_IMG_METABUF_H_
#define HW_IMGSTREAM_INC_DRV_COMMON_7_1_MT8188_MTK_IMG_METABUF_H_

#include "mtk_img_3a_user.h"
#include "mtk_img_reg_user.h"

/**
 * Common structure used in statistic meta buffers
 */

/**
 * struct mtk_img_uapi_meta_hw_buf
 *
 * @offset: buffer's start offset from the meta buffer's start
 * @size: The size of the buffer
 *
 * Some meta buffer may be written by hardware
 * and is variable size. We use the strcut to descibes the sub-bufs which are
 * written by statistic hardwares.
 */
struct mtk_img_uapi_meta_hw_buf {
  __u32 offset;
  __u32 size;
};

/**
 * struct mtk_img_uapi_tnc_stats - Tone2 statistic data for
 *                 Mediatek proprietary algorithm
 *
 * @tncso_buf: The buffer for tnc statistic hardware output. The buffer size
 *           is defined in MTK_IMG_UAPI_TNCSO_SIZE (680*510*2)
 */
#define MTK_IMG_UAPI_TNCSO_SIZE (680 * 510 * 2)
struct mtk_img_uapi_tnc_stats {
  struct mtk_img_uapi_meta_hw_buf tncso_buf;
};

/**
 * struct mtk_img_uapi_tnch_stats - Tone3 statistic data for Mediatek
 *                                  proprietary algorithm
 *
 * @tncsho_buf: The buffer for tnch statistic hardware output. The buffer size
 *           is defined in MTK_IMG_UAPI_TNCSHO_SIZE (1544)
 */
#define MTK_IMG_UAPI_TNCSHO_SIZE (1544)
struct mtk_img_uapi_tnch_stats {
  struct mtk_img_uapi_meta_hw_buf tncsho_buf;
};

/**
 * struct mtk_img_uapi_tncb_stats - Tone3 statistic data for Mediatek
 *                                  proprietary algorithm
 *
 * @tncsbo_buf: The buffer for tncb statistic hardware output. The buffer size
 *           is defined in MTK_IMG_UAPI_TNCSBO_SIZE (3888)
 */
#define MTK_IMG_UAPI_TNCSBO_SIZE (3888)
struct mtk_img_uapi_tncb_stats {
  struct mtk_img_uapi_meta_hw_buf tncsbo_buf;
};

/**
 * struct mtk_img_uapi_tncy_stats - Tone3 statistic data for Mediatek
 *                                  proprietary algorithm
 *
 * @tncsyo_buf: The buffer for tncy statistic hardware output. The buffer size
 *           is defined in MTK_IMG_UAPI_TNCSYO_SIZE (68)
 */
#define MTK_IMG_UAPI_TNCSYO_SIZE (68)
struct mtk_img_uapi_tncy_stats {
  struct mtk_img_uapi_meta_hw_buf tncsyo_buf;
};

/**
 * struct mtk_img_uapi_me_frame_status -
 *
 * @me_fst_buf: The buffer for me frame status statistic hardware output. The buffer size
 *           is defined in MTK_IMG_UAPI_ME_FST_SIZE (448)
 */
#define MTK_IMG_UAPI_ME_FST_SIZE (448)
struct mtk_img_uapi_me_frame_status {
  struct mtk_img_uapi_meta_hw_buf me_fst_buf;
};

/**
 * struct mtk_img_uapi_feature_match_blocks -
 *
 * @me_fmb_buf: The buffer for me feature match blocks statistic hardware output. The buffer size
 *           is defined in MTK_IMG_UAPI_ME_FMB_SIZE (3888)
 */
#define MTK_IMG_UAPI_ME_FMB_SIZE (3888)
#define MTK_IMG_UAPI_ME_FMB_L0_SIZE (MTK_IMG_UAPI_ME_FMB_SIZE)
#define MTK_IMG_UAPI_ME_FMB_L1_SIZE (MTK_IMG_UAPI_ME_FMB_SIZE)
struct mtk_img_uapi_me_feature_match_blocks {
  struct mtk_img_uapi_meta_hw_buf me_fmb_buf;
};

/**
 * struct mtk_img_uapi_me_local_motion_info -
 *
 * @me_lmi_buf: The buffer for me feature match blocks statistic hardware output. The buffer size
 *           is defined in MTK_IMG_UAPI_ME_LMI_SIZE (31104)
 */
#define MTK_IMG_UAPI_ME_LMI_SIZE (31104)
struct mtk_img_uapi_me_local_motion_info {
  struct mtk_img_uapi_meta_hw_buf me_lmi_buf;
};

/*
 * Mediatek camera bpc tuning setting from userspace
 * All the member of this structure are confidential
 */
#define MTK_IMG_BPCI_TABLE_SIZE (32)
struct mtk_img_uapi_bpc_param_prot {
  __u32 x_size;
  __u32 y_size;
  __u32 stride;

  __u8 table[MTK_IMG_BPCI_TABLE_SIZE];
};

/*
 * Mediatek camera lsc tuning setting from userspace
 * All the member of this structure are confidential
 */
#define MTK_IMG_LSCI_TABLE_SIZE (32768)
struct mtk_img_uapi_lsc_param_prot {
  __u32 x_blk_num;
  __u32 y_blk_num;
  __u32 x_size;
  __u32 y_size;
  __u32 stride;
  __u8 table[MTK_IMG_LSCI_TABLE_SIZE];
};

/*
 * Mediatek camera slk tuning setting from userspace
 * All the member of this structure are confidential
 *
 */
struct mtk_img_uapi_slk_param_prot {
  __u32 center_x;
  __u32 center_y;
  __u32 radius_0;
  __u32 radius_1;
  __u32 radius_2;
  __u32 gain0;
  __u32 gain1;
  __u32 gain2;
  __u32 gain3;
  __u32 gain4;
};

/*
 * Mediatek camera lsc tuning setting from userspace
 * All the member of this structure are confidential
 */
struct mtk_img_uapi_wb_param_prot {
  __u32 debug_info[39];
};

/*
 * struct mtk_img_uapi_camsys_drzs8t_crop_param_prot
 *
 *  @input_width: input image width
 *  @input_height: input image height
 *  @output_width: output image width
 *  @output_height: output image height
 *  @start_ofst_x: crop start offset X
 *  @start_ofst_y: crop start offset Y
 */
struct mtk_img_uapi_camsys_drzs8t_crop_param_prot {
  __u8 is_enable;
  __u32 input_width;
  __u32 input_height;
  __u32 output_width;
  __u32 output_height;
  __u32 start_ofst_x;
  __u32 start_ofst_y;
};

/*
 * struct mtk_img_uapi_wraping_param_prot
 *
 */
struct mtk_img_uapi_wraping_param_prot {
  __u8 CRT_EN;
  __u32 CRT_IN_WD;
  __u32 CRT_IN_HT;
  __u32 CRT_CROP_WD;
  __u32 CRT_CROP_HT;
  __u32 CRT_POINT_X0;
  __u32 CRT_POINT_Y0;
  __u32 CRT_POINT_X1;
  __u32 CRT_POINT_Y1;
  __u32 CRT_POINT_X2;
  __u32 CRT_POINT_Y2;
  __u32 CRT_POINT_X3;
  __u32 CRT_POINT_Y3;
};

/*
 * struct mtk_img_uapi_pq_serial_num_param_prot
 *
 *  @serial_num_p1a: indicate serial num of P1A tuning
 *  @serial_num_p1b: indicate serial num of P1B tuning
 */
struct mtk_img_uapi_pq_serial_num_param_prot {
  __u32 serial_num_p1a;
  __u32 serial_num_p1b;
};

/*
 * struct mtk_img_uapi_drzs8t_param_prot
 *
 *  @tbl_shift: adjust the computed table
 *  @tbl_min: use to limit the min table
 */
struct mtk_img_uapi_drzs8t_param_prot {
  __u32 tbl_shift;
  __u32 tbl_min;
};

/*
 * Mediatek camera tnr tuning setting from userspace
 * All the member of this structure are confidential
 */
struct mtk_img_uapi_tnr_param_prot {
  __u32 ink_mode_enable;
  __u32 mcnr_me_srch_rng_v;
  __u32 mcnr_me_srch_rng_h;
  __u32 me_srch_rng_zoom_crop_en;
    __u32 tnr_frame_idx;
    __u32 tnr_frame_total;
    __u32 tnr_vb_sel_en;
};

/* The following sw setting are generated by script */
/*
 * struct mtk_img_uapi_ccm_param_prot - CCM parameters *
 */
struct mtk_img_uapi_ccm_param_prot {
  __u32 ccm_acc;
};

/*
 * struct mtk_img_uapi_cnr_param_prot - CNR parameters *
 */
struct mtk_img_uapi_cnr_param_prot {
  __u32 cnr_cnr_slk_link;
  __u32 cnr_cnr_ver_c_ref_y;
  __u32 cnr_cnr_scale_mode;
  __u32 cnr_video_mode;
  __u32 cnr_lbit_mode;
  __u32 cnr_mode;
  __u32 cnr_spk_en;
  __u32 cnr_cnr_enc;
};

/*
 * struct mtk_img_uapi_drz8t_param_prot - DRZ8T parameters *
 */
struct mtk_img_uapi_drz8t_param_prot {
  __u32 drz8t_c42_filt_dis;
  __u32 tbl_shift;
  __u32 tbl_min;
};

/*
 * struct mtk_img_uapi_drzh2n_param_prot - DRZH2N parameters *
 */
struct mtk_img_uapi_drzh2n_param_prot {
  __u32 drzh2n_vert_tbl_sel;
  __u32 drzh2n_hori_tbl_sel;
};

/*
 * struct mtk_cam_uapi_drzs4n_param_prot - DRZS4N parameters *
 */
struct mtk_img_uapi_drzs4n_param_prot {
  __u32 drzs4n_vert_tbl_sel;
  __u32 drzs4n_hori_tbl_sel;
};

/*
 * struct mtk_img_uapi_me_param_prot - ME parameters *
 */
struct mtk_img_uapi_me_param_prot {
  __u32 me_mode;
  __u32 me_slk_en;
  __u32 me_fnl_mv_flt_en;
};

/*
 * struct mtk_img_uapi_rec_param_prot - REC parameters *
 */
struct mtk_img_uapi_rec_param_prot {
  __u32 rec_mode;
};

/*
 * struct mtk_img_uapi_snr_param_prot - SNR parameters *
 */

#define APLHA_MAP_SIZE (320 * 320)
struct mtk_img_uapi_snr_param_prot {
  __u32 snr_tbl_prc;
  __u32 snr_table_en;
  __u32 snr_tbl_base_std;
  __u32 snr_y_pflt_idx;
  __u32 snr_tnr_mode;
  __u32 snr_slk_link;
  __u32 snr_tnr_link;
  __u32 snr_y_l3_off;
  __u32 snr_c_flt_idx;
  __u32 snr_y_flt3_idx;
  __u32 snr_y_flt2_idx;
  __u32 snr_y_flt1_idx;
  __u32 snr_eny;
  __u32 snr_enc;
  __u32 snri_x_size;
  __u32 snri_y_size;
  __u32 snri_stride;
  __u32 static_format;
  __u8 snri_alpha_map[APLHA_MAP_SIZE];
};

/*
 * struct mtk_img_uapi_tdshp_param_prot - TDSHP parameters *
 */
struct mtk_img_uapi_tdshp_param_prot {
  __u32 tdshp_demo_en;
  __u32 tdshp_relay_mode;
};

/*
 * struct mtk_img_uapi_tncs_param_prot - TNCS parameters *
 */
struct mtk_img_uapi_tncs_param_prot {
  __u32 tncs_ggm_lnr;
  __u32 tncs_ggm_end_var;
  __u32 tncs_gtms_drzs1n_vert_first;
  __u32 tncs_gtms_slm_drzs1n_vert_first;
  __u32 tncs_bces_drzs1n_vert_first;
  __u32 tncs_itune_gtms_crop_en;
  __u32 tncs_itune_gtms_crop_int_xstart;
  __u32 tncs_itune_gtms_crop_sub_xstart;
  __u32 tncs_itune_gtms_crop_int_ystart;
  __u32 tncs_itune_gtms_crop_sub_ystart;
  __u32 tncs_itune_gtms_crop_wd;
  __u32 tncs_itune_gtms_crop_ht;
  __u32 tncs_itune_gtms_slm_crop_en;
  __u32 tncs_itune_gtms_slm_crop_int_xstart;
  __u32 tncs_itune_gtms_slm_crop_sub_xstart;
  __u32 tncs_itune_gtms_slm_crop_int_ystart;
  __u32 tncs_itune_gtms_slm_crop_sub_ystart;
  __u32 tncs_itune_gtms_slm_crop_wd;
  __u32 tncs_itune_gtms_slm_crop_ht;
};

/*
 * struct mtk_img_uapi_tnc_param_prot - TNC parameters *
 */
struct mtk_img_uapi_tnc_param_prot {
  __u32 tnc_lmc_ink_ch;
  __u32 tnc_lmc_ink_delta_mode;
  __u32 tnc_lmc_w3_ink_en;
  __u32 tnc_lmc_w3_wgt_en;
  __u32 tnc_lmc_w3_en;
  __u32 tnc_lmc_w2_ink_en;
  __u32 tnc_lmc_w2_wgt_en;
  __u32 tnc_lmc_w2_en;
  __u32 tnc_lmc_w1_ink_en;
  __u32 tnc_lmc_w1_wgt_en;
  __u32 tnc_lmc_w1_en;
  __u32 tnc_lsp_sat_src;
  __u32 tnc_lsp_ink_en;
  __u32 tnc_lsp_sat_limit;
  __u32 tnc_lsp_en;
  __u32 tnc_s_g_y_en;
  __u32 tnc_seq_sel;
  __u32 tnc_wide_gamut_en;
  __u32 tnc_all_bypass;
  __u32 tnc_heng_bypass;
  __u32 tnc_seng_bypass;
  __u32 tnc_yeng_bypass;
  __u32 tnc_p2c_bypass;
  __u32 tnc_c2p_bypass;
  __u32 tnc_disp_color_start;
  __u32 tnc_cm_w3_ink_en;
  __u32 tnc_cm_w3_wgt_en;
  __u32 tnc_cm_w3_en;
  __u32 tnc_cm_w2_ink_en;
  __u32 tnc_cm_w2_wgt_en;
  __u32 tnc_cm_w2_en;
  __u32 tnc_cm_w1_ink_en;
  __u32 tnc_cm_w1_wgt_en;
  __u32 tnc_cm_w1_en;
  __u32 tnc_cm_bypass;
  __u32 tnc_c3d_mode_sel;
};

/*
 * struct mtk_img_uapi_urz6t_param_prot - URZ6T parameters *
 */
struct mtk_img_uapi_urz6t_param_prot {
  __u32 urz6t_rsz_c42_interp_en;
  __u32 urz6t_rsz_power_saving;
  __u32 urz6t_rsz_etc_chroma_en;
  __u32 urz6t_rsz_etc_switch_max_min_en;
  __u32 urz6t_rsz_etc_ring_ctrl_en;
  __u32 urz6t_rsz_etc_sim_prot_en;
  __u32 urz6t_rsz_etc_sim_prot_blend_mode;
  __u32 urz6t_rsz_etc_luma_curve_select;
  __u32 urz6t_rsz_etc_chroma_curve_select;
  __u32 urz6t_itune_default_up_table;
};

/* script generation done */

/*
 * Mediatek all camera isp setting
 *  struct mtk_img_uapi_meta_raw_stats_cfg
 *  @meta_size:     To indicate size of metadata buffer.
 *  @wb_enable:     To indicate if wb module should be enabled or not.
 *  @dgn_enable:    To indicate if dgn module should be enabled or not.
 *  @mtk_img_uapi_sensor_param: sensor config of p1 stat
 *  @mtk_img_uapi_crop_param:
 *  @mtk_img_uapi_dgn_param: DGN settings
 *  @mtk_img_uapi_wb_param: WB settings
 */
struct mtk_img_uapi_meta_raw_stats_cfg {
  __u32 meta_size;
  __u8 wb_enable;
  __u8 dgn_enable;

  struct mtk_img_uapi_sensor_param sensor_param;
  struct mtk_img_uapi_dgn_param dgn_param;
  struct mtk_img_uapi_wb_param wb_param;

  struct mtk_img_uapi_prot {
    /**
     * Not open source - Parameters
     */
    /* The following top control are generated by script */
    __u8 dm_t1_tuning_enable;
    __u8 drz8t_p1a_tuning_enable;
    __u8 drz8t_p1b_tuning_enable;
    __u8 drzh2n_t1_tuning_enable;
    __u8 drzh2n_t2_tuning_enable;
    __u8 drzh2n_t3_tuning_enable;
    __u8 drzh2n_t4_tuning_enable;
    __u8 drzh2n_t5_tuning_enable;
    __u8 drzh2n_t6_tuning_enable;
    __u8 drzh2n_t7_tuning_enable;
    __u8 drzh2n_t8_tuning_enable;
    __u8 drzh2n_t9_tuning_enable;
    __u8 drzh2n_d1_tuning_enable;
    __u8 drzs4n_t1_tuning_enable;
    __u8 drzs8t_d1_tuning_enable;
    __u8 drzs8t_t1_tuning_enable;
    __u8 ggm_t1_tuning_enable;
    __u8 rec_d1_tuning_enable;
    __u8 tncs_t1_tuning_enable;
    __u8 tncs_d1_tuning_enable;
    __u8 urz6t_p1a_tuning_enable;
    __u8 urz6t_p1b_tuning_enable;
    // Tile Raw
    __u8 traw_tuning_enable;
      __u8 bpc_t1_enable;
    __u8 c2g_t1_enable;
    __u8 ccm_t1_enable;
    __u8 g2cx_t1_enable;
      __u8 hlr_t1_enable;
    __u8 iggm_t1_enable;
      __u8 lsc_t1_enable;
      __u8 ltm_t1_enable;
      __u8 obc_t1_enable;
    __u8 tcy_t1_enable;
      __u8 tcy_t2_enable;
      __u8 tcy_t3_enable;
      __u8 tcy_t4_enable;
    // Dip Raw
    __u8 dip_tuning_enable;
    __u8 aks_d1_enable;
    __u8 cnr_d1_enable;
    __u8 ee_d1_enable;
    __u8 hpx_d1_enable;
    __u8 hpx_d2_enable;
    __u8 ndg_d1_enable;
    __u8 snrs_d1_enable;
    __u8 snr_d1_enable;
    __u8 tcy_d1_enable;
    __u8 tcy_d2_enable;
    __u8 tncsc_d1_enable;
    __u8 tnc_d1_enable;
    __u8 tnr_d1_enable;
    // PQ-Dip
    __u8 pq_dip_tuning_enable;
    __u8 tcc_p1a_enable;
    __u8 tcc_p1b_enable;
    __u8 tdshp_p1a_enable;
    __u8 tdshp_p1b_enable;
    __u8 me_e1_enable;
    __u8 wpe_e1a_enable;
    __u8 wpe_e1b_enable;
    __u8 wpe_e1c_enable;

    struct mtk_img_uapi_ccm_param_prot ccm_t1_param;
    struct mtk_img_uapi_drzh2n_param_prot drzh2n_t1_param;
    struct mtk_img_uapi_drzh2n_param_prot drzh2n_t2_param;
    struct mtk_img_uapi_drzh2n_param_prot drzh2n_t3_param;
    struct mtk_img_uapi_drzh2n_param_prot drzh2n_t4_param;
    struct mtk_img_uapi_drzh2n_param_prot drzh2n_t5_param;
    struct mtk_img_uapi_drzh2n_param_prot drzh2n_t6_param;
    struct mtk_img_uapi_drzh2n_param_prot drzh2n_t7_param;
    struct mtk_img_uapi_drzh2n_param_prot drzh2n_t8_param;
    struct mtk_img_uapi_drzh2n_param_prot drzh2n_t9_param;
    struct mtk_img_uapi_drzs4n_param_prot drzs4n_t1_param;
    struct mtk_img_uapi_tncs_param_prot tncs_t1_param;
    struct mtk_img_uapi_cnr_param_prot cnr_d1_param;
    struct mtk_img_uapi_drzh2n_param_prot drzh2n_d1_param;
    struct mtk_img_uapi_rec_param_prot rec_d1_param;
    struct mtk_img_uapi_snr_param_prot snr_d1_param;
    struct mtk_img_uapi_tncs_param_prot tncs_d1_param;
    struct mtk_img_uapi_tnc_param_prot tnc_d1_param;
    struct mtk_img_uapi_drz8t_param_prot drz8t_p1a_param;
    struct mtk_img_uapi_drz8t_param_prot drz8t_p1b_param;
    struct mtk_img_uapi_tdshp_param_prot tdshp_p1a_param;
    struct mtk_img_uapi_tdshp_param_prot tdshp_p1b_param;
    struct mtk_img_uapi_urz6t_param_prot urz6t_p1a_param;
    struct mtk_img_uapi_urz6t_param_prot urz6t_p1b_param;
    struct mtk_img_uapi_me_param_prot me_e1_param;
    /* script generation done */

    struct mtk_img_uapi_tnr_param_prot tnr_param;
    struct mtk_img_uapi_bpc_param_prot bpc_param;
    struct mtk_img_uapi_lsc_param_prot lsc_param;
    struct mtk_img_uapi_slk_param_prot slk_param;
    struct mtk_img_uapi_camsys_drzs8t_crop_param_prot drzs8t_crop_param;
    struct mtk_img_uapi_wraping_param_prot wraping_param;
      struct mtk_img_uapi_drzs8t_param_prot drzs8t_t1_param;
    struct mtk_img_uapi_drzs8t_param_prot drzs8t_d1_param;
    struct mtk_img_uapi_pq_serial_num_param_prot pq_serial_num_param;
    struct mtk_img_uapi_wb_param_prot wb_param;

    /* Not open source, direct-written register maps */
    /* The following module stuctures are generated by script */
    // Tile Raw
    struct mtk_img_uapi_regmap_raw_bpc bpc_t1;
    struct mtk_img_uapi_regmap_raw_c2g c2g_t1;
    struct mtk_img_uapi_regmap_raw_ccm ccm_t1;
    struct mtk_img_uapi_regmap_raw_dgn dgn_t1;
    struct mtk_img_uapi_regmap_raw_dm dm_t1;
    struct mtk_img_uapi_regmap_raw_drzh2n drzh2n_t1;
    struct mtk_img_uapi_regmap_raw_drzh2n drzh2n_t2;
    struct mtk_img_uapi_regmap_raw_drzh2n drzh2n_t3;
    struct mtk_img_uapi_regmap_raw_drzh2n drzh2n_t4;
    struct mtk_img_uapi_regmap_raw_drzh2n drzh2n_t5;
    struct mtk_img_uapi_regmap_raw_drzh2n drzh2n_t6;
    struct mtk_img_uapi_regmap_raw_drzh2n drzh2n_t7;
    struct mtk_img_uapi_regmap_raw_drzh2n drzh2n_t8;
    struct mtk_img_uapi_regmap_raw_drzh2n drzh2n_t9;
    struct mtk_img_uapi_regmap_raw_drzs4n drzs4n_t1;
    struct mtk_img_uapi_regmap_raw_drzs8t drzs8t_t1;
    struct mtk_img_uapi_regmap_raw_g2cx g2cx_t1;
    struct mtk_img_uapi_regmap_raw_ggm ggm_t1;
    struct mtk_img_uapi_regmap_raw_hlr hlr_t1;
    struct mtk_img_uapi_regmap_raw_iggm iggm_t1;
    struct mtk_img_uapi_regmap_raw_lsc lsc_t1;
    struct mtk_img_uapi_regmap_raw_ltm ltm_t1;
    struct mtk_img_uapi_regmap_raw_obc obc_t1;
    struct mtk_img_uapi_regmap_raw_tcy tcy_t1;
    struct mtk_img_uapi_regmap_raw_tcy tcy_t2;
    struct mtk_img_uapi_regmap_raw_tcy tcy_t3;
    struct mtk_img_uapi_regmap_raw_tcy tcy_t4;
    struct mtk_img_uapi_regmap_raw_tncs tncs_t1;
    struct mtk_img_uapi_regmap_raw_wb wb_t1;
    // Dip Raw
    struct mtk_img_uapi_regmap_raw_aks aks_d1;
    struct mtk_img_uapi_regmap_raw_cnr cnr_d1;
    struct mtk_img_uapi_regmap_raw_drzh2n drzh2n_d1;
    struct mtk_img_uapi_regmap_raw_drzs8t drzs8t_d1;
    struct mtk_img_uapi_regmap_raw_ee ee_d1;
    struct mtk_img_uapi_regmap_raw_ndg ndg_d1;
    struct mtk_img_uapi_regmap_raw_rec rec_d1;
    struct mtk_img_uapi_regmap_raw_snrs snrs_d1;
    struct mtk_img_uapi_regmap_raw_snr snr_d1;
    struct mtk_img_uapi_regmap_raw_tcy tcy_d1;
    struct mtk_img_uapi_regmap_raw_tcy tcy_d2;
    struct mtk_img_uapi_regmap_raw_tncsc tncsc_d1;
    struct mtk_img_uapi_regmap_raw_tncs tncs_d1;
    struct mtk_img_uapi_regmap_raw_tnc tnc_d1;
    struct mtk_img_uapi_regmap_raw_tnr tnr_d1;
    // PQ-Dip
    struct mtk_img_uapi_regmap_raw_drz8t drz8t_p1a;
    struct mtk_img_uapi_regmap_raw_drz8t drz8t_p1b;
    struct mtk_img_uapi_regmap_raw_tcc tcc_p1a;
    struct mtk_img_uapi_regmap_raw_tcc tcc_p1b;
    struct mtk_img_uapi_regmap_raw_tdshp tdshp_p1a;
    struct mtk_img_uapi_regmap_raw_tdshp tdshp_p1b;
    struct mtk_img_uapi_regmap_raw_urz6t urz6t_p1a;
    struct mtk_img_uapi_regmap_raw_urz6t urz6t_p1b;
    struct mtk_img_uapi_regmap_raw_me me_e1;
    struct mtk_img_uapi_regmap_raw_wpe wpe_e1a;
    struct mtk_img_uapi_regmap_raw_wpe wpe_e1b;
    struct mtk_img_uapi_regmap_raw_wpe wpe_e1c;
      /* script generation done */
  } prot;
};

/*
 **
 * struct mtk_img_uapi_meta_raw_stats_0 - shared statistics buffer
 *
 * @tnc_stats_available:  indicate that tnc_stats is ready or not in this buffer
 * @tnch_stats_available: indicate that the tnch_stats is ready or not in this
 *      buffer
 * @tncb_stats_available: indicate that the tncb_stats is ready or not in this
 *      buffer
 * @tncy_stats_available: indicate that the tncy_stats is ready or not in this
 *      buffer
 * @me_frame_status_available: indicate that the me_frame_status is ready or
 *      not in this buffer
 * @me_feature_match_blocks_available: indicate that the me_feature_match_blocks
 *      is ready or not in this buffer
 * @me_local_motion_info_available: indicate that the me_local_motion_info is
 *      ready or not in this buffer
 * @tnc_stats:  tnc statistics
 * @tnch_stats: tnch statistics
 * @tncb_stats: tncb statistics
 * @tncy_stats: tncy statistics
 * @me_frame_status:  me fst statistics
 * @me_feature_match_blocks: me fmb statistics
 * @me_local_motion_info: me lmi statistics
 *
 * The statistic output in this structure may be pushed to the other
 * driver such as dip.
 */
struct mtk_img_uapi_meta_raw_stats_0 {
  __u8 tnc_t1_stats_available;
  __u8 tnch_t1_stats_available;
  __u8 tncb_t1_stats_available;
  __u8 tncy_t1_stats_available;
  __u8 tnc_d1_stats_available;
  __u8 tnch_d1_stats_available;
  __u8 tncb_d1_stats_available;
  __u8 tncy_d1_stats_available;
  __u8 me_frame_status_available;
  __u8 me_feature_match_blocks_l0_available;
  __u8 me_feature_match_blocks_l1_available;
  __u8 me_local_motion_info_available;

  struct mtk_img_uapi_tnc_stats tnc_stats;
  struct mtk_img_uapi_tnch_stats tnch_stats;
  struct mtk_img_uapi_tncb_stats tncb_stats;
  struct mtk_img_uapi_tncy_stats tncy_stats;
  struct mtk_img_uapi_me_frame_status me_frame_status;
  struct mtk_img_uapi_me_feature_match_blocks me_feature_match_blocks_l0;
  struct mtk_img_uapi_me_feature_match_blocks me_feature_match_blocks_l1;
  struct mtk_img_uapi_me_local_motion_info me_local_motion_info;

  __u8 bces_blk_num_x;
  __u8 bces_blk_num_y;
  __u32 gtms_slm_drzs1n_out_wd;
  __u32 gtms_slm_drzs1n_out_ht;
};

#define MTK_IMG_META_VERSION_MAJOR 2
#define MTK_IMG_META_VERSION_MINOR 0
#define MTK_IMG_META_PLATFORM_NAME "isp71"
#define MTK_IMG_META_CHIP_NAME "mt8188"

#endif  // HW_IMGSTREAM_INC_DRV_COMMON_7_1_MT8188_MTK_IMG_METABUF_H_


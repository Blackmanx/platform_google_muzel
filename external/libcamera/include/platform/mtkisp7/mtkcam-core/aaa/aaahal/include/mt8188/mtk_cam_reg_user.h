/*
 * Copyright (C) 2022 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AAA_AAAHAL_INCLUDE_PRIVATE_MT8188_CAMSYS_MTK_CAM_REG_USER_H_
#define AAA_AAAHAL_INCLUDE_PRIVATE_MT8188_CAMSYS_MTK_CAM_REG_USER_H_

#include <all/cam_raw_register.h>

/**
 * Mediatek camera bpc tuning setting
 */
struct mtk_cam_uapi_regmap_raw_bpc {
    __u32 bpc_func_con;
    __u32 rsv0[49];
};

/**
 * Mediatek camera ccm tuning setting
 */
struct mtk_cam_uapi_regmap_raw_ccm {
    __u32 ccm_cnv_1;
    __u32 ccm_cnv_2;
    __u32 ccm_cnv_3;
    __u32 ccm_cnv_4;
    __u32 ccm_cnv_5;
    __u32 ccm_cnv_6;
};

/**
 * Mediatek camera dm tuning setting
 */
struct mtk_cam_uapi_regmap_raw_dm {
    __u32 rsv0;
    __u32 dm_intp_nat;
    __u32 rsv1[3];
    __u32 dm_sl_ctl;
    __u32 rsv2;
    __u32 dm_nr_str;
    __u32 dm_nr_act;
    __u32 dm_hf_str;
    __u32 dm_hf_act1;
    __u32 dm_hf_act2;
    __u32 dm_clip;
    __u32 rsv3[8];
    __u32 dm_ee;
    __u32 rsv4[4];
};

/**
 * Mediatek camera g2c tuning setting
 */
struct mtk_cam_uapi_regmap_raw_g2c {
    __u32 g2c_conv_0a;
    __u32 g2c_conv_0b;
    __u32 g2c_conv_1a;
    __u32 g2c_conv_1b;
    __u32 g2c_conv_2a;
    __u32 g2c_conv_2b;
};

#define MTK_CAM_UAPI_GGM_LUT (256)
/**
 * Mediatek camera ggm tuning setting
 */
struct mtk_cam_uapi_regmap_raw_ggm {
    __u32 ggm_lut[MTK_CAM_UAPI_GGM_LUT];
    __u32 ggm_ctrl;
};

/**
 * Mediatek camera lsc tuning setting
 */
struct mtk_cam_uapi_regmap_raw_lsc {
    __u32 lsc_ratio;
};

#define MTK_CAM_UAPI_LTM_CURVE_SIZE_2 (1728)
#define MTK_CAM_UAPI_LTM_CLP_SIZE_2 (108)

/**
 * Mediatek camera ltm tuning setting
 */
struct mtk_cam_uapi_regmap_raw_ltm {
    __u32 ltm_ctrl;
    __u32 ltm_blk_num;
    __u32 ltm_max_div;
    __u32 ltm_clip;
    __u32 ltm_cfg;
    __u32 ltm_clip_th;
    __u32 ltm_gain_map;
    __u32 ltm_cvnode_grp0;
    __u32 ltm_cvnode_grp1;
    __u32 ltm_cvnode_grp2;
    __u32 ltm_cvnode_grp3;
    __u32 ltm_cvnode_grp4;
    __u32 ltm_cvnode_grp5;
    __u32 ltm_cvnode_grp6;
    __u32 ltm_cvnode_grp7;
    __u32 ltm_cvnode_grp8;
    __u32 ltm_cvnode_grp9;
    __u32 ltm_cvnode_grp10;
    __u32 ltm_cvnode_grp11;
    __u32 ltm_cvnode_grp12;
    __u32 ltm_cvnode_grp13;
    __u32 ltm_cvnode_grp14;
    __u32 ltm_cvnode_grp15;
    __u32 ltm_cvnode_grp16;
    __u32 ltm_out_str;
    __u32 ltm_act_win_x;
    __u32 ltm_act_win_y;
    __u32 ltmtc_curve[MTK_CAM_UAPI_LTM_CURVE_SIZE_2];
    __u32 ltmtc_clp[MTK_CAM_UAPI_LTM_CLP_SIZE_2];
};

/**
 * Mediatek camera ltms tuning setting
 */
struct mtk_cam_uapi_regmap_raw_ltms {
    __u32 ltms_max_div;
    __u32 ltms_blkhist_lb;
    __u32 ltms_blkhist_mb;
    __u32 ltms_blkhist_ub;
    __u32 ltms_blkhist_int;
    __u32 ltms_clip_th_cal;
    __u32 ltms_clip_th_lb;
    __u32 ltms_clip_th_hb;
    __u32 ltms_glbhist_int;
};

/**
 * Mediatek camera obc tuning setting
 */
struct mtk_cam_uapi_regmap_raw_obc {
    __u32 obc_dbs;
    __u32 obc_gray_bld_0;
    __u32 obc_gray_bld_1;
    __u32 obc_wbg_rb;
    __u32 obc_wbg_g;
    __u32 obc_wbig_rb;
    __u32 obc_wbig_g;
    __u32 obc_obg_rb;
    __u32 obc_obg_g;
    __u32 obc_offset_r;
    __u32 obc_offset_gr;
    __u32 obc_offset_gb;
    __u32 obc_offset_b;
    __u32 rsv1[2];
};

/**
 * Mediatek camera tsfs tuning setting
 */
struct mtk_cam_uapi_regmap_raw_tsfs {
    __u32 tsfs_dgain;
};

/**
 * This regmap is for the following version
 *
 * MTK_CAM_META_VERSION_MAJOR: 1
 * MTK_CAM_META_VERSION_MINOR: 6
 * MTK_CAM_META_PLATFORM_NAME: isp71
 * MTK_CAM_META_CHIP_NAME: mt8188
 */


#endif /* AAA_AAAHAL_INCLUDE_PRIVATE_MT8188_CAMSYS_MTK_CAM_REG_USER_H_ */

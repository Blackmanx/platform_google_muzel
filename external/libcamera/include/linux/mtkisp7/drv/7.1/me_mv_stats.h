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

#ifndef HW_IMGSTREAM_INC_DRV_COMMON_7_1_ME_MV_STATS_H_
#define HW_IMGSTREAM_INC_DRV_COMMON_7_1_ME_MV_STATS_H_


typedef __signed__ char __s8; //TODO:remover later wait P2
typedef __signed__ short __s16; //TODO: remover later wait P2
typedef __signed__ int __s32; //TODO: remover later wait P2
typedef unsigned int __u32; //TODO: remover later wait P2
typedef unsigned short __u16; //TODO: remover later wait P2
typedef unsigned char __u8; //TODO: remover later wait P2


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
struct mtk_img_me_mv_hw_buf {
    __u32 offset;
    __u32 size;
};

/**
 * struct mtk_img_me_rmv_status -
 *
 * @me_fst_buf: The buffer for me frame status statistic hardware output. The buffer size
 *           is defined in MTK_IMG_ME_RMV_BUFFER_SIZE ()
 */
#define MTK_IMG_ME_L0RMV_BUFFER_SIZE (62208)
#define MTK_IMG_ME_L1RMV_BUFFER_SIZE (3888)

struct mtk_img_me_rmv_stats {
    struct mtk_img_me_mv_hw_buf me_rmv_buf;
};

/**
 * struct mtk_img_me_rmv_status -
 *
 * @me_fst_buf: The buffer for me frame status statistic hardware output. The buffer size
 *           is defined in MTK_IMG_ME_RMV_BUFFER_SIZE ()
 */
#define MTK_IMG_ME_L0WMV_BUFFER_SIZE (62208)
#define MTK_IMG_ME_L1WMV_BUFFER_SIZE (3888)

struct mtk_img_me_wmv_stats {
    struct mtk_img_me_mv_hw_buf me_wmv_buf;
};

/*
 **
 * struct mtk_img_me_mv_raw_stats_0 - me mv statistics buffer
 *
 * @me_rmv_available:  indicate that RMV buffer is ready or not in this buffer
 * @me_wmv_available: indicate that the WMV buffer is ready or not in this
 *      buffer
 *
 * @me_frame_status:  me fst statistics
 * @me_feature_match_blocks: me fmb statistics
 * @me_local_motion_info: me lmi statistics
 *
 * The statistic output in this structure may be pushed to the other
 * driver such as dip.
 */
struct mtk_img_me_mv_raw_stats_0 {
    __u8 me_l0_rmv0_available;
    __u8 me_l1_rmv0_available;
    __u8 me_l0_wmv0_available;
    __u8 me_l1_wmv0_available;


    struct mtk_img_me_rmv_stats  l0rmv0_stats;
    struct mtk_img_me_rmv_stats  l1rmv0_stats;
    struct mtk_img_me_wmv_stats  l0wmv0_stats;
    struct mtk_img_me_wmv_stats  l1wmv0_stats;
};

#endif  // HW_IMGSTREAM_INC_DRV_COMMON_7_1_ME_MV_STATS_H_


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

#ifndef HW_IMGSTREAM_INC_DRV_COMMON_7_1_MT8188_MTK_IMG_3A_USER_H_
#define HW_IMGSTREAM_INC_DRV_COMMON_7_1_MT8188_MTK_IMG_3A_USER_H_

#include <linux/types.h>

/*
 * struct mtk_img_uapi_dgn_param
 *
 *  @gain: digital gain to increase image brightness, 1x=1024
 */
struct mtk_img_uapi_dgn_param {
    __u32 gain;
};

/*
 * struct mtk_img_uapi_wb_param
 *
 *  @gain_r: white balance gain of R channel
 *  @gain_g: white balance gain of G channel
 *  @gain_b: white balance gain of B channel
 */
struct mtk_img_uapi_wb_param {
    __u32 gain_r;
    __u32 gain_g;
    __u32 gain_b;
    __u32 clip;
};

/*
 * Mediatek camera sensor tuning setting from userspace
 * All the member of this structure are used to store sensor information
 */
struct mtk_img_uapi_sensor_param {
    __u32 sensor_device;
    __u32 sensor_index;
    __u32 sensor_mode;
    __u32 tg_width;
    __u32 tg_height;
};

/**
 * This regmap is for the following version
 *
 * MTK_IMG_META_VERSION_MAJOR: 2
 * MTK_IMG_META_VERSION_MINOR: 0
 * MTK_IMG_META_PLATFORM_NAME: isp71
 * MTK_IMG_META_CHIP_NAME: mt8188
 */

#endif  // HW_IMGSTREAM_INC_DRV_COMMON_7_1_MT8188_MTK_IMG_3A_USER_H_


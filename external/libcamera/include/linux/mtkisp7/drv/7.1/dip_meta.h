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

#ifndef HW_IMGSTREAM_INC_DRV_COMMON_7_1_DIP_META_H_
#define HW_IMGSTREAM_INC_DRV_COMMON_7_1_DIP_META_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

enum EStreamTag {
  EStreamTag_Normal = 0,
  EStreamTag_Bokeh,
  EStreamTag_FM,
  EStreamTag_total
};

enum Img_Multi_Scale_Ratio {
  Img_Multi_Scale_Down4,
  Img_Multi_Scale_Down2,
  Img_Multi_Scale_Max
};

enum dip_resize_ratio {
  dip_resize_anyratio,
  dip_resize_down4,
  dip_resize_down2,
  dip_resize_down42,
  dip_resiz_max
};

struct dip_ctrl {
  enum EStreamTag streamtag;
  bool is_p_img4o_crop_info_exist;

  struct multiframe_info {
    unsigned int frameidx;
    unsigned int frametotal;
  } multiframe_info;

  struct multiscale_info {
    enum Img_Multi_Scale_Ratio scaleratio;
    unsigned int scaleidx;
    unsigned int scaletotal;
  } multiscale_info;

  struct fm_info {
    int height;
    int width;
    int sr_type;
    int offset_x;
    int offset_y;
    int res_th;
    int sad_th;
    int min_ratio;
  } fm_info;

  struct bokeh_rz {
    int srz_id;
    enum dip_resize_ratio rszratio;
    unsigned int in_w;
    unsigned int in_h;
    unsigned int out_w;
    unsigned int out_h;
    unsigned int crop_x;
    unsigned int crop_y;
    unsigned int crop_float_x;
    unsigned int crop_float_y;
    unsigned int crop_w;
    unsigned int crop_h;
  } bokeh_rz;
  unsigned int costlevel;
};

#endif  // HW_IMGSTREAM_INC_DRV_COMMON_7_1_DIP_META_H_

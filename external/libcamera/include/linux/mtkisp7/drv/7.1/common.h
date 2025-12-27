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

#ifndef HW_IMGSTREAM_INC_DRV_COMMON_7_1_COMMON_H_
#define HW_IMGSTREAM_INC_DRV_COMMON_7_1_COMMON_H_

#include <linux/videodev2.h>

typedef enum {
  SCENE_LIST_PREVIEW = 0,
  // SCENE_LIST_CAPTURE,
  SCENE_LIST_MAX
} SCENE_LIST_ENUM;

typedef enum {
  V4L2_MODE_DESC = 0,  // one dma to one video device and support multi frame
  V4L2_MODE_SIGNLE_DEVICE,  // multple dma to one video device and support multi
                            // frame
  V4L2_MODE_STANDARD        // one dma to one video device
} V4L2_MODE;

typedef enum {
  MEMORY_MODE_NORMAL = 0,
  MEMORY_MODE_SMVR,
  MEMORY_MODE_MAX,
} MEMORY_MODE;
class SceneList {
 public:
  SCENE_LIST_ENUM mScene;
  char mName[32];
  // std::shared_ptr<Scene> mpScene;
};

#define FRAME_SYNC_TOKEN_NUM 800

#define VN_DESC_NUM 360
#define VN_SIGDEV_NUM 24

#define V4L2_BUFFER_SIZE VN_SIGDEV_NUM

// VIDEO_MAX_FRAME, use sigdev number to save memory
#define CTRL_META_MAX_FRAME VN_SIGDEV_NUM

#define FRAME_NUM_IN_ONE_REQUEST TIME_MAX
#define NORM_FRAME_NUM_IN_ONE_REQUEST TMAX

#define TIMEOUT_MS 5000  // MS

#define DESC_USER_CNT_FOR_REGISTER_KVA 1

#define CTRLMETA_USER_CNT_FOR_REGISTER_BUFFER 1
#define DEVICE_TUNING_TIMEOUT_MS 8000  // MS

#define MW_CB_TIME 3000  // US

#define QUEUE_REQUEST_TIMEOUT_US 5000000  // US
const SceneList gSceneList[SCENE_LIST_MAX] = {{SCENE_LIST_PREVIEW, "preview"}};
#if 0
const SceneList gSceneList[SCENE_LIST_MAX] =
{ {SCENE_LIST_PREVIEW ,       "preview", NULL},
  {SCENE_LIST_CAPTURE ,       "capture", NULL}
};
#endif

#endif  // HW_IMGSTREAM_INC_DRV_COMMON_7_1_COMMON_H_

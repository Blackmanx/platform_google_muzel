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

#ifndef HW_IMGSTREAM_INC_DRV_COMMON_7_0_ME_META_H_
#define HW_IMGSTREAM_INC_DRV_COMMON_7_0_ME_META_H_

typedef enum MeMode { EME_MODE_0 = 0, EME_MODE_1, EME_MODE_2 } ME_MODE;
typedef enum MeScenario { EME_MODE_3PASS = 0, EME_MODE_1PASS,
    EME_MODE_BATCH } ME_SCENARIO;

struct me_ctrl_setting {
  ME_MODE me_mode;
};

struct me_ctrl {
  struct me_ctrl_setting me_setting;
  unsigned int BATCH_NUM;
};

#endif  // HW_IMGSTREAM_INC_DRV_COMMON_7_0_ME_META_H_

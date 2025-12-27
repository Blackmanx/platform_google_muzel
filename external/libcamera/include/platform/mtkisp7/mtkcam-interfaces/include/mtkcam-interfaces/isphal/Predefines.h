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

#ifndef INCLUDE_MTKCAM_INTERFACES_ISPHAL_PREDEFINES_H_
#define INCLUDE_MTKCAM_INTERFACES_ISPHAL_PREDEFINES_H_

#ifndef ISP_HAL_CC_LIKELY
#define ISP_HAL_CC_LIKELY(exp) (__builtin_expect(!!(exp), true))
#endif
#ifndef ISP_HAL_CC_UNLIKELY
#define ISP_HAL_CC_UNLIKELY(exp) (__builtin_expect(!!(exp), false))
#endif

#ifndef MTK_ISPHAL_ASSERT
#define MTK_ISPHAL_ASSERT(cond)         \
  do {                                  \
    if (!(cond)) {                      \
      *(volatile int*)(0x0) = 0xdead12; \
    }                                   \
  } while (0)
#endif

#ifndef MTKISPHAL_OPTIMIZE_LEVEL
#define MTKISPHAL_OPTIMIZE_LEVEL 0
#endif

/** structure align macros */
#ifndef MTK_ISPHAL_ALIGN
#define MTK_ISPHAL_ALIGN(x) __attribute__((aligned(x)))
#define MTK_ISPHAL_ALIGN_DEFAULT MTK_ISPHAL_ALIGN(4)
#endif

#endif  // INCLUDE_MTKCAM_INTERFACES_ISPHAL_PREDEFINES_H_

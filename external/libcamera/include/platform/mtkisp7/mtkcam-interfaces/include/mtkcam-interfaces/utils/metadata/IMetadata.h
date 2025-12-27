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

#ifndef INCLUDE_MTKCAM_INTERFACES_UTILS_METADATA_IMETADATA_H_
#define INCLUDE_MTKCAM_INTERFACES_UTILS_METADATA_IMETADATA_H_

#include <string>

#include "mtkcam-halif/utils/metadata/1.x/IMetadata.h"

/******************************************************************************
 * Log control
 ******************************************************************************/

#define META_LOG_MODULE_ID NSCam::Utils::ULog::MOD_METADATA


#define META_LOGE(fmt, arg...) \
  CAM_ULOGE(META_LOG_MODULE_ID, "%s(%d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define META_LOGW(fmt, arg...) \
  CAM_ULOGW(META_LOG_MODULE_ID, "%s(%d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define META_LOGI(fmt, arg...) \
  CAM_ULOGI(META_LOG_MODULE_ID, "%s(%d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define META_LOGD(fmt, arg...) \
  CAM_ULOGD(META_LOG_MODULE_ID, "%s(%d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define META_LOGV(fmt, arg...) \
  CAM_ULOGV(META_LOG_MODULE_ID, "%s(%d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define META_ASSERTV(cond, fmt, arg...)                                  \
  CAM_ULOG_ASSERT(META_LOG_MODULE_ID, cond, "%s(%d):" fmt, __FUNCTION__, \
                  __LINE__, ##arg)
#define META_ASSERT(cond, fmt, arg...)                                   \
  CAM_ULOG_ASSERT(META_LOG_MODULE_ID, cond, "%s(%d):" fmt, __FUNCTION__, \
                  __LINE__, ##arg)
#define META_FATAL(fmt, arg...)                                             \
  CAM_ULOG_FATAL(META_LOG_MODULE_ID, "%s(%d):" fmt, __FUNCTION__, __LINE__, \
                 ##arg)

#ifndef META_TRACE
    #define META_TRACE 0
#endif
#if META_TRACE
#define TRACE_FUNC(fmt, arg...) META_LOGD(fmt, ##arg)
#else
#define TRACE_FUNC(fmt, arg...)
#endif

#define META_LOGE_CALLSTACK(fmt, arg...)                                   \
  do {                                                                     \
    if (Utils::ULog::isULogDetailsEnabled(Utils::ULog::DETAILS_ERROR)) {   \
      std::string callstack;                                               \
      NSCam::Utils::Backtrace::unwindCurThreadBT(&callstack);              \
      CAM_ULOGE(META_LOG_MODULE_ID, "%s(%d):" fmt, __FUNCTION__, __LINE__, \
                ##arg);                                                    \
      CAM_ULOGE(META_LOG_MODULE_ID, "Callstack:%s", callstack.c_str());    \
    }                                                                      \
  } while (0)

#define META_LOGV_CALLSTACK(fmt, arg...)                                   \
  do {                                                                     \
    if (Utils::ULog::isULogDetailsEnabled(Utils::ULog::DETAILS_VERBOSE)) { \
      std::string callstack;                                               \
      NSCam::Utils::Backtrace::unwindCurThreadBT(&callstack);              \
      CAM_ULOGV(META_LOG_MODULE_ID, "%s(%d):" fmt, __FUNCTION__, __LINE__, \
                ##arg);                                                    \
      CAM_ULOGV(META_LOG_MODULE_ID, "Callstack:%s", callstack.c_str());    \
    }                                                                      \
  } while (0)

#define META_LOGV_IF(cond, ...) \
  do {                          \
    if ((cond)) {               \
      META_LOGV(__VA_ARGS__);   \
    }                           \
  } while (0)
#define META_LOGD_IF(cond, ...) \
  do {                          \
    if ((cond)) {               \
      META_LOGD(__VA_ARGS__);   \
    }                           \
  } while (0)
#define META_LOGI_IF(cond, ...) \
  do {                          \
    if ((cond)) {               \
      META_LOGI(__VA_ARGS__);   \
    }                           \
  } while (0)
#define META_LOGW_IF(cond, ...) \
  do {                          \
    if ((cond)) {               \
      META_LOGW(__VA_ARGS__);   \
    }                           \
  } while (0)
#define META_LOGE_IF(cond, ...) \
  do {                          \
    if ((cond)) {               \
      META_LOGE(__VA_ARGS__);   \
    }                           \
  } while (0)
#define META_LOGE_CALLSTACK_IF(cond, ...) \
  do {                                    \
    if ((cond)) {                         \
      META_LOGE_CALLSTACK(__VA_ARGS__);   \
    }                                     \
  } while (0)


// For performance
//      user     : empty all verbose/assert/v-callstack macro
//      userdebug: empty all verbose/assert macro
//      eng      : empty all verbose macro
#if defined(METADATA_USER)
#undef META_ASSERT
#undef META_ASSERTV
#undef META_LOGV
#undef META_LOGV_CALLSTACK
#undef META_LOGE_CALLSTACK

#define META_ASSERT(cond, fmt, arg...)
#define META_ASSERTV(cond, fmt, arg...)
#define META_LOGV(fmt, arg...)
#define META_LOGV_CALLSTACK(fmt, arg...)
#define META_LOGE_CALLSTACK(fmt, arg...) \
  CAM_ULOGE(META_LOG_MODULE_ID, "%s(%d):" fmt, __FUNCTION__, __LINE__, ##arg)

#elif defined(METADATA_USERDEBUG)
#undef META_ASSERTV
#define META_ASSERTV(cond, fmt, arg...)
#undef META_LOGV
#define META_LOGV(fmt, arg...)
#else  // ENG / default
#undef META_LOGV
#define META_LOGV(fmt, arg...)
#endif

#endif  //  INCLUDE_MTKCAM_INTERFACES_UTILS_METADATA_IMETADATA_H_

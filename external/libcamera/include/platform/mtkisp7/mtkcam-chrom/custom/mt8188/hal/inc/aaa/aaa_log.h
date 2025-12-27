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


#ifndef _AAA_LOG_H_
#define _AAA_LOG_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

/******************************************************************************
 *
 *  Usage:
 *      [.c/.cpp]
 *          #define LOG_TAG "<your-module-name>"
 *          #include <mtkcam/utils/std/Log.h>
 *
 *      [Android.mk]
 *          LOCAL_WHOLE_STATIC_LIBRARIES += libcameracustom.camera.3a.log
 *          PS:
 *              Only needed in EXECUTABLE and SHARED LIBRARY.
 *              Not needed in STATIC LIBRARY.
 *
 *  Note:
 *      1)  Make sure to define LOG_TAG 'before' including this file.
 *      2)  LOG_TAG should follow the syntax of system property naming.
 *              Allowed:    '0'~'9', 'a'~'z', 'A'~'Z', '.', '-', or '_'
 *              Disallowed: '/'
 *      3)  In your module public API header files,
 *              Do not define LOG_TAG.
 *              Do not include this file.
 *
 ******************************************************************************/
#include <log/log.h>
#ifndef USING_MTK_LDVT
//
#define AAA_LOGV(fmt, arg...)   do{ if(0!=aaa_testLog(LOG_TAG, 'V')) CAM_LOGV(fmt, ##arg); } while(0)
#define AAA_LOGD(fmt, arg...)   do{ if(0!=aaa_testLog(LOG_TAG, 'D')) CAM_LOGD(fmt, ##arg); } while(0)
#define AAA_LOGI(fmt, arg...)   do{ if(0!=aaa_testLog(LOG_TAG, 'I')) CAM_LOGI(fmt, ##arg); } while(0)
#define AAA_LOGW(fmt, arg...)   do{ if(0!=aaa_testLog(LOG_TAG, 'W')) CAM_LOGW(fmt, ##arg); } while(0)
#define AAA_LOGE(fmt, arg...)   do{ if(0!=aaa_testLog(LOG_TAG, 'E')) CAM_LOGE(fmt " (%s){#%d:%s}", ##arg, __FUNCTION__, __LINE__, __FILE__); } while(0)

__BEGIN_DECLS
int aaa_testLog(char const* tag, int prio);
__END_DECLS
//
#else //using LDVT

#ifndef DBG_LOG_TAG
#define DBG_LOG_TAG
#endif

#include <uvvf.h>
#define NEW_LINE_CHAR   "\n"

#define AAA_LOGV(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg) // <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
#define AAA_LOGD(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg) // <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...
#define AAA_LOGI(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg) // <Info>: Show general system information. Like OS version, start/end of Service...
#define AAA_LOGW(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] WARNING: " fmt NEW_LINE_CHAR, __func__, ##arg)    // <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
#define AAA_LOGE(fmt, arg...)        VV_ERRMSG(DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg) // When MP, will only show log of this level. // <Fatal>: Serious error that cause program can not execute. <Error>: Some error that causes some part of the functionality can not operate normally.
#define BASE_LOG_AST(cond, fmt, arg...)     \
        do {        \
            if (!(cond))        \
                VV_ERRMSG("[%s, %s, line%04d] ASSERTION FAILED! : " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg);        \
        } while (0)

#endif

/******************************************************************************
 *
 ******************************************************************************/
#define AAA_LOGV_IF(cond, ...)      do { if ( (cond) ) { AAA_LOGV(__VA_ARGS__); } }while(0)
#define AAA_LOGD_IF(cond, ...)      do { if ( (cond) ) { AAA_LOGD(__VA_ARGS__); } }while(0)
#define AAA_LOGI_IF(cond, ...)      do { if ( (cond) ) { AAA_LOGI(__VA_ARGS__); } }while(0)
#define AAA_LOGW_IF(cond, ...)      do { if ( (cond) ) { AAA_LOGW(__VA_ARGS__); } }while(0)
#define AAA_LOGE_IF(cond, ...)      do { if ( (cond) ) { AAA_LOGE(__VA_ARGS__); } }while(0)


/******************************************************************************
 *   (1) GLOBAL_ENABLE_MY_xxx == 0
 *        --> force to disable.
 *   (2) GLOBAL_ENABLE_MY_xxx == 1
 *        --> ENABLE_MY_xxx in local file decides to enable/disable.
 *       (2.1) ENABLE_MY_xxx in local file == 1 --> enable.
 *       (2.2) ENABLE_MY_xxx in local file == 0 --> disable.
 *       (2.3) ENABLE_MY_xxx in local file undefine
 *              --> ENABLE_MY_xxx in global file decides to enable/disable.
 ******************************************************************************/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Global On/Off
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#undef  GLOBAL_ENABLE_MY_LOG
#define GLOBAL_ENABLE_MY_LOG        (1)

#undef  GLOBAL_ENABLE_MY_ERR
#define GLOBAL_ENABLE_MY_ERR        (1)

#undef  GLOBAL_ENABLE_MY_LOG_OBJ
#define GLOBAL_ENABLE_MY_LOG_OBJ    (1)

#undef  GLOBAL_ENABLE_MY_ASSERT
#define GLOBAL_ENABLE_MY_ASSERT     (1)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Local On/Off
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG           (1)
#endif

#ifndef ENABLE_MY_ERR
    #define ENABLE_MY_ERR           (1)
#endif

#ifndef ENABLE_MY_LOG_OBJ
    #define ENABLE_MY_LOG_OBJ       (1)
#endif

#ifndef ENABLE_MY_ASSERT
    #define ENABLE_MY_ASSERT        (1)
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (GLOBAL_ENABLE_MY_LOG != 0 && ENABLE_MY_LOG != 0)
    #define MY_LOG(fmt, arg...) CAM_LOGD(fmt, ##arg)
#else
    #define MY_LOG(fmt, arg...)
#endif

#define MY_LOG_IF(cond, ...)      do { if ( (cond) ) { MY_LOG(__VA_ARGS__); } }while(0)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (GLOBAL_ENABLE_MY_ERR != 0 && ENABLE_MY_ERR != 0)
    #define MY_ERR(fmt, arg...) CAM_LOGE("[%s()] Err: %5d:, " fmt, __FUNCTION__, __LINE__, ##arg)
#else
    #define MY_ERR(fmt, arg...)
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (GLOBAL_ENABLE_MY_LOG != 0 && ENABLE_MY_LOG != 0)
    #define MY_LOGW(fmt, arg...) CAM_LOGW(fmt, ##arg)
#else
    #define MY_LOGW(fmt, arg...)
#endif

#define MY_LOGW_IF(cond, ...)      do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (GLOBAL_ENABLE_MY_LOG != 0 && ENABLE_MY_LOG != 0)
    #define MY_LOGE(fmt, arg...) CAM_LOGE(fmt, ##arg)
#else
    #define MY_LOGE(fmt, arg...)
#endif

#define MY_LOGE_IF(cond, ...)      do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (GLOBAL_ENABLE_MY_ASSERT != 0 && ENABLE_MY_ASSERT != 0)
    #define MY_ASSERT(x, str)\
        if (x) {} \
        else   {  \
            MY_ERR("[Assert %s, %d]: %s", __FILE__, __LINE__, str); while(1); \
        }
#else
    #define MY_ASSERT(x, str)
#endif

#endif // _AAA_LOG_H_


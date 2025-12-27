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

#ifndef _KAL_NON_SPECIFIC_GENERAL_TYPES_H
#define _KAL_NON_SPECIFIC_GENERAL_TYPES_H

#include <stdlib.h>

/*******************************************************************************
 * Type Definitions
 *******************************************************************************/
typedef unsigned char           kal_uint8;
typedef signed char             kal_int8;
typedef char                    kal_char;
typedef unsigned short          kal_wchar;

typedef unsigned short int      kal_uint16;
typedef signed short int        kal_int16;

typedef unsigned int            kal_uint32;
typedef signed int              kal_int32;


typedef enum
{
  KAL_FALSE,
  KAL_TRUE
} kal_bool;

#endif  /* _KAL_NON_SPECIFIC_GENERAL_TYPES_H */

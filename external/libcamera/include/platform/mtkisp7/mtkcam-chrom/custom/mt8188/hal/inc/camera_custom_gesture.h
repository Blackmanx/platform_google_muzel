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
#ifndef _GS_CONFIG_H
#define _GS_CONFIG_H

typedef struct
{
    unsigned char GDLevel;
}GS_Customize_PARA;


void get_gesture_CustomizeData(GS_Customize_PARA  *GSDataOut);
	
#endif /* _FD_CONFIG_H */


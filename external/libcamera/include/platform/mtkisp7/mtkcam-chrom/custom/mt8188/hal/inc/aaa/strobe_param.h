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

#ifndef __STROBE_PARAM_H__
#define __STROBE_PARAM_H__

/* strobe devie */
typedef enum {
	STROBE_DEVICE_NONE = 0,
	STROBE_DEVICE_FLASHLIGHT,   /** MTK flashlight sub-system */
	STROBE_DEVICE_LED,          /** Linux kernel standard LED sub-system */
	STROBE_DEVICE_XENON,        /** Xenon */
	STROBE_DEVICE_DISPLAY,      /** Display Flash */
	STROBE_DEVICE_IR,           /** IR */
	STROBE_DEVICE_V4L2,         /** V4L2 */
} STROBE_DEVICE_ENUM;

/* strobe type */
typedef enum {
	STROBE_TYPE_NONE = 0,
	STROBE_TYPE_REAR = 1,
	STROBE_TYPE_FRONT = 2,
} STROBE_TYPE_ENUM;

#endif  /* __STROBE_PARAM_H__ */


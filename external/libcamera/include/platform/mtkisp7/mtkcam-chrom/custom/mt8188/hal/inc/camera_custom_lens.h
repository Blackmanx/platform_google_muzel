// SPDX-License-Identifier: MediaTekProprietary
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


#ifndef __CAMERA_CUSTOM_LENS_H_
#define __CAMERA_CUSTOM_LENS_H_

#if 1 // V4L2

#define DUMMY_OIS_ID               0x0

#define MAX_NUM_OF_SUPPORT_VCM     16

//#define DUMMY_SENSOR_ID            0x0
//#define DUMMY_MODULE_ID            0x0
#define DUMMY_VCM_ID               0x0

/* VCM ID */
#define   LC898229AF_VCM_ID       0x8229
#define BU64253GWZAF_VCM_ID       0x4253
#define    DW9718SAF_VCM_ID       0x9718
#define    DW9800WAF_VCM_ID       0x9800
#define     GT9764AF_VCM_ID       0x9764
#define     GT9768AF_VCM_ID       0x9768
#define     GT9772AF_VCM_ID       0x9772
#define    GT9772AAF_VCM_ID       0x9772
#define    GT9772BAF_VCM_ID       0x9773
#define    AK7375CAF_VCM_ID       0x7375
#define    AK7377AAF_VCM_ID       0x7377

#define MAX_NUM_OF_SUPPORT_IRIS   2

/* IRIS ID */
#define DUMMY_IRIS_ID             0x0
#define ICS_IRIS_ID               0x3915

#define MAX_NUM_OF_SUPPORT_IRCUT   2

/* IRCUT ID */
#define DUMMY_IRCUT_ID             0x0
#define AP1511_IRCUT_ID               0x3919


#define MAX_NUM_OF_SUPPORT_OZOOM     2

/* OZOOM MODULE ID*/
/* OZOOM SENSOR ID*/
#define DUMMY_OZOOM_ID          0x0
#define EG3Z3915TCS_OZOOM_ID    0x3915


int GetVcmDriverName(int sensor_index, int sensor_id, int module_id, int *o_vcm_id, char *o_vcm_driver_name);
int GetOisDriverName(int sensor_index, int sensor_id, int module_id, int *o_vcm_id, char *o_vcm_driver_name);
int GetIrisDriverName(int sensor_index, int sensor_id, int module_id, int *o_iris_id, char *o_iris_driver_name);
int GetIrcutDriverName(int sensor_index, int sensor_id, int module_id, int *o_ircut_id, char *o_ircut_driver_name);
int GetOzoomProfile(int sensor_index, int sensor_id, int module_id, int *o_ozoom_id, char *o_ozoom_driver_name, int *o_zoom_profile_buf, int *o_focus_profile_buf, int o_profile_size);

#endif

#define MAX_NUM_OF_SUPPORT_LENS                 16

#define DUMMY_SENSOR_ID                      0xFFFF

#define DUMMY_MODULE_ID                      0x0
#define DW9839AF_MODULE_ID                   0x0001  //TBD
#define LC898229AF_MODULE_ID                 0x0700
#define GT9772BAF_MODULE_ID                  0x1000  //TBD
#define BU64253GWZAF_MODULE_ID               0x0005
/* LENS ID */
#define DUMMY_LENS_ID                        0xFFFF
#define FM50AF_LENS_ID                       0x0001
#define MT9P017AF_LENS_ID                    0x0002

#define SENSOR_DRIVE_LENS_ID                 0x1000
#define GAF001AF_LENS_ID                     0xFF01
#define GAF002AF_LENS_ID                     0xFF02
#define GAF008AF_LENS_ID                     0xFF08

#define OV8825AF_LENS_ID                     0x0003
#define BU6429AF_LENS_ID                     0x0004
#define BU6424AF_LENS_ID                     0x0005
#define BU64748AF_LENS_ID                    0x6474
#define BU63165AF_LENS_ID                    0x6316
#define BU63169AF_LENS_ID                    0x6369
#define AD5823AF_LENS_ID                     0x5823
#define DW9718AF_LENS_ID                     0x9718
#define DW9718SAF_LENS_ID                    0x9719
#define DW9800WAF_LENS_ID                    0x9800
#define DW9839AF_LENS_ID                     0x9839
#define AD5820AF_LENS_ID                     0x5820
#define DW9714AF_LENS_ID                     0x9714
#define GT9764AF_LENS_ID                     0x9764
#define GT9772AF_LENS_ID                     0x9772
#define LC898122AF_LENS_ID                   0x9812
#define LC898212AF_LENS_ID                   0x8212
#define LC898212XDAF_LENS_ID                 0x8213
#define LC898214AF_LENS_ID                   0x8214
#define LC898217AF_LENS_ID                   0x8217
#define LC898229AF_LENS_ID                   0x8229
#define BU64745GWZAF_LENS_ID                 0xFCE9
#define BU24253AF_LENS_ID                    0x2425
#define RUMBAAF_LENS_ID                      0x6334
#define AK7348AF_LENS_ID                     0x7348
#define AK7371AF_LENS_ID                     0x7371
#define BU64253GWZAF_LENS_ID                 0x6425
#define DW9718TAF_LENS_ID                    0x9720
/* AF LAMP THRESHOLD*/
#define AF_LAMP_LV_THRES 60


typedef unsigned int(*PFUNC_GETLENSDEFAULT)(void*, unsigned int);

typedef struct
{
    unsigned int SensorId;
    unsigned int ModuleId;
    unsigned int LensId;
    unsigned char  LensDrvName[32];
    unsigned int (*getLensDefault)(void *pDataBuf, unsigned int size);

} MSDK_LENS_INIT_FUNCTION_STRUCT, *PMSDK_LENS_INIT_FUNCTION_STRUCT;


unsigned int GetLensInitFuncList(PMSDK_LENS_INIT_FUNCTION_STRUCT pLensList, unsigned int a_u4CurrSensorDev);


#endif /* __MSDK_LENS_EXP_H */

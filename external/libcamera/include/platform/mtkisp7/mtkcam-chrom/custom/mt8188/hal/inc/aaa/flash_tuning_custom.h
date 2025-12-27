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


#ifndef __FLASH_TUNING_CUSTOM_H__
#define __FLASH_TUNING_CUSTOM_H__

#include "flash_param.h"
#include "strobe_param.h"
#include "camera_custom_nvram.h"
#include <camera_custom_isp_nvram.h>

/* flash_tuning_custom_[main|main2|sub|sub2][_part2].cpp */
void cust_getFlashQuick2CalibrationExp(int sensorDev, int* exp, int* afe, int* isp);
void cust_getFlashQuick2CalibrationExp_main(int* exp, int* afe, int* isp);
void cust_getFlashQuick2CalibrationExp_main_part2(int* exp, int* afe, int* isp);
void cust_getFlashQuick2CalibrationExp_sub(int* exp, int* afe, int* isp);
void cust_getFlashQuick2CalibrationExp_sub_part2(int* exp, int* afe, int* isp);

void cust_getFlashHalTorchDuty(int sensorDev, int* duty, int* dutyLt);
void cust_getFlashHalTorchDuty_main(int* duty, int* dutyLt);
void cust_getFlashHalTorchDuty_main_part2(int* duty, int* dutyLt);
void cust_getFlashHalTorchDuty_sub(int* duty, int* dutyLt);
void cust_getFlashHalTorchDuty_sub_part2(int* duty, int* dutyLt);

typedef int (*FlashIMapFP)(int, int );
FlashIMapFP cust_getFlashIMapFunc(int sensorDev);
FlashIMapFP cust_getFlashIMapFunc_main();
FlashIMapFP cust_getFlashIMapFunc_main_part2();
FlashIMapFP cust_getFlashIMapFunc_sub();
FlashIMapFP cust_getFlashIMapFunc_sub_part2();

void cust_getFlashITab1(int sensorDev, short* ITab1);
void cust_getFlashITab1_main(short* ITab1);
void cust_getFlashITab1_main_part2(short* ITab1);
void cust_getFlashITab1_sub(short* ITab1);
void cust_getFlashITab1_sub_part2(short* ITab1);

void cust_getFlashITab2(int sensorDev, short* ITab2);
void cust_getFlashITab2_main(short* ITab2);
void cust_getFlashITab2_main_part2(short* ITab2);
void cust_getFlashITab2_sub(short* ITab2);
void cust_getFlashITab2_sub_part2(short* ITab2);

void cust_getFlashMaxIDuty(int sensorDev, int dutyNum, int dutyNumLt, int* duty, int* dutyLt);
void cust_getFlashMaxIDuty_main(int dutyNum, int dutyNumLt, int* duty, int* dutyLt);
void cust_getFlashMaxIDuty_main_part2(int dutyNum, int dutyNumLt, int* duty, int* dutyLt);
void cust_getFlashMaxIDuty_sub(int dutyNum, int dutyNumLt, int* duty, int* dutyLt);
void cust_getFlashMaxIDuty_sub_part2(int dutyNum, int dutyNumLt, int* duty, int* dutyLt);

FLASH_PROJECT_PARA& cust_getFlashProjectPara_V3(int sensorDev, int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame); // isForceFlash: 0: auto, 1: forceOn
FLASH_PROJECT_PARA& cust_getFlashProjectPara_main(int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame);
FLASH_PROJECT_PARA& cust_getFlashProjectPara_main_part2(int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame);
FLASH_PROJECT_PARA& cust_getFlashProjectPara_sub(int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame);
FLASH_PROJECT_PARA& cust_getFlashProjectPara_sub_part2(int AEScene, int isForceFlash, NVRAM_CAMERA_STROBE_STRUCT* nvrame);


/* flash_custom.cpp */
int cust_isDualFlashSupport(int sensorDev);
int cust_isFaceFlashSupport(int sensorDev);
void cust_setFlashPartId(int dev, int id);
STROBE_DEVICE_ENUM cust_getStrobeDevice(int sensorDev);
int cust_transformFlashCaliData(int srcSensorDev, int dstSensorDev);
void cust_getEvCompPara(int& maxEvTar10Bit, int& indNum, float*& evIndTab, float*& evTab, float*& evLevel);
/* Panel flash pre-flash and main-flash setting */
void cust_getPanelFlashEnergy(int& m_i4EngPreflash, int& m_i4EngMainflash);

/* flash_custom_v4l2.cpp */
#define FLASH_V4L2_TYPE_MAX 2
#define FLASH_V4L2_CT_MAX 3
#define FLASH_V4L2_PART_MAX 2
#define FLASHLIGHT_NAME_SIZE 64 /* flashlight device name */

int cust_getMaxDuty(int type, int ct, int part);
int cust_getMaxTorchDuty(int type, int ct, int part);
int cust_duty2Current(int type, int ct, int part, int duty);
int cust_getDriverName(int type, int ct, int part, char *driverName);

#endif //#ifndef __FLASH_TUNING_CUSTOM_H__


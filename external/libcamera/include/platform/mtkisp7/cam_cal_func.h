/*
 * Copyright (C) 2023 MediaTek Inc.
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

#include <cstdint>
#include <pthread.h>

#include "linux/mtkisp7/cam_cal_format.h"

typedef unsigned char u8;
typedef uint16_t u16;
typedef unsigned int u32;
typedef uint8_t UINT8;
typedef int8_t INT8;
typedef uint16_t UINT16;
typedef int16_t INT16;
typedef uint32_t UINT32;
typedef int32_t INT32;
typedef uint64_t UINT64;
typedef int64_t INT64;

UINT32 DoCamCalModuleVersion(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData);
UINT32 DoCamCalPartNumber(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData);
UINT32 DoCamCalSingleLsc(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData);
UINT32 DoCamCal2AGain(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData);
UINT32 DoCamCalStereoData(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData);
UINT32 DoCamCalPDAF(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData);
UINT32 DoCamCal_Dump_All(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData);
UINT32 DoCamCalLensId(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData);

/* custom read eeprom function */
UINT32 DoCamCalSingleLscCus(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData);
UINT32 DoCamCalPDAFCus(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData);
UINT32 DoCamCal2AGainCus1339(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData);
UINT32 DoCamCal2AGainCus8A3(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData);
UINT32 DoCamCal2AGainCus(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData);

unsigned int ReadDefault(unsigned int *pGetSensorCalData);

static pthread_mutex_t mEEPROM_Mutex = PTHREAD_MUTEX_INITIALIZER;

enum {
	CALIBRATION_LAYOUT_SENSOR_OTP = 0,
	CALIBRATION_LAYOUT_STEREO_MAIN1,
	CALIBRATION_LAYOUT_FOUR_CELL,
	CALIBRATION_LAYOUT_EXT_OP_1339,
	CALIBRATION_LAYOUT_EXT_OP_8A3,
	CALIBRATION_LAYOUT_EXT_OP,
	MAX_CALIBRATION_LAYOUT_NUM
};

typedef enum // : Muint32_t
{ CAM_CAL_LAYOUT_RTN_PASS = 0x0,
  CAM_CAL_LAYOUT_RTN_FAILED = 0x1,
  CAM_CAL_LAYOUT_RTN_QUEUE = 0x2 } CAM_CAL_LAYOUT_T;

typedef struct {
	uint16_t Include; // calibration layout include this item?
	uint32_t StartAddr; // item Start Address
	uint32_t BlockSize; // BlockSize
	uint32_t (*GetCalDataProcess)(int32_t CamcamFID, uint32_t start_addr,
				      uint32_t BlockSize,
				      uint32_t *pGetSensorCalData);
} CALIBRATION_ITEM_STRUCT;

typedef struct {
	uint32_t HeaderAddr; // Header Address
	uint32_t HeaderId; // Header ID
	uint32_t DataVer; ////new for 658x CAM_CAL_SINGLE_EEPROM_DATA,
		/// CAM_CAL_SINGLE_OTP_DATA
	CALIBRATION_ITEM_STRUCT CalItemTbl[CAMERA_CAM_CAL_DATA_LIST];
} CALIBRATION_LAYOUT_STRUCT;

/*
//Const variable
*/
static UINT16 LayoutType = (MAX_CALIBRATION_LAYOUT_NUM + 1);
static uint16_t rfLayoutType = CALIBRATION_LAYOUT_EXT_OP_1339;
static uint16_t ffLayoutType = CALIBRATION_LAYOUT_EXT_OP_8A3;

//TODO
/*
This code is test under geralt, 
We may need to fix it when running this code in other module (like Ciri)
*/

const CALIBRATION_LAYOUT_STRUCT CalLayoutTbl[MAX_CALIBRATION_LAYOUT_NUM] = {
	{ // CALIBRATION_LAYOUT_SENSOR_OTP
	  0x00001017,
	  0x010b00ff,
	  CAM_CAL_SINGLE_OTP_DATA,
	  { { 0x00000001, 0x00000000, 0x00000000,
	      DoCamCalModuleVersion }, // CAMERA_CAM_CAL_DATA_MODULE_VERSION
	    { 0x00000001, 0x00000005, 0x00000002,
	      DoCamCalPartNumber }, // CAMERA_CAM_CAL_DATA_PART_NUMBER
	    { 0x00000001, 0x00001029, 0x0000074C,
	      DoCamCalSingleLsc }, // CAMERA_CAM_CAL_DATA_SHADING_TABLE
	    { 0x00000001, 0x0000101D, 0x0000000E,
	      DoCamCal2AGain }, // CAMERA_CAM_CAL_DATA_3A_GAIN
	    { 0x00000000, 0x00000763, 0x00000800, DoCamCalPDAF },
	    { 0x00000000, 0x00000FAE, 0x00000550,
	      DoCamCalStereoData }, // CAMERA_CAM_CAL_DATA_STEREO_DATA
	    { 0x00000000, 0x00000000, 0x00001600, DoCamCal_Dump_All },
	    { 0x00000000, 0x00000F80, 0x0000000A, DoCamCalLensId } } },
	{ // CALIBRATION_LAYOUT_STEREO_MAIN1
	  0x00000001,
	  0x030b00ff,
	  CAM_CAL_SINGLE_EEPROM_DATA,
	  { { 0x00000001, 0x00000000, 0x00000000,
	      DoCamCalModuleVersion }, // CAMERA_CAM_CAL_DATA_MODULE_VERSION
	    { 0x00000001, 0x00000005, 0x00000002,
	      DoCamCalPartNumber }, // CAMERA_CAM_CAL_DATA_PART_NUMBER
	    { 0x00000001, 0x00000017, 0x0000074C,
	      DoCamCalSingleLsc }, // CAMERA_CAM_CAL_DATA_SHADING_TABLE
	    { 0x00000001, 0x00000007, 0x0000000E,
	      DoCamCal2AGain }, // CAMERA_CAM_CAL_DATA_3A_GAIN
	    { 0x00000001, 0x00000763, 0x00000800, DoCamCalPDAF },
	    { 0x00000001, 0x00000FAE, 0x00000550,
	      DoCamCalStereoData }, // CAMERA_CAM_CAL_DATA_STEREO_DATA
	    { 0x00000001, 0x00000000, 0x00001600, DoCamCal_Dump_All },
	    { 0x00000001, 0x00000F80, 0x0000000A, DoCamCalLensId } } },
	{ // Four Cell
	  0x00000001,
	  0x050b00ff,
	  CAM_CAL_SINGLE_EEPROM_DATA,
	  { { 0x00000001, 0x00000000, 0x00000000,
	      DoCamCalModuleVersion }, // CAMERA_CAM_CAL_DATA_MODULE_VERSION
	    { 0x00000001, 0x00000005, 0x00000002,
	      DoCamCalPartNumber }, // CAMERA_CAM_CAL_DATA_PART_NUMBER
	    { 0x00000001, 0x00000017, 0x0000074C,
	      DoCamCalSingleLsc }, // CAMERA_CAM_CAL_DATA_SHADING_TABLE
	    { 0x00000001, 0x00000007, 0x0000000E,
	      DoCamCal2AGain }, // CAMERA_CAM_CAL_DATA_3A_GAIN
	    { 0x00000001, 0x00000763, 0x000000C0, DoCamCalPDAF },
	    { 0x00000001, 0x00000FAE, 0x00000550,
	      DoCamCalStereoData }, // CAMERA_CAM_CAL_DATA_STEREO_DATA
	    { 0x00000001, 0x00000000, 0x00001600, DoCamCal_Dump_All },
	    { 0x00000001, 0x00000F80, 0x0000000A, DoCamCalLensId } } },
	{ // OP format 1339
	  0x00001017,
	  0x010b00ff,
	  CAM_CAL_SINGLE_EEPROM_DATA,
	  { { 0x00000000, 0x00000000, 0x00000000,
	      DoCamCalModuleVersion }, // CAMERA_CAM_CAL_DATA_MODULE_VERSION
	    { 0x00000000, 0x00000000, 0x0000000C,
	      DoCamCalPartNumber }, // CAMERA_CAM_CAL_DATA_PART_NUMBER
	    { 0x00000001, 0x00000052, 0x0000074C,
	      DoCamCalSingleLscCus }, // CAMERA_CAM_CAL_DATA_SHADING_TABLE
	    { 0x00000001, 0x00000027, 0x00000028,
	      DoCamCal2AGainCus1339 }, // CAMERA_CAM_CAL_DATA_3A_GAIN
	    { 0x00000001, 0x000007A1, 0x000005DC, DoCamCalPDAFCus },
	    { 0x00000000, 0x00000FAE, 0x00000550,
	      DoCamCalStereoData }, // CAMERA_CAM_CAL_DATA_STEREO_DATA
	    { 0x00000000, 0x00000000, 0x00002488, DoCamCal_Dump_All },
	    { 0x00000000, 0x00000008, 0x00000002, DoCamCalLensId } } },
	{ // OP format 8A3
	  0x00001017,
	  0x010b00ff,
	  CAM_CAL_SINGLE_EEPROM_DATA,
	  { { 0x00000000, 0x00000000, 0x00000000,
	      DoCamCalModuleVersion }, // CAMERA_CAM_CAL_DATA_MODULE_VERSION
	    { 0x00000000, 0x00000000, 0x0000000C,
	      DoCamCalPartNumber }, // CAMERA_CAM_CAL_DATA_PART_NUMBER
	    { 0x00000001, 0x00000026, 0x0000074C,
	      DoCamCalSingleLscCus }, // CAMERA_CAM_CAL_DATA_SHADING_TABLE
	    { 0x00000001, 0x00000014, 0x00000010,
	      DoCamCal2AGainCus8A3 }, // CAMERA_CAM_CAL_DATA_3A_GAIN
	    { 0x00000000, 0x000007A1, 0x000005DC, DoCamCalPDAF },
	    { 0x00000000, 0x00000FAE, 0x00000550,
	      DoCamCalStereoData }, // CAMERA_CAM_CAL_DATA_STEREO_DATA
	    { 0x00000000, 0x00000000, 0x00002488, DoCamCal_Dump_All },
	    { 0x00000000, 0x00000008, 0x00000002, DoCamCalLensId } } },
	{ //OP format
	  0x00001017,
	  0x010b00ff,
	  CAM_CAL_SINGLE_EEPROM_DATA,
	  { { 0x00000000, 0x00000000, 0x00000000,
	      DoCamCalModuleVersion }, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
	    { 0x00000000, 0x00000005, 0x00000002,
	      DoCamCalPartNumber }, //CAMERA_CAM_CAL_DATA_PART_NUMBER
	    { 0x00000001, 0x00000017, 0x0000074C,
	      DoCamCalSingleLscCus }, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
	    { 0x00000001, 0x00000007, 0x0000000E,
	      DoCamCal2AGainCus }, //CAMERA_CAM_CAL_DATA_3A_GAIN
	    { 0x00000000, 0x000007A1, 0x000005DC,
	      DoCamCalPDAF },
	    { 0x00000000, 0x00000FAE, 0x00000550,
	      DoCamCalStereoData }, //CAMERA_CAM_CAL_DATA_STEREO_DATA
	    { 0x00000000, 0x00000000, 0x00002488,
	      DoCamCal_Dump_All },
	    { 0x00000000, 0x00000008, 0x00000002,
	      DoCamCalLensId } } },
};

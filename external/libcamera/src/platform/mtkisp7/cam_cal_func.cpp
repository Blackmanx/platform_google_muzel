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

#include "platform/mtkisp7/cam_cal_func.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "platform/mtkisp7/mtkcam-interfaces/include/mtkcam-interfaces/hw/mem/cam_cal_drv.h"

#define LOG_ERR(fmt, ...) printf((fmt "\n"), ##__VA_ARGS__)
#define LOG_WRN(fmt, ...)
#define LOG_ADBDBG(...)
#define LOG_INF(...)
#define LOG_VRB(...)
#define LOG_DBG(...)
#define LOG_INF_IF(cond, ...)                 \
	do {                                  \
		if ((cond)) {                 \
			LOG_INF(__VA_ARGS__); \
		}                             \
	} while (0)
#define CAM_CAL_ERR_NO_ERR 0x00000000
#define CAM_CAL_ERR_NO_SHADING 0x00000100
#define CAM_CAL_ERR_NO_DEVICE 0x8FFFFFFF

static MINT32 dumpEnable = 0;

UINT32 ShowCmdErrorLog(CAMERA_CAM_CAL_TYPE_ENUM cmd)
{
	LOG_ERR("Return ERROR %s\n", CamCalErrString[cmd]);
	return 0;
}

INT32 getMtkFormatVersion(INT32 CamcamFID, UINT32 *pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

	INT32 ret = 0;
	INT32 ioctlerr;

	lseek(CamcamFID, 0xFA3, SEEK_SET);
	ioctlerr = read(CamcamFID, (u8 *)&ret, 1);
	if (ioctlerr > 0) {
		LOG_ERR("pCamCalData = %p", pCamCalData);
		LOG_ERR("Mtk format version = 0x%x\n", ret);
	} else {
		LOG_ERR("ioctl err \n");
		ret = -1;
	}

	return ret;
}

/*********************************************************************/

UINT32 DoCamCalModuleVersion(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize,
			     UINT32 *pGetSensorCalData)
{
	(void)CamcamFID;
	(void)start_addr;
	(void)BlockSize;
	(void)pGetSensorCalData;

	return 0;
}

UINT32 DoCamCalPartNumber(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
	INT32 ioctlerr;
	UINT32 err = CamCalReturnErr[pCamCalData->Command];
	UINT32 sizeLimit = sizeof(pCamCalData->PartNumber);

	memset(&pCamCalData->PartNumber[0], 0, sizeLimit);

	if (BlockSize > sizeLimit) {
		LOG_ERR("part number size can't larger than %u\n", sizeLimit);
		return err;
	}

	lseek(CamcamFID, start_addr, SEEK_SET);
	ioctlerr = read(CamcamFID, (u8 *)&pCamCalData->PartNumber[0], BlockSize);
	/*ioctlerr = pCamCalHelper->readData(pCamCalData->sensorID, pCamCalData->deviceID,
            start_addr, BlockSize, (u8 *)&pCamCalData->PartNumber[0]);*/
	if (ioctlerr > 0) {
		err = CAM_CAL_ERR_NO_ERR;
	} else {
		LOG_ERR("ioctl err\n");
		ShowCmdErrorLog(pCamCalData->Command);
	}

#ifdef DEBUG_CALIBRATION_LOAD
	LOG_ERR("======================Part Number==================\n");
	LOG_ERR("[Part Number] = %x %x %x %x\n",
		pCamCalData->PartNumber[0], pCamCalData->PartNumber[1],
		pCamCalData->PartNumber[2], pCamCalData->PartNumber[3]);
	LOG_ERR("[Part Number] = %x %x %x %x\n",
		pCamCalData->PartNumber[4], pCamCalData->PartNumber[5],
		pCamCalData->PartNumber[6], pCamCalData->PartNumber[7]);
	LOG_ERR("[Part Number] = %x %x %x %x\n",
		pCamCalData->PartNumber[8], pCamCalData->PartNumber[9],
		pCamCalData->PartNumber[10], pCamCalData->PartNumber[11]);
	LOG_ERR("======================Part Number==================\n");
#endif
	return err;
}

/***********************************************************************************
    Function : To read 2A infomation. Please put your AWB+AF data funtion, here.
************************************************************************************/
UINT32 DoCamCal2AGainCus1339(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
	INT32 retval;
	UINT32 err = CamCalReturnErr[pCamCalData->Command];
	UINT64 CalGain, FacGain;
	u16 AFInf, AFMacro;

	int tempMax = 0;
	int CalR = 1, CalGr = 1, CalGb = 1, CalG = 1, CalB = 1, FacR = 1, FacGr = 1, FacGb = 1, FacG = 1, FacB = 1;

	LOG_ERR("DoCamCal2AGain is enter..BlockSize=%d SensorID=%x\n", BlockSize, pCamCalData->sensorID);

	//To set init value
	memset((void *)&pCamCalData->Single2A, 0, sizeof(CAM_CAL_SINGLE_2A_STRUCT));

	if (pCamCalData->DataVer >= CAM_CAL_TYPE_NUM) {
		err = CAM_CAL_ERR_NO_DEVICE;
		LOG_ERR("DataVer err\n");
		ShowCmdErrorLog(pCamCalData->Command);
	} else if (pCamCalData->DataVer < CAM_CAL_TYPE_NUM) {
		/* AWB Unit Gain */
		LOG_ERR("AWB unit gain offset=%d\n", start_addr + 26);
		lseek(CamcamFID, start_addr + 26, SEEK_SET);
		retval = read(CamcamFID, (u8 *)&CalGain, 8);
		LOG_ERR("Read CalGain OK %x\n", retval);
		if (retval > 0) {
			CalR = ((CalGain & 0xFF) << 8) | ((CalGain >> 8) & 0xFF);
			CalGr = (((CalGain >> 16) & 0xFF) << 8) | ((CalGain >> 24) & 0xFF);
			CalGb = (((CalGain >> 32) & 0xFF) << 8) | ((CalGain >> 40) & 0xFF);
			CalG = ((CalGr + CalGb) + 1) >> 1;
			CalB = (((CalGain >> 48) & 0xFF) << 8) | ((CalGain >> 56) & 0xFF);

			if (CalR > CalG) {
				/* R > G */
				if (CalR > CalB)
					tempMax = CalR;
				else
					tempMax = CalB;
			} else {
				/* G > R */
				if (CalG > CalB)
					tempMax = CalG;
				else
					tempMax = CalB;
			}
			LOG_ERR("UnitR:%d, UnitG:%d, UnitB:%d, New Unit Max=%d", CalR, CalG, CalB, tempMax);

			err = CAM_CAL_ERR_NO_ERR;
		} else {
			LOG_ERR("read AWB unit err\n");
			ShowCmdErrorLog(pCamCalData->Command);
		}

		if (CalGain != 0 &&
		    CalGain != 0xFFFFFFFF &&
		    CalR != 0 &&
		    CalG != 0 &&
		    CalB != 0) {
			pCamCalData->Single2A.S2aAwb.rGainSetNum = 1;
			pCamCalData->Single2A.S2aAwb.rUnitGainu4R = (u32)((tempMax * 512 + (CalR >> 1)) / CalR);
			pCamCalData->Single2A.S2aAwb.rUnitGainu4G = (u32)((tempMax * 512 + (CalG >> 1)) / CalG);
			pCamCalData->Single2A.S2aAwb.rUnitGainu4B = (u32)((tempMax * 512 + (CalB >> 1)) / CalB);
		} else {
			LOG_ERR("There are something wrong on EEPROM, plz contact module vendor Unit R=%d G=%d B=%d!!\n", CalR, CalG, CalB);
		}

		/* AWB Golden Gain */
		LOG_ERR("AWB golden gain offset=%d\n", start_addr + 12);
		lseek(CamcamFID, start_addr + 12, SEEK_SET);
		retval = read(CamcamFID, (u8 *)&FacGain, 8);
		LOG_ERR("Read FacGain OK %x\n", retval);
		if (retval > 0) {
			FacR = ((FacGain & 0xFF) << 8) | ((FacGain >> 8) & 0xFF);
			FacGr = (((FacGain >> 16) & 0xFF) << 8) | ((FacGain >> 24) & 0xFF);
			FacGb = (((FacGain >> 32) & 0xFF) << 8) | ((FacGain >> 40) & 0xFF);
			FacG = ((FacGr + FacGb) + 1) >> 1;
			FacB = (((FacGain >> 48) & 0xFF) << 8) | ((FacGain >> 56) & 0xFF);

			if (FacR > FacG) {
				/* R > G */
				if (FacR > FacB)
					tempMax = FacR;
				else
					tempMax = FacB;
			} else {
				/* G > R */
				if (FacG > FacB)
					tempMax = FacG;
				else
					tempMax = FacB;
			}
			LOG_ERR("GoldenR:%d, GoldenG:%d, GoldenB:%d, New Golden Max=%d", FacR, FacG, FacB, tempMax);

			err = CAM_CAL_ERR_NO_ERR;
		} else {
			LOG_ERR("read AWB golden err\n");
			ShowCmdErrorLog(pCamCalData->Command);
		}

		if (FacGain != 0 &&
		    FacGain != 0xFFFFFFFF &&
		    FacR != 0 &&
		    FacG != 0 &&
		    FacB != 0) {
			pCamCalData->Single2A.S2aAwb.rGoldGainu4R = (u32)((tempMax * 512 + (FacR >> 1)) / FacR);
			pCamCalData->Single2A.S2aAwb.rGoldGainu4G = (u32)((tempMax * 512 + (FacG >> 1)) / FacG);
			pCamCalData->Single2A.S2aAwb.rGoldGainu4B = (u32)((tempMax * 512 + (FacB >> 1)) / FacB);
		} else {
			LOG_ERR("There are something wrong on EEPROM, plz contact module vendor Golden R=%d G=%d B=%d!!\n", FacR, FacG, FacB);
		}

		//Set original data to 3A Layer
		pCamCalData->Single2A.S2aAwb.rValueR = CalR;
		pCamCalData->Single2A.S2aAwb.rValueGr = CalGr;
		pCamCalData->Single2A.S2aAwb.rValueGb = CalGb;
		pCamCalData->Single2A.S2aAwb.rValueB = CalB;
		pCamCalData->Single2A.S2aAwb.rGoldenR = FacR;
		pCamCalData->Single2A.S2aAwb.rGoldenGr = FacGr;
		pCamCalData->Single2A.S2aAwb.rGoldenGb = FacGb;
		pCamCalData->Single2A.S2aAwb.rGoldenB = FacB;
////Only AWB Gain Gathering <////
#ifdef DEBUG_CALIBRATION_LOAD
		LOG_ERR("======================AWB CAM_CAL==================\n");
		LOG_ERR("[CalGain] = 0x%llx\n", CalGain);
		LOG_ERR("[FacGain] = 0x%llx\n", FacGain);
		LOG_ERR("[rCalGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4R);
		LOG_ERR("[rCalGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4G);
		LOG_ERR("[rCalGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
		LOG_ERR("[rFacGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4R);
		LOG_ERR("[rFacGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4G);
		LOG_ERR("[rFacGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4B);
		LOG_ERR("======================AWB CAM_CAL==================\n");
#endif

		/* AF Calibration */
		LOG_ERR("AF Infinity offset=%d\n", start_addr + 1);
		lseek(CamcamFID, start_addr + 1, SEEK_SET);
		retval = read(CamcamFID, (u8 *)&AFInf, 2);
		LOG_ERR("Read AFInf OK %x\n", retval);
		if (retval > 0) {
			err = CAM_CAL_ERR_NO_ERR;
		} else {
			LOG_ERR("read AF Inf err\n");
			ShowCmdErrorLog(pCamCalData->Command);
		}

		LOG_ERR("AF Macro offset=%d\n", start_addr + 3);
		lseek(CamcamFID, start_addr + 3, SEEK_SET);
		retval = read(CamcamFID, (u8 *)&AFMacro, 2);
		LOG_ERR("Read AFMacro OK %x\n", retval);
		if (retval > 0) {
			err = CAM_CAL_ERR_NO_ERR;
		} else {
			LOG_ERR("read AF Macro err\n");
			ShowCmdErrorLog(pCamCalData->Command);
		}

		pCamCalData->Single2A.S2aAf[0] = (((AFInf >> 8) & 0xFF) | ((AFInf & 0xFF) << 8));
		pCamCalData->Single2A.S2aAf[1] = (((AFMacro >> 8) & 0xFF) | ((AFMacro & 0xFF) << 8));

		// custom formula
		pCamCalData->Single2A.S2aAf[0] = pCamCalData->Single2A.S2aAf[0] / 64;
		pCamCalData->Single2A.S2aAf[1] = pCamCalData->Single2A.S2aAf[1] / 64;

////Only AF Gathering <////
#ifdef DEBUG_CALIBRATION_LOAD
		LOG_ERR("======================AF CAM_CAL==================\n");
		LOG_ERR("[AFInf] = 0x%x\n", AFInf);
		LOG_ERR("[AFMacro] = 0x%x\n", AFMacro);
		LOG_ERR("[S2aAf 0] = %d\n", pCamCalData->Single2A.S2aAf[0]);
		LOG_ERR("[S2aAf 1] = %d\n", pCamCalData->Single2A.S2aAf[1]);
		LOG_ERR("======================AF CAM_CAL==================\n");
#endif
	}
	return err;
}

UINT32 DoCamCal2AGainCus8A3(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
	INT32 retval;
	UINT32 err = CamCalReturnErr[pCamCalData->Command];
	UINT64 CalGain, FacGain;

	int tempMax = 0;
	int CalR = 1, CalGr = 1, CalGb = 1, CalG = 1, CalB = 1, FacR = 1, FacGr = 1, FacGb = 1, FacG = 1, FacB = 1;

	LOG_ERR("DoCamCal2AGain is enter..BlockSize=%d SensorID=%x\n", BlockSize, pCamCalData->sensorID);

	//To set init value
	memset((void *)&pCamCalData->Single2A, 0, sizeof(CAM_CAL_SINGLE_2A_STRUCT));

	if (pCamCalData->DataVer >= CAM_CAL_TYPE_NUM) {
		err = CAM_CAL_ERR_NO_DEVICE;
		LOG_ERR("DataVer err\n");
		ShowCmdErrorLog(pCamCalData->Command);
	} else if (pCamCalData->DataVer < CAM_CAL_TYPE_NUM) {
		/* AWB Unit Gain */
		LOG_ERR("AWB unit gain offset=%d\n", start_addr);
		lseek(CamcamFID, start_addr, SEEK_SET);
		retval = read(CamcamFID, (u8 *)&CalGain, 8);
		LOG_ERR("Read CalGain OK %x\n", retval);
		if (retval > 0) {
			CalR = ((CalGain & 0xFF) << 8) | ((CalGain >> 8) & 0xFF);
			CalGr = (((CalGain >> 16) & 0xFF) << 8) | ((CalGain >> 24) & 0xFF);
			CalGb = (((CalGain >> 32) & 0xFF) << 8) | ((CalGain >> 40) & 0xFF);
			CalG = ((CalGr + CalGb) + 1) >> 1;
			CalB = (((CalGain >> 48) & 0xFF) << 8) | ((CalGain >> 56) & 0xFF);

			if (CalR > CalG) {
				/* R > G */
				if (CalR > CalB)
					tempMax = CalR;
				else
					tempMax = CalB;
			} else {
				/* G > R */
				if (CalG > CalB)
					tempMax = CalG;
				else
					tempMax = CalB;
			}
			LOG_ERR("UnitR:%d, UnitG:%d, UnitB:%d, New Unit Max=%d", CalR, CalG, CalB, tempMax);

			err = CAM_CAL_ERR_NO_ERR;
		} else {
			LOG_ERR("read AWB unit err\n");
			ShowCmdErrorLog(pCamCalData->Command);
		}

		if (CalGain != 0 &&
		    CalGain != 0xFFFFFFFF &&
		    CalR != 0 &&
		    CalG != 0 &&
		    CalB != 0) {
			pCamCalData->Single2A.S2aAwb.rGainSetNum = 1;
			pCamCalData->Single2A.S2aAwb.rUnitGainu4R = (u32)((tempMax * 512 + (CalR >> 1)) / CalR);
			pCamCalData->Single2A.S2aAwb.rUnitGainu4G = (u32)((tempMax * 512 + (CalG >> 1)) / CalG);
			pCamCalData->Single2A.S2aAwb.rUnitGainu4B = (u32)((tempMax * 512 + (CalB >> 1)) / CalB);
		} else {
			LOG_ERR("There are something wrong on EEPROM, plz contact module vendor Unit R=%d G=%d B=%d!!\n", CalR, CalG, CalB);
		}

		/* AWB Golden Gain */
		LOG_ERR("AWB golden gain offset=%d\n", start_addr + 8);
		lseek(CamcamFID, start_addr + 8, SEEK_SET);
		retval = read(CamcamFID, (u8 *)&FacGain, 8);
		LOG_ERR("Read FacGain OK %x\n", retval);
		if (retval > 0) {
			FacR = ((FacGain & 0xFF) << 8) | ((FacGain >> 8) & 0xFF);
			FacGr = (((FacGain >> 16) & 0xFF) << 8) | ((FacGain >> 24) & 0xFF);
			FacGb = (((FacGain >> 32) & 0xFF) << 8) | ((FacGain >> 40) & 0xFF);
			FacG = ((FacGr + FacGb) + 1) >> 1;
			FacB = (((FacGain >> 48) & 0xFF) << 8) | ((FacGain >> 56) & 0xFF);

			if (FacR > FacG) {
				/* R > G */
				if (FacR > FacB)
					tempMax = FacR;
				else
					tempMax = FacB;
			} else {
				/* G > R */
				if (FacG > FacB)
					tempMax = FacG;
				else
					tempMax = FacB;
			}
			LOG_ERR("GoldenR:%d, GoldenG:%d, GoldenB:%d, New Golden Max=%d", FacR, FacG, FacB, tempMax);

			err = CAM_CAL_ERR_NO_ERR;
		} else {
			LOG_ERR("read AWB err\n");
			ShowCmdErrorLog(pCamCalData->Command);
		}

		if (FacGain != 0 &&
		    FacGain != 0xFFFFFFFF &&
		    FacR != 0 &&
		    FacG != 0 &&
		    FacB != 0) {
			pCamCalData->Single2A.S2aAwb.rGoldGainu4R = (u32)((tempMax * 512 + (FacR >> 1)) / FacR);
			pCamCalData->Single2A.S2aAwb.rGoldGainu4G = (u32)((tempMax * 512 + (FacG >> 1)) / FacG);
			pCamCalData->Single2A.S2aAwb.rGoldGainu4B = (u32)((tempMax * 512 + (FacB >> 1)) / FacB);
		} else {
			LOG_ERR("There are something wrong on EEPROM, plz contact module vendor Golden R=%d G=%d B=%d!!\n", FacR, FacG, FacB);
		}

		//Set original data to 3A Layer
		pCamCalData->Single2A.S2aAwb.rValueR = CalR;
		pCamCalData->Single2A.S2aAwb.rValueGr = CalGr;
		pCamCalData->Single2A.S2aAwb.rValueGb = CalGb;
		pCamCalData->Single2A.S2aAwb.rValueB = CalB;
		pCamCalData->Single2A.S2aAwb.rGoldenR = FacR;
		pCamCalData->Single2A.S2aAwb.rGoldenGr = FacGr;
		pCamCalData->Single2A.S2aAwb.rGoldenGb = FacGb;
		pCamCalData->Single2A.S2aAwb.rGoldenB = FacB;
////Only AWB Gain Gathering <////
#ifdef DEBUG_CALIBRATION_LOAD
		LOG_ERR("======================AWB CAM_CAL==================\n");
		LOG_ERR("[CalGain] = 0x%llx\n", CalGain);
		LOG_ERR("[FacGain] = 0x%llx\n", FacGain);
		LOG_ERR("[rCalGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4R);
		LOG_ERR("[rCalGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4G);
		LOG_ERR("[rCalGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
		LOG_ERR("[rFacGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4R);
		LOG_ERR("[rFacGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4G);
		LOG_ERR("[rFacGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4B);
		LOG_ERR("======================AWB CAM_CAL==================\n");
#endif
	}
	return err;
}

UINT32 DoCamCal2AGainCus(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    INT32 ioctlerr;
    UINT32 err = CamCalReturnErr[pCamCalData->Command];

    UINT32 CalGain, FacGain;
    INT8 AWBAFConfig;

    u16 AFInf, AFMacro;
    int tempMax = 0;
    int CalR=1, CalGr=1, CalGb=1, CalG=1, CalB=1, FacR=1, FacGr=1, FacGb=1, FacG=1, FacB=1;

    LOG_ERR("DoCamCal2AGainCus is enter..BlockSize=%d SensorID=%x\n", BlockSize, pCamCalData->sensorID);

    //To set init value
    memset((void*)&pCamCalData->Single2A, 0, sizeof(CAM_CAL_SINGLE_2A_STRUCT));

    if(pCamCalData->DataVer >= CAM_CAL_TYPE_NUM)
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        LOG_ERR("ioctl err\n");
        ShowCmdErrorLog(pCamCalData->Command);
    }
    else if(pCamCalData->DataVer < CAM_CAL_TYPE_NUM)
    {
        if(BlockSize!=14)
        {
            LOG_ERR("BlockSize(%d) is not correct (%d)\n",BlockSize,14);
            ShowCmdErrorLog(pCamCalData->Command);
        }
        else
        {
            // Check the config. for AWB & AF
            lseek(CamcamFID, start_addr+1, SEEK_SET);
            ioctlerr = read(CamcamFID, (u8 *)&AWBAFConfig, 1);
            if(ioctlerr>0)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                LOG_ERR("ioctl err\n");
                ShowCmdErrorLog(pCamCalData->Command);
            }

            pCamCalData->Single2A.S2aVer = 0x01;
            pCamCalData->Single2A.S2aBitEn = (0x03 & AWBAFConfig);
            //LOG_INF_IF(dumpEnable,"S2aBitEn=0x%x", pCamCalData->Single2A.S2aBitEn);
            pCamCalData->Single2A.S2aAfBitflagEn = (0x0C & AWBAFConfig);// //Bit: step 0(inf.), 1(marco), 2, 3, 4,5,6,7
            //memset(pCamCalData->Single2A.S2aAf,0x0,sizeof(pCamCalData->Single2A.S2aAf));

            if(0x1&AWBAFConfig){
                ////AWB////
                LOG_INF("AWB offset=%d\n", start_addr + 2);
                lseek(CamcamFID, start_addr + 2, SEEK_SET);
                ioctlerr = read(CamcamFID, (u8 *)&CalGain, 4);
                LOG_INF("Read CalGain OK %x\n", ioctlerr);

                if(ioctlerr>0)
                {
                    // Get min gain
                    CalR  = CalGain&0xFF;
                    CalGr = (CalGain>>8)&0xFF;
                    CalGb = (CalGain>>16)&0xFF;
                    CalG = ((CalGr + CalGb) + 1) >> 1;
                    CalB  = (CalGain>>24)&0xFF;

                    if(CalR > CalG) {
                        /* R > G */
                        if(CalR > CalB)
                            tempMax = CalR;
                        else
                            tempMax = CalB;
                    }
                    else {
                        /* G > R */
                        if(CalG > CalB)
                            tempMax = CalG;
                        else
                            tempMax = CalB;
                    }
                    LOG_INF("UnitR:%d, UnitG:%d, UnitB:%d, New Unit Max=%d", CalR, CalG, CalB, tempMax);

                    err = CAM_CAL_ERR_NO_ERR;

                }
                else
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    LOG_ERR("ioctl err\n");
                    ShowCmdErrorLog(pCamCalData->Command);
                }

                if (CalGain!=0 &&
                    CalGain!=0xFFFFFFFF &&
                    CalR!=0 &&
                    CalG!=0 &&
                    CalB!=0 )
                {
                    pCamCalData->Single2A.S2aAwb.rGainSetNum = 1;
                    pCamCalData->Single2A.S2aAwb.rUnitGainu4R = (u32)((tempMax*512 + (CalR >> 1))/CalR);
                    pCamCalData->Single2A.S2aAwb.rUnitGainu4G = (u32)((tempMax*512 + (CalG >> 1))/CalG);
                    pCamCalData->Single2A.S2aAwb.rUnitGainu4B  = (u32)((tempMax*512 + (CalB >> 1))/CalB);
                }
                else
                {
                    LOG_INF("There are something wrong on EEPROM, plz contact module vendor R=%d G=%d B=%d!!\n", CalR, CalG, CalB);
                }
                lseek(CamcamFID, start_addr + 6, SEEK_SET);
                ioctlerr = read(CamcamFID, (u8 *)&FacGain, 4);
                LOG_INF("Read FacGain OK\n");
                if(ioctlerr>0)
                {
                    // Get min gain
                    FacR  = FacGain&0xFF;
                    FacGr = (FacGain>>8)&0xFF;
                    FacGb = (FacGain>>16)&0xFF;
                    FacG = ((FacGr + FacGb) + 1) >> 1;
                    FacB  = (FacGain>>24)&0xFF;

                    LOG_INF("Extract CalGain OK\n");

                    if(FacR > FacG) {
                        /* R > G */
                        if(FacR > FacB)
                            tempMax = FacR;
                        else
                            tempMax = FacB;
                    }
                    else {
                        /* G > R */
                        if(FacG > FacB)
                            tempMax = FacG;
                        else
                            tempMax = FacB;
                    }

                    LOG_INF("GoldenR:%d, GoldenG:%d, GoldenB:%d, New Golden Max=%d", FacR, FacG, FacB, tempMax);

                    err = CAM_CAL_ERR_NO_ERR;
                }
                else
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    LOG_ERR("ioctl err\n");
                    ShowCmdErrorLog(pCamCalData->Command);
                }
                LOG_INF("Start assign value\n");

               if (FacGain!=0 &&
                    FacGain!=0xFFFFFFFF &&
                    FacR!=0 &&
                    FacG!=0 &&
                    FacB!=0 )
                {
                    pCamCalData->Single2A.S2aAwb.rGoldGainu4R = (u32)((tempMax * 512 + (FacR >> 1)) /FacR);
                    pCamCalData->Single2A.S2aAwb.rGoldGainu4G = (u32)((tempMax * 512 + (FacG >> 1)) /FacG);
                    pCamCalData->Single2A.S2aAwb.rGoldGainu4B  = (u32)((tempMax * 512 + (FacB >> 1)) /FacB);
                }
                else
                {
                    LOG_INF("There are something wrong on EEPROM, plz contact module vendor!! Golden R=%d G=%d B=%d\n", FacR, FacG, FacB);
                }
                //Set original data to 3A Layer
                pCamCalData->Single2A.S2aAwb.rValueR = CalR;
                pCamCalData->Single2A.S2aAwb.rValueGr = CalGr;
                pCamCalData->Single2A.S2aAwb.rValueGb = CalGb;
                pCamCalData->Single2A.S2aAwb.rValueB = CalB;
                pCamCalData->Single2A.S2aAwb.rGoldenR = FacR;
                pCamCalData->Single2A.S2aAwb.rGoldenGr = FacGr;
                pCamCalData->Single2A.S2aAwb.rGoldenGb = FacGb;
                pCamCalData->Single2A.S2aAwb.rGoldenB = FacB;
                ////Only AWB Gain Gathering <////
                #ifdef DEBUG_CALIBRATION_LOAD
                LOG_INF("======================AWB CAM_CAL==================\n");
                LOG_INF("[CalGain] = 0x%x\n", CalGain);
                LOG_INF("[FacGain] = 0x%x\n", FacGain);
                LOG_INF("[rCalGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4R);
                LOG_INF("[rCalGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4G);
                LOG_INF("[rCalGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
                LOG_INF("[rFacGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4R);
                LOG_INF("[rFacGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4G);
                LOG_INF("[rFacGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4B);
                LOG_INF("======================AWB CAM_CAL==================\n");
                #endif
            }
            if(0x2&AWBAFConfig){
                ////AF////
                LOG_INF("AF Infinity offset=%d\n", start_addr + 10);
                lseek(CamcamFID, start_addr + 10, SEEK_SET);
                ioctlerr = read(CamcamFID, (u8 *)&AFInf, 2);
                if(ioctlerr>0)
                {
                    err = CAM_CAL_ERR_NO_ERR;
                    LOG_INF("Read AFInf OK %x\n", ioctlerr);
                }
                else
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    LOG_ERR("ioctl err\n");
                    ShowCmdErrorLog(pCamCalData->Command);
                }

                LOG_INF("AF Macro offset=%d\n", start_addr + 12);
                lseek(CamcamFID, start_addr + 12, SEEK_SET);
                ioctlerr = read(CamcamFID, (u8 *)&AFMacro, 2);
                if(ioctlerr>0)
                {
                    err = CAM_CAL_ERR_NO_ERR;
                    LOG_INF("Read AFMacro OK %x\n",ioctlerr);
                }
                else
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    LOG_ERR("ioctl err\n");
                    ShowCmdErrorLog(pCamCalData->Command);
                }

                pCamCalData->Single2A.S2aAf[0] = (((AFInf>>8)&0xFF) | ((AFInf&0xFF)<<8));
                pCamCalData->Single2A.S2aAf[1] = (((AFMacro>>8)&0xFF) | ((AFMacro&0xFF)<<8));

                ////Only AF Gathering <////
                #ifdef DEBUG_CALIBRATION_LOAD
                LOG_INF("======================AF CAM_CAL==================\n");
                LOG_INF("[AFInf] = 0x%x\n", AFInf);
                LOG_INF("[AFMacro] = 0x%x\n", AFMacro);
                LOG_INF("[S2aAf 0] = %d\n", pCamCalData->Single2A.S2aAf[0]);
                LOG_INF("[S2aAf 1] = %d\n", pCamCalData->Single2A.S2aAf[1]);
                LOG_INF("======================AF CAM_CAL==================\n");
                #endif
            }
        }
    }
    return err;
}

UINT32 DoCamCal2AGain(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
	INT32 ioctlerr;
	UINT32 err = CamCalReturnErr[pCamCalData->Command];

	UINT32 CalGain, FacGain;
	INT8 AWBAFConfig;
	UINT32 moduleId = 0;

	u16 AFInf, AFMacro;
	int tempMax = 0;
	int CalR = 1, CalGr = 1, CalGb = 1, CalG = 1, CalB = 1, FacR = 1, FacGr = 1, FacGb = 1, FacG = 1, FacB = 1;

	LOG_ERR("DoCamCal2AGain is enter..BlockSize=%d SensorID=%x\n", BlockSize, pCamCalData->sensorID);

#ifdef MTK_LOAD_DEBUG
	dumpEnable = 1;
#else
	dumpEnable = 0;
#endif

	memset((void *)&pCamCalData->Single2A, 0, sizeof(CAM_CAL_SINGLE_2A_STRUCT)); //To set init value

	if (pCamCalData->DataVer >= CAM_CAL_TYPE_NUM) {
		err = CAM_CAL_ERR_NO_DEVICE;
		LOG_ERR("ioctl err\n");
		ShowCmdErrorLog(pCamCalData->Command);
	} else if (pCamCalData->DataVer < CAM_CAL_TYPE_NUM) {
		if (BlockSize != 14) {
			LOG_ERR("BlockSize(%d) is not correct (%d)\n", BlockSize, 14);
			ShowCmdErrorLog(pCamCalData->Command);
		} else {
			// Check the config. for AWB & AF
			lseek(CamcamFID, start_addr + 1, SEEK_SET);
			ioctlerr = read(CamcamFID, (u8 *)&AWBAFConfig, 1);
			if (ioctlerr > 0) {
				err = CAM_CAL_ERR_NO_ERR;
			} else {
				pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
				LOG_ERR("ioctl err\n");
				ShowCmdErrorLog(pCamCalData->Command);
			}

			// IMX586 workaround AF enable bit
			lseek(CamcamFID, 0x0005, SEEK_SET);
			ioctlerr = read(CamcamFID, (u8 *)&moduleId, 2);
			if (ioctlerr > 0 && moduleId == 0x0700) {
				LOG_ERR("imx586 AF bit force enable");
				AWBAFConfig |= 0x2; // AF bit enable
			}

			pCamCalData->Single2A.S2aVer = 0x01;
			pCamCalData->Single2A.S2aBitEn = (0x03 & AWBAFConfig);
			//LOG_INF_IF(dumpEnable,"S2aBitEn=0x%x", pCamCalData->Single2A.S2aBitEn);
			if (getMtkFormatVersion(CamcamFID, pGetSensorCalData) >= 0x18)
				if (0x2 & AWBAFConfig)
					pCamCalData->Single2A.S2aAfBitflagEn = 0x0C;
				else
					pCamCalData->Single2A.S2aAfBitflagEn = 0x00;
			else
				pCamCalData->Single2A.S2aAfBitflagEn = (0x0C & AWBAFConfig); // //Bit: step 0(inf.), 1(marco), 2, 3, 4,5,6,7
			//memset(pCamCalData->Single2A.S2aAf,0x0,sizeof(pCamCalData->Single2A.S2aAf));

			if (0x1 & AWBAFConfig) {
				////AWB////
				LOG_INF_IF(dumpEnable, "AWB offset=%d\n", start_addr + 2);
				lseek(CamcamFID, start_addr + 2, SEEK_SET);
				ioctlerr = read(CamcamFID, (u8 *)&CalGain, 4);
				LOG_INF_IF(dumpEnable, "Read CalGain OK %x\n", ioctlerr);

				if (ioctlerr > 0) {
					// Get min gain
					CalR = CalGain & 0xFF;
					CalGr = (CalGain >> 8) & 0xFF;
					CalGb = (CalGain >> 16) & 0xFF;
					CalG = ((CalGr + CalGb) + 1) >> 1;
					CalB = (CalGain >> 24) & 0xFF;

					if (CalR > CalG) {
						/* R > G */
						if (CalR > CalB)
							tempMax = CalR;
						else
							tempMax = CalB;
					} else {
						/* G > R */
						if (CalG > CalB)
							tempMax = CalG;
						else
							tempMax = CalB;
					}
					LOG_INF_IF(dumpEnable, "UnitR:%d, UnitG:%d, UnitB:%d, New Unit Max=%d", CalR, CalG, CalB, tempMax);

					err = CAM_CAL_ERR_NO_ERR;

				} else {
					pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
					LOG_ERR("ioctl err\n");
					ShowCmdErrorLog(pCamCalData->Command);
				}

				if (CalGain != 0 &&
				    CalGain != 0xFFFFFFFF &&
				    CalR != 0 &&
				    CalG != 0 &&
				    CalB != 0) {
					pCamCalData->Single2A.S2aAwb.rGainSetNum = 1;
					pCamCalData->Single2A.S2aAwb.rUnitGainu4R = (u32)((tempMax * 512 + (CalR >> 1)) / CalR);
					pCamCalData->Single2A.S2aAwb.rUnitGainu4G = (u32)((tempMax * 512 + (CalG >> 1)) / CalG);
					pCamCalData->Single2A.S2aAwb.rUnitGainu4B = (u32)((tempMax * 512 + (CalB >> 1)) / CalB);
				} else {
					LOG_ERR("There are something wrong on EEPROM, plz contact module vendor R=%d G=%d B=%d!!\n", CalR, CalG, CalB);
				}
				lseek(CamcamFID, start_addr + 6, SEEK_SET);
				ioctlerr = read(CamcamFID, (u8 *)&FacGain, 4);
				LOG_INF_IF(dumpEnable, "Read FacGain OK\n");
				if (ioctlerr > 0) {
					// Get min gain
					FacR = FacGain & 0xFF;
					FacGr = (FacGain >> 8) & 0xFF;
					FacGb = (FacGain >> 16) & 0xFF;
					FacG = ((FacGr + FacGb) + 1) >> 1;
					FacB = (FacGain >> 24) & 0xFF;

					LOG_INF_IF(dumpEnable, "Extract CalGain OKK\n");

					if (FacR > FacG) {
						/* R > G */
						if (FacR > FacB)
							tempMax = FacR;
						else
							tempMax = FacB;
					} else {
						/* G > R */
						if (FacG > FacB)
							tempMax = FacG;
						else
							tempMax = FacB;
					}

					LOG_INF_IF(dumpEnable, "GoldenR:%d, GoldenG:%d, GoldenB:%d, New Golden Max=%d", FacR, FacG, FacB, tempMax);

					err = CAM_CAL_ERR_NO_ERR;
				} else {
					pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
					LOG_ERR("ioctl err\n");
					ShowCmdErrorLog(pCamCalData->Command);
				}
				LOG_INF_IF(dumpEnable, "Start assign value\n");

				if (FacGain != 0 &&
				    FacGain != 0xFFFFFFFF &&
				    FacR != 0 &&
				    FacG != 0 &&
				    FacB != 0) {
					pCamCalData->Single2A.S2aAwb.rGoldGainu4R = (u32)((tempMax * 512 + (FacR >> 1)) / FacR);
					pCamCalData->Single2A.S2aAwb.rGoldGainu4G = (u32)((tempMax * 512 + (FacG >> 1)) / FacG);
					pCamCalData->Single2A.S2aAwb.rGoldGainu4B = (u32)((tempMax * 512 + (FacB >> 1)) / FacB);
				} else {
					LOG_ERR("There are something wrong on EEPROM, plz contact module vendor!! Golden R=%d G=%d B=%d\n", FacR, FacG, FacB);
				}
				//Set original data to 3A Layer
				pCamCalData->Single2A.S2aAwb.rValueR = CalR;
				pCamCalData->Single2A.S2aAwb.rValueGr = CalGr;
				pCamCalData->Single2A.S2aAwb.rValueGb = CalGb;
				pCamCalData->Single2A.S2aAwb.rValueB = CalB;
				pCamCalData->Single2A.S2aAwb.rGoldenR = FacR;
				pCamCalData->Single2A.S2aAwb.rGoldenGr = FacGr;
				pCamCalData->Single2A.S2aAwb.rGoldenGb = FacGb;
				pCamCalData->Single2A.S2aAwb.rGoldenB = FacB;
////Only AWB Gain Gathering <////
#ifdef DEBUG_CALIBRATION_LOAD
				LOG_ERR("======================AWB CAM_CAL==================\n");
				LOG_ERR("[CalGain] = 0x%x\n", CalGain);
				LOG_ERR("[FacGain] = 0x%x\n", FacGain);
				LOG_ERR("[rCalGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4R);
				LOG_ERR("[rCalGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4G);
				LOG_ERR("[rCalGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4B);
				LOG_ERR("[rFacGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4R);
				LOG_ERR("[rFacGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4G);
				LOG_ERR("[rFacGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4B);
				LOG_ERR("======================AWB CAM_CAL==================\n");
#endif

				if (getMtkFormatVersion(CamcamFID, pGetSensorCalData) >= 0x22) {
					// Low gain
					CalR = CalGr = CalGb = CalG = CalB = FacR = FacGr = FacGb = FacG = FacB = 0;
					tempMax = 0;
					lseek(CamcamFID, 0x178B, SEEK_SET);
					ioctlerr = read(CamcamFID, (u8 *)&CalGain, 4);
					if (ioctlerr > 0) {
						// Get min gain
						CalR = CalGain & 0xFF;
						CalGr = (CalGain >> 8) & 0xFF;
						CalGb = (CalGain >> 16) & 0xFF;
						CalG = ((CalGr + CalGb) + 1) >> 1;
						CalB = (CalGain >> 24) & 0xFF;

						if (CalR > CalG) {
							/* R > G */
							if (CalR > CalB)
								tempMax = CalR;
							else
								tempMax = CalB;
						} else {
							/* G > R */
							if (CalG > CalB)
								tempMax = CalG;
							else
								tempMax = CalB;
						}
						LOG_INF_IF(dumpEnable, "UnitR:%d, UnitG:%d, UnitB:%d, New Unit Max=%d", CalR, CalG, CalB, tempMax);

						err = CAM_CAL_ERR_NO_ERR;

						if (CalGain != 0 &&
						    CalGain != 0xFFFFFFFF &&
						    CalR != 0 &&
						    CalG != 0 &&
						    CalB != 0) {
							pCamCalData->Single2A.S2aAwb.rGainSetNum = 2;
							pCamCalData->Single2A.S2aAwb.rUnitGainu4R_low = (u32)((tempMax * 512 + (CalR >> 1)) / CalR);
							pCamCalData->Single2A.S2aAwb.rUnitGainu4G_low = (u32)((tempMax * 512 + (CalG >> 1)) / CalG);
							pCamCalData->Single2A.S2aAwb.rUnitGainu4B_low = (u32)((tempMax * 512 + (CalB >> 1)) / CalB);
						} else {
							LOG_ERR("There are something wrong on EEPROM, plz contact module vendor R=%d G=%d B=%d!!\n", CalR, CalG, CalB);
						}
					} else {
						pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
						LOG_ERR("ioctl err\n");
						ShowCmdErrorLog(pCamCalData->Command);
					}
					lseek(CamcamFID, 0x178F, SEEK_SET);
					ioctlerr = read(CamcamFID, (u8 *)&FacGain, 4);
					if (ioctlerr > 0) {
						// Get min gain
						FacR = FacGain & 0xFF;
						FacGr = (FacGain >> 8) & 0xFF;
						FacGb = (FacGain >> 16) & 0xFF;
						FacG = ((FacGr + FacGb) + 1) >> 1;
						FacB = (FacGain >> 24) & 0xFF;

						LOG_INF_IF(dumpEnable, "Extract CalGain OKK\n");

						if (FacR > FacG) {
							/* R > G */
							if (FacR > FacB)
								tempMax = FacR;
							else
								tempMax = FacB;
						} else {
							/* G > R */
							if (FacG > FacB)
								tempMax = FacG;
							else
								tempMax = FacB;
						}

						LOG_INF_IF(dumpEnable, "GoldenR:%d, GoldenG:%d, GoldenB:%d, New Golden Max=%d", FacR, FacG, FacB, tempMax);

						err = CAM_CAL_ERR_NO_ERR;

						if (FacGain != 0 &&
						    FacGain != 0xFFFFFFFF &&
						    FacR != 0 &&
						    FacG != 0 &&
						    FacB != 0) {
							pCamCalData->Single2A.S2aAwb.rGoldGainu4R_low = (u32)((tempMax * 512 + (FacR >> 1)) / FacR);
							pCamCalData->Single2A.S2aAwb.rGoldGainu4G_low = (u32)((tempMax * 512 + (FacG >> 1)) / FacG);
							pCamCalData->Single2A.S2aAwb.rGoldGainu4B_low = (u32)((tempMax * 512 + (FacB >> 1)) / FacB);
						} else {
							LOG_ERR("There are something wrong on EEPROM, plz contact module vendor!! Golden R=%d G=%d B=%d\n", FacR, FacG, FacB);
						}
					} else {
						pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
						LOG_ERR("ioctl err\n");
						ShowCmdErrorLog(pCamCalData->Command);
					}

					LOG_ERR("======================AWB CAM_CAL L==================\n");
					LOG_ERR("[CalGain][L] = 0x%x\n", CalGain);
					LOG_ERR("[FacGain][L] = 0x%x\n", FacGain);
					LOG_ERR("[rCalGain.u4R][L] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4R_low);
					LOG_ERR("[rCalGain.u4G][L] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4G_low);
					LOG_ERR("[rCalGain.u4B][L] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4B_low);
					LOG_ERR("[rFacGain.u4R][L] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4R_low);
					LOG_ERR("[rFacGain.u4G][L] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4G_low);
					LOG_ERR("[rFacGain.u4B][L] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4B_low);
					LOG_ERR("======================AWB CAM_CAL L==================\n");

					// Mid gain
					CalR = CalGr = CalGb = CalG = CalB = FacR = FacGr = FacGb = FacG = FacB = 0;
					tempMax = 0;
					lseek(CamcamFID, 0x1793, SEEK_SET);
					ioctlerr = read(CamcamFID, (u8 *)&CalGain, 4);
					if (ioctlerr > 0) {
						// Get min gain
						CalR = CalGain & 0xFF;
						CalGr = (CalGain >> 8) & 0xFF;
						CalGb = (CalGain >> 16) & 0xFF;
						CalG = ((CalGr + CalGb) + 1) >> 1;
						CalB = (CalGain >> 24) & 0xFF;

						if (CalR > CalG) {
							/* R > G */
							if (CalR > CalB)
								tempMax = CalR;
							else
								tempMax = CalB;
						} else {
							/* G > R */
							if (CalG > CalB)
								tempMax = CalG;
							else
								tempMax = CalB;
						}
						LOG_INF_IF(dumpEnable, "UnitR:%d, UnitG:%d, UnitB:%d, New Unit Max=%d", CalR, CalG, CalB, tempMax);

						err = CAM_CAL_ERR_NO_ERR;

						if (CalGain != 0 &&
						    CalGain != 0xFFFFFFFF &&
						    CalR != 0 &&
						    CalG != 0 &&
						    CalB != 0) {
							pCamCalData->Single2A.S2aAwb.rGainSetNum = 3;
							pCamCalData->Single2A.S2aAwb.rUnitGainu4R_mid = (u32)((tempMax * 512 + (CalR >> 1)) / CalR);
							pCamCalData->Single2A.S2aAwb.rUnitGainu4G_mid = (u32)((tempMax * 512 + (CalG >> 1)) / CalG);
							pCamCalData->Single2A.S2aAwb.rUnitGainu4B_mid = (u32)((tempMax * 512 + (CalB >> 1)) / CalB);
						} else {
							LOG_ERR("There are something wrong on EEPROM, plz contact module vendor R=%d G=%d B=%d!!\n", CalR, CalG, CalB);
						}
					} else {
						pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
						LOG_ERR("ioctl err\n");
						ShowCmdErrorLog(pCamCalData->Command);
					}
					lseek(CamcamFID, 0x1797, SEEK_SET);
					ioctlerr = read(CamcamFID, (u8 *)&FacGain, 4);
					if (ioctlerr > 0) {
						// Get min gain
						FacR = FacGain & 0xFF;
						FacGr = (FacGain >> 8) & 0xFF;
						FacGb = (FacGain >> 16) & 0xFF;
						FacG = ((FacGr + FacGb) + 1) >> 1;
						FacB = (FacGain >> 24) & 0xFF;

						LOG_INF_IF(dumpEnable, "Extract CalGain OKK\n");

						if (FacR > FacG) {
							/* R > G */
							if (FacR > FacB)
								tempMax = FacR;
							else
								tempMax = FacB;
						} else {
							/* G > R */
							if (FacG > FacB)
								tempMax = FacG;
							else
								tempMax = FacB;
						}

						LOG_INF_IF(dumpEnable, "GoldenR:%d, GoldenG:%d, GoldenB:%d, New Golden Max=%d", FacR, FacG, FacB, tempMax);

						err = CAM_CAL_ERR_NO_ERR;

						if (FacGain != 0 &&
						    FacGain != 0xFFFFFFFF &&
						    FacR != 0 &&
						    FacG != 0 &&
						    FacB != 0) {
							pCamCalData->Single2A.S2aAwb.rGoldGainu4R_mid = (u32)((tempMax * 512 + (FacR >> 1)) / FacR);
							pCamCalData->Single2A.S2aAwb.rGoldGainu4G_mid = (u32)((tempMax * 512 + (FacG >> 1)) / FacG);
							pCamCalData->Single2A.S2aAwb.rGoldGainu4B_mid = (u32)((tempMax * 512 + (FacB >> 1)) / FacB);
						} else {
							LOG_ERR("There are something wrong on EEPROM, plz contact module vendor!! Golden R=%d G=%d B=%d\n", FacR, FacG, FacB);
						}
					} else {
						pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
						LOG_ERR("ioctl err\n");
						ShowCmdErrorLog(pCamCalData->Command);
					}
				}
			}
			if (0x2 & AWBAFConfig) {
				////AF////
				lseek(CamcamFID, start_addr + 10, SEEK_SET);
				ioctlerr = read(CamcamFID, (u8 *)&AFInf, 2);
				if (ioctlerr > 0) {
					err = CAM_CAL_ERR_NO_ERR;
				} else {
					pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
					LOG_ERR("ioctl err\n");
					ShowCmdErrorLog(pCamCalData->Command);
				}
				lseek(CamcamFID, start_addr + 12, SEEK_SET);
				ioctlerr = read(CamcamFID, (u8 *)&AFMacro, 2);
				if (ioctlerr > 0) {
					err = CAM_CAL_ERR_NO_ERR;
				} else {
					pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
					LOG_ERR("ioctl err\n");
					ShowCmdErrorLog(pCamCalData->Command);
				}

				pCamCalData->Single2A.S2aAf[0] = AFInf;
				pCamCalData->Single2A.S2aAf[1] = AFMacro;

////Only AF Gathering <////
#ifdef DEBUG_CALIBRATION_LOAD
				LOG_ERR("======================AF CAM_CAL==================\n");
				LOG_ERR("[AFInf] = %d\n", AFInf);
				LOG_ERR("[AFMacro] = %d\n", AFMacro);
				LOG_ERR("======================AF CAM_CAL==================\n");
#endif
			}

			if (((getMtkFormatVersion(CamcamFID, pGetSensorCalData) < 0x18) && (0x4 & AWBAFConfig)) ||
			    ((getMtkFormatVersion(CamcamFID, pGetSensorCalData) >= 0x18) && (0x2 & AWBAFConfig))) {
				//load AF addition info
				int EEPROM_Header;
				UINT8 AF_INFO[64];
				unsigned int af_info_offset;

				memset(AF_INFO, 0, 64);
				lseek(CamcamFID, 1, SEEK_SET);
				ioctlerr = read(CamcamFID, (u8 *)&EEPROM_Header, 4);
				if (ioctlerr <= 0) {
					pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
					LOG_ERR("ioctl err\n");
					ShowCmdErrorLog(pCamCalData->Command);
				}
				LOG_INF_IF(dumpEnable, "EEPROM Header = %x\n", EEPROM_Header);

				if (EEPROM_Header == 0x040b00ff) {
					//print main2 AF info, only for EEPROM 0x040b00ff version
					UINT16 AF_Inf_main2 = 0, AF_Marco_main2 = 0;
					lseek(CamcamFID, 0x89a, SEEK_SET);
					ioctlerr = read(CamcamFID, (u8 *)&AF_Inf_main2, 2);
					if (ioctlerr <= 0) {
						pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
						LOG_ERR("ioctl err\n");
						ShowCmdErrorLog(pCamCalData->Command);
					}
					lseek(CamcamFID, 0x89c, SEEK_SET);
					ioctlerr = read(CamcamFID, (u8 *)&AF_Marco_main2, 2);
					if (ioctlerr <= 0) {
						pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
						LOG_ERR("ioctl err\n");
						ShowCmdErrorLog(pCamCalData->Command);
					}
					pCamCalData->Single2A.S2aAf[6] = AF_Inf_main2;
					pCamCalData->Single2A.S2aAf[7] = AF_Marco_main2;
					LOG_INF_IF(dumpEnable, "[AFInf_main2] = %d\n", AF_Inf_main2);
					LOG_INF_IF(dumpEnable, "[AFMacro_main2] = %d\n", AF_Marco_main2);

					af_info_offset = 0x823;
				} else
					af_info_offset = 0xf63;
				LOG_INF_IF(dumpEnable, "af_info_offset = %d\n", af_info_offset);

				lseek(CamcamFID, af_info_offset, SEEK_SET);
				ioctlerr = read(CamcamFID, (u8 *)&AF_INFO, 64);
				if (ioctlerr > 0) {
					err = CAM_CAL_ERR_NO_ERR;
				} else {
					pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
					LOG_ERR("ioctl err\n");
					ShowCmdErrorLog(pCamCalData->Command);
				}
				LOG_INF_IF(dumpEnable, "AF Test = %x %x %x %x\n", AF_INFO[6], AF_INFO[7], AF_INFO[8], AF_INFO[9]);

				pCamCalData->Single2A.S2aAF_t.Close_Loop_AF_Min_Position = (AF_INFO[0] | (AF_INFO[1] << 8));
				pCamCalData->Single2A.S2aAF_t.Close_Loop_AF_Max_Position = (AF_INFO[2] | (AF_INFO[3] << 8));
				pCamCalData->Single2A.S2aAF_t.Close_Loop_AF_Hall_AMP_Offset = AF_INFO[4];
				pCamCalData->Single2A.S2aAF_t.Close_Loop_AF_Hall_AMP_Gain = AF_INFO[5];
				pCamCalData->Single2A.S2aAF_t.AF_infinite_pattern_distance = (AF_INFO[6] | (AF_INFO[7] << 8));
				pCamCalData->Single2A.S2aAF_t.AF_Macro_pattern_distance = (AF_INFO[8] | (AF_INFO[9] << 8));
				pCamCalData->Single2A.S2aAF_t.AF_infinite_calibration_temperature = (AF_INFO[10]);
				if (getMtkFormatVersion(CamcamFID, pGetSensorCalData) >= 0x22) {
					pCamCalData->Single2A.S2aAF_t.AF_macro_calibration_temperature = 0;
					pCamCalData->Single2A.S2aAF_t.AF_dac_code_bit_depth = (AF_INFO[11]);
					pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration_temperature = 0;
				} else {
					pCamCalData->Single2A.S2aAF_t.AF_macro_calibration_temperature = (AF_INFO[11]);
					pCamCalData->Single2A.S2aAF_t.AF_dac_code_bit_depth = 0;
					pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration_temperature = (AF_INFO[20]);
				}

				if (getMtkFormatVersion(CamcamFID, pGetSensorCalData) >= 0x18) {
					pCamCalData->Single2A.S2aAF_t.Posture_AF_infinite_calibration = (AF_INFO[12] | (AF_INFO[13] << 8));
					pCamCalData->Single2A.S2aAF_t.Posture_AF_macro_calibration = (AF_INFO[14] | (AF_INFO[15] << 8));
				}

				pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration = (AF_INFO[18] | (AF_INFO[19] << 8));

				if (getMtkFormatVersion(CamcamFID, pGetSensorCalData) >= 0x22) {
					memset(AF_INFO, 0, 64);
					lseek(CamcamFID, 0x154F, SEEK_SET);
					ioctlerr = read(CamcamFID, (u8 *)&AF_INFO, 41);
					if (ioctlerr >= 0) {
						pCamCalData->Single2A.S2aAF_t.Optical_zoom_cali_num = (AF_INFO[0]);
						memcpy(pCamCalData->Single2A.S2aAF_t.Optical_zoom_AF_cali, &AF_INFO[1], 40);
					}
				}

////AF addition info////
#ifdef DEBUG_CALIBRATION_LOAD
				LOG_ERR("======================AF addition CAM_CAL==================\n");
				LOG_ERR("[AF_infinite_pattern_distance] = %dmm\n", pCamCalData->Single2A.S2aAF_t.AF_infinite_pattern_distance);
				LOG_ERR("[AF_Macro_pattern_distance] = %dmm\n", pCamCalData->Single2A.S2aAF_t.AF_Macro_pattern_distance);
				LOG_ERR("[AF_Middle_calibration] = %d \n", pCamCalData->Single2A.S2aAF_t.AF_Middle_calibration);
				LOG_ERR("======================AF addition CAM_CAL==================\n");
#endif
			}
		}
	}
	return err;
}

/***********************************************************************************

    Function : To read LSC Table

************************************************************************************/
UINT32 DoCamCalSingleLscCus(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

	UINT32 err = CamCalReturnErr[pCamCalData->Command];
	u16 table_size;
	UINT32 retval;

#ifdef MTK_LOAD_DEBUG
	dumpEnable = 1;
#else
	dumpEnable = 0;
#endif

	if (pCamCalData->DataVer >= CAM_CAL_TYPE_NUM) {
		err = CAM_CAL_ERR_NO_DEVICE;
		LOG_ERR("DataVer err\n");
		ShowCmdErrorLog(pCamCalData->Command);
	} else {
		if (BlockSize != CAM_CAL_SINGLE_LSC_SIZE) {
			LOG_INF("BlockSize(%d) is not match (%d)\n", BlockSize, CAM_CAL_SINGLE_LSC_SIZE);
		}
		pCamCalData->SingleLsc.LscTable.MtkLcsData.MtkLscType = 2; //mtk type
		pCamCalData->SingleLsc.LscTable.MtkLcsData.PixId = 8; //hardcode.... need to fix

		table_size = 1868;
		LOG_INF("lsc table_size %d\n", table_size);
		pCamCalData->SingleLsc.LscTable.MtkLcsData.TableSize = table_size;
		if (table_size > 0) {
			pCamCalData->SingleLsc.TableRotation = 0;
			LOG_INF_IF(dumpEnable, "u4Offset=%d u4Length=%d ", start_addr, table_size);

			lseek(CamcamFID, start_addr, SEEK_SET);
			retval = read(CamcamFID, (u8 *)&pCamCalData->SingleLsc.LscTable.MtkLcsData.SlimLscType, table_size);
			if (retval == table_size) {
				err = CAM_CAL_ERR_NO_ERR;
			} else {
				LOG_ERR("read shading err\n");
				err = CamCalReturnErr[pCamCalData->Command];
				ShowCmdErrorLog(pCamCalData->Command);
			}
		}
	}
#ifdef DEBUG_CALIBRATION_LOAD
	LOG_INF("======================SingleLsc Data==================\n");
	LOG_INF("[1st] = %x, %x, %x, %x \n", pCamCalData->SingleLsc.LscTable.MtkLcsData.CapTable[0],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapTable[1],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapTable[2],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapTable[3]);
	LOG_INF("[1st] = SensorLSC(1)?MTKLSC(2)?  %x \n", pCamCalData->SingleLsc.LscTable.MtkLcsData.MtkLscType);
	LOG_INF("CapIspReg =0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[0],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[1],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[2],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[3],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[4]);
	LOG_INF("RETURN = 0x%x \n", err);
	LOG_INF("======================SingleLsc Data==================\n");
#endif
	//    err =  CamCalReturnErr[pCamCalData->Command];  //seanlin121121 wait for OTP put correct sensor LSC data
	return err;
}

UINT32 DoCamCalSingleLsc(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

	INT32 ioctlerr;
	UINT32 err = CamCalReturnErr[pCamCalData->Command];
	u16 table_size;

#ifdef MTK_LOAD_DEBUG
	dumpEnable = 1;
#else
	dumpEnable = 0;
#endif

	if (pCamCalData->DataVer >= CAM_CAL_TYPE_NUM) {
		err = CAM_CAL_ERR_NO_DEVICE;
		LOG_ERR("ioctl err\n");
		ShowCmdErrorLog(pCamCalData->Command);
	} else {
		if (BlockSize != CAM_CAL_SINGLE_LSC_SIZE) {
			LOG_ERR("BlockSize(%d) is not match (%d)\n", BlockSize, CAM_CAL_SINGLE_LSC_SIZE);
		}
		pCamCalData->SingleLsc.LscTable.MtkLcsData.MtkLscType = 2; //mtk type
		pCamCalData->SingleLsc.LscTable.MtkLcsData.PixId = 8; //hardcode.... need to fix

		LOG_INF_IF(dumpEnable, "u4Offset=%d u4Length=%lu", start_addr - 2, sizeof(table_size));
		lseek(CamcamFID, start_addr - 2, SEEK_SET);
		ioctlerr = read(CamcamFID, (u8 *)&table_size, sizeof(table_size));
		if (ioctlerr <= 0) {
			err = CAM_CAL_ERR_NO_SHADING;
		}

		if (pCamCalData->sensorID == 0x0386 || pCamCalData->sensorID == 0x0398) {
			table_size = 1868;
		}
		LOG_ERR("lsc table_size %d\n", table_size);
		pCamCalData->SingleLsc.LscTable.MtkLcsData.TableSize = table_size;
		if (table_size > 0) {
			pCamCalData->SingleLsc.TableRotation = 0;
			LOG_INF_IF(dumpEnable, "u4Offset=%d u4Length=%d ", start_addr, table_size);
			lseek(CamcamFID, start_addr, SEEK_SET);
			ioctlerr = read(CamcamFID, (u8 *)&pCamCalData->SingleLsc.LscTable.MtkLcsData.SlimLscType, table_size);
			if (table_size == ioctlerr) {
				err = CAM_CAL_ERR_NO_ERR;
			} else {
				LOG_ERR("ioctl err\n");
				err = CamCalReturnErr[pCamCalData->Command];
				ShowCmdErrorLog(pCamCalData->Command);
			}
		}
	}
#ifdef DEBUG_CALIBRATION_LOAD
	LOG_ERR("======================SingleLsc Data==================\n");
	LOG_ERR("[1st] = %x, %x, %x, %x \n", pCamCalData->SingleLsc.LscTable.Data[0],
		pCamCalData->SingleLsc.LscTable.Data[1],
		pCamCalData->SingleLsc.LscTable.Data[2],
		pCamCalData->SingleLsc.LscTable.Data[3]);
	LOG_ERR("[1st] = SensorLSC(1)?MTKLSC(2)?  %x \n", pCamCalData->SingleLsc.LscTable.MtkLcsData.MtkLscType);
	LOG_ERR("CapIspReg =0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[0],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[1],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[2],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[3],
		pCamCalData->SingleLsc.LscTable.MtkLcsData.CapIspReg[4]);
	LOG_ERR("RETURN = 0x%x \n", err);
	LOG_ERR("======================SingleLsc Data==================\n");
#endif
	//    err =  CamCalReturnErr[pCamCalData->Command];  //seanlin121121 wait for OTP put correct sensor LSC data
	return err;
}

/******************************************************************************
This function will add after sensor support FOV data
*******************************************************************************/
UINT32 DoCamCalStereoData(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
	INT32 ioctlerr;
	UINT32 err = 0;
	char Stereo_Data[1360];

	LOG_INF_IF(dumpEnable, "DoCamCal_Stereo_Data sensorID = %x\n", pCamCalData->sensorID);
	lseek(CamcamFID, start_addr, SEEK_SET);
	ioctlerr = read(CamcamFID, (u8 *)Stereo_Data, BlockSize);
	if (ioctlerr > 0) {
		err = CAM_CAL_ERR_NO_ERR;
	} else {
		LOG_ERR("ioctl err\n");
		ShowCmdErrorLog(pCamCalData->Command);
	}

#ifdef DEBUG_CALIBRATION_LOAD
	LOG_INF("======================DoCamCal_Stereo_Data==================\n");
	LOG_INF("======================DoCamCal_Stereo_Data==================\n");
#endif
	return err;
}

/******************************************************************************
* Depredicated entry function
*******************************************************************************/
UINT32 CAM_CALGetCalData(UINT32 *pGetSensorCalData)
{
	(void)pGetSensorCalData;
	return 0;
}

UINT32 DoCamCalPDAF(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

	INT32 ioctlerr;
	INT32 err = CamCalReturnErr[pCamCalData->Command];
	{
#ifdef MTK_LOAD_DEBUG
		dumpEnable = 1;
#else
		dumpEnable = 0;
#endif
		pCamCalData->PDAF.Size_of_PDAF = BlockSize;
		LOG_INF_IF(dumpEnable, "PDAF start_addr =%x table_size=%d\n", start_addr, BlockSize);

		lseek(CamcamFID, start_addr, SEEK_SET);
		ioctlerr = read(CamcamFID, (u8 *)&pCamCalData->PDAF.Data[0], BlockSize);
		if (ioctlerr > 0) {
			err = CAM_CAL_ERR_NO_ERR;
		}
	}
#ifdef DEBUG_CALIBRATION_LOAD
	LOG_INF("======================PDAF Data==================\n");
	LOG_INF("First five %x, %x, %x, %x, %x \n", pCamCalData->PDAF.Data[0],
		pCamCalData->PDAF.Data[1],
		pCamCalData->PDAF.Data[2],
		pCamCalData->PDAF.Data[3],
		pCamCalData->PDAF.Data[4]);

	LOG_INF("RETURN = 0x%x \n", err);
	LOG_INF("======================PDAF Data==================\n");
#endif

	return err;
}

UINT32 DoCamCalPDAFCus(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;

	INT32 retval;
	INT32 err = CamCalReturnErr[pCamCalData->Command];
	{
		pCamCalData->PDAF.Size_of_PDAF = BlockSize;
		LOG_INF("PDAF start_addr =%x table_size=%d\n", start_addr, BlockSize);

		lseek(CamcamFID, start_addr, SEEK_SET);
		retval = read(CamcamFID, (u8 *)&pCamCalData->PDAF.Data[0], BlockSize);
		if (retval == (INT32)BlockSize) {
			err = CAM_CAL_ERR_NO_ERR;
		} else {
			LOG_ERR("read err\n");
			err = CamCalReturnErr[pCamCalData->Command];
			ShowCmdErrorLog(pCamCalData->Command);
		}
	}

#ifdef DEBUG_CALIBRATION_LOAD
	LOG_INF("======================PDAF Data==================\n");
	LOG_INF("First five %x, %x, %x, %x, %x \n", pCamCalData->PDAF.Data[0],
		pCamCalData->PDAF.Data[1],
		pCamCalData->PDAF.Data[2],
		pCamCalData->PDAF.Data[3],
		pCamCalData->PDAF.Data[4]);
	LOG_INF("RETURN = 0x%x \n", err);
	LOG_INF("======================PDAF Data==================\n");
#endif

	return err;
}

UINT32 DoCamCalLensId_Base(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
	INT32 ioctlerr;
	UINT32 err = CamCalReturnErr[pCamCalData->Command];
	UINT32 sizeLimit = sizeof(pCamCalData->LensDrvId);

	memset(&pCamCalData->LensDrvId[0], 0, sizeLimit);

	if (BlockSize > sizeLimit) {
		LOG_ERR("lens id size can't larger than %u\n", sizeLimit);
		return err;
	}

	lseek(CamcamFID, start_addr, SEEK_SET);
	ioctlerr = read(CamcamFID, (u8 *)&pCamCalData->LensDrvId[0], BlockSize);
	if (ioctlerr > 0) {
		err = CAM_CAL_ERR_NO_ERR;
	} else {
		LOG_ERR("ioctl err\n");
		ShowCmdErrorLog(pCamCalData->Command);
	}

#ifdef DEBUG_CALIBRATION_LOAD
	LOG_INF("======================Lens Id==================\n");
	LOG_INF("[Lens Id] = %x %x %x %x %x\n",
		pCamCalData->LensDrvId[0], pCamCalData->LensDrvId[1],
		pCamCalData->LensDrvId[2], pCamCalData->LensDrvId[3],
		pCamCalData->LensDrvId[4]);
	LOG_INF("[Lens Id] = %x %x %x %x %x\n",
		pCamCalData->LensDrvId[5], pCamCalData->LensDrvId[6],
		pCamCalData->LensDrvId[7], pCamCalData->LensDrvId[8],
		pCamCalData->LensDrvId[9]);
	LOG_INF("======================Lens Id==================\n");
#endif
	return err;
}

UINT32 DoCamCalLensId(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
	UINT32 err = CamCalReturnErr[pCamCalData->Command];

	if (getMtkFormatVersion(CamcamFID, pGetSensorCalData) >= 0x18) {
		LOG_ERR("No lens id data\n");
		return err;
	}

	return DoCamCalLensId_Base(CamcamFID, start_addr, BlockSize, pGetSensorCalData);
}

UINT32 DoCamCal_Dump_All(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32 *pGetSensorCalData)
{
	PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
	char *info;
	INT32 ioctlerr, ret;
	UINT32 err = CAM_CAL_ERR_NO_DEVICE, idx;
	UINT32 dumpSize = BlockSize;

	// specify dump size if property set
	/*
    int debugDumpSize = property_get_int32("vendor.debug.eeprom.dumpsize", 0);
    if (debugDumpSize > 0) {
        dumpSize = (UINT32) debugDumpSize;
    }*/

	// create folder
	ioctlerr = mkdir("/sdcard/EEPROM", S_IRUSR | S_IWUSR);
	LOG_INF("create folder /sdcard/EEPROM (%d)", ioctlerr);

	// open file
	char targetFile[50];
	ioctlerr = snprintf(targetFile, sizeof(targetFile), "/sdcard/EEPROM/SensorDev%d", pCamCalData->deviceID);
	if (ioctlerr < 0 || ioctlerr > (INT32)sizeof(targetFile)) {
		LOG_INF("generate path fail!");
		return err;
	}

	FILE *fp = fopen(targetFile, "w");

	if (fp == NULL) {
		LOG_INF("open file fail!");
		return err;
	}

	// get data
	info = new char[dumpSize];
	lseek(CamcamFID, start_addr, SEEK_SET);
	ioctlerr = read(CamcamFID, (u8 *)info, dumpSize);
	if (ioctlerr > 0) {
		ret = fprintf(fp, "SensorID=0x%x\n", pCamCalData->sensorID);
		if (ret < 0) {
			ret = fclose(fp);
			if (ret != 0) {
				LOG_ERR("fclose err\n");
			}
			return err;
		}
		for (idx = 0; idx < dumpSize; idx++) {
			ret = fprintf(fp, "0x%04x,0x%02x\n", idx, info[idx]);
			if (ret < 0) {
				ret = fclose(fp);
				if (ret != 0) {
					LOG_ERR("fclose err\n");
				}
				return err;
			}
		}
		err = CAM_CAL_ERR_NO_ERR;
	} else {
		LOG_ERR("ioctl err\n");
		ShowCmdErrorLog(pCamCalData->Command);
		err = CAM_CAL_ERR_DUMP_FAILED;
	}

	// release resource
	delete[] info;
	ret = fclose(fp);
	if (ret != 0) {
		LOG_ERR("fclose err\n");
	}
	return err;
}

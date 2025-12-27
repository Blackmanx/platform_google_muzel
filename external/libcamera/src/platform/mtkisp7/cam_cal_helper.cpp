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

#include "platform/mtkisp7/cam_cal_helper.h"

#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

#include "platform/mtkisp7/cam_cal_func.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/kernel-headers/kd_imgsensor.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/mtkcam-interfaces/hw/mem/cam_cal_drv.h"
#include "platform/mtkisp7/platform_utils.h"

#define LOG_ERR(fmt, ...) printf((fmt "\n"), ##__VA_ARGS__)
#define LOG_WRN(fmt, ...)
#define LOG_ADBDBG(...)
#define LOG_INF(...)
#define LOG_VRB(...)
#define LOG_DBG(...)

constexpr uint32_t MAX_CAL_HELPER_INFO_COUNT = 10;
std::shared_ptr<CamCalHelper> camCalHelpers_[MAX_CAL_HELPER_INFO_COUNT] = {
	nullptr
};

std::shared_ptr<CamCalHelper> CamCalHelper::getInstance(uint32_t sensorIndex)
{
	if (!camCalHelpers_[sensorIndex]) {
		camCalHelpers_[sensorIndex].reset(new CamCalHelper());
	}
	return camCalHelpers_[sensorIndex];
}

int CamCalHelper::get_cal_data(ENUM_CAMERA_CAM_CAL_TYPE_ENUM cal_enum,
			       int sensor_id, int sensor_dev, void *a_pCamCalData)
{
	PCAM_CAL_DATA_STRUCT pCamcalData = &StCamCalCaldata;
	pCamcalData->deviceID = sensor_dev;
	pCamcalData->Command = cal_enum;
	pCamcalData->sensorID = sensor_id;
	get_cam_cal_data(pCamcalData);
	switch (cal_enum) {
	case CAMERA_CAM_CAL_DATA_MODULE_VERSION: {
		PCAM_CAL_MODULE_VERSION_STRUCT pRetData =
			(PCAM_CAL_MODULE_VERSION_STRUCT)a_pCamCalData;
		memcpy(&(pRetData->DataVer), &(pCamcalData->DataVer),
		       sizeof(CAM_CAL_DATA_VER_ENUM));
		return 0;
	} break;
	case CAMERA_CAM_CAL_DATA_PART_NUMBER: {
		PCAM_CAL_PART_NUM_STRUCT pRetData =
			(PCAM_CAL_PART_NUM_STRUCT)a_pCamCalData;
		memcpy(&(pRetData->PartNumber), &(pCamcalData->PartNumber),
		       sizeof(pCamcalData->PartNumber));
	} break;
	case CAMERA_CAM_CAL_DATA_SHADING_TABLE: {
		PCAM_CAL_LSC_DATA_STRUCT pRetData =
			(PCAM_CAL_LSC_DATA_STRUCT)a_pCamCalData;
		memcpy(&(pRetData->SingleLsc), &(pCamcalData->SingleLsc),
		       sizeof(CAM_CAL_SINGLE_LSC_STRUCT));
	} break;
	case CAMERA_CAM_CAL_DATA_3A_GAIN: {
		PCAM_CAL_2A_DATA_STRUCT pRetData = (PCAM_CAL_2A_DATA_STRUCT)a_pCamCalData;
		memcpy(&(pRetData->Single2A), &(pCamcalData->Single2A),
		       sizeof(CAM_CAL_SINGLE_2A_STRUCT));
	} break;
	case CAMERA_CAM_CAL_DATA_STEREO_DATA: {
		PCAM_CAL_STEREO_DATA_STRUCT pRetData =
			(PCAM_CAL_STEREO_DATA_STRUCT)a_pCamCalData;
		memcpy(&(pRetData->Stereo_Data), &(pCamcalData->Stereo_Data),
		       sizeof(CAM_CAL_Stereo_Data_STRUCT));
	} break;
	case CAMERA_CAM_CAL_DATA_PDAF: {
		PCAM_CAL_PDAF_DATA_STRUCT pRetData =
			(PCAM_CAL_PDAF_DATA_STRUCT)a_pCamCalData;
		memcpy(&(pRetData->PDAF), &(pCamcalData->PDAF),
		       sizeof(CAM_CAL_PDAF_STRUCT));
	} break;
	case CAMERA_CAM_CAL_DATA_DUMP: {
		PCAM_CAL_DATA_STRUCT pRetData = (PCAM_CAL_DATA_STRUCT)a_pCamCalData;
		memcpy(pRetData, pCamcalData, sizeof(CAM_CAL_DATA_STRUCT));
	} break;
	case CAMERA_CAM_CAL_DATA_LENS_ID: {
		PCAM_CAL_LENS_ID_STRUCT pRetData = (PCAM_CAL_LENS_ID_STRUCT)a_pCamCalData;
		memcpy(&(pRetData->LensDrvId), &(pCamcalData->LensDrvId),
		       sizeof(CAM_CAL_LENS_ID_STRUCT));
	} break;
	default:
		LOG_ERR("cal_enum (%d) is not defined", cal_enum);
		break;
	}
	int result = 0;
	return result;
}

int CamCalHelper::get_cam_cal_data(PCAM_CAL_DATA_STRUCT pCamCalData)
{
	unsigned int result = CAM_CAL_ERR_NO_DEVICE;

	LOG_INF("Command= %d", pCamCalData->Command);
	pthread_mutex_lock(&mEEPROM_Mutex);

	CAMERA_CAM_CAL_TYPE_ENUM lsCommand = pCamCalData->Command;

	if (lsCommand < 0 || lsCommand >= CAMERA_CAM_CAL_DATA_LIST) {
		LOG_ERR("Invalid Command = %d", lsCommand);
		pthread_mutex_unlock(&mEEPROM_Mutex);
		return CAM_CAL_ERR_NO_CMD;
	}

	/* camcal flow for custom data */
	int CamcamFID = 0;

	std::string eepromDev = "";

	switch (libcamera::PlatformUtils::platform_) {
	case libcamera::PlatformUtils::MtkISP7Platform::NONE:
		LOG_ERR("Platform unconfigured");
		break;

	case libcamera::PlatformUtils::MtkISP7Platform::GOOGLE:
		if (pCamCalData->sensorID == HI1339_SENSOR_ID) {
			LOG_INF("Read Sensor ID 0x%x Data", pCamCalData->sensorID);

			CamcamFID = getFakeFdWithEepromData();
			if (CamcamFID < 0) {
				pthread_mutex_unlock(&mEEPROM_Mutex);
				return result;
			}

			pCamCalData->DataVer = (CAM_CAL_DATA_VER_ENUM)CalLayoutTbl[rfLayoutType].DataVer;

			if ((CalLayoutTbl[rfLayoutType].CalItemTbl[lsCommand].Include != 0) && (CalLayoutTbl[rfLayoutType].CalItemTbl[lsCommand].GetCalDataProcess != NULL)) {
				result = CalLayoutTbl[rfLayoutType].CalItemTbl[lsCommand].GetCalDataProcess(
					CamcamFID,
					CalLayoutTbl[rfLayoutType].CalItemTbl[lsCommand].StartAddr,
					CalLayoutTbl[rfLayoutType].CalItemTbl[lsCommand].BlockSize,
					reinterpret_cast<uint32_t *>(pCamCalData));
			} else {
				result = CamCalReturnErr[lsCommand];
				//ShowCmdErrorLog(lsCommand);
			}
		} else if (pCamCalData->sensorID == GC08A3_SENSOR_ID) {
			LOG_INF("Read Sensor ID 0x%x Data", pCamCalData->sensorID);

			CamcamFID = getFakeFdWithEepromData();
			if (CamcamFID < 0) {
				pthread_mutex_unlock(&mEEPROM_Mutex);
				return result;
			}

			pCamCalData->DataVer = (CAM_CAL_DATA_VER_ENUM)CalLayoutTbl[ffLayoutType].DataVer;

			if ((CalLayoutTbl[ffLayoutType].CalItemTbl[lsCommand].Include != 0) && (CalLayoutTbl[ffLayoutType].CalItemTbl[lsCommand].GetCalDataProcess != NULL)) {
				result = CalLayoutTbl[ffLayoutType].CalItemTbl[lsCommand].GetCalDataProcess(
					CamcamFID,
					CalLayoutTbl[ffLayoutType].CalItemTbl[lsCommand].StartAddr,
					CalLayoutTbl[ffLayoutType].CalItemTbl[lsCommand].BlockSize,
					reinterpret_cast<uint32_t *>(pCamCalData));
			} else {
				result = CamCalReturnErr[lsCommand];
				//ShowCmdErrorLog(lsCommand);
			}
		}
		break;

	case libcamera::PlatformUtils::MtkISP7Platform::LENOVO:
		LayoutType = CALIBRATION_LAYOUT_EXT_OP;
		if (pCamCalData->sensorID == GC08A3_SENSOR_ID) {
			eepromDev = "/sys/bus/i2c/devices/6-0058/eeprom";
		} else if (pCamCalData->sensorID == GC05A2_SENSOR_ID) {
			eepromDev = "/sys/bus/i2c/devices/5-0050/eeprom";
		}
		LOG_INF("Read Sensor ID 0x%x Data. Open eeprom device %s", pCamCalData->sensorID, eepromDev.c_str());

		CamcamFID = getFakeFdWithEepromData();
		if (CamcamFID < 0) {
			pthread_mutex_unlock(&mEEPROM_Mutex);
			return result;
		}

		pCamCalData->DataVer = (CAM_CAL_DATA_VER_ENUM)CalLayoutTbl[LayoutType].DataVer;

		if ((CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].Include != 0) && (CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].GetCalDataProcess != NULL)) {
			result = CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].GetCalDataProcess(
				CamcamFID,
				CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].StartAddr,
				CalLayoutTbl[LayoutType].CalItemTbl[lsCommand].BlockSize,
				reinterpret_cast<uint32_t *>(pCamCalData));
		} else {
			result = CamCalReturnErr[lsCommand];
			//ShowCmdErrorLog(lsCommand);
		}
		break;
	}

	close(CamcamFID);
	pthread_mutex_unlock(&mEEPROM_Mutex);
	return result;
}

int CamCalHelper::getFakeFdWithEepromData()
{
	int fd = memfd_create("my_memfd", MFD_CLOEXEC);
	if (fd == -1) {
		LOG_ERR("Failed to memfd_create");
		printf("Failed to memfd_create");

		return -1;
	}

	// Set the initial size of the file (optional)
	if (ftruncate(fd, eepromData_.size()) == -1) {
		LOG_ERR("Failed to ftruncate");
		printf("Failed to ftruncate");

		close(fd);
		return -1;
	}

	void *ptr = mmap(NULL, eepromData_.size(), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED) {
		LOG_ERR("Failed to mmap");
		printf("Failed to mmap");

		close(fd);
		return -1;
	}

	// Write to the memory region
	memcpy(ptr, eepromData_.data(), eepromData_.size());
	munmap(ptr, eepromData_.size());

	return fd;
}

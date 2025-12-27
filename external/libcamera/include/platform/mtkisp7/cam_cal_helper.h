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

#include "linux/mtkisp7/cam_cal_format.h"
#include "platform/mtkisp7/mtkcam-core/aaa/peripheralcontroller/include/PeripheralInfoDef.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/mtkcam-interfaces/hw/mem/cam_cal_drv.h"

class CamCalHelper
{
public:
	static std::shared_ptr<CamCalHelper> getInstance(uint32_t sensorIndex);

	int get_cal_data(ENUM_CAMERA_CAM_CAL_TYPE_ENUM cal_enum, int sensor_id,
			 int sensor_dev, void *a_pCamCalData);

	void setEepromData(const std::vector<uint8_t> &eepromData)
	{
		eepromData_ = eepromData;
	}

private:
	CAM_CAL_DATA_STRUCT StCamCalCaldata;

	std::vector<uint8_t> eepromData_;
	int get_cam_cal_data(PCAM_CAL_DATA_STRUCT pCamCalData);

	int getFakeFdWithEepromData();
};

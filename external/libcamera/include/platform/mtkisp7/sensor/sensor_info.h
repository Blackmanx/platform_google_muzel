// Copyright 2023 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include <array>
#include <memory>
#include <vector>

#include "linux/mtkisp7/cam_cal_format.h"
#include "platform/mtkisp7/mtkcam-core/aaa/peripheralcontroller/include/PeripheralInfoDef.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/mtkcam-interfaces/hw/mem/cam_cal_drv.h"

#define MAX_SENSOR_INFO_COUNT 10

class SensorInfo
{
public:
	struct CamSysData {
		bool has_af;
		uint32_t mbus_code;
	};

	SensorInfo(int sensor_idx);
	void init(int sensor_dev, int sensor_id);
	static std::shared_ptr<SensorInfo> getInstance(int sensor_idx);
	static void add_sensor(const std::vector<CamSysData> &camSysDataArray);
	void get_sensor_static_info(
		std::array<mtk::hal3a::SensorStaticInfo, kMaxSensorCnt> *
			sensor_static_info_array);
	int get_cal_data(ENUM_CAMERA_CAM_CAL_TYPE_ENUM cal_enum, void *pCamCalData);

	bool is_af_support();

private:
	uint32_t m_sensor_index;
	uint32_t m_sensor_dev;
	uint32_t m_sensor_id;
	static std::shared_ptr<SensorInfo> sensor_info_[MAX_SENSOR_INFO_COUNT];
	static std::vector<std::shared_ptr<NSCam::SensorStaticInfo>>
		nscam_sensor_static_info_;
	static std::vector<CamSysData> camSysDataArray_;
	static void construct_sensor_static_info(
		int index, std::shared_ptr<NSCam::SensorStaticInfo> pSensorStaticInfo);
	int get_cam_cal_data(PCAM_CAL_DATA_STRUCT pCamCalData);
};

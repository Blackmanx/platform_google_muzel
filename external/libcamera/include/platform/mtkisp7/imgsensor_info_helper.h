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

#include <memory>

#include "mtkcam-interfaces/hw/sensor/IHalSensor.h"
#include "platform/mtkisp7/mtkcam-chrom/custom/mt8188/hal/inc/camera_custom_imgsensor_cfg.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/mtkcam-interfaces/hw/sensor/imgsensor_info.h"
typedef enum {
	IMAGE_SENSOR_TYPE_RAW,
	IMAGE_SENSOR_TYPE_YUV,
	IMAGE_SENSOR_TYPE_YCBCR,
	IMAGE_SENSOR_TYPE_RGB565,
	IMAGE_SENSOR_TYPE_RGB888,
	IMAGE_SENSOR_TYPE_JPEG,
	IMAGE_SENSOR_TYPE_RAW8,
	IMAGE_SENSOR_TYPE_RAW12,
	IMAGE_SENSOR_TYPE_RAW14,
	IMAGE_SENSOR_TYPE_UNKNOWN = 0xFFFF,
} IMAGE_SENSOR_TYPE;

enum SensorOrientation {
	kRear = 0,
	kFront,
};

NSCamCustomSensor::CUSTOM_CFG *getCustomConfig(IMGSENSOR_SENSOR_IDX const sensorIdx);

int get_binning_type(int scenario_id);
int get_resolution(ACDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution,
		   int index);
int get_info(ACDK_SENSOR_INFO_STRUCT *sensor_info,
	     struct imgsensor_info_struct *imgsensor_info);
imgsensor_info_struct *get_imgsensor_info_by_id(int id);
int get_format_type_and_order(int format, unsigned int *type,
			      unsigned int *order);
int get_output_format_by_scenario(
	struct imgsensor_info_struct *imgsensor_info, int scenario_id);
int get_default_framerate_by_scenario(
	struct imgsensor_info_struct *imgsensor_info, int scenario_id);
IMAGE_SENSOR_TYPE getType(ACDK_SENSOR_INFO_STRUCT *info);
void querySensorInfo(
	int sensorId_idx, IMGSENSOR_SENSOR_IDX sensorIdx,
	struct imgsensor_info_struct *imgsensor_info,
	std::shared_ptr<NSCam::SensorStaticInfo> pSensorStaticInfo);

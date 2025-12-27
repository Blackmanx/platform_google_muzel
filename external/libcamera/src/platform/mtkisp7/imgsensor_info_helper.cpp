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

#include "platform/mtkisp7/imgsensor_info_helper.h"

#include <fcntl.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/media.h>
#include <linux/types.h>
#include <linux/v4l2-mediabus.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>

#include "platform/mtkisp7/mtkcam-chrom/custom/mt8188/hal/imgsensor_src/imgsensor_info_custom.h"
#include "platform/mtkisp7/mtkcam-chrom/custom/mt8188/hal/inc/camera_custom_imgsensor_cfg.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/kernel-headers/imgsensor-user.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/kernel-headers/kd_imgsensor.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/kernel-headers/kd_imgsensor_define_v4l2.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/mtkcam-interfaces/hw/sensor/IHalSensor.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/mtkcam-interfaces/hw/sensor/imgsensor_info.h"

#define LOG_ERR(fmt, ...) printf((fmt "\n"), ##__VA_ARGS__)
#define LOG_WRN(fmt, ...)
#define LOG_ADBDBG(...)
#define LOG_INF(...)
#define LOG_VRB(...)
#define LOG_DBG(...)

//=========================================================

static NSCamCustomSensor::CUSTOM_CFG gCustomCfg[] = {
	{ .sensorIdx = IMGSENSOR_SENSOR_IDX_MAIN,
	  .mclk = NSCamCustomSensor::CUSTOM_CFG_MCLK_2,
	  .port = NSCamCustomSensor::CUSTOM_CFG_CSI_PORT_2,
	  .dir = NSCamCustomSensor::CUSTOM_CFG_DIR_REAR,
	  .bitOrder = NSCamCustomSensor::CUSTOM_CFG_BITORDER_9_2,
	  .orientation = 0,
	  .horizontalFov = 67,
	  .verticalFov = 49,
	  .secure = NSCamCustomSensor::CUSTOM_CFG_SECURE_NONE },
	{ .sensorIdx = IMGSENSOR_SENSOR_IDX_SUB,
	  .mclk = NSCamCustomSensor::CUSTOM_CFG_MCLK_4,
	  .port = NSCamCustomSensor::CUSTOM_CFG_CSI_PORT_0,
	  .dir = NSCamCustomSensor::CUSTOM_CFG_DIR_FRONT,
	  .bitOrder = NSCamCustomSensor::CUSTOM_CFG_BITORDER_9_2,
	  .orientation = 0,
	  .horizontalFov = 63,
	  .verticalFov = 40,
	  .secure = NSCamCustomSensor::CUSTOM_CFG_SECURE_M0 },
	{ .sensorIdx = IMGSENSOR_SENSOR_IDX_MAIN2,
	  .mclk = NSCamCustomSensor::CUSTOM_CFG_MCLK_1,
	  .port = NSCamCustomSensor::CUSTOM_CFG_CSI_PORT_1,
	  .dir = NSCamCustomSensor::CUSTOM_CFG_DIR_REAR,
	  .bitOrder = NSCamCustomSensor::CUSTOM_CFG_BITORDER_9_2,
	  .orientation = 90,
	  .horizontalFov = 75,
	  .verticalFov = 60,
	  .secure = NSCamCustomSensor::CUSTOM_CFG_SECURE_NONE },
	{ .sensorIdx = IMGSENSOR_SENSOR_IDX_SUB2,
	  .mclk = NSCamCustomSensor::CUSTOM_CFG_MCLK_3,
	  .port = NSCamCustomSensor::CUSTOM_CFG_CSI_PORT_0,
	  .dir = NSCamCustomSensor::CUSTOM_CFG_DIR_FRONT,
	  .bitOrder = NSCamCustomSensor::CUSTOM_CFG_BITORDER_9_2,
	  .orientation = 90,
	  .horizontalFov = 75,
	  .verticalFov = 60,
	  .secure = NSCamCustomSensor::CUSTOM_CFG_SECURE_NONE },
	{ .sensorIdx = IMGSENSOR_SENSOR_IDX_MAIN3,
	  .mclk = NSCamCustomSensor::CUSTOM_CFG_MCLK_3,
	  .port = NSCamCustomSensor::CUSTOM_CFG_CSI_PORT_0,
	  .dir = NSCamCustomSensor::CUSTOM_CFG_DIR_REAR,
	  .bitOrder = NSCamCustomSensor::CUSTOM_CFG_BITORDER_9_2,
	  .orientation = 90,
	  .horizontalFov = 67,
	  .verticalFov = 49,
	  .secure = NSCamCustomSensor::CUSTOM_CFG_SECURE_NONE },
	{ .sensorIdx = IMGSENSOR_SENSOR_IDX_NONE,
	  .mclk = NSCamCustomSensor::CUSTOM_CFG_MCLK_NONE,
	  .port = NSCamCustomSensor::CUSTOM_CFG_CSI_PORT_NONE,
	  .dir = NSCamCustomSensor::CUSTOM_CFG_DIR_NONE,
	  .bitOrder = NSCamCustomSensor::CUSTOM_CFG_BITORDER_NONE,
	  .orientation = 0,
	  .horizontalFov = 0,
	  .verticalFov = 0,
	  .secure = NSCamCustomSensor::CUSTOM_CFG_SECURE_NONE }
};

NSCamCustomSensor::CUSTOM_CFG *getCustomConfig(
	IMGSENSOR_SENSOR_IDX const sensorIdx)
{
	NSCamCustomSensor::CUSTOM_CFG *pCustomCfg =
		&gCustomCfg[IMGSENSOR_SENSOR_IDX_MIN_NUM];

	if (sensorIdx >= IMGSENSOR_SENSOR_IDX_MAX_NUM ||
	    sensorIdx < IMGSENSOR_SENSOR_IDX_MIN_NUM)
		return NULL;

	while (pCustomCfg->sensorIdx != IMGSENSOR_SENSOR_IDX_NONE &&
	       pCustomCfg->sensorIdx != sensorIdx)
		pCustomCfg++;

	if (pCustomCfg->sensorIdx == IMGSENSOR_SENSOR_IDX_NONE)
		return NULL;

	return pCustomCfg;
}

int get_format_type_and_order(int format, unsigned int *type,
			      unsigned int *order)
{
	switch (format) {
	case SENSOR_OUTPUT_FORMAT_RAW_B:
	case SENSOR_OUTPUT_FORMAT_RAW8_B:
	case SENSOR_OUTPUT_FORMAT_RAW12_B:
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_B;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_Gb:
	case SENSOR_OUTPUT_FORMAT_RAW8_Gb:
	case SENSOR_OUTPUT_FORMAT_RAW12_Gb:
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_Gb;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_Gr:
	case SENSOR_OUTPUT_FORMAT_RAW8_Gr:
	case SENSOR_OUTPUT_FORMAT_RAW12_Gr:
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_Gr;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_R:
	case SENSOR_OUTPUT_FORMAT_RAW8_R:
	case SENSOR_OUTPUT_FORMAT_RAW12_R:
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_R;
		break;
	case SENSOR_OUTPUT_FORMAT_UYVY:
	case SENSOR_OUTPUT_FORMAT_CbYCrY:
		*order = NSCam::SENSOR_FORMAT_ORDER_UYVY;
		break;
	case SENSOR_OUTPUT_FORMAT_VYUY:
	case SENSOR_OUTPUT_FORMAT_CrYCbY:
		*order = NSCam::SENSOR_FORMAT_ORDER_VYUY;
		break;
	case SENSOR_OUTPUT_FORMAT_YUYV:
	case SENSOR_OUTPUT_FORMAT_YCbYCr:
		*order = NSCam::SENSOR_FORMAT_ORDER_YUYV;
		break;
	case SENSOR_OUTPUT_FORMAT_YVYU:
	case SENSOR_OUTPUT_FORMAT_YCrYCb:
		*order = NSCam::SENSOR_FORMAT_ORDER_YVYU;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_RWB_B:
		*type = NSCam::SENSOR_RAW_RWB;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_B;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_RWB_Wb:
		*type = NSCam::SENSOR_RAW_RWB;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_Gb;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_RWB_Wr:
		*type = NSCam::SENSOR_RAW_RWB;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_Gr;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_RWB_R:
		*type = NSCam::SENSOR_RAW_RWB;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_R;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_MONO:
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_B;
		*type = NSCam::SENSOR_RAW_MONO;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_4CELL_B:
		*type = NSCam::SENSOR_RAW_4CELL;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_B;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_4CELL_Gb:
		*type = NSCam::SENSOR_RAW_4CELL;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_Gb;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_4CELL_Gr:
		*type = NSCam::SENSOR_RAW_4CELL;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_Gr;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_4CELL_R:
		*type = NSCam::SENSOR_RAW_4CELL;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_R;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_B:
		*type = NSCam::SENSOR_RAW_4CELL_HW_BAYER;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_B;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_Gb:
		*type = NSCam::SENSOR_RAW_4CELL_HW_BAYER;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_Gb;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_Gr:
		*type = NSCam::SENSOR_RAW_4CELL_HW_BAYER;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_Gr;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_R:
		*type = NSCam::SENSOR_RAW_4CELL_HW_BAYER;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_R;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_B:
		*type = NSCam::SENSOR_RAW_4CELL_BAYER;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_B;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_Gb:
		*type = NSCam::SENSOR_RAW_4CELL_BAYER;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_Gb;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_Gr:
		*type = NSCam::SENSOR_RAW_4CELL_BAYER;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_Gr;
		break;
	case SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_R:
		*type = NSCam::SENSOR_RAW_4CELL_BAYER;
		*order = NSCam::SENSOR_FORMAT_ORDER_RAW_R;
		break;
	default:
		*order = NSCam::SENSOR_FORMAT_ORDER_NONE;
		*type = NSCam::SENSOR_RAW_FMT_NONE;
		break;
	}
	return 0;
}

int get_output_format_by_scenario(struct imgsensor_info_struct *imgsensor_info,
				  int scenario_id)
{
	int value = 0;

	switch (scenario_id) {
	case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
		value = imgsensor_info->sensor_output_dataformat;
		break;
	case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
		value = imgsensor_info->sensor_output_dataformat;
		break;
	case SENSOR_SCENARIO_ID_HIGHSPEED_VIDEO:
		value = imgsensor_info->sensor_output_dataformat;
		break;
	case SENSOR_SCENARIO_ID_SLIM_VIDEO:
		value = imgsensor_info->sensor_output_dataformat;
		break;
	case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
	default:
		value = imgsensor_info->sensor_output_dataformat;
		break;
	}

	return value;
}

int get_default_framerate_by_scenario(
	struct imgsensor_info_struct *imgsensor_info, int scenario_id)
{
	int fps = 0;
	switch (scenario_id) {
	case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
		fps = imgsensor_info->pre.max_framerate;
		break;
	case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
		fps = imgsensor_info->normal_video.max_framerate;
		break;
	case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
		fps = imgsensor_info->cap.max_framerate;
		break;
	case SENSOR_SCENARIO_ID_HIGHSPEED_VIDEO:
		fps = imgsensor_info->hs_video.max_framerate;
		break;
	case SENSOR_SCENARIO_ID_SLIM_VIDEO:
		fps = imgsensor_info->slim_video.max_framerate;
		break;
	default:
		fps = imgsensor_info->pre.max_framerate;
		break;
	}

	return fps;
}

int get_resolution(ACDK_SENSOR_RESOLUTION_INFO_STRUCT *sensor_resolution,
		   int index)
{
	struct imgsensor_info_struct *imgsensor_info;
	imgsensor_info = &gImgsensor_info[index];

	SENSOR_WINSIZE_INFO_STRUCT *pWinSizeInfo;

	int i = 0;

	for (i = SENSOR_SCENARIO_ID_MIN; i < SENSOR_SCENARIO_ID_MAX; i++) {
		if (i < imgsensor_info->sensor_mode_num) {
			pWinSizeInfo = &gImgsensor_winsize_info[index][i];
			sensor_resolution->SensorWidth[i] = pWinSizeInfo->w2_tg_size;
			sensor_resolution->SensorHeight[i] = pWinSizeInfo->h2_tg_size;
		} else {
			sensor_resolution->SensorWidth[i] = 0;
			sensor_resolution->SensorHeight[i] = 0;
		}
	}

	return 0;
}

int get_binning_type(int scenario_id)
{
	int binning_type = 1;
	switch (scenario_id) {
	case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
	case SENSOR_SCENARIO_ID_HIGHSPEED_VIDEO:
	case SENSOR_SCENARIO_ID_SLIM_VIDEO:
	case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
	case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
	default:
		binning_type = 1; /*BINNING_AVERAGE*/
		break;
	}
	return binning_type;
}

int get_info(ACDK_SENSOR_INFO_STRUCT *sensor_info,
	     struct imgsensor_info_struct *imgsensor_info)
{
	sensor_info->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
	/* not use */
	sensor_info->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorHsyncPolarity =
		SENSOR_CLOCK_POLARITY_LOW; // inverse with datasheet
	sensor_info->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	sensor_info->SensorInterruptDelayLines = 4; /* not use */
	sensor_info->SensorResetActiveHigh = MFALSE; /* not use */
	sensor_info->SensorResetDelayCount = 5; /* not use */

	sensor_info->SensroInterfaceType =
		(ACDK_SENSOR_INTERFACE_TYPE_ENUM)imgsensor_info->sensor_interface_type;
	sensor_info->MIPIsensorType =
		(SENSOR_MIPI_TYPE_ENUM)imgsensor_info->mipi_sensor_type;
	// sensor_info->SettleDelayMode = imgsensor_info.mipi_settle_delay_mode;
	sensor_info->SensorOutputDataFormat =
		(ACDK_SENSOR_OUTPUT_DATA_FORMAT_ENUM)
			imgsensor_info->sensor_output_dataformat;

	sensor_info->DelayFrame[SENSOR_SCENARIO_ID_NORMAL_PREVIEW] =
		imgsensor_info->pre_delay_frame;
	sensor_info->DelayFrame[SENSOR_SCENARIO_ID_NORMAL_CAPTURE] =
		imgsensor_info->cap_delay_frame;
	sensor_info->DelayFrame[SENSOR_SCENARIO_ID_NORMAL_VIDEO] =
		imgsensor_info->video_delay_frame;
	sensor_info->DelayFrame[SENSOR_SCENARIO_ID_HIGHSPEED_VIDEO] =
		imgsensor_info->hs_video_delay_frame;
	sensor_info->DelayFrame[SENSOR_SCENARIO_ID_SLIM_VIDEO] =
		imgsensor_info->slim_video_delay_frame;

	sensor_info->SensorMasterClockSwitch = 0; /* not use */
	sensor_info->SensorDrivingCurrent = imgsensor_info->isp_driving_current;

	/* The frame of setting shutter default 0 for TG int */
	sensor_info->AEShutDelayFrame = imgsensor_info->ae_shut_delay_frame;
	/* The frame of setting sensor gain */
	sensor_info->AESensorGainDelayFrame =
		imgsensor_info->ae_sensor_gain_delay_frame;
	sensor_info->AEISPGainDelayFrame = imgsensor_info->ae_ispGain_delay_frame;
	sensor_info->IHDR_Support = imgsensor_info->ihdr_support;
	sensor_info->IHDR_LE_FirstLine = imgsensor_info->ihdr_le_firstline;
	sensor_info->SensorModeNum = imgsensor_info->sensor_mode_num;

	sensor_info->SensorMIPILaneNumber =
		(ACDK_SENSOR_MIPI_LANE_NUMBER_ENUM)imgsensor_info->mipi_lane_num;
	sensor_info->SensorClockFreq = imgsensor_info->mclk;
	sensor_info->SensorClockDividCount = 3; /* not use */
	sensor_info->SensorClockRisingCount = 0;
	sensor_info->SensorClockFallingCount = 2; /* not use */
	sensor_info->SensorPixelClockCount = 3; /* not use */
	sensor_info->SensorDataLatchCount = 2; /* not use */

	sensor_info->SensorWidthSampling = 0; // 0 is default 1x
	sensor_info->SensorHightSampling = 0; // 0 is default 1x
	sensor_info->SensorPacketECCOrder = 1;

	return 0;
}

IMAGE_SENSOR_TYPE
getType(ACDK_SENSOR_INFO_STRUCT *info)
{
	if (info->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_B &&
	    info->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_R) {
		return IMAGE_SENSOR_TYPE_RAW;
	} else if (info->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW8_B &&
		   info->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW8_R) {
		return IMAGE_SENSOR_TYPE_RAW8;
	} else if (info->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_UYVY &&
		   info->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YVYU) {
		return IMAGE_SENSOR_TYPE_YUV;
	} else if (info->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_CbYCrY &&
		   info->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YCrYCb) {
		return IMAGE_SENSOR_TYPE_YCBCR;
	} else if (info->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_RWB_B &&
		   info->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_RWB_R) {
		return IMAGE_SENSOR_TYPE_RAW;
	} else if (info->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_4CELL_B &&
		   info->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_4CELL_R) {
		return IMAGE_SENSOR_TYPE_RAW;
	} else if (info->SensorOutputDataFormat >=
			   SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_B &&
		   info->SensorOutputDataFormat <=
			   SENSOR_OUTPUT_FORMAT_RAW_4CELL_HW_BAYER_R) {
		return IMAGE_SENSOR_TYPE_RAW;

	} else if (info->SensorOutputDataFormat >=
			   SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_B &&
		   info->SensorOutputDataFormat <=
			   SENSOR_OUTPUT_FORMAT_RAW_4CELL_BAYER_R) {
		return IMAGE_SENSOR_TYPE_RAW;
	} else if (info->SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_RAW_MONO) {
		return IMAGE_SENSOR_TYPE_RAW;
	} else if (info->SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW12_B &&
		   info->SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW12_R) {
		return IMAGE_SENSOR_TYPE_RAW12;
	} else {
		return IMAGE_SENSOR_TYPE_UNKNOWN;
	}

	return IMAGE_SENSOR_TYPE_UNKNOWN;
}

void querySensorInfo(
	int sensorId_idx, IMGSENSOR_SENSOR_IDX sensorIdx,
	struct imgsensor_info_struct *imgsensor_info,
	std::shared_ptr<NSCam::SensorStaticInfo> pSensorStaticInfo)
{
	pSensorStaticInfo->sensorDevID = gimgsensor_sensor_list[sensorId_idx].id;
	NSCamCustomSensor::CUSTOM_CFG *pCustomCfg = getCustomConfig(sensorIdx);

	pSensorStaticInfo->orientationAngle = pCustomCfg->orientation;
	pSensorStaticInfo->facingDirection = pCustomCfg->dir;
	pSensorStaticInfo->horizontalViewAngle = pCustomCfg->horizontalFov;
	pSensorStaticInfo->verticalViewAngle = pCustomCfg->verticalFov;

	if (!imgsensor_info) {
		LOG_ERR("imgsensor_info is null!!");
		return;
	}

	struct SENSOR_WINSIZE_INFO_STRUCT *imgsensor_winsize_info =
		&gImgsensor_winsize_info[sensorId_idx][0];
	ACDK_SENSOR_INFO_STRUCT sensor_info;
	get_info(&sensor_info, imgsensor_info);
	ACDK_SENSOR_RESOLUTION_INFO_STRUCT sensor_resolution;
	get_resolution(&sensor_resolution, sensorId_idx);
	unsigned int scenario_id = SENSOR_SCENARIO_ID_MIN;
	for (scenario_id = SENSOR_SCENARIO_ID_MIN;
	     scenario_id < SENSOR_SCENARIO_ID_MAX; scenario_id++) {
		pSensorStaticInfo->mode_stats[scenario_id].fps =
			get_default_framerate_by_scenario(imgsensor_info, scenario_id);
		memcpy(&pSensorStaticInfo->mode_stats[scenario_id].cropInfo,
		       &imgsensor_winsize_info[scenario_id],
		       sizeof(SENSOR_WINSIZE_INFO_STRUCT));

		pSensorStaticInfo->mode_stats[scenario_id].seamless_targets_count = 0;
		// This is always 1 now
		pSensorStaticInfo->mode_stats[scenario_id].binning_type = 1;
		// pSensorStaticInfo->mode_stats[scenario_id].binning_type =
		// get_binning_type(scenario_id);

		pSensorStaticInfo->mode_stats[scenario_id].delayFrame =
			sensor_info.DelayFrame[scenario_id];
		int sensor_output_format =
			get_output_format_by_scenario(imgsensor_info, scenario_id);
		get_format_type_and_order(
			sensor_output_format,
			&pSensorStaticInfo->mode_stats[scenario_id].rawFmtType,
			&pSensorStaticInfo->mode_stats[scenario_id].sensorFormatOrder);
	}
#ifndef SENSOR_MODE_EXTEND
	pSensorStaticInfo->previewFrameRate =
		pSensorStaticInfo->mode_stats[SENSOR_SCENARIO_ID_NORMAL_PREVIEW].fps;
	pSensorStaticInfo->captureFrameRate =
		pSensorStaticInfo->mode_stats[SENSOR_SCENARIO_ID_NORMAL_CAPTURE].fps;
	pSensorStaticInfo->videoFrameRate =
		pSensorStaticInfo->mode_stats[SENSOR_SCENARIO_ID_NORMAL_VIDEO].fps;
	pSensorStaticInfo->video1FrameRate =
		pSensorStaticInfo->mode_stats[SENSOR_SCENARIO_ID_HIGHSPEED_VIDEO].fps;
	pSensorStaticInfo->video2FrameRate =
		pSensorStaticInfo->mode_stats[SENSOR_SCENARIO_ID_SLIM_VIDEO].fps;
	pSensorStaticInfo->custom1FrameRate =
		pSensorStaticInfo->mode_stats[SENSOR_SCENARIO_ID_CUSTOM1].fps;
	pSensorStaticInfo->custom2FrameRate =
		pSensorStaticInfo->mode_stats[SENSOR_SCENARIO_ID_CUSTOM2].fps;
	pSensorStaticInfo->custom3FrameRate =
		pSensorStaticInfo->mode_stats[SENSOR_SCENARIO_ID_CUSTOM3].fps;
	pSensorStaticInfo->custom4FrameRate =
		pSensorStaticInfo->mode_stats[SENSOR_SCENARIO_ID_CUSTOM4].fps;
	pSensorStaticInfo->custom5FrameRate =
		pSensorStaticInfo->mode_stats[SENSOR_SCENARIO_ID_CUSTOM5].fps;
#endif
	IMAGE_SENSOR_TYPE image_sensor_type = getType(&sensor_info);
	switch (image_sensor_type) {
	case IMAGE_SENSOR_TYPE_RAW:
		pSensorStaticInfo->sensorType = NSCam::SENSOR_TYPE_RAW;
		pSensorStaticInfo->rawSensorBit = NSCam::RAW_SENSOR_10BIT;
		break;
	case IMAGE_SENSOR_TYPE_RAW8:
		pSensorStaticInfo->sensorType = NSCam::SENSOR_TYPE_RAW;
		pSensorStaticInfo->rawSensorBit = NSCam::RAW_SENSOR_8BIT;
		break;
	case IMAGE_SENSOR_TYPE_RAW12:
		pSensorStaticInfo->sensorType = NSCam::SENSOR_TYPE_RAW;
		pSensorStaticInfo->rawSensorBit = NSCam::RAW_SENSOR_12BIT;
		break;
	case IMAGE_SENSOR_TYPE_RAW14:
		pSensorStaticInfo->sensorType = NSCam::SENSOR_TYPE_RAW;
		pSensorStaticInfo->rawSensorBit = NSCam::RAW_SENSOR_14BIT;
		break;
	case IMAGE_SENSOR_TYPE_YUV:
	case IMAGE_SENSOR_TYPE_YCBCR:
		pSensorStaticInfo->sensorType = NSCam::SENSOR_TYPE_YUV;
		pSensorStaticInfo->rawSensorBit = NSCam::RAW_SENSOR_ERROR;
		break;
	case IMAGE_SENSOR_TYPE_RGB565:
		pSensorStaticInfo->sensorType = NSCam::SENSOR_TYPE_RGB;
		pSensorStaticInfo->rawSensorBit = NSCam::RAW_SENSOR_ERROR;
		break;
	case IMAGE_SENSOR_TYPE_JPEG:
		pSensorStaticInfo->sensorType = NSCam::SENSOR_TYPE_JPEG;
		pSensorStaticInfo->rawSensorBit = NSCam::RAW_SENSOR_ERROR;
		break;
	default:
		pSensorStaticInfo->sensorType = NSCam::SENSOR_TYPE_UNKNOWN;
		pSensorStaticInfo->rawSensorBit = NSCam::RAW_SENSOR_ERROR;
		break;
	}
	/*Use capture for default*/
	pSensorStaticInfo->sensorFormatOrder =
		pSensorStaticInfo->mode_stats[1].sensorFormatOrder;
	pSensorStaticInfo->rawFmtType = pSensorStaticInfo->mode_stats[1].rawFmtType;
	switch (sensor_info.PDAF_Support) {
	case 1: /* 1: PDAF Raw Data mode */
		pSensorStaticInfo->rawFmtType = NSCam::SENSOR_RAW_PD;
		break;
	default:
		break;
	}
	pSensorStaticInfo->iHDRSupport = sensor_info.IHDR_Support;
	pSensorStaticInfo->PDAF_Support = sensor_info.PDAF_Support;
	pSensorStaticInfo->HDR_Support = sensor_info.HDR_Support;
	pSensorStaticInfo->aeShutDelayFrame = sensor_info.AEShutDelayFrame;
	pSensorStaticInfo->aeSensorGainDelayFrame =
		sensor_info.AESensorGainDelayFrame;
	pSensorStaticInfo->aeISPGainDelayFrame = sensor_info.AEISPGainDelayFrame;
	pSensorStaticInfo->FrameTimeDelayFrame = sensor_info.FrameTimeDelayFrame;

#ifndef SENSOR_MODE_EXTEND
	NSCam::SensorStaticInfo *pS = pSensorStaticInfo;

	pS->previewDelayFrame =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_PREVIEW].delayFrame;
	pS->captureDelayFrame =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_CAPTURE].delayFrame;
	pS->videoDelayFrame =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_VIDEO].delayFrame;
	pS->video1DelayFrame =
		pS->mode_stats[SENSOR_SCENARIO_ID_HIGHSPEED_VIDEO].delayFrame;

	pS->video2DelayFrame =
		pS->mode_stats[SENSOR_SCENARIO_ID_SLIM_VIDEO].delayFrame;
	pS->Custom1DelayFrame = pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM1].delayFrame;
	pS->Custom2DelayFrame = pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM2].delayFrame;
	pS->Custom3DelayFrame = pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM3].delayFrame;
	pS->Custom4DelayFrame = pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM4].delayFrame;
	pS->Custom5DelayFrame = pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM5].delayFrame;
	pS->SensorGrabStartX_PRV =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_PREVIEW].cropInfo.x2_tg_offset;
	pS->SensorGrabStartY_PRV =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_PREVIEW].cropInfo.y2_tg_offset;
	pS->SensorGrabStartX_CAP =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_CAPTURE].cropInfo.x2_tg_offset;
	pS->SensorGrabStartY_CAP =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_CAPTURE].cropInfo.y2_tg_offset;
	pS->SensorGrabStartX_VD =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_VIDEO].cropInfo.x2_tg_offset;
	pS->SensorGrabStartY_VD =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_VIDEO].cropInfo.y2_tg_offset;
	pS->SensorGrabStartX_VD1 =
		pS->mode_stats[SENSOR_SCENARIO_ID_HIGHSPEED_VIDEO].cropInfo.x2_tg_offset;
	pS->SensorGrabStartY_VD1 =
		pS->mode_stats[SENSOR_SCENARIO_ID_HIGHSPEED_VIDEO].cropInfo.y2_tg_offset;
	pS->SensorGrabStartX_VD2 =
		pS->mode_stats[SENSOR_SCENARIO_ID_SLIM_VIDEO].cropInfo.x2_tg_offset;
	pS->SensorGrabStartY_VD2 =
		pS->mode_stats[SENSOR_SCENARIO_ID_SLIM_VIDEO].cropInfo.y2_tg_offset;
	pS->SensorGrabStartX_CST1 =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM1].cropInfo.x2_tg_offset;
	pS->SensorGrabStartY_CST1 =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM1].cropInfo.y2_tg_offset;
	pS->SensorGrabStartX_CST2 =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM2].cropInfo.x2_tg_offset;
	pS->SensorGrabStartY_CST2 =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM2].cropInfo.y2_tg_offset;
	pS->SensorGrabStartX_CST3 =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM3].cropInfo.x2_tg_offset;
	pS->SensorGrabStartY_CST3 =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM3].cropInfo.y2_tg_offset;
	pS->SensorGrabStartX_CST4 =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM4].cropInfo.x2_tg_offset;
	pS->SensorGrabStartY_CST4 =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM4].cropInfo.y2_tg_offset;
	pS->SensorGrabStartX_CST5 =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM5].cropInfo.x2_tg_offset;
	pS->SensorGrabStartY_CST5 =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM5].cropInfo.y2_tg_offset;
	pS->previewWidth =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_PREVIEW].cropInfo.w2_tg_size;
	pS->previewHeight =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_PREVIEW].cropInfo.h2_tg_size;
	pS->captureWidth =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_CAPTURE].cropInfo.w2_tg_size;
	pS->captureHeight =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_CAPTURE].cropInfo.h2_tg_size;
	pS->videoWidth =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_VIDEO].cropInfo.w2_tg_size;
	pS->videoHeight =
		pS->mode_stats[SENSOR_SCENARIO_ID_NORMAL_VIDEO].cropInfo.h2_tg_size;
	pS->video1Width =
		pS->mode_stats[SENSOR_SCENARIO_ID_HIGHSPEED_VIDEO].cropInfo.w2_tg_size;
	pS->video1Height =
		pS->mode_stats[SENSOR_SCENARIO_ID_HIGHSPEED_VIDEO].cropInfo.h2_tg_size;
	pS->video2Width =
		pS->mode_stats[SENSOR_SCENARIO_ID_SLIM_VIDEO].cropInfo.w2_tg_size;
	pS->video2Height =
		pS->mode_stats[SENSOR_SCENARIO_ID_SLIM_VIDEO].cropInfo.h2_tg_size;
	pS->SensorCustom1Width =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM1].cropInfo.w2_tg_size;
	pS->SensorCustom1Height =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM1].cropInfo.h2_tg_size;
	pS->SensorCustom2Width =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM2].cropInfo.w2_tg_size;
	pS->SensorCustom2Height =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM2].cropInfo.h2_tg_size;
	pS->SensorCustom3Width =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM3].cropInfo.w2_tg_size;
	pS->SensorCustom3Height =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM3].cropInfo.h2_tg_size;
	pS->SensorCustom4Width =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM4].cropInfo.w2_tg_size;
	pS->SensorCustom4Height =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM4].cropInfo.h2_tg_size;
	pS->SensorCustom5Width =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM5].cropInfo.w2_tg_size;
	pS->SensorCustom5Height =
		pS->mode_stats[SENSOR_SCENARIO_ID_CUSTOM5].cropInfo.h2_tg_size;
#endif

	pSensorStaticInfo->iHDR_First_IS_LE = sensor_info.IHDR_LE_FirstLine;
	pSensorStaticInfo->SensorModeNum = sensor_info.SensorModeNum;
	pSensorStaticInfo->PerFrameCTL_Support = sensor_info.PerFrameCTL_Support;
	pSensorStaticInfo->ZHDR_MODE = sensor_info.ZHDR_Mode;

	if (sensor_info.SensorHorFOV != 0) /*from sensor driver*/
		pSensorStaticInfo->horizontalViewAngle = sensor_info.SensorHorFOV;
	if (sensor_info.SensorVerFOV != 0)
		pSensorStaticInfo->verticalViewAngle = sensor_info.SensorVerFOV;
	if (sensor_info.SensorOrientation != 0)
		pSensorStaticInfo->SensorOrientation = sensor_info.SensorOrientation;

	pSensorStaticInfo->sensorModuleID = sensor_info.SensorModuleID;

	pSensorStaticInfo->virtualChannelSupport = MFALSE;
}

// Workaround for the issue of imgsensor_info_custom.h:
// it defines global variables in a header file.

#include <memory>

#include "platform/mtkisp7/cam_cal_helper.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/mtkcam-interfaces/hw/sensor/imgsensor_info.h"
#include "platform/mtkisp7/platform_utils.h"
#include "sensor/sensor_info.h"

std::shared_ptr<SensorInfo> SensorInfo::sensor_info_[MAX_SENSOR_INFO_COUNT] = {
	nullptr
};
std::vector<std::shared_ptr<NSCam::SensorStaticInfo>>
	SensorInfo::nscam_sensor_static_info_;
std::vector<SensorInfo::CamSysData> SensorInfo::camSysDataArray_;

/*
map senidx to sensnorId
0 -> GC08A3_SENSOR_ID
1 -> HI1339_SENSOR_ID
2 -> GC05A2_SENSOR_ID
*/

std::map<int, int> sensorId_idx_map_geralt = { { 0, 1 }, { 1, 0 } };
std::map<int, int> sensorId_idx_map_ciri = { { 0, 0 }, { 1, 2 } };
SensorInfo::SensorInfo(int sensor_idx)
	: m_sensor_index(sensor_idx),
	  m_sensor_dev(0),
	  m_sensor_id(0)
{
}

void SensorInfo::init(int sensor_dev, int sensor_id)
{
	m_sensor_dev = sensor_dev;
	m_sensor_id = sensor_id;
}

std::shared_ptr<SensorInfo> SensorInfo::getInstance(int sensor_idx)
{
	if (!sensor_info_[sensor_idx]) {
		sensor_info_[sensor_idx].reset(new SensorInfo(sensor_idx));
	}
	return sensor_info_[sensor_idx];
}

void SensorInfo::add_sensor(const std::vector<SensorInfo::CamSysData> &camSysDataArray)
{
	if (!camSysDataArray_.empty())
		return;

	camSysDataArray_ = camSysDataArray;
	for (unsigned i = 0; i < camSysDataArray_.size(); i++) {
		std::shared_ptr<NSCam::SensorStaticInfo> s =
			std::shared_ptr<NSCam::SensorStaticInfo>(new NSCam::SensorStaticInfo);
		nscam_sensor_static_info_.push_back(s);
	}

	for (int i = 0; i < (int)nscam_sensor_static_info_.size(); ++i) {
		std::shared_ptr<NSCam::SensorStaticInfo> s = nscam_sensor_static_info_[i];
		construct_sensor_static_info(i, s);
	}
}

void SensorInfo::get_sensor_static_info(
	std::array<mtk::hal3a::SensorStaticInfo, kMaxSensorCnt> *
		nscam_sensor_static_info_array)
{
	*nscam_sensor_static_info_array = {};
	for (int i = 0; i < (int)kMaxSensorCnt; i++) {
		if (i >= (int)camSysDataArray_.size()) {
			break;
		}
		nscam_sensor_static_info_array->at(i).index = i;
		nscam_sensor_static_info_array->at(i).dev_id = 1 << i;
		nscam_sensor_static_info_array->at(i).sensor_id =
			nscam_sensor_static_info_[i]->sensorDevID;
		// TODO, Query module id from EEProm
		nscam_sensor_static_info_array->at(i).module_id = 0;
		nscam_sensor_static_info_array->at(i).orientation =
			nscam_sensor_static_info_[i]->facingDirection;
		nscam_sensor_static_info_array->at(i).info = *nscam_sensor_static_info_[i];
	}
}

int SensorInfo::get_cal_data(ENUM_CAMERA_CAM_CAL_TYPE_ENUM cal_enum,
			     void *a_pCamCalData)
{
	return CamCalHelper::getInstance(m_sensor_index)->get_cal_data(cal_enum, m_sensor_id, m_sensor_dev, a_pCamCalData);
}

bool SensorInfo::is_af_support()
{
	if (m_sensor_index >= camSysDataArray_.size()) {
		return false;
	} else {
		return camSysDataArray_[m_sensor_index].has_af;
	}
}

void SensorInfo::construct_sensor_static_info(
	int index, std::shared_ptr<NSCam::SensorStaticInfo> pSensorStaticInfo)
{
	int sensorId_idx = 0;
	switch (libcamera::PlatformUtils::platform_) {
	case libcamera::PlatformUtils::MtkISP7Platform::NONE:
		// TODO: add a fatal
		break;

	case libcamera::PlatformUtils::MtkISP7Platform::GOOGLE:
		sensorId_idx = sensorId_idx_map_geralt[index];
		break;

	case libcamera::PlatformUtils::MtkISP7Platform::LENOVO:
		sensorId_idx = sensorId_idx_map_ciri[index];
		break;
	}

	IMGSENSOR_SENSOR_IDX sensorIdx = (IMGSENSOR_SENSOR_IDX)index;
	struct imgsensor_info_struct *imgsensor_info;
	if (index >
	    int(sizeof(gImgsensor_info) / sizeof(struct imgsensor_info_struct))) {
		imgsensor_info = nullptr;
	} else {
		imgsensor_info = &gImgsensor_info[sensorId_idx];
	}
	querySensorInfo(sensorId_idx, sensorIdx, imgsensor_info, pSensorStaticInfo);
	pSensorStaticInfo->sensorMBusCode = camSysDataArray_[index].mbus_code;
}

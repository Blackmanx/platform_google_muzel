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

#ifndef HW_SENSOR_INCLUDE_IMGSENSOR_INFO_H_
#define HW_SENSOR_INCLUDE_IMGSENSOR_INFO_H_

#include "platform/mtkisp7/mtkcam-interfaces/include/kernel-headers//imgsensor-user.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/kernel-headers/kd_imgsensor_define_v4l2.h"
#include "platform/mtkisp7/mtkcam-core/libcamera/mt8188/include/libfdft_lib/MTKDetectionType.h"
//==========================================
#define USE_SENSOR_UPS 1
//==========================================
//typedef enum {
//	IMGSENSOR_MODE_INIT,
//	IMGSENSOR_MODE_PREVIEW,
//	IMGSENSOR_MODE_CAPTURE,
//	IMGSENSOR_MODE_VIDEO,
//	IMGSENSOR_MODE_HIGH_SPEED_VIDEO,
//	IMGSENSOR_MODE_SLIM_VIDEO,
//} IMGSENSOR_MODE;

struct imgsensor_mode_struct {
	MUINT32 pclk; /* record different mode's pclk */
	MUINT32 linelength; /* record different mode's linelength */
	MUINT32 framelength; /* record different mode's framelength */

	MUINT8 startx; /* record different mode's startx of grabwindow */
	MUINT8 starty; /* record different mode's startx of grabwindow */

	MUINT16 grabwindow_width; /* record different mode's width of grabwindow */
	MUINT16 grabwindow_height; /* record different mode's height of grabwindow */

	MUINT32 mipi_pixel_rate;

	/*       following for GetDefaultFramerateByScenario()  */
	MUINT16 max_framerate;

	MUINT32 margin;

	struct mtk_mbus_frame_desc_entry fd_entry[MTK_FRAME_DESC_ENTRY_MAX];
};

/* SENSOR PRIVATE STRUCT FOR VARIABLES*/
struct imgsensor_struct {
	MUINT8 mirror; /* mirrorflip information */

	MUINT8 sensor_mode; /* record IMGSENSOR_MODE enum value */

	MUINT32 shutter; /* current shutter */
	MUINT16 gain; /* current gain */

	MUINT32 pclk; /* current pclk */

	MUINT32 frame_length; /* current framelength */
	MUINT32 line_length; /* current linelength */

	MUINT32 min_frame_length; /* current min  framelength to max framerate */
	MUINT16 dummy_pixel; /* current dummypixel */
	MUINT16 dummy_line; /* current dummline */

	MUINT16 current_fps; /* current max fps */
	MBOOL autoflicker_en; /* record autoflicker enable or disable */
	MBOOL test_pattern; /* record test pattern mode or not */
	SENSOR_SCENARIO_ID_ENUM current_scenario_id; /* current scenario id */

	MUINT8 hdr_mode; /* HDR mode */
	MUINT8 pdaf_mode; /* ihdr enable or disable */
	//	MUINT8 i2c_write_id;	/* record current sensor's i2c write id */
};

/* SENSOR PRIVATE STRUCT FOR CONSTANT*/
struct imgsensor_info_struct {
	MUINT32 sensor_id; /* record sensor id defined in Kd_imgsensor.h */
	MUINT32 checksum_value; /* checksum value for Camera Auto Test */

	struct imgsensor_mode_struct pre; /* preview scenario relative information */
	struct imgsensor_mode_struct cap; /* capture scenario relative information */
	struct imgsensor_mode_struct normal_video; /* normal video  scenario relative information */
	struct imgsensor_mode_struct hs_video; /* high speed video scenario relative information */
	struct imgsensor_mode_struct slim_video; /* slim video for VT scenario relative information */
	struct imgsensor_mode_struct custom1;
	struct imgsensor_mode_struct custom2;
	struct imgsensor_mode_struct custom3;

	MUINT8 ae_shut_delay_frame; /* shutter delay frame for AE cycle */
	MUINT8 ae_sensor_gain_delay_frame; /* sensor gain delay frame for AE cycle */
	MUINT8 ae_ispGain_delay_frame; /* isp gain delay frame for AE cycle */
	MUINT8 ihdr_support; /* 1, support; 0,not support */
	MUINT8 ihdr_le_firstline; /* 1,le first ; 0, se first */
	MUINT8 temperature_support; /* 1, support; 0,not support */
	MUINT8 sensor_mode_num; /* support sensor mode num */
	MUINT8 frame_time_delay_frame;
	MUINT8 cap_delay_frame; /* enter capture delay frame num */
	MUINT8 pre_delay_frame; /* enter preview delay frame num */
	MUINT8 video_delay_frame; /* enter video delay frame num */
	MUINT8 hs_video_delay_frame; /* enter high speed video  delay frame num */
	MUINT8 slim_video_delay_frame; /* enter slim video delay frame num */
	MUINT8 custom1_delay_frame; /* enter custom1 delay frame num */
	MUINT8 custom2_delay_frame; /* enter custom1 delay frame num */
	MUINT8 custom3_delay_frame; /* enter custom1 delay frame num */
	MUINT8 margin; /* sensor framelength & shutter margin */
	MUINT32 min_shutter; /* min shutter */
	MUINT32 max_frame_length; /* max framelength by sensor register's limitation */
	MUINT32 min_gain;
	MUINT32 max_gain;
	MUINT32 min_ana_gain;
	MUINT32 max_ana_gain;
	MUINT32 min_gain_iso;
	MUINT32 gain_step;
	MUINT32 exp_step;
	MUINT32 gain_type;

	MUINT8 sensor_output_dataformat; /* sensor output first pixel color */

	MUINT8 isp_driving_current; /* mclk driving current */
	MUINT8 sensor_interface_type; /* sensor_interface_type */
	MUINT8 mipi_sensor_type;
	MUINT8 mipi_settle_delay_mode;
	MUINT8 mipi_lane_num; /* mipi lane num */

	MUINT8 mclk; /* mclk value, suggest 24 or 26 for 24Mhz or 26Mhz */

	//	MUINT32 i2c_speed;	/* i2c speed */
	//	MUINT8 i2c_addr_table[5];	/* record sensor support all write id addr, only supprt 4must end with 0xff */
	MUINT8 ob_flag;
};

MUINT32 getSensorListIdx_by_Id(MUINT32 Id);
MUINT32 getSensorListId(MUINT8 listIdx);
char *getSensorListName(MUINT8 listIdx);
int g_sensor_info(int sensorlist_idx, struct mtk_sensor_info *info);

#endif

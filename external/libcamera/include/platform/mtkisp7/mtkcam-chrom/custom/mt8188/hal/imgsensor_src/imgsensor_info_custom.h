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

#ifndef HW_SENSOR_INCLUDE_IMGSENSOR_INFO_CUSTOM_H_
#define HW_SENSOR_INCLUDE_IMGSENSOR_INFO_CUSTOM_H_

#include "platform/mtkisp7/mtkcam-interfaces/include/kernel-headers/kd_imgsensor.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/kernel-headers/kd_imgsensor_define_v4l2.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/mtkcam-interfaces/hw/sensor/imgsensor_info.h"

/* 30, 85, 120, 14 */
#define SETTLE_DELAY 180

#define GAIN_TBL_IDX_MAX     16

//===================================================================
struct PLATFORM_IMGSENSOR_CFG gimgsensor_cfg_list[MAX_PLATFORM_NUM][MAX_SENSOR_IN_PLATFORM] =
{
    { // google proto device:
        {SENSOR_DRVNAME_HI1339_MIPI_RAW, "sensor0"}, //gimgsensor_cfg_list[0][0]
        {SENSOR_DRVNAME_GC08A3_MIPI_RAW, "sensor1"}  //gimgsensor_cfg_list[0][1]
    },
    { // Lenovo proto device:
        {SENSOR_DRVNAME_GC08A3_MIPI_RAW, "sensor0"}, //gimgsensor_cfg_list[1][0]
        {SENSOR_DRVNAME_GC05A2_MIPI_RAW, "sensor1"}  //gimgsensor_cfg_list[1][1]
    },

        // TODO:  ADD more other sensor info
        /*  ADD sensor driver before this line */
};

struct IMGSENSOR_SENSOR_LIST gimgsensor_sensor_list[MAX_NUM_OF_SUPPORT_SENSOR] =
{
  {GC08A3_SENSOR_ID, SENSOR_DRVNAME_GC08A3_MIPI_RAW, NULL},
  {HI1339_SENSOR_ID, SENSOR_DRVNAME_HI1339_MIPI_RAW, NULL},
  {GC05A2_SENSOR_ID, SENSOR_DRVNAME_GC05A2_MIPI_RAW, NULL},

  // TODO:  ADD more other sensor info
  /*  ADD sensor driver before this line */
  {0, {0}, NULL}, /* end of list */
};

// sensor order of win size info  must match with gImgsensor_info
static SENSOR_WINSIZE_INFO_STRUCT gImgsensor_winsize_info[][SENSOR_SCENARIO_ID_MAX] = {
    {
	// gc08a3
	{3264, 2448, 0, 0, 3264, 2448, 3264, 2448,
	 0000, 0000, 3264, 2448, 0, 0, 3264, 2448},  /* Preview */
	{3264, 2448, 0, 0, 3264, 2448, 3264, 2448,
	 0000, 0000, 3264, 2448, 0, 0, 3264, 2448},  /* capture */
	{3264, 2448, 0, 0, 3264, 2448, 3264, 2448,
	 0000, 0000, 3264, 2448, 0, 0, 3264, 2448},  /* video */
	{3264, 2448, 0, 0, 1920, 1080, 1920, 1080,
	 0000, 0000, 1920, 1080, 0, 0, 1920, 1080},  /* hight speed video */
	{3264, 2448, 0, 0, 3264, 2448, 3264, 2448,
	 0000, 0000, 3264, 2448, 0, 0, 3264, 2448} /* slim video */
  },
  {
	// hi1339: 4208x3120
	{4208, 3120, 0, 0, 4208, 3120, 4208, 3120,
	 0000, 0000, 4208, 3120, 0, 0, 4208, 3120},  /* Preview */
	{4208, 3120, 0, 0, 4208, 3120, 4208, 3120,
	 0000, 0000, 4208, 3120, 0, 0, 4208, 3120},  /* capture */
	{4208, 3120, 0, 0, 4208, 3120, 4208, 3120,
	 0000, 0000, 4208, 3120, 0, 0, 4208, 3120},  /* video */
	{4208, 3120, 0, 0, 1920, 1080, 1920, 1080,
	 0000, 0000, 1920, 1080, 0, 0, 1920, 1080},  /* hight speed video */
	{4208, 3120, 0, 0, 4208, 3120, 4208, 3120,
	 0000, 0000, 4208, 3120, 0, 0, 4208, 3120} /* slim video */
  },
  {
    // gc05a2
    {2592, 1944, 0, 0, 2592, 1944, 2592, 1944,
     0000, 0000, 2592, 1944, 0, 0, 2592, 1944},  /* Preview */
    {2592, 1944, 0, 0, 2592, 1944, 2592, 1944,
     0000, 0000, 2592, 1944, 0, 0, 2592, 1944},  /* capture */
    {2592, 1944, 0, 0, 2592, 1944, 2592, 1944,
     0000, 0000, 2592, 1944, 0, 0, 2592, 1944},  /* video */
    {2592, 1944, 0, 0, 1280, 720, 1280, 720,
     0000, 0000, 1280, 720, 0, 0, 1280, 720},  /* hight speed video */
    {2592, 1944, 0, 0, 2592, 1944, 2592, 1944,
     0000, 0000, 2592, 1944, 0, 0, 2592, 1944} /* slim video */
  },

  // TODO:	ADD more other sensor info
};


static MUINT32 gImgsensor_gain_tbl[][GAIN_TBL_IDX_MAX] = {
    {
        // gc08a3
        1024*1,  1024*2,  1024*3,   1024*4,
        1024*5,  1024*6,  1024*7,   1024*8,
        1024*9,  1024*10, 1024*11,  1024*12,
        1024*13, 1024*14, 1024*15,  1024*16
    },
    {
        // hi1339
        1024*1,  1024*2,  1024*3,   1024*4,
        1024*5,  1024*6,  1024*7,   1024*8,
        1024*9,  1024*10, 1024*11,  1024*12,
        1024*13, 1024*14, 1024*15,  1024*16
    },
    {
        // gc05a2
        1024*1,  1024*2,  1024*3,   1024*4,
        1024*5,  1024*6,  1024*7,   1024*8,
        1024*9,  1024*10, 1024*11,  1024*12,
        1024*13, 1024*14, 1024*15,  1024*16
    },

    // TODO:  ADD more other sensor info
};


struct imgsensor_info_struct gImgsensor_info[] = {
    //===============================================
    {
    .sensor_id = GC08A3_SENSOR_ID,  /* record sensor id defined in Kd_imgsensor.h */
    .checksum_value = 0x6252c5ee,   /* checksum value for Camera Auto Test */

    .pre = {
       .pclk = 280000000,   /* 45000000      //record different mode's pclk */
       .linelength = 3640,  /* record different mode's linelength */
       .framelength = 2548,    /* record different mode's framelength */
       .startx = 0,    /* record different mode's startx of grabwindow */
       .starty = 0,    /* record different mode's starty of grabwindow */
       .grabwindow_width = 3264,   //record different mode's width of grabwindow */
       .grabwindow_height = 2448,  //record different mode's height of grabwindow */
       .mipi_pixel_rate = 672000000,
       .max_framerate = 300,
       .margin = 0,
       .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = IMGSENSOR_VC_RAW10,
                .enable = 0,
                .hsize = 3264,
                .vsize = 2448,
                .user_data_desc = 0,
            },
       },

    },
    .cap = {
       .pclk = 280000000,
       .linelength = 3640,
       .framelength = 2548,
       .startx = 0,
       .starty = 0,
       .grabwindow_width = 3264,
       .grabwindow_height = 2448,
       .mipi_pixel_rate = 672000000,
       .max_framerate = 300,
       .margin = 0,
       .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .normal_video = {
       .pclk = 280000000,
       .linelength = 3640,
       .framelength = 2548,
       .startx = 0,
       .starty = 0,
       .grabwindow_width = 3264,
       .grabwindow_height = 2448,
       .mipi_pixel_rate = 672000000,
       .max_framerate = 300,
       .margin = 0,
       .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .hs_video = {
       .pclk = 280000000,
       .linelength = 3640,
       .framelength = 1276,
       .startx = 0,
       .starty = 0,
       .grabwindow_width = 1920,
       .grabwindow_height = 1080,
       .mipi_pixel_rate = 414000000,
       .max_framerate = 600,
       .margin = 0,
       .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = IMGSENSOR_VC_RAW10,
                .enable = 0,
                .hsize = 1920,
                .vsize = 1080,
                .user_data_desc = 0,
           },
       },

    },
    .slim_video = {
        .pclk = 45000000,
        .linelength = 750,
        .framelength = 2000,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 1280,
        .grabwindow_height = 960,
        .mipi_pixel_rate = 87000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .custom1 = {
        .pclk = 45000000,
        .linelength = 750,
        .framelength = 2000,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 1280,
        .grabwindow_height = 960,
        .mipi_pixel_rate = 87000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .custom2 = {
        .pclk = 45000000,
        .linelength = 750,
        .framelength = 2000,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 1280,
        .grabwindow_height = 960,
        .mipi_pixel_rate = 87000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .custom3 = {
        .pclk = 45000000,
        .linelength = 750,
        .framelength = 2000,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 1280,
        .grabwindow_height = 960,
        .mipi_pixel_rate = 87000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .ae_shut_delay_frame = 0,
    .ae_sensor_gain_delay_frame = 0,
    .ae_ispGain_delay_frame = 2,    /* isp gain delay frame for AE cycle */
    .ihdr_support = 0,
    .ihdr_le_firstline = 0,
    .temperature_support = 0,
    .sensor_mode_num = 5,   /* support sensor mode num */
    .frame_time_delay_frame = 3,
    .cap_delay_frame = 2,   /* enter capture delay frame num */
    .pre_delay_frame = 2,   /* enter preview delay frame num */
    .video_delay_frame = 2, /* enter video delay frame num */
    .hs_video_delay_frame = 2,  /* enter high speed video  delay frame num */
    .slim_video_delay_frame = 2,    /* enter slim video delay frame num */
    .custom1_delay_frame = 0,
    .custom2_delay_frame = 0,
    .custom3_delay_frame = 0,
    .margin = 16,        /* sensor framelength & shutter margin */
    .min_shutter = 4,   /* min shutter */
    .max_frame_length = 0x7FFF, /* max framelength by sensor register's limitation */
    .min_gain = 1024,
    .max_gain = 1024*16,
    .min_ana_gain = 1024,
    .max_ana_gain = 1024*16,
    .min_gain_iso = 100,
    .gain_step = 1,
    .exp_step = 2,
    .gain_type = 3,

    .sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_R, /* sensor output first pixel color */
    .isp_driving_current = 0,
    .sensor_interface_type = 0,
    .mipi_sensor_type = 0,
    .mipi_settle_delay_mode = 0,
    .mipi_lane_num = 0,
    .mclk = 0,
    .ob_flag = 0,
    },
    //===============================================
    //===============================================
    {
    .sensor_id = HI1339_SENSOR_ID,  /* record sensor id defined in Kd_imgsensor.h */
    .checksum_value = 0x6252c5ee,   /* checksum value for Camera Auto Test */

    .pre = { //4208x3120
        .pclk = 72000000,   /*       //record different mode's pclk */
        .linelength = 720,  /* record different mode's linelength */
        .framelength = 3332,    /* record different mode's framelength */
        .startx = 0,    /* record different mode's startx of grabwindow */
        .starty = 0,    /* record different mode's starty of grabwindow */
        .grabwindow_width = 4208,   //record different mode's width of grabwindow */
        .grabwindow_height = 3120,  //record different mode's height of grabwindow */
        .mipi_pixel_rate = 1440000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
            .bus.csi2 = {
                .channel = 0,
                .data_type = IMGSENSOR_VC_RAW10,
                .enable = 0,
                .hsize = 4208,
                .vsize = 3120,
                .user_data_desc = 0,
            },
        },

    },
    .cap = {
        .pclk = 72000000,
        .linelength = 720,
        .framelength = 3332,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 4208,
        .grabwindow_height = 3120,
        .mipi_pixel_rate = 1440000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .normal_video = {
        .pclk = 72000000,
        .linelength = 720,
        .framelength = 3332,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 4208,
        .grabwindow_height = 3120,
        .mipi_pixel_rate = 1440000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .hs_video = {
        .pclk = 72000000,
        .linelength = 720,
        .framelength = 1666,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 1920,
        .grabwindow_height = 1080,
        .mipi_pixel_rate = 624000000,
        .max_framerate = 600,
        .margin = 0,
        .fd_entry[0] = {
            .bus.csi2 = {
                .channel = 0,
                .data_type = IMGSENSOR_VC_RAW10,
                .enable = 0,
                .hsize = 1920,
                .vsize = 1080,
                .user_data_desc = 0,
            },
        },

    },
    .slim_video = {
        .pclk = 45000000,
        .linelength = 750,
        .framelength = 2000,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 1280,
        .grabwindow_height = 960,
        .mipi_pixel_rate = 87000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .custom1 = {
        .pclk = 45000000,
        .linelength = 750,
        .framelength = 2000,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 1280,
        .grabwindow_height = 960,
        .mipi_pixel_rate = 87000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .custom2 = {
        .pclk = 45000000,
        .linelength = 750,
        .framelength = 2000,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 1280,
        .grabwindow_height = 960,
        .mipi_pixel_rate = 87000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .custom3 = {
        .pclk = 45000000,
        .linelength = 750,
        .framelength = 2000,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 1280,
        .grabwindow_height = 960,
        .mipi_pixel_rate = 87000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .ae_shut_delay_frame = 0,
    .ae_sensor_gain_delay_frame = 0,
    .ae_ispGain_delay_frame = 2,    /* isp gain delay frame for AE cycle */
    .ihdr_support = 0,
    .ihdr_le_firstline = 0,
    .temperature_support = 0,
    .sensor_mode_num = 5,   /* support sensor mode num */
    .frame_time_delay_frame = 3,
    .cap_delay_frame = 2,   /* enter capture delay frame num */
    .pre_delay_frame = 2,   /* enter preview delay frame num */
    .video_delay_frame = 2, /* enter video delay frame num */
    .hs_video_delay_frame = 2,  /* enter high speed video  delay frame num */
    .slim_video_delay_frame = 2,    /* enter slim video delay frame num */
    .custom1_delay_frame = 0,
    .custom2_delay_frame = 0,
    .custom3_delay_frame = 0,
    .margin = 16,        /* sensor framelength & shutter margin */
    .min_shutter = 4,   /* min shutter */
    .max_frame_length = 0x7FFF, /* max framelength by sensor register's limitation */
    .min_gain = 1024,
    .max_gain = 1024*16,
    .min_ana_gain = 1024,
    .max_ana_gain = 1024*16,
    .min_gain_iso = 100,
    .gain_step = 1,
    .exp_step = 2,
    .gain_type = 3,

    .sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_Gb, /* sensor output first pixel color */
    .isp_driving_current = 0,
    .sensor_interface_type = 0,
    .mipi_sensor_type = 0,
    .mipi_settle_delay_mode = 0,
    .mipi_lane_num = 0,
    .mclk = 0,
    .ob_flag = 0,
    },
    //===============================================
    //===============================================
    {
    .sensor_id = GC05A2_SENSOR_ID,  /* record sensor id defined in Kd_imgsensor.h */
    .checksum_value = 0x6252c5ee,   /* checksum value for Camera Auto Test */

    .pre = {
       .pclk = 224000000,   /*       //record different mode's pclk */
       .linelength = 3664,  /* record different mode's linelength */
       .framelength = 2032,    /* record different mode's framelength */
       .startx = 0,    /* record different mode's startx of grabwindow */
       .starty = 0,    /* record different mode's starty of grabwindow */
       .grabwindow_width = 2592,   //record different mode's width of grabwindow */
       .grabwindow_height = 1944,  //record different mode's height of grabwindow */
       .mipi_pixel_rate = 179200000,
       .max_framerate = 300,
       .margin = 0,
       .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = IMGSENSOR_VC_RAW10,
                .enable = 0,
                .hsize = 2592,
                .vsize = 1944,
                .user_data_desc = 0,
            },
       },

    },
    .cap = {
       .pclk = 224000000,
       .linelength = 3664,
       .framelength = 2032,
       .startx = 0,
       .starty = 0,
       .grabwindow_width = 2592,
       .grabwindow_height = 1944,
       .mipi_pixel_rate = 179200000,
       .max_framerate = 300,
       .margin = 0,
       .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .normal_video = {
       .pclk = 224000000,
       .linelength = 3664,
       .framelength = 2032,
       .startx = 0,
       .starty = 0,
       .grabwindow_width = 2592,
       .grabwindow_height = 1944,
       .mipi_pixel_rate = 179200000,
       .max_framerate = 300,
       .margin = 0,
       .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .hs_video = {
       .pclk = 224000000,
       .linelength = 3616,
       .framelength = 1032,
       .startx = 0,
       .starty = 0,
       .grabwindow_width = 1280,
       .grabwindow_height = 720,
       .mipi_pixel_rate = 89600000,
       .max_framerate = 600,
       .margin = 0,
       .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = IMGSENSOR_VC_RAW10,
                .enable = 0,
                .hsize = 1280,
                .vsize = 720,
                .user_data_desc = 0,
           },
       },

    },
    .slim_video = {
        .pclk = 224000000,
        .linelength = 750,
        .framelength = 2000,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 1280,
        .grabwindow_height = 960,
        .mipi_pixel_rate = 87000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .custom1 = {
        .pclk = 45000000,
        .linelength = 750,
        .framelength = 2000,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 1280,
        .grabwindow_height = 960,
        .mipi_pixel_rate = 87000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .custom2 = {
        .pclk = 45000000,
        .linelength = 750,
        .framelength = 2000,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 1280,
        .grabwindow_height = 960,
        .mipi_pixel_rate = 87000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .custom3 = {
        .pclk = 45000000,
        .linelength = 750,
        .framelength = 2000,
        .startx = 0,
        .starty = 0,
        .grabwindow_width = 1280,
        .grabwindow_height = 960,
        .mipi_pixel_rate = 87000000,
        .max_framerate = 300,
        .margin = 0,
        .fd_entry[0] = {
           .bus.csi2 = {
                .channel = 0,
                .data_type = 0,
                .enable = 0,
                .hsize = 0,
                .vsize = 0,
                .user_data_desc = 0,
            },
       },
    },
    .ae_shut_delay_frame = 0,
    .ae_sensor_gain_delay_frame = 0,
    .ae_ispGain_delay_frame = 2,    /* isp gain delay frame for AE cycle */
    .ihdr_support = 0,
    .ihdr_le_firstline = 0,
    .temperature_support = 0,
    .sensor_mode_num = 5,   /* support sensor mode num */
    .frame_time_delay_frame = 3,
    .cap_delay_frame = 2,   /* enter capture delay frame num */
    .pre_delay_frame = 2,   /* enter preview delay frame num */
    .video_delay_frame = 2, /* enter video delay frame num */
    .hs_video_delay_frame = 2,  /* enter high speed video  delay frame num */
    .slim_video_delay_frame = 2,    /* enter slim video delay frame num */
    .custom1_delay_frame = 0,
    .custom2_delay_frame = 0,
    .custom3_delay_frame = 0,
    .margin = 16,        /* sensor framelength & shutter margin */
    .min_shutter = 4,   /* min shutter */
    .max_frame_length = 0x7FFF, /* max framelength by sensor register's limitation */
    .min_gain = 1024,
    .max_gain = 1024*16,
    .min_ana_gain = 1024,
    .max_ana_gain = 1024*16,
    .min_gain_iso = 100,
    .gain_step = 1,
    .exp_step = 2,
    .gain_type = 3,

    .sensor_output_dataformat = SENSOR_OUTPUT_FORMAT_RAW_Gr, /* sensor output first pixel color */
    .isp_driving_current = 0,
    .sensor_interface_type = 0,
    .mipi_sensor_type = 0,
    .mipi_settle_delay_mode = 0,
    .mipi_lane_num = 0,
    .mclk = 0,
    .ob_flag = 0,
    }
        //===============================================

    // TODO:  ADD more other sensor info

};

#endif

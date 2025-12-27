/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#ifndef __MTK_CAMERA_V4l2_CONTROLS_H
#define __MTK_CAMERA_V4l2_CONTROLS_H

#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>

/* Allowed value of V4L2_CID_MTK_CAM_RAW_PATH_SELECT */
#define V4L2_MTK_CAM_RAW_PATH_SELECT_BPC	1
#define V4L2_MTK_CAM_RAW_PATH_SELECT_FUS	3
#define V4L2_MTK_CAM_RAW_PATH_SELECT_DGN	4
#define V4L2_MTK_CAM_RAW_PATH_SELECT_LSC	5
#define V4L2_MTK_CAM_RAW_PATH_SELECT_LTM	7

/* exposure for m-stream */
struct mtk_cam_shutter_gain {
        __u32 shutter;
        __u32 gain;
};

/* multiple exposure for m-stream(2 exposures) */
struct mtk_cam_mstream_exposure {
        struct mtk_cam_shutter_gain exposure[2];
        unsigned int valid;
};

#define V4L2_MBUS_FRAMEFMT_PAD_ENABLE  BIT(1)

#define MEDIA_BUS_FMT_MTISP_SBGGR10_1X10		0x8001
#define MEDIA_BUS_FMT_MTISP_SBGGR12_1X12		0x8002
#define MEDIA_BUS_FMT_MTISP_SBGGR14_1X14		0x8003
#define MEDIA_BUS_FMT_MTISP_SGBRG10_1X10		0x8004
#define MEDIA_BUS_FMT_MTISP_SGBRG12_1X12		0x8005
#define MEDIA_BUS_FMT_MTISP_SGBRG14_1X14		0x8006
#define MEDIA_BUS_FMT_MTISP_SGRBG10_1X10		0x8007
#define MEDIA_BUS_FMT_MTISP_SGRBG12_1X12		0x8008
#define MEDIA_BUS_FMT_MTISP_SGRBG14_1X14		0x8009
#define MEDIA_BUS_FMT_MTISP_SRGGB10_1X10		0x800a
#define MEDIA_BUS_FMT_MTISP_SRGGB12_1X12		0x800b
#define MEDIA_BUS_FMT_MTISP_SRGGB14_1X14		0x800c
#define MEDIA_BUS_FMT_MTISP_BAYER8_UFBC			0x800d
#define MEDIA_BUS_FMT_MTISP_BAYER10_UFBC		0x800e
#define MEDIA_BUS_FMT_MTISP_BAYER12_UFBC		0x8010
#define MEDIA_BUS_FMT_MTISP_BAYER14_UFBC		0x8011
#define MEDIA_BUS_FMT_MTISP_BAYER16_UFBC		0x8012
#define MEDIA_BUS_FMT_MTISP_NV12			0x8013
#define MEDIA_BUS_FMT_MTISP_NV21			0x8014
#define MEDIA_BUS_FMT_MTISP_NV12_10			0x8015
#define MEDIA_BUS_FMT_MTISP_NV21_10			0x8016
#define MEDIA_BUS_FMT_MTISP_NV12_10P			0x8017
#define MEDIA_BUS_FMT_MTISP_NV21_10P			0x8018
#define MEDIA_BUS_FMT_MTISP_NV12_12			0x8019
#define MEDIA_BUS_FMT_MTISP_NV21_12			0x801a
#define MEDIA_BUS_FMT_MTISP_NV12_12P			0x801b
#define MEDIA_BUS_FMT_MTISP_NV21_12P			0x801c
#define MEDIA_BUS_FMT_MTISP_YUV420			0x801d
#define MEDIA_BUS_FMT_MTISP_NV12_UFBC			0x801e
#define MEDIA_BUS_FMT_MTISP_NV21_UFBC			0x8020
#define MEDIA_BUS_FMT_MTISP_NV12_10_UFBC		0x8021
#define MEDIA_BUS_FMT_MTISP_NV21_10_UFBC		0x8022
#define MEDIA_BUS_FMT_MTISP_NV12_12_UFBC		0x8023
#define MEDIA_BUS_FMT_MTISP_NV21_12_UFBC		0x8024
#define MEDIA_BUS_FMT_MTISP_NV16			0x8025
#define MEDIA_BUS_FMT_MTISP_NV61			0x8026
#define MEDIA_BUS_FMT_MTISP_NV16_10			0x8027
#define MEDIA_BUS_FMT_MTISP_NV61_10			0x8028
#define MEDIA_BUS_FMT_MTISP_NV16_10P			0x8029
#define MEDIA_BUS_FMT_MTISP_NV61_10P			0x802a

#define MTK_CAM_RESOURCE_DEFAULT	0xFFFF

/*
 * struct mtk_cam_resource_sensor - sensor resoruces for format negotiation
 *
 */
struct mtk_cam_resource_sensor {
	struct v4l2_fract interval;
	__u32 hblank;
	__u32 vblank;
	__u64 pixel_rate;
	__u64 cust_pixel_rate;
};

/*
 * struct mtk_cam_resource_raw - MTK camsys raw resoruces for format negotiation
 *
 * @feature: value of V4L2_CID_MTK_CAM_FEATURE the user want to check the
 *		  resource with. If it is used in set CTRL, we will apply the value
 *		  to V4L2_CID_MTK_CAM_FEATURE ctrl directly.
 * @strategy: indicate the order of multiple raws, binning or DVFS to be selected
 *	      when doing format negotiation of raw's source pads (output pads).
 *	      Please pass MTK_CAM_RESOURCE_DEFAULT if you want camsys driver to
 *	      determine it.
 * @raw_max: indicate the max number of raw to be used for the raw pipeline.
 *	     Please pass MTK_CAM_RESOURCE_DEFAULT if you want camsys driver to
 *	     determine it.
 * @raw_min: indicate the max number of raw to be used for the raw pipeline.
 *	     Please pass MTK_CAM_RESOURCE_DEFAULT if you want camsys driver to
 *	     determine it.
 * @raw_used: The number of raw used. The used don't need to writ this failed,
 *	      the driver always updates the field.
 * @bin: indicate if the driver should enable the bining or not. The driver
 *	 update the field depanding the hardware supporting status. Please pass
 *	 MTK_CAM_RESOURCE_DEFAULT if you want camsys driver to determine it.
 * @path_sel: indicate the user selected raw path. The driver
 *	      update the field depanding the hardware supporting status. Please
 *	      pass MTK_CAM_RESOURCE_DEFAULT if you want camsys driver to
 *	      determine it.
 * @pixel_mode: the pixel mode driver used in the raw pipeline. It is written by
 *		driver only.
 * @throughput: the throughput be used in the raw pipeline. It is written by
 *		driver only.
 *
 */
struct mtk_cam_resource_raw {
	__s64	feature;
	__u16	strategy;
	__u8	raw_max;
	__u8	raw_min;
	__u8	raw_used;
	__u8	bin;
	__u8	path_sel;
	__u8	pixel_mode;
	__u64	throughput;
};

/*
 * struct mtk_cam_resource - MTK camsys resoruces for format negotiation
 *
 * @sink_fmt: sink_fmt pad's format, it must be return by g_fmt or s_fmt
 *		from driver.
 * @sensor_res: senor information to calculate the required resource, it is
 *		read-only and camsys driver will not change it.
 * @raw_res: user hint and resource negotiation result.
 * @status:	TBC
 *
 */
struct mtk_cam_resource {
	__u64 sink_fmt;
	struct mtk_cam_resource_sensor sensor_res;
	struct mtk_cam_resource_raw raw_res;
	__u8 status;
};

/**
 * struct mtk_cam_pde_info - PDE module information for raw
 *
 * @pdo_max_size: the max pdo size of pde sensor.
 * @pdi_max_size: the max pdi size of pde sensor or max pd table size.
 * @pd_table_offset: the offest of meta config for pd table content.
 */
struct mtk_cam_pde_info {
	__u32 pdo_max_size;
	__u32 pdi_max_size;
	__u32 pd_table_offset;
};
#endif /* __MTK_CAMERA_V4l2_CONTROLS_H */

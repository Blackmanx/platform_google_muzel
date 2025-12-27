/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Google Inc.
 *
 * vendor_tag.h - libcamera Android Vendor Tag
 */

#pragma once

#include <map>
#include <stddef.h>
#include <tuple>
#include <vector>

#include <hardware/camera_common.h>
#include <hardware/hardware.h>
#include <system/camera_metadata.h>

#include <libcamera/base/class.h>
#include <libcamera/base/mutex.h>

#include <libcamera/camera_manager.h>
#include <system/camera_vendor_tags.h>

#include "camera_hal_config.h"
extern vendor_tag_ops hal_vendor_tag_ops;

#define VENDOR_TAG_SECTION_START 0x80080000

typedef enum vendor_metadata_tag {
	VENDOR_TAG_STILL_CAPTURE_MULTI_FRAME_NOISE_REDUCTION =
		VENDOR_TAG_SECTION_START,
	VENDOR_TAG_SECTION_END,
} vendor_metadata_tag_t;

#define VENDOR_SECTION_COUNT (VENDOR_TAG_SECTION_END - VENDOR_TAG_SECTION_START)

// VENDOR_TAG_STILL_CAPTURE_MULTI_FRAME_NOISE_REDUCTION
typedef enum vendor_metadata_enum_vendor_still_capture_multiframe_noise_reduction {
	VENDOR_TAG_STILL_CAPTURE_MULTI_FRAME_NOISE_REDUCTION_OFF,
	VENDOR_TAG_STILL_CAPTURE_MULTI_FRAME_NOISE_REDUCTION_ON,
} vendor_metadata_enum_vendor_still_capture_multiframe_noise_reduction_t;

/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Google Inc.
 *
 * vendor_tag.cc - libcamera Android Vendor Tag
 */

#include "vendor_tag.h"

#include <libcamera/base/log.h>

#include <libcamera/camera.h>
#include <libcamera/property_ids.h>

#include "camera_device.h"

using namespace libcamera;

LOG_DECLARE_CATEGORY(HAL)

/** Tag information */
typedef struct tag_info {
	const char* tag_name;
	uint8_t tag_type;
} tag_info_t;

static tag_info_t vendor_tag_info[VENDOR_SECTION_COUNT] = {
	{"stillCaptureMFNR", TYPE_BYTE},
};

static int getTagCount([[maybe_unused]] const vendor_tag_ops_t* v)
{
	return VENDOR_SECTION_COUNT;
}

static void getAllTags([[maybe_unused]] const vendor_tag_ops_t* v, uint32_t* tag_array)
{
	uint32_t tag = VENDOR_TAG_SECTION_START;
	for (unsigned int i = 0; i < VENDOR_SECTION_COUNT; i++)
		*tag_array++ = tag++;
}

static const char* getSectionName([[maybe_unused]] const vendor_tag_ops_t* v,
				  [[maybe_unused]] uint32_t tag)
{
	return "org.libcamera";
}

static const char* getTagName([[maybe_unused]] const vendor_tag_ops_t* v, uint32_t tag)
{
	if (tag >= VENDOR_TAG_SECTION_START && tag < VENDOR_TAG_SECTION_END)
		return vendor_tag_info[tag - VENDOR_TAG_SECTION_START].tag_name;

	return nullptr;
}

static int getTagType([[maybe_unused]] const vendor_tag_ops_t* v, uint32_t tag)
{
	if (tag >= VENDOR_TAG_SECTION_START && tag < VENDOR_TAG_SECTION_END)
		return vendor_tag_info[tag - VENDOR_TAG_SECTION_START].tag_type;

	return 0;
}

vendor_tag_ops hal_vendor_tag_ops = {
	.get_tag_count = getTagCount,
	.get_all_tags = getAllTags,
	.get_section_name = getSectionName,
	.get_tag_name = getTagName,
	.get_tag_type = getTagType,
	.reserved = { nullptr },
};

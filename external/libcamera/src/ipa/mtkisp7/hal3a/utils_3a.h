/*
 * Copyright (C) 2024, Google Inc.
 *
 * utils_3a.h - Utilities and static data for 3A.
 */

#pragma once

#include <array>

#include "peripheralcontroller/include/PeripheralInfoDef.h"

namespace libcamera {

void getInitialDynamicInfo(uint32_t m_sensor_index,
			   mtk::hal3a::SensorInitialDynamicInfo
				   &sensor_init_dynamic_info);

} /* namespace libcamera */

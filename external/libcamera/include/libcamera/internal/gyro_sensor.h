/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Google Inc.
 *
 * gyro_sensor.h - A gyroscope sensor
 */

#pragma once

#include "libcamera/base/class.h"
namespace libcamera {

class GyroSensor final : public libcamera::Extensible
{
	LIBCAMERA_DECLARE_PRIVATE()

public:
	enum class Location {
		kNone = 0, // The device doesn't have the location attribute.
		kBase = 1,
		kLid = 2,
		kCamera = 3,
	};

	struct SensorSample {
		double x_value;
		double y_value;
		double z_value;
		int64_t timestamp;
	};

	GyroSensor();

	int init(Location location);

	bool startReading(double frequency);
	void stopReading();

	SensorSample getLatestSample();
};

#define PUBLIC_GYRO_SENSOR_IMPLEMENTATION                      \
	GyroSensor::GyroSensor()                               \
		: Extensible(std::make_unique<Private>())      \
	{                                                      \
	}                                                      \
	int GyroSensor::init(Location location)                \
	{                                                      \
		return _d()->init(location);                   \
	}                                                      \
	bool GyroSensor::startReading(double frequency)        \
	{                                                      \
		return _d()->startReading(frequency);          \
	}                                                      \
	void GyroSensor::stopReading()                         \
	{                                                      \
		_d()->stopReading();                           \
	}                                                      \
	GyroSensor::SensorSample GyroSensor::getLatestSample() \
	{                                                      \
		return _d()->getLatestSample();                \
	}

} /* namespace libcamera */

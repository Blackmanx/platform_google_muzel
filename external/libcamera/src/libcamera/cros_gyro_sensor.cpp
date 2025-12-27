/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Google Inc.
 *
 * cros_gyro_sensor.cpp - Chromium OS gyroscope sensor using mojo
 */

#include <libcamera/base/log.h>

#include "libcamera/internal/gyro_sensor.h"

#include <cros-camera/sensor_hal_client.h>

#include "../android/cros_mojo_token.h"
#include "libcamera/base/mutex.h"
#include "libcamera/base/thread_annotations.h"

namespace libcamera {

namespace {
std::string convertErrorTypeToString(cros::SamplesObserver::ErrorType error)
{
	switch (error) {
	case cros::SamplesObserver::ErrorType::MOJO_DISCONNECTED:
		return "MOJO_DISCONNECTED";

	case cros::SamplesObserver::ErrorType::READ_FAILED:
		return "READ_FAILED";

	case cros::SamplesObserver::ErrorType::INVALID_ARGUMENT:
		return "INVALID_ARGUMENT";

	case cros::SamplesObserver::ErrorType::DEVICE_REMOVED:
		return "DEVICE_REMOVED";
	}
}

} /* namespace */

LOG_DEFINE_CATEGORY(CrosGyroSensor)

class GyroSensor::Private : public Extensible::Private,
			    public cros::SamplesObserver
{
	LIBCAMERA_DECLARE_PUBLIC(GyroSensor)

public:
	Private();

	int init(Location location);

	bool startReading(double frequency);
	void stopReading();

	SensorSample getLatestSample();

	void OnSampleUpdated(cros::SamplesObserver::Sample sample) override;
	void OnErrorOccurred(cros::SamplesObserver::ErrorType error) override;

private:
	cros::SensorHalClient *sensorHalClient_;
	cros::SensorHalClient::Location location_;

	libcamera::Mutex gyroSampleMutex_;
	SensorSample gyroSample_ LIBCAMERA_TSA_GUARDED_BY(gyroSampleMutex_);
};

GyroSensor::Private::Private()
{
}

int GyroSensor::Private::init(Location location)
{
	sensorHalClient_ = cros::SensorHalClient::GetInstance(gCrosMojoToken);
	location_ = static_cast<cros::SensorHalClient::Location>(location);
	if (sensorHalClient_ &&
	    sensorHalClient_->HasDevice(
		    cros::SensorHalClient::DeviceType::kAnglVel, location_)) {
		return 0;
	}

	return -ENXIO;
}

bool GyroSensor::Private::startReading(double frequency)
{
	return sensorHalClient_->RegisterSamplesObserver(
		cros::SensorHalClient::DeviceType::kAnglVel,
		location_, frequency, this);
}

void GyroSensor::Private::stopReading()
{
	sensorHalClient_->UnregisterSamplesObserver(this);
}

GyroSensor::SensorSample GyroSensor::Private::getLatestSample()
{
	MutexLocker lock(gyroSampleMutex_);

	return gyroSample_;
}

void GyroSensor::Private::OnSampleUpdated(cros::SamplesObserver::Sample sample)
{
	MutexLocker lock(gyroSampleMutex_);

	gyroSample_.x_value = sample.x_value;
	gyroSample_.y_value = sample.y_value;
	gyroSample_.z_value = sample.z_value;
	gyroSample_.timestamp = sample.timestamp;
}

void GyroSensor::Private::OnErrorOccurred(cros::SamplesObserver::ErrorType error)
{
	switch (error) {
	case cros::SamplesObserver::ErrorType::READ_FAILED:
		LOG(CrosGyroSensor, Error) << "SensorHalClient error: "
					   << convertErrorTypeToString(error);
		break;

	case cros::SamplesObserver::ErrorType::MOJO_DISCONNECTED:
	case cros::SamplesObserver::ErrorType::INVALID_ARGUMENT:
	case cros::SamplesObserver::ErrorType::DEVICE_REMOVED:
		LOG(CrosGyroSensor, Error) << "SensorHalClient error: "
					   << convertErrorTypeToString(error)
					   << ", aborting all usages";
		auto *sensorHalClient =
			cros::SensorHalClient::GetInstance(gCrosMojoToken);
		if (sensorHalClient)
			sensorHalClient->UnregisterSamplesObserver(this);

		break;
	}
}

PUBLIC_GYRO_SENSOR_IMPLEMENTATION

} /* namespace libcamera */

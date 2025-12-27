/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Google Inc.
 *
 * cros_gyro_sensor.cpp - Chromium OS gyroscope sensor using mojo
 */

#include "libcamera/internal/gyro_sensor.h"

#include <aidl/android/frameworks/sensorservice/ISensorManager.h>
#include <android/binder_manager.h>
#include <android/sensor.h>

#include "libcamera/base/log.h"
#include "libcamera/base/mutex.h"
#include "libcamera/base/thread_annotations.h"

namespace libcamera {

namespace {

} /* namespace */

LOG_DEFINE_CATEGORY(ALGyroSensor)

class GyroSensor::Private : public Extensible::Private
{
	LIBCAMERA_DECLARE_PUBLIC(GyroSensor)

public:
	Private();
	~Private();

	int init(Location location);

	bool startReading(double frequency);
	void stopReading();

	SensorSample getLatestSample();

private:
	ASensorManager *sensor_manager_ = nullptr;
	ASensorRef ref_;

	ALooper *looper_;
	ASensorEventQueue *queue_;

	libcamera::Mutex gyroSampleMutex_;
	SensorSample gyroSample_ LIBCAMERA_TSA_GUARDED_BY(gyroSampleMutex_);
};

GyroSensor::Private::Private()
{
}

GyroSensor::Private::~Private()
{
	if (sensor_manager_ != nullptr && queue_ != nullptr &&
	    looper_ != nullptr) {
		ASensorManager_destroyEventQueue(sensor_manager_, queue_);
	}
}

int GyroSensor::Private::init(Location)
{
	{
		using ISensorManager =
			aidl::android::frameworks::sensorservice::ISensorManager;
		static const std::string instance = std::string() + ISensorManager::descriptor + "/default";
		AIBinder *binder = AServiceManager_checkService(instance.c_str());
		if (binder == nullptr) {
			LOG(ALGyroSensor, Error) << "Failed to init: sensor service isn't ready.";
			return -ESRCH;
		}
	}

	sensor_manager_ = ASensorManager_getInstanceForPackage(nullptr);
	ref_ = ASensorManager_getDefaultSensor(sensor_manager_,
					       ASENSOR_TYPE_GYROSCOPE);
	if (ref_ == nullptr)
		return -ENXIO;

	looper_ = ALooper_forThread();
	if (looper_ == nullptr)
		looper_ = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);

	if (looper_ == nullptr) {
		LOG(ALGyroSensor, Error) << "Failed to prepare an event looper.";
		return -ESRCH;
	}

	queue_ = ASensorManager_createEventQueue(sensor_manager_, looper_, 0, nullptr,
						 nullptr);
	if (queue_ == nullptr) {
		LOG(ALGyroSensor, Error) << "Unable to get event queue.";
		return -ESRCH;
	}
	return 0;
}

bool GyroSensor::Private::startReading(double frequency)
{
	ASensorEventQueue_registerSensor(queue_, ref_, 1000000 / frequency, 0);
	return true;
}

void GyroSensor::Private::stopReading()
{
	ASensorEventQueue_disableSensor(queue_, ref_);
}

GyroSensor::SensorSample GyroSensor::Private::getLatestSample()
{
	MutexLocker lock(gyroSampleMutex_);

	ASensorEvent event;
	while (ASensorEventQueue_getEvents(queue_, &event, 1) > 0) {
		gyroSample_.x_value = event.gyro.x;
		gyroSample_.y_value = event.gyro.y;
		gyroSample_.z_value = event.gyro.z;
		gyroSample_.timestamp = event.timestamp;
	}

	return gyroSample_;
}

PUBLIC_GYRO_SENSOR_IMPLEMENTATION

} /* namespace libcamera */

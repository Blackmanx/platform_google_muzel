/*
 * Copyright (C) 2023, Google Inc.
 *
 * capture.h - MTK MtkISP7 Hal 3A Manager
 */

#pragma once

#include <cstdint>
#include <sys/types.h>

#include "libcamera/internal/dma_heaps.h"
#include "libcamera/internal/gyro_sensor.h"
#include "libcamera/internal/task_scheduler.h"

#include "pipeline/mtkisp7/camsys/camsys.h"
#include "pipeline/mtkisp7/camsys/capture.h"
#include "pipeline/mtkisp7/face_detect/detector.h"
#include "pipeline/mtkisp7/ipa/ipa_delegate.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"

namespace libcamera {

class AAATask;
class MtkISP7CameraData;

class FocusController
{
public:
	FocusController(CameraLens *cameraLens);

	VcmFocusInformation getFocusInfo();
	ipa::mtkisp7::LensPositionInfo getLensPositionInfo()
	{
		return this->lensPositionInfo_;
	}

	void set(int32_t position, int64_t timestamp);

	void setLensPositionInfo(ipa::mtkisp7::LensPositionInfo lensPositionInfo)
	{
		this->lensPositionInfo_ = lensPositionInfo;
	}

	uint64_t isLensMoving()
	{
		return isLensMoving_;
	}

private:
	void reset();
	bool isFirstRun();

	CameraLens *cameraLens_;
	bool firstRun_ = true;

	int32_t focusPosition_;
	int32_t previousFocusPosition_;
	int64_t movingTimestamp_;
	int64_t previousMovingTimestamp_;
	bool isLensMoving_;

	ipa::mtkisp7::LensPositionInfo lensPositionInfo_;
};

class Hal3AManager
{
public:
	void configure(DmaHeap *dmaHeap, CamSysDevice *camSys,
		       GyroSensor *gyroSensor,
		       IPADelegate *ipa);
	int start(int32_t lens_position);

	void releaseBuffers();

	AAATask *
	make3ATasks(Scheduler *scheduler, Request *request,
		    CaptureFrames &captureFrames,
		    uint32_t internalRequestId, uint32_t camSysMetaRequestId,
		    FaceDetector *faceDetector);

	void fetchTuningBuffer(SharedMailBox<InfoFrame> &mailBox)
	{
		tuningPool_.fetch(mailBox);
	}

	std::pair<uint32_t, SharedMailBox<InfoFrame>> getDummyTuning();

	void setMfnrMode(bool mfnrMode)
	{
		mfnrMode_ = mfnrMode;
	}
	bool getMfnrMode() const
	{
		return mfnrMode_;
	}

	bool isLensMoving();
	float getIntpolateDis();
	float getLensFocusDistance();

private:
	friend MtkISP7CameraData;

	bool hasAF() const;

	DmaHeap *dmaHeap_;
	CamSysDevice *camSys_;
	IPADelegate *ipa_;

	GyroSensor *gyroSensor_;

	std::unique_ptr<FocusController> focusController_;

	InfoFramePool tuningPool_;

	uint32_t dummyMetaRequestId_;
	SharedMailBox<InfoFrame> dummyTuning_;

	bool mfnrMode_;
};

// AE & AWB & AF task.
class AAATask : public Task
{
public:
	constexpr static uint32_t kLensDelay = 3;

	struct PerFrameControl {
		int delayIdx = 0;
		bool isStillCapture = false;
		ControlList controls = controls::controls;
	};

	AAATask(Hal3AManager *manager, Scheduler *scheduler, const std::string &id,
		SharedMailBox<InfoFrame> statistics0,
		SharedMailBox<InfoFrame> statistics1,
		SharedMailBox<InfoFrame> tuningOutput,
		SharedMailBox<uint64_t> timestamp,
		SharedMailBox<ipa::mtkisp7::SensorSetting> exposureAndGainOutput,
		SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange,
		GyroSensor *gyroSensor,
		IPADelegate *ipa,
		FocusController *focusController,
		uint32_t internalRequestId, uint32_t camSysMetaRequestId,
		FaceDetector *faceDetector)
		: Task(scheduler, id), manager_(manager),
		  statistics0_(statistics0), statistics1_(statistics1),
		  tuningOutput_(tuningOutput), timestamp_(timestamp),
		  exposureAndGainOutput_(exposureAndGainOutput),
		  aaaIspExchange_(aaaIspExchange),
		  gyroSensor_(gyroSensor),
		  ipa_(ipa), focusController_(focusController),
		  internalRequestId_(internalRequestId),
		  camSysMetaRequestId_(camSysMetaRequestId), faceDetector_(faceDetector)
	{
	}

	void setPerFrameControl(PerFrameControl perFrameControl);

	void AAAResultReady(ipa::mtkisp7::SensorSetting exposureAndGain,
			    const ipa::mtkisp7::AaaIspExchange &aaaIspExchange,
			    const ipa::mtkisp7::LensPositionInfo &lensPositionInfo);

	void run() override final;

	void setInternalRequestIdApplied(uint32_t internalRequestIdApplied);
	void setFeatureApplied(Feature feature);

	std::optional<uint32_t> internalRequestIdApplied_;
	std::optional<Feature> featureApplied_;

	Hal3AManager *manager_;

	SharedMailBox<InfoFrame> statistics0_;
	SharedMailBox<InfoFrame> statistics1_;
	SharedMailBox<InfoFrame> tuningOutput_;
	SharedMailBox<uint64_t> timestamp_;
	SharedMailBox<ipa::mtkisp7::SensorSetting> exposureAndGainOutput_;
	SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange_;

	GyroSensor *gyroSensor_;

	IPADelegate *ipa_;
	FocusController *focusController_;

	uint32_t internalRequestId_;
	uint32_t camSysMetaRequestId_;

	FaceDetector *faceDetector_;

	PerFrameControl perFrameControl_;
};

} // namespace libcamera

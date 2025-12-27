/*
 * Copyright (C) 2023, Google Inc.
 *
 * capture.h - MTK MtkISP7 Hal 3A Manager
 */

#include "aaa.h"

#include <libcamera/formats.h>

#include "libcamera/internal/framebuffer.h"
#include "libcamera/internal/mapped_framebuffer.h"

#include "libcamera/request.h"
#include "libfdft_lib/faces.h"
#include "peripheraldriver/lens/vcm_drv.h"
#include "pipeline/mtkisp7/hal3a/const.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"

namespace libcamera {

namespace {

static constexpr Size kMetaSize = Size{ kHal3ARawMetaSize, 1 };
static constexpr uint32_t kDummyMetaRequestId = 0xFFF12345;

// Todo: Move the funtion to common utils
uint64_t getMonotonicTimestamp()
{
	struct timespec t;
	t.tv_sec = t.tv_nsec = 0;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (uint64_t)((t.tv_sec) * 1000000000LL + t.tv_nsec);
}

} // namespace

LOG_DECLARE_CATEGORY(MtkISP7)

FocusController::FocusController(CameraLens *cameraLens)
{
	cameraLens_ = cameraLens;
	reset();
}

void FocusController::reset()
{
	firstRun_ = true;

	focusPosition_ = 0;
	previousFocusPosition_ = 0;
	movingTimestamp_ = 0;
	previousMovingTimestamp_ = 0;
	isLensMoving_ = false;
}

VcmFocusInformation FocusController::getFocusInfo()
{
	VcmFocusInformation info;
	info.focus_position = focusPosition_;
	info.previous_focus_position = previousFocusPosition_;
	info.moving_timestamp = movingTimestamp_;
	info.previous_moving_timestamp = previousMovingTimestamp_;
	LOG(MtkISP7, Debug) << "getFocusInfo. focus position: " << focusPosition_
			    << ", previous focus position: " << previousFocusPosition_
			    << ", moving timestamp: " << movingTimestamp_
			    << ", previous moving timestamp: " << previousMovingTimestamp_;

	return info;
}

void FocusController::set(int32_t position, int64_t timestamp)
{
	if (position < 0 || position == focusPosition_) {
		isLensMoving_ = false;
		return;
	}

	if (isFirstRun()) {
		cameraLens_->setFocusPosition(
			position == 0 ? 1 : position - 1);
	}

	ASSERT(cameraLens_);
	if (cameraLens_)
		cameraLens_->setFocusPosition(position);

	// Do not update moving timestamp if the target position is unchanged
	if (position == focusPosition_) {
		isLensMoving_ = false;
		return;
	}

	previousFocusPosition_ = focusPosition_;
	previousMovingTimestamp_ = movingTimestamp_;
	focusPosition_ = position;
	movingTimestamp_ = timestamp;
	isLensMoving_ = true;
}

bool FocusController::isFirstRun()
{
	bool firstRun = firstRun_;
	firstRun_ = false;
	return firstRun;
}

void Hal3AManager::configure(DmaHeap *dmaHeap, CamSysDevice *camSys,
			     GyroSensor *gyroSensor,
			     IPADelegate *ipa)
{
	dmaHeap_ = dmaHeap;
	camSys_ = camSys;
	gyroSensor_ = gyroSensor;
	ipa_ = ipa;

	if (hasAF())
		focusController_ = std::make_unique<FocusController>(camSys_->getCameraLens());

	if (tuningPool_.size() == 0)
		tuningPool_.createBuffers(dmaHeap_, formats::MTFP_MTISP, kMetaSize, 8,
					  DmaHeap::CMA);

	dummyMetaRequestId_ = kDummyMetaRequestId;

	dummyTuning_ = makeMailBox<InfoFrame>();
	fetchTuningBuffer(dummyTuning_);
}

int Hal3AManager::start(int32_t lens_position)
{
	if (hasAF())
		focusController_->set(lens_position, 0);

	return 0;
}

void Hal3AManager::releaseBuffers()
{
	dummyTuning_.reset();

	tuningPool_.release();
}

bool Hal3AManager::hasAF() const
{
	if (!camSys_)
		LOG(MtkISP7, Fatal) << "CamSysDevice hasn't been configured yet.";

	return camSys_->getCameraLens();
}

bool Hal3AManager::isLensMoving()
{
	if (!focusController_)
		return false;

	return focusController_->isLensMoving();
}

float Hal3AManager::getLensFocusDistance()
{
	if (!focusController_)
		return 0.0;

	return focusController_->getLensPositionInfo().focusDistance;
}

AAATask *Hal3AManager::make3ATasks(
	Scheduler *scheduler, Request *request,
	CaptureFrames &captureFrames, uint32_t internalRequestId,
	uint32_t camSysMetaRequestId,
	FaceDetector *faceDetector)
{
	std::string sequence = "padding";
	if (request)
		sequence = std::to_string(request->sequence());

	// Update the dummyTuning and the related camSysMetaRequestId
	dummyMetaRequestId_ = camSysMetaRequestId;
	dummyTuning_ = captureFrames.tuning;

	return new AAATask(this, scheduler, "3A " + sequence,
			   captureFrames.statistics0,
			   captureFrames.statistics1,
			   captureFrames.tuningOutput,
			   captureFrames.timestamp,
			   captureFrames.exposureAndGainOutput,
			   captureFrames.aaaIspExchange,
			   gyroSensor_,
			   ipa_, focusController_.get(),
			   internalRequestId, camSysMetaRequestId,
			   faceDetector);
}

std::pair<uint32_t, SharedMailBox<InfoFrame>> Hal3AManager::getDummyTuning()
{
	if (!dummyTuning_)
		LOG(MtkISP7, Fatal) << "Empty dummy tuning buffer";

	return std::make_pair(dummyMetaRequestId_, dummyTuning_);
}

void AAATask::run()
{
	manager_->fetchTuningBuffer(tuningOutput_);

	FrameBuffer *tuningBuffer = tuningOutput_->get().buffer();
	MappedFrameBuffer mappedBuffer(tuningBuffer,
				       MappedFrameBuffer::MapFlag::ReadWrite);
	tuningBuffer->_d()->metadata().planes()[0].bytesused =
		tuningBuffer->planes()[0].length;

	ipa::mtkisp7::GyroSampleData gyroSample;
	if (gyroSensor_) {
		GyroSensor::SensorSample sample = gyroSensor_->getLatestSample();
		if (sample.timestamp == 0) {
			LOG(MtkISP7, Error) << "Gyro not found";
		} else {
			gyroSample.x_value = sample.x_value;
			gyroSample.y_value = sample.y_value;
			gyroSample.z_value = sample.z_value;
			gyroSample.timestamp = sample.timestamp;
		}
	}

	ipa::mtkisp7::VcmFocusInformation vcm;

	if (focusController_) {
		vcm.focus_position = focusController_->getFocusInfo().focus_position;
		vcm.previous_focus_position = focusController_->getFocusInfo().previous_focus_position;
		vcm.moving_timestamp = focusController_->getFocusInfo().moving_timestamp;
		vcm.previous_moving_timestamp = focusController_->getFocusInfo().previous_moving_timestamp;
	}

	ipa_->doCalculation3A(
		this, internalRequestId_,
		statistics0_->get().buffer()->cookie(),
		focusController_ ? statistics1_->get().buffer()->cookie() : 0,
		timestamp_->get(), camSysMetaRequestId_,
		internalRequestId_ - kLensDelay,
		perFrameControl_.isStillCapture,
		tuningOutput_->get().buffer()->cookie(),
		gyroSample, internalRequestIdApplied_.value_or(0),
		featureApplied_, vcm,
		perFrameControl_.controls);
}

void AAATask::AAAResultReady(ipa::mtkisp7::SensorSetting exposureAndGain,
			     const ipa::mtkisp7::AaaIspExchange &aaaIspExchange,
			     const ipa::mtkisp7::LensPositionInfo &lensPositionInfo)
{
	uint64_t timestamp = getMonotonicTimestamp();
	if (focusController_) {
		focusController_->set(exposureAndGain.position, timestamp / 1000);
		focusController_->setLensPositionInfo(lensPositionInfo);
	}

	exposureAndGainOutput_->put(exposureAndGain, nullptr);

	manager_->setMfnrMode(aaaIspExchange.mfnrMode);
	aaaIspExchange_->put(aaaIspExchange, nullptr);

	notifyDone();
}

void AAATask::setPerFrameControl(PerFrameControl perFrameControl)
{
	float oldFocusDistance = perFrameControl_.controls.get(controls::draft::LensFocusDistance).value_or(0);
	perFrameControl_ = perFrameControl;
	if (perFrameControl.delayIdx >= static_cast<int>(CaptureTasksManager::kRawMetaDelay - 2)) {
		// Lens change event is fast, delay it by 2 frames to synchroize with lens state.
		perFrameControl_.controls.set(controls::draft::LensFocusDistance, oldFocusDistance);
	}
}

void AAATask::setInternalRequestIdApplied(uint32_t internalRequestIdApplied)
{
	internalRequestIdApplied_ = internalRequestIdApplied;
}

void AAATask::setFeatureApplied(Feature featureApplied)
{
	featureApplied_ = featureApplied;
}

} // namespace libcamera

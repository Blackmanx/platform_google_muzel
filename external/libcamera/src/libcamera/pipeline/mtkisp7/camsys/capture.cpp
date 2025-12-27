/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Google Inc.
 *
 * capture.cpp - MTK MtkISP7 Camsys Device Capture Tasks
 */

#include "capture.h"

#include <cstdint>

#include <libcamera/formats.h>
#include <libcamera/geometry.h>
#include <libcamera/request.h>

#include "libcamera/internal/framebuffer.h"
#include "libcamera/internal/pipeline_handler.h"
#include "libcamera/internal/request.h"

#include <libcamera/internal/info_frame.h>

#include "mt8188/mtk_cam_metabuf.h"
#include "pipeline/mtkisp7/hal3a/aaa.h"
#include "pipeline/mtkisp7/imgsys/mfnr.h"

#include "camsys.h"
#include "control_ids.h"
#include "mtk_cam_metabuf.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

namespace {

static constexpr Size kMeSize = Size{ 576, 432 };
static constexpr Size kFdSize = Size{ 640, 480 };
static constexpr Size kStatSize0 = Size{ 1081344, 1 };
static constexpr Size kStatSize1 = Size{ 528384, 1 };

// Todo: Move the funtion to common utils
uint64_t getMonotonicTimestamp()
{
	struct timespec t;
	t.tv_sec = t.tv_nsec = 0;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (uint64_t)((t.tv_sec) * 1000000000LL + t.tv_nsec);
}

} // namespace

CaptureTasksManager::CaptureTasksManager(OnDeviceTuner *odt)
	: onDeviceTuner_(odt)
{
}

int CaptureTasksManager::configure(DmaHeap *dmaHeap,
				   CamSysDevice *camSys,
				   PipelineHandler *pipe,
				   History<MfnrInput> *mfnrInput,
				   const Size &rawFrameSize,
				   const Size &yuvFrameSize,
				   int32_t pipelineDepth)
{
	dmaHeap_ = dmaHeap;
	camSys_ = camSys;
	pipe_ = pipe;

	mfnrInput_ = mfnrInput;

	rawFrameSize_ = rawFrameSize;
	yuvFrameSize_ = yuvFrameSize;
	pipelineDepth_ = pipelineDepth;

	releaseBuffers();
	allocateBuffers();

	return 0;
}

void CaptureTasksManager::allocateBuffers()
{
	rawPool_.setFormat(dmaHeap_, camSys_->bayerFormat(), rawFrameSize_,
			   onDeviceTuner_->isEnabled() ? pipelineDepth_ + MFNR_QUEUE_SIZE
						       : MFNR_QUEUE_SIZE + 4);
	yuvo1Pool_.createBuffers(dmaHeap_, formats::NV12_10P_MTISP, yuvFrameSize_, pipelineDepth_ + MFNR_QUEUE_SIZE);
	yuvo2Pool_.createBuffers(dmaHeap_, formats::NV12_12P_MTISP, yuvFrameSize_ / 2, pipelineDepth_);
	// Me needs one more buffer to be kept in MCNRPrevOutput.
	mePool_.createBuffers(dmaHeap_, formats::GREY, kMeSize, pipelineDepth_ + 1, DmaHeap::System, 64);
	faceDetectPool_.createBuffers(dmaHeap_, formats::NV12, kFdSize, 8);
	statistics0Pool_.createBuffers(dmaHeap_, formats::MTFA_MTISP, kStatSize0, kRawMetaDelay, DmaHeap::CMA);
	statistics1Pool_.createBuffers(dmaHeap_, formats::MTFF_MTISP, kStatSize1, kRawMetaDelay, DmaHeap::CMA);

	if (onDeviceTuner_->isCamsysDebugFrameEnabled())
		rawi2Pool_.createBuffers(dmaHeap_, camSys_->bayerFormat(), rawFrameSize_, 8);
}

void CaptureTasksManager::releaseBuffers()
{
	rawPool_.release();

	yuvo1Pool_.release();
	yuvo2Pool_.release();
	yuvo3Pool_.release();
	yuvo4Pool_.release();

	mePool_.release();
	faceDetectPool_.release();

	statistics0Pool_.release();
	statistics1Pool_.release();

	rawi2Pool_.release();
}

void CaptureTasksManager::releaseElasticBuffers()
{
	rawPool_.releaseElastic();
}

void CaptureTasksManager::makeCaptureFrames(CaptureFrames &captureFrames,
					    bool needRaw, bool needYuvo1,
					    bool hasVideo)
{
	captureFrames.tuningOutput = makeMailBox<InfoFrame>();

	if (needRaw)
		captureFrames.raw = makeMailBox<InfoFrame>();

	if (needYuvo1) {
		captureFrames.yuvo1 = makeMailBox<InfoFrame>();
	}
	if (hasVideo) {
		captureFrames.yuvo2 = makeMailBox<InfoFrame>();
		captureFrames.me = makeMailBox<InfoFrame>();
	}
	captureFrames.faceDetection = makeMailBox<InfoFrame>();

	captureFrames.statistics0 = makeMailBox<InfoFrame>();
	captureFrames.statistics1 = makeMailBox<InfoFrame>();

	captureFrames.timestamp = makeMailBox<uint64_t>();
	captureFrames.exposureAndGainOutput =
		makeMailBox<ipa::mtkisp7::SensorSetting>();

	captureFrames.aaaIspExchange = makeMailBox<ipa::mtkisp7::AaaIspExchange>();

	captureFrames.rawInject = makeMailBox<InfoFrame>();
}

std::tuple<QueueTask *, DequeueTask *, SofTask *>
CaptureTasksManager::makeCaptureTasks(Scheduler *scheduler,
				      const std::string &id,
				      Request *request,
				      CaptureFrames &captureFrames,
				      uint32_t internalRequestId,
				      Hal3AManager *hal3AManager,
				      SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange)
{
	(void)id;

	std::string sequence = "padding";
	if (request)
		sequence = std::to_string(request->sequence());

	// Create CaptureData after CaptureFrames SharedMailBoxes are set.
	auto data = std::make_shared<CaptureData>(captureFrames);

	SofTask *sofTask = new SofTask(
		scheduler, "Sof " + sequence, request,
		internalRequestId, data, camSys_, this, hal3AManager);

	QueueTask *qTask = new QueueTask(
		this, scheduler, "Queue " + sequence, request,
		internalRequestId, data, aaaIspExchange);
	DequeueTask *dqTask = new DequeueTask(
		this, scheduler, "Dequeue " + sequence, request,
		internalRequestId, data);

	return std::make_tuple(qTask, dqTask, sofTask);
}

void SofTask::setSensorSetting()
{
	auto sensorSetting = data_->frames.exposureAndGain->get();
	if (sensorSetting.exposure != 0) { // Assuming it couldn't be zero.
		camSys_->setVBlank(sensorSetting.vblank);
		camSys_->setExposureGain(sensorSetting.exposure, sensorSetting.gain);
	}
	LOG(MtkISP7, Debug) << "exposure: " << sensorSetting.exposure
			    << ", gain: " << sensorSetting.gain;
}

void SofTask::run()
{
	run_ = true;

	if (trigger_) {
		LOG(MtkISP7, Warning) << "Sof Task triggered before run()."
				      << " Some previous tasks may delay the Sof task";
		setSensorSetting();
		notifyDone();
	}
}

void SofTask::trigger()
{
	uint64_t timestamp = getMonotonicTimestamp();

	if (run_)
		setSensorSetting();

	data_->frames.timestamp->put(timestamp,
				     []([[maybe_unused]] uint64_t &timestamp) {});

	if (request_) {
		ControlList metadata;
		auto lensFocusDistance = hal3AManager_->getLensFocusDistance();
		metadata.set(controls::SensorTimestamp, timestamp);
		if (hal3AManager_->isLensMoving()) {
			metadata.set(controls::draft::LensState, 1);
			LOG(MtkISP7, Debug) << "Lens is moving: " << lensFocusDistance;
		} else {
			metadata.set(controls::draft::LensState, 0);
			LOG(MtkISP7, Debug) << "Lens is staionary: " << lensFocusDistance;
		}
		metadata.set(controls::draft::LensFocusDistance, lensFocusDistance);
		manager_->pipe_->metadataAvailable(request_, metadata);
	}

	trigger_ = true;
	if (run_ && trigger_) {
		notifyDone();
	}
}

void QueueTask::run()
{
	if (request_ && aaaIspExchange_) {
		ControlList metadata;
		if (aaaIspExchange_->valid()) {
			auto aaaIspExchange = aaaIspExchange_->get();
			metadata.merge(aaaIspExchange.aaaMetadata);
		} else {
			// Set to 15 fps by default.
			metadata.set(controls::ExposureTime, (int64_t)66'666);
		}

		if (!metadata.contains(controls::AF_STATE)) {
			metadata.set(controls::AfState, 0);
		}

		manager_->pipe_->metadataAvailable(request_, metadata);
	}

	CamSysDevice *camSys = manager_->camSys_;
	auto &frames = data_->frames;
	auto &camSysRequest = data_->request;

	if (manager_->onDeviceTuner_->isCamsysDebugFrameEnabled()) {
		manager_->rawi2Pool_.fetch(frames.rawInject);
		auto &rawInject = camSysRequest.rawInject;
		rawInject = frames.rawInject->get().buffer();
		MappedFrameBuffer mapped(
			rawInject, MappedFrameBuffer::MapFlag::Read);
		rawInject->_d()->metadata().planes()[0].bytesused =
			mapped.planes()[0].size();
	}

	if (request_) {
		const auto testPatternControl =
			request_->controls().get(controls::draft::TestPatternMode);
		if (testPatternControl) {
			camSys->setTestPattern(
				static_cast<controls::draft::TestPatternModeEnum>(*testPatternControl));
		}
		manager_->onDeviceTuner_->fillCamsysDebugFrame(
			internalRequestId_, frames.rawInject);
	}

	manager_->statistics0Pool_.fetch(frames.statistics0);
	camSysRequest.statistics0 = frames.statistics0->get().buffer();

	manager_->statistics1Pool_.fetch(frames.statistics1);
	camSysRequest.statistics1 = frames.statistics1->get().buffer();

	camSysRequest.tuning = frames.tuning->get().buffer();

	if (frames.me) {
		manager_->mePool_.fetch(frames.me);
		camSysRequest.me = frames.me->get().buffer();
	}

	manager_->faceDetectPool_.fetch(frames.faceDetection);
	camSysRequest.faceDetect = frames.faceDetection->get().buffer();

	if (frames.raw) {
		manager_->rawPool_.fetch(frames.raw);
		camSysRequest.main = frames.raw->get().buffer();
	}

	if (frames.yuvo1) {
		manager_->yuvo1Pool_.fetch(frames.yuvo1);
		camSysRequest.yuvo1 = frames.yuvo1->get().buffer();
	}

	if (frames.yuvo2) {
		manager_->yuvo2Pool_.fetch(frames.yuvo2);
		camSysRequest.yuvo2 = frames.yuvo2->get().buffer();
	}

	camSys->queueRequest(&camSysRequest);

	notifyDone();
}

void DequeueTask::run()
{
	CamSysDevice *camSys = manager_->camSys_;

	/* If request is not finished, register call back when it's done. */
	if (camSys->claimCompletedRequest(&data_->request)) {
		camSys->requestCompleted.connect(this, &DequeueTask::requestReady);
		return;
	}

	done();
}

void DequeueTask::requestReady(CamSysDevice::Request *request)
{
	if (request != &data_->request)
		return;

	CamSysDevice *camSys = manager_->camSys_;

	camSys->requestCompleted.disconnect(this, &DequeueTask::requestReady);
	camSys->claimCompletedRequest(&data_->request);

	done();
}

void DequeueTask::done()
{
	if (data_->frames.raw) {
		MfnrInput mfnrInput;
		mfnrInput.raw = data_->frames.raw;
		mfnrInput.yuvo1 = data_->frames.yuvo1;
		manager_->mfnrInput_->add(internalRequestId_, mfnrInput);
	}

	if (request_)
		manager_->onDeviceTuner_->tuneCamsys(internalRequestId_, data_->frames);

	notifyDone();
}

} /* namespace libcamera */

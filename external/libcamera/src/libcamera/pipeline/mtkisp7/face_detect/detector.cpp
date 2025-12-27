/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * detector.cpp - Run face detection.
 */

#include "detector.h"

#include <memory>
#include <utility>

#include <libcamera/control_ids.h>
#include <libcamera/formats.h>
#include <libcamera/geometry.h>

#include "libcamera/internal/info_frame.h"
#include "libcamera/internal/mailbox.h"

#include "../ipa/ipa_delegate.h"
#include "libcamera/request.h"
#include "mtkcam-core/hw/aie/3.1/hardware/v4l2/cam_fdvt_v4l2.h"

/**
 * \file pipeline/mtkisp7/face_detect/detector.h
 * \brief Manages face detection for MTKISP7
 */

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

FaceDetectTask::FaceDetectTask(Scheduler *scheduler, const std::string &id,
			       uint32_t camSysMetaRequestId, FaceDetector *detector,
			       SharedMailBox<InfoFrame> mailBoxInputImage)
	: Task(scheduler, id), camSysMetaRequestId_(camSysMetaRequestId),
	  mailBoxInputImage_(std::move(mailBoxInputImage)), detector_(detector)
{
}

void FaceDetectTask::run()
{
	if (!detector_->shouldRun(camSysMetaRequestId_)) {
		notifyDone();
		return;
	}

	FaceDetector::FaceDetectRequest request;
	request.detectorInput = mailBoxInputImage_;
	request.internalRequestId = camSysMetaRequestId_;
	request.camSysMetaRequestId = camSysMetaRequestId_;

	detector_->queueRequest(request);

	notifyDone();
}

/**
 * \var FaceDetector::period_
 * \brief How many frames between face detection run.
 *
 * If \a period is 15 then the face detection should run
 * on frame 0, 15, 30, and so on.
 */

/**
 * \class FaceDetector
 * \brief Delegates face detector hardware/algorithm lifecycle and configuration
 *
 * This \a FaceDetector is using MediaTek AIE device and using MediaTek
 * face detection algorithm library to parse the result of the device.
 */
FaceDetector::FaceDetector()
	: period_(3)
{
}

FaceDetector::~FaceDetector()
{
	threadFaceDetect_.exit();
	threadFaceDetect_.wait();
}

void FaceDetector::sourceVideoReady(std::pair<FrameBuffer *, int> bufferWithRequest)
{
	(void)bufferWithRequest;

	ASSERT(runningRequest_.has_value());

	if (--runningRequest_->pending == 0)
		notifyHardwareDone();
}

void FaceDetector::resultMetaReady(FrameBuffer *bufferWithRequest)
{
	(void)bufferWithRequest;

	ASSERT(runningRequest_.has_value());

	if (--runningRequest_->pending == 0)
		notifyHardwareDone();
}

void FaceDetector::notifyHardwareDone()
{
	aieDev_->media_->reInitRequest(runningRequest_->faceDetectRequestFd);
	requestFDPool_.put(runningRequest_->faceDetectRequestFd);

	if (runningRequest_->toneClassifyRequestFd >= 0) {
		aieDev_->media_->reInitRequest(runningRequest_->toneClassifyRequestFd);
		requestFDPool_.put(runningRequest_->toneClassifyRequestFd);
	}

	triggerParse();
}

void FaceDetector::AieParseResultReady(bool success,
				       const ipa::mtkisp7::PrimaryFaceData &faceToneRoi,
				       const ControlList &out)
{
	if (!success)
		return;

	latestFaceToneROI.emplace(faceToneRoi);
	runningRequest_.reset();

	{
		MutexLocker locker(isProcessingMutex_);
		isProcessing_ = false;
		isProcessingCv_.notify_one();
	}

	setLatestFaceControls(out);

	this->invokeMethod(&FaceDetector::triggerNextRequest,
			   ConnectionTypeQueued);
}

void FaceDetector::triggerParse()
{
	ipa_->aieParse(
		runningRequest_->detectorInput->get().buffer()->cookie(),
		runningRequest_->faceResultMeta->get().buffer()->cookie(),
		(runningRequest_->toneResultMeta->valid()) ? runningRequest_->toneResultMeta->get().buffer()->cookie() : 0,
		currentSensorSize_, runningRequest_->camSysMetaRequestId);
}

void FaceDetector::queueHardwareRequest(FrameBuffer *input, FrameBuffer *result,
					int requestFd, FdDrv_input_struct &config)
{
	struct v4l2_ext_control extControl{
		.id = aieDev_->inferenceParamControlId_,
		.size = sizeof(FdDrv_input_struct),
		.reserved2 = {},
		.p_u32 = reinterpret_cast<__u32 *>(&config)
	};

	int ret = aieDev_->sourceVideo_->setExtControl(&extControl, requestFd);
	if (ret != 0) {
		LOG(MtkISP7, Fatal) << "Failed to set ext controls: " << ret;
		return;
	}

	ret = aieDev_->sourceVideo_->queueBuffer(input, requestFd);

	if (ret) {
		LOG(MtkISP7, Fatal) << "Failed to queue image buf: " << ret;
		return;
	}

	ret = aieDev_->resultMeta_->queueBuffer(result);
	if (ret) {
		LOG(MtkISP7, Fatal) << "Failed to queue metadata buf: " << ret;
		return;
	}

	ret = aieDev_->media_->queueRequest(requestFd);
	if (ret) {
		LOG(MtkISP7, Fatal) << "Failed to queue request: " << ret;
		return;
	}
}

void FaceDetector::triggerNextRequest()
{
	if (pendingRequests_.empty() || runningRequest_.has_value()) {
		return;
	}

	{
		MutexLocker locker(isProcessingMutex_);
		isProcessing_ = true;
	}

	runningRequest_ = pendingRequests_.front();
	pendingRequests_.pop_front();

	runningRequest_->faceDetectRequestFd = requestFDPool_.get();

	runningRequest_->faceResultMeta = makeMailBox<InfoFrame>();
	resultMetadataPool_.fetch(runningRequest_->faceResultMeta);

	runningRequest_->toneResultMeta = makeMailBox<InfoFrame>();

	runningRequest_->pending = 2;

	FdDrv_input_struct config = aieDev_->createFaceDetectionDriverConfig();
	queueHardwareRequest(runningRequest_->detectorInput->get().buffer(),
			     runningRequest_->faceResultMeta->get().buffer(),
			     runningRequest_->faceDetectRequestFd, config);

	if (!latestFaceToneROI.has_value())
		return;

	if (latestFaceToneROI->x1 == 0 &&
	    latestFaceToneROI->y1 == 0 &&
	    latestFaceToneROI->x2 == 0 &&
	    latestFaceToneROI->y2 == 0)
		return;

	runningRequest_->pending += 2;

	config = aieDev_->createFaceToneClassificationDriverConfig();

	config.src_roi.x1 = latestFaceToneROI->x1;
	config.src_roi.y1 = latestFaceToneROI->y1;
	config.src_roi.x2 = latestFaceToneROI->x2;
	config.src_roi.y2 = latestFaceToneROI->y2;

	config.src_padding.left = latestFaceToneROI->padding_left;
	config.src_padding.up = latestFaceToneROI->padding_up;
	config.src_padding.right = latestFaceToneROI->padding_right;
	config.src_padding.down = latestFaceToneROI->padding_down;

	resultMetadataPool_.fetch(runningRequest_->toneResultMeta);
	runningRequest_->toneClassifyRequestFd = requestFDPool_.get();

	queueHardwareRequest(runningRequest_->detectorInput->get().buffer(),
			     runningRequest_->toneResultMeta->get().buffer(),
			     runningRequest_->toneClassifyRequestFd, config);
}

void FaceDetector::queueRequest(FaceDetectRequest &request)
{
	if (Thread::current() != thread())
		return this->invokeMethod(&FaceDetector::queueRequest,
					  ConnectionTypeQueued,
					  request);

	// Skip the request if too many request pending.
	if (pendingRequests_.size() > 1)
		pendingRequests_.pop_front();

	pendingRequests_.emplace_back(request);
	triggerNextRequest();
}

bool FaceDetector::shouldRun(uint32_t internalRequestId)
{
	return internalRequestId % period_ == 0;
}

// Called from other thread
int FaceDetector::configure(Size currentSensorSize, IPADelegate *ipa)
{
	if (Thread::current() != thread())
		return this->invokeMethod(&FaceDetector::configure,
					  ConnectionTypeBlocking,
					  currentSensorSize, ipa);

	ipa_ = ipa;
	currentSensorSize_ = currentSensorSize;
	faceToneConfig_ = makeMailBox<FdDrv_input_struct>();

	latestFaceToneROI.reset();

	aieDev_->configure();

	aieDev_->sourceVideo_->requestBufferReady.disconnect();
	aieDev_->resultMeta_->bufferReady.disconnect();

	aieDev_->sourceVideo_->requestBufferReady.connect(this, &FaceDetector::sourceVideoReady);
	aieDev_->resultMeta_->bufferReady.connect(this, &FaceDetector::resultMetaReady);

	V4L2DeviceFormat metaFormat;
	aieDev_->resultMeta_->getFormat(&metaFormat);

	resultMetadataPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP,
					  { metaFormat.planes[0].size, 1 }, 8);

	std::vector<UniqueFD> requests;
	aieDev_->media_->allocateRequests(8, requests);

	requestFDPool_.setData(requests);

	return 0;
}

void FaceDetector::releaseBuffers()
{
	if (Thread::current() != thread())
		return this->invokeMethod(&FaceDetector::releaseBuffers,
					  ConnectionTypeBlocking);

	requestFDPool_.release();
	resultMetadataPool_.release();
}

void FaceDetector::getLatestFaceControls(ControlList &latest)
{
	MutexLocker locker(faceControlMutex_);
	latest = latestFaceControls_;
}

void FaceDetector::setLatestFaceControls(const ControlList &latest)
{
	MutexLocker locker(faceControlMutex_);
	latestFaceControls_ = latest;
}

Task *FaceDetector::makeFaceDetectionTask(
	Scheduler *scheduler, Request *request,
	SharedMailBox<InfoFrame> detectorInput, uint32_t camSysMetaRequestId)
{
	const std::string id = "FaceDetectionTask #" +
			       std::to_string(request->sequence());

	FaceDetectTask *fdTask = new FaceDetectTask(
		scheduler, id, camSysMetaRequestId, this, detectorInput);

	return fdTask;
}

void FaceDetector::cancelPendingRequests()
{
	pendingRequests_.clear();
}

// Called from main thread
int FaceDetector::init(MediaDevice *media, DmaHeap *dmaHeap)
{
	dmaHeap_ = dmaHeap;

	moveToThread(&threadFaceDetect_);
	threadFaceDetect_.start();

	return this->invokeMethod(&FaceDetector::initOnThread,
				  ConnectionTypeBlocking, media);
}

int FaceDetector::initOnThread(MediaDevice *media)
{
	aieDev_ = std::make_unique<AieDevice>();

	int ret = aieDev_->init(media);
	if (ret)
		return -ENODEV;

	return 0;
}

// Called from main thread
int FaceDetector::start()
{
	return aieDev_->invokeMethod(&AieDevice::start, ConnectionTypeBlocking);
}

// Called from main thread
int FaceDetector::stop()
{
	// Cancel pending requests
	this->invokeMethod(&FaceDetector::cancelPendingRequests, ConnectionTypeBlocking);

	// Wait for current running request finish
	{
		MutexLocker locker(isProcessingMutex_);
		isProcessingCv_.wait(
			locker, [&]() LIBCAMERA_TSA_REQUIRES(isProcessingMutex_) { return !isProcessing_; });
	}

	return aieDev_->invokeMethod(&AieDevice::stop, ConnectionTypeBlocking);
}

} /* namespace libcamera */

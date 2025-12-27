/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * detector.h - Run face detection.
 */
#pragma once

#include <memory>
#include <tuple>

#include <libcamera/base/object.h>
#include <libcamera/base/thread.h>

#include <libcamera/controls.h>

#include "libcamera/internal/info_frame.h"
#include "libcamera/internal/mailbox.h"
#include "libcamera/internal/task_scheduler.h"

#include "libcamera/base/mutex.h"
#include "libfdft_lib/faces.h"
#include "pipeline/mtkisp7/face_detect/aie.h"

#include "mtkisp7_ipa_interface.h"

namespace libcamera {

class IPADelegate;

class FaceDetector : public Object
{
public:
	struct FaceDetectRequest {
		// Filled by User
		uint32_t internalRequestId;
		uint32_t camSysMetaRequestId;
		SharedMailBox<InfoFrame> detectorInput;

		// Filled by FaceDetector
		int faceDetectRequestFd = -1;
		int toneClassifyRequestFd = -1;
		SharedMailBox<InfoFrame> faceResultMeta;
		SharedMailBox<InfoFrame> toneResultMeta;
		int pending = 0;
	};

	FaceDetector();
	virtual ~FaceDetector();

	int init(MediaDevice *media, DmaHeap *dmaHeap);
	int configure(Size currentSensorSize, IPADelegate *ipa);

	void releaseBuffers();

	int start();
	int stop();

	void queueRequest(FaceDetectRequest &request);

	Task *makeFaceDetectionTask(
		Scheduler *scheduler, Request *request,
		SharedMailBox<InfoFrame> detectorInput,
		uint32_t camSysMetaRequestId);

	bool shouldRun(uint32_t internalRequestId);

	void getLatestFaceControls(ControlList &latest);

	void AieParseResultReady(bool success,
				 const ipa::mtkisp7::PrimaryFaceData &primaryFace,
				 const ControlList &faceControls);

	InfoFramePool resultMetadataPool_;

private:
	int initOnThread(MediaDevice *media);

	void cancelPendingRequests();
	void queueHardwareRequest(FrameBuffer *input, FrameBuffer *result,
				  int requestFd, FdDrv_input_struct &config);

	void sourceVideoReady(std::pair<FrameBuffer *, int> bufferWithRequest);
	void resultMetaReady(FrameBuffer *bufferWithRequest);

	void notifyHardwareDone();
	void triggerParse();
	void triggerNextRequest();

	void setLatestFaceControls(const ControlList &latest);

private:
	DmaHeap *dmaHeap_;

	IPADelegate *ipa_;

	std::unique_ptr<AieDevice> aieDev_;
	const uint32_t period_;

	SharedMailBox<FdDrv_input_struct> faceToneConfig_;

	Size currentSensorSize_;

	Pool<int, UniqueFD> requestFDPool_;

	/* Protects access to the isProcessing_ flag. */
	libcamera::Mutex isProcessingMutex_;
	libcamera::ConditionVariable isProcessingCv_;

	/* Indicate if a request is processing */
	bool isProcessing_ LIBCAMERA_TSA_GUARDED_BY(isProcessingMutex_) = false;

	/* Protects access to the latestFaceControls_. */
	libcamera::Mutex faceControlMutex_;

	std::optional<FaceDetectRequest> runningRequest_;
	std::deque<FaceDetectRequest> pendingRequests_;
	std::optional<ipa::mtkisp7::PrimaryFaceData> latestFaceToneROI;
	ControlList latestFaceControls_;

	Thread threadFaceDetect_;
};

class FaceDetectTask : public Task
{
public:
	FaceDetectTask(Scheduler *scheduler, const std::string &id, uint32_t camSysMetaRequestId,
		       FaceDetector *detector, SharedMailBox<InfoFrame> mailBoxInputImage);

	void run() override;

private:
	[[maybe_unused]] uint32_t camSysMetaRequestId_;
	SharedMailBox<InfoFrame> mailBoxInputImage_;

	[[maybe_unused]] FaceDetector *detector_;
};

} /* namespace libcamera */

/*
 * Copyright (C) 2022, Google Inc.
 *
 * capture.h - MTK MtkISP7 Camsys Device Capture Tasks
 */

#pragma once

#include <cstdint>
#include <memory>

#include <libcamera/base/signal.h>

#include <libcamera/geometry.h>

#include "libcamera/internal/task_scheduler.h"

#include "pipeline/mtkisp7/imgsys/const.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"
#include "utils/history.h"

#include "camsys.h"
#include "mtkisp7_ipa_interface.h"

namespace libcamera {

class CamSysDevice;
class DequeueTask;
class DmaHeap;
class PipelineHandler;
class QueueTask;
class SofTask;
class MtkISP7CameraData;
class Hal3AManager;

struct CaptureFrames {
	SharedMailBox<InfoFrame> raw;
	SharedMailBox<InfoFrame> yuvo1;
	SharedMailBox<InfoFrame> yuvo2;
	SharedMailBox<InfoFrame> me;
	SharedMailBox<InfoFrame> faceDetection;
	SharedMailBox<InfoFrame> statistics0;
	SharedMailBox<InfoFrame> statistics1;
	SharedMailBox<InfoFrame> tuning; // Input
	SharedMailBox<InfoFrame> tuningOutput; // Output

	SharedMailBox<uint64_t> timestamp;

	SharedMailBox<ipa::mtkisp7::SensorSetting> exposureAndGain; // input
	SharedMailBox<ipa::mtkisp7::SensorSetting> exposureAndGainOutput; // output

	SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange;

	SharedMailBox<InfoFrame> rawInject; // Debug frame / ODT
};

class CaptureData
{
public:
	CaptureData(CaptureFrames &captureFrames)
		: frames(captureFrames) {}

	CamSysDevice::Request request;
	CaptureFrames frames;
};

class CaptureTasksManager
{
public:
	// TODO: Currently fix the sensor exposure/gain delay as 2.
	static const uint32_t kExposureAndGainDelay = 2;
	// TODO: Currently fix (k-4)th 3A task to prepare for kth request's raw meta.
	static const uint32_t kRawMetaDelay = 4;
	// TODO: Assume the delay between 3A task and Sof task is kRawMetaDelay - kExposureAndGainDelay.
	static const uint32_t kAAToSofDelay = kRawMetaDelay - kExposureAndGainDelay;

	CaptureTasksManager(OnDeviceTuner *odt);
	CaptureTasksManager() = default;
	~CaptureTasksManager() = default;

	int configure(DmaHeap *dmaHeap, CamSysDevice *camSys, PipelineHandler *pipe,
		      History<MfnrInput> *mfnrInput,
		      const Size &rawFrameSize, const Size &yuvFrameSize,
		      int32_t pipelineDepth);

	void allocateBuffers();
	void releaseBuffers();
	void releaseElasticBuffers();

	void makeCaptureFrames(CaptureFrames &captureFrames, bool needRaw,
			       bool needYuvo1, bool hasVideo);

	std::tuple<QueueTask *, DequeueTask *, SofTask *>
	makeCaptureTasks(Scheduler *scheduler, const std::string &id,
			 Request *request, CaptureFrames &captureFrames,
			 uint32_t internalRequestId, Hal3AManager *hal3AManager,
			 SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange);

private:
	friend QueueTask;
	friend DequeueTask;
	friend SofTask;
	friend MtkISP7CameraData;

	Size rawFrameSize_;
	Size yuvFrameSize_;
	int32_t pipelineDepth_;

	CamSysDevice *camSys_;
	PipelineHandler *pipe_;
	DmaHeap *dmaHeap_;
	OnDeviceTuner *onDeviceTuner_;

	History<MfnrInput> *mfnrInput_;

	ElasticInfoFramePool rawPool_;
	InfoFramePool yuvo1Pool_;
	InfoFramePool yuvo2Pool_;
	InfoFramePool yuvo3Pool_;
	InfoFramePool yuvo4Pool_;
	InfoFramePool mePool_;
	InfoFramePool faceDetectPool_;
	InfoFramePool statistics0Pool_;
	InfoFramePool statistics1Pool_;
	InfoFramePool rawi2Pool_;
};

class SofTask : public Task
{
public:
	SofTask(Scheduler *scheduler, const std::string &id,
		Request *request, uint32_t internalRequestId,
		std::shared_ptr<CaptureData> &data, CamSysDevice *camSys,
		CaptureTasksManager *manager, Hal3AManager *hal3AManager)
		: Task(scheduler, id), request_(request), internalRequestId_(internalRequestId),
		  data_(data), camSys_(camSys), manager_(manager), hal3AManager_(hal3AManager) {}

	virtual void run() override final;
	void trigger();
	void setSensorSetting();

	Request *request_;
	uint32_t internalRequestId_;

	std::shared_ptr<CaptureData> data_;

	CamSysDevice *camSys_;
	CaptureTasksManager *manager_;

	bool run_ = false;
	bool trigger_ = false;

	Hal3AManager *hal3AManager_;
};

class QueueTask : public Task
{
public:
	QueueTask(CaptureTasksManager *manager,
		  Scheduler *scheduler, const std::string &id,
		  Request *request, uint32_t internalRequestId,
		  std::shared_ptr<CaptureData> &data,
		  SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange)
		: Task(scheduler, id), request_(request), internalRequestId_(internalRequestId),
		  manager_(manager), data_(data), aaaIspExchange_(aaaIspExchange) {}

	void run() override final;

	Request *request_;
	uint32_t internalRequestId_;

	CaptureTasksManager *manager_;
	std::shared_ptr<CaptureData> data_;

	SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange_;
};

class DequeueTask : public Task
{
public:
	DequeueTask(CaptureTasksManager *manager,
		    Scheduler *scheduler, const std::string &id,
		    Request *request, uint32_t internalRequestId,
		    std::shared_ptr<CaptureData> &data)
		: Task(scheduler, id), request_(request), internalRequestId_(internalRequestId),
		  manager_(manager), data_(data) {}

	void run() override final;
	void done();
	void requestReady(CamSysDevice::Request *request);

	Request *request_;
	uint32_t internalRequestId_;

	CaptureTasksManager *manager_;

	std::shared_ptr<CaptureData> data_;
};

} /* namespace libcamera */

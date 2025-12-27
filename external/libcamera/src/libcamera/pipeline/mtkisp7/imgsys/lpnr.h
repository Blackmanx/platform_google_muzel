/*
 * Copyright (C) 2023, Google Inc.
 *
 * lpnr.h - MtkISP7 ImgSys Low Pass Noise Reduction Tasks
 */

#pragma once

#include <memory>
#include <string>

#include "libcamera/internal/info_frame.h"
#include "libcamera/internal/task_scheduler.h"

#include "pipeline/mtkisp7/odt/on_device_tuner.h"

#include "imgsys.h"

namespace libcamera {

class XTRTask;
class LpnrDipTask;
class MtkISP7CameraData;

struct XtrFrames {
	struct {
		SharedMailBox<InfoFrame> p1Raw;
		SharedMailBox<InfoFrame> xtrTun;
	} in;
	struct {
		SharedMailBox<InfoFrame> xtrStt;
		std::vector<SharedMailBox<InfoFrame>> dipImgi;
	} out;
};

struct LpnrDipFrames {
	struct {
		SharedMailBox<bool> highIsoMode;
		SharedMailBox<InfoFrame> dipTunPq;
		SharedMailBox<InfoFrame> dipTunY2YPq;
		std::vector<SharedMailBox<InfoFrame>> dipTun;
		std::vector<SharedMailBox<InfoFrame>> dipImgi;
	} in;
};

struct LPNRFrames {
	XtrFrames xtrFrames;
	LpnrDipFrames lpnrDipFrames;

	FrameBuffer *still1Output;
	FrameBuffer *still2Output;
};

class LpnrTasksManager
{
public:
	LpnrTasksManager(
		ImgSysDevice *imgSys, DmaHeap *dmaHeap, OnDeviceTuner *odt);

	int configure(const Size &bayerInputSize,
		      const Size &yuvOutputSize1, const Size &yuvOutputSize2);

	int start();
	int stop();

	int releaseBuffers();
	void releaseElasticBuffers();

	void makeLPNRFrames(LPNRFrames &lpnr,
			    SharedMailBox<InfoFrame> &p1Raw,
			    FrameBuffer *output1Frame,
			    FrameBuffer *output2Frame);

	std::tuple<XTRTask *, LpnrDipTask *>
	makeLpnrTasks(LPNRFrames &lpnr, Scheduler *scheduler, const std::string &id,
		      Request *request, uint32_t internalRequestId, ImgSysDevice *imgSys);

private:
	friend class XTRTask;
	friend class LpnrDipTask;
	friend MtkISP7CameraData;

	Size yuvOutputSize1_;
	Size yuvOutputSize2_;
	Size bayerInputSize_;

	std::vector<Size> lpnrSizes;

	InfoFramePool lpnrStt_;
	std::array<ElasticInfoFramePool, 4> lpnr_;

	ImgSysDevice *imgSys_;
	DmaHeap *dmaHeap_;
	OnDeviceTuner *onDeviceTuner_;
};

class XTRTask : public Task
{
public:
	XTRTask(Scheduler *scheduler, const std::string &id,
		Request *request, uint32_t internalRequestId,
		ImgSysDevice *imgSys, LPNRFrames &lpnr, LpnrTasksManager *manager);

	void run() override;
	void notifyDone() override;

private:
	void allocateOutputBuffers();

	XtrFrames frames_;
	ImgSysRequestHelper requestHelper_;

	Request *request_;
	[[maybe_unused]] uint32_t internalRequestId_;

	LpnrTasksManager *manager_;
};

class LpnrDipTask : public Task
{
public:
	LpnrDipTask(Scheduler *scheduler, const std::string &id,
		    Request *request, uint32_t internalRequestId,
		    ImgSysDevice *imgSys, LPNRFrames &lpnr, LpnrTasksManager *manager);

	void run() override;
	void notifyDone() override;

private:
	void allocateOutputBuffers();
	void LowIsoStages(SingleDeviceRequest &sdRequest);
	void HighIsoStage(SingleDeviceRequest &sdRequest);

	/* Intermediate Frames */
	std::vector<SharedMailBox<InfoFrame>> dipImg3o;
	std::vector<SharedMailBox<InfoFrame>> reci;

	LpnrDipFrames frames_;
	FrameBuffer *stillOutput1_;
	FrameBuffer *stillOutput2_;

	ImgSysRequestHelper requestHelper_;
	Request *request_;
	[[maybe_unused]] uint32_t internalRequestId_;

	LpnrTasksManager *manager_;
};

} /* namespace libcamera */

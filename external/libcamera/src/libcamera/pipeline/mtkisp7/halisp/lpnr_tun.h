/*
 * Copyright (C) 2023, Google Inc.
 *
 * lpnr_tun.h - MTK MtkISP7 LPNR tuning generator
 */

#pragma once

#include <libcamera/base/signal.h>
#include <libcamera/base/thread.h>

#include <libcamera/geometry.h>

#include "libcamera/internal/task_scheduler.h"

#include "pipeline/mtkisp7/imgsys/lpnr.h"
#include "pipeline/mtkisp7/ipa/ipa_delegate.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"

#include "imgsys_task.h"

namespace libcamera {

class DmaHeap;
class MtkISP7CameraData;
class PipelineHandler;
class LpnrTunXtrTask;
class LpnrTunDipTask;

class LpnrTunTasksManager
{
public:
	LpnrTunTasksManager(
		DmaHeap *dmaHeap, IPADelegate *ipa, OnDeviceTuner *odt);

	int configure(const Size &bayerInputSize,
		      const Size &yuvOutput1Size, const Size &yuvOutput2Size);

	void allocateBuffers();
	void releaseBuffers();

	std::tuple<LpnrTunXtrTask *, LpnrTunDipTask *>
	makeLpnrTunTasks(
		LPNRFrames &lpnr,
		SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange,
		uint32_t camSysMetaRequestId,
		Scheduler *scheduler,
		const std::string &id, Request *request,
		uint32_t internalRequestId);

private:
	friend MtkISP7CameraData;
	friend LpnrTunXtrTask;
	friend LpnrTunDipTask;

	bool needCropTNC16x9_;

	Size yuvOutput1Size_;
	Size yuvOutput2Size_;

	Size bayerInputSize_;

	std::vector<Size> lpnrSizes;

	DmaHeap *dmaHeap_;
	InfoFramePool lpnrTun_;

	IPADelegate *ipa_;

	OnDeviceTuner *onDeviceTuner_;
};

class LpnrTunXtrTask : public ImgSysTask
{
public:
	LpnrTunXtrTask(LPNRFrames &lpnr,
		       uint32_t camSysMetaRequestId,
		       Scheduler *scheduler, const std::string &id,
		       Request *request, LpnrTunTasksManager *manager,
		       uint32_t internalRequestId);

	virtual void run() override final;

	SharedMailBox<InfoFrame> xtrTun_;

	Request *request_;

	LpnrTunTasksManager *manager_;
};

class LpnrTunDipTask : public ImgSysTask
{
public:
	LpnrTunDipTask(LPNRFrames &lpnr,
		       SharedMailBox<ipa::mtkisp7::AaaIspExchange> &aaaIspExchange,
		       uint32_t camSysMetaRequestId,
		       Scheduler *scheduler, const std::string &id,
		       Request *request, LpnrTunTasksManager *manager,
		       uint32_t internalRequestId);

	virtual void run() override final;

	SharedMailBox<bool> highIsoMode_;
	SharedMailBox<InfoFrame> xtrStt_;
	SharedMailBox<InfoFrame> dipTunPq_;
	SharedMailBox<InfoFrame> dipTunY2YPq_;
	std::vector<SharedMailBox<InfoFrame>> dipTun_;

	SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange_;

	Request *request_;

	LpnrTunTasksManager *manager_;
};

} /* namespace libcamera */

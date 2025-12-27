/*
 * Copyright (C) 2023, Google Inc.
 *
 * mcnr_tun.h - MTK MtkISP7 MCNR tuning generator
 */

#pragma once

#include <libcamera/base/signal.h>

#include <libcamera/geometry.h>

#include "libcamera/internal/info_frame.h"
#include "libcamera/internal/mailbox.h"
#include "libcamera/internal/task_scheduler.h"

#include "pipeline/mtkisp7/imgsys/mcnr.h"
#include "pipeline/mtkisp7/ipa/ipa_delegate.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"

#include "imgsys_task.h"

namespace libcamera {

class DmaHeap;
class MtkISP7CameraData;
class PipelineHandler;
class McnrMeATask;
class McnrMeBTask;
class McnrTrTask;
class McnrDipTask;

class McnrTunManager
{
public:
	McnrTunManager(DmaHeap *dmaHeap, IPADelegate *ipa, OnDeviceTuner *odt);
	~McnrTunManager();

	int configure(const Size &yuvInputSize, const Size &yuvOutputSize1,
		      const Size &yuvOutputSize2);

	void allocateBuffers();
	void releaseBuffers();

	std::tuple<McnrMeATask *, McnrMeBTask *, McnrTrTask *, McnrDipTask *>
	makeMcnrTunTasks(MCNRFrames &mcnr,
			 uint32_t camSysMetaRequestId,
			 Scheduler *scheduler,
			 const std::string &id, Request *request,
			 uint32_t internalRequestId);

private:
	friend McnrMeATask;
	friend McnrMeBTask;
	friend McnrTrTask;
	friend McnrDipTask;
	friend MtkISP7CameraData;

	bool needCropTNC16x9_;

	Size yuvOutputSize1_;
	Size yuvOutputSize2_;
	Size yuvInputSize_;
	Size yuvInputSize2_;

	std::vector<Size> mcnrSizes;

	InfoFramePool fwmeFst_;
	InfoFramePool fwmmFst_;
	InfoFramePool fwmmRst_;
	InfoFramePool fwmmMil_;
	InfoFramePool fwmmGyro_;

	InfoFramePool swHist_;
	InfoFramePool meTun_;
	InfoFramePool wpeTun_;
	InfoFramePool dipTun_;
	InfoFramePool trawTun_;

	std::vector<InfoFramePool *> poolsWritenByCpu_;

	DmaHeap *dmaHeap_;
	IPADelegate *ipa_;
	OnDeviceTuner *onDeviceTuner_;
};

class McnrMeATask : public ImgSysTask
{
public:
	McnrMeATask(MCNRFrames &mcnr,
		    uint32_t camSysMetaRequestId,
		    Scheduler *scheduler, const std::string &id,
		    Request *request, McnrTunManager *manager,
		    uint32_t internalRequestId);

	virtual void run() override final;

	SharedMailBox<InfoFrame> trMeTun;
	SharedMailBox<InfoFrame> meATun;
	SharedMailBox<InfoFrame> swHist;

	SharedMailBox<InfoFrame> prevFwMeFst;
	SharedMailBox<InfoFrame> prevFwMmFst;
	SharedMailBox<InfoFrame> prevPrevMeAFst;
	SharedMailBox<InfoFrame> prevPrevMeBFst;

	SharedMailBox<InfoFrame> fwMeFst;

	Request *request_;

	McnrTunManager *manager_;
};

class McnrMeBTask : public ImgSysTask
{
public:
	McnrMeBTask(MCNRFrames &mcnr,
		    uint32_t camSysMetaRequestId,
		    Scheduler *scheduler, const std::string &id,
		    Request *request, McnrTunManager *manager,
		    uint32_t internalRequestId);

	virtual void run() override final;

	SharedMailBox<InfoFrame> meATun;
	SharedMailBox<InfoFrame> meBTun;

	SharedMailBox<InfoFrame> meMil;
	SharedMailBox<InfoFrame> fwMeFst;
	SharedMailBox<InfoFrame> fwMmFst;
	SharedMailBox<InfoFrame> fwMmRst;
	SharedMailBox<InfoFrame> fwMmGryo;

	SharedMailBox<InfoFrame> meAFst;
	SharedMailBox<InfoFrame> meAFmb0;

	SharedMailBox<InfoFrame> swHist;

	Request *request_;

	McnrTunManager *manager_;
};

class McnrTrTask : public ImgSysTask
{
public:
	McnrTrTask(MCNRFrames &mcnr,
		   uint32_t camSysMetaRequestId,
		   Scheduler *scheduler, const std::string &id,
		   Request *request, McnrTunManager *manager,
		   uint32_t internalRequestId);

	virtual void run() override final;

	SharedMailBox<InfoFrame> trTunF1;
	SharedMailBox<InfoFrame> trTunF4;

	SharedMailBox<InfoFrame> swHist;

	Request *request_;

	McnrTunManager *manager_;
};

class McnrDipTask : public ImgSysTask
{
public:
	McnrDipTask(MCNRFrames &mcnr,
		    uint32_t camSysMetaRequestId,
		    Scheduler *scheduler, const std::string &id,
		    Request *request, McnrTunManager *manager,
		    uint32_t internalRequestId);

	virtual void run() override final;

	SharedMailBox<InfoFrame> fwMeFst;
	SharedMailBox<InfoFrame> trawStt;

	SharedMailBox<InfoFrame> ltrTunF1;
	SharedMailBox<InfoFrame> ltrTunF4;
	SharedMailBox<InfoFrame> ltrTunVbi;
	SharedMailBox<InfoFrame> wpeTun;
	std::vector<SharedMailBox<InfoFrame>> dipTun;

	SharedMailBox<InfoFrame> swHist;

	Request *request_;

	McnrTunManager *manager_;
};

} /* namespace libcamera */

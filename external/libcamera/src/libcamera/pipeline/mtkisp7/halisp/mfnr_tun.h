/*
 * Copyright (C) 2024, Google Inc.
 *
 * mfnr_tun.h - MTK MtkISP7 MFNR tuning generator
 */

#pragma once

#include <memory>
#include <vector>

#include <libcamera/base/signal.h>
#include <libcamera/base/thread.h>

#include <libcamera/geometry.h>

#include "libcamera/internal/task_scheduler.h"

#include "pipeline/mtkisp7/imgsys/mfnr.h"
#include "pipeline/mtkisp7/ipa/ipa_delegate.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"

#include "imgsys_task.h"

namespace libcamera {

class DmaHeap;
class PipelineHandler;

class MfnrTunBssTask;
class MfnrTunBfbldTask;
class MfnrTunBfmeTask;
class MfnrTunSwmeTask;
class MfnrTunDsTask;
class MfnrTunDsVbiTask;
class MfnrTunMcdsF1Task;
class MfnrTunMsbldTask;
class MfnrTunAfbldTask;
class MtkISP7CameraData;

class MfnrTunManager
{
	friend class MfnrTunBssTask;
	friend class MfnrTunBfbldTask;
	friend class MfnrTunBfmeTask;
	friend class MfnrTunSwmeTask;
	friend class MfnrTunDsTask;
	friend class MfnrTunDsVbiTask;
	friend class MfnrTunMcdsF1Task;
	friend class MfnrTunMsbldTask;
	friend class MfnrTunAfbldTask;

public:
	MfnrTunManager(
		DmaHeap *dmaHeap, IPADelegate *ipa, OnDeviceTuner *odt);

	int configure(const Size &bayerInputSize,
		      const Size &yuvOutput1Size, const Size &yuvOutput2Size,
		      const Size &mfnrSize_aligned16, const Size &wrappingMapSize,
		      const Size &confMapSize);

	void allocateBuffers();
	void releaseBuffers();

	std::tuple<MfnrTunBssTask *, MfnrTunBfbldTask *, MfnrTunBfmeTask *,
		   MfnrTunSwmeTask *, MfnrTunDsTask *, MfnrTunDsVbiTask *,
		   MfnrTunMcdsF1Task *, MfnrTunMsbldTask *, MfnrTunMsbldTask *,
		   MfnrTunAfbldTask *>
	makeMfnrTunTasks(
		MFNRFrames &mfnr,
		SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange,
		uint32_t camSysMetaRequestId,
		Scheduler *scheduler,
		const std::string &id, Request *request,
		uint32_t internalRequestId);

private:
	friend MtkISP7CameraData;

	Size yuvOutput1Size_;
	Size yuvOutput2Size_;

	Size bayerInputSize_;
	Size mfnrSize_aligned16_;

	bool needCropTNC16x9_;

	std::vector<Size> mfnrSizes_;

	DmaHeap *dmaHeap_;
	InfoFramePool mfnrTun_;

	InfoFramePool tnrciPool_;
	InfoFramePool wrap2pPool_;
	InfoFramePool fourBytes_1_16_pool_;
	InfoFramePool swmeOutPool_;
	InfoFramePool swmeParamPool_;
	InfoFramePool swmeTuningPool_;

	InfoFramePool bssParamPool_;
	InfoFramePool bssDataGPool_;
	InfoFramePool bssVerPool_;
	InfoFramePool bssTuningPool_;
	InfoFramePool bssFdMainPool_;
	InfoFramePool bssFdPool_;
	InfoFramePool bssFacePool_;
	InfoFramePool bssPosPool_;
	InfoFramePool bssOutDataPool_;

	Size wrappingMapSize_;
	Size confMapSize_;

	IPADelegate *ipa_;

	OnDeviceTuner *onDeviceTuner_;
};

class MfnrTunBssTask : public Task
{
public:
	MfnrTunBssTask(MFNRFrames &mfnr,
		       SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange,
		       Scheduler *scheduler, const std::string &id,
		       MfnrTunManager *manager, uint32_t internalRequestId);

	virtual void run() override final;

	void notifyBssResult(const std::vector<int32_t> &bssOrder);

	BssFrames bssFrames_;

	MfnrTunManager *manager_;
	uint32_t internalRequestId_;
	SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange_;

private:
	void allocateBuffers();
};

class MfnrTunBfbldTask : public ImgSysTask
{
public:
	MfnrTunBfbldTask(MFNRFrames &mfnr,
			 uint32_t camSysMetaRequestId,
			 Scheduler *scheduler, const std::string &id,
			 Request *request, MfnrTunManager *manager,
			 uint32_t internalRequestId);

	virtual void run() override final;

	std::vector<SharedMailBox<InfoFrame>> bfbldTun_;
	SharedMailBox<std::vector<int>> bssOrder_;

	Request *request_;

	MfnrTunManager *manager_;
};

class MfnrTunBfmeTask : public ImgSysTask
{
public:
	MfnrTunBfmeTask(MFNRFrames &mfnr,
			uint32_t camSysMetaRequestId,
			Scheduler *scheduler, const std::string &id,
			Request *request, MfnrTunManager *manager,
			uint32_t internalRequestId);

	virtual void run() override final;

	std::vector<SharedMailBox<InfoFrame>> bfmeTun_;
	SharedMailBox<std::vector<int>> bssOrder_;
	SharedMailBox<InfoFrame> tncso_;

	Request *request_;

	MfnrTunManager *manager_;
};

class MfnrTunSwmeTask : public Task
{
public:
	MfnrTunSwmeTask(MFNRFrames &mfnr,
			Scheduler *scheduler, const std::string &id,
			MfnrTunManager *manager,
			uint32_t internalRequestId);

	virtual void run() override final;

	void notifySwmeResultReady();

	SharedMailBox<std::vector<int>> bssOrder_;
	SwmeFrames swmeFrames_;

	MfnrTunManager *manager_;
	uint32_t internalRequestId_;

private:
	void allocateBuffers();
};

class MfnrTunDsTask : public ImgSysTask
{
public:
	MfnrTunDsTask(MFNRFrames &mfnr,
		      uint32_t camSysMetaRequestId,
		      Scheduler *scheduler, const std::string &id,
		      Request *request, MfnrTunManager *manager,
		      uint32_t internalRequestId);

	virtual void run() override final;

	std::vector<SharedMailBox<InfoFrame>> dsTun;
	SharedMailBox<std::vector<int>> bssOrder_;

	Request *request_;

	MfnrTunManager *manager_;
};

class MfnrTunDsVbiTask : public ImgSysTask
{
public:
	MfnrTunDsVbiTask(MFNRFrames &mfnr,
			 uint32_t camSysMetaRequestId,
			 Scheduler *scheduler, const std::string &id,
			 Request *request, MfnrTunManager *manager,
			 uint32_t internalRequestId);

	virtual void run() override final;

	std::vector<SharedMailBox<InfoFrame>> dsVbiV2Tun_;
	std::vector<SharedMailBox<InfoFrame>> dsVbiV5Tun_;
	SharedMailBox<std::vector<int>> bssOrder_;

	Request *request_;

	MfnrTunManager *manager_;
};

class MfnrTunMcdsF1Task : public ImgSysTask
{
public:
	MfnrTunMcdsF1Task(MFNRFrames &mfnr,
			  uint32_t camSysMetaRequestId,
			  Scheduler *scheduler, const std::string &id,
			  Request *request, MfnrTunManager *manager,
			  uint32_t internalRequestId);

	virtual void run() override final;

	std::vector<SharedMailBox<InfoFrame>> mcdsF1Tun_;
	SharedMailBox<std::vector<int>> bssOrder_;

	Request *request_;

	MfnrTunManager *manager_;
};

class MfnrTunMsbldTask : public ImgSysTask
{
public:
	MfnrTunMsbldTask(MFNRFrames &mfnr,
			 uint32_t camSysMetaRequestId,
			 Scheduler *scheduler, const std::string &id,
			 Request *request, MfnrTunManager *manager,
			 uint32_t internalRequestId, int msbldInd);

	virtual void run() override final;

	SharedMailBox<InfoFrame> msbldF0Tun_;
	SharedMailBox<InfoFrame> msbldF1Tun_;
	SharedMailBox<InfoFrame> msbldF2Tun_;
	SharedMailBox<InfoFrame> msbldF3Tun_;
	SharedMailBox<InfoFrame> msbldF4Tun_;
	SharedMailBox<InfoFrame> msbldF5Tun_;
	SharedMailBox<InfoFrame> msbldF6Tun_;
	SharedMailBox<std::vector<int>> bssOrder_;

	Request *request_;

	MfnrTunManager *manager_;

	int msbldIdx_;
};

class MfnrTunAfbldTask : public ImgSysTask
{
public:
	MfnrTunAfbldTask(MFNRFrames &mfnr,
			 uint32_t camSysMetaRequestId,
			 Scheduler *scheduler, const std::string &id,
			 Request *request, MfnrTunManager *manager,
			 uint32_t internalRequestId);

	virtual void run() override final;

	SharedMailBox<InfoFrame> afbldF0Tun_;
	SharedMailBox<InfoFrame> afbldF1Tun_;
	SharedMailBox<InfoFrame> afbldF2Tun_;
	SharedMailBox<InfoFrame> afbldF3Tun_;
	SharedMailBox<InfoFrame> afbldF4Tun_;
	SharedMailBox<InfoFrame> afbldF5Tun_;
	SharedMailBox<InfoFrame> afbldF6Tun_;
	SharedMailBox<std::vector<int>> bssOrder_;

	SharedMailBox<InfoFrame> tncso_;

	Request *request_;

	MfnrTunManager *manager_;
};

} /* namespace libcamera */

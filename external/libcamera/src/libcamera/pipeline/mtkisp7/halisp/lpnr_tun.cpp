/*
 * Copyright (C) 2023, Google Inc.
 *
 * lpnr_tun.cpp - MTK MtkISP7 LPNR tuning generator
 */

#include "lpnr_tun.h"

#include <libcamera/base/signal.h>
#include <libcamera/base/thread.h>

#include <libcamera/formats.h>
#include <libcamera/geometry.h>

#include "libcamera/internal/task_scheduler.h"

#include "../utils/img_meta_request_data_helper.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/feature.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

namespace {

static constexpr Size kTunSize{ 219348, 1 };

} //namespace

[[maybe_unused]] static void fillTuning(SharedMailBox<InfoFrame> &mailBox, uint8_t *tuning)
{
	assert(tuning);

	const InfoFrame &info = mailBox->get();

	void *dest = info.address(0);
	size_t length = info.buffer()->planes()[0].length;

	assert(dest);
	assert(mailBox->valid());

	{
		DmaSyncer syncer(info.buffer()->planes()[0].fd.get());
		memcpy(dest, tuning, length);
	}
}

LpnrTunTasksManager::LpnrTunTasksManager(
	DmaHeap *dmaHeap, IPADelegate *ipa, OnDeviceTuner *odt)
{
	dmaHeap_ = dmaHeap;
	ipa_ = ipa;
	onDeviceTuner_ = odt;
}

void LpnrTunTasksManager::allocateBuffers()
{
	// In enforced lowIsoMode, it's 15. In highIsoMode though, it's 18.
	lpnrTun_.createBuffers(dmaHeap_, formats::MTFD_MTISP, kTunSize, 15, DmaHeap::CMA);
}

void LpnrTunTasksManager::releaseBuffers()
{
	lpnrTun_.release();
}

int LpnrTunTasksManager::configure(const Size &bayerInputSize,
				   const Size &yuvOutput1Size, const Size &yuvOutput2Size)
{
	yuvOutput1Size_ = yuvOutput1Size;
	yuvOutput2Size_ = yuvOutput2Size;

	bayerInputSize_ = bayerInputSize;

	lpnrSizes.resize(4);
	Size size = bayerInputSize_;

	/* Assign the size to 1/4 of the previous level.
	 * Align to 2 for hardware's requirement */
	for (size_t i = 0; i < lpnrSizes.size(); i++) {
		lpnrSizes[i] = size;
		size.width = (size.width + 3) / 4;
		size.height = (size.height + 3) / 4;
		size.alignUpTo(2, 2);
	}

	needCropTNC16x9_ = false;
	if ((yuvOutput1Size_.width * 9 == yuvOutput1Size_.height * 16) &&
	    (yuvOutput2Size_.width * 9 == yuvOutput2Size_.height * 16))
		needCropTNC16x9_ = true;

	allocateBuffers();

	return 0;
}

std::tuple<LpnrTunXtrTask *, LpnrTunDipTask *>
LpnrTunTasksManager::makeLpnrTunTasks(LPNRFrames &lpnr,
				      SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange,
				      uint32_t camSysMetaRequestId,
				      Scheduler *scheduler,
				      const std::string &id, Request *request,
				      uint32_t internalRequestId)
{
	LpnrTunXtrTask *lpnrTunXtrTask = new LpnrTunXtrTask(
		lpnr, camSysMetaRequestId, scheduler, id + " Xtr", request, this, internalRequestId);

	LpnrTunDipTask *lpnrTunDipTask = new LpnrTunDipTask(
		lpnr, aaaIspExchange, camSysMetaRequestId, scheduler, id + " Dip", request, this, internalRequestId);

	return std::make_tuple(lpnrTunXtrTask, lpnrTunDipTask);
}

LpnrTunXtrTask::LpnrTunXtrTask(LPNRFrames &lpnr,
			       uint32_t camSysMetaRequestId,
			       Scheduler *scheduler,
			       const std::string &id, Request *request, LpnrTunTasksManager *manager,
			       uint32_t internalRequestId)
	: ImgSysTask(scheduler, id, camSysMetaRequestId, internalRequestId,
		     Feature::Capture_lpnr, manager->ipa_, request->controls()),
	  request_(request), manager_(manager)
{
	xtrTun_ = lpnr.xtrFrames.in.xtrTun;
}

void LpnrTunXtrTask::run()
{
	manager_->lpnrTun_.fetch(xtrTun_);
	const InfoFrame &frame = xtrTun_->get();

	auto request = makeImgMetaRequestDataNonMfnr(
		true, EStage_TR_R2Y, frame.buffer()->cookie(),
		0, 0, manager_->bayerInputSize_, manager_->bayerInputSize_,
		{}, manager_->lpnrSizes[0], {});

	getImgSysMetaTuning(
		manager_->needCropTNC16x9_, { request });
}

LpnrTunDipTask::LpnrTunDipTask(LPNRFrames &lpnr,
			       SharedMailBox<ipa::mtkisp7::AaaIspExchange> &aaaIspExchange,
			       uint32_t camSysMetaRequestId,
			       Scheduler *scheduler,
			       const std::string &id, Request *request, LpnrTunTasksManager *manager,
			       uint32_t internalRequestId)
	: ImgSysTask(scheduler, id, camSysMetaRequestId, internalRequestId,
		     Feature::Capture_lpnr, manager->ipa_, request->controls()),
	  request_(request), manager_(manager)
{
	highIsoMode_ = lpnr.lpnrDipFrames.in.highIsoMode;
	xtrStt_ = lpnr.xtrFrames.out.xtrStt;
	dipTunPq_ = lpnr.lpnrDipFrames.in.dipTunPq;
	dipTunY2YPq_ = lpnr.lpnrDipFrames.in.dipTunY2YPq;
	dipTun_ = lpnr.lpnrDipFrames.in.dipTun;

	aaaIspExchange_ = aaaIspExchange;
}

void LpnrTunDipTask::run()
{
	ipa::mtkisp7::ImgMetaRequestData request;
	std::vector<ipa::mtkisp7::ImgMetaRequestData> requests;
	auto *aaaIspExchange = &aaaIspExchange_->get();

	bool highIsoMode = aaaIspExchange->highIsoMode;
	if (manager_->onDeviceTuner_->isLowIsoLpnrEnforced()) {
		highIsoMode = false;
	}
	highIsoMode_->put(highIsoMode, nullptr);

	manager_->lpnrTun_.fetch(dipTun_[3]);

	request = makeImgMetaRequestDataNonMfnr(
		true, EStage_P2_MS_F3, dipTun_[3]->get().buffer()->cookie(),
		0, 0, manager_->lpnrSizes[3], manager_->yuvOutput1Size_,
		manager_->yuvOutput2Size_, manager_->lpnrSizes[0], {});
	requests.push_back(std::move(request));

	manager_->lpnrTun_.fetch(dipTun_[2]);

	request = makeImgMetaRequestDataNonMfnr(
		true, EStage_P2_MS_F2, dipTun_[2]->get().buffer()->cookie(),
		0, 0, manager_->lpnrSizes[2], manager_->yuvOutput1Size_,
		manager_->yuvOutput2Size_, manager_->lpnrSizes[0], {});
	requests.push_back(std::move(request));

	manager_->lpnrTun_.fetch(dipTun_[1]);

	request = makeImgMetaRequestDataNonMfnr(
		true, EStage_P2_MS_F1, dipTun_[1]->get().buffer()->cookie(),
		0, 0, manager_->lpnrSizes[1], manager_->yuvOutput1Size_,
		manager_->yuvOutput2Size_, manager_->lpnrSizes[0], {});
	requests.push_back(std::move(request));

	if (highIsoMode) {
		manager_->lpnrTun_.fetch(dipTun_[0]);

		request = makeImgMetaRequestDataNonMfnr(
			true, EStage_P2_MS_F0_H, dipTun_[0]->get().buffer()->cookie(),
			0, 0, manager_->lpnrSizes[0], manager_->yuvOutput1Size_,
			manager_->yuvOutput2Size_, manager_->lpnrSizes[0], {});
		requests.push_back(std::move(request));

		manager_->lpnrTun_.fetch(dipTunY2YPq_);

		request = makeImgMetaRequestDataNonMfnr(
			true, EStage_P2_Y2Y_PQ_DIP,
			dipTunY2YPq_->get().buffer()->cookie(),
			xtrStt_->get().buffer()->cookie(), 0,
			manager_->lpnrSizes[0], manager_->yuvOutput1Size_,
			manager_->yuvOutput2Size_, manager_->lpnrSizes[0], {});
		requests.push_back(std::move(request));
	} else {
		manager_->lpnrTun_.fetch(dipTunPq_);

		request = makeImgMetaRequestDataNonMfnr(
			true, EStage_P2_MS_F0_PQ_DIP,
			dipTunPq_->get().buffer()->cookie(),
			xtrStt_->get().buffer()->cookie(), 0,
			manager_->lpnrSizes[0], manager_->yuvOutput1Size_,
			manager_->yuvOutput2Size_, manager_->lpnrSizes[0], {});
		requests.push_back(std::move(request));
	}

	getImgSysMetaTuning(
		manager_->needCropTNC16x9_, requests);
}

} /* namespace libcamera */

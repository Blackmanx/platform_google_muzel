/*
 * Copyright (C) 2023, Google Inc.
 *
 * mcnr_tun.cpp - MTK MtkISP7 MCNR tuning generator
 */

#include "mcnr_tun.h"

#include <libcamera/base/signal.h>

#include <libcamera/formats.h>
#include <libcamera/geometry.h>

#include "libcamera/internal/info_frame.h"
#include "libcamera/internal/mailbox.h"
#include "libcamera/internal/task_scheduler.h"

#include "../utils/img_meta_request_data_helper.h"
#include "mtkcam-interfaces/isphal/IspTuningMeta.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

namespace {

constexpr Size kMeL1Size{ 144, 108 };
constexpr Size kTunSize{ 219348, 1 };
constexpr Size kHistSize{ 11776, 1 };

static void zeroImage(SharedMailBox<InfoFrame> &mailBox)
{
	const InfoFrame &info = mailBox->get();

	void *dest = info.address(0);
	size_t length = info.buffer()->planes()[0].length;

	assert(dest);
	assert(mailBox->valid());

	{
		DmaSyncer syncer(info.buffer()->planes()[0].fd.get(), DmaHeap::SyncWrite);
		memset(dest, 0, length);
	}
}

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

McnrTunManager::McnrTunManager(
	DmaHeap *dmaHeap, IPADelegate *ipa, OnDeviceTuner *odt)
{
	poolsWritenByCpu_.emplace_back(&fwmeFst_);
	poolsWritenByCpu_.emplace_back(&fwmmFst_);
	poolsWritenByCpu_.emplace_back(&fwmmRst_);
	poolsWritenByCpu_.emplace_back(&fwmmMil_);
	poolsWritenByCpu_.emplace_back(&fwmmGyro_);
	poolsWritenByCpu_.emplace_back(&swHist_);

	poolsWritenByCpu_.emplace_back(&meTun_);
	poolsWritenByCpu_.emplace_back(&wpeTun_);
	poolsWritenByCpu_.emplace_back(&dipTun_);
	poolsWritenByCpu_.emplace_back(&trawTun_);

	dmaHeap_ = dmaHeap;
	ipa_ = ipa;
	onDeviceTuner_ = odt;
}

McnrTunManager::~McnrTunManager() = default;

void McnrTunManager::allocateBuffers()
{
	fwmeFst_.createBuffers(dmaHeap_, formats::Y8_MTISP, Size{ 400, 1 }, 8);
	fwmmFst_.createBuffers(dmaHeap_, formats::Y8_MTISP, Size{ 80, 1 }, 8);
	fwmmRst_.createBuffers(dmaHeap_, formats::Y8_MTISP, Size{ 132, 1 }, 8);
	fwmmGyro_.createBuffers(dmaHeap_, formats::Y32_MTISP, Size{ 32, 24 }, 8);
	fwmmMil_.createBuffers(dmaHeap_, formats::Y8_MTISP, kMeL1Size, 8);
	dipTun_.createBuffers(dmaHeap_, formats::MTFD_MTISP, kTunSize, 21, DmaHeap::CMA);
	meTun_.createBuffers(dmaHeap_, formats::MTFD_MTISP, kTunSize, 6, DmaHeap::CMA);
	trawTun_.createBuffers(dmaHeap_, formats::MTFD_MTISP, kTunSize, 28, DmaHeap::CMA);
	wpeTun_.createBuffers(dmaHeap_, formats::MTFD_MTISP, kTunSize, 4, DmaHeap::CMA);
	swHist_.createBuffers(dmaHeap_, formats::Y8_MTISP, kHistSize, 12);

	for (auto &pool : poolsWritenByCpu_)
		pool->mmap();
}

void McnrTunManager::releaseBuffers()
{
	for (auto &pool : poolsWritenByCpu_) {
		pool->unmap();
		pool->release();
	}
}

int McnrTunManager::configure(const Size &yuvInputSize, const Size &yuvOutputSize1,
			      const Size &yuvOutputSize2)
{
	yuvOutputSize1_ = yuvOutputSize1;
	yuvOutputSize2_ = yuvOutputSize2;
	yuvInputSize_ = yuvInputSize;
	yuvInputSize2_ = yuvInputSize / 2;

	mcnrSizes.resize(7);
	Size size = yuvInputSize_;

	/* Assign the size to 1/2 of the previous level.
	 * Align to 2 for hardware's requirement */
	for (size_t i = 0; i < mcnrSizes.size(); i++) {
		mcnrSizes[i] = size;
		size.width = (size.width + 1) / 2;
		size.height = (size.height + 1) / 2;
		size.alignUpTo(2, 2);
	}

	needCropTNC16x9_ = false;
	if ((yuvOutputSize1_.width * 9 == yuvOutputSize1_.height * 16) &&
	    (yuvOutputSize2_.width * 9 == yuvOutputSize2_.height * 16))
		needCropTNC16x9_ = true;

	allocateBuffers();

	return 0;
}

std::tuple<McnrMeATask *, McnrMeBTask *, McnrTrTask *, McnrDipTask *>
McnrTunManager::makeMcnrTunTasks(
	MCNRFrames &mcnr,
	uint32_t camSysMetaRequestId,
	Scheduler *scheduler,
	const std::string &id, Request *request,
	uint32_t internalRequestId)
{
	McnrMeATask *meATunTask = new McnrMeATask(
		mcnr, camSysMetaRequestId, scheduler, id, request, this, internalRequestId);

	McnrMeBTask *meBTunTask = new McnrMeBTask(
		mcnr, camSysMetaRequestId, scheduler, id, request, this, internalRequestId);

	McnrTrTask *trTunTask = new McnrTrTask(
		mcnr, camSysMetaRequestId, scheduler, id, request, this, internalRequestId);

	McnrDipTask *dipTunTask = new McnrDipTask(
		mcnr, camSysMetaRequestId, scheduler, id, request, this, internalRequestId);

	return std::make_tuple(meATunTask, meBTunTask, trTunTask, dipTunTask);
}

McnrMeATask::McnrMeATask(MCNRFrames &mcnr,
			 uint32_t camSysMetaRequestId,
			 Scheduler *scheduler,
			 const std::string &id, Request *request, McnrTunManager *manager,
			 uint32_t internalRequestId)
	: ImgSysTask(scheduler, id, camSysMetaRequestId, internalRequestId,
		     Feature::Preview, manager->ipa_, request->controls()),
	  request_(request), manager_(manager)
{
	(void)mcnr;

	trMeTun = mcnr.meFrames.in.trMeTun;
	meATun = mcnr.meFrames.in.meATun;

	prevFwMeFst = mcnr.meFrames.in.prevFwMeFst;
	prevFwMmFst = mcnr.meFrames.in.prevFwMmFst;
	prevPrevMeAFst = mcnr.meFrames.in.prevPrevMeAFst;
	prevPrevMeBFst = mcnr.meFrames.in.prevPrevMeBFst;

	fwMeFst = mcnr.meFrames.in.fwMeFst;

	swHist = mcnr.dip1Frames.out.swHist;
}

void McnrMeATask::run()
{
	manager_->swHist_.fetch(swHist);
	zeroImage(swHist);

	manager_->trawTun_.fetch(trMeTun);

	ipa::mtkisp7::ImgMetaRequestData request;
	std::vector<ipa::mtkisp7::ImgMetaRequestData> requests;

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_LTR_ME_L1, trMeTun->get().buffer()->cookie(),
		0, swHist->get().buffer()->cookie(), Size{ 576, 432 },
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	requests.push_back(std::move(request));

	manager_->meTun_.fetch(meATun);
	manager_->fwmeFst_.fetch(fwMeFst);

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_ME_3PASS_MODE0, meATun->get().buffer()->cookie(),
		0, swHist->get().buffer()->cookie(), manager_->yuvInputSize_,
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	request.reserved[mtk::isphal::kISPExtBif_IN_FWME_FST] = prevFwMeFst->get().buffer()->cookie();
	request.reserved[mtk::isphal::kISPExtBif_IN_FWMM_MMG_FBFST] = prevFwMmFst->get().buffer()->cookie();
	request.reserved[mtk::isphal::kISPExtBif_IN_HWME_STAT_FST_MD0] = prevPrevMeAFst->get().buffer()->cookie();
	request.reserved[mtk::isphal::kISPExtBif_IN_HWME_STAT_FST_MD1] = prevPrevMeBFst->get().buffer()->cookie();
	request.reserved[mtk::isphal::kISPExtBif_OUT_FWME_FST] = fwMeFst->get().buffer()->cookie();
	requests.push_back(std::move(request));

	getImgSysMetaTuning(manager_->needCropTNC16x9_, requests);
}

McnrMeBTask::McnrMeBTask(MCNRFrames &mcnr,
			 uint32_t camSysMetaRequestId,
			 Scheduler *scheduler,
			 const std::string &id, Request *request, McnrTunManager *manager,
			 uint32_t internalRequestId)
	: ImgSysTask(scheduler, id, camSysMetaRequestId, internalRequestId,
		     Feature::Preview, manager->ipa_, request->controls()),
	  request_(request), manager_(manager)
{
	meBTun = mcnr.meFrames.in.meBTun;
	meMil = mcnr.meFrames.in.meMil;

	meATun = mcnr.meFrames.in.meATun;
	fwMeFst = mcnr.meFrames.in.fwMeFst;
	fwMmFst = mcnr.meFrames.in.fwMmFst;
	fwMmRst = mcnr.meFrames.in.fwMmRst;
	fwMmGryo = makeMailBox<InfoFrame>();

	meAFst = mcnr.meFrames.out.meAFst;
	meAFmb0 = mcnr.meFrames.out.meAFmb0;

	swHist = mcnr.dip1Frames.out.swHist;
}

void McnrMeBTask::run()
{
	manager_->meTun_.fetch(meBTun);
	manager_->fwmmFst_.fetch(fwMmFst);
	manager_->fwmmRst_.fetch(fwMmRst);
	manager_->fwmmMil_.fetch(meMil);
	manager_->fwmmGyro_.fetch(fwMmGryo);

	/* TODO: Read the Gyro data from gyro sensor */
	zeroImage(fwMmGryo);

	ipa::mtkisp7::ImgMetaRequestData request;
	std::vector<ipa::mtkisp7::ImgMetaRequestData> requests;

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_ME_3PASS_MM, meBTun->get().buffer()->cookie(),
		0, swHist->get().buffer()->cookie(), manager_->yuvInputSize_,
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	request.reserved[mtk::isphal::kISPExtBif_IN_HWME_STAT_FST_MD0] = meAFst->get().buffer()->cookie();
	request.reserved[mtk::isphal::kISPExtBif_IN_HWME_STAT_FMB_MD0] = meAFmb0->get().buffer()->cookie();
	request.reserved[mtk::isphal::kISPExtBif_OUT_FWMM_MMG_FBFST] = fwMmFst->get().buffer()->cookie();
	request.reserved[mtk::isphal::kISPExtBif_OUT_FWMM_MMG_RST] = fwMmRst->get().buffer()->cookie();
	request.reserved[mtk::isphal::kISPExtBif_OUT_FWMM_MIL] = meMil->get().buffer()->cookie();
	request.reserved[mtk::isphal::kISPExtBif_IN_GYRO_MV] = fwMmGryo->get().buffer()->cookie();
	requests.push_back(std::move(request));

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_ME_3PASS_MODE1, meBTun->get().buffer()->cookie(),
		0, swHist->get().buffer()->cookie(), manager_->yuvInputSize_,
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	request.reserved[mtk::isphal::kISPExtBif_IN_HWME_MODE_0_TUN_BUF] = meATun->get().buffer()->cookie();
	request.reserved[mtk::isphal::kISPExtBif_IN_HWME_STAT_FST_MD0] = meAFst->get().buffer()->cookie();
	request.reserved[mtk::isphal::kISPExtBif_IN_FWMM_MMG_RST] = fwMmRst->get().buffer()->cookie();
	request.reserved[mtk::isphal::kISPExtBif_IN_FWME_FST] = fwMeFst->get().buffer()->cookie();
	requests.push_back(std::move(request));

	getImgSysMetaTuning(manager_->needCropTNC16x9_, requests);
}

McnrTrTask::McnrTrTask(MCNRFrames &mcnr,
		       uint32_t camSysMetaRequestId,
		       Scheduler *scheduler,
		       const std::string &id, Request *request, McnrTunManager *manager,
		       uint32_t internalRequestId)
	: ImgSysTask(scheduler, id, camSysMetaRequestId, internalRequestId,
		     Feature::Preview, manager->ipa_, request->controls()),
	  request_(request), manager_(manager)
{
	trTunF1 = mcnr.trFrames.in.trTunF1;
	trTunF4 = mcnr.trFrames.in.trTunF4;

	swHist = mcnr.dip1Frames.out.swHist;
}

void McnrTrTask::run()
{
	ipa::mtkisp7::ImgMetaRequestData request;
	std::vector<ipa::mtkisp7::ImgMetaRequestData> requests;

	manager_->trawTun_.fetch(trTunF1);

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_TR_Y2Y_F1, trTunF1->get().buffer()->cookie(),
		0, swHist->get().buffer()->cookie(), manager_->mcnrSizes[1],
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	requests.push_back(std::move(request));

	manager_->trawTun_.fetch(trTunF4);

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_TR_Y2Y_F4, trTunF4->get().buffer()->cookie(),
		0, swHist->get().buffer()->cookie(), manager_->mcnrSizes[4],
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	requests.push_back(std::move(request));

	getImgSysMetaTuning(manager_->needCropTNC16x9_, requests);
}

McnrDipTask::McnrDipTask(MCNRFrames &mcnr,
			 uint32_t camSysMetaRequestId,
			 Scheduler *scheduler,
			 const std::string &id, Request *request, McnrTunManager *manager,
			 uint32_t internalRequestId)
	: ImgSysTask(scheduler, id, camSysMetaRequestId, internalRequestId,
		     Feature::Preview, manager->ipa_, request->controls()),
	  request_(request), manager_(manager)
{
	fwMeFst = mcnr.meFrames.in.fwMeFst;
	trawStt = mcnr.trFrames.out.trawStt;

	ltrTunF1 = mcnr.dip1Frames.in.ltrTunF1;
	ltrTunF4 = mcnr.dip1Frames.in.ltrTunF4;
	ltrTunVbi = mcnr.dip1Frames.in.ltrTunVbi;
	wpeTun = mcnr.dip1Frames.in.wpeTun;
	dipTun = mcnr.dip1Frames.in.dipTun;

	swHist = mcnr.dip1Frames.out.swHist;
}

void McnrDipTask::run()
{
	ipa::mtkisp7::ImgMetaRequestData request;
	std::vector<ipa::mtkisp7::ImgMetaRequestData> requests;

	manager_->trawTun_.fetch(ltrTunF1);

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_WPE_LTR_Y2Y_F1, ltrTunF1->get().buffer()->cookie(),
		0, swHist->get().buffer()->cookie(), manager_->mcnrSizes[1],
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	requests.push_back(std::move(request));

	manager_->trawTun_.fetch(ltrTunF4);

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_LTR_Y2Y_F4, ltrTunF4->get().buffer()->cookie(),
		0, swHist->get().buffer()->cookie(), manager_->mcnrSizes[4],
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	requests.push_back(std::move(request));

	manager_->trawTun_.fetch(ltrTunVbi);

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_LTR_VBI, ltrTunVbi->get().buffer()->cookie(),
		0, swHist->get().buffer()->cookie(), manager_->mcnrSizes[3],
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	requests.push_back(std::move(request));

	manager_->wpeTun_.fetch(wpeTun);

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_WPE_WghtMap, wpeTun->get().buffer()->cookie(),
		0, swHist->get().buffer()->cookie(), manager_->yuvInputSize_,
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	requests.push_back(std::move(request));

	for (size_t i = 0; i < dipTun.size(); i++) {
		manager_->dipTun_.fetch(dipTun[i]);
	}

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_P2_IDI, dipTun[6]->get().buffer()->cookie(),
		trawStt->get().buffer()->cookie(),
		swHist->get().buffer()->cookie(), manager_->mcnrSizes[6],
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	request.reserved[mtk::isphal::kISPExtBif_IN_FWME_FST] = fwMeFst->get().buffer()->cookie();
	requests.push_back(std::move(request));

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_P2_MS_F_SMALL, dipTun[5]->get().buffer()->cookie(),
		trawStt->get().buffer()->cookie(),
		swHist->get().buffer()->cookie(), manager_->mcnrSizes[5],
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	request.reserved[mtk::isphal::kISPExtBif_IN_FWME_FST] = fwMeFst->get().buffer()->cookie();
	requests.push_back(std::move(request));

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_P2_MS_F4, dipTun[4]->get().buffer()->cookie(),
		trawStt->get().buffer()->cookie(),
		swHist->get().buffer()->cookie(), manager_->mcnrSizes[4],
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	request.reserved[mtk::isphal::kISPExtBif_IN_FWME_FST] = fwMeFst->get().buffer()->cookie();
	requests.push_back(std::move(request));

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_P2_MS_F3, dipTun[3]->get().buffer()->cookie(),
		trawStt->get().buffer()->cookie(),
		swHist->get().buffer()->cookie(), manager_->mcnrSizes[3],
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	request.reserved[mtk::isphal::kISPExtBif_IN_FWME_FST] = fwMeFst->get().buffer()->cookie();
	requests.push_back(std::move(request));

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_P2_MS_F2, dipTun[2]->get().buffer()->cookie(),
		trawStt->get().buffer()->cookie(),
		swHist->get().buffer()->cookie(), manager_->mcnrSizes[2],
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	request.reserved[mtk::isphal::kISPExtBif_IN_FWME_FST] = fwMeFst->get().buffer()->cookie();
	requests.push_back(std::move(request));

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_P2_MS_F1, dipTun[1]->get().buffer()->cookie(),
		trawStt->get().buffer()->cookie(),
		swHist->get().buffer()->cookie(), manager_->mcnrSizes[1],
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	request.reserved[mtk::isphal::kISPExtBif_IN_FWME_FST] = fwMeFst->get().buffer()->cookie();
	requests.push_back(std::move(request));

	request = makeImgMetaRequestDataNonMfnr(
		false, EStage_WPE_P2_PQDIP_MS_F0, dipTun[0]->get().buffer()->cookie(),
		trawStt->get().buffer()->cookie(),
		swHist->get().buffer()->cookie(), manager_->mcnrSizes[0],
		manager_->yuvOutputSize1_, manager_->yuvOutputSize2_,
		manager_->yuvInputSize_, {});
	request.reserved[mtk::isphal::kISPExtBif_IN_FWME_FST] = fwMeFst->get().buffer()->cookie();
	requests.push_back(std::move(request));

	getImgSysMetaTuning(manager_->needCropTNC16x9_, requests);
}

} /* namespace libcamera */

/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * mcnr.cpp - MtkISP7 ImgSys Device Motion Compensation Noise Reduction
 */

#include "mcnr.h"

#include <libcamera/control_ids.h>
#include <libcamera/formats.h>
#include <libcamera/request.h>

#include "libcamera/internal/dma_heaps.h"
#include "libcamera/internal/framebuffer.h"
#include "libcamera/internal/media_device.h"
#include "libcamera/internal/task_scheduler.h"

#include "pipeline/mtkisp7/imgsys/imgsys.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"
#include "platform/mtkisp7/IImgStreamDef.h"

#include "single_device.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

namespace {

constexpr Size kMeL0Size{ 576, 432 };
constexpr Size kMeL1Size{ 144, 108 };

constexpr Size kMeMapSize0{ 289, 217 };
constexpr Size kMeMapSize1{ 145, 109 };
constexpr Size kMeMapSize2{ 73, 55 };
constexpr Size kMeMapSize3{ 37, 28 };

constexpr Size kFmbSize{ 36, 27 };
constexpr Size kFstSize{ 1, 112 };
constexpr Size kTnrsoSize{ 40, 1 };

constexpr Size kTrawSttSize{ 738624, 1 };

constexpr Size kFwMeFstSize{ 400, 1 };
constexpr Size kFwMmFstSize{ 80, 1 };
constexpr Size kFwMmRstSize{ 132, 1 };

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

} // namespace

/* todo: hide the NSCam::NSImgStream namespace in the single device interface. */
using namespace NSCam::NSImgStream;

McnrTasksManager::McnrTasksManager(
	ImgSysDevice *imgSys, DmaHeap *dmaHeap, OnDeviceTuner *odt)
{
	imgSys_ = imgSys;
	dmaHeap_ = dmaHeap;
	onDeviceTuner_ = odt;

	allBufferPools_.emplace_back(&fwmeFst_);
	allBufferPools_.emplace_back(&fwmmFst);
	allBufferPools_.emplace_back(&fwmmRst_);
	allBufferPools_.emplace_back(&fwmmMil_);
	allBufferPools_.emplace_back(&fwmmGyro_);
	allBufferPools_.emplace_back(&meIn_);
	allBufferPools_.emplace_back(&meMv0_);
	allBufferPools_.emplace_back(&meMv1_);

	allBufferPools_.emplace_back(&meFst_);
	allBufferPools_.emplace_back(&meFmb0_);
	allBufferPools_.emplace_back(&meFmb1_);
	allBufferPools_.emplace_back(&meLmi_);

	allBufferPools_.emplace_back(&meMmap0_);
	allBufferPools_.emplace_back(&meMmap1_);
	allBufferPools_.emplace_back(&meMmap2_);
	allBufferPools_.emplace_back(&meMmap3_);

	allBufferPools_.emplace_back(&meConf0_);
	allBufferPools_.emplace_back(&meConf4_);
	allBufferPools_.emplace_back(&meConf5_);

	allBufferPools_.emplace_back(&trawStt_);

	allBufferPools_.emplace_back(&idi_);
	allBufferPools_.emplace_back(&tnrSo_);

	allBufferPools_.emplace_back(&img4oF0_);
	allBufferPools_.emplace_back(&img4oF1_);

	for (unsigned int i = 0; i < 7; i++) {
		allBufferPools_.emplace_back(&wt_[i]);
		allBufferPools_.emplace_back(&img3o_[i]);
		allBufferPools_.emplace_back(&tnrmo_[i]);
		allBufferPools_.emplace_back(&vbi_[i]);
	}

	poolsWritenByCpu_.emplace_back(&fwmeFst_);
	poolsWritenByCpu_.emplace_back(&fwmmFst);
	poolsWritenByCpu_.emplace_back(&fwmmRst_);
	poolsWritenByCpu_.emplace_back(&fwmmMil_);
	poolsWritenByCpu_.emplace_back(&fwmmGyro_);

	// Read/Written by the FWMVP
	poolsWritenByCpu_.emplace_back(&meMmap0_);
	poolsWritenByCpu_.emplace_back(&meMmap1_);
	poolsWritenByCpu_.emplace_back(&meMmap2_);
	poolsWritenByCpu_.emplace_back(&meMmap3_);

	// Need to memset to zero for the first frame
	poolsWritenByCpu_.emplace_back(&meMv0_);
	poolsWritenByCpu_.emplace_back(&meMv1_);

	// Input of FwMM
	poolsWritenByCpu_.emplace_back(&meFst_);
	poolsWritenByCpu_.emplace_back(&meFmb0_);

	// Input of DIP Tuning generation
	poolsWritenByCpu_.emplace_back(&trawStt_);

	// Need to memset to zero for the first frame
	for (unsigned int i = 0; i < 7; i++)
		poolsWritenByCpu_.emplace_back(&wt_[i]);

	// Need to memset to zero for the first frame
	poolsWritenByCpu_.emplace_back(&tnrSo_);
}

int McnrTasksManager::configure(const Size yuvInputSize, const Size videoOut1Size,
				const Size videoOut2Size)
{
	videoOut1Size_ = videoOut1Size;
	videoOut2Size_ = videoOut2Size;
	yuvInputSize_ = yuvInputSize;

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

	meMmapSizes.resize(4);
	meMmapSizes[0] = kMeMapSize0;
	meMmapSizes[1] = kMeMapSize1;
	meMmapSizes[2] = kMeMapSize2;
	meMmapSizes[3] = kMeMapSize3;

	wtSizes.resize(7);
	wtSizes[0] = mcnrSizes[3];
	wtSizes[1] = mcnrSizes[3];
	wtSizes[2] = mcnrSizes[3];
	wtSizes[3] = mcnrSizes[3];
	wtSizes[4] = mcnrSizes[4];
	wtSizes[5] = mcnrSizes[5];
	wtSizes[6] = mcnrSizes[5];

	needCropTNC16x9_ = false;
	if ((videoOut1Size_.width * 9 == videoOut1Size_.height * 16) &&
	    (videoOut2Size_.width * 9 == videoOut2Size_.height * 16))
		needCropTNC16x9_ = true;

	configureBuffers();

	return 0;
}

int McnrTasksManager::configureBuffers()
{
	fwmeFst_.createBuffers(dmaHeap_, formats::Y8_MTISP, kFwMeFstSize, 8);
	fwmmFst.createBuffers(dmaHeap_, formats::Y8_MTISP, kFwMmFstSize, 8);
	fwmmRst_.createBuffers(dmaHeap_, formats::Y8_MTISP, kFwMmRstSize, 8);
	fwmmMil_.createBuffers(dmaHeap_, formats::GREY, kMeL1Size, 8, DmaHeap::System, 64);
	fwmmGyro_.createBuffers(dmaHeap_, formats::Y32_MTISP, Size{ 32, 24 }, 8);

	meFst_.createBuffers(dmaHeap_, formats::Y32_MTISP, kFstSize, 8);
	meFmb0_.createBuffers(dmaHeap_, formats::Y32_MTISP, kFmbSize, 8);
	meFmb1_.createBuffers(dmaHeap_, formats::Y32_MTISP, kFmbSize, 8);
	meLmi_.createBuffers(dmaHeap_, formats::Y16_MTISP, kMeL1Size, 8);
	meIn_.createBuffers(dmaHeap_, formats::GREY, kMeL1Size, 12, DmaHeap::System, 576, 432);
	meMv0_.createBuffers(dmaHeap_, formats::Y32_MTISP, kMeL1Size, 24);
	meMv1_.createBuffers(dmaHeap_, formats::Y32_MTISP, kFmbSize, 24);

	meMmap0_.createBuffers(dmaHeap_, formats::WARP2P_MTISP, meMmapSizes[0], 8, DmaHeap::System, 1168, 217);
	meMmap1_.createBuffers(dmaHeap_, formats::WARP2P_MTISP, meMmapSizes[1], 8, DmaHeap::System, 1168, 217);
	meMmap2_.createBuffers(dmaHeap_, formats::WARP2P_MTISP, meMmapSizes[2], 8, DmaHeap::System, 1168, 217);
	meMmap3_.createBuffers(dmaHeap_, formats::WARP2P_MTISP, meMmapSizes[3], 8, DmaHeap::System, 1168, 217);

	meConf0_.createBuffers(dmaHeap_, formats::GREY, kMeL1Size, 8, DmaHeap::System, 144, 108);
	meConf4_.createBuffers(dmaHeap_, formats::GREY,
			       mcnrSizes[4].boundedTo(kMeL1Size), 12, DmaHeap::System, 144, 108);
	meConf5_.createBuffers(dmaHeap_, formats::GREY,
			       mcnrSizes[5].boundedTo(kMeL1Size), 12, DmaHeap::System, 144, 108);

	idi_.createBuffers(dmaHeap_, formats::NV21, mcnrSizes[6], 21);
	tnrSo_.createBuffers(dmaHeap_, formats::Y32_MTISP, kTnrsoSize, 8);

	img4oF0_.createBuffers(dmaHeap_, formats::NV12_10P_MTISP, mcnrSizes[0], 8);
	img4oF1_.createBuffers(dmaHeap_, formats::NV12_12P_MTISP, mcnrSizes[1], 8);

	trawStt_.createBuffers(dmaHeap_, formats::MTFD_MTISP, kTrawSttSize, 4, DmaHeap::CMA);

	wt_[0].createBuffers(dmaHeap_, formats::GREY, wtSizes[0], 12, DmaHeap::System, 192, 192);
	wt_[1].createBuffers(dmaHeap_, formats::GREY, wtSizes[1], 12, DmaHeap::System, 192, 192);
	wt_[2].createBuffers(dmaHeap_, formats::GREY, wtSizes[2], 12, DmaHeap::System, 192, 192);
	wt_[3].createBuffers(dmaHeap_, formats::GREY, wtSizes[3], 12, DmaHeap::System, 192, 192);

	wt_[4].createBuffers(dmaHeap_, formats::GREY, wtSizes[4], 12, DmaHeap::System, 192, 192);
	wt_[5].createBuffers(dmaHeap_, formats::GREY, wtSizes[5], 12, DmaHeap::System, 192, 192);
	wt_[6].createBuffers(dmaHeap_, formats::GREY, wtSizes[5], 12, DmaHeap::System, 192, 192);

	Size tncSize(mcnrSizes[0]);
	if (needCropTNC16x9_) {
		tncSize.height = tncSize.width * 9 / 16;
	}

	img3o_[0].createBuffers(dmaHeap_, formats::NV12_10P_MTISP, tncSize, 3);
	img3o_[1].createBuffers(dmaHeap_, formats::NV12_12P_MTISP, mcnrSizes[1], 16, DmaHeap::System, 1, 64);
	img3o_[2].createBuffers(dmaHeap_, formats::NV12_12P_MTISP, mcnrSizes[2], 16, DmaHeap::System, 1, 64);
	img3o_[3].createBuffers(dmaHeap_, formats::NV12_12P_MTISP, mcnrSizes[3], 16, DmaHeap::System, 1, 64);
	img3o_[4].createBuffers(dmaHeap_, formats::NV12_12P_MTISP, mcnrSizes[4], 16, DmaHeap::System, 1, 64);
	img3o_[5].createBuffers(dmaHeap_, formats::NV12_12P_MTISP, mcnrSizes[5], 16, DmaHeap::System, 1, 64);
	img3o_[6].createBuffers(dmaHeap_, formats::NV12_12P_MTISP, mcnrSizes[6], 16, DmaHeap::System, 1, 64);

	tnrmo_[1].createBuffers(dmaHeap_, formats::GREY, mcnrSizes[1], 3, DmaHeap::System, 16);
	tnrmo_[2].createBuffers(dmaHeap_, formats::GREY, mcnrSizes[2], 3, DmaHeap::System, 16);
	tnrmo_[3].createBuffers(dmaHeap_, formats::GREY, mcnrSizes[3], 3, DmaHeap::System, 16);
	tnrmo_[4].createBuffers(dmaHeap_, formats::GREY, mcnrSizes[4], 3, DmaHeap::System, 16);
	tnrmo_[5].createBuffers(dmaHeap_, formats::GREY, mcnrSizes[5], 3, DmaHeap::System, 16);
	tnrmo_[6].createBuffers(dmaHeap_, formats::GREY, mcnrSizes[6], 3, DmaHeap::System, 16);

	vbi_[1].createBuffers(dmaHeap_, formats::GREY, mcnrSizes[2], 4, DmaHeap::System, 16);
	vbi_[2].createBuffers(dmaHeap_, formats::GREY, mcnrSizes[2], 4, DmaHeap::System, 16);
	vbi_[3].createBuffers(dmaHeap_, formats::GREY, mcnrSizes[3], 4, DmaHeap::System, 16);
	vbi_[4].createBuffers(dmaHeap_, formats::GREY, mcnrSizes[4], 4, DmaHeap::System, 16);
	vbi_[5].createBuffers(dmaHeap_, formats::GREY, mcnrSizes[5], 4, DmaHeap::System, 16);
	vbi_[6].createBuffers(dmaHeap_, formats::GREY, mcnrSizes[6], 4, DmaHeap::System, 16);

	for (auto &pool : poolsWritenByCpu_)
		pool->mmap();

	return 0;
}

int McnrTasksManager::start()
{
#if !V4L2_STANDARD_MODE
	for (auto &pool : allBufferPools_)
		imgSys_->handleIova(ImgSysDevice::Add, *pool);
#endif
	return 0;
}

int McnrTasksManager::stop()
{
#if !V4L2_STANDARD_MODE
	for (auto &pool : allBufferPools_)
		imgSys_->handleIova(ImgSysDevice::Delete, *pool);
#endif
	return 0;
}

int McnrTasksManager::releaseBuffers()
{
	for (auto &pool : allBufferPools_)
		pool->release();

	return 0;
}

void McnrTasksManager::makeMCNRFrames(MCNRFrames &mcnr,
				      MCNRPrevOutput &prev,
				      SharedMailBox<InfoFrame> &meL0,
				      SharedMailBox<InfoFrame> &p1F0,
				      SharedMailBox<InfoFrame> &p1F1,
				      FrameBuffer *videoOut1,
				      FrameBuffer *videoOut2)
{
	/* Prepare mailboxes shared among MCNR tasks */
	mcnr.videoOut1 = videoOut1;
	mcnr.videoOut2 = videoOut2;

	/* todo: The tuning frames should be created by IPA */
	TuningFrames tunings;
	tunings.trMeTun = makeMailBox<InfoFrame>();
	tunings.meATun = makeMailBox<InfoFrame>();
	tunings.meBTun = makeMailBox<InfoFrame>();
	tunings.trTunF1 = makeMailBox<InfoFrame>();
	tunings.trTunF4 = makeMailBox<InfoFrame>();
	tunings.ltrTunF1 = makeMailBox<InfoFrame>();
	tunings.ltrTunF4 = makeMailBox<InfoFrame>();
	tunings.meMil = makeMailBox<InfoFrame>();
	tunings.ltrTunVbi = makeMailBox<InfoFrame>();
	tunings.wpeTun = makeMailBox<InfoFrame>();
	tunings.dipTun = makeMailBoxVector<InfoFrame>(7);

	/* Outputs of ME, motion estimation confidence */
	SharedMailBox<InfoFrame> meConf0 = makeMailBox<InfoFrame>();

	/* Output of TR, downscaled from meConf0 */
	SharedMailBox<InfoFrame> meConf4 = makeMailBox<InfoFrame>();
	SharedMailBox<InfoFrame> meConf5 = makeMailBox<InfoFrame>();

	/* meMmap[0] is an output of ME and it's downscaled
	 * meMmap[1~3] are downscaled from meMmap[0] in TR.
	 * DIP uses them as wrap maps to align the previous frame for TNR */
	std::vector<SharedMailBox<InfoFrame>> meMmap = makeMailBoxVector<InfoFrame>(4);

	/* Outputs of stage TR, Downscaled from P1F0 and P1F1.
	 * The smaller index has larger size and downscaled to half from the
	 * previous level, where dipImgi[0] = P1F0 and dipImgi[1] = P1F1.
	 * Inputs of DIP1 and DIP2 for each level's TNR */
	std::vector<SharedMailBox<InfoFrame>> dipImgi = makeMailBoxVector<InfoFrame>(7);
	dipImgi[0] = p1F0;
	dipImgi[1] = p1F1;

	/* Downscaled from prevImgoF0 and prevImgoF1 DIP1, will be aligned to the
	 * same level of dipImgi. Used in DIP1 and DIP2 */
	std::vector<SharedMailBox<InfoFrame>> dipVipi = makeMailBoxVector<InfoFrame>(7);
	dipVipi[0] = prev.prevImg4oF0;

	std::vector<SharedMailBox<InfoFrame>> dipVbi = makeMailBoxVector<InfoFrame>(7);
	dipVbi[0] = dipVbi[1] = dipVbi[2];

	/* Intermediate frames shared by DIP1 and DIP2 */
	SharedMailBox<InfoFrame> tnrlfdi = makeMailBox<InfoFrame>();
	SharedMailBox<InfoFrame> dipTnrso = makeMailBox<InfoFrame>();

	std::vector<SharedMailBox<InfoFrame>> dipTnrwi = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> dipTnrwo = makeMailBoxVector<InfoFrame>(7);

	/* Input of DIP1 and DIP2 tnrci, renamed from meconf for easier mapping
	 * to DIP levels, where dipTnrci[0~3] = meConf0, dipTnrci[4] = meConf4
	 * and dipTnrci[5~6] = meConf5. */
	std::vector<SharedMailBox<InfoFrame>> dipTnrci;
	dipTnrci.resize(7);
	dipTnrci[0] = dipTnrci[1] = meConf0;
	dipTnrci[2] = dipTnrci[3] = meConf0;
	if (mcnrSizes[4] > kMeL1Size)
		dipTnrci[4] = meConf0;
	else
		dipTnrci[4] = meConf4;

	if (mcnrSizes[5] > kMeL1Size)
		dipTnrci[5] = dipTnrci[6] = meConf0;
	else
		dipTnrci[5] = dipTnrci[6] = meConf5;

	/* Inputs/outptus of DIP, propagate from low to high levels. Link
	 * tnrmi[i] to the previous level tnrmo[i+1] for easier use. */
	std::vector<SharedMailBox<InfoFrame>> dipTnrmo = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> dipTnrmi;
	dipTnrmi.resize(7);
	for (int i = 0; i < 6; i++)
		dipTnrmi[i] = dipTnrmo[i + 1];

	/* Intermediate frames used by DIP, propagate from low to high levels.
	 * Link reci[i] to the previous level img3o[i+1] for easier use. */
	std::vector<SharedMailBox<InfoFrame>> img3o = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> reci;
	reci.resize(7);
	for (int i = 0; i < 6; i++)
		reci[i] = img3o[i + 1];

	/* Rename of meMmap to 7 levels for easier assign as DIP inputs,
	 * linked as the largest size which is smaller than the corresponding
	 * level size. */
	std::vector<SharedMailBox<InfoFrame>> wpeVeci;
	wpeVeci.resize(7);
	for (unsigned int i = 0; i < 7; i++) {
		for (unsigned int j = 0; j < 4; j++) {
			if (wtSizes[i] > meMmapSizes[j]) {
				wpeVeci[i] = meMmap[j];
				break;
			}
		}
	}

	/* Outputs down scaled from ME frame by stage HW_LTR_ME_L1*/
	SharedMailBox<InfoFrame> meL1 = makeMailBox<InfoFrame>();

	if (!prev.valid) {
		prev.prevMeL0 = meL0;
		prev.prevMeL1 = meL1;
		prev.prevImg4oF0 = p1F0;
		prev.prevImg4oF1 = p1F1;

		prev.prevFwMeFst = makeMailBox<InfoFrame>();
		fwmeFst_.fetch(prev.prevFwMeFst);
		zeroImage(prev.prevFwMeFst);

		prev.prevFwMmFst = makeMailBox<InfoFrame>();
		fwmmFst.fetch(prev.prevFwMmFst);
		zeroImage(prev.prevFwMmFst);

		prev.prevMeAFst = makeMailBox<InfoFrame>();
		meFst_.fetch(prev.prevMeAFst);
		zeroImage(prev.prevMeAFst);

		prev.prevMeBFst = makeMailBox<InfoFrame>();
		meFst_.fetch(prev.prevMeBFst);
		zeroImage(prev.prevMeBFst);

		prev.prevPrevMeAFst = makeMailBox<InfoFrame>();
		meFst_.fetch(prev.prevPrevMeAFst);
		zeroImage(prev.prevPrevMeAFst);

		prev.prevPrevMeBFst = makeMailBox<InfoFrame>();
		meFst_.fetch(prev.prevPrevMeBFst);
		zeroImage(prev.prevPrevMeBFst);

		prev.prevMeAMv1 = makeMailBox<InfoFrame>();
		meMv1_.fetch(prev.prevMeAMv1);
		zeroImage(prev.prevMeAMv1);

		prev.prevMeBMv0 = makeMailBox<InfoFrame>();
		meMv0_.fetch(prev.prevMeBMv0);
		zeroImage(prev.prevMeBMv0);

		prev.preDipTnrso = makeMailBox<InfoFrame>();
		tnrSo_.fetch(prev.preDipTnrso);
		zeroImage(prev.preDipTnrso);

		prev.prevDipTnrwo.resize(7);
		for (size_t i = 0; i < prev.prevDipTnrwo.size(); i++) {
			prev.prevDipTnrwo[i] = makeMailBox<InfoFrame>();
			wt_[i].fetch(prev.prevDipTnrwo[i]);
			zeroImage(prev.prevDipTnrwo[i]);
		}
	}

	/* Frames for ME task */
	MeFrames &meFrames = mcnr.meFrames;

	meFrames.in.trMeTun = tunings.trMeTun;
	meFrames.in.meATun = tunings.meATun;
	meFrames.in.meBTun = tunings.meBTun;
	meFrames.in.meMil = tunings.meMil;

	meFrames.in.prevFwMeFst = prev.prevFwMeFst;
	meFrames.in.prevFwMmFst = prev.prevFwMmFst;
	meFrames.in.prevMeAFst = prev.prevMeAFst;
	meFrames.in.prevMeBFst = prev.prevMeBFst;

	meFrames.in.prevPrevMeAFst = prev.prevPrevMeAFst;
	meFrames.in.prevPrevMeBFst = prev.prevPrevMeBFst;

	meFrames.in.prevMeAMv1 = prev.prevMeAMv1;
	meFrames.in.prevMeBMv0 = prev.prevMeBMv0;
	meFrames.in.prevMeL0 = prev.prevMeL0;
	meFrames.in.prevMeL1 = prev.prevMeL1;
	meFrames.in.meL0 = meL0;

	meFrames.in.fwMeFst = makeMailBox<InfoFrame>();
	meFrames.in.fwMmFst = makeMailBox<InfoFrame>();
	meFrames.in.fwMmRst = makeMailBox<InfoFrame>();

	meFrames.out.meAMv0 = makeMailBox<InfoFrame>();
	meFrames.out.meAMv1 = makeMailBox<InfoFrame>();
	meFrames.out.meAFmb0 = makeMailBox<InfoFrame>();
	meFrames.out.meAFmb1 = makeMailBox<InfoFrame>();
	meFrames.out.meALmi = makeMailBox<InfoFrame>();
	meFrames.out.meAFst = makeMailBox<InfoFrame>();

	/* Hardware ME requires the MV and Fmb buffer to be reused between
	 * MeA and MeB stages */
	meFrames.out.meBMv0 = meFrames.out.meAMv0;
	meFrames.out.meBMv1 = meFrames.out.meAMv1;
	meFrames.out.meBFmb0 = meFrames.out.meAFmb0;
	meFrames.out.meBFmb1 = meFrames.out.meAFmb1;

	meFrames.out.meBFst = makeMailBox<InfoFrame>();
	meFrames.out.meBLmi = makeMailBox<InfoFrame>();
	meFrames.out.meL1 = meL1;
	meFrames.out.meMmap = meMmap;
	meFrames.out.meConf0 = meConf0;

	/* Frames for TR task */
	TrFrames &trFrames = mcnr.trFrames;

	trFrames.in.p1F1 = p1F1;
	trFrames.in.trTunF1 = tunings.trTunF1;
	trFrames.in.trTunF4 = tunings.trTunF4;
	trFrames.in.meConf0 = meConf0;
	trFrames.in.meMmap = meMmap;

	trFrames.out.trawStt = makeMailBox<InfoFrame>();
	trFrames.out.dipImgi = dipImgi;
	trFrames.out.meConf4 = meConf4;
	trFrames.out.meConf5 = meConf5;

	/* Frames for DIP1 task */
	Dip1Frames &dip1Frames = mcnr.dip1Frames;

	dip1Frames.in.ltrTunF1 = tunings.ltrTunF1;
	dip1Frames.in.ltrTunF4 = tunings.ltrTunF4;
	dip1Frames.in.ltrTunVbi = tunings.ltrTunVbi;
	dip1Frames.in.wpeTun = tunings.wpeTun;
	dip1Frames.in.dipTun = tunings.dipTun;
	dip1Frames.in.preDipTnrso = prev.preDipTnrso;
	dip1Frames.in.prevImg4oF0 = prev.prevImg4oF0;
	dip1Frames.in.prevImg4oF1 = prev.prevImg4oF1;
	dip1Frames.in.prevDipTnrwo = prev.prevDipTnrwo;

	dip1Frames.out.tnrlfdi = tnrlfdi;
	dip1Frames.out.dipTnrso = dipTnrso;
	dip1Frames.out.reci = reci;
	dip1Frames.out.img3o = img3o;
	dip1Frames.out.dipTnrwi = dipTnrwi;
	dip1Frames.out.dipTnrwo = dipTnrwo;
	dip1Frames.out.dipTnrmi = dipTnrmi;
	dip1Frames.out.dipTnrmo = dipTnrmo;
	dip1Frames.out.wpeVeci = wpeVeci;
	dip1Frames.out.dipVbi = dipVbi;
	dip1Frames.out.dipVipi = dipVipi;
	dip1Frames.out.dipImgi = dipImgi;
	dip1Frames.out.dipTnrci = dipTnrci;
	dip1Frames.out.meMmap = meMmap;
	dip1Frames.out.img4oF1 = makeMailBox<InfoFrame>();
	dip1Frames.out.swHist = makeMailBox<InfoFrame>();

	/* Frames for DIP2 task */
	Dip2Frames &dip2Frames = mcnr.dip2Frames;

	dip2Frames.in.dipTun = tunings.dipTun;
	dip2Frames.in.prevImg4oF0 = prev.prevImg4oF0;
	dip2Frames.in.tnrlfdi = tnrlfdi;
	dip2Frames.in.reci = reci;
	dip2Frames.in.dipTnrwi = dipTnrwi;
	dip2Frames.in.dipTnrmi = dipTnrmi;
	dip2Frames.in.meMmap = meMmap;
	dip2Frames.in.dipImgi = dipImgi;
	dip2Frames.in.dipTnrci = dipTnrci;

	dip2Frames.out.img3o = img3o;
	dip2Frames.out.img4oF0 = makeMailBox<InfoFrame>();
	dip2Frames.out.dipTnrso = dipTnrso;
	dip2Frames.out.dipTnrwo = dipTnrwo;

	/* Update prev */
	prev.prevFwMeFst = meFrames.in.fwMeFst;
	prev.prevFwMmFst = meFrames.in.fwMmFst;
	prev.prevPrevMeAFst = prev.prevMeAFst;
	prev.prevPrevMeBFst = prev.prevMeBFst;
	prev.prevMeAFst = meFrames.out.meAFst;
	prev.prevMeBFst = meFrames.out.meBFst;
	prev.prevMeBMv0 = meFrames.out.meBMv0;
	prev.prevMeAMv1 = meFrames.out.meAMv1;
	prev.preDipTnrso = dip1Frames.out.dipTnrso;
	prev.prevDipTnrwo = dip1Frames.out.dipTnrwo;
	prev.prevMeL0 = meL0;
	prev.prevMeL1 = meFrames.out.meL1;
	prev.prevImg4oF1 = dip1Frames.out.img4oF1;
	prev.prevImg4oF0 = dip2Frames.out.img4oF0;
	prev.valid = true;
}

std::tuple<MeATask *, MeBTask *, TrTask *, Dip1Task *, Dip2Task *>
McnrTasksManager::makeMcnrTasks(MCNRFrames &mcnr, Scheduler *scheduler,
				const std::string &id, Request *request,
				uint32_t internalRequestId, ImgSysDevice *imgSys)
{
	(void)id;
	std::string sequence = std::to_string(request->sequence());

	MeATask *meATask = new MeATask(
		scheduler, "MeA " + sequence, request,
		internalRequestId, imgSys, mcnr, this);
	MeBTask *meBTask = new MeBTask(
		scheduler, "MeB " + sequence, request,
		internalRequestId, imgSys, mcnr, this);
	TrTask *trTask = new TrTask(
		scheduler, "Tr " + sequence, request,
		internalRequestId, imgSys, mcnr, this);
	Dip1Task *dip1Task = new Dip1Task(
		scheduler, "Dip 1 " + sequence, request,
		internalRequestId, imgSys, mcnr, this);

	Dip2Task *dip2Task = new Dip2Task(
		scheduler, "Dip 2 " + sequence, request,
		internalRequestId, imgSys, mcnr, this);

	return std::make_tuple(meATask, meBTask, trTask, dip1Task, dip2Task);
}

MeATask::MeATask(Scheduler *scheduler, const std::string &id,
		 Request *request, uint32_t internalRequestId,
		 ImgSysDevice *imgSys, MCNRFrames &mcnr, McnrTasksManager *manager)
	: Task(scheduler, id), requestHelper_(this, request, imgSys),
	  request_(request), internalRequestId_(internalRequestId), manager_(manager), imgSys_(imgSys)
{
	/* Collect MailBoxes used for the task */
	frames_ = mcnr.meFrames;
	syncLtrMeA_ = 0;
}

void MeATask::allocateOutputBuffers()
{
	auto &out = frames_.out;

	manager_->meIn_.fetch(out.meL1);
	manager_->meMv0_.fetch(out.meAMv0);
	manager_->meMv1_.fetch(out.meAMv1);
	manager_->meFst_.fetch(out.meAFst);
	manager_->meLmi_.fetch(out.meALmi);
	manager_->meFmb0_.fetch(out.meAFmb0);
	manager_->meFmb1_.fetch(out.meAFmb1);

	manager_->meConf0_.fetch(out.meConf0);

	syncLtrMeA_ = imgSys_->syncPool().get();
}

void MeATask::notifyDone()
{
	if (syncLtrMeA_)
		imgSys_->syncPool().put(syncLtrMeA_);

	manager_->onDeviceTuner_->tuneMeA(internalRequestId_, frames_);
	Task::notifyDone();
}

void MeATask::run()
{
	allocateOutputBuffers();

	MUINT32 timestampMili = request_->metadata().get(controls::SensorTimestamp).value_or(0);
	SingleDeviceRequest sdRequest;

	sdRequest.init(internalRequestId_, timestampMili, "MeATask");

	auto &in = frames_.in;
	auto &out = frames_.out;

	/* HW_LTR_ME_L1 */
	StageEx &HW_LTR_ME_L1 = sdRequest.emplaceStage(PEU_Stage::HW_LTR_ME_L1);

	HW_LTR_ME_L1.input(in.trMeTun->get(), NSCam::NSImgStream::IMG_PORT_METAI, 0, Size{ 0, 0 });
	HW_LTR_ME_L1.input(in.meL0->get(), NSCam::NSImgStream::IMG_PORT_LTIMGI, 0, Size{ 0, 0 });
	HW_LTR_ME_L1.output(out.meL1->get(), NSCam::NSImgStream::IMG_PORT_LTYUV2O, 1, kMeL0Size);
	HW_LTR_ME_L1.setAplInfo();

	/* Set notify fence from HW_TR_ME_L1 to HW_ME_3PASS_MODE_0 */
	HW_LTR_ME_L1.addNotify(syncLtrMeA_);

	/* HW_ME_3PASS_MODE_0 */
	StageEx &HW_ME_3PASS_MODE_0 = sdRequest.emplaceStage(PEU_Stage::HW_ME_3PASS_MODE_0);

	HW_ME_3PASS_MODE_0.input(in.meATun->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_0.input(in.prevMeL0->get(), IMG_PORT_ME_L0_IMG0I, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_0.input(in.meL0->get(), IMG_PORT_ME_L0_IMG1I, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_0.input(in.prevMeL1->get(), IMG_PORT_ME_L1_IMG0I, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_0.input(out.meL1->get(), IMG_PORT_ME_L1_IMG1I, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_0.input(in.prevMeAMv1->get(), IMG_PORT_ME_L1_RMVI, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_0.input(in.prevMeBMv0->get(), IMG_PORT_ME_L0_RMVI, 0, Size{ 0, 0 });

	HW_ME_3PASS_MODE_0.output(out.meAFst->get(), IMG_PORT_ME_FSTO, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_0.output(out.meAFmb0->get(), IMG_PORT_ME_L0_FMBO, 0, kFmbSize);
	HW_ME_3PASS_MODE_0.output(out.meAFmb1->get(), IMG_PORT_ME_L1_FMBO, 0, kFmbSize);
	HW_ME_3PASS_MODE_0.output(out.meALmi->get(), IMG_PORT_ME_LMIO, 0, kMeL1Size);
	HW_ME_3PASS_MODE_0.output(out.meConf0->get(), IMG_PORT_ME_CONFO, 0, kMeL1Size);
	HW_ME_3PASS_MODE_0.output(out.meAMv0->get(), IMG_PORT_ME_L0_WMVO, 0, kMeL1Size);
	HW_ME_3PASS_MODE_0.output(out.meAMv1->get(), IMG_PORT_ME_L1_WMVO, 0, kFmbSize);

	HW_ME_3PASS_MODE_0.setMeInfo(NSCam::NSImgStream::EME_MODE_0);

	/* Set wait fence for HW_ME_3PASS_MODE_0 from HW_TR_ME_L1 */
	HW_ME_3PASS_MODE_0.addWait(syncLtrMeA_);

	requestHelper_.queueRequest(ImgSysDevice::kUserIdMcnr, sdRequest);
}

MeBTask::MeBTask(Scheduler *scheduler, const std::string &id,
		 Request *request, uint32_t internalRequestId,
		 ImgSysDevice *imgSys, MCNRFrames &mcnr, McnrTasksManager *manager)
	: Task(scheduler, id), requestHelper_(this, request, imgSys),
	  request_(request), internalRequestId_(internalRequestId), manager_(manager), imgSys_(imgSys)
{
	/* Collect MailBoxes used for the task */
	frames_.in.prevMeL0 = mcnr.meFrames.in.prevMeL0;
	frames_.in.prevMeL1 = mcnr.meFrames.in.prevMeL1;
	frames_.in.meL0 = mcnr.meFrames.in.meL0;
	frames_.in.meBTun = mcnr.meFrames.in.meBTun;
	frames_.in.meMil = mcnr.meFrames.in.meMil;

	frames_.out.meL1 = mcnr.meFrames.out.meL1;
	frames_.out.meAMv0 = mcnr.meFrames.out.meAMv0;
	frames_.out.meAFmb0 = mcnr.meFrames.out.meAFmb0;
	frames_.out.meAFmb1 = mcnr.meFrames.out.meAFmb1;

	frames_.out.meBMv0 = mcnr.meFrames.out.meBMv0;
	frames_.out.meBMv1 = mcnr.meFrames.out.meBMv1;
	frames_.out.meBFst = mcnr.meFrames.out.meBFst;
	frames_.out.meBFmb0 = mcnr.meFrames.out.meBFmb0;
	frames_.out.meBFmb1 = mcnr.meFrames.out.meBFmb1;
	frames_.out.meBLmi = mcnr.meFrames.out.meBLmi;
	frames_.out.meConf0 = mcnr.meFrames.out.meConf0;
	frames_.out.meMmap = mcnr.meFrames.out.meMmap;
}

void MeBTask::allocateOutputBuffers()
{
	manager_->meFst_.fetch(frames_.out.meBFst);
	manager_->meLmi_.fetch(frames_.out.meBLmi);

	manager_->meMmap0_.fetch(frames_.out.meMmap[0]);
}

void MeBTask::notifyDone()
{
	manager_->onDeviceTuner_->tuneMeB(internalRequestId_, frames_);
	Task::notifyDone();
}

void MeBTask::run()
{
	allocateOutputBuffers();

	MUINT32 timestampMili = request_->metadata().get(controls::SensorTimestamp).value_or(0);
	SingleDeviceRequest sdRequest;

	sdRequest.init(internalRequestId_, timestampMili, "MeBTask");

	auto &in = frames_.in;
	auto &out = frames_.out;

	/* HW_ME_3PASS_MODE_1 */
	StageEx &HW_ME_3PASS_MODE_1 = sdRequest.emplaceStage(PEU_Stage::HW_ME_3PASS_MODE_1);

	HW_ME_3PASS_MODE_1.input(in.meBTun->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_1.input(in.prevMeL0->get(), IMG_PORT_ME_L0_IMG0I, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_1.input(in.meL0->get(), IMG_PORT_ME_L0_IMG1I, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_1.input(in.prevMeL1->get(), IMG_PORT_ME_L1_IMG0I, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_1.input(out.meL1->get(), IMG_PORT_ME_L1_IMG1I, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_1.input(out.meAFmb0->get(), IMG_PORT_ME_L0_FMBI, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_1.input(out.meAFmb1->get(), IMG_PORT_ME_L1_FMBI, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_1.input(out.meAMv0->get(), IMG_PORT_ME_L0_RMVI, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_1.input(in.meMil->get(), IMG_PORT_ME_MEMILI, 0, Size{ 0, 0 });

	HW_ME_3PASS_MODE_1.output(out.meConf0->get(), IMG_PORT_ME_CONFO, 0, kMeL1Size);
	HW_ME_3PASS_MODE_1.output(out.meMmap[0]->get(), IMG_PORT_ME_WMAPO, 0, kMeMapSize0);
	HW_ME_3PASS_MODE_1.output(out.meBFst->get(), IMG_PORT_ME_FSTO, 0, Size{ 0, 0 });
	HW_ME_3PASS_MODE_1.output(out.meBFmb0->get(), IMG_PORT_ME_L0_FMBO, 0, kFmbSize);
	HW_ME_3PASS_MODE_1.output(out.meBFmb1->get(), IMG_PORT_ME_L1_FMBO, 0, kFmbSize);
	HW_ME_3PASS_MODE_1.output(out.meBLmi->get(), IMG_PORT_ME_LMIO, 0, kMeL1Size);
	HW_ME_3PASS_MODE_1.output(out.meBMv0->get(), IMG_PORT_ME_L0_WMVO, 0, kMeL1Size);
	HW_ME_3PASS_MODE_1.output(out.meBMv1->get(), IMG_PORT_ME_L1_WMVO, 0, kFmbSize);

	HW_ME_3PASS_MODE_1.setMeInfo(NSCam::NSImgStream::EME_MODE_1);

	requestHelper_.queueRequest(ImgSysDevice::kUserIdMcnr, sdRequest);
}

TrTask::TrTask(Scheduler *scheduler, const std::string &id,
	       Request *request, uint32_t internalRequestId,
	       ImgSysDevice *imgSys, MCNRFrames &mcnr, McnrTasksManager *manager)
	: Task(scheduler, id), requestHelper_(this, request, imgSys),
	  request_(request), internalRequestId_(internalRequestId), manager_(manager), imgSys_(imgSys)
{
	(void)imgSys_;
	frames_ = mcnr.trFrames;
}

void TrTask::allocateOutputBuffers()
{
	auto &out = frames_.out;

	manager_->meMmap1_.fetch(frames_.in.meMmap[1]);
	manager_->meMmap2_.fetch(frames_.in.meMmap[2]);
	manager_->meMmap3_.fetch(frames_.in.meMmap[3]);

	/* dipImgi[0] (p1F0) and dipImgi[1] (p1F1) are from P1 */
	manager_->img3o_[2].fetch(out.dipImgi[2]);
	manager_->img3o_[3].fetch(out.dipImgi[3]);
	manager_->img3o_[4].fetch(out.dipImgi[4]);
	manager_->img3o_[5].fetch(out.dipImgi[5]);
	manager_->img3o_[6].fetch(out.dipImgi[6]);

	manager_->meConf4_.fetch(out.meConf4);
	manager_->meConf5_.fetch(out.meConf5);

	/* Statstistic */
	manager_->trawStt_.fetch(out.trawStt);
}

void TrTask::notifyDone()
{
	manager_->onDeviceTuner_->tuneTr(internalRequestId_, frames_);
	Task::notifyDone();
}

void TrTask::run()
{
	allocateOutputBuffers();

	auto &mcnrSizes = manager_->mcnrSizes;

	MUINT32 timestampMili = request_->metadata().get(controls::SensorTimestamp).value_or(0);
	SingleDeviceRequest sdRequest;

	sdRequest.init(internalRequestId_, timestampMili, "TRTask");

	auto &in = frames_.in;
	auto &out = frames_.out;

	/* HW_TR_F1 */
	StageEx &HW_TR_F1 = sdRequest.emplaceStage(PEU_Stage::HW_TR_F1);

	HW_TR_F1.input(in.trTunF1->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	HW_TR_F1.input(in.p1F1->get(), IMG_PORT_TIMGI, 0, Size{ 0, 0 });

	HW_TR_F1.output(out.dipImgi[2]->get(), IMG_PORT_TYUV2O, 2, mcnrSizes[1]);
	HW_TR_F1.output(out.dipImgi[3]->get(), IMG_PORT_TYUV3O, 2, mcnrSizes[2]);
	HW_TR_F1.output(out.dipImgi[4]->get(), IMG_PORT_TYUV4O, 2, mcnrSizes[3]);
	HW_TR_F1.output(out.trawStt->get(), IMG_PORT_IMGSTATO, 0, Size{ 0, 0 });

	/* HW_TR_F4 */
	StageEx &HW_TR_F4 = sdRequest.emplaceStage(PEU_Stage::HW_TR_F4);

	HW_TR_F4.input(in.trTunF4->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	HW_TR_F4.input(out.dipImgi[4]->get(), IMG_PORT_TIMGI, 0, Size{ 0, 0 });

	HW_TR_F4.output(out.dipImgi[5]->get(), IMG_PORT_TYUV2O, 2, mcnrSizes[4]);
	HW_TR_F4.output(out.dipImgi[6]->get(), IMG_PORT_TYUV3O, 2, mcnrSizes[5]);

	/* HW_TR_HWMVP */
	StageEx &HW_TR_HWMVP = sdRequest.emplaceStage(PEU_Stage::HW_TR_HWMVP);

	HW_TR_HWMVP.input(in.meMmap[0]->get(), IMG_PORT_TIMGI, 0, Size{ 0, 0 });

	HW_TR_HWMVP.output(in.meMmap[1]->get(), IMG_PORT_TYUV2O, 0, kMeMapSize1);
	HW_TR_HWMVP.output(in.meMmap[2]->get(), IMG_PORT_TYUV3O, 0, kMeMapSize2);
	HW_TR_HWMVP.output(in.meMmap[3]->get(), IMG_PORT_TYUV4O, 0, kMeMapSize3);

	/* HW_TR_CONF4 */
	StageEx &HW_TR_CONF4 = sdRequest.emplaceStage(PEU_Stage::HW_TR_CONF4);

	HW_TR_CONF4.input(in.meConf0->get(), IMG_PORT_TIMGI, 0, Size{ 0, 0 });
	HW_TR_CONF4.output(out.meConf4->get(), IMG_PORT_TYUV5O, 0, kMeL1Size);

	HW_TR_CONF4.setMvFrame(mcnrSizes[0], kMeL0Size);

	/* HW_TR_CONF5 */
	StageEx &HW_TR_CONF5 = sdRequest.emplaceStage(PEU_Stage::HW_TR_CONF5);

	HW_TR_CONF5.input(in.meConf0->get(), IMG_PORT_TIMGI, 0, Size{ 0, 0 });
	HW_TR_CONF5.output(out.meConf5->get(), IMG_PORT_TYUV5O, 0, kMeL1Size);

	HW_TR_CONF5.setMvFrame(mcnrSizes[0], kMeL0Size);

	requestHelper_.queueRequest(ImgSysDevice::kUserIdMcnr, sdRequest);
}

Dip1Task::Dip1Task(Scheduler *scheduler, const std::string &id,
		   Request *request, uint32_t internalRequestId,
		   ImgSysDevice *imgSys, MCNRFrames &mcnr, McnrTasksManager *manager)
	: Task(scheduler, id), requestHelper_(this, request, imgSys),
	  request_(request), internalRequestId_(internalRequestId), manager_(manager), imgSys_(imgSys)
{
	frames_ = mcnr.dip1Frames;

	syncLtrDip_ = 0;
	syncWpeDip_ = 0;
}

void Dip1Task::allocateOutputBuffers()
{
	auto &out = frames_.out;

	/* dipVipi[0] is not used. */
	for (int i = 1; i < 7; i++)
		manager_->img3o_[i].fetch(out.dipVipi[i]);

	/* dipVbi[0] is not used, dipVbi[1] = dipVbi[2]. */
	for (int i = 2; i < 7; i++)
		manager_->vbi_[i].fetch(out.dipVbi[i]);

	/* dipTnrwi[0] uses manager_->wt0_, dipTnrwi[6] is not used */
	for (int i = 0; i < 6; i++)
		manager_->wt_[i].fetch(out.dipTnrwi[i]);

	/* dipTnrwo[6] is not used */
	for (int i = 0; i < 6; i++)
		manager_->wt_[i].fetch(out.dipTnrwo[i]);

	manager_->tnrSo_.fetch(out.dipTnrso);
	manager_->idi_.fetch(out.tnrlfdi);

	/* allocate tnrmo[] only since tnrmi is linked to it, tnrmo[0] is not used */
	for (int i = 1; i < 7; i++)
		manager_->tnrmo_[i].fetch(out.dipTnrmo[i]);

	/* allocate img3o[] only since reci is linked to them */
	for (int i = 0; i < 7; i++)
		manager_->img3o_[i].fetch(out.img3o[i]);

	manager_->img4oF1_.fetch(out.img4oF1);

	syncLtrDip_ = imgSys_->syncPool().get();
	syncWpeDip_ = imgSys_->syncPool().get();
}

void Dip1Task::notifyDone()
{
	if (syncLtrDip_)
		imgSys_->syncPool().put(syncLtrDip_);

	if (syncWpeDip_)
		imgSys_->syncPool().put(syncWpeDip_);

	manager_->onDeviceTuner_->tuneDip1(internalRequestId_, frames_);

	Task::notifyDone();
}

void Dip1Task::run()
{
	allocateOutputBuffers();

	auto &mcnrSizes = manager_->mcnrSizes;

	MUINT32 timestampMili = request_->metadata().get(controls::SensorTimestamp).value_or(0);
	SingleDeviceRequest sdRequest;

	sdRequest.init(internalRequestId_, timestampMili, "DIPTask");

	auto &in = frames_.in;
	auto &out = frames_.out;

	/* HW_LTR_F1 */
	StageEx &HW_LTR_F1 = sdRequest.emplaceStage(PEU_Stage::HW_LTR_F1);

	HW_LTR_F1.input(in.ltrTunF1->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	HW_LTR_F1.input(in.prevImg4oF1->get(), IMG_PORT_WPE_WPEI, 0, Size{ 0, 0 });
	HW_LTR_F1.input(out.meMmap[0]->get(), IMG_PORT_WPE_VECI, 0, Size{ 0, 0 });

	HW_LTR_F1.output(out.dipVipi[2]->get(), IMG_PORT_LTYUV2O, 2, mcnrSizes[1]);
	HW_LTR_F1.output(out.dipVipi[3]->get(), IMG_PORT_LTYUV3O, 2, mcnrSizes[2]);
	HW_LTR_F1.output(out.dipVipi[4]->get(), IMG_PORT_LTYUV4O, 2, mcnrSizes[3]);
	HW_LTR_F1.output(out.dipVbi[2]->get(), IMG_PORT_LTYUV5O, 2, mcnrSizes[2]);
	HW_LTR_F1.output(out.dipVipi[1]->get(), IMG_PORT_WPE_WPEO, 0, mcnrSizes[1]);

	HW_LTR_F1.setWpeInfo(IMG_EXTRA_PARAM_ID_WPE_INFO,
			     mcnrSizes[1], NSCam::NSImgStream::EWPE_HW_LITE,
			     (unsigned int)NSCam::NSImgStream::EWPE_MVMAP);

	HW_LTR_F1.setMvFrame(mcnrSizes[0], kMeL0Size);

	/* HW_LTR_F4 */
	StageEx &HW_LTR_F4 = sdRequest.emplaceStage(PEU_Stage::HW_LTR_F4);

	HW_LTR_F4.input(in.ltrTunF4->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	HW_LTR_F4.input(out.dipVipi[4]->get(), IMG_PORT_LTIMGI, 0, Size{ 0, 0 });

	HW_LTR_F4.output(out.dipVipi[5]->get(), IMG_PORT_LTYUV2O, 2, mcnrSizes[4]);
	HW_LTR_F4.output(out.dipVipi[6]->get(), IMG_PORT_LTYUV3O, 2, mcnrSizes[5]);

	/* HW_LTR_VBI */
	StageEx &HW_LTR_VBI = sdRequest.emplaceStage(PEU_Stage::HW_LTR_VBI);

	HW_LTR_VBI.input(in.ltrTunVbi->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	HW_LTR_VBI.input(out.dipVbi[2]->get(), IMG_PORT_LTIMGI, 0, Size{ 0, 0 });

	HW_LTR_VBI.output(out.dipVbi[3]->get(), IMG_PORT_LTYUV2O, 2, mcnrSizes[2]);
	HW_LTR_VBI.output(out.dipVbi[4]->get(), IMG_PORT_LTYUV3O, 2, mcnrSizes[3]);
	HW_LTR_VBI.output(out.dipVbi[5]->get(), IMG_PORT_LTYUV4O, 2, mcnrSizes[4]);

	HW_LTR_VBI.addNotify(syncLtrDip_);

	/* HW_WPE_W_F1 */
	StageEx &HW_WPE_W_F1 = sdRequest.emplaceStage(PEU_Stage::HW_WPE_W_F1);
	setWpeParams(HW_WPE_W_F1, 1);

	/* HW_WPE_W_F2 */
	StageEx &HW_WPE_W_F2 = sdRequest.emplaceStage(PEU_Stage::HW_WPE_W_F2);
	setWpeParams(HW_WPE_W_F2, 2);

	/* HW_WPE_W_F3 */
	StageEx &HW_WPE_W_F3 = sdRequest.emplaceStage(PEU_Stage::HW_WPE_W_F3);
	setWpeParams(HW_WPE_W_F3, 3);

	/* HW_WPE_W_F4 */
	StageEx &HW_WPE_W_F4 = sdRequest.emplaceStage(PEU_Stage::HW_WPE_W_F4);
	setWpeParams(HW_WPE_W_F4, 4);

	/* HW_WPE_W_F5 */
	StageEx &HW_WPE_W_F5 = sdRequest.emplaceStage(PEU_Stage::HW_WPE_W_F5);
	setWpeParams(HW_WPE_W_F5, 5);

	/* Set notify fence from WPE to DIP */
	HW_WPE_W_F5.addNotify(syncWpeDip_);

	/* HW_WPE_W_F0 */
	StageEx &HW_WPE_W_F0 = sdRequest.emplaceStage(PEU_Stage::HW_WPE_W_F0);
	setWpeParams(HW_WPE_W_F0, 0);

	/* HW_DIP_IDI */
	StageEx &HW_DIP_IDI = sdRequest.emplaceStage(PEU_Stage::HW_DIP_IDI);

	HW_DIP_IDI.input(in.dipTun[6]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	HW_DIP_IDI.input(out.dipImgi[6]->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	HW_DIP_IDI.input(out.dipVipi[6]->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	HW_DIP_IDI.input(in.preDipTnrso->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });

	HW_DIP_IDI.output(out.dipTnrso->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	HW_DIP_IDI.output(out.tnrlfdi->get(), IMG_PORT_IMG3O, 0, mcnrSizes[6]);

	HW_DIP_IDI.setMultiScale(IMG_MULTI_SCALE_DOWN2, 6, 7);
	HW_DIP_IDI.setPqInfo();

	/* Set wait fence from LTR to DIP */
	HW_DIP_IDI.addWait(syncLtrDip_);
	/* Set wait fence from WPE to DIP */
	HW_DIP_IDI.addWait(syncWpeDip_);

	/* HW_DIP_IDI2 */
	StageEx &HW_DIP_IDI2 = sdRequest.emplaceStage(PEU_Stage::HW_DIP_IDI2);
	setDipParams(HW_DIP_IDI2, 5);

	/* HW_DIP_F4 */
	StageEx &HW_DIP_F4 = sdRequest.emplaceStage(PEU_Stage::HW_DIP_F4);
	setDipParams(HW_DIP_F4, 4);

	/* HW_DIP_F3 */
	StageEx &HW_DIP_F3 = sdRequest.emplaceStage(PEU_Stage::HW_DIP_F3);
	setDipParams(HW_DIP_F3, 3);

	/* HW_DIP_F2 */
	StageEx &HW_DIP_F2 = sdRequest.emplaceStage(PEU_Stage::HW_DIP_F2);
	setDipParams(HW_DIP_F2, 2);

	/* HW_DIP_F1 */
	StageEx &HW_DIP_F1 = sdRequest.emplaceStage(PEU_Stage::HW_DIP_F1);
	setDipParams(HW_DIP_F1, 1);

	requestHelper_.queueRequest(ImgSysDevice::kUserIdMcnr, sdRequest);
}

void Dip1Task::setWpeParams(StageEx &stage, unsigned int level)
{
	auto &mcnrSizes = manager_->mcnrSizes;
	auto &wtSizes = manager_->wtSizes;

	auto &in = frames_.in;
	auto &out = frames_.out;

	stage.input(in.wpeTun->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	stage.input(in.prevDipTnrwo[level]->get(), IMG_PORT_WPE_WPEI, 0, Size{ 0, 0 });
	stage.input(out.wpeVeci[level]->get(), IMG_PORT_WPE_VECI, 0, Size{ 0, 0 });
	stage.output(out.dipTnrwi[level]->get(), IMG_PORT_WPE_WPEO, 0, wtSizes[level]);
	if (level == 0) {
		stage.setWpeInfo(IMG_EXTRA_PARAM_ID_WPE_INFO, wtSizes[level],
				 NSCam::NSImgStream::EWPE_HW_TNR,
				 (unsigned int)NSCam::NSImgStream::EWPE_MVMAP);
	} else {
		stage.setWpeInfo(IMG_EXTRA_PARAM_ID_WPE_INFO, wtSizes[level],
				 NSCam::NSImgStream::EWPE_HW_LITE,
				 (unsigned int)NSCam::NSImgStream::EWPE_MVMAP);
	}
	stage.setMvFrame(mcnrSizes[0], kMeL0Size);
}

void Dip1Task::setDipParams(StageEx &stage, unsigned int level)
{
	auto &mcnrSizes = manager_->mcnrSizes;

	auto &in = frames_.in;
	auto &out = frames_.out;

	ASSERT(level != 0);

	if (level == 1)
		stage.output(out.img4oF1->get(), IMG_PORT_IMG4O, 0, mcnrSizes[level]);

	if (level == 5)
		stage.input(out.dipImgi[level + 1]->get(), IMG_PORT_REC_DSI, 2, Size{ 0, 0 });
	else
		stage.input(out.reci[level]->get(), IMG_PORT_REC_DSI, 2, Size{ 0, 0 });

	stage.input(out.dipVipi[level]->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	stage.input(out.dipVbi[level]->get(), IMG_PORT_TNRVBI, 2, Size{ 0, 0 });

	stage.input(in.dipTun[level]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	stage.input(out.dipImgi[level]->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	stage.input(out.dipTnrwi[level]->get(), IMG_PORT_TNRWI, 2, Size{ 0, 0 });
	stage.input(out.dipTnrmi[level]->get(), IMG_PORT_TNRMI, 2, Size{ 0, 0 });
	stage.input(out.dipTnrso->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	stage.input(out.dipTnrci[level]->get(), IMG_PORT_TNRCI, 2, Size{ 0, 0 });
	stage.input(out.tnrlfdi->get(), IMG_PORT_TNRLFDI, 2, Size{ 0, 0 });

	stage.output(out.dipTnrwo[level]->get(), IMG_PORT_TNRWO, 0, mcnrSizes[level]);
	stage.output(out.dipTnrso->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });

	stage.output(out.dipTnrmo[level]->get(), IMG_PORT_TNRMO, 0, mcnrSizes[level]);
	stage.output(out.img3o[level]->get(), IMG_PORT_IMG3O, 0, mcnrSizes[level]);

	stage.setMultiScale(IMG_MULTI_SCALE_DOWN2, level, 7);
	stage.setPqInfo();
	stage.setMvFrame(mcnrSizes[0], kMeL0Size);
}

Dip2Task::Dip2Task(Scheduler *scheduler, const std::string &id,
		   Request *request, uint32_t internalRequestId,
		   ImgSysDevice *imgSys, MCNRFrames &mcnr, McnrTasksManager *manager)
	: Task(scheduler, id), requestHelper_(this, request, imgSys),
	  request_(request), internalRequestId_(internalRequestId), manager_(manager), imgSys_(imgSys)
{
	frames_ = mcnr.dip2Frames;
	videoOut1 = mcnr.videoOut1;
	videoOut2 = mcnr.videoOut2;
}

void Dip2Task::allocateOutputBuffers()
{
	manager_->img4oF0_.fetch(frames_.out.img4oF0);
}

void Dip2Task::notifyDone()
{
	manager_->onDeviceTuner_->tuneDip2(request_, internalRequestId_, frames_,
					   videoOut1, videoOut2);
	Task::notifyDone();
}

void Dip2Task::run()
{
	allocateOutputBuffers();

	auto &mcnrSizes = manager_->mcnrSizes;

	MUINT32 timestampMili = request_->metadata().get(controls::SensorTimestamp).value_or(0);
	SingleDeviceRequest sdRequest;

	sdRequest.init(internalRequestId_, timestampMili, "DIPTask");

	auto &in = frames_.in;
	auto &out = frames_.out;

	/* HW_DIP_F0 */
	StageEx &HW_DIP_F0 = sdRequest.emplaceStage(PEU_Stage::HW_DIP_F0);

	HW_DIP_F0.input(in.prevImg4oF0->get(), IMG_PORT_WPE_TNR_WPEI, 0, Size{ 0, 0 });
	HW_DIP_F0.input(in.reci[0]->get(), IMG_PORT_REC_DSI, 2, Size{ 0, 0 });
	HW_DIP_F0.input(in.dipTun[0]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	HW_DIP_F0.input(in.dipImgi[0]->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	HW_DIP_F0.input(in.dipTnrwi[0]->get(), IMG_PORT_TNRWI, 2, Size{ 0, 0 });
	HW_DIP_F0.input(in.dipTnrmi[0]->get(), IMG_PORT_TNRMI, 2, Size{ 0, 0 });
	HW_DIP_F0.input(out.dipTnrso->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	HW_DIP_F0.input(in.dipTnrci[0]->get(), IMG_PORT_TNRCI, 2, Size{ 0, 0 });
	HW_DIP_F0.input(in.tnrlfdi->get(), IMG_PORT_TNRLFDI, 2, Size{ 0, 0 });
	HW_DIP_F0.input(in.meMmap[0]->get(), IMG_PORT_WPE_TNR_VECI, 0, Size{ 0, 0 });

	Rectangle tncCrop(mcnrSizes[0]);
	if (manager_->needCropTNC16x9_) {
		tncCrop.height = tncCrop.width * 9 / 16;
		tncCrop.y = (mcnrSizes[0].height - tncCrop.height) / 2;
	}

	Rectangle tncCropAlign = tncCrop;
	tncCropAlign.y = tncCropAlign.y / 16 * 16;

	HW_DIP_F0.output(out.img3o[0]->get(), IMG_PORT_IMG3O, 0, tncCropAlign);
	HW_DIP_F0.output(out.img4oF0->get(), IMG_PORT_IMG4O, 0, tncCropAlign);
	HW_DIP_F0.output(out.dipTnrwo[0]->get(), IMG_PORT_TNRWO, 0, tncCropAlign);
	HW_DIP_F0.output(out.dipTnrso->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	HW_DIP_F0.setMultiScale(IMG_MULTI_SCALE_DOWN2, 0, 7);
	HW_DIP_F0.setPqInfo();
	HW_DIP_F0.setMvFrame(mcnrSizes[0], kMeL0Size);

	assert(videoOut1 || videoOut2);

	if (videoOut1) {
		InfoFrame info(formats::NV12, manager_->videoOut1Size_, videoOut1, 64);
		Rectangle crop = ImgSysDevice::getCrop(mcnrSizes[0], info.size());
		crop = ImgSysDevice::cropNoisyBorder(crop);
		HW_DIP_F0.output(info, IMG_PORT_WDMAO, 0, crop);
	}

	if (videoOut2) {
		InfoFrame info(formats::NV12, manager_->videoOut2Size_, videoOut2, 64);
		Rectangle crop = ImgSysDevice::getCrop(mcnrSizes[0], info.size());
		crop = ImgSysDevice::cropNoisyBorder(crop);
		HW_DIP_F0.output(info, IMG_PORT_WROTO, 0, crop);
	}

	HW_DIP_F0.setWpeInfo(IMG_EXTRA_PARAM_ID_WPE_TNR_INFO, mcnrSizes[0],
			     NSCam::NSImgStream::EWPE_HW_TNR,
			     (unsigned int)NSCam::NSImgStream::EWPE_MVMAP | NSCam::NSImgStream::EWPE_IROI);

	HW_DIP_F0.setCostLevel();
	HW_DIP_F0.setImg4oCrop(tncCropAlign);

	requestHelper_.queueRequest(ImgSysDevice::kUserIdMcnr, sdRequest);
}

} /* namespace libcamera */

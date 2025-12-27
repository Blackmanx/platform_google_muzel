/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * mfnr.cpp - MtkISP7 ImgSys Device Mutiple Frame Noise Reduction
 */

#include "mfnr.h"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vector>

#include <libcamera/control_ids.h>
#include <libcamera/formats.h>
#include <libcamera/request.h>

#include "libcamera/internal/dma_heaps.h"
#include "libcamera/internal/framebuffer.h"
#include "libcamera/internal/info_frame.h"
#include "libcamera/internal/mailbox.h"
#include "libcamera/internal/media_device.h"
#include "libcamera/internal/pipeline_handler.h"
#include "libcamera/internal/task_scheduler.h"

#include "pipeline/mtkisp7/odt/on_device_tuner.h"

#include "ImgPortDef.h"
#include "const.h"
#include "single_device.h"
#include "single_device_helper.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

constexpr const char *kMfnrPrecheck = "/run/camera/mfnr_precheck";

constexpr Size kP2sttoSize{ 738624, 1 };
constexpr Size kTnrsoSize{ 40, 1 };
constexpr Size kWrotoSize{ 192, 144 };
using namespace NSCam::NSImgStream;

static void zeroImage(SharedMailBox<InfoFrame> &mailBox)
{
	const InfoFrame &info = mailBox->get();

	MappedFrameBuffer mappedBuffer =
		MappedFrameBuffer(info.buffer(), MappedFrameBuffer::MapFlag::ReadWrite);

	void *dest = mappedBuffer.planes()[0].data();
	size_t length = info.buffer()->planes()[0].length;

	assert(dest);
	assert(mailBox->valid());

	{
		DmaSyncer syncer(info.buffer()->planes()[0].fd.get());
		memset(dest, 0, length);
	}
}

MfnrTasksManager::MfnrTasksManager(
	ImgSysDevice *imgSys, DmaHeap *dmaHeap, OnDeviceTuner *odt)
{
	imgSys_ = imgSys;
	dmaHeap_ = dmaHeap;
	onDeviceTuner_ = odt;

	allBufferPools_.emplace_back(&yuvp010_1_1_pool_);
	allBufferPools_.emplace_back(&yuvp010_1_4_pool_aligned16_);
	allBufferPools_.emplace_back(&yuvp012_1_2_pool_);
	allBufferPools_.emplace_back(&yuvp012_1_4_pool_);
	allBufferPools_.emplace_back(&yuvp012_1_8_pool_);
	allBufferPools_.emplace_back(&yuvp012_1_16_pool_);
	allBufferPools_.emplace_back(&yuvp012_1_32_pool_);
	allBufferPools_.emplace_back(&yuvp012_1_64_pool_);
	allBufferPools_.emplace_back(&y8_1_1_pool_);
	allBufferPools_.emplace_back(&y8_1_2_pool_);
	allBufferPools_.emplace_back(&y8_1_4_pool_);
	allBufferPools_.emplace_back(&y8_1_8_pool_);
	allBufferPools_.emplace_back(&y8_1_16_pool_);
	allBufferPools_.emplace_back(&y8_1_32_pool_);

	allBufferPools_.emplace_back(&fourBytes_pool_);
	allBufferPools_.emplace_back(&nv21_1_64_pool_);
	allBufferPools_.emplace_back(&nv12_wroto_pool_);
}

// static
Size MfnrTasksManager::getSizeAligned(const Size &bayerInputSize)
{
	Size size = bayerInputSize;
	for (size_t i = 0; i < 2; i++) {
		size.width = (size.width + 1) / 2;
		size.height = (size.height + 1) / 2;
		size.alignDownTo(16, 16);
	}

	return size;
}

int MfnrTasksManager::configure(History<MfnrInput> *mfnrInput,
				const Size &bayerInputSize,
				const Size &yuvOutputSize1, const Size &yuvOutputSize2,
				const Size &videoOutputSize1, const Size &videoOutputSize2,
				const Size &confMapSize,
				int sensor_idx)
{
	mfnrInput_ = mfnrInput;
	yuvOutputSize1_ = yuvOutputSize1;
	yuvOutputSize2_ = yuvOutputSize2;
	videoOutputSize1_ = videoOutputSize1;
	videoOutputSize2_ = videoOutputSize2;
	bayerInputSize_ = bayerInputSize;
	sensor_idx_ = sensor_idx;

	mfnrSizes_.resize(7);
	Size size = bayerInputSize_;

	/* Assign the size to 1/2 of the previous level.
	 * Align to 2 for hardware's requirement */
	for (size_t i = 0; i < mfnrSizes_.size(); i++) {
		LOG(MtkISP7, Info) << "mfnrSizes_[" << i << "] = " << size;
		mfnrSizes_[i] = size;
		size.width = (size.width + 1) / 2;
		size.height = (size.height + 1) / 2;
		size.alignUpTo(2, 2);
	}

	mfnrSize_aligned16_ = getSizeAligned(bayerInputSize_);

	confMapSize_ = confMapSize;

	configureBuffers();
	return 0;
}

int MfnrTasksManager::configureBuffers()
{
	p2sttoPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, kP2sttoSize, 4, DmaHeap::CMA);

	yuvp010_1_1_pool_.setFormat(dmaHeap_, formats::NV12_10P_MTISP, mfnrSizes_[0], 0, DmaHeap::System, 16, 16);
	yuvp010_1_4_pool_aligned16_.setFormat(dmaHeap_, formats::NV12_10P_MTISP, mfnrSize_aligned16_, 0, DmaHeap::System, 16, 16);

	yuvp012_1_2_pool_.setFormat(dmaHeap_, formats::NV12_12P_MTISP, mfnrSizes_[1], 0, DmaHeap::System, 16, 16);
	yuvp012_1_4_pool_.setFormat(dmaHeap_, formats::NV12_12P_MTISP, mfnrSizes_[2], 0, DmaHeap::System, 16, 16);
	y8_1_4_pool_aligned16_.createBuffers(dmaHeap_, formats::Y8_MTISP, mfnrSize_aligned16_, 4, DmaHeap::System, 8, 8);

	yuvp012_1_8_pool_.setFormat(dmaHeap_, formats::NV12_12P_MTISP, mfnrSizes_[3], 0, DmaHeap::System, 16, 16);
	yuvp012_1_16_pool_.setFormat(dmaHeap_, formats::NV12_12P_MTISP, mfnrSizes_[4], 0, DmaHeap::System, 16, 16);
	yuvp012_1_32_pool_.setFormat(dmaHeap_, formats::NV12_12P_MTISP, mfnrSizes_[5], 0, DmaHeap::System, 16, 16);
	yuvp012_1_64_pool_.setFormat(dmaHeap_, formats::NV12_12P_MTISP, mfnrSizes_[6], 0, DmaHeap::System, 16, 16);

	y8_1_1_pool_.setFormat(dmaHeap_, formats::Y8_MTISP, mfnrSizes_[0], 0, DmaHeap::System, 16, 16);
	y8_1_2_pool_.setFormat(dmaHeap_, formats::Y8_MTISP, mfnrSizes_[1], 0, DmaHeap::System, 16, 16);
	y8_1_4_pool_.setFormat(dmaHeap_, formats::Y8_MTISP, mfnrSizes_[2], 0, DmaHeap::System, 16, 16);
	y8_1_8_pool_.setFormat(dmaHeap_, formats::Y8_MTISP, mfnrSizes_[3], 0, DmaHeap::System, 16, 16);
	y8_1_16_pool_.setFormat(dmaHeap_, formats::Y8_MTISP, mfnrSizes_[4], 0, DmaHeap::System, 16, 16);
	y8_1_32_pool_.setFormat(dmaHeap_, formats::Y8_MTISP, mfnrSizes_[5], 0, DmaHeap::System, 16, 16);

	fourBytes_pool_.setFormat(dmaHeap_, formats::Y32_MTISP, kTnrsoSize);
	nv21_1_64_pool_.setFormat(dmaHeap_, formats::NV21, mfnrSizes_[6]);
	nv12_wroto_pool_.setFormat(dmaHeap_, formats::NV12, kWrotoSize);

	return 0;
}

int MfnrTasksManager::start()
{
#if !V4L2_STANDARD_MODE
	for (auto &pool : allBufferPools_)
		imgSys_->handleIova(ImgSysDevice::Add, *pool);
#endif
	return 0;
}

int MfnrTasksManager::stop()
{
#if !V4L2_STANDARD_MODE
	for (auto &pool : allBufferPools_)
		imgSys_->handleIova(ImgSysDevice::Delete, *pool);
#endif
	return 0;
}

int MfnrTasksManager::releaseBuffers()
{
	for (auto &pool : allBufferPools_)
		pool->release();

	p2sttoPool_.release();
	y8_1_4_pool_aligned16_.release();

	return 0;
}

void MfnrTasksManager::releaseElasticBuffers()
{
	for (auto &pool : allBufferPools_)
		pool->releaseElastic();
}

bool MfnrTasksManager::mfnrPrecheck()
{
	if (std::filesystem::exists(kMfnrPrecheck)) {
		return true;
	}
	return false;
}

void MfnrTasksManager::makeMFNRFrames(
	MFNRFrames &mfnr,
	uint32_t internalRequestId,
	FrameBuffer *output1Frame,
	FrameBuffer *output2Frame)
{
	mfnr.still1Output = output1Frame;
	mfnr.still2Output = output2Frame;

	/* Should be created and generated by IPA, once it's ready */
	SharedMailBox<std::vector<int>> bssOrder = makeMailBox<std::vector<int>>();

	std::vector<SharedMailBox<InfoFrame>> bssFdMain = makeMailBoxVector<InfoFrame>(kInputRawCount);
	std::vector<SharedMailBox<InfoFrame>> bssFd = makeMailBoxVector<InfoFrame>(kInputRawCount);
	std::vector<SharedMailBox<InfoFrame>> bssFace = makeMailBoxVector<InfoFrame>(kInputRawCount);
	std::vector<SharedMailBox<InfoFrame>> bssPos = makeMailBoxVector<InfoFrame>(kInputRawCount);

	std::vector<SharedMailBox<InfoFrame>> bfbldTun = makeMailBoxVector<InfoFrame>(kInputRawCount);
	std::vector<SharedMailBox<InfoFrame>> bfbldP2stto = makeMailBoxVector<InfoFrame>(kInputRawCount);
	std::vector<SharedMailBox<InfoFrame>> bfbldImg2o = makeMailBoxVector<InfoFrame>(kInputRawCount);
	std::vector<SharedMailBox<InfoFrame>> bfbldImg3o = makeMailBoxVector<InfoFrame>(kInputRawCount);

	std::vector<SharedMailBox<InfoFrame>> bfmeTun = makeMailBoxVector<InfoFrame>(kInputRawCount);
	std::vector<SharedMailBox<InfoFrame>> bfmeImg2o = makeMailBoxVector<InfoFrame>(kInputRawCount);

	std::vector<SharedMailBox<InfoFrame>> swmeTun = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> swmeConfMapBuf = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> swmeWrappingBuf = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> swmeMcmvBuf = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> swmeParamInBuf = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> swmeParamOutBuf = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);

	std::vector<SharedMailBox<InfoFrame>> mcdsF1Tun = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);

	std::vector<SharedMailBox<InfoFrame>> mcdsWpeWpeo = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> mcdsF1Ltyuv2o = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> mcdsF1Ltyuv3o = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> mcdsF1Ltyuv4o = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> mcdsF1Ltyuv5o = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);

	std::vector<SharedMailBox<InfoFrame>> dsTun = makeMailBoxVector<InfoFrame>(kInputRawCount + 1);
	std::vector<SharedMailBox<InfoFrame>> dsYuv2o = makeMailBoxVector<InfoFrame>(kInputRawCount + 1);
	std::vector<SharedMailBox<InfoFrame>> dsYuv3o = makeMailBoxVector<InfoFrame>(kInputRawCount + 1);
	std::vector<SharedMailBox<InfoFrame>> dsYuv4o = makeMailBoxVector<InfoFrame>(kInputRawCount + 1);

	std::vector<SharedMailBox<InfoFrame>> dsVbiV2Tun = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> dsVbiV2Tyuv2o = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> dsVbiV2Tyuv4o = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> dsVbiV2Tyuv3o = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);

	std::vector<SharedMailBox<InfoFrame>> dsVbiV5Tun = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> dsVbiV5Tyuv2o = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> dsVbiV5Tyuv4o = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);
	std::vector<SharedMailBox<InfoFrame>> dsVbiV5Tyuv3o = makeMailBoxVector<InfoFrame>(kInputRawCount - 1);

	std::vector<SharedMailBox<InfoFrame>> msbldFx_Tnrwi = makeMailBoxVector<InfoFrame>(6);
	std::vector<SharedMailBox<InfoFrame>> msbldFx_0Tun = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> msbldFx_0Img4o = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> msbldFx_0Tnrwo = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> msbldFx_0Tnrmo = makeMailBoxVector<InfoFrame>(7);

	std::vector<SharedMailBox<InfoFrame>> msbldFx_1Tun = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> msbldFx_1Img4o = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> msbldFx_1Tnrwo = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> msbldFx_1Tnrmo = makeMailBoxVector<InfoFrame>(7);

	std::vector<SharedMailBox<InfoFrame>> afbldFx_Tun = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> afbldFx_Wroto = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> afbldFx_Wdmao = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> afbldFx_Img3o = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> afbldFx_Img4o = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> afbldFx_Tnrwo = makeMailBoxVector<InfoFrame>(7);
	std::vector<SharedMailBox<InfoFrame>> afbldFx_Tnrmo = makeMailBoxVector<InfoFrame>(7);

	/* Frames used by MfnrTunBsstask */
	BssFrames &bssFrames = mfnr.bssFrames;
	bssFrames.in.bssParamInfo = makeMailBox<InfoFrame>();
	bssFrames.in.bssDataGInfo = makeMailBox<InfoFrame>();
	bssFrames.in.bssTuningInfo = makeMailBox<InfoFrame>();
	bssFrames.in.bssVerInfo = makeMailBox<InfoFrame>();
	bssFrames.out.bssOutDataInfo = makeMailBox<InfoFrame>();

	bssFrames.in.imgi.resize(kInputRawCount);
	bssFrames.in.bssFdMainInfo.resize(kInputRawCount);
	bssFrames.in.bssFdInfo.resize(kInputRawCount);
	bssFrames.in.bssFaceInfo.resize(kInputRawCount);
	bssFrames.in.bssPosInfo.resize(kInputRawCount);
	mfnr.bss_order = bssOrder;
	bssFrames.out.bss_order = bssOrder;
	for (auto i = 0; i < kInputRawCount; i++) {
		MfnrInput *mfnrInput = mfnrInput_->query(internalRequestId - (kInputRawCount - 1 - i));
		bssFrames.in.imgi[i] = mfnrInput->yuvo1;
		bssFrames.in.bssFdMainInfo[i] = bssFdMain[i];
		bssFrames.in.bssFdInfo[i] = bssFd[i];
		bssFrames.in.bssFaceInfo[i] = bssFace[i];
		bssFrames.in.bssPosInfo[i] = bssPos[i];
	}
	/* Frames used by BfbldTask */
	BfbldFrames &bfbldFrames = mfnr.bfbldFrames;
	bfbldFrames.capturedRaws.resize(kInputRawCount);
	bfbldFrames.in.timgi.resize(kInputRawCount);
	for (auto i = 0; i < kInputRawCount; i++) {
		MfnrInput *mfnrInput = mfnrInput_->query(internalRequestId - (kInputRawCount - 1 - i));
		bfbldFrames.capturedRaws[i] = mfnrInput->raw;
		bfbldFrames.in.tunbufi.push_back(bfbldTun[i]);
		bfbldFrames.out.p2stto.push_back(bfbldP2stto[i]);
		bfbldFrames.out.img2o.push_back(bfbldImg2o[i]);
		bfbldFrames.out.img3o.push_back(bfbldImg3o[i]);
	}
	/* Frames used by BfmeTask */
	BfmeFrames &bfmeFrames = mfnr.bfmeFrames;
	bfmeFrames.tncso = bfbldFrames.out.p2stto[0];
	for (auto i = 0; i < kInputRawCount; i++) {
		bfmeFrames.in.imgi.push_back(bfbldFrames.out.img2o[i]);
		bfmeFrames.in.tunbufi.push_back(bfmeTun[i]);
		bfmeFrames.out.img2o.push_back(bfmeImg2o[i]);
	}

	/* Frames used by BfmeTask */
	SwmeFrames &swmeFrame = mfnr.swmeFrames;
	for (auto i = 0; i < kInputRawCount - 1; i++) {
		swmeFrame.in.bss_buf.push_back(bssFrames.out.bssOutDataInfo);
		swmeFrame.in.base_buf.push_back(bfmeFrames.out.img2o[0]);
		swmeFrame.in.ref_buf.push_back(bfmeFrames.out.img2o[i + 1]);
		swmeFrame.in.tuningInfo.push_back(swmeTun[i]);
		swmeFrame.in.paramInInfo.push_back(swmeParamInBuf[i]);
		swmeFrame.out.warpping_map.push_back(swmeWrappingBuf[i]);
		swmeFrame.out.conf_map.push_back(swmeConfMapBuf[i]);
		swmeFrame.out.mcmv.push_back(swmeMcmvBuf[i]);
		swmeFrame.out.paramOutInfo.push_back(swmeParamOutBuf[i]);
	}

	/* Frames used by MCDS_F1 Task */
	//mcdsWpeVeci.resize(3);
	McdsF1Frames &mcdsF1Frames = mfnr.mcdsF1Frames;
	for (auto i = 0; i < kInputRawCount - 1; i++) {
		mcdsF1Frames.in.wpe_wpei.push_back(bfbldFrames.out.img3o[i + 1]);
		mcdsF1Frames.in.tunbufi.push_back(mcdsF1Tun[i]);
		mcdsF1Frames.in.wpe_veci.push_back(swmeFrame.out.warpping_map[i]);
		mcdsF1Frames.out.wpe_wpeo.push_back(mcdsWpeWpeo[i]);
		mcdsF1Frames.out.ltyuv2o.push_back(mcdsF1Ltyuv2o[i]);
		mcdsF1Frames.out.ltyuv3o.push_back(mcdsF1Ltyuv3o[i]);
		mcdsF1Frames.out.ltyuv4o.push_back(mcdsF1Ltyuv4o[i]);
		mcdsF1Frames.out.ltyuv5o.push_back(mcdsF1Ltyuv5o[i]);
	}

	/* Frames used by DS Task */
	DsFrames &dsFrames = mfnr.dsFrames;
	dsFrames.in.ltimgi.push_back(bfbldFrames.out.img3o[0]);
	dsFrames.in.tunbufi.push_back(dsTun[0]);
	dsFrames.out.ltyuv2o.push_back(dsYuv2o[0]);
	dsFrames.out.ltyuv3o.push_back(dsYuv3o[0]);
	dsFrames.out.ltyuv4o.push_back(dsYuv4o[0]);
	for (auto i = 1; i < kInputRawCount + 1; i++) {
		if (i == 1) {
			dsFrames.in.ltimgi.push_back(dsFrames.out.ltyuv4o[0]);
		} else {
			dsFrames.in.ltimgi.push_back(mcdsF1Frames.out.ltyuv4o[i - 2]);
		}
		dsFrames.in.tunbufi.push_back(dsTun[i]);
		dsFrames.out.ltyuv2o.push_back(dsYuv2o[i]);
		dsFrames.out.ltyuv3o.push_back(dsYuv3o[i]);
		dsFrames.out.ltyuv4o.push_back(dsYuv4o[i]);
	}

	/* Frames used by DS_VBI Task */
	DsVbiFrames &dsVbiFramesV2 = mfnr.dsVbiFramesV2;
	DsVbiFrames &dsVbiFramesV5 = mfnr.dsVbiFramesV5;
	for (auto i = 0; i < kInputRawCount - 1; i++) {
		dsVbiFramesV2.in.timgi.push_back(mcdsF1Frames.out.ltyuv5o[i]);
		dsVbiFramesV2.in.tunbufi.push_back(dsVbiV2Tun[i]);
		dsVbiFramesV2.out.tyuv2o.push_back(dsVbiV2Tyuv2o[i]);
		dsVbiFramesV2.out.tyuv3o.push_back(dsVbiV2Tyuv3o[i]);
		dsVbiFramesV2.out.tyuv4o.push_back(dsVbiV2Tyuv4o[i]);

		dsVbiFramesV5.in.timgi.push_back(dsVbiFramesV2.out.tyuv4o[i]);
		dsVbiFramesV5.in.tunbufi.push_back(dsVbiV5Tun[i]);
		dsVbiFramesV5.out.tyuv2o.push_back(dsVbiV5Tyuv2o[i]);
		dsVbiFramesV5.out.tyuv3o.push_back(dsVbiV5Tyuv3o[i]);
		dsVbiFramesV5.out.tyuv4o.push_back(dsVbiV5Tyuv4o[i]);
	}

	auto constructMsbldMailBox =
		[](MsbldFrames &msbld, int idx,
		   std::vector<SharedMailBox<InfoFrame>> &msbldFx_Tun,
		   std::vector<SharedMailBox<InfoFrame>> &msbldFx_Img4o,
		   std::vector<SharedMailBox<InfoFrame>> &msbldFx_Tnrmo,
		   std::vector<SharedMailBox<InfoFrame>> &msbldFx_Tnrwo,
		   SharedMailBox<InfoFrame> &msbldFx_Tnrci,
		   SharedMailBox<InfoFrame> &tnrso) {
			msbld.in.tunbufi = msbldFx_Tun[idx];
			msbld.in.tnrci = msbldFx_Tnrci;
			msbld.out.img4o = msbldFx_Img4o[idx];
			msbld.out.tnrmo = msbldFx_Tnrmo[idx];
			msbld.out.tnrwo = msbldFx_Tnrwo[idx];
			msbld.out.tnrso = tnrso;
		};

	auto constructAfbldMailBox =
		[](AfbldFrames &afbld, int idx,
		   std::vector<SharedMailBox<InfoFrame>> &afbldFx_Tun,
		   std::vector<SharedMailBox<InfoFrame>> &afbldFx_Wroto,
		   std::vector<SharedMailBox<InfoFrame>> &afbldFx_Wdmao,
		   std::vector<SharedMailBox<InfoFrame>> &afbldFx_Img3o,
		   std::vector<SharedMailBox<InfoFrame>> &afbldFx_Img4o,
		   std::vector<SharedMailBox<InfoFrame>> &afbldFx_Tnrwo,
		   std::vector<SharedMailBox<InfoFrame>> &afbldFx_Tnrmo,
		   SharedMailBox<InfoFrame> &afbldFx_Tnrci,
		   SharedMailBox<InfoFrame> &tnrso) {
			afbld.in.tunbufi.push_back(afbldFx_Tun[idx]);
			afbld.in.tnrci.push_back(afbldFx_Tnrci);
			afbld.out.img4o.push_back(afbldFx_Img4o[idx]);
			afbld.out.img3o.push_back(afbldFx_Img3o[idx]);
			afbld.out.wdmao.push_back(afbldFx_Wdmao[idx]);
			afbld.out.wroto.push_back(afbldFx_Wroto[idx]);
			afbld.out.tnrwo.push_back(afbldFx_Tnrwo[idx]);
			afbld.out.tnrmo.push_back(afbldFx_Tnrmo[idx]);
			afbld.out.tnrso.push_back(tnrso);
		};

	AfbldFrames &afbldF6 = mfnr.afbldF6;
	AfbldFrames &afbldF5 = mfnr.afbldF5;
	AfbldFrames &afbldF4 = mfnr.afbldF4;
	AfbldFrames &afbldF3 = mfnr.afbldF3;
	AfbldFrames &afbldF2 = mfnr.afbldF2;
	AfbldFrames &afbldF1 = mfnr.afbldF1;
	AfbldFrames &afbldF0 = mfnr.afbldF0;

	// Create ping-pong buffer for tnrsi/tnrso
	std::vector<SharedMailBox<InfoFrame>> tnrsi = makeMailBoxVector<InfoFrame>(2);
	SharedMailBox<InfoFrame> firstMsbld_tnrsi = makeMailBox<InfoFrame>();
	SharedMailBox<InfoFrame> secondMsbld_tnrsi = makeMailBox<InfoFrame>();
	SharedMailBox<InfoFrame> afbld_tnrsi = makeMailBox<InfoFrame>();
	SharedMailBox<InfoFrame> firstMsbld_tnrso = makeMailBox<InfoFrame>();
	SharedMailBox<InfoFrame> secondMsbld_tnrso = makeMailBox<InfoFrame>();
	SharedMailBox<InfoFrame> afbld_tnrso = makeMailBox<InfoFrame>();
	firstMsbld_tnrsi = tnrsi[0];
	secondMsbld_tnrsi = tnrsi[1];
	afbld_tnrsi = tnrsi[0];
	firstMsbld_tnrso = tnrsi[1];
	secondMsbld_tnrso = tnrsi[0];
	afbld_tnrso = tnrsi[1];

	constructMsbldMailBox(mfnr.msbldFrames[0].msbldF6, 6, msbldFx_0Tun, msbldFx_0Img4o, msbldFx_0Tnrmo, msbldFx_0Tnrwo, swmeFrame.out.conf_map[0], firstMsbld_tnrso);
	constructMsbldMailBox(mfnr.msbldFrames[0].msbldF5, 5, msbldFx_0Tun, msbldFx_0Img4o, msbldFx_0Tnrmo, msbldFx_0Tnrwo, swmeFrame.out.conf_map[0], firstMsbld_tnrso);
	constructMsbldMailBox(mfnr.msbldFrames[0].msbldF4, 4, msbldFx_0Tun, msbldFx_0Img4o, msbldFx_0Tnrmo, msbldFx_0Tnrwo, swmeFrame.out.conf_map[0], firstMsbld_tnrso);
	constructMsbldMailBox(mfnr.msbldFrames[0].msbldF3, 3, msbldFx_0Tun, msbldFx_0Img4o, msbldFx_0Tnrmo, msbldFx_0Tnrwo, swmeFrame.out.conf_map[0], firstMsbld_tnrso);
	constructMsbldMailBox(mfnr.msbldFrames[0].msbldF2, 2, msbldFx_0Tun, msbldFx_0Img4o, msbldFx_0Tnrmo, msbldFx_0Tnrwo, swmeFrame.out.conf_map[0], firstMsbld_tnrso);
	constructMsbldMailBox(mfnr.msbldFrames[0].msbldF1, 1, msbldFx_0Tun, msbldFx_0Img4o, msbldFx_0Tnrmo, msbldFx_0Tnrwo, swmeFrame.out.conf_map[0], firstMsbld_tnrso);
	constructMsbldMailBox(mfnr.msbldFrames[0].msbldF0, 0, msbldFx_0Tun, msbldFx_0Img4o, msbldFx_0Tnrmo, msbldFx_0Tnrwo, swmeFrame.out.conf_map[0], firstMsbld_tnrso);

	constructMsbldMailBox(mfnr.msbldFrames[1].msbldF6, 6, msbldFx_1Tun, msbldFx_1Img4o, msbldFx_1Tnrmo, msbldFx_1Tnrwo, swmeFrame.out.conf_map[1], secondMsbld_tnrso);
	constructMsbldMailBox(mfnr.msbldFrames[1].msbldF5, 5, msbldFx_1Tun, msbldFx_1Img4o, msbldFx_1Tnrmo, msbldFx_1Tnrwo, swmeFrame.out.conf_map[1], secondMsbld_tnrso);
	constructMsbldMailBox(mfnr.msbldFrames[1].msbldF4, 4, msbldFx_1Tun, msbldFx_1Img4o, msbldFx_1Tnrmo, msbldFx_1Tnrwo, swmeFrame.out.conf_map[1], secondMsbld_tnrso);
	constructMsbldMailBox(mfnr.msbldFrames[1].msbldF3, 3, msbldFx_1Tun, msbldFx_1Img4o, msbldFx_1Tnrmo, msbldFx_1Tnrwo, swmeFrame.out.conf_map[1], secondMsbld_tnrso);
	constructMsbldMailBox(mfnr.msbldFrames[1].msbldF2, 2, msbldFx_1Tun, msbldFx_1Img4o, msbldFx_1Tnrmo, msbldFx_1Tnrwo, swmeFrame.out.conf_map[1], secondMsbld_tnrso);
	constructMsbldMailBox(mfnr.msbldFrames[1].msbldF1, 1, msbldFx_1Tun, msbldFx_1Img4o, msbldFx_1Tnrmo, msbldFx_1Tnrwo, swmeFrame.out.conf_map[1], secondMsbld_tnrso);
	constructMsbldMailBox(mfnr.msbldFrames[1].msbldF0, 0, msbldFx_1Tun, msbldFx_1Img4o, msbldFx_1Tnrmo, msbldFx_1Tnrwo, swmeFrame.out.conf_map[1], secondMsbld_tnrso);

	constructAfbldMailBox(afbldF6, 6, afbldFx_Tun, afbldFx_Wroto, afbldFx_Wdmao, afbldFx_Img3o, afbldFx_Img4o, afbldFx_Tnrwo, afbldFx_Tnrmo, swmeFrame.out.conf_map[2], afbld_tnrso);
	constructAfbldMailBox(afbldF5, 5, afbldFx_Tun, afbldFx_Wroto, afbldFx_Wdmao, afbldFx_Img3o, afbldFx_Img4o, afbldFx_Tnrwo, afbldFx_Tnrmo, swmeFrame.out.conf_map[2], afbld_tnrso);
	constructAfbldMailBox(afbldF4, 4, afbldFx_Tun, afbldFx_Wroto, afbldFx_Wdmao, afbldFx_Img3o, afbldFx_Img4o, afbldFx_Tnrwo, afbldFx_Tnrmo, swmeFrame.out.conf_map[2], afbld_tnrso);
	constructAfbldMailBox(afbldF3, 3, afbldFx_Tun, afbldFx_Wroto, afbldFx_Wdmao, afbldFx_Img3o, afbldFx_Img4o, afbldFx_Tnrwo, afbldFx_Tnrmo, swmeFrame.out.conf_map[2], afbld_tnrso);
	constructAfbldMailBox(afbldF2, 2, afbldFx_Tun, afbldFx_Wroto, afbldFx_Wdmao, afbldFx_Img3o, afbldFx_Img4o, afbldFx_Tnrwo, afbldFx_Tnrmo, swmeFrame.out.conf_map[2], afbld_tnrso);
	constructAfbldMailBox(afbldF1, 1, afbldFx_Tun, afbldFx_Wroto, afbldFx_Wdmao, afbldFx_Img3o, afbldFx_Img4o, afbldFx_Tnrwo, afbldFx_Tnrmo, swmeFrame.out.conf_map[2], afbld_tnrso);
	constructAfbldMailBox(afbldF0, 0, afbldFx_Tun, afbldFx_Wroto, afbldFx_Wdmao, afbldFx_Img3o, afbldFx_Img4o, afbldFx_Tnrwo, afbldFx_Tnrmo, swmeFrame.out.conf_map[2], afbld_tnrso);

	afbldF0.tncso = bfbldFrames.out.p2stto[0];

	mfnr.msbldFrames[0].msbldF5.in.tnrwi = msbldFx_Tnrwi[5];
	mfnr.msbldFrames[0].msbldF4.in.tnrwi = msbldFx_Tnrwi[4];
	mfnr.msbldFrames[0].msbldF3.in.tnrwi = msbldFx_Tnrwi[3];
	mfnr.msbldFrames[0].msbldF2.in.tnrwi = msbldFx_Tnrwi[2];
	mfnr.msbldFrames[0].msbldF1.in.tnrwi = msbldFx_Tnrwi[1];
	mfnr.msbldFrames[0].msbldF0.in.tnrwi = msbldFx_Tnrwi[0];

	//MSBLD_F6(0)
	mfnr.msbldFrames[0].msbldF6.in.vipi = dsFrames.out.ltyuv4o[1]; //MTK_YUV_P012:52x40
	mfnr.msbldFrames[0].msbldF6.in.imgi = dsFrames.out.ltyuv4o[2]; //MTK_YUV_P012:52x40
	mfnr.msbldFrames[0].msbldF6.in.tnrsi = firstMsbld_tnrsi; // 4BYTE:40x1

	//MSBLD_F5(0)
	mfnr.msbldFrames[0].msbldF5.in.vipi = dsFrames.out.ltyuv3o[1]; //MTK_YUV_P012:102x78
	mfnr.msbldFrames[0].msbldF5.in.imgi = dsFrames.out.ltyuv3o[2]; //MTK_YUV_P012:102x78
	mfnr.msbldFrames[0].msbldF5.in.tnrsi = firstMsbld_tnrso; //4BYTE:40x1
	mfnr.msbldFrames[0].msbldF5.in.rec_dsi = dsFrames.out.ltyuv4o[1]; //MTK_YUV_P012:52x40
	mfnr.msbldFrames[0].msbldF5.in.tnrvbi = dsVbiFramesV5.out.tyuv2o[0]; //Y8:102x78
	mfnr.msbldFrames[0].msbldF5.in.tnrlfdi = mfnr.msbldFrames[0].msbldF6.out.img4o; //NV21:52x40

	//MSBLD_F4(0)
	mfnr.msbldFrames[0].msbldF4.in.vipi = dsFrames.out.ltyuv2o[1]; //MTK_YUV_P012:204x154
	mfnr.msbldFrames[0].msbldF4.in.imgi = dsFrames.out.ltyuv2o[2]; //MTK_YUV_P012:204x154
	mfnr.msbldFrames[0].msbldF4.in.tnrsi = firstMsbld_tnrso; //4BYTE:40x1
	mfnr.msbldFrames[0].msbldF4.in.rec_dsi = mfnr.msbldFrames[0].msbldF5.out.img4o; //MTK_YUV_P012:102x78
	mfnr.msbldFrames[0].msbldF4.in.tnrvbi = dsVbiFramesV2.out.tyuv4o[0]; //Y8:204x154
	mfnr.msbldFrames[0].msbldF4.in.tnrlfdi = mfnr.msbldFrames[0].msbldF6.out.img4o; //NV21:52x40
	mfnr.msbldFrames[0].msbldF4.in.tnrmi = mfnr.msbldFrames[0].msbldF5.out.tnrmo; //Y8:102x78

	//MSBLD_F3(0)
	mfnr.msbldFrames[0].msbldF3.in.vipi = dsFrames.out.ltyuv4o[0]; //MTK_YUV_P012:408x306
	mfnr.msbldFrames[0].msbldF3.in.imgi = mcdsF1Frames.out.ltyuv4o[0]; //MTK_YUV_P012:408x306
	mfnr.msbldFrames[0].msbldF3.in.tnrsi = firstMsbld_tnrso; //4BYTE:40x1
	mfnr.msbldFrames[0].msbldF3.in.rec_dsi = mfnr.msbldFrames[0].msbldF4.out.img4o; //MTK_YUV_P012:204x154
	mfnr.msbldFrames[0].msbldF3.in.tnrvbi = dsVbiFramesV2.out.tyuv3o[0]; //Y8:408x306
	mfnr.msbldFrames[0].msbldF3.in.tnrlfdi = mfnr.msbldFrames[0].msbldF6.out.img4o; //NV21:52x40
	mfnr.msbldFrames[0].msbldF3.in.tnrmi = mfnr.msbldFrames[0].msbldF4.out.tnrmo; //Y8:204x154

	//MSBLD_F2(0)
	mfnr.msbldFrames[0].msbldF2.in.vipi = dsFrames.out.ltyuv3o[0]; //MTK_YUV_P012:816x612
	mfnr.msbldFrames[0].msbldF2.in.imgi = mcdsF1Frames.out.ltyuv3o[0]; //MTK_YUV_P012:816x612
	mfnr.msbldFrames[0].msbldF2.in.tnrsi = firstMsbld_tnrso; //4BYTE:40x1
	mfnr.msbldFrames[0].msbldF2.in.rec_dsi = mfnr.msbldFrames[0].msbldF3.out.img4o; //MTK_YUV_P012:408x306
	mfnr.msbldFrames[0].msbldF2.in.tnrvbi = dsVbiFramesV2.out.tyuv2o[0]; //Y8:816x612
	mfnr.msbldFrames[0].msbldF2.in.tnrlfdi = mfnr.msbldFrames[0].msbldF6.out.img4o; //NV21:52x40
	mfnr.msbldFrames[0].msbldF2.in.tnrmi = mfnr.msbldFrames[0].msbldF3.out.tnrmo; //Y8:408x306

	//MSBLD_F1(0)
	mfnr.msbldFrames[0].msbldF1.in.vipi = dsFrames.out.ltyuv2o[0]; //MTK_YUV_P012:1632x1224
	mfnr.msbldFrames[0].msbldF1.in.imgi = mcdsF1Frames.out.ltyuv2o[0]; //MTK_YUV_P012:1632x1224
	mfnr.msbldFrames[0].msbldF1.in.tnrsi = firstMsbld_tnrso; //4BYTE:40x1
	mfnr.msbldFrames[0].msbldF1.in.rec_dsi = mfnr.msbldFrames[0].msbldF2.out.img4o; //MTK_YUV_P012:816x612
	mfnr.msbldFrames[0].msbldF1.in.tnrvbi = mcdsF1Frames.out.ltyuv5o[0]; //Y8:1632x1224
	mfnr.msbldFrames[0].msbldF1.in.tnrlfdi = mfnr.msbldFrames[0].msbldF6.out.img4o; //NV21:52x40
	mfnr.msbldFrames[0].msbldF1.in.tnrmi = mfnr.msbldFrames[0].msbldF2.out.tnrmo; //Y8:816x612

	//MSBLD_F0(0)
	mfnr.msbldFrames[0].msbldF0.in.vipi = bfbldFrames.out.img3o[0]; //MTK_YUV_P010:3264x2448
	mfnr.msbldFrames[0].msbldF0.in.imgi = mcdsF1Frames.out.wpe_wpeo[0]; //MTK_YUV_P010:3264x2448
	mfnr.msbldFrames[0].msbldF0.in.tnrsi = firstMsbld_tnrso; //4BYTE:40x1
	mfnr.msbldFrames[0].msbldF0.in.rec_dsi = mfnr.msbldFrames[0].msbldF1.out.img4o; //MTK_YUV_P012:1632x1224
	mfnr.msbldFrames[0].msbldF0.in.tnrvbi = mcdsF1Frames.out.ltyuv5o[0]; //Y8:1632x1224
	mfnr.msbldFrames[0].msbldF0.in.tnrlfdi = mfnr.msbldFrames[0].msbldF6.out.img4o; //NV21:52x40
	mfnr.msbldFrames[0].msbldF0.in.tnrmi = mfnr.msbldFrames[0].msbldF1.out.tnrmo; //Y8:1632x1224

	//MSBLD_F6(1)
	mfnr.msbldFrames[1].msbldF6.in.vipi = dsFrames.out.ltyuv4o[1]; //MTK_YUV_P012:52x40
	mfnr.msbldFrames[1].msbldF6.in.imgi = dsFrames.out.ltyuv4o[3]; //MTK_YUV_P012:52x40
	mfnr.msbldFrames[1].msbldF6.in.tnrsi = secondMsbld_tnrsi; //4BYTE:40x1

	//MSBLD_F5(1)
	mfnr.msbldFrames[1].msbldF5.in.vipi = mfnr.msbldFrames[0].msbldF5.out.img4o; //MTK_YUV_P012:102x78
	mfnr.msbldFrames[1].msbldF5.in.imgi = dsFrames.out.ltyuv3o[3]; //MTK_YUV_P012:102x78
	mfnr.msbldFrames[1].msbldF5.in.tnrsi = secondMsbld_tnrso; //4BYTE:40x1
	mfnr.msbldFrames[1].msbldF5.in.rec_dsi = dsFrames.out.ltyuv4o[1]; //MTK_YUV_P012:52x40
	mfnr.msbldFrames[1].msbldF5.in.tnrwi = mfnr.msbldFrames[0].msbldF5.out.tnrwo; //Y8:102x78
	mfnr.msbldFrames[1].msbldF5.in.tnrvbi = dsVbiFramesV5.out.tyuv2o[1]; //Y8:102x78
	mfnr.msbldFrames[1].msbldF5.in.tnrlfdi = mfnr.msbldFrames[1].msbldF6.out.img4o; //NV21:52x40

	//MSBLD_F4(1)
	mfnr.msbldFrames[1].msbldF4.in.vipi = mfnr.msbldFrames[0].msbldF4.out.img4o; //MTK_YUV_P012:204x154
	mfnr.msbldFrames[1].msbldF4.in.imgi = dsFrames.out.ltyuv2o[3]; //MTK_YUV_P012:204x154
	mfnr.msbldFrames[1].msbldF4.in.tnrsi = secondMsbld_tnrso; //4BYTE:40x1
	mfnr.msbldFrames[1].msbldF4.in.rec_dsi = mfnr.msbldFrames[1].msbldF5.out.img4o; //MTK_YUV_P012:102x78
	mfnr.msbldFrames[1].msbldF4.in.tnrwi = mfnr.msbldFrames[0].msbldF4.out.tnrwo; //Y8:204x154
	mfnr.msbldFrames[1].msbldF4.in.tnrvbi = dsVbiFramesV2.out.tyuv4o[1]; //Y8:204x154
	mfnr.msbldFrames[1].msbldF4.in.tnrlfdi = mfnr.msbldFrames[1].msbldF6.out.img4o; //NV21:52x40
	mfnr.msbldFrames[1].msbldF4.in.tnrmi = mfnr.msbldFrames[1].msbldF5.out.tnrmo; //Y8:102x78

	//MSBLD_F3(1)
	mfnr.msbldFrames[1].msbldF3.in.vipi = mfnr.msbldFrames[0].msbldF3.out.img4o; //MTK_YUV_P012:408x306
	mfnr.msbldFrames[1].msbldF3.in.imgi = mcdsF1Frames.out.ltyuv4o[1]; //MTK_YUV_P012:408x306
	mfnr.msbldFrames[1].msbldF3.in.tnrsi = secondMsbld_tnrso; //4BYTE:40x1
	mfnr.msbldFrames[1].msbldF3.in.rec_dsi = mfnr.msbldFrames[1].msbldF4.out.img4o; //MTK_YUV_P012:204x154
	mfnr.msbldFrames[1].msbldF3.in.tnrwi = mfnr.msbldFrames[0].msbldF3.out.tnrwo; //Y8:408x306
	mfnr.msbldFrames[1].msbldF3.in.tnrvbi = dsVbiFramesV2.out.tyuv3o[1]; //Y8:408x306
	mfnr.msbldFrames[1].msbldF3.in.tnrlfdi = mfnr.msbldFrames[1].msbldF6.out.img4o; //NV21:52x40
	mfnr.msbldFrames[1].msbldF3.in.tnrmi = mfnr.msbldFrames[1].msbldF4.out.tnrmo; //Y8:204x154

	//MSBLD_F2(1)
	mfnr.msbldFrames[1].msbldF2.in.vipi = mfnr.msbldFrames[0].msbldF2.out.img4o; //MTK_YUV_P012:816x612
	mfnr.msbldFrames[1].msbldF2.in.imgi = mcdsF1Frames.out.ltyuv3o[1]; //MTK_YUV_P012:816x612
	mfnr.msbldFrames[1].msbldF2.in.tnrsi = secondMsbld_tnrso; //4BYTE:40x1
	mfnr.msbldFrames[1].msbldF2.in.rec_dsi = mfnr.msbldFrames[1].msbldF3.out.img4o; //MTK_YUV_P012:408x306
	mfnr.msbldFrames[1].msbldF2.in.tnrwi = mfnr.msbldFrames[0].msbldF2.out.tnrwo; //Y8:816x612
	mfnr.msbldFrames[1].msbldF2.in.tnrvbi = dsVbiFramesV2.out.tyuv2o[1]; //Y8:816x612
	mfnr.msbldFrames[1].msbldF2.in.tnrlfdi = mfnr.msbldFrames[1].msbldF6.out.img4o; //NV21:52x40
	mfnr.msbldFrames[1].msbldF2.in.tnrmi = mfnr.msbldFrames[1].msbldF3.out.tnrmo; //Y8:408x306

	//MSBLD_F1(1)
	mfnr.msbldFrames[1].msbldF1.in.vipi = mfnr.msbldFrames[0].msbldF1.out.img4o; //MTK_YUV_P012:1632x1224
	mfnr.msbldFrames[1].msbldF1.in.imgi = mcdsF1Frames.out.ltyuv2o[1]; //MTK_YUV_P012:1632x1224
	mfnr.msbldFrames[1].msbldF1.in.tnrsi = secondMsbld_tnrso; //4BYTE:40x1
	mfnr.msbldFrames[1].msbldF1.in.rec_dsi = mfnr.msbldFrames[1].msbldF2.out.img4o; //MTK_YUV_P012:816x612
	mfnr.msbldFrames[1].msbldF1.in.tnrwi = mfnr.msbldFrames[0].msbldF1.out.tnrwo; //Y8:1632x1224
	mfnr.msbldFrames[1].msbldF1.in.tnrvbi = mcdsF1Frames.out.ltyuv5o[1]; //Y8:1632x1224
	mfnr.msbldFrames[1].msbldF1.in.tnrlfdi = mfnr.msbldFrames[1].msbldF6.out.img4o; //NV21:52x40
	mfnr.msbldFrames[1].msbldF1.in.tnrmi = mfnr.msbldFrames[1].msbldF2.out.tnrmo; //Y8:816x612

	//MSBLD_F0(1)
	mfnr.msbldFrames[1].msbldF0.in.vipi = mfnr.msbldFrames[0].msbldF0.out.img4o; //MTK_YUV_P010:3264x2448
	mfnr.msbldFrames[1].msbldF0.in.imgi = mcdsF1Frames.out.wpe_wpeo[1]; //MTK_YUV_P010:3264x2448
	mfnr.msbldFrames[1].msbldF0.in.tnrsi = secondMsbld_tnrso; //4BYTE:40x1
	mfnr.msbldFrames[1].msbldF0.in.rec_dsi = mfnr.msbldFrames[1].msbldF1.out.img4o; //MTK_YUV_P012:1632x1224
	mfnr.msbldFrames[1].msbldF0.in.tnrwi = mfnr.msbldFrames[0].msbldF0.out.tnrwo; //Y8:3264x2448
	mfnr.msbldFrames[1].msbldF0.in.tnrvbi = mcdsF1Frames.out.ltyuv5o[1]; //Y8:1632x1224
	mfnr.msbldFrames[1].msbldF0.in.tnrlfdi = mfnr.msbldFrames[1].msbldF6.out.img4o; //NV21:52x40
	mfnr.msbldFrames[1].msbldF0.in.tnrmi = mfnr.msbldFrames[1].msbldF1.out.tnrmo; //Y8:1632x1224

	//AFBLD_F6(0)
	afbldF6.in.vipi.push_back(dsFrames.out.ltyuv4o[1]); //MTK_YUV_P012:52x40
	afbldF6.in.imgi.push_back(dsFrames.out.ltyuv4o[4]); //MTK_YUV_P012:52x40
	afbldF6.in.tnrsi.push_back(afbld_tnrsi); //4BYTE:40x1

	//AFBLD_F5(0)
	afbldF5.in.vipi.push_back(mfnr.msbldFrames[1].msbldF5.out.img4o); //MTK_YUV_P012:102x78
	afbldF5.in.imgi.push_back(dsFrames.out.ltyuv3o[4]); //MTK_YUV_P012:102x78
	afbldF5.in.tnrsi.push_back(afbld_tnrso); //4BYTE:40x1
	afbldF5.in.rec_dsi.push_back(dsFrames.out.ltyuv4o[1]); //MTK_YUV_P012:52x40
	afbldF5.in.tnrwi.push_back(mfnr.msbldFrames[1].msbldF5.out.tnrwo); //Y8:102x78
	afbldF5.in.tnrvbi.push_back(dsVbiFramesV5.out.tyuv2o[2]); //Y8:102x78
	afbldF5.in.tnrlfdi.push_back(afbldF6.out.img4o[0]); //NV21:52x40

	//AFBLD_F4(0)
	afbldF4.in.vipi.push_back(mfnr.msbldFrames[1].msbldF4.out.img4o); //MTK_YUV_P012:204x154
	afbldF4.in.imgi.push_back(dsFrames.out.ltyuv2o[4]); //MTK_YUV_P012:204x154
	afbldF4.in.tnrsi.push_back(afbld_tnrso); //4BYTE:40x1
	afbldF4.in.rec_dsi.push_back(afbldF5.out.img3o[0]); //MTK_YUV_P012:102x78
	afbldF4.in.tnrwi.push_back(mfnr.msbldFrames[1].msbldF4.out.tnrwo); //Y8:204x154
	afbldF4.in.tnrvbi.push_back(dsVbiFramesV2.out.tyuv4o[2]); //Y8:204x154
	afbldF4.in.tnrlfdi.push_back(afbldF6.out.img4o[0]); //NV21:52x40
	afbldF4.in.tnrmi.push_back(afbldF5.out.tnrmo[0]); //Y8:102x78

	//AFBLD_F3(0)
	afbldF3.in.vipi.push_back(mfnr.msbldFrames[1].msbldF3.out.img4o); //MTK_YUV_P012:408x306
	afbldF3.in.imgi.push_back(mcdsF1Frames.out.ltyuv4o[2]); //MTK_YUV_P012:408x306
	afbldF3.in.tnrsi.push_back(afbld_tnrso); //4BYTE:40x1
	afbldF3.in.rec_dsi.push_back(afbldF4.out.img3o[0]); //MTK_YUV_P012:204x154
	afbldF3.in.tnrwi.push_back(mfnr.msbldFrames[1].msbldF3.out.tnrwo); //Y8:408x306
	afbldF3.in.tnrvbi.push_back(dsVbiFramesV2.out.tyuv3o[2]); //Y8:408x306
	afbldF3.in.tnrlfdi.push_back(afbldF6.out.img4o[0]); //NV21:52x40
	afbldF3.in.tnrmi.push_back(afbldF4.out.tnrmo[0]); //Y8:204x154

	//AFBLD_F2(0)
	afbldF2.in.vipi.push_back(mfnr.msbldFrames[1].msbldF2.out.img4o); //MTK_YUV_P012:816x612
	afbldF2.in.imgi.push_back(mcdsF1Frames.out.ltyuv3o[2]); //MTK_YUV_P012:816x612
	afbldF2.in.tnrsi.push_back(afbld_tnrso); //4BYTE:40x1
	afbldF2.in.rec_dsi.push_back(afbldF3.out.img3o[0]); //MTK_YUV_P012:408x306
	afbldF2.in.tnrwi.push_back(mfnr.msbldFrames[1].msbldF2.out.tnrwo); //Y8:816x612
	afbldF2.in.tnrvbi.push_back(dsVbiFramesV2.out.tyuv2o[2]); //Y8:816x612
	afbldF2.in.tnrlfdi.push_back(afbldF6.out.img4o[0]); //NV21:52x40
	afbldF2.in.tnrmi.push_back(afbldF3.out.tnrmo[0]); //Y8:408x306

	//AFBLD_F1(0)
	afbldF1.in.vipi.push_back(mfnr.msbldFrames[1].msbldF1.out.img4o); //MTK_YUV_P012:1632x1224
	afbldF1.in.imgi.push_back(mcdsF1Frames.out.ltyuv2o[2]); //MTK_YUV_P012:1632x1224
	afbldF1.in.tnrsi.push_back(afbld_tnrso); //4BYTE:40x1
	afbldF1.in.rec_dsi.push_back(afbldF2.out.img3o[0]); //MTK_YUV_P012:816x612
	afbldF1.in.tnrwi.push_back(mfnr.msbldFrames[1].msbldF1.out.tnrwo); //Y8:1632x1224
	afbldF1.in.tnrvbi.push_back(mcdsF1Frames.out.ltyuv5o[2]); //Y8:1632x1224
	afbldF1.in.tnrlfdi.push_back(afbldF6.out.img4o[0]); //NV21:52x40
	afbldF1.in.tnrmi.push_back(afbldF2.out.tnrmo[0]); //Y8:816x612

	//AFBLD_F0(0)
	afbldF0.in.vipi.push_back(mfnr.msbldFrames[1].msbldF0.out.img4o); //MTK_YUV_P010:3264x2448
	afbldF0.in.imgi.push_back(mcdsF1Frames.out.wpe_wpeo[2]); //MTK_YUV_P010:3264x2448
	afbldF0.in.tnrsi.push_back(afbld_tnrso); //4BYTE:40x1
	afbldF0.in.rec_dsi.push_back(afbldF1.out.img3o[0]); //MTK_YUV_P012:1632x1224
	afbldF0.in.tnrwi.push_back(mfnr.msbldFrames[1].msbldF0.out.tnrwo); //Y8:3264x2448
	afbldF0.in.tnrvbi.push_back(mcdsF1Frames.out.ltyuv5o[2]); //Y8:1632x1224
	afbldF0.in.tnrlfdi.push_back(afbldF6.out.img4o[0]); //NV21:52x40
	afbldF0.in.tnrmi.push_back(afbldF1.out.tnrmo[0]); //Y8:1632x1224
}

std::tuple<BfbldTask *, BfmeTask *, McdsF1Task *, DsTask *, DsVbiTask *, MsbldTask *, MsbldTask *, AfbldTask *>
MfnrTasksManager::makeMfnrTasks(MFNRFrames &mfnr, Scheduler *scheduler,
				const std::string &id, Request *request,
				uint32_t internalRequestId, ImgSysDevice *imgSys, PipelineHandler *pipe)
{
	BfbldTask *bfbldTask = new BfbldTask(scheduler, id + " BFBLD", request, internalRequestId, imgSys, mfnr, this);
	BfmeTask *bfmeTask = new BfmeTask(scheduler, id + " BFME", request, internalRequestId, imgSys, mfnr, this);
	McdsF1Task *mcdsF1Task = new McdsF1Task(scheduler, id + " MCDSF1", request, internalRequestId, imgSys, mfnr, this);
	DsTask *dsTask = new DsTask(scheduler, id + " DS", request, internalRequestId, imgSys, mfnr, this);
	DsVbiTask *dsVbiTask = new DsVbiTask(scheduler, id + " DSVBI", request, internalRequestId, imgSys, mfnr, this);
	MsbldTask *msbldTask1st = new MsbldTask(scheduler, id + " MSBLD_1st", request, internalRequestId, imgSys, mfnr, this, 0);
	MsbldTask *msbldTask2nd = new MsbldTask(scheduler, id + " MSBLD_2nd", request, internalRequestId, imgSys, mfnr, this, 1);
	AfbldTask *afbldTask = new AfbldTask(scheduler, id + " AFBLD", request, internalRequestId, imgSys, mfnr, this, pipe);

	return std::make_tuple(bfbldTask, bfmeTask, mcdsF1Task, dsTask, dsVbiTask, msbldTask1st, msbldTask2nd, afbldTask);
}

BfbldTask::BfbldTask(Scheduler *scheduler, const std::string &id, Request *request, uint32_t internalRequestId,
		     ImgSysDevice *imgSys, MFNRFrames &mfnr, MfnrTasksManager *manager)
	: Task(scheduler, id), requestHelper_(this, request, imgSys),
	  request_(request), internalRequestId_(internalRequestId), manager_(manager)
{
	frames_ = mfnr.bfbldFrames;
	bssOrder_ = mfnr.bss_order;
}

void BfbldTask::allocateOutputBuffers()
{
	auto &out = frames_.out;
	for (auto i = 0; i < kInputRawCount; i++) {
		manager_->p2sttoPool_.fetch(out.p2stto[i]);
		manager_->yuvp010_1_4_pool_aligned16_.fetch(out.img2o[i]);
		manager_->yuvp010_1_1_pool_.fetch(out.img3o[i]);
	}
}

void BfbldTask::notifyDone()
{
	auto bssOrder = bssOrder_->get();
	manager_->onDeviceTuner_->tuneBfbld(internalRequestId_, frames_, bssOrder);
	//BFBLD Precheck
	if (MfnrTasksManager::mfnrPrecheck()) {
		for (auto i = 0; i < kInputRawCount; i++) {
			if (i == 0) {
				LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_BFBLD_BASE:1";
			} else {
				LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_BFBLD_REF:1";
			}
		}
	}
	Task::notifyDone();
}

void BfbldTask::run()
{
	allocateOutputBuffers();
	auto &mfnrSizes_ = manager_->mfnrSizes_;

	MUINT32 timestampMili = request_->metadata().get(controls::SensorTimestamp).value_or(0);
	auto &in = frames_.in;
	auto &out = frames_.out;
	auto &capturedRaws = frames_.capturedRaws;
	auto bssOrder = bssOrder_->get();
	for (auto i = 0; i < (int)bssOrder.size(); i++) {
		in.timgi[i] = capturedRaws[bssOrder[i]];
	}
	SingleDeviceRequest sdRequest;
	sdRequest.init(internalRequestId_, timestampMili, "BfbldTask");
	for (auto i = 0; i < kInputRawCount; i++) {
		if (i == 0) {
			StageEx &BFBLD_BASE = sdRequest.emplaceStage(PEU_Stage::BFBLD_BASE, internalRequestId_ + bssOrder[i]);
			BFBLD_BASE.input(in.tunbufi[0]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
			BFBLD_BASE.input(in.timgi[0]->get(), IMG_PORT_TIMGI, 0, Size{ 0, 0 });
			BFBLD_BASE.output(out.img3o[0]->get(), IMG_PORT_IMG3O, 0, mfnrSizes_[0]);
			BFBLD_BASE.output(out.img2o[0]->get(), IMG_PORT_IMG2O, 0, mfnrSizes_[0]);
			BFBLD_BASE.output(out.p2stto[0]->get(), IMG_PORT_IMGSTATO, 0, mfnrSizes_[0]);
		} else {
			StageEx &BFBLD_REF = sdRequest.emplaceStage(PEU_Stage::BFBLD_REF, internalRequestId_ + bssOrder[i]);
			BFBLD_REF.input(in.tunbufi[i]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
			BFBLD_REF.input(in.timgi[i]->get(), IMG_PORT_TIMGI, 0, Size{ 0, 0 });
			BFBLD_REF.output(out.img3o[i]->get(), IMG_PORT_IMG3O, 0, mfnrSizes_[0]);
			BFBLD_REF.output(out.img2o[i]->get(), IMG_PORT_IMG2O, 0, mfnrSizes_[0]);
			BFBLD_REF.output(out.p2stto[i]->get(), IMG_PORT_IMGSTATO, 0, mfnrSizes_[0]);
		}
	}
	requestHelper_.queueRequest(ImgSysDevice::kUserIdMfnr, sdRequest);
}

McdsF1Task::McdsF1Task(Scheduler *scheduler, const std::string &id, Request *request, uint32_t internalRequestId,
		       ImgSysDevice *imgSys, MFNRFrames &mfnr, MfnrTasksManager *manager)
	: Task(scheduler, id), requestHelper_(this, request, imgSys),
	  request_(request), internalRequestId_(internalRequestId), manager_(manager)
{
	frames_ = mfnr.mcdsF1Frames;
	bssOrder_ = mfnr.bss_order;
}

void McdsF1Task::allocateOutputBuffers()
{
	auto &out = frames_.out;
	for (auto i = 0; i < kInputRawCount - 1; i++) {
		manager_->yuvp010_1_1_pool_.fetch(out.wpe_wpeo[i]);
		manager_->yuvp012_1_2_pool_.fetch(out.ltyuv2o[i]);
		manager_->yuvp012_1_4_pool_.fetch(out.ltyuv3o[i]);
		manager_->yuvp012_1_8_pool_.fetch(out.ltyuv4o[i]);
		manager_->y8_1_2_pool_.fetch(out.ltyuv5o[i]);
	}
}

void McdsF1Task::notifyDone()
{
	auto bssOrder = bssOrder_->get();
	manager_->onDeviceTuner_->tuneMcdsF1(internalRequestId_, frames_, bssOrder);
	//MCDS_F1 Precheck
	if (MfnrTasksManager::mfnrPrecheck()) {
		for (auto i = 0; i < kInputRawCount; i++) {
			LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_MCDS_F1:1";
		}
	}
	Task::notifyDone();
}

void McdsF1Task::run()
{
	allocateOutputBuffers();

	auto &mfnrSizes_ = manager_->mfnrSizes_;
	auto bssOrder = bssOrder_->get();

	MUINT32 timestampMili = request_->metadata().get(controls::SensorTimestamp).value_or(0);
	SingleDeviceRequest sdRequest;

	sdRequest.init(internalRequestId_, timestampMili, "McdsF1Task");

	auto &in = frames_.in;
	auto &out = frames_.out;

	for (auto i = 0; i < kInputRawCount - 1; i++) {
		StageEx &MCDS_F1 = sdRequest.emplaceStage(PEU_Stage::MCDS_F1, internalRequestId_ + bssOrder[i + 1]);
		MCDS_F1.input(in.wpe_wpei[i]->get(), IMG_PORT_WPE_WPEI, 0, Size{ 0, 0 });
		MCDS_F1.input(in.wpe_veci[i]->get(), IMG_PORT_WPE_VECI, 0, Size{ 0, 0 });
		MCDS_F1.input(in.tunbufi[i]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });

		MCDS_F1.output(out.wpe_wpeo[i]->get(), IMG_PORT_WPE_WPEO, 0, Size{ 0, 0 });
		MCDS_F1.output(out.ltyuv2o[i]->get(), IMG_PORT_LTYUV2O, 2, Size{ 0, 0 });
		MCDS_F1.output(out.ltyuv3o[i]->get(), IMG_PORT_LTYUV3O, 2, Size{ 0, 0 });
		MCDS_F1.output(out.ltyuv4o[i]->get(), IMG_PORT_LTYUV4O, 2, Size{ 0, 0 });
		MCDS_F1.output(out.ltyuv5o[i]->get(), IMG_PORT_LTYUV5O, 0, Size{ 0, 0 });
		MCDS_F1.setMcdsF1WpeInfo(IMG_EXTRA_PARAM_ID_WPE_INFO, mfnrSizes_[0], EWPE_HW_LITE);
	}
	requestHelper_.queueRequest(ImgSysDevice::kUserIdMfnr, sdRequest);
}

BfmeTask::BfmeTask(Scheduler *scheduler, const std::string &id, Request *request, uint32_t internalRequestId,
		   ImgSysDevice *imgSys, MFNRFrames &mfnr, MfnrTasksManager *manager)
	: Task(scheduler, id), requestHelper_(this, request, imgSys),
	  request_(request), internalRequestId_(internalRequestId), manager_(manager)
{
	frames_ = mfnr.bfmeFrames;
	bssOrder_ = mfnr.bss_order;
}

void BfmeTask::allocateOutputBuffers()
{
	auto &out = frames_.out;

	for (auto i = 0; i < kInputRawCount; i++) {
		manager_->y8_1_4_pool_aligned16_.fetch(out.img2o[i]);
	}
}

void BfmeTask::notifyDone()
{
	auto bssOrder = bssOrder_->get();
	manager_->onDeviceTuner_->tuneBfme(internalRequestId_, frames_, bssOrder);
	//BFME Precheck
	if (MfnrTasksManager::mfnrPrecheck()) {
		for (auto i = 0; i < kInputRawCount; i++) {
			LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_BFME:1";
		}
	}
	Task::notifyDone();
}

void BfmeTask::run()
{
	allocateOutputBuffers();

	auto &mfnrSize_aligned16 = manager_->mfnrSize_aligned16_;

	MUINT32 timestampMili = request_->metadata().get(controls::SensorTimestamp).value_or(0);
	SingleDeviceRequest sdRequest;

	sdRequest.init(internalRequestId_, timestampMili, "BfmeTask");

	auto &in = frames_.in;
	auto &out = frames_.out;
	auto bssOrder = bssOrder_->get();
	for (auto i = 0; i < kInputRawCount; i++) {
		StageEx &BFME = sdRequest.emplaceStage(PEU_Stage::BFME, internalRequestId_ + bssOrder[i]);
		BFME.input(in.imgi[i]->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
		BFME.input(in.tunbufi[i]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
		BFME.output(out.img2o[i]->get(), IMG_PORT_IMG2O, 0, mfnrSize_aligned16);
		BFME.setMultiScale(IMG_MULTI_SCALE_DOWN4, 1, 0);
	}
	requestHelper_.queueRequest(ImgSysDevice::kUserIdMfnr, sdRequest);
}

DsTask::DsTask(Scheduler *scheduler, const std::string &id, Request *request, uint32_t internalRequestId,
	       ImgSysDevice *imgSys, MFNRFrames &mfnr, MfnrTasksManager *manager)
	: Task(scheduler, id), requestHelper_(this, request, imgSys),
	  request_(request), internalRequestId_(internalRequestId), manager_(manager)
{
	frames_ = mfnr.dsFrames;
	bssOrder_ = mfnr.bss_order;
}

void DsTask::allocateOutputBuffers()
{
	auto &out = frames_.out;
	manager_->yuvp012_1_2_pool_.fetch(out.ltyuv2o[0]);
	manager_->yuvp012_1_4_pool_.fetch(out.ltyuv3o[0]);
	manager_->yuvp012_1_8_pool_.fetch(out.ltyuv4o[0]);
	for (auto i = 1; i < kInputRawCount + 1; i++) {
		manager_->yuvp012_1_16_pool_.fetch(out.ltyuv2o[i]);
		manager_->yuvp012_1_32_pool_.fetch(out.ltyuv3o[i]);
		manager_->yuvp012_1_64_pool_.fetch(out.ltyuv4o[i]);
	}
}

void DsTask::notifyDone()
{
	auto bssOrder = bssOrder_->get();
	manager_->onDeviceTuner_->tuneDs(internalRequestId_, frames_, bssOrder);
	//DS Precheck
	if (MfnrTasksManager::mfnrPrecheck()) {
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_DS:1";
	}

	Task::notifyDone();
}

void DsTask::run()
{
	allocateOutputBuffers();

	//auto &mfnrSizes_ = manager_->mfnrSizes_;

	MUINT32 timestampMili = request_->metadata().get(controls::SensorTimestamp).value_or(0);
	SingleDeviceRequest sdRequest;

	sdRequest.init(internalRequestId_, timestampMili, "DS");

	auto &in = frames_.in;
	auto &out = frames_.out;
	auto bssOrder = bssOrder_->get();
	for (auto i = 0; i < kInputRawCount + 1; i++) {
		if (i == 0 || i == 1) {
			int frameNumber = internalRequestId_ + bssOrder[0];
			StageEx &DS = sdRequest.emplaceStage(PEU_Stage::DS, frameNumber, i);
			DS.input(in.ltimgi[i]->get(), IMG_PORT_LTIMGI, 0, Size{ 0, 0 });
			DS.input(in.tunbufi[i]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
			DS.output(out.ltyuv2o[i]->get(), IMG_PORT_LTYUV2O, 2, Size{ 0, 0 });
			DS.output(out.ltyuv3o[i]->get(), IMG_PORT_LTYUV3O, 2, Size{ 0, 0 });
			DS.output(out.ltyuv4o[i]->get(), IMG_PORT_LTYUV4O, 2, Size{ 0, 0 });
		} else {
			int frameNumber = internalRequestId_ + bssOrder[i - 1];
			StageEx &DS = sdRequest.emplaceStage(PEU_Stage::DS, frameNumber);
			DS.input(in.ltimgi[i]->get(), IMG_PORT_LTIMGI, 0, Size{ 0, 0 });
			DS.input(in.tunbufi[i]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
			DS.output(out.ltyuv2o[i]->get(), IMG_PORT_LTYUV2O, 2, Size{ 0, 0 });
			DS.output(out.ltyuv3o[i]->get(), IMG_PORT_LTYUV3O, 2, Size{ 0, 0 });
			DS.output(out.ltyuv4o[i]->get(), IMG_PORT_LTYUV4O, 2, Size{ 0, 0 });
		}
	}
	requestHelper_.queueRequest(ImgSysDevice::kUserIdMfnr, sdRequest);
}

DsVbiTask::DsVbiTask(Scheduler *scheduler, const std::string &id, Request *request, uint32_t internalRequestId,
		     ImgSysDevice *imgSys, MFNRFrames &mfnr, MfnrTasksManager *manager)
	: Task(scheduler, id), requestHelper_(this, request, imgSys),
	  request_(request), internalRequestId_(internalRequestId), manager_(manager)
{
	dsVbiFramesV2_ = mfnr.dsVbiFramesV2;
	dsVbiFramesV5_ = mfnr.dsVbiFramesV5;
	bssOrder_ = mfnr.bss_order;
}

void DsVbiTask::allocateOutputBuffers()
{
	auto &outV2 = dsVbiFramesV2_.out;
	for (auto i = 0; i < kInputRawCount - 1; i++) {
		manager_->y8_1_4_pool_.fetch(outV2.tyuv2o[i]);
		manager_->y8_1_8_pool_.fetch(outV2.tyuv3o[i]);
		manager_->y8_1_16_pool_.fetch(outV2.tyuv4o[i]);
	}
	auto &outV5 = dsVbiFramesV5_.out;
	for (auto i = 0; i < kInputRawCount - 1; i++) {
		manager_->y8_1_32_pool_.fetch(outV5.tyuv2o[i]);
	}
}

void DsVbiTask::notifyDone()
{
	auto bssOrder = bssOrder_->get();
	manager_->onDeviceTuner_->tuneDsVbi(internalRequestId_, dsVbiFramesV2_, dsVbiFramesV5_, bssOrder);
	//DS_VBI Precheck
	if (MfnrTasksManager::mfnrPrecheck()) {
		for (auto i = 0; i < kInputRawCount - 1; i++) {
			LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_DS_VBI_V2:1";
			LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_DS_VBI_V5:1";
		}
	}
	Task::notifyDone();
}

void DsVbiTask::run()
{
	allocateOutputBuffers();

	//auto &mfnrSizes_ = manager_->mfnrSizes_;

	MUINT32 timestampMili = request_->metadata().get(controls::SensorTimestamp).value_or(0);
	SingleDeviceRequest sdRequest;

	sdRequest.init(internalRequestId_, timestampMili, "DS_VBI");

	auto &inV2 = dsVbiFramesV2_.in;
	auto &outV2 = dsVbiFramesV2_.out;
	auto bssOrder = bssOrder_->get();
	for (auto i = 0; i < kInputRawCount - 1; i++) {
		StageEx &DS_VBI_V2 = sdRequest.emplaceStage(PEU_Stage::DS_VBI_V2, internalRequestId_ + bssOrder[i + 1]);
		DS_VBI_V2.input(inV2.timgi[i]->get(), IMG_PORT_TIMGI, 0, Size{ 0, 0 });
		DS_VBI_V2.input(inV2.tunbufi[i]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });

		DS_VBI_V2.output(outV2.tyuv2o[i]->get(), IMG_PORT_TYUV2O, 2, Size{ 0, 0 });
		DS_VBI_V2.output(outV2.tyuv3o[i]->get(), IMG_PORT_TYUV3O, 2, Size{ 0, 0 });
		DS_VBI_V2.output(outV2.tyuv4o[i]->get(), IMG_PORT_TYUV4O, 2, Size{ 0, 0 });
	}
	auto &inV5 = dsVbiFramesV5_.in;
	auto &outV5 = dsVbiFramesV5_.out;
	for (auto i = 0; i < kInputRawCount - 1; i++) {
		StageEx &DS_VBI_V5 = sdRequest.emplaceStage(PEU_Stage::DS_VBI_V5, internalRequestId_ + bssOrder[i + 1]);

		DS_VBI_V5.input(inV5.timgi[i]->get(), IMG_PORT_TIMGI, 0, Size{ 0, 0 });
		DS_VBI_V5.input(inV5.tunbufi[i]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });

		DS_VBI_V5.output(outV5.tyuv2o[i]->get(), IMG_PORT_TYUV2O, 2, Size{ 0, 0 });
	}
	requestHelper_.queueRequest(ImgSysDevice::kUserIdMfnr, sdRequest);
}

MsbldTask::MsbldTask(Scheduler *scheduler, const std::string &id, Request *request, uint32_t internalRequestId,
		     ImgSysDevice *imgSys, MFNRFrames &mfnr, MfnrTasksManager *manager, int msbldIdx)
	: Task(scheduler, id), requestHelper_(this, request, imgSys),
	  request_(request), internalRequestId_(internalRequestId), manager_(manager),
	  msbldIdx_(msbldIdx)
{
	msbldF6_ = mfnr.msbldFrames[msbldIdx_].msbldF6;
	msbldF5_ = mfnr.msbldFrames[msbldIdx_].msbldF5;
	msbldF4_ = mfnr.msbldFrames[msbldIdx_].msbldF4;
	msbldF3_ = mfnr.msbldFrames[msbldIdx_].msbldF3;
	msbldF2_ = mfnr.msbldFrames[msbldIdx_].msbldF2;
	msbldF1_ = mfnr.msbldFrames[msbldIdx_].msbldF1;
	msbldF0_ = mfnr.msbldFrames[msbldIdx_].msbldF0;
	bssOrder_ = mfnr.bss_order;
}

void MsbldTask::allocateOutputBuffers()
{
	auto &msbldF6_out = msbldF6_.out;
	manager_->nv21_1_64_pool_.fetch(msbldF6_out.img4o);
	auto &msbldF5_out = msbldF5_.out;
	manager_->yuvp012_1_32_pool_.fetch(msbldF5_out.img4o);
	manager_->y8_1_32_pool_.fetch(msbldF5_out.tnrwo);
	manager_->y8_1_32_pool_.fetch(msbldF5_out.tnrmo);
	auto &msbldF4_out = msbldF4_.out;
	manager_->yuvp012_1_16_pool_.fetch(msbldF4_out.img4o);
	manager_->y8_1_16_pool_.fetch(msbldF4_out.tnrwo);
	manager_->y8_1_16_pool_.fetch(msbldF4_out.tnrmo);
	auto &msbldF3_out = msbldF3_.out;
	manager_->yuvp012_1_8_pool_.fetch(msbldF3_out.img4o);
	manager_->y8_1_8_pool_.fetch(msbldF3_out.tnrwo);
	manager_->y8_1_8_pool_.fetch(msbldF3_out.tnrmo);
	auto &msbldF2_out = msbldF2_.out;
	manager_->yuvp012_1_4_pool_.fetch(msbldF2_out.img4o);
	manager_->y8_1_4_pool_.fetch(msbldF2_out.tnrwo);
	manager_->y8_1_4_pool_.fetch(msbldF2_out.tnrmo);
	auto &msbldF1_out = msbldF1_.out;
	manager_->yuvp012_1_2_pool_.fetch(msbldF1_out.img4o);
	manager_->y8_1_2_pool_.fetch(msbldF1_out.tnrwo);
	manager_->y8_1_2_pool_.fetch(msbldF1_out.tnrmo);
	auto &msbldF0_out = msbldF0_.out;
	manager_->yuvp010_1_1_pool_.fetch(msbldF0_out.img4o);
	manager_->y8_1_1_pool_.fetch(msbldF0_out.tnrwo);

	if (msbldIdx_ == 0) {
		manager_->fourBytes_pool_.fetch(msbldF6_.in.tnrsi);
		zeroImage(msbldF6_.in.tnrsi);
		manager_->fourBytes_pool_.fetch(msbldF5_.in.tnrsi);
		zeroImage(msbldF5_.in.tnrsi);

		manager_->y8_1_32_pool_.fetch(msbldF5_.in.tnrwi);
		manager_->y8_1_16_pool_.fetch(msbldF4_.in.tnrwi);
		manager_->y8_1_8_pool_.fetch(msbldF3_.in.tnrwi);
		manager_->y8_1_4_pool_.fetch(msbldF2_.in.tnrwi);
		manager_->y8_1_2_pool_.fetch(msbldF1_.in.tnrwi);
		manager_->y8_1_1_pool_.fetch(msbldF0_.in.tnrwi);

		zeroImage(msbldF5_.in.tnrwi);
		zeroImage(msbldF4_.in.tnrwi);
		zeroImage(msbldF3_.in.tnrwi);
		zeroImage(msbldF2_.in.tnrwi);
		zeroImage(msbldF1_.in.tnrwi);
		zeroImage(msbldF0_.in.tnrwi);
	}
}

void MsbldTask::notifyDone()
{
	auto bssOrder = bssOrder_->get();
	manager_->onDeviceTuner_->tuneMsbld(
		internalRequestId_, msbldF0_, msbldF1_, msbldF2_,
		msbldF3_, msbldF4_, msbldF5_, msbldF6_, bssOrder, msbldIdx_);
	//MSBLD Precheck
	if (MfnrTasksManager::mfnrPrecheck()) {
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_MSBLD_F0:1";
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_MSBLD_F1:1";
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_MSBLD_F2:1";
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_MSBLD_F3:1";
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_MSBLD_F4:1";
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_MSBLD_F5:1";
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_MSBLD_F6:1";
	}
	Task::notifyDone();
}

void MsbldTask::run()
{
	allocateOutputBuffers();
	auto bssOrder = bssOrder_->get();
	auto &mfnrSizes_ = manager_->mfnrSizes_;
	auto &mfnrSize_aligned16_ = manager_->mfnrSize_aligned16_;

	MUINT32 timestampMili = request_->metadata().get(controls::SensorTimestamp).value_or(0);
	SingleDeviceRequest sdRequest;

	sdRequest.init(internalRequestId_, timestampMili, "MSBLD");

	int i = msbldIdx_;

	int frameNumber = internalRequestId_ + bssOrder[i];
	StageEx &MSBLD_F6 = sdRequest.emplaceStage(PEU_Stage::MSBLD_F6, frameNumber);
	auto &msbldF6_in = msbldF6_.in;
	auto &msbldF6_out = msbldF6_.out;
	MSBLD_F6.input(msbldF6_in.vipi->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	MSBLD_F6.input(msbldF6_in.imgi->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	MSBLD_F6.input(msbldF6_in.tnrsi->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	MSBLD_F6.input(msbldF6_in.tunbufi->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	MSBLD_F6.output(msbldF6_out.tnrso->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	MSBLD_F6.output(msbldF6_out.img4o->get(), IMG_PORT_IMG4O, 0, mfnrSizes_[6]);
	MSBLD_F6.setMultiScale(IMG_MULTI_SCALE_DOWN2, 6, 7);
	MSBLD_F6.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);

	StageEx &MSBLD_F5 = sdRequest.emplaceStage(PEU_Stage::MSBLD_F5, frameNumber);
	auto &msbldF5_in = msbldF5_.in;
	auto &msbldF5_out = msbldF5_.out;
	MSBLD_F5.input(msbldF5_in.vipi->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	MSBLD_F5.input(msbldF5_in.imgi->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	MSBLD_F5.input(msbldF5_in.tnrsi->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	MSBLD_F5.input(msbldF5_in.rec_dsi->get(), IMG_PORT_REC_DSI, 2, mfnrSizes_[6]);
	MSBLD_F5.input(msbldF5_in.tnrci->get(), IMG_PORT_TNRCI, 2, manager_->confMapSize_);
	MSBLD_F5.input(msbldF5_in.tnrwi->get(), IMG_PORT_TNRWI, 2, mfnrSizes_[5]);
	MSBLD_F5.input(msbldF5_in.tnrvbi->get(), IMG_PORT_TNRVBI, 2, mfnrSizes_[5]);
	MSBLD_F5.input(msbldF5_in.tnrlfdi->get(), IMG_PORT_TNRLFDI, 2, mfnrSizes_[6]);
	MSBLD_F5.input(msbldF5_in.tunbufi->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });

	MSBLD_F5.output(msbldF5_out.img4o->get(), IMG_PORT_IMG4O, 0, mfnrSizes_[5]);
	MSBLD_F5.output(msbldF5_out.tnrwo->get(), IMG_PORT_TNRWO, 0, mfnrSizes_[5]);
	MSBLD_F5.output(msbldF5_out.tnrmo->get(), IMG_PORT_TNRMO, 0, mfnrSizes_[5]);
	MSBLD_F5.output(msbldF5_out.tnrso->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	MSBLD_F5.setMultiScale(IMG_MULTI_SCALE_DOWN2, 5, 7);
	MSBLD_F5.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);

	StageEx &MSBLD_F4 = sdRequest.emplaceStage(PEU_Stage::MSBLD_F4, frameNumber);
	auto &msbldF4_in = msbldF4_.in;
	auto &msbldF4_out = msbldF4_.out;
	MSBLD_F4.input(msbldF4_in.vipi->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	MSBLD_F4.input(msbldF4_in.imgi->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	MSBLD_F4.input(msbldF4_in.tnrsi->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	MSBLD_F4.input(msbldF4_in.rec_dsi->get(), IMG_PORT_REC_DSI, 2, mfnrSizes_[5]);
	MSBLD_F4.input(msbldF4_in.tnrci->get(), IMG_PORT_TNRCI, 2, manager_->confMapSize_);
	MSBLD_F4.input(msbldF4_in.tnrwi->get(), IMG_PORT_TNRWI, 2, mfnrSizes_[4]);
	MSBLD_F4.input(msbldF4_in.tnrvbi->get(), IMG_PORT_TNRVBI, 2, mfnrSizes_[4]);
	MSBLD_F4.input(msbldF4_in.tnrlfdi->get(), IMG_PORT_TNRLFDI, 2, mfnrSizes_[6]);
	MSBLD_F4.input(msbldF4_in.tnrmi->get(), IMG_PORT_TNRMI, 2, mfnrSizes_[5]);
	MSBLD_F4.input(msbldF4_in.tunbufi->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });

	MSBLD_F4.output(msbldF4_out.img4o->get(), IMG_PORT_IMG4O, 0, mfnrSizes_[4]);
	MSBLD_F4.output(msbldF4_out.tnrwo->get(), IMG_PORT_TNRWO, 0, mfnrSizes_[4]);
	MSBLD_F4.output(msbldF4_out.tnrmo->get(), IMG_PORT_TNRMO, 0, mfnrSizes_[4]);
	MSBLD_F4.output(msbldF4_out.tnrso->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	MSBLD_F4.setMultiScale(IMG_MULTI_SCALE_DOWN2, 4, 7);
	MSBLD_F4.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);
	StageEx &MSBLD_F3 = sdRequest.emplaceStage(PEU_Stage::MSBLD_F3, frameNumber);
	auto &msbldF3_in = msbldF3_.in;
	auto &msbldF3_out = msbldF3_.out;
	MSBLD_F3.input(msbldF3_in.vipi->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	MSBLD_F3.input(msbldF3_in.imgi->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	MSBLD_F3.input(msbldF3_in.tnrsi->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	MSBLD_F3.input(msbldF3_in.rec_dsi->get(), IMG_PORT_REC_DSI, 2, mfnrSizes_[4]);
	MSBLD_F3.input(msbldF3_in.tnrci->get(), IMG_PORT_TNRCI, 2, manager_->confMapSize_);
	MSBLD_F3.input(msbldF3_in.tnrwi->get(), IMG_PORT_TNRWI, 2, mfnrSizes_[3]);
	MSBLD_F3.input(msbldF3_in.tnrvbi->get(), IMG_PORT_TNRVBI, 2, mfnrSizes_[3]);
	MSBLD_F3.input(msbldF3_in.tnrlfdi->get(), IMG_PORT_TNRLFDI, 2, mfnrSizes_[6]);
	MSBLD_F3.input(msbldF3_in.tnrmi->get(), IMG_PORT_TNRMI, 2, mfnrSizes_[4]);
	MSBLD_F3.input(msbldF3_in.tunbufi->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });

	MSBLD_F3.output(msbldF3_out.img4o->get(), IMG_PORT_IMG4O, 0, mfnrSizes_[3]);
	MSBLD_F3.output(msbldF3_out.tnrwo->get(), IMG_PORT_TNRWO, 0, mfnrSizes_[3]);
	MSBLD_F3.output(msbldF3_out.tnrmo->get(), IMG_PORT_TNRMO, 0, mfnrSizes_[3]);
	MSBLD_F3.output(msbldF3_out.tnrso->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	MSBLD_F3.setMultiScale(IMG_MULTI_SCALE_DOWN2, 3, 7);
	MSBLD_F3.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);
	StageEx &MSBLD_F2 = sdRequest.emplaceStage(PEU_Stage::MSBLD_F2, frameNumber);
	auto &msbldF2_in = msbldF2_.in;
	auto &msbldF2_out = msbldF2_.out;

	MSBLD_F2.input(msbldF2_in.vipi->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	MSBLD_F2.input(msbldF2_in.imgi->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	MSBLD_F2.input(msbldF2_in.tnrsi->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	MSBLD_F2.input(msbldF2_in.rec_dsi->get(), IMG_PORT_REC_DSI, 2, mfnrSizes_[3]);
	MSBLD_F2.input(msbldF2_in.tnrci->get(), IMG_PORT_TNRCI, 2, manager_->confMapSize_);
	MSBLD_F2.input(msbldF2_in.tnrwi->get(), IMG_PORT_TNRWI, 2, mfnrSizes_[2]);
	MSBLD_F2.input(msbldF2_in.tnrvbi->get(), IMG_PORT_TNRVBI, 2, mfnrSizes_[2]);
	MSBLD_F2.input(msbldF2_in.tnrlfdi->get(), IMG_PORT_TNRLFDI, 2, mfnrSizes_[6]);
	MSBLD_F2.input(msbldF2_in.tnrmi->get(), IMG_PORT_TNRMI, 2, mfnrSizes_[3]);
	MSBLD_F2.input(msbldF2_in.tunbufi->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });

	MSBLD_F2.output(msbldF2_out.img4o->get(), IMG_PORT_IMG4O, 0, mfnrSizes_[2]);
	MSBLD_F2.output(msbldF2_out.tnrwo->get(), IMG_PORT_TNRWO, 0, mfnrSizes_[2]);
	MSBLD_F2.output(msbldF2_out.tnrmo->get(), IMG_PORT_TNRMO, 0, mfnrSizes_[2]);
	MSBLD_F2.output(msbldF2_out.tnrso->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	MSBLD_F2.setMultiScale(IMG_MULTI_SCALE_DOWN2, 2, 7);
	MSBLD_F2.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);

	StageEx &MSBLD_F1 = sdRequest.emplaceStage(PEU_Stage::MSBLD_F1, frameNumber);
	auto &msbldF1_in = msbldF1_.in;
	auto &msbldF1_out = msbldF1_.out;
	MSBLD_F1.input(msbldF1_in.vipi->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	MSBLD_F1.input(msbldF1_in.imgi->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	MSBLD_F1.input(msbldF1_in.tnrsi->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	MSBLD_F1.input(msbldF1_in.rec_dsi->get(), IMG_PORT_REC_DSI, 2, mfnrSizes_[2]);
	MSBLD_F1.input(msbldF1_in.tnrci->get(), IMG_PORT_TNRCI, 2, manager_->confMapSize_);
	MSBLD_F1.input(msbldF1_in.tnrwi->get(), IMG_PORT_TNRWI, 2, mfnrSizes_[1]);
	MSBLD_F1.input(msbldF1_in.tnrvbi->get(), IMG_PORT_TNRVBI, 2, mfnrSizes_[1]);
	MSBLD_F1.input(msbldF1_in.tnrlfdi->get(), IMG_PORT_TNRLFDI, 2, mfnrSizes_[6]);
	MSBLD_F1.input(msbldF1_in.tnrmi->get(), IMG_PORT_TNRMI, 2, mfnrSizes_[2]);
	MSBLD_F1.input(msbldF1_in.tunbufi->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	MSBLD_F1.output(msbldF1_out.img4o->get(), IMG_PORT_IMG4O, 0, mfnrSizes_[1]);
	MSBLD_F1.output(msbldF1_out.tnrwo->get(), IMG_PORT_TNRWO, 0, mfnrSizes_[1]);
	MSBLD_F1.output(msbldF1_out.tnrmo->get(), IMG_PORT_TNRMO, 0, mfnrSizes_[1]);
	MSBLD_F1.output(msbldF1_out.tnrso->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	MSBLD_F1.setMultiScale(IMG_MULTI_SCALE_DOWN2, 1, 7);
	MSBLD_F1.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);

	StageEx &MSBLD_F0 = sdRequest.emplaceStage(PEU_Stage::MSBLD_F0, frameNumber);
	auto &msbldF0_in = msbldF0_.in;
	auto &msbldF0_out = msbldF0_.out;

	MSBLD_F0.input(msbldF0_in.vipi->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	MSBLD_F0.input(msbldF0_in.imgi->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	MSBLD_F0.input(msbldF0_in.tnrsi->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	MSBLD_F0.input(msbldF0_in.rec_dsi->get(), IMG_PORT_REC_DSI, 2, mfnrSizes_[1]);
	MSBLD_F0.input(msbldF0_in.tnrci->get(), IMG_PORT_TNRCI, 2, manager_->confMapSize_);
	MSBLD_F0.input(msbldF0_in.tnrwi->get(), IMG_PORT_TNRWI, 2, mfnrSizes_[0]);
	MSBLD_F0.input(msbldF0_in.tnrvbi->get(), IMG_PORT_TNRVBI, 2, mfnrSizes_[1]);
	MSBLD_F0.input(msbldF0_in.tnrlfdi->get(), IMG_PORT_TNRLFDI, 2, mfnrSizes_[6]);
	MSBLD_F0.input(msbldF0_in.tnrmi->get(), IMG_PORT_TNRMI, 2, mfnrSizes_[1]);
	MSBLD_F0.input(msbldF0_in.tunbufi->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	MSBLD_F0.output(msbldF0_out.img4o->get(), IMG_PORT_IMG4O, 0, mfnrSizes_[0]);
	MSBLD_F0.output(msbldF0_out.tnrwo->get(), IMG_PORT_TNRWO, 0, mfnrSizes_[0]);
	MSBLD_F0.output(msbldF0_out.tnrso->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	MSBLD_F0.setMultiScale(IMG_MULTI_SCALE_DOWN2, 0, 7);
	MSBLD_F0.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);

	requestHelper_.queueRequest(ImgSysDevice::kUserIdMfnr, sdRequest);
}

AfbldTask::AfbldTask(Scheduler *scheduler, const std::string &id, Request *request, uint32_t internalRequestId,
		     ImgSysDevice *imgSys, MFNRFrames &mfnr, MfnrTasksManager *manager, PipelineHandler *pipe)
	: Task(scheduler, id), requestHelper_(this, request, imgSys),
	  request_(request), internalRequestId_(internalRequestId), manager_(manager), pipe_(pipe)
{
	afbldF0_ = mfnr.afbldF0;
	afbldF1_ = mfnr.afbldF1;
	afbldF2_ = mfnr.afbldF2;
	afbldF3_ = mfnr.afbldF3;
	afbldF4_ = mfnr.afbldF4;
	afbldF5_ = mfnr.afbldF5;
	afbldF6_ = mfnr.afbldF6;
	stillOutput1_ = mfnr.still1Output;
	stillOutput2_ = mfnr.still2Output;
	bssOrder_ = mfnr.bss_order;
}

void AfbldTask::allocateOutputBuffers()
{
	auto &afbldF6_out = afbldF6_.out;
	manager_->nv21_1_64_pool_.fetch(afbldF6_out.img4o[0]);
	auto &afbldF5_out = afbldF5_.out;

	manager_->yuvp012_1_32_pool_.fetch(afbldF5_out.img3o[0]);
	manager_->y8_1_32_pool_.fetch(afbldF5_out.tnrwo[0]);
	manager_->y8_1_32_pool_.fetch(afbldF5_out.tnrmo[0]);

	auto &afbldF4_out = afbldF4_.out;
	manager_->yuvp012_1_16_pool_.fetch(afbldF4_out.img3o[0]);
	manager_->y8_1_16_pool_.fetch(afbldF4_out.tnrwo[0]);
	manager_->y8_1_16_pool_.fetch(afbldF4_out.tnrmo[0]);

	auto &afbldF3_out = afbldF3_.out;
	manager_->yuvp012_1_8_pool_.fetch(afbldF3_out.img3o[0]);
	manager_->y8_1_8_pool_.fetch(afbldF3_out.tnrwo[0]);
	manager_->y8_1_8_pool_.fetch(afbldF3_out.tnrmo[0]);

	auto &afbldF2_out = afbldF2_.out;
	manager_->yuvp012_1_4_pool_.fetch(afbldF2_out.img3o[0]);
	manager_->y8_1_4_pool_.fetch(afbldF2_out.tnrwo[0]);
	manager_->y8_1_4_pool_.fetch(afbldF2_out.tnrmo[0]);

	auto &afbldF1_out = afbldF1_.out;
	manager_->yuvp012_1_2_pool_.fetch(afbldF1_out.img3o[0]);
	manager_->y8_1_2_pool_.fetch(afbldF1_out.tnrwo[0]);
	manager_->y8_1_2_pool_.fetch(afbldF1_out.tnrmo[0]);

	auto &afbldF0_out = afbldF0_.out;
	manager_->yuvp010_1_1_pool_.fetch(afbldF0_out.img3o[0]);
	manager_->y8_1_1_pool_.fetch(afbldF0_out.tnrwo[0]);
	manager_->nv12_wroto_pool_.fetch(afbldF0_out.wroto[0]);
}

void AfbldTask::notifyDone()
{
	auto bssOrder = bssOrder_->get();
	manager_->onDeviceTuner_->tuneAfbld(
		request_, internalRequestId_, afbldF0_, afbldF1_, afbldF2_,
		afbldF3_, afbldF4_, afbldF5_, afbldF6_, bssOrder, stillOutput1_, stillOutput2_);
	//MSBLD Precheck
	if (MfnrTasksManager::mfnrPrecheck()) {
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_AFBLD_F0:1";
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_AFBLD_F1:1";
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_AFBLD_F2:1";
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_AFBLD_F3:1";
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_AFBLD_F4:1";
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_AFBLD_F5:1";
		LOG(MtkISP7, Info) << "[CAT][MFNR] EStage_AFBLD_F6:1";
	}

	if (stillOutput1_)
		pipe_->completeBuffer(request_, stillOutput1_);
	if (stillOutput2_)
		pipe_->completeBuffer(request_, stillOutput2_);

	Task::notifyDone();
}

void AfbldTask::run()
{
	allocateOutputBuffers();

	auto &mfnrSizes_ = manager_->mfnrSizes_;
	auto &mfnrSize_aligned16_ = manager_->mfnrSize_aligned16_;

	MUINT32 timestampMili = request_->metadata().get(controls::SensorTimestamp).value_or(0);
	SingleDeviceRequest sdRequest;
	auto bssOrder = bssOrder_->get();
	int frameNumber = internalRequestId_ + bssOrder[2];
	sdRequest.init(internalRequestId_, timestampMili, "AFBLD");

	StageEx &AFBLD_F6 = sdRequest.emplaceStage(PEU_Stage::AFBLD_F6, frameNumber);
	auto &afbldF6_in = afbldF6_.in;
	auto &afbldF6_out = afbldF6_.out;
	AFBLD_F6.input(afbldF6_in.vipi[0]->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	AFBLD_F6.input(afbldF6_in.imgi[0]->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	AFBLD_F6.input(afbldF6_in.tnrsi[0]->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	AFBLD_F6.input(afbldF6_in.tunbufi[0]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	AFBLD_F6.output(afbldF6_out.tnrso[0]->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	AFBLD_F6.output(afbldF6_out.img4o[0]->get(), IMG_PORT_IMG4O, 0, mfnrSizes_[6]);
	AFBLD_F6.setMultiScale(IMG_MULTI_SCALE_DOWN2, 6, 7);
	AFBLD_F6.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);

	StageEx &AFBLD_F5 = sdRequest.emplaceStage(PEU_Stage::AFBLD_F5, frameNumber);
	auto &afbldF5_in = afbldF5_.in;
	auto &afbldF5_out = afbldF5_.out;
	AFBLD_F5.input(afbldF5_in.vipi[0]->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	AFBLD_F5.input(afbldF5_in.imgi[0]->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	AFBLD_F5.input(afbldF5_in.tnrsi[0]->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	AFBLD_F5.input(afbldF5_in.rec_dsi[0]->get(), IMG_PORT_REC_DSI, 2, mfnrSizes_[6]);
	AFBLD_F5.input(afbldF5_in.tnrci[0]->get(), IMG_PORT_TNRCI, 2, manager_->confMapSize_);
	AFBLD_F5.input(afbldF5_in.tnrwi[0]->get(), IMG_PORT_TNRWI, 2, mfnrSizes_[5]);
	AFBLD_F5.input(afbldF5_in.tnrvbi[0]->get(), IMG_PORT_TNRVBI, 2, mfnrSizes_[5]);
	AFBLD_F5.input(afbldF5_in.tnrlfdi[0]->get(), IMG_PORT_TNRLFDI, 2, mfnrSizes_[6]);
	AFBLD_F5.input(afbldF5_in.tunbufi[0]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	AFBLD_F5.output(afbldF5_out.img3o[0]->get(), IMG_PORT_IMG3O, 0, mfnrSizes_[5]);
	AFBLD_F5.output(afbldF5_out.tnrwo[0]->get(), IMG_PORT_TNRWO, 0, mfnrSizes_[5]);
	AFBLD_F5.output(afbldF5_out.tnrmo[0]->get(), IMG_PORT_TNRMO, 0, mfnrSizes_[5]);
	AFBLD_F5.output(afbldF5_out.tnrso[0]->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	AFBLD_F5.setMultiScale(IMG_MULTI_SCALE_DOWN2, 5, 7);
	AFBLD_F5.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);

	StageEx &AFBLD_F4 = sdRequest.emplaceStage(PEU_Stage::AFBLD_F4, frameNumber);
	auto &afbldF4_in = afbldF4_.in;
	auto &afbldF4_out = afbldF4_.out;
	AFBLD_F4.input(afbldF4_in.vipi[0]->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	AFBLD_F4.input(afbldF4_in.imgi[0]->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	AFBLD_F4.input(afbldF4_in.tnrsi[0]->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	AFBLD_F4.input(afbldF4_in.rec_dsi[0]->get(), IMG_PORT_REC_DSI, 2, mfnrSizes_[5]);
	AFBLD_F4.input(afbldF4_in.tnrci[0]->get(), IMG_PORT_TNRCI, 2, manager_->confMapSize_);
	AFBLD_F4.input(afbldF4_in.tnrwi[0]->get(), IMG_PORT_TNRWI, 2, mfnrSizes_[4]);
	AFBLD_F4.input(afbldF4_in.tnrvbi[0]->get(), IMG_PORT_TNRVBI, 2, mfnrSizes_[4]);
	AFBLD_F4.input(afbldF4_in.tnrlfdi[0]->get(), IMG_PORT_TNRLFDI, 2, mfnrSizes_[6]);
	AFBLD_F4.input(afbldF4_in.tnrmi[0]->get(), IMG_PORT_TNRMI, 2, mfnrSizes_[5]);
	AFBLD_F4.input(afbldF4_in.tunbufi[0]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	AFBLD_F4.output(afbldF4_out.img3o[0]->get(), IMG_PORT_IMG3O, 0, mfnrSizes_[4]);
	AFBLD_F4.output(afbldF4_out.tnrwo[0]->get(), IMG_PORT_TNRWO, 0, mfnrSizes_[4]);
	AFBLD_F4.output(afbldF4_out.tnrmo[0]->get(), IMG_PORT_TNRMO, 0, mfnrSizes_[4]);
	AFBLD_F4.output(afbldF4_out.tnrso[0]->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	AFBLD_F4.setMultiScale(IMG_MULTI_SCALE_DOWN2, 4, 7);
	AFBLD_F4.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);

	StageEx &AFBLD_F3 = sdRequest.emplaceStage(PEU_Stage::AFBLD_F3, frameNumber);
	auto &afbldF3_in = afbldF3_.in;
	auto &afbldF3_out = afbldF3_.out;
	AFBLD_F3.input(afbldF3_in.vipi[0]->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	AFBLD_F3.input(afbldF3_in.imgi[0]->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	AFBLD_F3.input(afbldF3_in.tnrsi[0]->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	AFBLD_F3.input(afbldF3_in.rec_dsi[0]->get(), IMG_PORT_REC_DSI, 2, mfnrSizes_[4]);
	AFBLD_F3.input(afbldF3_in.tnrci[0]->get(), IMG_PORT_TNRCI, 2, manager_->confMapSize_);
	AFBLD_F3.input(afbldF3_in.tnrwi[0]->get(), IMG_PORT_TNRWI, 2, mfnrSizes_[3]);
	AFBLD_F3.input(afbldF3_in.tnrvbi[0]->get(), IMG_PORT_TNRVBI, 2, mfnrSizes_[3]);
	AFBLD_F3.input(afbldF3_in.tnrlfdi[0]->get(), IMG_PORT_TNRLFDI, 2, mfnrSizes_[6]);
	AFBLD_F3.input(afbldF3_in.tnrmi[0]->get(), IMG_PORT_TNRMI, 2, mfnrSizes_[4]);
	AFBLD_F3.input(afbldF3_in.tunbufi[0]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	AFBLD_F3.output(afbldF3_out.img3o[0]->get(), IMG_PORT_IMG3O, 0, mfnrSizes_[3]);
	AFBLD_F3.output(afbldF3_out.tnrwo[0]->get(), IMG_PORT_TNRWO, 0, mfnrSizes_[3]);
	AFBLD_F3.output(afbldF3_out.tnrmo[0]->get(), IMG_PORT_TNRMO, 0, mfnrSizes_[3]);
	AFBLD_F3.output(afbldF3_out.tnrso[0]->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	AFBLD_F3.setMultiScale(IMG_MULTI_SCALE_DOWN2, 3, 7);
	AFBLD_F3.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);

	StageEx &AFBLD_F2 = sdRequest.emplaceStage(PEU_Stage::AFBLD_F2, frameNumber);
	auto &afbldF2_in = afbldF2_.in;
	auto &afbldF2_out = afbldF2_.out;
	AFBLD_F2.input(afbldF2_in.vipi[0]->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	AFBLD_F2.input(afbldF2_in.imgi[0]->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	AFBLD_F2.input(afbldF2_in.tnrsi[0]->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	AFBLD_F2.input(afbldF2_in.rec_dsi[0]->get(), IMG_PORT_REC_DSI, 2, mfnrSizes_[3]);
	AFBLD_F2.input(afbldF2_in.tnrci[0]->get(), IMG_PORT_TNRCI, 2, manager_->confMapSize_);
	AFBLD_F2.input(afbldF2_in.tnrwi[0]->get(), IMG_PORT_TNRWI, 2, mfnrSizes_[2]);
	AFBLD_F2.input(afbldF2_in.tnrvbi[0]->get(), IMG_PORT_TNRVBI, 2, mfnrSizes_[2]);
	AFBLD_F2.input(afbldF2_in.tnrlfdi[0]->get(), IMG_PORT_TNRLFDI, 2, mfnrSizes_[6]);
	AFBLD_F2.input(afbldF2_in.tnrmi[0]->get(), IMG_PORT_TNRMI, 2, mfnrSizes_[3]);
	AFBLD_F2.input(afbldF2_in.tunbufi[0]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	AFBLD_F2.output(afbldF2_out.img3o[0]->get(), IMG_PORT_IMG3O, 0, mfnrSizes_[2]);
	AFBLD_F2.output(afbldF2_out.tnrwo[0]->get(), IMG_PORT_TNRWO, 0, mfnrSizes_[2]);
	AFBLD_F2.output(afbldF2_out.tnrmo[0]->get(), IMG_PORT_TNRMO, 0, mfnrSizes_[2]);
	AFBLD_F2.output(afbldF2_out.tnrso[0]->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	AFBLD_F2.setMultiScale(IMG_MULTI_SCALE_DOWN2, 2, 7);
	AFBLD_F2.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);

	StageEx &AFBLD_F1 = sdRequest.emplaceStage(PEU_Stage::AFBLD_F1, frameNumber);
	auto &afbldF1_in = afbldF1_.in;
	auto &afbldF1_out = afbldF1_.out;
	AFBLD_F1.input(afbldF1_in.vipi[0]->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	AFBLD_F1.input(afbldF1_in.imgi[0]->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	AFBLD_F1.input(afbldF1_in.tnrsi[0]->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	AFBLD_F1.input(afbldF1_in.rec_dsi[0]->get(), IMG_PORT_REC_DSI, 2, mfnrSizes_[2]);
	AFBLD_F1.input(afbldF1_in.tnrci[0]->get(), IMG_PORT_TNRCI, 2, manager_->confMapSize_);
	AFBLD_F1.input(afbldF1_in.tnrwi[0]->get(), IMG_PORT_TNRWI, 2, mfnrSizes_[1]);
	AFBLD_F1.input(afbldF1_in.tnrvbi[0]->get(), IMG_PORT_TNRVBI, 2, mfnrSizes_[1]);
	AFBLD_F1.input(afbldF1_in.tnrlfdi[0]->get(), IMG_PORT_TNRLFDI, 2, mfnrSizes_[6]);
	AFBLD_F1.input(afbldF1_in.tnrmi[0]->get(), IMG_PORT_TNRMI, 2, mfnrSizes_[2]);
	AFBLD_F1.input(afbldF1_in.tunbufi[0]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });
	AFBLD_F1.output(afbldF1_out.img3o[0]->get(), IMG_PORT_IMG3O, 0, mfnrSizes_[1]);
	AFBLD_F1.output(afbldF1_out.tnrwo[0]->get(), IMG_PORT_TNRWO, 0, mfnrSizes_[1]);
	AFBLD_F1.output(afbldF1_out.tnrmo[0]->get(), IMG_PORT_TNRMO, 0, mfnrSizes_[1]);
	AFBLD_F1.output(afbldF1_out.tnrso[0]->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });
	AFBLD_F1.setMultiScale(IMG_MULTI_SCALE_DOWN2, 1, 7);
	AFBLD_F1.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);

	StageEx &AFBLD_F0 = sdRequest.emplaceStage(PEU_Stage::AFBLD_F0, frameNumber);
	auto &afbldF0_in = afbldF0_.in;
	auto &afbldF0_out = afbldF0_.out;
	AFBLD_F0.input(afbldF0_in.vipi[0]->get(), IMG_PORT_VIPI, 0, Size{ 0, 0 });
	AFBLD_F0.input(afbldF0_in.imgi[0]->get(), IMG_PORT_IMGI, 0, Size{ 0, 0 });
	AFBLD_F0.input(afbldF0_in.tnrsi[0]->get(), IMG_PORT_TNRSI, 0, Size{ 0, 0 });
	AFBLD_F0.input(afbldF0_in.rec_dsi[0]->get(), IMG_PORT_REC_DSI, 2, mfnrSizes_[1]);
	AFBLD_F0.input(afbldF0_in.tnrci[0]->get(), IMG_PORT_TNRCI, 2, manager_->confMapSize_);
	AFBLD_F0.input(afbldF0_in.tnrwi[0]->get(), IMG_PORT_TNRWI, 2, mfnrSizes_[0]);
	AFBLD_F0.input(afbldF0_in.tnrvbi[0]->get(), IMG_PORT_TNRVBI, 2, mfnrSizes_[1]);
	AFBLD_F0.input(afbldF0_in.tnrlfdi[0]->get(), IMG_PORT_TNRLFDI, 2, mfnrSizes_[6]);
	AFBLD_F0.input(afbldF0_in.tnrmi[0]->get(), IMG_PORT_TNRMI, 2, mfnrSizes_[1]);
	AFBLD_F0.input(afbldF0_in.tunbufi[0]->get(), IMG_PORT_METAI, 0, Size{ 0, 0 });

	AFBLD_F0.output(afbldF0_out.img3o[0]->get(), IMG_PORT_IMG3O, 0, mfnrSizes_[0]);
	AFBLD_F0.output(afbldF0_out.tnrwo[0]->get(), IMG_PORT_TNRWO, 0, mfnrSizes_[0]);
	AFBLD_F0.output(afbldF0_out.tnrso[0]->get(), IMG_PORT_TNRSO, 0, Size{ 0, 0 });

	AFBLD_F0.setMultiScale(IMG_MULTI_SCALE_DOWN2, 0, 7);
	AFBLD_F0.setMvFrame(mfnrSizes_[0], mfnrSize_aligned16_, 8);

	if (stillOutput1_) {
		InfoFrame info(formats::NV12, manager_->yuvOutputSize1_, stillOutput1_, 64);
		Rectangle crop = ImgSysDevice::getCrop(mfnrSizes_[0], info.size());
		AFBLD_F0.output(info, IMG_PORT_WDMAO, 0, crop);
	}

	if (stillOutput2_) {
		InfoFrame info(formats::NV12, manager_->yuvOutputSize2_, stillOutput2_, 64);
		Rectangle crop = ImgSysDevice::getCrop(Size{ 192, 144 }, info.size());
		AFBLD_F0.output(info, IMG_PORT_WROTO, 0, crop);
	}

	AFBLD_F0.setPqInfo();
	requestHelper_.queueRequest(ImgSysDevice::kUserIdMfnr, sdRequest);
}

} /* namespace libcamera */

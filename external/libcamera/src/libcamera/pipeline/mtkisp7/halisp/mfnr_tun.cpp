/*
 * Copyright (C) 2024, Google Inc.
 *
 * mfnr_tun.cpp - MTK MtkISP7 mfnr tuning generator
 */

#include "mfnr_tun.h"

#include <functional>
#include <memory>

#include <libcamera/base/signal.h>
#include <libcamera/base/thread.h>

#include <libcamera/formats.h>
#include <libcamera/geometry.h>

#include "libcamera/internal/mailbox.h"
#include "libcamera/internal/task_scheduler.h"

#include "pipeline/mtkisp7/imgsys/const.h"
#include "pipeline/mtkisp7/imgsys/mfnr.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/feature.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"
#include "platform/mtkisp7/halisp/ITuningDataProvider.h"
#include "tuning_mapping/cam_idx_struct_ext_pub.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

namespace {

static constexpr Size kTunSize{ 219348, 1 };
constexpr Size kBssGmDataMSize{ 5, 1 };

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

MfnrTunManager::MfnrTunManager(
	DmaHeap *dmaHeap, IPADelegate *ipa, OnDeviceTuner *odt)
{
	dmaHeap_ = dmaHeap;
	ipa_ = ipa;
	onDeviceTuner_ = odt;
}

void MfnrTunManager::allocateBuffers()
{
	mfnrTun_.createBuffers(dmaHeap_, formats::MTFD_MTISP, kTunSize, 43, DmaHeap::CMA);
	mfnrTun_.mmap();

	bssParamPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, Size(sizeof(IBSS_PARAM_STRUCT), 1), 1);
	bssDataGPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, Size(sizeof(IBSS_INPUT_DATA_G), 1), 1);
	bssVerPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, kBssGmDataMSize, 1);
	bssTuningPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, Size(sizeof(mtk::isphal::v1::isp_bss_Param), 1), 1);
	bssFdMainPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, Size(sizeof(FD_DATATYPE), 1), kInputRawCount);
	bssFdPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, Size(sizeof(IBssFaceMetadata), 1), kInputRawCount);
	bssFacePool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, Size(sizeof(IBssFace) * 15, 1), kInputRawCount);
	bssPosPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, Size(sizeof(IBssFaceInfo) * 15, 1), kInputRawCount);
	bssOutDataPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, Size(sizeof(IBSS_OUTPUT_DATA), 1), 1);

	bssParamPool_.mmap();
	bssDataGPool_.mmap();
	bssVerPool_.mmap();
	bssTuningPool_.mmap();
	bssFdMainPool_.mmap();
	bssFdPool_.mmap();
	bssFacePool_.mmap();
	bssPosPool_.mmap();
	bssOutDataPool_.mmap();

	swmeOutPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, Size(sizeof(IMFBLL_PROC1_OUT_STRUCT), 1), kInputRawCount - 1);
	swmeParamPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, Size(sizeof(IMFBLL_SET_PROC_INFO_STRUCT), 1), kInputRawCount - 1);
	swmeTuningPool_.createBuffers(dmaHeap_, formats::MTFD_MTISP, Size(sizeof(mtk::isphal::v1::isp_swme_Param), 1), kInputRawCount - 1);
	wrap2pPool_.createBuffers(dmaHeap_, formats::WARP2P_MTISP, wrappingMapSize_, 3, DmaHeap::System, 1, 1);
	tnrciPool_.createBuffers(dmaHeap_, formats::Y8_MTISP, confMapSize_, 3, DmaHeap::System);
	fourBytes_1_16_pool_.createBuffers(dmaHeap_, formats::Y32_MTISP, mfnrSizes_[4], 3);

	tnrciPool_.mmap();
	wrap2pPool_.mmap();
	fourBytes_1_16_pool_.mmap();
	swmeOutPool_.mmap();
	swmeParamPool_.mmap();
	swmeTuningPool_.mmap();
}

void MfnrTunManager::releaseBuffers()
{
	mfnrTun_.release();

	bssParamPool_.release();
	bssDataGPool_.release();
	bssVerPool_.release();
	bssTuningPool_.release();
	bssFdMainPool_.release();
	bssFdPool_.release();
	bssFacePool_.release();
	bssPosPool_.release();
	bssOutDataPool_.release();

	tnrciPool_.release();
	wrap2pPool_.release();
	fourBytes_1_16_pool_.release();
	swmeOutPool_.release();
	swmeParamPool_.release();
	swmeTuningPool_.release();
}

int MfnrTunManager::configure(const Size &bayerInputSize,
			      const Size &yuvOutput1Size, const Size &yuvOutput2Size,
			      const Size &mfnrSize_aligned16, const Size &wrappingMapSize,
			      const Size &confMapSize)
{
	yuvOutput1Size_ = yuvOutput1Size;
	yuvOutput2Size_ = yuvOutput2Size;

	bayerInputSize_ = bayerInputSize;
	mfnrSize_aligned16_ = mfnrSize_aligned16;

	wrappingMapSize_ = wrappingMapSize;
	confMapSize_ = confMapSize;

	mfnrSizes_.resize(7);
	Size size = bayerInputSize_;

	/* Assign the size to 1/2 of the previous level.
	 * Align to 2 for hardware's requirement */
	for (size_t i = 0; i < mfnrSizes_.size(); i++) {
		mfnrSizes_[i] = size;
		size.width = (size.width + 1) / 2;
		size.height = (size.height + 1) / 2;
		size.alignUpTo(2, 2);
	}
	needCropTNC16x9_ = false;
	if ((yuvOutput1Size_.width * 9 == yuvOutput1Size_.height * 16) &&
	    (yuvOutput2Size_.width * 9 == yuvOutput2Size_.height * 16))
		needCropTNC16x9_ = true;

	allocateBuffers();

	return 0;
}

std::tuple<MfnrTunBssTask *, MfnrTunBfbldTask *, MfnrTunBfmeTask *,
	   MfnrTunSwmeTask *, MfnrTunDsTask *, MfnrTunDsVbiTask *, MfnrTunMcdsF1Task *,
	   MfnrTunMsbldTask *, MfnrTunMsbldTask *, MfnrTunAfbldTask *>
MfnrTunManager::makeMfnrTunTasks(MFNRFrames &mfnr,
				 SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange,
				 uint32_t camSysMetaRequestId,
				 Scheduler *scheduler,
				 const std::string &id, Request *request,
				 uint32_t internalRequestId)
{
	MfnrTunBssTask *mfnrTunBssTask = new MfnrTunBssTask(
		mfnr, aaaIspExchange, scheduler, id + " Bss", this, internalRequestId);

	MfnrTunBfbldTask *mfnrTunBfbldTask = new MfnrTunBfbldTask(
		mfnr, camSysMetaRequestId, scheduler, id + " Bfbld", request, this, internalRequestId);

	MfnrTunBfmeTask *mfnrTunBfmeTask = new MfnrTunBfmeTask(
		mfnr, camSysMetaRequestId, scheduler, id + " Bfme", request, this, internalRequestId);

	MfnrTunSwmeTask *mfnrTunSwmeTask = new MfnrTunSwmeTask(
		mfnr, scheduler, id + " Swme", this, internalRequestId);

	MfnrTunDsTask *mfnrTunDsTask = new MfnrTunDsTask(
		mfnr, camSysMetaRequestId, scheduler, id + " Ds", request, this, internalRequestId);

	MfnrTunDsVbiTask *mfnrTunDsVbiTask = new MfnrTunDsVbiTask(
		mfnr, camSysMetaRequestId, scheduler, id + " DsVbi", request, this, internalRequestId);

	MfnrTunMcdsF1Task *mfnrTunMcdsF1Task = new MfnrTunMcdsF1Task(
		mfnr, camSysMetaRequestId, scheduler, id + " McdsF1", request, this, internalRequestId);

	MfnrTunMsbldTask *mfnrTunMsbldTask1st = new MfnrTunMsbldTask(
		mfnr, camSysMetaRequestId, scheduler, id + " Msbld1", request, this, internalRequestId, 0);

	MfnrTunMsbldTask *mfnrTunMsbldTask2nd = new MfnrTunMsbldTask(
		mfnr, camSysMetaRequestId, scheduler, id + " Msbld2", request, this, internalRequestId, 1);

	MfnrTunAfbldTask *mfnrTunAfbldTask = new MfnrTunAfbldTask(
		mfnr, camSysMetaRequestId, scheduler, id + " Afbld", request, this, internalRequestId);

	return std::make_tuple(mfnrTunBssTask, mfnrTunBfbldTask, mfnrTunBfmeTask,
			       mfnrTunSwmeTask, mfnrTunDsTask, mfnrTunDsVbiTask, mfnrTunMcdsF1Task,
			       mfnrTunMsbldTask1st, mfnrTunMsbldTask2nd, mfnrTunAfbldTask);
}

MfnrTunBssTask::MfnrTunBssTask(MFNRFrames &mfnr,
			       SharedMailBox<ipa::mtkisp7::AaaIspExchange> aaaIspExchange,
			       Scheduler *scheduler,
			       const std::string &id, MfnrTunManager *manager,
			       uint32_t internalRequestId)
	: Task(scheduler, id), manager_(manager),
	  internalRequestId_(internalRequestId),
	  aaaIspExchange_(aaaIspExchange)
{
	bssFrames_ = mfnr.bssFrames;
}

void MfnrTunBssTask::allocateBuffers()
{
	manager_->bssParamPool_.fetch(bssFrames_.in.bssParamInfo);
	manager_->bssDataGPool_.fetch(bssFrames_.in.bssDataGInfo);
	manager_->bssTuningPool_.fetch(bssFrames_.in.bssTuningInfo);
	manager_->bssVerPool_.fetch(bssFrames_.in.bssVerInfo);
	manager_->bssOutDataPool_.fetch(bssFrames_.out.bssOutDataInfo);

	for (auto i = 0; i < kInputRawCount; i++) {
		manager_->bssFdMainPool_.fetch(bssFrames_.in.bssFdMainInfo[i]);
		manager_->bssFdPool_.fetch(bssFrames_.in.bssFdInfo[i]);
		manager_->bssFacePool_.fetch(bssFrames_.in.bssFaceInfo[i]);
		manager_->bssPosPool_.fetch(bssFrames_.in.bssPosInfo[i]);
	}
}

void MfnrTunBssTask::run()
{
	allocateBuffers();

	auto &in = bssFrames_.in;
	// TODO: check if needed:

	auto &out = bssFrames_.out;
	ipa::mtkisp7::BssFramesData bssFramesData;
	bssFramesData.bssParamInfoId = in.bssParamInfo->get().buffer()->cookie();
	bssFramesData.bssDataGInfoId = in.bssDataGInfo->get().buffer()->cookie();
	bssFramesData.bssVerInfoId = in.bssVerInfo->get().buffer()->cookie();
	bssFramesData.bssTuningInfoId = in.bssTuningInfo->get().buffer()->cookie();
	for (const SharedMailBox<InfoFrame> &info : in.bssFdMainInfo)
		bssFramesData.bssFdMainInfoId.push_back(info->get().buffer()->cookie());
	for (const SharedMailBox<InfoFrame> &info : in.imgi)
		bssFramesData.imgiId.push_back(info->get().buffer()->cookie());
	for (const SharedMailBox<InfoFrame> &info : in.bssFdInfo)
		bssFramesData.bssFdInfoId.push_back(info->get().buffer()->cookie());
	for (const SharedMailBox<InfoFrame> &info : in.bssFaceInfo)
		bssFramesData.bssFaceInfoId.push_back(info->get().buffer()->cookie());
	for (const SharedMailBox<InfoFrame> &info : in.bssPosInfo)
		bssFramesData.bssPosInfoId.push_back(info->get().buffer()->cookie());
	bssFramesData.bssOutDataInfoId = out.bssOutDataInfo->get().buffer()->cookie();

	auto aaaIspExchange = aaaIspExchange_->get();
	bssFramesData.exposure = aaaIspExchange.aaaMetadata.get(controls::ExposureTime).value_or(333333);
	bssFramesData.iso = aaaIspExchange.aaaMetadata.get(controls::AnalogueGain).value_or(100);
	manager_->ipa_->doBss(this, bssFramesData, internalRequestId_);
}

void MfnrTunBssTask::notifyBssResult(
	const std::vector<int32_t> &bssOrder)
{
	auto &in = bssFrames_.in;
	auto &out = bssFrames_.out;

	out.bss_order->put(bssOrder, NULL);
	if (!manager_->onDeviceTuner_->isEnabled()) {
		notifyDone();
		return;
	}

	// TODO: remove sync when odt is disabled.
	MappedFrameBuffer mappedBssParamBuffers =
		MappedFrameBuffer(in.bssParamInfo->get().buffer(), MappedFrameBuffer::MapFlag::ReadWrite);
	DmaSyncer syncer_bssParam(in.bssParamInfo->get().buffer()->planes()[0].fd.get());

	MappedFrameBuffer mappedBssDataGBuffers =
		MappedFrameBuffer(in.bssDataGInfo->get().buffer(), MappedFrameBuffer::MapFlag::ReadWrite);
	DmaSyncer syncer_dataG(in.bssDataGInfo->get().buffer()->planes()[0].fd.get());

	std::vector<MappedFrameBuffer> mappedBssFdMain;
	std::vector<MappedFrameBuffer> mappedBssFd;
	std::vector<MappedFrameBuffer> mappedFace;
	std::vector<MappedFrameBuffer> mappedPos;

	MappedFrameBuffer mappedBssTuningBuffers =
		MappedFrameBuffer(in.bssTuningInfo->get().buffer(), MappedFrameBuffer::MapFlag::ReadWrite);
	DmaSyncer syncer_bssTuningInfo(in.bssTuningInfo->get().buffer()->planes()[0].fd.get());

	MappedFrameBuffer mappedBssVerInfoBuffers =
		MappedFrameBuffer(in.bssVerInfo->get().buffer(), MappedFrameBuffer::MapFlag::ReadWrite);
	DmaSyncer syncer_bssVerInfo(in.bssVerInfo->get().buffer()->planes()[0].fd.get());

	MappedFrameBuffer mappedBssOutBuffers =
		MappedFrameBuffer(out.bssOutDataInfo->get().buffer(), MappedFrameBuffer::MapFlag::ReadWrite);
	DmaSyncer syncer_bssOut(out.bssOutDataInfo->get().buffer()->planes()[0].fd.get());

	for (int i = 0; i < kInputRawCount; i++) {
		mappedBssFdMain.push_back(MappedFrameBuffer(in.bssFdMainInfo[i]->get().buffer(), MappedFrameBuffer::MapFlag::ReadWrite));
		DmaSyncer syncer_bssFdMainInfo(in.bssFdMainInfo[i]->get().buffer()->planes()[0].fd.get());
		mappedBssFd.push_back(MappedFrameBuffer(in.bssFdInfo[i]->get().buffer(), MappedFrameBuffer::MapFlag::ReadWrite));
		DmaSyncer syncer_bssFdInfo(in.bssFdInfo[i]->get().buffer()->planes()[0].fd.get());
		mappedFace.push_back(MappedFrameBuffer(in.bssFaceInfo[i]->get().buffer(), MappedFrameBuffer::MapFlag::ReadWrite));
		DmaSyncer syncer_bssFaceInfo(in.bssFaceInfo[i]->get().buffer()->planes()[0].fd.get());
		mappedPos.push_back(MappedFrameBuffer(in.bssPosInfo[i]->get().buffer(), MappedFrameBuffer::MapFlag::ReadWrite));
		DmaSyncer syncer_bssPosInfo(in.bssPosInfo[i]->get().buffer()->planes()[0].fd.get());
	}

	manager_->onDeviceTuner_->tuneBss(internalRequestId_, bssFrames_, kInputRawCount);
	notifyDone();
}

MfnrTunBfbldTask::MfnrTunBfbldTask(MFNRFrames &mfnr,
				   uint32_t camSysMetaRequestId,
				   Scheduler *scheduler,
				   const std::string &id, Request *request, MfnrTunManager *manager,
				   uint32_t internalRequestId)
	: ImgSysTask(scheduler, id, camSysMetaRequestId, internalRequestId,
		     Feature::Capture_mfnr, manager->ipa_, request->controls()),
	  request_(request), manager_(manager)
{
	bfbldTun_ = mfnr.bfbldFrames.in.tunbufi;
	bssOrder_ = mfnr.bss_order;
}

void MfnrTunBfbldTask::run()
{
	ipa::mtkisp7::ImgMetaRequestData request;
	std::vector<ipa::mtkisp7::ImgMetaRequestData> requests;

	for (auto i = 0; i < kInputRawCount; i++) {
		manager_->mfnrTun_.fetch(bfbldTun_[i]);
	}
	auto &bssOrder = bssOrder_->get();

	//fillTuning(bfbldRefTun_, tuningBuffers.capture_BFBLD_REF_tunbufi);
	for (auto i = 0; i < kInputRawCount; i++) {
		auto frameNumber = internalRequestId_ + bssOrder[i];
		request = ipa::mtkisp7::ImgMetaRequestData(
			true,
			(i == 0) ? NSIspTuning::EStage_BFBLD_BASE
				 : NSIspTuning::EStage_BFBLD_REF,
			bfbldTun_[i]->get().buffer()->cookie(),
			0, 0, manager_->mfnrSizes_[0],
			manager_->yuvOutput1Size_, manager_->yuvOutput2Size_,
			manager_->mfnrSizes_[0], {}, true, 0, 1,
			bssOrder[i], i == 0, true, frameNumber);
		requests.push_back(std::move(request));
	}

	getImgSysMetaTuning(manager_->needCropTNC16x9_, requests);
}

MfnrTunBfmeTask::MfnrTunBfmeTask(MFNRFrames &mfnr,
				 uint32_t camSysMetaRequestId,
				 Scheduler *scheduler,
				 const std::string &id, Request *request, MfnrTunManager *manager,
				 uint32_t internalRequestId)
	: ImgSysTask(scheduler, id, camSysMetaRequestId, internalRequestId,
		     Feature::Capture_mfnr, manager->ipa_, request->controls()),
	  request_(request), manager_(manager)
{
	bfmeTun_ = mfnr.bfmeFrames.in.tunbufi;
	bssOrder_ = mfnr.bss_order;
	tncso_ = mfnr.bfmeFrames.tncso;
}

void MfnrTunBfmeTask::run()
{
	ipa::mtkisp7::ImgMetaRequestData request;
	std::vector<ipa::mtkisp7::ImgMetaRequestData> requests;

	for (auto i = 0; i < kInputRawCount; i++) {
		manager_->mfnrTun_.fetch(bfmeTun_[i]);
	}
	//fillTuning(bfmeTun_, tuningBuffers.capture_BFME_tunbufi);

	auto &bssOrder = bssOrder_->get();
	for (auto i = 0; i < kInputRawCount; i++) {
		auto frameNumber = internalRequestId_ + bssOrder[i];
		request = ipa::mtkisp7::ImgMetaRequestData(
			true, NSIspTuning::EStage_BFME,
			bfmeTun_[i]->get().buffer()->cookie(),
			tncso_->get().buffer()->cookie(), 0,
			manager_->mfnrSizes_[2],
			manager_->yuvOutput1Size_, manager_->yuvOutput2Size_,
			manager_->mfnrSizes_[0], {}, true, 0, 1,
			bssOrder[i], i == 0, true, frameNumber);
		requests.push_back(std::move(request));
	}

	getImgSysMetaTuning(manager_->needCropTNC16x9_, requests);
}

MfnrTunSwmeTask::MfnrTunSwmeTask(MFNRFrames &mfnr,
				 Scheduler *scheduler,
				 const std::string &id,
				 MfnrTunManager *manager,
				 uint32_t internalRequestId)
	: Task(scheduler, id), manager_(manager),
	  internalRequestId_(internalRequestId)
{
	bssOrder_ = mfnr.bss_order;
	swmeFrames_ = mfnr.swmeFrames;
}

void MfnrTunSwmeTask::allocateBuffers()
{
	auto &in = swmeFrames_.in;
	auto &out = swmeFrames_.out;

	for (auto i = 0; i < kInputRawCount - 1; i++) {
		manager_->tnrciPool_.fetch(out.conf_map[i]);
		manager_->wrap2pPool_.fetch(out.warpping_map[i]);
		manager_->fourBytes_1_16_pool_.fetch(out.mcmv[i]);
		manager_->swmeOutPool_.fetch(out.paramOutInfo[i]);

		manager_->swmeParamPool_.fetch(in.paramInInfo[i]);
		manager_->swmeTuningPool_.fetch(in.tuningInfo[i]);
	}
}

void MfnrTunSwmeTask::run()
{
	allocateBuffers();

	auto &in = swmeFrames_.in;
	auto &out = swmeFrames_.out;

	std::vector<ipa::mtkisp7::SwmeFramesData> swmeFramesData;
	for (auto i = 0; i < kInputRawCount - 1; i++) {
		ipa::mtkisp7::SwmeFramesData data;
		data.base_buf = in.base_buf[i]->get().buffer()->cookie();
		data.ref_buf = in.ref_buf[i]->get().buffer()->cookie();
		data.bss_buf = in.bss_buf[i]->get().buffer()->cookie();
		data.paramInInfo = in.paramInInfo[i]->get().buffer()->cookie();
		data.tuningInfo = in.tuningInfo[i]->get().buffer()->cookie();

		data.conf_map = out.conf_map[i]->get().buffer()->cookie();
		data.warpping_map = out.warpping_map[i]->get().buffer()->cookie();
		data.mcmv = out.mcmv[i]->get().buffer()->cookie();
		data.paramOutInfo = out.paramOutInfo[i]->get().buffer()->cookie();

		swmeFramesData.push_back(std::move(data));
	}

	manager_->ipa_->doSwme(this, swmeFramesData);
}

void MfnrTunSwmeTask::notifySwmeResultReady()
{
	auto &out = swmeFrames_.out;
	for (auto i = 0; i < kInputRawCount - 1; i++) {
		//SWME Precheck
		if (MfnrTasksManager::mfnrPrecheck()) {
			bool hasVal = false;
			MINT32 *px = static_cast<MINT32 *>(reinterpret_cast<void *>(out.warpping_map[i]->get().address(0)));
			MINT32 *py = static_cast<MINT32 *>(reinterpret_cast<void *>(out.warpping_map[i]->get().address(1)));
			Size warppingMapSize = out.warpping_map[i]->get().size();
			int stride = out.warpping_map[i]->get().buffer()->planes()[0].stride;
			for (int h = 0; h < (int)warppingMapSize.height && !hasVal; h++) {
				for (int w = 0; w < (int)warppingMapSize.width; w++) {
					MUINT8 *x = reinterpret_cast<MUINT8 *>(px + w + h * stride);
					MUINT8 *y = reinterpret_cast<MUINT8 *>(py + w + h * stride);
					if (*x != 0 || *y != 0) {
						hasVal = true;
						break;
					}
				}
			}
			LOG(MtkISP7, Info) << "[CAT][MFNR] swme_out:" << hasVal;
		}
	}

	manager_->onDeviceTuner_->tuneSwme(internalRequestId_, swmeFrames_, bssOrder_->get());
	notifyDone();
}

MfnrTunDsTask::MfnrTunDsTask(MFNRFrames &mfnr,
			     uint32_t camSysMetaRequestId,
			     Scheduler *scheduler,
			     const std::string &id, Request *request, MfnrTunManager *manager,
			     uint32_t internalRequestId)
	: ImgSysTask(scheduler, id, camSysMetaRequestId, internalRequestId,
		     Feature::Capture_mfnr, manager->ipa_, request->controls()),
	  request_(request), manager_(manager)
{
	dsTun = mfnr.dsFrames.in.tunbufi;
	bssOrder_ = mfnr.bss_order;
}

void MfnrTunDsTask::run()
{
	ipa::mtkisp7::ImgMetaRequestData request;
	std::vector<ipa::mtkisp7::ImgMetaRequestData> requests;

	auto &bssOrder = bssOrder_->get();

	for (auto i = 0; i < (int)bssOrder.size() + 1; i++) {
		manager_->mfnrTun_.fetch(dsTun[i]);
	}

	for (auto i = 0; i < (int)bssOrder.size() + 1; i++) {
		int frameNumber = (i == 0 || i == 1) ? internalRequestId_ + bssOrder[0] : internalRequestId_ + bssOrder[i - 1];
		request = ipa::mtkisp7::ImgMetaRequestData(
			true, NSIspTuning::EStage_DS,
			dsTun[i]->get().buffer()->cookie(),
			0, 0, manager_->mfnrSizes_[0],
			manager_->yuvOutput1Size_, manager_->yuvOutput2Size_,
			manager_->mfnrSizes_[0], {}, true, 0, 1,
			frameNumber - internalRequestId_, (i == 0 || i == 1), true, frameNumber);
		requests.push_back(std::move(request));
	}

	getImgSysMetaTuning(manager_->needCropTNC16x9_, requests);
}

MfnrTunMcdsF1Task::MfnrTunMcdsF1Task(MFNRFrames &mfnr,
				     uint32_t camSysMetaRequestId,
				     Scheduler *scheduler,
				     const std::string &id, Request *request, MfnrTunManager *manager,
				     uint32_t internalRequestId)
	: ImgSysTask(scheduler, id, camSysMetaRequestId, internalRequestId,
		     Feature::Capture_mfnr, manager->ipa_, request->controls()),
	  request_(request), manager_(manager)
{
	mcdsF1Tun_.resize(kInputRawCount - 1);
	for (auto i = 0; i < kInputRawCount - 1; i++) {
		mcdsF1Tun_[i] = mfnr.mcdsF1Frames.in.tunbufi[i];
	}
	bssOrder_ = mfnr.bss_order;
}

void MfnrTunMcdsF1Task::run()
{
	ipa::mtkisp7::ImgMetaRequestData request;
	std::vector<ipa::mtkisp7::ImgMetaRequestData> requests;

	for (auto i = 0; i < kInputRawCount - 1; i++) {
		manager_->mfnrTun_.fetch(mcdsF1Tun_[i]);
		//fillTuning(mcdsF1Tun_[i], tuningBuffers.capture_MCDS_F1_tunbufi);
	}
	auto &bssOrder = bssOrder_->get();
	for (auto i = 0; i < kInputRawCount - 1; i++) {
		int frameNumber = internalRequestId_ + bssOrder[i + 1];
		request = ipa::mtkisp7::ImgMetaRequestData(
			true, NSIspTuning::EStage_MCDS_F1,
			mcdsF1Tun_[i]->get().buffer()->cookie(),
			0, 0, manager_->mfnrSizes_[0],
			manager_->yuvOutput1Size_, manager_->yuvOutput2Size_,
			manager_->mfnrSizes_[0], {}, true, 0, 1,
			bssOrder[i + 1], false, true, frameNumber);
		requests.push_back(std::move(request));
	}

	getImgSysMetaTuning(manager_->needCropTNC16x9_, requests);
}

MfnrTunDsVbiTask::MfnrTunDsVbiTask(MFNRFrames &mfnr,
				   uint32_t camSysMetaRequestId,
				   Scheduler *scheduler,
				   const std::string &id, Request *request, MfnrTunManager *manager,
				   uint32_t internalRequestId)
	: ImgSysTask(scheduler, id, camSysMetaRequestId, internalRequestId,
		     Feature::Capture_mfnr, manager->ipa_, request->controls()),
	  request_(request), manager_(manager)
{
	// 0 for BFBLD_BASE Task
	dsVbiV2Tun_ = mfnr.dsVbiFramesV2.in.tunbufi;
	dsVbiV5Tun_ = mfnr.dsVbiFramesV5.in.tunbufi;
	bssOrder_ = mfnr.bss_order;
}

void MfnrTunDsVbiTask::run()
{
	ipa::mtkisp7::ImgMetaRequestData request;
	std::vector<ipa::mtkisp7::ImgMetaRequestData> requests;

	auto bssOrder = bssOrder_->get();
	for (auto i = 0; i < (int)bssOrder.size() - 1; i++) {
		manager_->mfnrTun_.fetch(dsVbiV2Tun_[i]);
		manager_->mfnrTun_.fetch(dsVbiV5Tun_[i]);
	}

	//fillTuning(dsVbiV2Tun_, tuningBuffers.capture_DS_VBI_V2_tunbufi);
	//fillTuning(dsVbiV5Tun_, tuningBuffers.capture_DS_VBI_V5_tunbufi);
	for (auto i = 0; i < (int)bssOrder.size() - 1; i++) {
		int frameNumber = internalRequestId_ + bssOrder[i + 1];
		request = ipa::mtkisp7::ImgMetaRequestData(
			true, NSIspTuning::EStage_DS_VBI_V2,
			dsVbiV2Tun_[i]->get().buffer()->cookie(),
			0, 0, manager_->mfnrSizes_[1],
			manager_->yuvOutput1Size_, manager_->yuvOutput2Size_,
			manager_->mfnrSizes_[0], {}, true, 0, 1,
			bssOrder[i + 1], false, true, frameNumber);
		requests.push_back(std::move(request));

		request = ipa::mtkisp7::ImgMetaRequestData(
			true, NSIspTuning::EStage_DS_VBI_V5,
			dsVbiV5Tun_[i]->get().buffer()->cookie(),
			0, 0, manager_->mfnrSizes_[4],
			manager_->yuvOutput1Size_, manager_->yuvOutput2Size_,
			manager_->mfnrSizes_[0], {}, true, 0, 1,
			bssOrder[i + 1], false, true, frameNumber);
		requests.push_back(std::move(request));
	}

	getImgSysMetaTuning(manager_->needCropTNC16x9_, requests);
}

MfnrTunMsbldTask::MfnrTunMsbldTask(MFNRFrames &mfnr,
				   uint32_t camSysMetaRequestId,
				   Scheduler *scheduler,
				   const std::string &id, Request *request, MfnrTunManager *manager,
				   uint32_t internalRequestId, int msbldIdx)
	: ImgSysTask(scheduler, id, camSysMetaRequestId, internalRequestId,
		     Feature::Capture_mfnr, manager->ipa_, request->controls()),
	  request_(request), manager_(manager), msbldIdx_(msbldIdx)
{
	msbldF0Tun_ = mfnr.msbldFrames[msbldIdx].msbldF0.in.tunbufi;
	msbldF1Tun_ = mfnr.msbldFrames[msbldIdx].msbldF1.in.tunbufi;
	msbldF2Tun_ = mfnr.msbldFrames[msbldIdx].msbldF2.in.tunbufi;
	msbldF3Tun_ = mfnr.msbldFrames[msbldIdx].msbldF3.in.tunbufi;
	msbldF4Tun_ = mfnr.msbldFrames[msbldIdx].msbldF4.in.tunbufi;
	msbldF5Tun_ = mfnr.msbldFrames[msbldIdx].msbldF5.in.tunbufi;
	msbldF6Tun_ = mfnr.msbldFrames[msbldIdx].msbldF6.in.tunbufi;

	bssOrder_ = mfnr.bss_order;
}

void MfnrTunMsbldTask::run()
{
	ipa::mtkisp7::ImgMetaRequestData request;
	std::vector<ipa::mtkisp7::ImgMetaRequestData> requests;
	manager_->mfnrTun_.fetch(msbldF0Tun_);
	manager_->mfnrTun_.fetch(msbldF1Tun_);
	manager_->mfnrTun_.fetch(msbldF2Tun_);
	manager_->mfnrTun_.fetch(msbldF3Tun_);
	manager_->mfnrTun_.fetch(msbldF4Tun_);
	manager_->mfnrTun_.fetch(msbldF5Tun_);
	manager_->mfnrTun_.fetch(msbldF6Tun_);

	std::map<EStage_T, SharedMailBox<InfoFrame>, std::greater<EStage_T>> stageToTuningMap = {
		{ NSIspTuning::EStage_MSBLD_F0, msbldF0Tun_ },
		{ NSIspTuning::EStage_MSBLD_F1, msbldF1Tun_ },
		{ NSIspTuning::EStage_MSBLD_F2, msbldF2Tun_ },
		{ NSIspTuning::EStage_MSBLD_F3, msbldF3Tun_ },
		{ NSIspTuning::EStage_MSBLD_F4, msbldF4Tun_ },
		{ NSIspTuning::EStage_MSBLD_F5, msbldF5Tun_ },
		{ NSIspTuning::EStage_MSBLD_F6, msbldF6Tun_ }
	};

	auto bssOrder = bssOrder_->get();
	for (auto it = stageToTuningMap.begin(); it != stageToTuningMap.end(); it++) {
		int size_idx = it->first - EStage_MSBLD_F0;
		int frameNumber = internalRequestId_ + bssOrder[msbldIdx_];
		request = ipa::mtkisp7::ImgMetaRequestData(
			true, it->first,
			it->second->get().buffer()->cookie(),
			0, 0, manager_->mfnrSizes_[size_idx],
			manager_->yuvOutput1Size_, manager_->yuvOutput2Size_,
			manager_->mfnrSizes_[0], {}, true, msbldIdx_,
			kInputRawCount,
			bssOrder[msbldIdx_], msbldIdx_ == 0,
			true, frameNumber);
		requests.push_back(std::move(request));
	}

	getImgSysMetaTuning(manager_->needCropTNC16x9_, requests);
}

MfnrTunAfbldTask::MfnrTunAfbldTask(MFNRFrames &mfnr,
				   uint32_t camSysMetaRequestId,
				   Scheduler *scheduler,
				   const std::string &id, Request *request, MfnrTunManager *manager,
				   uint32_t internalRequestId)
	: ImgSysTask(scheduler, id, camSysMetaRequestId, internalRequestId,
		     Feature::Capture_mfnr, manager->ipa_, request->controls()),
	  request_(request), manager_(manager)
{
	afbldF0Tun_ = mfnr.afbldF0.in.tunbufi[0];
	afbldF1Tun_ = mfnr.afbldF1.in.tunbufi[0];
	afbldF2Tun_ = mfnr.afbldF2.in.tunbufi[0];
	afbldF3Tun_ = mfnr.afbldF3.in.tunbufi[0];
	afbldF4Tun_ = mfnr.afbldF4.in.tunbufi[0];
	afbldF5Tun_ = mfnr.afbldF5.in.tunbufi[0];
	afbldF6Tun_ = mfnr.afbldF6.in.tunbufi[0];
	bssOrder_ = mfnr.bss_order;
	tncso_ = mfnr.afbldF0.tncso;
}

void MfnrTunAfbldTask::run()
{
	ipa::mtkisp7::ImgMetaRequestData request;
	std::vector<ipa::mtkisp7::ImgMetaRequestData> requests;

	manager_->mfnrTun_.fetch(afbldF0Tun_);
	manager_->mfnrTun_.fetch(afbldF1Tun_);
	manager_->mfnrTun_.fetch(afbldF2Tun_);
	manager_->mfnrTun_.fetch(afbldF3Tun_);
	manager_->mfnrTun_.fetch(afbldF4Tun_);
	manager_->mfnrTun_.fetch(afbldF5Tun_);
	manager_->mfnrTun_.fetch(afbldF6Tun_);

	std::map<EStage_T, SharedMailBox<InfoFrame>, std::greater<EStage_T>> stageToTuningMap = {
		{ NSIspTuning::EStage_AFBLD_F6, afbldF6Tun_ },
		{ NSIspTuning::EStage_AFBLD_F5, afbldF5Tun_ },
		{ NSIspTuning::EStage_AFBLD_F4, afbldF4Tun_ },
		{ NSIspTuning::EStage_AFBLD_F3, afbldF3Tun_ },
		{ NSIspTuning::EStage_AFBLD_F2, afbldF2Tun_ },
		{ NSIspTuning::EStage_AFBLD_F1, afbldF1Tun_ },
		{ NSIspTuning::EStage_AFBLD_F0, afbldF0Tun_ }
	};
	auto bssOrder = bssOrder_->get();
	for (auto it = stageToTuningMap.begin(); it != stageToTuningMap.end(); it++) {
		int size_idx = it->first - EStage_AFBLD_F0;
		int frameNumber = internalRequestId_ + bssOrder[2];

		if (it->first == NSIspTuning::EStage_AFBLD_F0) {
			request = ipa::mtkisp7::ImgMetaRequestData(
				true, it->first,
				it->second->get().buffer()->cookie(),
				tncso_->get().buffer()->cookie(), 0,
				manager_->mfnrSizes_[size_idx],
				manager_->yuvOutput1Size_,
				manager_->yuvOutput2Size_,
				manager_->mfnrSizes_[0], {}, true,
				kInputRawCount - 2, kInputRawCount,
				bssOrder[2], false,
				true, frameNumber);
		} else {
			request = ipa::mtkisp7::ImgMetaRequestData(
				true, it->first,
				it->second->get().buffer()->cookie(),
				0, 0, manager_->mfnrSizes_[size_idx],
				manager_->yuvOutput1Size_,
				manager_->yuvOutput2Size_,
				manager_->mfnrSizes_[0], {}, true,
				kInputRawCount - 2, kInputRawCount,
				bssOrder[2], false,
				true, frameNumber);
		}

		requests.push_back(std::move(request));
	}

	getImgSysMetaTuning(manager_->needCropTNC16x9_, requests);
}

} // namespace libcamera
/* namespace libcamera */

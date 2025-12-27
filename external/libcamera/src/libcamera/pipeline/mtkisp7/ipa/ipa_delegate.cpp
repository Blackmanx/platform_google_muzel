/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Google Inc.
 *
 * ipa_delegate.cpp - IPA Delegate to handle signals and callbacks.
 */

#include "ipa_delegate.h"

#include <cstdint>

#include "../face_detect/detector.h"
#include "../hal3a/aaa.h"
#include "../halisp/mfnr_tun.h"
#include "pipeline/mtkisp7/halisp/imgsys_task.h"

#include "mtkisp7_ipa_interface.h"

namespace libcamera {
LOG_DEFINE_CATEGORY(IPADelegateMtkISP7)

// Don't disconnect to avoid issues of BoundMethod.
IPADelegate::IPADelegate() = default;

int IPADelegate::init(std::unique_ptr<ipa::mtkisp7::IPAProxyMtkISP7> ipaProxy,
		      const std::string &model, const int32_t sensorIdx,
		      const std::vector<uint8_t> &eeprom,
		      const std::vector<ipa::mtkisp7::CamSysData> &camSysDataArray)
{
	ipaProxy_ = std::move(ipaProxy);
	if (!ipaProxy_)
		return -ENOENT;

	ipaProxy_->AieParseResultReady.connect(this,
					       &IPADelegate::AieParseResultReady);

	ipaProxy_->AAAResultReady.connect(this, &IPADelegate::AAAResultReady);

	ipaProxy_->ImgSysMetaTuningDone.connect(this, &IPADelegate::ImgSysMetaTuningDone);

	ipaProxy_->BssResultReady.connect(this, &IPADelegate::BssResultReady);
	ipaProxy_->SwmeResultReady.connect(this, &IPADelegate::SwmeResultReady);

	int ret = ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::init,
					  ConnectionTypeBlocking, model, sensorIdx,
					  eeprom, camSysDataArray);

	return ret;
}

void IPADelegate::releaseProxy()
{
	ipaProxy_.reset();
}

void IPADelegate::start(const uint32_t rawMetaBufferId,
			ipa::mtkisp7::SensorSetting *sensorSetting,
			int32_t *lens_position)
{
	return ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::start,
				       ConnectionTypeBlocking, rawMetaBufferId,
				       sensorSetting, lens_position);
}

void IPADelegate::stop()
{
	return ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::stop,
				       ConnectionTypeBlocking);
}

int IPADelegate::configure(
	const Size &camsysYuvSize, FaceDetector *faceDetector,
	const Size &maxVideoSize,
	const Size &maxStillSize, const std::string &sensorId,
	const uint32_t camsysIndex, const int32_t sessionTimestamp,
	bool isVideo, const Size &sensorFullSize,
	const Size &swmeAlignedSize,
	Size *wrappingMapSize,
	Size *confMapSize)
{
	faceDetector_ = faceDetector;

	return ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::configure,
				       ConnectionTypeBlocking,
				       camsysYuvSize, maxVideoSize, maxStillSize,
				       sensorId, camsysIndex, sessionTimestamp,
				       isVideo, sensorFullSize, swmeAlignedSize,
				       wrappingMapSize, confMapSize);
}

void IPADelegate::mapBuffers(const std::vector<IPABuffer> &buffers)
{
	return ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::mapBuffers,
				       ConnectionTypeBlocking, buffers);
}

void IPADelegate::unmapBuffers(const std::vector<unsigned int> &ids)
{
	return ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::unmapBuffers,
				       ConnectionTypeBlocking, ids);
}

void IPADelegate::writeStillCaptureDebugMetadata(
	const uint32_t camSysMetaRequestId,
	ControlList *metadata)
{
	ipaProxy_->invokeMethod(
		&ipa::mtkisp7::IPAProxyMtkISP7::writeStillCaptureDebugMetadata,
		ConnectionTypeBlocking,
		camSysMetaRequestId, metadata);
}

void IPADelegate::notifyRequestBegin(const uint32_t baseFrame,
				     const uint32_t curFrame,
				     const bool hasStillCapture)
{
	ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::notifyRequestBegin,
				ConnectionTypeBlocking, baseFrame, curFrame, hasStillCapture);
}

void IPADelegate::notifyRequestEnd(const uint32_t frame)
{
	ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::notifyRequestEnd,
				ConnectionTypeBlocking, frame);
}

void IPADelegate::notifyExportBegin(
	const uint32_t exportBegin,
	const uint32_t exportEnd)
{
	ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::notifyExportBegin,
				ConnectionTypeBlocking, exportBegin, exportEnd);
}

void IPADelegate::notifyImportBegin(
	const uint32_t importBegin,
	const uint32_t importEnd)
{
	ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::notifyImportBegin,
				ConnectionTypeBlocking, importBegin, importEnd);
}

void IPADelegate::aieParse(
	const uint32_t inputImageBufferId,
	const uint32_t faceDetectionMetadataBufferId,
	const uint32_t faceToneClassificationMetadataBufferId,
	const Size &currentSensorSize,
	const uint32_t camSysMetaRequestId)
{
	ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::aieParse,
				ConnectionTypeBlocking, inputImageBufferId,
				faceDetectionMetadataBufferId,
				faceToneClassificationMetadataBufferId,
				currentSensorSize, camSysMetaRequestId);
}

void IPADelegate::doCalculation3A(
	AAATask *aaaTask,
	const uint32_t frame,
	const uint32_t stat0BufferId, const uint32_t stat1BufferId,
	const uint64_t timestamp, const uint32_t camSysMetaRequestId,
	const uint32_t afCamSysMetaRequestId,
	const bool isStillCapture, const uint32_t rawMetaBufferId,
	const ipa::mtkisp7::GyroSampleData &gyroSample,
	const uint32_t internalRequestIdApplied,
	std::optional<Feature> featureApplied,
	const ipa::mtkisp7::VcmFocusInformation &vcmFocusInfo,
	const ControlList &controls)
{
	aaaTasks_.emplace(frame, aaaTask);

	int32_t featureEnum = -1;
	if (featureApplied.has_value())
		featureEnum = static_cast<int32_t>(featureApplied.value());

	ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::doCalculation3A,
				ConnectionTypeBlocking,
				frame, stat0BufferId, stat1BufferId,
				timestamp, camSysMetaRequestId,
				afCamSysMetaRequestId, isStillCapture,
				rawMetaBufferId, gyroSample,
				internalRequestIdApplied, featureEnum, vcmFocusInfo,
				controls);
}

void IPADelegate::getImgSysMetaTuning(
	ImgSysTask *imgSysTask,
	const uint32_t camSysMetaRequestId,
	const uint32_t frame,
	const bool needCropTNC16x9,
	const Feature feature,
	const std::vector<ipa::mtkisp7::ImgMetaRequestData> &imgMetaRequests,
	const ControlList &controls)
{
	uint64_t cookie = imgSysCookieCounter_++;
	imgSysTasks_.emplace(cookie, imgSysTask);

	ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::getImgSysMetaTuning,
				ConnectionTypeBlocking,
				cookie, camSysMetaRequestId, frame,
				needCropTNC16x9, static_cast<uint32_t>(feature),
				imgMetaRequests, controls);
}

void IPADelegate::doBss(MfnrTunBssTask *mfnrTunBssTask,
			const ipa::mtkisp7::BssFramesData &bssFramesData,
			const uint32_t internalRequestId)
{
	uint64_t cookie = bssCookieCounter_++;
	bssTasks_.emplace(cookie, mfnrTunBssTask);

	ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::doBss,
				ConnectionTypeBlocking,
				cookie, bssFramesData,
				internalRequestId);
}

void IPADelegate::doSwme(
	MfnrTunSwmeTask *swmeTask,
	const std::vector<ipa::mtkisp7::SwmeFramesData> &swmeFramesData)
{
	uint64_t cookie = swmeCookieCounter_++;
	swmeTasks_.emplace(cookie, swmeTask);

	ipaProxy_->invokeMethod(&ipa::mtkisp7::IPAProxyMtkISP7::doSwme,
				ConnectionTypeBlocking, cookie, swmeFramesData);
}

void IPADelegate::AieParseResultReady(
	bool success,
	const ipa::mtkisp7::PrimaryFaceData &primaryFace,
	const ControlList &faceControls)
{
	faceDetector_->AieParseResultReady(success, primaryFace, faceControls);
}

void IPADelegate::AAAResultReady(uint32_t id,
				 const ipa::mtkisp7::SensorSetting &sensorSetting,
				 const ipa::mtkisp7::AaaIspExchange &aaaIspExchange,
				 const ipa::mtkisp7::LensPositionInfo &lensPositionInfo)
{
	auto it = aaaTasks_.find(id);
	if (it == aaaTasks_.end()) {
		LOG(IPADelegateMtkISP7, Fatal)
			<< "AAResultReady: couldn't find task with id: " << id;
		return;
	}
	it->second->AAAResultReady(sensorSetting, aaaIspExchange, lensPositionInfo);

	aaaTasks_.erase(it);
}

void IPADelegate::ImgSysMetaTuningDone(uint64_t cookie)
{
	auto it = imgSysTasks_.find(cookie);
	if (it == imgSysTasks_.end()) {
		LOG(IPADelegateMtkISP7, Fatal)
			<< "ImgSysMetaTuningDone: couldn't find task with cookie: "
			<< cookie;
		return;
	}
	it->second->notifyDone();

	imgSysTasks_.erase(it);
}

void IPADelegate::BssResultReady(uint64_t cookie, const std::vector<int32_t> &bssOrder)
{
	auto it = bssTasks_.find(cookie);
	if (it == bssTasks_.end()) {
		LOG(IPADelegateMtkISP7, Fatal)
			<< "BssResult: couldn't find task with cookie"
			<< cookie;
		return;
	}
	it->second->notifyBssResult(bssOrder);

	bssTasks_.erase(it);
}

void IPADelegate::SwmeResultReady(uint64_t cookie)
{
	auto it = swmeTasks_.find(cookie);
	if (it == swmeTasks_.end()) {
		LOG(IPADelegateMtkISP7, Fatal)
			<< "SwmeResultReady: couldn't find task with cookie: "
			<< cookie;
		return;
	}
	it->second->notifySwmeResultReady();

	swmeTasks_.erase(it);
}

} // namespace libcamera

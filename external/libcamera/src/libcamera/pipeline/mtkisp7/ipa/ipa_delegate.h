/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Google Inc.
 *
 * ipa_delegate.h - IPA Delegate to handle signals and callbacks.
 */
#pragma once

#include <libcamera/ipa/mtkisp7_ipa_proxy.h>

#include "libcamera/base/object.h"
#include "pipeline/mtkisp7/face_detect/detector.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/feature.h"

#include "mtkisp7_ipa_interface.h"

namespace libcamera {

class AAATask;
class MfnrTunSwmeTask;
class MtkISP7CameraData;
class ImgSysTask;
class MfnrTunBssTask;

class IPADelegate : public Object
{
public:
	IPADelegate();

	int init(std::unique_ptr<ipa::mtkisp7::IPAProxyMtkISP7> ipaProxy,
		 const std::string &model, const int32_t sensorIdx,
		 const std::vector<uint8_t> &eeprom,
		 const std::vector<ipa::mtkisp7::CamSysData> &camSysDataArray);
	void releaseProxy();
	bool isValid() { return ipaProxy_.get(); }

	void start(const uint32_t rawMetaBufferId,
		   ipa::mtkisp7::SensorSetting *sensorSetting,
		   int32_t *lens_position);
	void stop();

	int configure(const Size &camsysYuvSize, FaceDetector *faceDetector,
		      const Size &maxVideoSize,
		      const Size &maxStillSize, const std::string &sensorId,
		      const uint32_t camsysIndex, const int32_t sessionTimestamp,
		      bool isVideo, const Size &sensorFullSize,
		      const Size &swmeAlignedSize,
		      Size *wrappingMapSize,
		      Size *confMapSize);

	void mapBuffers(const std::vector<IPABuffer> &buffers);
	void unmapBuffers(const std::vector<unsigned int> &ids);

	void writeStillCaptureDebugMetadata(
		const uint32_t camSysMetaRequestId,
		ControlList *metadata);

	void notifyRequestBegin(const uint32_t baseFrame,
				const uint32_t curFrame,
				const bool hasStillCapture);
	void notifyRequestEnd(const uint32_t frame);

	void notifyExportBegin(
		const uint32_t exportBegin,
		const uint32_t exportEnd);
	void notifyImportBegin(
		const uint32_t importBegin,
		const uint32_t importEnd);

	void aieParse(
		const uint32_t inputImageBufferId,
		const uint32_t faceDetectionMetadataBufferId,
		const uint32_t faceToneClassificationMetadataBufferId,
		const Size &currentSensorSize,
		const uint32_t camSysMetaRequestId);

	void doCalculation3A(
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
		const ControlList &controls);

	void getImgSysMetaTuning(
		ImgSysTask *imgSysTask,
		const uint32_t camSysMetaRequestId,
		const uint32_t frame,
		const bool needCropTNC16x9,
		const Feature feature,
		const std::vector<ipa::mtkisp7::ImgMetaRequestData> &imgMetaRequests,
		const ControlList &controls);

	void doBss(MfnrTunBssTask *mfnrTunbssTask,
		   const ipa::mtkisp7::BssFramesData &bssFramesData,
		   const uint32_t internalRequestId);
	void doSwme(
		MfnrTunSwmeTask *swmeTask,
		const std::vector<ipa::mtkisp7::SwmeFramesData> &swmeFramesData);

private:
	friend MtkISP7CameraData;

	void AieParseResultReady(
		bool success,
		const ipa::mtkisp7::PrimaryFaceData &primaryFace,
		const ControlList &faceControls);

	void AAAResultReady(uint32_t id,
			    const ipa::mtkisp7::SensorSetting &sensorSetting,
			    const ipa::mtkisp7::AaaIspExchange &aaaIspExchange,
			    const ipa::mtkisp7::LensPositionInfo &lensPositionInfo);
	void AFResultReady(uint32_t id, int32_t position);

	void ImgSysMetaTuningDone(uint64_t cookie);

	void BssResultReady(uint64_t cookie, const std::vector<int32_t> &bssOrder);
	void SwmeResultReady(uint64_t cookie);

	std::unique_ptr<ipa::mtkisp7::IPAProxyMtkISP7> ipaProxy_;

	FaceDetector *faceDetector_;

	std::unordered_map<uint32_t, AAATask *> aaaTasks_;

	uint64_t imgSysCookieCounter_ = 1;
	std::unordered_map<uint64_t, ImgSysTask *> imgSysTasks_;

	uint64_t bssCookieCounter_ = 1;
	std::unordered_map<uint64_t, MfnrTunBssTask *> bssTasks_;

	uint64_t swmeCookieCounter_ = 1;
	std::unordered_map<uint64_t, MfnrTunSwmeTask *> swmeTasks_;
};

} // namespace libcamera

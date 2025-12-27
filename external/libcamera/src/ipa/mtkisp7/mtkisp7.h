/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Google Inc.
 *
 * mtkisp7.h - IPA implementation for MtkISP7
 */
#pragma once

#include "libcamera/internal/gyro_sensor.h"
#include "libcamera/internal/mapped_framebuffer.h"

#include "face_detect/parser.h"
#include "hal3a/hal_3a.h"
#include "halisp/bss.h"
#include "halisp/hal_isp.h"
#include "halisp/swme.h"
#include "libcamera/base/thread.h"
#include "libcamera/controls.h"
#include "libfdft_lib/faces.h"
#include "mtkcam-interfaces/utils/hw/faces.h"
#include "peripheraldriver/lens/vcm_drv.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"

#include "mtkisp7_ipa_interface.h"

namespace libcamera {

using namespace std::literals::chrono_literals;

namespace ipa::mtkisp7 {

class IPAMtkISP7 : public IPAMtkISP7Interface, public Object
{
public:
	IPAMtkISP7();
	~IPAMtkISP7();

	int init(const std::string &model, const int32_t sensorIdx,
		 const std::vector<uint8_t> &eeprom,
		 const std::vector<ipa::mtkisp7::CamSysData> &camSysDataArray)
		override;

	void start(const uint32_t rawMetaBufferId,
		   SensorSetting *sensorSetting,
		   int32_t *lens_position) override;
	void stop() override;

	int configure(const Size &camsysYuvSize, const Size &maxVideoSize,
		      const Size &maxStillSize, const std::string &sensorId,
		      const uint32_t camsysIndex, const int32_t sessionTimestamp,
		      bool isVideo, const Size &sensorFullSize,
		      const Size &swmeAlignedSize,
		      Size *wrappingMapSize, Size *confMapSize) override;

	void mapBuffers(const std::vector<IPABuffer> &buffers) override;
	void unmapBuffers(const std::vector<unsigned int> &ids) override;

	void writeStillCaptureDebugMetadata(
		const uint32_t camSysMetaRequestId,
		ControlList *metadata) override;

	void notifyRequestBegin(const uint32_t baseFrame,
				const uint32_t curFrame,
				const bool hasStillCapture) override;
	void notifyRequestEnd(const uint32_t frame) override;

	void notifyExportBegin(
		const uint32_t exportBegin,
		const uint32_t exportEnd) override;
	void notifyImportBegin(
		const uint32_t importBegin,
		const uint32_t importEnd) override;

	void aieParse(
		const uint32_t inputImageBufferId,
		const uint32_t faceDetectionMetadataBufferId,
		const uint32_t faceToneClassificationMetadataBufferId,
		const Size &currentSensorSize,
		const uint32_t camSysMetaRequestId) override;

	void doCalculation3A(
		const uint32_t frame,
		const uint32_t stat0BufferId, const uint32_t stat1BufferId,
		const uint64_t timestamp, const uint32_t camSysMetaRequestId,
		const uint32_t afCamSysMetaRequestId,
		const bool isStillCapture, const uint32_t rawMetaBufferId,
		const GyroSampleData &gyroSample,
		const uint32_t internalRequestIdApplied,
		const int32_t featureEnum,
		const VcmFocusInformation &vcmFocusInfo,
		const ControlList &controls) override;

	void getImgSysMetaTuning(
		const uint64_t cookie,
		const uint32_t camSysMetaRequestId,
		const uint32_t frame,
		const bool needCropTNC16x9,
		const uint32_t featureEnum,
		const std::vector<ipa::mtkisp7::ImgMetaRequestData> &imgMetaRequests,
		const ControlList &controls) override;

	void doBss(const uint64_t cookie,
		   const BssFramesData &bssFramesData,
		   const uint32_t internalRequestId) override;
	void doSwme(
		const uint64_t cookie,
		const std::vector<ipa::mtkisp7::SwmeFramesData> &swmeFramesData) override;

private:
	void doAAAResultReady(uint32_t frame, SensorSetting sensorSetting,
			      const AaaIspExchange &aaaIspExchange,
			      LensPositionInfo lensPositionInfo);

	void doImgSysMetaTuningDone(uint64_t taskCounter);

	void adjustRLimit();

	struct IPAMappedBuffer {
		IPAMappedBuffer(const std::vector<FrameBuffer::Plane> &planes)
			: buffer(planes) {}

		FrameBuffer buffer;
		std::unique_ptr<MappedFrameBuffer> mapped;
	};

	class AAAManager : public Object
	{
	public:
		AAAManager(IPAMtkISP7 *ipa);
		void doCalculation(FrameBuffer *statistics0, FrameBuffer *statistics1,
				   uint64_t timestamp,
				   uint32_t internalRequestId,
				   uint32_t camSysMetaRequestId,
				   const uint32_t afCamSysMetaRequestId,
				   bool isStillCapture, int rawMetaFd,
				   unsigned char *rawMetaBuffer,
				   ::VcmFocusInformation vcmFocusInfo,
				   std::optional<MtkCameraFaceMetadata> metadata,
				   GyroSensor::SensorSample gyroSample,
				   const uint32_t internalRequestIdApplied,
				   const ControlList &controls,
				   const int32_t featureEnum);

	private:
		IPAMtkISP7 *ipa_;
	};

	class IspManager : public Object
	{
	public:
		struct DataMappedBuffers {
			bool valid;

			IPAMappedBuffer *tuning;
			IPAMappedBuffer *statistics;
			IPAMappedBuffer *swHist;
			std::vector<IPAMappedBuffer *> reserved;
		};

		IspManager(IPAMtkISP7 *ipa);
		void getImgSysMetaTuning(
			const uint64_t cookie,
			const uint32_t camSysMetaRequestId,
			const uint32_t frame,
			const bool needCropTNC16x9,
			const Feature feature,
			const std::vector<ipa::mtkisp7::ImgMetaRequestData> imgMetaRequests,
			const std::vector<DataMappedBuffers> dataMappedBuffersList,
			const ControlList &controls);

	private:
		IPAMtkISP7 *ipa_;
	};

	friend AAAManager;
	friend IspManager;

	IPAMappedBuffer *getMappedBufferIter(unsigned int bufferId);

	ControlList convertFaceMetadata();

	OnDeviceTuner onDeviceTuner_;

	std::map<unsigned int, IPAMappedBuffer> buffers_;

	std::optional<MtkCameraFaceMetadata> latestFaceMetadata_;

	// TODO: Check if we create a different instance for each CameraData.
	std::unique_ptr<Hal3A> hal3A_;
	std::unique_ptr<HalIsp> halIsp_;
	std::unique_ptr<AieParser> aieParser_;

	Thread aaaThread_;
	std::unique_ptr<AAAManager, decltype(&Object::Deleter)> aaaManager_;

	Thread ispThread_;
	std::unique_ptr<IspManager, decltype(&Object::Deleter)> ispManager_;

	// The sensor being configured.
	int32_t sensorIdx_;

	std::shared_ptr<BssWrapper> bssWrapper_;

	Size sensorFullSize_;
	Size swmeAlignedSize_;
	std::vector<std::shared_ptr<SwmeWrapper>> swmeWrapper_;
	std::map<int, int> mfnrExifData_;

	std::vector<uint8_t> swmeWorkbuf_;
};

} // namespace ipa::mtkisp7
} // namespace libcamera

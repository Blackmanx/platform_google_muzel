/*
 * Copyright (C) 2023, Google Inc.
 *
 * hal_3a.h - Wrapper of MtkISP7 mtk::hal3a::IHal3A
 */

#pragma once

#include <cstddef>

#include <libcamera/ipa/mtkisp7_ipa_interface.h>

#include "libcamera/internal/gyro_sensor.h"

#include "libcamera/base/mutex.h"
#include "libcamera/controls.h"
#include "libcamera/framebuffer.h"
#include "libcamera/geometry.h"
#include "libipa/camera_sensor_helper.h"
#include "mtkcam-core/include/mtkcam-core/aaahal/aaa_hal/IHal3A.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"
#include "platform/mtkisp7/utils/history.h"
#include "sensor/sensor_info.h"

class SensorInfo;

namespace libcamera {

class HalIsp;

class Hal3A
{
public:
	Hal3A(const uint32_t sensor_idx, HalIsp *halIsp, OnDeviceTuner *odt);
	~Hal3A();

	void configure(Size camsysYuvSize, Size maxVideoSize, bool isVideo, bool force3AConsistency);
	void start(mtk_cam_uapi_meta_raw_stats_cfg *rawMetaBuffer);

	void doCalculation(FrameBuffer *statistics0, uint64_t timestamp,
			   uint32_t internalRequestId, uint32_t camSysMetaRequestId,
			   bool isStillCapture, unsigned char *rawMetaBuffer,
			   std::optional<MtkCameraFaceMetadata> metadata,
			   GyroSensor::SensorSample gyroSample,
			   ipa::mtkisp7::SensorSetting *exposureAndGain,
			   ipa::mtkisp7::AaaIspExchange *aaaIspExchange,
			   std::optional<uint32_t> internalRequestIdApplied,
			   std::optional<Feature> featureApplied,
			   ipa::mtkisp7::LensPositionInfo *lensPositionInfo,
			   ControlList controls);

	void doCalculationAF(FrameBuffer *statistics1, uint64_t timestamp,
			     uint32_t internalRequestId, uint32_t camSysMetaRequestId,
			     VcmFocusInformation vcmFocusInfo,
			     std::optional<MtkCameraFaceMetadata> metadata,
			     GyroSensor::SensorSample gyroSample, int32_t *position,
			     ControlList controls);

	void getExposureAndGain(ipa::mtkisp7::SensorSetting *exposureAndGain,
				uint32_t &exposureTimeMs);

	mtk::hal3a::v1_0::mtk_3a_result r3AResult_ = {};

	History<mtk::hal3a::v1_0::mtk_3a_result> resultHistory_;

private:
	void init();
	void getInitialInfo();
	void config();
	void startInternal();

	mtk::hal3a::v1_0::mtk_3a_param get3AParam(uint32_t internalRequestId,
						  std::optional<MtkCameraFaceMetadata> metadata,
						  GyroSensor::SensorSample gyroSample,
						  bool isStillCapture,
						  std::optional<ControlList> controls_opt);

	uint32_t convertGain(uint32_t aeGain);

	const uint32_t sensor_idx_;
	int sensor_id_;
	uint32_t sensor_dev_;
	Size camsysYuvSize_;
	Size maxVideoSize_;
	bool isVideo_ = false;

	mtk::hal3a::IHal3A *m_hal3a_ = nullptr;
	HalIsp *halIsp_ = nullptr;
	bool inited_ = false;
	bool force3AConsistency_ = false;

	mtk::hal3a::v1_0::mtk_hw_initial_setting initialSetting_ = {};

	OnDeviceTuner *onDeviceTuner_;
	std::shared_ptr<SensorInfo> sensor_info_ = nullptr;

	std::unique_ptr<ipa::CameraSensorHelper> camHelper_;
};

} /* namespace libcamera */

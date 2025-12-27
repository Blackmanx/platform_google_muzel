/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Google Inc.
 *
 * aie.h - MtkISP7 AI Engine device
 */

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>

#include "libcamera/internal/dma_heaps.h"
#include "libcamera/internal/media_device.h"
#include "libcamera/internal/v4l2_videodevice.h"

#include "mtkcam-core/hw/aie/3.1/hardware/v4l2/cam_fdvt_v4l2.h"

namespace libcamera {

class AieDevice : public Object
{
public:
	AieDevice();

	int configure();
	int init(MediaDevice *media);
	int start();
	int stop();

	FdDrv_input_struct createFaceDetectionDriverConfig();
	FdDrv_input_struct createFaceToneClassificationDriverConfig();

	int configureStreams();
	FdDrv_input_struct createDefaultDriverConfig();
	int releaseBuffers();
	int requestBuffers();

	const Size inputSize_;
	const unsigned int bufferNum_;

	std::unique_ptr<V4L2VideoDevice> sourceVideo_;
	std::unique_ptr<V4L2VideoDevice> resultMeta_;

	MediaDevice *media_;

	FdDrv_init_struct driverInitConfig_;

	uint32_t initControlId_;
	uint32_t inferenceParamControlId_;
};

} /* namespace libcamera */

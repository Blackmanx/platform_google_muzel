/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Google Inc.
 *
 * aie_parser.h - AieParser that calls MTKDetection interface
 */
#pragma once

#include "libcamera/internal/mapped_framebuffer.h"

#include "libcamera/framebuffer.h"
#include "libcamera/geometry.h"
#include "libfdft_lib/MTKDetection.h"
#include "libfdft_lib/faces.h"
#include "mtkcam-core/hw/aie/3.1/hardware/v4l2/cam_fdvt_v4l2.h"

#include "mtkisp7_ipa_interface.h"

namespace libcamera {
namespace ipa::mtkisp7 {

class AieParser
{
public:
	MUINT8 *getWorkingBuffer();

	int initialize();

	std::unique_ptr<MTKDetection> algoInterface_;

	// Face detection buffers
	int16_t rawFaceToneResult_[MAX_CROP_NUM][MAX_AIE2_ATT_LEN];
	uint16_t parserBuffers_[MAX_CROP_NUM][8];
	uint16_t *parserBufferList_[MAX_CROP_NUM];
	unsigned char parserTaskList_[MAX_AIE_ATTR_TYPE][MAX_CROP_NUM];
	unsigned char parserBufferStatus_[MAX_AIE_ATTR_TYPE][MAX_CROP_NUM];
	int patchSize_[MAX_CROP_NUM];
	int parserAttributeTask_[MAX_AIE_ATTR_TYPE] = { 0 };
	result workingBuffer_[MAX_FACE_NUM];
};

class AieParseTask
{
public:
	AieParseTask(std::shared_ptr<AieParser> parser,
		     FrameBuffer *inputImage,
		     FrameBuffer *faceDetectionMetadata,
		     FrameBuffer *faceToneClassificationMetadata,
		     const Size &currentSensorSize, int camSysMetaRequestId);

	bool run();

	ipa::mtkisp7::PrimaryFaceData primaryFace_;
	MtkCameraFaceMetadata detectionResult_;

private:
	FdOptions createBufferOptions() const;
	void init();
	int prepareBuffer();

	void updateFaceToneDriverConfig();

	int parseAll();
	int parseFaceDetectionOutput();
	void parseFaceLandmark(
		FDRESULT *resultSet, int calibrationIndex, int resultSetIndex);
	void parseFaceRoi(
		FDRESULT *resultSet, int calibrationIndex, int resultSetIndex);
	int parseFaceToneClassificationOutput();

	void transformAllDetectionCoordinates(
		MtkCameraFaceMetadata &faceMetadata) const;
	void transformDetectionCoordinate(int32_t &x, int32_t &y) const;

	std::shared_ptr<AieParser> parser_;
	FrameBuffer *inputImage_;
	FrameBuffer *faceDetectionMetadata_;
	FrameBuffer *faceToneClassificationMetadata_;
	const Size currentSensorSize_;
	int camSysMetaRequestId_;

	std::unique_ptr<MappedFrameBuffer> currentMappedImageBuffer_;
	fd_cal_struct *algoCalibration_;
};

} // namespace ipa::mtkisp7
} // namespace libcamera

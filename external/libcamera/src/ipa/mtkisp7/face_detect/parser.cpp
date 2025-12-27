/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2022, Google Inc.
 *
 * parser.cpp - MtkISP7 AIE device output parser
 */

#include "parser.h"

#include <memory>

#include <libcamera/control_ids.h>

#include "mtkcam-core/feature/common/faceeffect/FaceDetection/FD_Tuning/TuningPara.h"
#include "mtkcam-halif/def/BuiltinTypes.h"

#include "mtkisp7_ipa_interface.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

namespace {
constexpr int kFdVersion = 1946050;
}

MUINT8 *AieParser::getWorkingBuffer()
{
	return reinterpret_cast<MUINT8 *>(workingBuffer_);
}

bool AieParser::isValid()
{
	return algoInterface.get() != nullptr;
}

int AieParser::initialize()
{
	algoInterface.reset(MTKDetection::createInstance(DRV_FD_OBJ_HW));
	if (!isValid()) {
		LOG(MtkISP7, Error) << "Failed to initialize algorithm";
		return -ENOMEM;
	}
	for (int i = 0; i < MAX_CROP_NUM; i++) {
		parserBufferList_[i] = parserBuffers_[i];
	}

	return 0;
}

void AieParser::configure()
{
	latestOutput_.reset();
}

/**
 * @brief Sets the latest face detection result. Should only be called by
 *        AieParseTasks.
 * \param[in] output of the face detection result from AieParseTask
 */
void AieParser::setLatestOutput(const MtkCameraFaceMetadata &output)
{
	latestOutput_.emplace(output);
}

/**
 * @brief Get the latest face detection result
 *
 * \param[in] latest output of the face detection result from AieParseTask
 */
void AieParser::getLatestOutput(std::optional<MtkCameraFaceMetadata> &latest)
{
	latest = latestOutput_;
}

int AieParser::doParse(FrameBuffer *input, FrameBuffer *faceResult,
		       FrameBuffer *toneResult, Size currentSensorSize,
		       uint32_t camSysMetaRequestId,
		       ipa::mtkisp7::PrimaryFaceData &faceToneRoi,
		       ControlList &out)
{
	initParse();

	algoCalibration_ = algoInterface->FDGetCalData();

	if (!algoCalibration_) {
		LOG(MtkISP7, Error) << "Failed to prepare calibration buffer!";
		return -ENOMEM;
	}

	for (int i = 0; i < MAX_FACE_SEL_NUM; i++) {
		algoCalibration_->display_flag[i] = KAL_FALSE;
	}

	currentSensorSize_ = currentSensorSize;
	MappedFrameBuffer mappedImageBuffer(input,
					    MappedFrameBuffer::MapFlag::Read);

	if (!mappedImageBuffer.isValid()) {
		LOG(MtkISP7, Error) << "Failed to map image buffer!";
		return mappedImageBuffer.error();
	}

	FdOptions opts = createBufferOptions();
	opts.ImageBufferY = mappedImageBuffer.planes()[0].data();
	opts.ImageBufferUV20 = mappedImageBuffer.planes()[1].data();

	// todo(yerlandinata): check if FDVTGetMode is needed.
	FDVT_OPERATION_MODE_ENUM mode;
	algoInterface->FDVTGetMode(&mode);
	algoInterface->FDVTMain(&opts);

	return parseAll(faceResult, toneResult,
			camSysMetaRequestId, faceToneRoi, out);
}

void AieParser::initParse()
{
	auto inputSize = Size{ 640, 480 };

	MTKFDFTInitInfo config;
	config.FDBufWidth = inputSize.width;
	config.FDBufHeight = inputSize.height;
	config.FDTBufWidth = inputSize.width;
	config.FDTBufHeight = inputSize.height;
	config.FDSrcWidth = inputSize.width;
	config.FDSrcHeight = inputSize.height;

	// Tuning
	config.FDThreshold = tuning_53.FDThreshold;
	config.MajorFaceDecision = tuning_53.MajorFaceDecision;
	config.DelayThreshold = tuning_53.DelayThreshold;
	config.DelayCount = tuning_53.DelayCount;
	config.DisLimit = tuning_53.DisLimit;
	config.DecreaseStep = tuning_53.DecreaseStep;
	config.FDMINSZ = tuning_53.FDMINSZ;
	config.OTBndOverlap = tuning_53.OTBndOverlap;
	config.OTRatio = tuning_53.OTRatio;
	config.OTds = tuning_53.OTds;
	config.OTtype = tuning_53.OTtype;
	config.SmoothLevel = tuning_53.SmoothLevel;
	config.Momentum = tuning_53.Momentum;
	config.SmoothLevelUI = tuning_53.SmoothLevelUI;
	config.MomentumUI = tuning_53.MomentumUI;
	config.MaxTrackCount = tuning_53.MaxTrackCount;
	config.SilentModeFDSkipNum = tuning_53.SilentModeFDSkipNum;
	config.HistUpdateSkipNum = tuning_53.HistUpdateSkipNum;
	config.FDSkipStep = tuning_53.FDSkipStep;
	config.FDRectify = tuning_53.FDRectify;
	config.FDRefresh = tuning_53.FDRefresh;

	// MW control tuning
	config.WorkingBufAddr = nullptr;
	config.WorkingBufSize = 0;
	config.FDThreadNum = 1;
	config.OTFlow = HWPIPE_FLOW_P1;
	config.FDImageArrayNum = 0;
	config.FDImageWidthArray = NULL;
	config.FDImageHeightArray = NULL;
	config.FDCurrent_mode = 0;
	config.FDModel = 1;
	config.FDMinFaceLevel = 0;
	config.FDMaxFaceLevel = 13;
	config.FDImgFmtCH1 = FACEDETECT_IMG_Y_SINGLE;
	config.FDImgFmtCH2 = FACEDETECT_IMG_RGB565;
	config.SDImgFmtCH1 = FACEDETECT_IMG_Y_SCALES;
	config.SDImgFmtCH2 = FACEDETECT_IMG_Y_SINGLE;
	config.SDThreshold = 70;
	config.SDMainFaceMust = 1;
	config.GSensor = 1;
	config.GenScaleImageBySw = 1;
	config.ParallelRGB565Conversion = false;
	config.LockOtBufferFunc = nullptr;
	config.UnlockOtBufferFunc = nullptr;
	config.lockAgent = nullptr;
	config.CameraFOV = 53; // todo
	config.CamID = 0; // todo

	config.isSecureFD = false;
	config.LandmarkEnableCnt = 5;
	config.FLDAttribConfig = 0;
	config.GenderEnableCnt = 5;
	config.PoseEnableCnt = 5;
	config.FDVersion = 53;
	config.ModelVersion = kFdVersion; // same value as in driver

	algoInterface->FDVTInit(&config);
}

FdOptions AieParser::createBufferOptions() const
{
	FdOptions opts;
	opts.fd_state = FDVT_GFD_MODE;
	opts.direction = FACEDETECT_GSENSOR_DIRECTION_NO_SENSOR;
	opts.fd_scale_start_position = 0;
	opts.gfd_fast_mode = 0;
	opts.ae_stable = false;
	opts.ForceFDMode = FDVT_GFD_MODE;
	opts.inputPlaneCount = 1;
	opts.ImageBufferY = nullptr;
	opts.ImageBufferUV20 = nullptr;
	opts.ImageBufferRGB565 = nullptr;
	opts.ImageBufferSrcVirtual = nullptr;
	opts.ImageBufferPhyPlane1 = nullptr;
	opts.ImageBufferPhyPlane2 = nullptr;
	opts.ImageBufferPhyPlane3 = nullptr;
	opts.startW = 0;
	opts.startH = 0;
	opts.model_version = 0;
	opts.curr_gtype = 0; // todo
	opts.LV = 74; // todo: FDNodeImp.cpp aeInfo->ae_lv_x10;
	opts.DynamicLandmarkCnt = 0;
	opts.TGWidthHeight[0] = currentSensorSize_.width;
	opts.TGWidthHeight[1] = currentSensorSize_.height;
	opts.TGCropOffsetXY[0] = 0;
	opts.TGCropOffsetXY[1] = 0;
	opts.TGCropWidthHeight[0] = currentSensorSize_.width;
	opts.TGCropWidthHeight[1] = currentSensorSize_.height;
	opts.MainCamID = -1;
	opts.MainCamFov = 53; // todo: FOV
	opts.MainCamFaceSize = 0;
	opts.MainCamFOVRatio[0] = 100;
	opts.MainCamFOVRatio[1] = 100;
	opts.ThisCamFOVRatio[0] = 100;
	opts.ThisCamFOVRatio[1] = 100;
	return opts;
}

/**
 * @brief Parses hardware output into MTK's library input
 *
 * The hardware has a fixed set of "regions", if the hardware
 * detects a face in a region, it will be included in the
 * \a deviceOutput. The regions may overlap with each other,
 * the MTK face library can combine these regions into one (or several).
 * The hardware have 3 sets of region sets.
 * https://en.wikipedia.org/wiki/Region_Based_Convolutional_Neural_Networks
 *
 */
int AieParser::parseFaceDetectionOutput(FrameBuffer *faceResult)
{
	MappedFrameBuffer metaMapped(
		faceResult,
		MappedFrameBuffer::MapFlag::Read);
	if (!metaMapped.isValid()) {
		LOG(MtkISP7, Error) << "Failed to map metadata buffer!";
		return metaMapped.error();
	}
	FdDrv_output_struct *deviceOutput = reinterpret_cast<FdDrv_output_struct *>(
		metaMapped.planes()[0].data());
	const size_t setCount = 3;
	std::array<FDRESULT *, setCount> resultSet{
		&deviceOutput->FDOUTPUT.PYRAMID0_RESULT,
		&deviceOutput->FDOUTPUT.PYRAMID1_RESULT,
		&deviceOutput->FDOUTPUT.PYRAMID2_RESULT,
	};
	std::array<size_t, setCount> setLength{
		deviceOutput->FDOUTPUT.FD_PYRAMID0_NUM,
		deviceOutput->FDOUTPUT.FD_PYRAMID1_NUM,
		deviceOutput->FDOUTPUT.FD_PYRAMID2_NUM,
	};
	size_t currentSet = 0;
	size_t currentSetIdx = 0;
	for (size_t i = 0; i < setLength[0] + setLength[1] + setLength[2]; i++) {
		if (currentSetIdx >= setLength[currentSet]) {
			++currentSet;
			currentSetIdx = 0;
		}
		parseFaceRoi(resultSet[currentSet], i, currentSetIdx);
		parseFaceLandmark(resultSet[currentSet], i, currentSetIdx);
		++currentSetIdx;
	}
	return 0;
}

void AieParser::parseFaceLandmark(
	FDRESULT *resultSet, int calibrationIndex, int resultSetIndex)
{
	algoCalibration_->fld_leye_x0[calibrationIndex] =
		resultSet->rip_landmark_score0[resultSetIndex];
	algoCalibration_->fld_leye_y0[calibrationIndex] =
		resultSet->rip_landmark_score1[resultSetIndex];
	algoCalibration_->fld_leye_x1[calibrationIndex] =
		resultSet->rip_landmark_score2[resultSetIndex];
	algoCalibration_->fld_leye_y1[calibrationIndex] =
		resultSet->rip_landmark_score3[resultSetIndex];
	algoCalibration_->fld_reye_x0[calibrationIndex] =
		resultSet->rip_landmark_score4[resultSetIndex];
	algoCalibration_->fld_reye_y0[calibrationIndex] =
		resultSet->rip_landmark_score5[resultSetIndex];
	algoCalibration_->fld_reye_x1[calibrationIndex] =
		resultSet->rip_landmark_score6[resultSetIndex];
	algoCalibration_->fld_reye_y1[calibrationIndex] =
		resultSet->rop_landmark_score0[resultSetIndex];
	algoCalibration_->fld_nose_x[calibrationIndex] =
		resultSet->rop_landmark_score1[resultSetIndex];
	algoCalibration_->fld_nose_y[calibrationIndex] =
		resultSet->rop_landmark_score2[resultSetIndex];
}

void AieParser::parseFaceRoi(
	FDRESULT *resultSet, int calibrationIndex, int resultSetIndex)
{
	algoCalibration_->face_candi_pos_x0[calibrationIndex] =
		resultSet->anchor_x0[resultSetIndex];
	algoCalibration_->face_candi_pos_y0[calibrationIndex] =
		resultSet->anchor_y0[resultSetIndex];
	algoCalibration_->face_candi_pos_x1[calibrationIndex] =
		resultSet->anchor_x1[resultSetIndex];
	algoCalibration_->face_candi_pos_y1[calibrationIndex] =
		resultSet->anchor_y1[resultSetIndex];
	algoCalibration_->face_reliabiliy_value[calibrationIndex] =
		(resultSet->anchor_score[resultSetIndex] + 3600);
	algoCalibration_->display_flag[calibrationIndex] = KAL_TRUE;
	algoCalibration_->face_feature_set_index[calibrationIndex] =
		algoCalibration_->current_feature_index;
	algoCalibration_->rip_dir[calibrationIndex] =
		algoCalibration_->current_feature_index;
	algoCalibration_->rop_dir[calibrationIndex] = 0;
	algoCalibration_->result_type[calibrationIndex] = GFD_RST_TYPE;
}

int AieParser::parseFaceToneClassificationOutput(FrameBuffer *toneResult)
{
	MappedFrameBuffer metaMapped(
		toneResult,
		MappedFrameBuffer::MapFlag::Read);
	if (!metaMapped.isValid()) {
		LOG(MtkISP7, Error) << "Failed to map metadata buffer!";
		return metaMapped.error();
	}

	ATTRIBUTE_V_RESULT &deviceOutput =
		reinterpret_cast<FdDrv_output_struct *>(
			metaMapped.planes()[0].data())
			->ATTRIBUTEOUTPUT;

	auto &targetCopy =
		rawFaceToneResult_[parserTaskList_[AIE_ATTR_TYPE_GENDER][0]];

	targetCopy[0] = deviceOutput.MERGED_GENDER_RESULT.RESULT[0];
	targetCopy[1] = deviceOutput.MERGED_GENDER_RESULT.RESULT[1];
	targetCopy[2] = deviceOutput.MERGED_RACE_RESULT.RESULT[0];
	targetCopy[3] = deviceOutput.MERGED_RACE_RESULT.RESULT[1];
	targetCopy[4] = deviceOutput.MERGED_RACE_RESULT.RESULT[2];
	targetCopy[5] = deviceOutput.MERGED_IS_INDIAN_RESULT.RESULT[0];
	targetCopy[6] = deviceOutput.MERGED_IS_INDIAN_RESULT.RESULT[1];
	targetCopy[7] = deviceOutput.MERGED_AGE_RESULT.RESULT[0];

	parserBufferStatus_[AIE_ATTR_TYPE_GENDER][parserTaskList_[AIE_ATTR_TYPE_GENDER][0]] = 2;

	return 0;
}

int AieParser::parseAll(FrameBuffer *faceResult,
			FrameBuffer *toneResult,
			uint32_t camSysMetaRequestId,
			ipa::mtkisp7::PrimaryFaceData &faceToneRoi,
			ControlList &out)
{
	int ret = parseFaceDetectionOutput(faceResult);
	if (ret) {
		return ret;
	}
	int32_t gammaControl[193];
	algoInterface->FDVTMainFastPhase(gammaControl);
	algoInterface->FDVTMainCropPhaseV2(
		parserTaskList_,
		parserBufferStatus_,
		parserBufferList_,
		patchSize_,
		parserAttributeTask_);
	algoInterface->FDVTMainPostPhase();
	if (toneResult) {
		ret = parseFaceToneClassificationOutput(toneResult);
		if (ret) {
			return ret;
		}
		algoInterface->FDVTMainJoinPhaseV2(
			parserBufferStatus_[AIE_ATTR_TYPE_GENDER],
			rawFaceToneResult_, 4);
	}
	updateFaceToneDriverConfig(faceToneRoi);

	algoInterface->FDVTMainJoinPhaseV2(
		parserBufferStatus_[AIE_ATTR_TYPE_POSE],
		rawFaceToneResult_, -1);
	MtkCameraFaceMetadata detectionResult;
	detectionResult.tcy_index = gammaControl[0];
	for (int i = 0; i < 32; i++) {
		detectionResult.tcy_y_curve[i] = gammaControl[i + 1];
	}
	detectionResult.tcy_uv_gain = gammaControl[33];

	detectionResult.magicNo = camSysMetaRequestId;

	detectionResult.number_of_faces = 0;
	auto inputSize = Size{ 640, 480 };
	algoInterface->FDVTGetICSResult(
		reinterpret_cast<MUINT8 *>(&detectionResult),
		getWorkingBuffer(), inputSize.width,
		inputSize.height, 0, 0, 0, 5);

	transformAllDetectionCoordinates(detectionResult);
	setLatestOutput(detectionResult);
	convertFaceMetadata(&detectionResult, out);

	return 0;
}

void AieParser::updateFaceToneDriverConfig(ipa::mtkisp7::PrimaryFaceData &faceToneRoi)
{
	const auto &configSource =
		parserBuffers_[parserTaskList_[AIE_ATTR_TYPE_GENDER][0]];

	if (configSource[0] == 0 &&
	    configSource[1] == 0 &&
	    configSource[2] == 0 &&
	    configSource[3] == 0) {
		return;
	}

	faceToneRoi = {};

	faceToneRoi.x1 = configSource[0];
	faceToneRoi.y1 = configSource[1];
	faceToneRoi.x2 = configSource[2];
	faceToneRoi.y2 = configSource[3];

	faceToneRoi.padding_left = configSource[4];
	faceToneRoi.padding_up = configSource[5];
	faceToneRoi.padding_right = configSource[6];
	faceToneRoi.padding_down = configSource[7];
}

/**
 * @brief Transforms detection coordinates into active sensor coordinate space.
 *
 * The face detection algorithm is not aware of the actual camera sensor size,
 * it only knows its image input size. This function will map the point in
 * face detection algo coordinate space to the camera sensor coordinate space.
 *
 * Transformation is done in place, no new variable / return value.
 *
 * @param[in,out] x
 * @param[in,out] y
 */
void AieParser::transformDetectionCoordinate(
	int32_t &x, int32_t &y) const
{
	x = ((x + 1000) * currentSensorSize_.width / 2000);
	y = ((y + 1000) * currentSensorSize_.height / 2000);
}

void AieParser::transformAllDetectionCoordinates(
	MtkCameraFaceMetadata &faceMetadata) const
{
	for (int i = 0; i < faceMetadata.number_of_faces; i++) {
		transformDetectionCoordinate(faceMetadata.faces[i].rect[0],
					     faceMetadata.faces[i].rect[1]);
		transformDetectionCoordinate(faceMetadata.faces[i].rect[2],
					     faceMetadata.faces[i].rect[3]);
		transformDetectionCoordinate(faceMetadata.leyex0[i],
					     faceMetadata.leyey0[i]);
		transformDetectionCoordinate(faceMetadata.leyex1[i],
					     faceMetadata.leyey1[i]);
		transformDetectionCoordinate(faceMetadata.reyex0[i],
					     faceMetadata.reyey0[i]);
		transformDetectionCoordinate(faceMetadata.reyex1[i],
					     faceMetadata.reyey1[i]);
		transformDetectionCoordinate(faceMetadata.nosex[i],
					     faceMetadata.nosey[i]);
		transformDetectionCoordinate(faceMetadata.mouthx0[i],
					     faceMetadata.mouthy0[i]);
		transformDetectionCoordinate(faceMetadata.mouthx1[i],
					     faceMetadata.mouthy1[i]);
		transformDetectionCoordinate(faceMetadata.leyeux[i],
					     faceMetadata.leyeuy[i]);
		transformDetectionCoordinate(faceMetadata.leyedx[i],
					     faceMetadata.leyedy[i]);
		transformDetectionCoordinate(faceMetadata.reyeux[i],
					     faceMetadata.reyeuy[i]);
		transformDetectionCoordinate(faceMetadata.reyedx[i],
					     faceMetadata.reyedy[i]);
	}
}

void AieParser::convertFaceMetadata(MtkCameraFaceMetadata *faceMetadata, ControlList &out)
{
	std::vector<uint8_t> faceScores;
	std::vector<Rectangle> faceRectangles;
	std::vector<Point> faceLandmarks;

	faceScores.reserve(faceMetadata->number_of_faces);
	faceRectangles.reserve(faceMetadata->number_of_faces);
	faceLandmarks.reserve(3 * faceMetadata->number_of_faces);
	for (int i = 0; i < faceMetadata->number_of_faces; i++) {
		faceScores.push_back(faceMetadata->faces[i].score);
		Point faceTopLeft = Point{
			faceMetadata->faces[i].rect[0],
			faceMetadata->faces[i].rect[1]
		};
		Point faceBottomRight = Point{
			faceMetadata->faces[i].rect[2],
			faceMetadata->faces[i].rect[3]
		};
		faceRectangles.emplace_back(faceTopLeft, faceBottomRight);
		Point leftEye = Point{
			(faceMetadata->leyex0[i] + faceMetadata->leyex1[i]) / 2,
			(faceMetadata->leyey0[i] + faceMetadata->leyey1[i]) / 2
		};
		faceLandmarks.push_back(leftEye);

		Point rightEye = Point{
			(faceMetadata->reyex0[i] + faceMetadata->reyex1[i]) / 2,
			(faceMetadata->reyey0[i] + faceMetadata->reyey1[i]) / 2
		};
		faceLandmarks.push_back(rightEye);

		Point mouth = Point{
			(faceMetadata->mouthx0[i] + faceMetadata->mouthx1[i]) / 2,
			(faceMetadata->mouthy0[i] + faceMetadata->mouthy1[i]) / 2
		};
		faceLandmarks.push_back(mouth);
	}
	out.set(controls::draft::FaceDetectFaceScores, faceScores);
	out.set(controls::draft::FaceDetectFaceRectangles, faceRectangles);
	out.set(controls::draft::FaceDetectFaceLandmarks, faceLandmarks);
}

} /* namespace libcamera */

/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023; Google Inc.
 *
 * hal_isp.cpp - Delegate of MtkISP7 HalIsp
 */

#include "hal_isp.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <sys/mman.h>

#include <libcamera/base/log.h>

#include "libcamera/internal/mapped_framebuffer.h"

#include "../hal3a/hal_3a.h"
#include "debug_exif/aaa/dbg_aaa_param.h"
#include "halisp/ITuningDataProvider.h"
#include "halisp/IspControls.h"
#include "halisp/utils/Size.h"
#include "libcamera/request.h"
#include "mtkcam-interfaces/utils/ndd/ndd_autogen_def.h"
#include "pipeline/mtkisp7/odt/imagiq_adapter/static_metadata/stage.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"
#include "platform/mtkisp7/mtkcam-core/aaa/include/nvbuf_util.h"
#include "platform/mtkisp7/mtkcam-interfaces/include/kernel-headers/kd_imgsensor.h"
#include "platform/mtkisp7/platform_utils.h"
#include "tuning_mapping/cam_idx_struct_ext_pub.h"

#include "control_ids.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

std::shared_ptr<mtk::isphal::v1::IHalIsp> HalIsp::m_pHalisp;
std::shared_ptr<mtk::isphal::v1::IHalIsp> HalIsp::m_pHalispCapture;

HalIsp::HalIsp(OnDeviceTuner *odt)
	: onDeviceTuner_(odt)
{
}

int HalIsp::init(int32_t sensorIdx, int32_t sensorDev, Hal3A *hal3A)
{
	sensorIdx_ = sensorIdx;
	sensorDev_ = sensorDev;

	hal3A_ = hal3A;

	// TODO: implement a proper init() for m_P1CamInfo to avoid vtable pointer overwritten.
	memset(&m_P1CamInfo, 0, sizeof(m_P1CamInfo));

	NVRAM_SENSOR_IDX_INFO _sensorIdxInfo;

	switch (PlatformUtils::platform_) {
	case PlatformUtils::MtkISP7Platform::NONE:
		LOG(MtkISP7, Fatal) << "Platform unconfigured";
		break;

	case PlatformUtils::MtkISP7Platform::GOOGLE:
		// TODO(chenghaoyang): Abstract sensors' information to support different sensor modules.
		if (sensorIdx_ == 0) { // back camera
			_sensorIdxInfo.sensorDev = 1;
			_sensorIdxInfo.sensorId = 4921;
			_sensorIdxInfo.facing = 0;
			_sensorIdxInfo.moduleId = 0;
			_sensorIdxInfo.sensorName = "HI1339_MIPI_RAW";
			sensorId_ = HI1339_SENSOR_ID;

		} else { // front camera
			_sensorIdxInfo.sensorDev = 2;
			_sensorIdxInfo.sensorId = 2211;
			_sensorIdxInfo.facing = 1;
			_sensorIdxInfo.moduleId = 0;
			_sensorIdxInfo.sensorName = "GC08A3_MIPI_RAW";
			sensorId_ = GC08A3_SENSOR_ID;
		}
		break;

	case PlatformUtils::MtkISP7Platform::LENOVO:
		if (sensorIdx_ == 0) { // back camera
			_sensorIdxInfo.sensorDev = 1;
			_sensorIdxInfo.sensorId = 2211;
			_sensorIdxInfo.facing = 0;
			_sensorIdxInfo.moduleId = 0;
			_sensorIdxInfo.sensorName = "GC08A3_MIPI_RAW";
			sensorId_ = GC08A3_SENSOR_ID;
		} else { // front camera
			_sensorIdxInfo.sensorDev = 2;
			_sensorIdxInfo.sensorId = 1442;
			_sensorIdxInfo.facing = 1;
			_sensorIdxInfo.moduleId = 0;
			_sensorIdxInfo.sensorName = "GC05A2_MIPI_RAW";
			sensorId_ = GC05A2_SENSOR_ID;
		}
		break;
	}
	m_P1CamInfo.i4_sensor_id = _sensorIdxInfo.sensorId;
	m_P1CamInfo.app_iso_value = 100;
	m_P1CamInfo.i4ZoomRatio_x100 = 100;
	m_P1CamInfo.fgFDEnable = 1;
	m_P1CamInfo.rFdInfo.FD_source = 1;
	m_P1CamInfo.user_id = sensorDev;

	NvBufUtil::initSensorInfo(sensorIdx_, _sensorIdxInfo);

	//TODO, seperate the config for differnt module (geralt, ciri)
	switch (sensorId_) {
	case HI1339_SENSOR_ID:
		m_P1CamInfo.rCropRzInfo.sTGout = mtk::isphal::Size{ 4208, 3120 };
		activeArray_ = Rectangle{ 0, 0, 4208, 3120 };
		break;
	case GC08A3_SENSOR_ID:
		m_P1CamInfo.rCropRzInfo.sTGout = mtk::isphal::Size{ 3264, 2448 };
		activeArray_ = Rectangle{ 0, 0, 3264, 2448 };
		break;
	case GC05A2_SENSOR_ID:
		m_P1CamInfo.rCropRzInfo.sTGout = mtk::isphal::Size{ 2592, 1944 };
		activeArray_ = Rectangle{ 0, 0, 2592, 1944 };
		break;
	default:
		LOG(MtkISP7, Error) << "Un-handle sensor_id: " << sensorId_;
		m_P1CamInfo.rCropRzInfo.sTGout = mtk::isphal::Size{ 2592, 1944 };
		activeArray_ = Rectangle{ 0, 0, 2592, 1944 };
		break;
	}

	provider_ = mtk::isphal::v1_0::TuningDataProvider::createInstance(
		sensorIdx_, sensorDev_, 0);

	pPluginNotifier_ = mtk::ispcf::IPluginNotifier::createInstance(sensorDev_, sensorIdx_);
	return 0;
}

void HalIsp::configure(const Size &maxVideoSize,
		       const Size &maxStillSize,
		       const bool isVideo)
{
	maxVideoStreamSize_ = maxVideoSize;
	maxStillStreamSize_ = maxStillSize;
	isVideo_ = isVideo;

	m_P1CamInfo.rMapping_Info.eSensorMode =
		(isVideo_) ? ESensorMode_Video : ESensorMode_Preview;

	m_P1CamInfo.rMapping_Info.eFeature =
		(isVideo_) ? NSIspTuning::EFeature_Video : EFeature_Preview;

	m_P1CamInfo.rMapping_Info.eStage = NSIspTuning::EStage_P1;
	m_P1CamInfo.rMapping_Info.eCustomFeature = NSIspTuning::ECustomFeature_OFF;

	m_P1CamInfo.p1_yuv_port = 0x0111; // rConfigInfo.direct_yuv_path;
	m_P1CamInfo.bYUV_after_rrz = 0;

	m_P1CamInfo.rMapping_Info.eSensorFeature = NSIspTuning::ESensorFeature_OFF;

	m_P1CamInfo.hwhdr_info.i4fus_num = 0;
	m_P1CamInfo.hwhdr_info.hdr_type = mtk::isphal::v1_0::EISP_HWHDRType_None;

	m_P1CamInfo.yuvo_ds_mode_info.yuvo_r2_ds = 2;
	m_P1CamInfo.yuvo_ds_mode_info.yuvo_r4_ds = 0;

	m_P1CamInfo.user_id = sensorDev_;

	if (m_pHalisp)
		m_pHalisp.reset();

	if (m_pHalispCapture)
		m_pHalispCapture.reset();

	m_pHalisp = mtk::isphal::v1::IHalIsp::createInstance(
		sensorDev_, sensorIdx_, m_P1CamInfo.user_id);

	m_pHalispCapture = mtk::isphal::v1::IHalIsp::createInstance(
		sensorDev_, sensorIdx_, m_P1CamInfo.user_id + 10000);

	mtk_isp_buf_info bufferInfo;
	m_pHalisp->queryISPBufferInfo(&bufferInfo);

	mtk_isp_buf_info bufferInfoCapture;
	m_pHalispCapture->queryISPBufferInfo(&bufferInfoCapture);
}

uint32_t HalIsp::getLpnrIsoThreshold(mtk::isphal::v1_0::IspPerframeControl &cam_info)
{
	if (lpnrThredshold_)
		return lpnrThredshold_.value();

	CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO qry = cam_info.rMapping_Info_with_sys_info;

	qry.mapping_info.eFeature = EFeature_Capture_lpnr;
	qry.mapping_info.eStage = EStage_TR_R2Y;
	qry.mapping_info.eAction = EAction_Capture;

	mtk::isphal::v1::isp_lpnrthres_Param param;
	provider_->readDataForFeature(&param, sizeof(param), EModuleDB_LPNR_THRES, qry);
	lpnrThredshold_ = static_cast<uint32_t>(param.LPNR_ISO_HIGH_TH);

	return lpnrThredshold_.value();
}

std::shared_ptr<mtk::isphal::v1::isp_swme_Param> HalIsp::getIspSwmeParam()
{
	if (isp_swme_Param_) {
		return isp_swme_Param_;
	}
	isp_swme_Param_ = std::make_shared<mtk::isphal::v1::isp_swme_Param>();
	CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO qry = m_P1CamInfo.rMapping_Info_with_sys_info;
	qry.mapping_info.eFeature = EFeature_Capture_mfnr;
	qry.mapping_info.eStage = EStage_SWME;
	qry.mapping_info.eAction = EAction_Capture;

	provider_->readDataForFeature(isp_swme_Param_.get(), sizeof(mtk::isphal::v1::isp_swme_Param), EModuleDB_SW_ME, qry);

	return isp_swme_Param_;
}

std::shared_ptr<mtk::isphal::v1::isp_bss_Param> HalIsp::getIspBssParam()
{
	if (isp_bss_Param_) {
		return isp_bss_Param_;
	}
	isp_bss_Param_ = std::make_shared<mtk::isphal::v1::isp_bss_Param>();
	CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO qry = m_P1CamInfo.rMapping_Info_with_sys_info;

	qry.mapping_info.eFeature = EFeature_Capture_mfnr_single_nr;
	qry.mapping_info.eStage = EStage_BSS;
	qry.mapping_info.eAction = EAction_Capture;

	provider_->readDataForFeature(isp_bss_Param_.get(), sizeof(mtk::isphal::v1::isp_bss_Param), NSIspTuning::EModuleDB_BSS, qry);

	return isp_bss_Param_;
}

std::shared_ptr<mtk::isphal::v1::isp_mfnrthres_Param> HalIsp::getIspMfnrThresParam()
{
	if (isp_mfnrthres_Param_) {
		return isp_mfnrthres_Param_;
	}
	isp_mfnrthres_Param_ = std::make_shared<mtk::isphal::v1::isp_mfnrthres_Param>();
	CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO qry = m_P1CamInfo.rMapping_Info_with_sys_info;

	qry.mapping_info.eFeature = EFeature_Capture_mfnr;
	qry.mapping_info.eStage = EStage_P1;
	qry.mapping_info.eAction = EAction_Capture;

	provider_->readDataForFeature(isp_mfnrthres_Param_.get(), sizeof(mtk::isphal::v1::isp_mfnrthres_Param), NSIspTuning::EModuleDB_MFNR_THRES, qry);

	return isp_mfnrthres_Param_;
}

void HalIsp::fillCamInfoFaceData(MtkCameraFaceMetadata *faces,
				 mtk::isphal::CAMERA_TUNING_FD_INFO_T &fdInfo,
				 NSIspTuning::CAM_IDX_QRY_COMB_ISP7 &rMapping_Info)
{
	static_assert(sizeof(MtkCameraFaceMetadata::YUVsts) <=
			      sizeof(mtk::isphal::CAMERA_TUNING_FD_INFO_T::YUVsts),
		      "face struct YUVsts size error");
	static_assert(sizeof(MtkCameraFaceMetadata::GenderLabel) <=
			      sizeof(mtk::isphal::CAMERA_TUNING_FD_INFO_T::fld_GenderLabel),
		      "face struct fld_GenderLabel size error");
	static_assert(sizeof(MtkCameraFaceMetadata::fld_GenderInfo) <=
			      sizeof(mtk::isphal::CAMERA_TUNING_FD_INFO_T::fld_GenderInfo),
		      "face struct fld_GenderInfo size error");
	static_assert(sizeof(MtkCameraFaceMetadata::fld_rop) <=
			      sizeof(mtk::isphal::CAMERA_TUNING_FD_INFO_T::fld_rop),
		      "face struct fld_rop size error");

	if (!faces) {
		m_P1CamInfo.fgFDEnable = false;
		memset(&fdInfo, 0, sizeof(mtk::isphal::CAMERA_TUNING_FD_INFO_T));
		return;
	}

	m_P1CamInfo.fgFDEnable = true;
	memset(&fdInfo, 0, sizeof(mtk::isphal::CAMERA_TUNING_FD_INFO_T));

	fdInfo.FD_magicNo = faces->magicNo;
	fdInfo.FaceNum = faces->number_of_faces;

	if (fdInfo.FaceNum > 0) {
		rMapping_Info.eFaceDetection = EFaceDetection_Yes;
	} else {
		rMapping_Info.eFaceDetection = EFaceDetection_No;
	}
	memcpy(&(fdInfo.YUVsts), &(faces->YUVsts), sizeof(faces->YUVsts));
	memcpy(&(fdInfo.fld_GenderLabel), &(faces->GenderLabel), sizeof(faces->GenderLabel));
	memcpy(&(fdInfo.fld_GenderInfo), &(faces->fld_GenderInfo), sizeof(faces->fld_GenderInfo));
	memcpy(&(fdInfo.fld_rop), &(faces->fld_rop), sizeof(faces->fld_rop));
	memcpy(&(fdInfo.Landmark_CV), &(faces->fa_cv), sizeof(faces->fa_cv));

	fdInfo.GenderNum = faces->genderNum;
	fdInfo.LandmarkNum = faces->poseNum;

	// FD TCY
	fdInfo.tcy_index = faces->tcy_index;
	fdInfo.tcy_uv_gain = faces->tcy_uv_gain;
	memcpy(&(fdInfo.tcy_y_curve), &(faces->tcy_y_curve), sizeof(faces->tcy_y_curve));

	if (faces->number_of_faces != 0) {
		int i = 0;
		mtk::hal3a::mtk_3a_area area;

		for (i = 0; i < faces->number_of_faces; i++) {
			fdInfo.fld_rip[i] = faces->posInfo[i].rip_dir;

			// Face
			fdInfo.rect[i][0] = faces->faces[i].rect[0];
			fdInfo.rect[i][1] = faces->faces[i].rect[1];
			fdInfo.rect[i][2] = faces->faces[i].rect[2];
			fdInfo.rect[i][3] = faces->faces[i].rect[3];

			if (faces->fa_cv[i] > 0) {
				// Left eye
				fdInfo.Face_Leye[i][0] = faces->leyex0[i];
				fdInfo.Face_Leye[i][1] = faces->leyey0[i];
				fdInfo.Face_Leye[i][2] = faces->leyex1[i];
				fdInfo.Face_Leye[i][3] = faces->leyey1[i];

				// Right eye
				fdInfo.Face_Reye[i][0] = faces->reyex0[i];
				fdInfo.Face_Reye[i][1] = faces->reyey0[i];
				fdInfo.Face_Reye[i][2] = faces->reyex1[i];
				fdInfo.Face_Reye[i][3] = faces->reyey1[i];
			}
		}
	}
}

mtk::isphal::Size HalIsp::getTargetSize(bool isCapture)
{
	if (isCapture) {
		return mtk::isphal::Size(maxStillStreamSize_.width,
					 maxStillStreamSize_.height);
	}
	return mtk::isphal::Size(maxVideoStreamSize_.width,
				 maxVideoStreamSize_.height);
}

int HalIsp::getCamSysMetaTuning(uint64_t frmId, uint64_t aaaFrmId,
				int fd, intptr_t va, size_t offset,
				size_t bufSize, bool isCapture,
				MtkCameraFaceMetadata *faces,
				std::optional<uint32_t> internalRequestIdApplied,
				std::optional<Feature> featureApplied,
				ipa::mtkisp7::AaaIspExchange *aaaIspExchange,
				const ControlList &controls_opt)
{
	ASSERT(aaaIspExchange);

	mtk::isphal::IspTuningBufferP1 tuning_data = {};

	mtk::isphal::Buffer regBuf1((intptr_t)va, fd, offset, bufSize);
	tuning_data.p1_meta_buffer = regBuf1;

	m_P1CamInfo.mock_camsys = false;
	fillCamInfoFaceData(faces, m_P1CamInfo.rFdInfo, m_P1CamInfo.rMapping_Info);

	// Target structure
	mtk::isphal::v1_0::TuningParamP1 tuning_param_p1 = {};
	mtk::isphal::v1_0::ReturnParamP1 result_p1 = {};

	// setup input/out data
	result_p1.tuning_data = &tuning_data;

	tuning_param_p1.cam_info = &m_P1CamInfo;
	tuning_param_p1.cam_info_3a = &m_P1CamInfo_3a;
	tuning_param_p1.tuning_stat = nullptr;

	tuning_param_p1.cam_info->hwhdr_info.i4fus_num = 0;
	tuning_param_p1.cam_info->hwhdr_info.hdr_type = mtk::isphal::v1_0::EISP_HWHDRType_None;

	tuning_param_p1.is_need_exif = true;

	tuning_param_p1.cam_info->rMapping_Info.eFeature = (isVideo_) ? NSIspTuning::EFeature_Video : NSIspTuning::EFeature_Preview;

	tuning_param_p1.cam_info->rMapping_Info.eStage = NSIspTuning::EStage_P1;
	tuning_param_p1.cam_info->rMapping_Info.eSensorFeature = NSIspTuning::ESensorFeature_OFF;

	tuning_param_p1.cam_info->qrm_enable = 0;
	tuning_param_p1.cam_info->qbpc_enable = 0;

	tuning_param_p1.cam_info->i4ZoomRatio_x100 = 100;

	tuning_param_p1.cam_info->rCropRzInfo.targetSize =
		getTargetSize(isCapture);

	tuning_param_p1.cam_info->control_mode = mtk::isphal::v1_0::kControlModeOn;
	tuning_param_p1.capture_mode = mtk::isphal::v1_0::kCaptureModeNone;

	uint8_t android_color_correction_mode = 1;

	android_color_correction_mode = controls_opt.get(controls::draft::ColorCorrectionMode).value_or(1);

	tuning_param_p1.cam_info->color_correction_mode =
		(android_color_correction_mode) ? mtk::isphal::v1_0::kColorCorrectionModeAuto : mtk::isphal::v1_0::kColorCorrectionModeManual;
	tuning_param_p1.cam_info->sensor_test_pattern_mode = mtk::isphal::v1_0::kSensorTestPatternModeOff;

	float mat[9] = { 1.0f, 0.0f, 0.0f,
			 0.0f, 1.0f, 0.0f,
			 0.0f, 0.0f, 1.0f };

	const auto &colorCorrectionMatrix =
		controls_opt.get(controls::ColourCorrectionMatrix).value_or(mat);

	memcpy(tuning_param_p1.cam_info->color_correction_transform.mat,
	       colorCorrectionMatrix.data(), colorCorrectionMatrix.size() * sizeof(float));

	uint8_t android_tonemap_mode = controls_opt.get(controls::draft::TonemapMode).value_or(1);
	if (android_tonemap_mode == 0) {
		tuning_param_p1.cam_info->tone_map_mode = mtk::isphal::v1_0::kToneMapModeMaual;
		std::vector<float> default_tonemap_curve_red = { 0.0f, 0.0f, 1.0f, 1.0f };
		std::vector<float> default_tonemap_curve_green = { 0.0f, 0.0f, 1.0f, 1.0f };
		std::vector<float> default_tonemap_curve_blue = { 0.0f, 0.0f, 1.0f, 1.0f };
		auto tonemap_curve_red = controls_opt.get(controls::draft::TonemapCurveRed).value_or(default_tonemap_curve_red);
		auto tonemap_curve_green = controls_opt.get(controls::draft::TonemapCurveGreen).value_or(default_tonemap_curve_blue);
		auto tonemap_curve_blue = controls_opt.get(controls::draft::TonemapCurveBlue).value_or(default_tonemap_curve_green);
		tuning_param_p1.cam_info->tone_map_curve.red_Cnt = tonemap_curve_red.size() / 2;
		for (auto i = 0; i < (int)tonemap_curve_red.size() / 2; i++) {
			tuning_param_p1.cam_info->tone_map_curve.red_X[i] = tonemap_curve_red[i * 2];
			tuning_param_p1.cam_info->tone_map_curve.red_Y[i] = tonemap_curve_red[i * 2 + 1];
		}
		tuning_param_p1.cam_info->tone_map_curve.green_Cnt = tonemap_curve_green.size() / 2;
		for (auto i = 0; i < (int)tonemap_curve_green.size() / 2; i++) {
			tuning_param_p1.cam_info->tone_map_curve.green_X[i] = tonemap_curve_green[i * 2];
			tuning_param_p1.cam_info->tone_map_curve.green_Y[i] = tonemap_curve_green[i * 2 + 1];
		}
		tuning_param_p1.cam_info->tone_map_curve.blue_Cnt = tonemap_curve_blue.size() / 2;
		for (auto i = 0; i < (int)tonemap_curve_blue.size() / 2; i++) {
			tuning_param_p1.cam_info->tone_map_curve.blue_X[i] = tonemap_curve_blue[i * 2];
			tuning_param_p1.cam_info->tone_map_curve.blue_Y[i] = tonemap_curve_blue[i * 2 + 1];
		}
	} else {
		tuning_param_p1.cam_info->tone_map_mode = mtk::isphal::v1_0::kToneMapModeAuto;
	}

	tuning_param_p1.cam_info->hdr10_enable = false;

	tuning_param_p1.cam_info->rMapping_Info.eCustomFeature = NSIspTuning::ECustomFeature_OFF;
	tuning_param_p1.cam_info->frame_rate = 30;

	tuning_param_p1.cam_info->rP1SyncInfo.bSync2AMode = false;
	tuning_param_p1.cam_info->rP1SyncInfo.bSlave_P1 = false;

	tuning_param_p1.pModulesCtrl = NULL;
	tuning_param_p1.shading_table = NULL;

	tuning_param_p1.cam_info->sr_para.enable = 0;
	tuning_param_p1.cam_info->ggm_info.is_ai3a_en = false;

	tuning_param_p1.cam_info->u8Id = frmId; // update frame number
	tuning_param_p1.cam_info->aaaId = aaaFrmId; // update frame number

	tuning_param_p1.magic_num = frmId;
	tuning_param_p1.aaa_magic_num = aaaFrmId;
	tuning_param_p1.subsample_count = 1;

	tuning_param_p1.cam_info->rNdd_info = {};

	mtk::hal3a::v1_0::mtk_3a_result *aaaResult = hal3A_->resultHistory_.query(aaaFrmId);

	int32_t threshold = getLpnrIsoThreshold(*tuning_param_p1.cam_info);
	int32_t sensorSensitivity = aaaResult->ae_result.sensor_sensitivity;

	aaaIspExchange->highIsoMode = (sensorSensitivity > threshold);

	if (onDeviceTuner_->isLowIsoLpnrEnforced()) {
		aaaIspExchange->highIsoMode = false;
	}

	bool shouldDump = false;
	if (internalRequestIdApplied && featureApplied) {
		// Not dummy frame
		shouldDump = onDeviceTuner_->tuneCamsysHalIsp(
			internalRequestIdApplied.value(),
			tuning_param_p1, result_p1, *aaaResult,
			featureApplied.value(),
			aaaIspExchange->highIsoMode);
	}

	m_pHalisp->getCamSysMetaTuning(&tuning_param_p1, &result_p1);

	if (result_p1.exif.valid) {
		// Exif data may be filled by IHalIsp::getCamSysMetaTuning
		std::memcpy(
			reinterpret_cast<uint8_t *>(&aaaResult->debug_isp_info),
			result_p1.exif.data, sizeof(AAA_DEBUG_INFO2_T));
	}

	if (shouldDump) {
		m_pHalisp->dump4CamSysModule(
			&tuning_param_p1, &result_p1);
	}

	tuning_param_p1.cam_info->ae_info.isp_data.bAEStable &= aaaResult->awb_result.awb_stable;
	addHistory(frmId, *tuning_param_p1.cam_info, *tuning_param_p1.cam_info_3a);

	std::shared_ptr<mtk::isphal::v1::isp_mfnrthres_Param> mfnrThresParam = getIspMfnrThresParam();
	aaaIspExchange->mfnrMode = (sensorSensitivity > mfnrThresParam->iso_th);

	return 0;
}

void fillPqInfo(NSIspTuning::EStage_T stage, Size inputSize,
		Size outputSize, Size outputSize2,
		mtk::isphal::IspTuningBufferP2 &tuning_data)
{
	(void)inputSize;
	mtk::isphal::PQInfo pqInfo = {};
	mtk::isphal::WPEInfo wpeInfo = {};

	switch (stage) {
	case EStage_AFBLD_F0:
	case EStage_AFBLD_F1:
	case EStage_AFBLD_F2:
	case EStage_AFBLD_F3:
	case EStage_AFBLD_F4:
	case EStage_AFBLD_F5:
	case EStage_AFBLD_F6:
		pqInfo.CropSize = { outputSize.width, outputSize.height };
		pqInfo.OutSize = { outputSize.width, outputSize.height };
		pqInfo.serial_id = 1;
		tuning_data.pq_info.push_back(pqInfo);
		break;
	case EStage_BFBLD_BASE:
	case EStage_BFBLD_REF:
	case EStage_BFME:
	case EStage_DS:
	case EStage_DS_VBI_V2:
	case EStage_DS_VBI_V5:
		break;
	case EStage_MCDS_F1:
		wpeInfo.buf_id = mtk::isphal::kWPE_LITE;
		wpeInfo.is_motion = 0;
		tuning_data.wpe_info.push_back(wpeInfo);
		break;
	case EStage_MSBLD_F0:
	case EStage_MSBLD_F1:
	case EStage_MSBLD_F2:
	case EStage_MSBLD_F3:
	case EStage_MSBLD_F4:
	case EStage_MSBLD_F5:
	case EStage_MSBLD_F6:
		break;
	case EStage_P2_Y2Y_PQ_DIP:
	case EStage_P2_MS_F0_PQ_DIP:
		pqInfo.CropSize = { inputSize.width, inputSize.height };
		pqInfo.OutSize = { outputSize.width, outputSize.height };
		pqInfo.serial_id = 1;
		tuning_data.pq_info.push_back(pqInfo);

		pqInfo.serial_id = 2;
		tuning_data.pq_info.push_back(pqInfo);
		break;
	case EStage_WPE_LTR_Y2Y_F1:
		wpeInfo.buf_id = mtk::isphal::kWPE_LITE;
		wpeInfo.is_motion = 1;
		tuning_data.wpe_info.push_back(wpeInfo);
		break;
	case EStage_WPE_WghtMap:
		wpeInfo.is_motion = 1;
		wpeInfo.buf_id = mtk::isphal::kWPE_TNR;
		tuning_data.wpe_info.push_back(wpeInfo);
		wpeInfo.buf_id = mtk::isphal::kWPE_LITE;
		tuning_data.wpe_info.push_back(wpeInfo);
		break;
	case EStage_WPE_P2_PQDIP_MS_F0:
		pqInfo.active_tcc = 1;
		pqInfo.ctrl = mtk::isphal::kIspPQControlModeAuto;

		pqInfo.serial_id = 1;
		pqInfo.CropSize = { outputSize.width, outputSize.height };
		pqInfo.OutSize = { outputSize.width, outputSize.height };
		tuning_data.pq_info.push_back(pqInfo);

		pqInfo.serial_id = 2;
		pqInfo.CropSize = { outputSize2.width, outputSize2.height };
		pqInfo.OutSize = { outputSize2.width, outputSize2.height };
		tuning_data.pq_info.push_back(pqInfo);

		wpeInfo.is_motion = 1;
		wpeInfo.buf_id = mtk::isphal::kWPE_TNR;
		tuning_data.wpe_info.push_back(wpeInfo);
		break;
	case EStage_TR_Y2Y_F1:
	case EStage_TR_R2Y:
	case EStage_TR_Y2Y_F4:
	case EStage_LTR_VBI:
	case EStage_LTR_Y2Y_F4:
	case EStage_P2_MS_F0_H:
	case EStage_ME_3PASS_MODE0:
	case EStage_ME_3PASS_MM:
	case EStage_ME_3PASS_MODE1:
	case EStage_LTR_ME_L1:
	case EStage_P2_IDI:
	case EStage_P2_MS_F_SMALL:
	case EStage_P2_MS_F4:
	case EStage_P2_MS_F3:
	case EStage_P2_MS_F2:
	case EStage_P2_MS_F1:
		break;
	default:
		ASSERT(false);
	}
}

void fillIndex(NSIspTuning::EStage_T stage, bool isCapture,
	       mtk::isphal::v1_0::IspImgSysControl &imgsys_info,
	       int tnr_frameTotal, int tnr_frameIndex)
{
	imgsys_info.tnr_fw_config.frameIndex = 0;
	imgsys_info.tnr_fw_config.scaleIndex = 0;
	imgsys_info.tnr_fw_config.totalScaleNo = 0;
	imgsys_info.tnr_fw_config.bInkMode = 0;
	imgsys_info.tnr_fw_config.frameTotal = 0;
	imgsys_info.ds_mode = 0;
	imgsys_info.total_frame_num = 0;

	switch (stage) {
	case EStage_BFBLD_BASE:
	case EStage_BFBLD_REF:
	case EStage_BFME:
	case EStage_DS:
	case EStage_DS_VBI_V2:
	case EStage_DS_VBI_V5:
	case EStage_MCDS_F1:
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_AFBLD_F0:
	case EStage_MSBLD_F0:
		imgsys_info.tnr_fw_config.scaleIndex = 0;
		imgsys_info.tnr_fw_config.frameTotal = tnr_frameTotal;
		imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.tnr_fw_config.frameIndex = tnr_frameIndex;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_MSBLD_F1:
	case EStage_AFBLD_F1:
		imgsys_info.tnr_fw_config.scaleIndex = 1;
		imgsys_info.tnr_fw_config.frameTotal = tnr_frameTotal;
		imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.tnr_fw_config.frameIndex = tnr_frameIndex;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_MSBLD_F2:
	case EStage_AFBLD_F2:
		imgsys_info.tnr_fw_config.scaleIndex = 2;
		imgsys_info.tnr_fw_config.frameTotal = tnr_frameTotal;
		imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.tnr_fw_config.frameIndex = tnr_frameIndex;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_MSBLD_F3:
	case EStage_AFBLD_F3:
		imgsys_info.tnr_fw_config.scaleIndex = 3;
		imgsys_info.tnr_fw_config.frameTotal = tnr_frameTotal;
		imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.tnr_fw_config.frameIndex = tnr_frameIndex;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_MSBLD_F4:
	case EStage_AFBLD_F4:
		imgsys_info.tnr_fw_config.scaleIndex = 4;
		imgsys_info.tnr_fw_config.frameTotal = tnr_frameTotal;
		imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.tnr_fw_config.frameIndex = tnr_frameIndex;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_MSBLD_F5:
	case EStage_AFBLD_F5:
		imgsys_info.tnr_fw_config.scaleIndex = 5;
		imgsys_info.tnr_fw_config.frameTotal = tnr_frameTotal;
		imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.tnr_fw_config.frameIndex = tnr_frameIndex;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_MSBLD_F6:
	case EStage_AFBLD_F6:
		imgsys_info.tnr_fw_config.scaleIndex = 6;
		imgsys_info.tnr_fw_config.frameTotal = tnr_frameTotal;
		imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.tnr_fw_config.frameIndex = tnr_frameIndex;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_TR_Y2Y_F1:
	case EStage_TR_Y2Y_F4:
	case EStage_LTR_VBI:
	case EStage_LTR_Y2Y_F4:
	case EStage_WPE_LTR_Y2Y_F1:
	case EStage_WPE_WghtMap:
	case EStage_TR_R2Y:
	case EStage_ME_3PASS_MODE0:
	case EStage_ME_3PASS_MM:
	case EStage_ME_3PASS_MODE1:
	case EStage_LTR_ME_L1:
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_P2_Y2Y_PQ_DIP:
	case EStage_P2_MS_F0_PQ_DIP:
		imgsys_info.ds_mode = 1;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_P2_MS_F0_H:
		imgsys_info.ds_mode = 1;
		imgsys_info.total_frame_num = 1;
		imgsys_info.tnr_fw_config.scaleIndex = 0;
		break;
	case EStage_P2_IDI:
		imgsys_info.tnr_fw_config.frameIndex = 255;
		imgsys_info.tnr_fw_config.scaleIndex = 6;
		imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_P2_MS_F_SMALL:
		imgsys_info.tnr_fw_config.frameIndex = 255;
		imgsys_info.tnr_fw_config.scaleIndex = 5;
		imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_P2_MS_F4:
		imgsys_info.tnr_fw_config.frameIndex = 255;
		imgsys_info.tnr_fw_config.scaleIndex = 4;
		imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_P2_MS_F3:
		imgsys_info.tnr_fw_config.frameIndex = 255;
		imgsys_info.tnr_fw_config.scaleIndex = 3;
		if (isCapture)
			imgsys_info.tnr_fw_config.totalScaleNo = 4;
		else
			imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_P2_MS_F2:
		imgsys_info.tnr_fw_config.frameIndex = 255;
		imgsys_info.tnr_fw_config.scaleIndex = 2;
		if (isCapture)
			imgsys_info.tnr_fw_config.totalScaleNo = 4;
		else
			imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_P2_MS_F1:
		imgsys_info.tnr_fw_config.frameIndex = 255;
		imgsys_info.tnr_fw_config.scaleIndex = 1;
		if (isCapture)
			imgsys_info.tnr_fw_config.totalScaleNo = 4;
		else
			imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.total_frame_num = 1;
		break;
	case EStage_WPE_P2_PQDIP_MS_F0:
		imgsys_info.tnr_fw_config.frameIndex = 255;
		imgsys_info.tnr_fw_config.scaleIndex = 0;
		imgsys_info.tnr_fw_config.totalScaleNo = 7;
		imgsys_info.total_frame_num = 1;
		break;
	default:
		ASSERT(false);
	}
}

mtk::isphal::Rectangle getTncCrop(uint32_t width, uint32_t height, bool needCropTNC16x9)
{
	unsigned int tncHeight = height;
	int tncY = 0;

	/* 16:9 */
	if (needCropTNC16x9) {
		tncHeight = width * 9 / 16;
		tncY = (height - tncHeight) / 2;
	}

	return mtk::isphal::Rectangle{ 0, tncY, width, tncHeight };
}

void fillTncInfo(NSIspTuning::EStage_T stage, Size inputSize, Size outputSize, Size fullDipSize,
		 mtk::isphal::v1_0::IspImgSysControl &imgsys_info, bool needCropTNC16x9)
{
	(void)stage;
	(void)outputSize;
	imgsys_info.rCropRzInfo.rBefore_Warp_Crop = {};
	imgsys_info.rCropRzInfo.rBefore_Warp_Size = {};
	imgsys_info.tncs_info = {};
	imgsys_info.tnc_roi = {};

	switch (stage) {
	case EStage_AFBLD_F0:
	case EStage_AFBLD_F1:
	case EStage_AFBLD_F2:
	case EStage_AFBLD_F3:
	case EStage_AFBLD_F4:
	case EStage_AFBLD_F5:
	case EStage_AFBLD_F6:
		break;
	case EStage_BFBLD_REF:
	case EStage_BFBLD_BASE:
		imgsys_info.rCropRzInfo.rBefore_Warp_Crop =
			mtk::isphal::Rectangle{ 0, 0, inputSize.width, inputSize.height };
		imgsys_info.rCropRzInfo.rBefore_Warp_Size = mtk::isphal::Size{
			inputSize.width, inputSize.height
		};
		break;

	case EStage_BFME:
	case EStage_DS:
	case EStage_DS_VBI_V2:
	case EStage_DS_VBI_V5:
	case EStage_MCDS_F1:
	case EStage_MSBLD_F0:
	case EStage_MSBLD_F1:
	case EStage_MSBLD_F2:
	case EStage_MSBLD_F3:
	case EStage_MSBLD_F4:
	case EStage_MSBLD_F5:
	case EStage_MSBLD_F6:
		break;
	case EStage_TR_Y2Y_F1:
	case EStage_TR_R2Y:
		imgsys_info.rCropRzInfo.rBefore_Warp_Crop =
			mtk::isphal::Rectangle{ 0, 0, inputSize.width, inputSize.width };
		imgsys_info.rCropRzInfo.rBefore_Warp_Size = mtk::isphal::Size{
			inputSize.width, inputSize.height
		};
		break;
	case EStage_P2_Y2Y_PQ_DIP:
	case EStage_P2_MS_F0_PQ_DIP:
	case EStage_WPE_P2_PQDIP_MS_F0:
	case EStage_P2_IDI:
		imgsys_info.rCropRzInfo.rBefore_Warp_Size = mtk::isphal::Size{
			fullDipSize.width, fullDipSize.height
		};
		break;
	case EStage_P2_MS_F_SMALL:
	case EStage_P2_MS_F4:
	case EStage_P2_MS_F3:
	case EStage_P2_MS_F2:
	case EStage_P2_MS_F1:
	case EStage_TR_Y2Y_F4:
	case EStage_LTR_VBI:
	case EStage_LTR_Y2Y_F4:
	case EStage_WPE_LTR_Y2Y_F1:
	case EStage_WPE_WghtMap:
	case EStage_P2_MS_F0_H:
	case EStage_ME_3PASS_MODE0:
	case EStage_ME_3PASS_MM:
	case EStage_ME_3PASS_MODE1:
	case EStage_LTR_ME_L1:
		break;
	default:
		ASSERT(false);
	}

	switch (stage) {
	case EStage_AFBLD_F0:
		imgsys_info.tnc_roi.bValid = 1;
		imgsys_info.tnc_roi.tnc_in_cropinfo = mtk::isphal::Rectangle{
			0, 0, inputSize.width, inputSize.height
		};
		break;
	case EStage_AFBLD_F1:
	case EStage_AFBLD_F2:
	case EStage_AFBLD_F3:
	case EStage_AFBLD_F4:
	case EStage_AFBLD_F5:
	case EStage_AFBLD_F6:
		break;
	case EStage_BFBLD_REF:
	case EStage_BFBLD_BASE:
		imgsys_info.tncs_info.bValid = 1;
		imgsys_info.tncs_info.tncs_in_cropinfo = mtk::isphal::Rectangle{
			0, 0, inputSize.width, inputSize.height
		};
		;
		imgsys_info.tncs_info.target_tnc_size = mtk::isphal::Size{
			fullDipSize.width, fullDipSize.height
		};
		break;
	case EStage_BFME:
		imgsys_info.tnc_roi.bValid = 1;
		imgsys_info.tnc_roi.tnc_in_cropinfo = mtk::isphal::Rectangle{
			0, 0, inputSize.width, inputSize.height
		};
		break;
	case EStage_DS:
	case EStage_DS_VBI_V2:
	case EStage_DS_VBI_V5:
	case EStage_MCDS_F1:
	case EStage_MSBLD_F0:
	case EStage_MSBLD_F1:
	case EStage_MSBLD_F2:
	case EStage_MSBLD_F3:
	case EStage_MSBLD_F4:
	case EStage_MSBLD_F5:
	case EStage_MSBLD_F6:
		break;
	case EStage_TR_Y2Y_F1:
	case EStage_TR_R2Y:
		imgsys_info.tncs_info.bValid = 1;
		imgsys_info.tncs_info.tncs_in_cropinfo =
			getTncCrop(inputSize.width, inputSize.height, needCropTNC16x9);
		imgsys_info.tncs_info.target_tnc_size = mtk::isphal::Size{
			fullDipSize.width, fullDipSize.height
		};
		break;
	case EStage_P2_Y2Y_PQ_DIP:
	case EStage_P2_MS_F0_PQ_DIP:
	case EStage_P2_IDI:
	case EStage_WPE_P2_PQDIP_MS_F0:
		imgsys_info.tnc_roi.bValid = 1;
		imgsys_info.tnc_roi.tnc_in_cropinfo =
			getTncCrop(fullDipSize.width, fullDipSize.height, needCropTNC16x9);
		break;
	case EStage_P2_MS_F_SMALL:
	case EStage_P2_MS_F4:
	case EStage_P2_MS_F3:
	case EStage_P2_MS_F2:
	case EStage_P2_MS_F1:
	case EStage_TR_Y2Y_F4:
	case EStage_LTR_VBI:
	case EStage_LTR_Y2Y_F4:
	case EStage_WPE_LTR_Y2Y_F1:
	case EStage_WPE_WghtMap:
	case EStage_P2_MS_F0_H:
	case EStage_ME_3PASS_MODE0:
	case EStage_ME_3PASS_MM:
	case EStage_ME_3PASS_MODE1:
	case EStage_LTR_ME_L1:
		break;
	default:
		ASSERT(false);
	}
}

int HalIsp::getImgSysMetaTuning(uint32_t camSysMetaRequestId,
				ImgMetaRequest &imgMetaRequest,
				uint32_t internalRequestId,
				bool needCropTNC16x9,
				Feature feature,
				const ControlList &controls_opt)
{
	return getImgSysMetaTuning(camSysMetaRequestId, imgMetaRequest,
				   internalRequestId, internalRequestId,
				   needCropTNC16x9, feature, controls_opt);
}

int HalIsp::getImgSysMetaTuning(uint32_t camSysMetaRequestId,
				ImgMetaRequest &imgMetaRequest,
				uint32_t internalRequestId,
				uint32_t frameNumber,
				bool needCropTNC16x9,
				Feature feature,
				const ControlList &controls_opt)
{
	bool is_capture = imgMetaRequest.isCapture;
	bool is_mfnr = imgMetaRequest.isMfnr;
	Size inputSize = imgMetaRequest.inputSize;
	Size outputSize = imgMetaRequest.outputSize;
	Size outputSize2 = imgMetaRequest.outputSize2;
	Size fullDipSize = imgMetaRequest.fullDipSize;

	FrameBuffer *tuningFrame = imgMetaRequest.tuningBuffer;
	FrameBuffer *statisFrame = imgMetaRequest.statisticsBuffer;
	FrameBuffer *swHistBuffer = imgMetaRequest.swHistBuffer;

	mtk::isphal::IspTuningControl tuning_control = {};
	mtk::isphal::IspTuningStatisticsP2 tuning_statistics = {};
	mtk::isphal::IspTuningBufferP2 tuning_data = {};

	NSCam::IMetadata appMeta, appMeta2;
	NSCam::IMetadata imgsSysHalMeta;
	NSCam::IMetadata camSysHalMeta;
	if (is_mfnr) {
		int scale = 0;
		bool isBlending = true;
		int mfnrIndex = imgMetaRequest.index;
		bool isGolden = imgMetaRequest.isGolden;
		switch (imgMetaRequest.stage) {
		case EStage_AFBLD_F0:
		case EStage_MSBLD_F0:
			scale = 0;
			break;
		case EStage_MSBLD_F1:
		case EStage_AFBLD_F1:
			scale = 1;
			break;
		case EStage_MSBLD_F2:
		case EStage_AFBLD_F2:
			scale = 2;
			break;
		case EStage_MSBLD_F3:
		case EStage_AFBLD_F3:
			scale = 3;
			break;
		case EStage_MSBLD_F4:
		case EStage_AFBLD_F4:
			scale = 4;
			break;
		case EStage_MSBLD_F5:
		case EStage_AFBLD_F5:
			scale = 5;
			break;
		case EStage_MSBLD_F6:
		case EStage_AFBLD_F6:
			scale = 6;
			break;
		default:
			scale = 0;
			isBlending = false;
			break;
		}
		NSCam::IMetadata::setEntry<MINT64>(&camSysHalMeta, MTK_TUNING_FEATURE_CAPTURE_HINT, NSIspTuning::EFeature_Capture_mfnr);
		NSCam::IMetadata::setEntry<MINT64>(&camSysHalMeta, MTK_TUNING_FEATURE_STREAMING_HINT, 0);
		NSCam::IMetadata::setEntry<MINT32>(&camSysHalMeta, MTK_HAL_REQUEST_INDEX, mfnrIndex);
		if (isGolden)
			NSCam::IMetadata::setEntry<BOOL>(&camSysHalMeta, MTK_FEATURE_BSS_ISGOLDEN, isGolden);
		NSCam::IMetadata::Memory qry_info;
		qry_info.resize(sizeof(CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO));
		CamInfo *prevCamInfo = queryHistory(frameNumber);
		CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO qry = prevCamInfo->cam_info.rMapping_Info_with_sys_info;
		memcpy(qry_info.editArray(), &qry, sizeof(CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO));
		NSCam::IMetadata::setEntry<NSCam::IMetadata::Memory>(&camSysHalMeta, MTK_ISP_ATMS_MAPPING_INFO_WITH_SYSTEM_INFO, qry_info);
		if (isBlending) {
			NSCam::IMetadata::setEntry<MINT32>(&imgsSysHalMeta, MTK_ISP_TNR_FRAME_INDEX, imgMetaRequest.tnr_frameIndex);
			NSCam::IMetadata::setEntry<MINT32>(&imgsSysHalMeta, MTK_ISP_TNR_FRAME_TOTAL, 4);
			NSCam::IMetadata::setEntry<MINT32>(&imgsSysHalMeta, MTK_ISP_TNR_SCALE_INDEX, scale);
			NSCam::IMetadata::setEntry<MINT32>(&imgsSysHalMeta, MTK_ISP_TNR_TOTAL_SCALE, 7);
		}
		// pre-processing for plugin
	}
	mtk_halisp_metaset control_from_camsys(&appMeta, &camSysHalMeta);
	mtk_halisp_metaset control_imgsys(&appMeta2, &imgsSysHalMeta);

	tuning_control.stage = imgMetaRequest.stage;
	tuning_control.mock = false;
	tuning_control.update_mode = mtk::isphal::kIspUpdateModeAuto;

	if (is_capture)
		tuning_control.action = NSIspTuning::EAction_Capture;
	else
		tuning_control.action = NSIspTuning::EAction_Preview;

	mtk::isphal::Buffer metaBuf(
		(intptr_t)imgMetaRequest.mappedTuningBuffer->planes()[0].data(),
		tuningFrame->planes()[0].fd.get(),
		tuningFrame->planes()[0].offset,
		tuningFrame->planes()[0].length);

	tuning_data.p2_meta_buffer = metaBuf;
	tuning_data.in_image = {};

	fillPqInfo(imgMetaRequest.stage, inputSize, outputSize, outputSize2, tuning_data);

	if (statisFrame) {
		mtk::isphal::Buffer statsBuf(
			(intptr_t)imgMetaRequest.mappedStatisticsBuffer->planes()[0].data(),
			statisFrame->planes()[0].fd.get(),
			statisFrame->planes()[0].offset,
			statisFrame->planes()[0].length);
		tuning_statistics.imgsys_statistics = statsBuf;
	}

	if (swHistBuffer) {
		mtk::isphal::Buffer swBuf(
			(intptr_t)imgMetaRequest.mappedSwHistBuffer->planes()[0].data(),
			swHistBuffer->planes()[0].fd.get(),
			swHistBuffer->planes()[0].offset,
			swHistBuffer->planes()[0].length);
		tuning_statistics.imgsys_hist_buffer = swBuf;
	}

	for (auto &[key, buffer] : imgMetaRequest.reserved) {
		mtk::isphal::Buffer reserveBuf(
			(intptr_t)buffer.second->planes()[0].data(),
			buffer.first->planes()[0].fd.get(),
			buffer.first->planes()[0].offset,
			buffer.first->planes()[0].length);
		tuning_statistics.reserved[key] = reserveBuf;
	}

	mtk::isphal::v1_0::TuningParamDip tuning_param_p2 = {};
	mtk::isphal::v1_0::ReturnParamDip result_p2 = {};

	tuning_param_p2.tuning_stat.push_back(&tuning_statistics);

	tuning_param_p2.imgsys_info.emplace_back();
	mtk::isphal::v1_0::IspImgSysControl &imgsys_info = tuning_param_p2.imgsys_info.back();

	result_p2.tuning_data.push_back(&tuning_data);

	imgsys_info.mock_imgsys = tuning_control.mock;

	mtk::hal3a::v1_0::mtk_3a_result *aaaResult =
		hal3A_->resultHistory_.query(camSysMetaRequestId);

	CamInfo *camInfo = queryHistory(camSysMetaRequestId);

	if (is_mfnr) {
		std::vector<
			std::pair<std::string,
				  std::vector<std::pair<mtk::ispcf::kISPPLUGIN_BUF_T, mtk::isphal::Buffer>>>>
			buf_list_in;
		bool plugin_ret1 = pPluginNotifier_->doAllImgSysPreproc(
			control_from_camsys, control_imgsys, tuning_control, tuning_statistics,
			*result_p2.tuning_data[0], buf_list_in);
		if (plugin_ret1) {
			LOG(MtkISP7, Debug) << "There are some plugin available for pre-processing";
		}
	}

	/* parsePipelineMetadata */
	{
		mtk::isphal::v1_0::IspPerframeControl *pCaminfoBuf = NULL;
		mtk::isphal::v1_0::IspReadOnlyControl *pCaminfoBuf_3a = NULL;
		const uint8_t *pModuleBuf = NULL;

		pCaminfoBuf = &camInfo->cam_info;
		pCaminfoBuf_3a = &camInfo->cam_info_3a;

		tuning_param_p2.is_need_exif = true;

		tuning_param_p2.cam_info = *pCaminfoBuf;
		tuning_param_p2.cam_info_3a = pCaminfoBuf_3a;
		tuning_param_p2.pModulesCtrl = pModuleBuf;

		tuning_param_p2.cam_info.drzs8t_crop_info.is_valid = true;
		tuning_param_p2.cam_info.drzs8t_crop_info.crop_region =
			NSCam::MRect(activeArray_.width, activeArray_.height);
		tuning_param_p2.cam_info.drzs8t_crop_info.dst_size =
			NSCam::MSize(fullDipSize.width, fullDipSize.height);

		tuning_param_p2.cam_info.ISP_3A_result_id = tuning_param_p2.cam_info.u8Id;

		auto &shading = aaaResult->shading_result;
		int32_t lsc_data_size = shading.lsc_data.size();

		if (lsc_data_size > 0) {
			tuning_param_p2.shading_table = shading.lsc_data.data();
			tuning_param_p2.shading_table_size = shading.lsc_data.size();
			tuning_param_p2.cam_info.shd_info.data_valid = true;
		}

		tuning_param_p2.cam_info.multi_frame_bss_index = 0;
		// tuning_param_p2.cam_info.user_id = 0;

		if (is_capture)
			tuning_param_p2.cam_info.rMapping_Info.eFeature = (is_mfnr) ? NSIspTuning::EFeature_Capture_mfnr : NSIspTuning::EFeature_Capture_lpnr;
		else
			tuning_param_p2.cam_info.rMapping_Info.eFeature = (isVideo_) ? NSIspTuning::EFeature_Video : NSIspTuning::EFeature_Preview;

		tuning_param_p2.cam_info.rMapping_Info.eCustomFeature = NSIspTuning::ECustomFeature_OFF;

		uint8_t android_tonemap_mode = controls_opt.get(controls::draft::TonemapMode).value_or(1);
		if (android_tonemap_mode == 0) {
			tuning_param_p2.cam_info.tone_map_mode = mtk::isphal::v1_0::kToneMapModeMaual;
			std::vector<float> default_tonemap_curve_red = { 0.0f, 0.0f, 1.0f, 1.0f };
			std::vector<float> default_tonemap_curve_green = { 0.0f, 0.0f, 1.0f, 1.0f };
			std::vector<float> default_tonemap_curve_blue = { 0.0f, 0.0f, 1.0f, 1.0f };
			auto tonemap_curve_red = controls_opt.get(controls::draft::TonemapCurveRed).value_or(default_tonemap_curve_red);
			auto tonemap_curve_green = controls_opt.get(controls::draft::TonemapCurveGreen).value_or(default_tonemap_curve_blue);
			auto tonemap_curve_blue = controls_opt.get(controls::draft::TonemapCurveBlue).value_or(default_tonemap_curve_green);
			tuning_param_p2.cam_info.tone_map_curve.red_Cnt = tonemap_curve_red.size() / 2;
			for (auto i = 0; i < (int)tonemap_curve_red.size() / 2; i++) {
				tuning_param_p2.cam_info.tone_map_curve.red_X[i] = tonemap_curve_red[i * 2];
				tuning_param_p2.cam_info.tone_map_curve.red_Y[i] = tonemap_curve_red[i * 2 + 1];
			}
			tuning_param_p2.cam_info.tone_map_curve.green_Cnt = tonemap_curve_green.size() / 2;
			for (auto i = 0; i < (int)tonemap_curve_green.size() / 2; i++) {
				tuning_param_p2.cam_info.tone_map_curve.green_X[i] = tonemap_curve_green[i * 2];
				tuning_param_p2.cam_info.tone_map_curve.green_Y[i] = tonemap_curve_green[i * 2 + 1];
			}
			tuning_param_p2.cam_info.tone_map_curve.blue_Cnt = tonemap_curve_blue.size() / 2;
			for (auto i = 0; i < (int)tonemap_curve_blue.size() / 2; i++) {
				tuning_param_p2.cam_info.tone_map_curve.blue_X[i] = tonemap_curve_blue[i * 2];
				tuning_param_p2.cam_info.tone_map_curve.blue_Y[i] = tonemap_curve_blue[i * 2 + 1];
			}
		} else {
			tuning_param_p2.cam_info.tone_map_mode = mtk::isphal::v1_0::kToneMapModeAuto;
		}

		tuning_param_p2.cam_info.edge_mode = mtk::isphal::v1_0::kEdgeModeOn;
		uint8_t android_edge_mode = controls_opt.get(controls::draft::EdgeMode).value_or(1);
		if ((android_edge_mode == MTK_EDGE_MODE_OFF) ||
		    (android_edge_mode == MTK_EDGE_MODE_ZERO_SHUTTER_LAG))
			tuning_param_p2.cam_info.edge_mode = mtk::isphal::v1_0::kEdgeModeOff;

		if (is_capture) {
			tuning_param_p2.camsys_history.tone_map_mode = MTK_TONEMAP_MODE_HIGH_QUALITY;
			tuning_param_p2.camsys_history.edge_mode = MTK_EDGE_MODE_HIGH_QUALITY;
			tuning_param_p2.camsys_history.nr_mode = MTK_NOISE_REDUCTION_MODE_HIGH_QUALITY;
		} else {
			tuning_param_p2.camsys_history.tone_map_mode = MTK_TONEMAP_MODE_FAST;
			tuning_param_p2.camsys_history.edge_mode = MTK_EDGE_MODE_FAST;
			tuning_param_p2.camsys_history.nr_mode = MTK_NOISE_REDUCTION_MODE_FAST;
		}

		if (is_capture)
			tuning_param_p2.capture_mode = mtk::isphal::v1_0::kCaptureModeNormal;
		else
			tuning_param_p2.capture_mode = mtk::isphal::v1_0::kCaptureModeNone;

		tuning_param_p2.cam_info.rCropRzInfo.targetSize =
			getTargetSize(is_capture);
	}

	// parseImgSysMetadata
	{
		fillIndex(imgMetaRequest.stage, is_capture, imgsys_info,
			  imgMetaRequest.tnr_frameTotal, imgMetaRequest.tnr_frameIndex);
		imgsys_info.sequence_num = internalRequestId;
		imgsys_info.is_need_dump_exif = 1;
		if (is_mfnr) {
			mtk::isphal::Size mel0Out(0, 0);
			mtk::isphal::Size gyroOut(0, 0);
			imgsys_info.rCropRzInfo.sMEL0out = mel0Out;
			imgsys_info.rCropRzInfo.sGyroMv = gyroOut;

		} else {
			mtk::isphal::Size mel0Out(576, 432);
			mtk::isphal::Size gyroOut(32, 24);
			imgsys_info.rCropRzInfo.sMEL0out = mel0Out;
			imgsys_info.rCropRzInfo.sGyroMv = gyroOut;
		}
		fillTncInfo(imgMetaRequest.stage, inputSize, outputSize,
			    fullDipSize, imgsys_info, needCropTNC16x9);

		imgsys_info.rWrappingInfo = {};
		imgsys_info.bypass_nr = 0;
	}

	// parseImgSysBriefPart
	{
		uint8_t u1P2TuningUpdate = tuning_control.update_mode;
		imgsys_info.tuing_update_mode =
			static_cast<mtk::isphal::v1_0::TuningUpdateMode>(u1P2TuningUpdate);

		imgsys_info.stage = static_cast<NSIspTuning::EStage_T>(tuning_control.stage);

		mtk::isphal::Size imgsys_in_size(inputSize.width, inputSize.height);
		imgsys_info.rCropRzInfo.imgsys_in_size = imgsys_in_size;

		imgsys_info.is_capture = is_capture;
		imgsys_info.action = tuning_control.action;
	}

	mtk::isphal::v1_0::IspPerframeControl &cam_info = tuning_param_p2.cam_info;
	{
		// Check the following values
		imgsys_info.srcimg_descriptor.p2_in_img_fmg = 0;
		imgsys_info.srcimg_descriptor.format = mtk::isphal::kImageFormatMtkRawBayer;

		uint8_t android_nr_mode = controls_opt.get(controls::draft::NoiseReductionMode).value_or(0);
		if ((android_nr_mode == MTK_NOISE_REDUCTION_MODE_OFF) ||
		    (android_nr_mode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG) ||
		    (android_nr_mode == MTK_NOISE_REDUCTION_MODE_MINIMAL))
			imgsys_info.nr_mode = mtk::isphal::v1_0::kNoiseReductionModeOff;
		else
			imgsys_info.nr_mode = mtk::isphal::v1_0::kNoiseReductionModeOn;

		// copy caminfo
		imgsys_info.rMapping_Info = cam_info.rMapping_Info;
		imgsys_info.rMapping_Info_with_sys_info =
			cam_info.rMapping_Info_with_sys_info;

		imgsys_info.rFdInfo_afterWarp = cam_info.rFdInfo;

		// replace with correct stage
		imgsys_info.rMapping_Info.eStage =
			static_cast<NSIspTuning::EStage_T>(imgsys_info.stage);
		imgsys_info.rMapping_Info.eAction =
			static_cast<NSIspTuning::EAction_T>(imgsys_info.action);

		tuning_param_p2.cam_info.rNdd_info = {};
		onDeviceTuner_->tuneImgsysHalIsp(
			internalRequestId, frameNumber, tuning_param_p2, result_p2,
			*aaaResult,
			imgsys_info.rMapping_Info.eStage, feature);

		imgsys_info.rNdd_info = cam_info.rNdd_info;
		imgsys_info.sr_para = cam_info.sr_para;
	}
	if (is_capture) {
		m_pHalispCapture->getImgSysMetaTuning(&tuning_param_p2, &result_p2);
	} else {
		m_pHalisp->getImgSysMetaTuning(&tuning_param_p2, &result_p2);
	}

	onDeviceTuner_->tuneExif(
		internalRequestId, frameNumber, tuning_param_p2.exif_3a,
		result_p2.exif, imgsys_info.rMapping_Info.eStage, feature);
	if (is_mfnr) {
		bool plugin_ret2 = pPluginNotifier_->doAllImgSysPostProc(
			control_from_camsys, control_imgsys, tuning_control, tuning_statistics,
			*result_p2.tuning_data[0], result_p2.exif);

		if (plugin_ret2) {
			LOG(MtkISP7, Debug) << "There are some plugin available for post-processing";
		}
	}

	return 0;
}

void HalIsp::addHistory(uint32_t internalRequestId,
			mtk::isphal::v1_0::IspPerframeControl &cam_info,
			mtk::isphal::v1_0::IspReadOnlyControl &cam_info_3a)
{
	CamInfo camInfo;
	std::memcpy((void *)&camInfo.cam_info, (void *)&cam_info, sizeof(cam_info));
	std::memcpy(&camInfo.cam_info_3a, &cam_info_3a, sizeof(cam_info_3a));

	camInfoHistory_.add(internalRequestId, camInfo);
}

HalIsp::CamInfo *HalIsp::queryHistory(uint32_t internalRequestId)
{
	return camInfoHistory_.query(internalRequestId);
}

} // namespace libcamera

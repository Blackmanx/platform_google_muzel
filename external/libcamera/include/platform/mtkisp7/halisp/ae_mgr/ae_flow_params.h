/*
 * Copyright (C) 2022 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/********************************************************************************************
 * LEGAL DISCLAIMER
 *
 * (Header of MediaTek Software/Firmware Release or Documentation)
 *
 * BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
 *AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN
 *"AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 *EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER
 *DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF
 *ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE
 *MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
 *WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR
 *ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO CONFORM TO
 *A PARTICULAR STANDARD OR OPEN FORUM.
 *
 * BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 *LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT
 *MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR
 *REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK
 *FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH
 *THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS
 *PRINCIPLES.
 ************************************************************************************************/
#ifndef INCLUDE_MTKCAM_CORE_AAAHAL_AE_MGR_AE_FLOW_PARAMS_H_
#define INCLUDE_MTKCAM_CORE_AAAHAL_AE_MGR_AE_FLOW_PARAMS_H_

#define AE_Y_HISTOGRAM_BIN (256)

#define AE_GAIN_BASE_OBC 1024  // temp define
#define AE_GAIN_BASE_ISP 4096  // temp define
#define AE_GAIN_BASE_AFE 1024  // temp define

#define MAX_AE_METER_AREAS 9
#define AE_BLOCK_NO 5

#include <mtkcam-core/aaahal/ae_mgr/ae_setting.h>
#include <array>

typedef struct AEMeterAreaInfo {
  int32_t i4Left;
  int32_t i4Top;
  int32_t i4Right;
  int32_t i4Bottom;
  int32_t i4Weight;
  int32_t i4Id;
  int32_t i4Type;  // 0:GFD, 1:LFD, 2:OT
  int32_t i4Motion[2];
  int32_t
      i4Landmark[3][4];  // index 0: left eye, index 1: right eye, index 2:mouth
  int32_t i4LandmarkCV;
  int32_t i4ROP;
  int32_t i4LandMarkRip;
  int32_t i4LandMarkRop;
} AEMeterAreaInfo_T;

// Update information for ISP used
typedef struct ae_output_isp_info_t {
  // req & mag
  uint32_t u4RequestNum;
  uint32_t u4MagicNumber;
  // basic AE info
  bool bAEStable;
  int32_t i4AESoftStable;  // for zsl
  uint16_t u4CWValue;
  int32_t i4AEPureCwr;       // for ltm
  int32_t i4AEDynamicRange;  // for custom
  int32_t i4BVvalue_x10;
  int32_t i4LightValue_x10;
  int32_t i4RealLightValue_x10;  // real LV
  int32_t i4AEComp;              // 1ev = 1000, 1000base
  bool bAELock;
  bool bManualAE;
  bool bAESubsample_en;
  int32_t i4AECacheHit_en;
  uint32_t rAETargetMode;
  int32_t i4AEEV_Diff_x1000;
  int32_t i4AEReachBound;
  int32_t i4NitsMappingVal;  // for HDR10
  int32_t i4NitsConfVal;     // for HDR10
  //
  uint64_t u8P2Exposuretime_ns;  //!<: Exposure time in ns, 1000 mean 1 us
  uint32_t u4P2SensorGain;       //!<: sensor gain,   1x = 1024
  uint32_t u4P2DGNGain;          //!<: digital gain,  1x = 4096
  uint32_t u4P2RealISOValue;
  // face relative
  bool bAETouchEnable;
  bool bEnableFaceAE;
  bool bFaceAELCELinkEnable;
  bool bOTFaceTimeOutLockAE;
  uint32_t u4FaceAEStable;
  uint32_t u4FaceRobustStable;
  uint8_t uFaceState;
  uint32_t u4FaceNum;
  AEMeterAreaInfo FDArea[MAX_AE_METER_AREAS];
  int32_t i4Crnt_FDY;
  uint32_t u4MeterFDTarget;
  uint32_t u4MeterFDLinkTarget;
  uint32_t u4FaceRobustCnt;
  uint32_t u4FaceRobustTrustCnt;
  uint32_t u4FD_Lock_MaxCnt;
  uint32_t u4FDDropTempSmoothCnt;
  uint32_t u4OTFaceTrustCnt;
  uint32_t u4MaxGain;
  int32_t u4AE_Face_v[3];
  int32_t u4AE_Face_H_v[3];
  int32_t u4AE_Face_L_v[3];
  // ROI relative
  uint32_t u4ActWinXStart;
  uint32_t u4ActWinXEnd;
  uint32_t u4ActWinYStart;
  uint32_t u4ActWinYEnd;
  // AI-Shutter
  bool bAiShutExistMotion;           //!<: exist motion or not
  uint32_t u4AiShutMIP;              //!<: motion in pixels
  uint32_t u4AiShutExposuretime_us;  //!<: Exposure time in us
  uint32_t u4AiShutSensorGain;       //!<: sensor gain,   1x = 1024
  uint32_t u4AiShutDGNGain;          //!<: digital gain,  1x = 4096
  uint32_t u4AiShutRealISOValue;     //!<: ISO
} ae_output_isp_info_t;

typedef struct {
  ae_output_isp_info_t isp_data;
  ae_exposure_setting_table exp_table;
  ae_calculation_setting_table calc_table;
  std::array<uint32_t, 128> pipe_info;
} ae_isp_info_t;

typedef struct {
  int32_t i4Left;
  int32_t i4Top;
  int32_t i4Right;
  int32_t i4Bottom;
  int32_t i4Weight;
  int32_t i4Id;
  int32_t i4Type;  // 0:GFD, 1:LFD, 2:OT
  int32_t i4Motion[2];
  int32_t
      i4Landmark[3][4];  // index 0: left eye, index 1: right eye, index 2:mouth
  int32_t i4LandmarkCV;
  int32_t i4ROP;
  int32_t i4LandMarkRip;
  int32_t i4LandMarkRop;
} AELTMMeterArea_T;

typedef struct {
  AELTMMeterArea_T rAreas[MAX_AE_METER_AREAS];
  uint32_t u4Count;
} AELTMMeteringArea_T;

struct AE_LTM_METER_INFO {
  uint32_t vhdr_state;
  int32_t i4ISP_ReqNum;
  // AE Result
  int32_t i4BV;
  uint32_t HdrRatio;
  // Face Info
  uint32_t u4FaceNum;
  AELTMMeteringArea_T* pFaceArea;
  // AE Zoom Crop Info
  AEZOOM_WINDOW_T eAEZoomWinInfo;
  // AAO
  void* pAaoAdr;

  // Magic Number
  int32_t i4AAOMagicNum;
  int32_t i4LTMSOMagicNum;

  // LTMSO
  uint32_t* pu4LtmsAddr;

  // AI Info
  uint8_t* puSegMap;
  uint8_t* puSegConf;

  //
  bool bSlave;

  uint32_t u4NVRAMIdx;     // SW uses
  uint32_t u4IspProfile;   // SW uses
  uint32_t u4FlashState;   // SW uses
  int32_t i4ExpIndex_Cap;  // SW uses
  int32_t i4ExpLevel;
  bool bCapFlag;
};

typedef struct {
  uint64_t u8FrameDuration;  // micro sec
  uint64_t u8ExposureTime;   // micro sec
  int32_t u4Sensitivity;     // ISO value
} AE_SENSOR_PARAM_QUEUE_T;

#define AE_SENSOR_MAX_QUEUE 4

typedef struct {
  uint8_t uInputIndex;
  uint8_t uOutputIndex;
  AE_SENSOR_PARAM_QUEUE_T rSensorParamQueue[AE_SENSOR_MAX_QUEUE];
} AE_SENSOR_QUEUE_CTRL_T;
// Sensor Input params for Camer 3
typedef struct {
  int64_t u8FrameDuration;  // naro sec
  int64_t u8ExposureTime;   // naro sec
  int32_t u4Sensitivity;    // ISO value
} AE_SENSOR_PARAM_T;
// only for AIHDR Capture
typedef struct {
  uint32_t u4Exposuretime_us;  //!<: Exposure time in us
  uint32_t u4SensorGain;       //!<: sensor gain,   1x = 1024
  uint32_t u4DGNGain;          //!<: digital gain,  1x = 4096
} AE_EV_Setting;

typedef struct {
  AE_EV_Setting sLE;
  AE_EV_Setting sNE_Capture;
  AE_EV_Setting sNE_Preview;
  AE_EV_Setting sSE;
  AE_EV_Setting sSSE;
  uint32_t u4EVidx;
  int32_t i4LENEEvdiff;
  int32_t i4NESEEvdiff;
  int32_t i4FaceEVdiff_real;
  int32_t i4TouchEVdiff;
  int32_t i4EVbarEVdiff;
} AE_HDR_Setting;

#endif  // INCLUDE_MTKCAM_CORE_AAAHAL_AE_MGR_AE_FLOW_PARAMS_H_

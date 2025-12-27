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

#ifndef INCLUDE_MTKCAM_INTERFACES_UTILS_METADATA_HAL_MTK_PLATFORM_METADATA_TAG_H_
#define INCLUDE_MTKCAM_INTERFACES_UTILS_METADATA_HAL_MTK_PLATFORM_METADATA_TAG_H_

/******************************************************************************
 *
 ******************************************************************************/
typedef enum mtk_platform_metadata_section {
  MTK_HAL_REQUEST =
      0xC000,  // MTK HAL internal metadata become from 0xC000 0000
  MTK_P1NODE,
  MTK_P2NODE,
  MTK_3A_TUNINING,
  MTK_3A_EXIF,
  MTK_MF_EXIF,
  MTK_EIS,
  MTK_STEREO,
  MTK_FRAMESYNC,
  MTK_VHDR,
  MTK_PIPELINE,
  MTK_NR,
  MTK_PLUGIN,
  MTK_DUALZOOM,
  MTK_FEATUREPIPE,
  MTK_POSTPROC,
  MTK_FEATURE,
  MTK_FSC,
  MTK_TUNING,
} mtk_platform_metadata_section_t;

/******************************************************************************
 *
 ******************************************************************************/
typedef enum mtk_platform_metadata_section_start {
  MTK_HAL_REQUEST_START = MTK_HAL_REQUEST << 16,
  MTK_P1NODE_START = MTK_P1NODE << 16,
  MTK_P2NODE_START = MTK_P2NODE << 16,
  MTK_3A_TUNINING_START = MTK_3A_TUNINING << 16,
  MTK_3A_EXIF_START = MTK_3A_EXIF << 16,
  MTK_EIS_START = MTK_EIS << 16,
  MTK_STEREO_START = MTK_STEREO << 16,
  MTK_FRAMESYNC_START = MTK_FRAMESYNC << 16,
  MTK_VHDR_START = MTK_VHDR << 16,
  MTK_PIPELINE_START = MTK_PIPELINE << 16,
  MTK_NR_START = MTK_NR << 16,
  MTK_PLUGIN_START = MTK_PLUGIN << 16,
  MTK_DUALZOOM_START = MTK_DUALZOOM << 16,
  MTK_FEATUREPIPE_START = MTK_FEATUREPIPE << 16,
  MTK_POSTPROC_START = MTK_POSTPROC << 16,
  MTK_FEATURE_START = MTK_FEATURE << 16,
  MTK_FSC_START = MTK_FSC << 16,
  MTK_TUNING_START = MTK_TUNING << 16,
} mtk_platform_metadata_section_start_t;

/******************************************************************************
 *
 ******************************************************************************/
typedef enum mtk_platform_metadata_tag {
  MTK_HAL_REQUEST_REQUIRE_EXIF = MTK_HAL_REQUEST_START,  // MUINT8
  MTK_HAL_REQUEST_DUMP_EXIF,                             // MUINT8
  MTK_HAL_REQUEST_REPEAT,                                // MUINT8
  MTK_HAL_REQUEST_DUMMY,                                 // MUINT8
  MTK_HAL_REQUEST_SENSOR_SIZE,                           // MSize
  MTK_HAL_REQUEST_SENSOR_ID,                             // MINT32
  MTK_HAL_REQUEST_DEVICE_ID,                             // MINT32
  MTK_HAL_REQUEST_HIGH_QUALITY_CAP,                      // MUINT8
  MTK_HAL_REQUEST_ISO_SPEED,                             // MINT32
  MTK_HAL_REQUEST_BRIGHTNESS_MODE,                       // MINT32
  MTK_HAL_REQUEST_CONTRAST_MODE,                         // MINT32
  MTK_HAL_REQUEST_HUE_MODE,                              // MINT32
  MTK_HAL_REQUEST_SATURATION_MODE,                       // MINT32
  MTK_HAL_REQUEST_EDGE_MODE,                             // MINT32
  MTK_HAL_REQUEST_PASS1_DISABLE,                         // MINT32
  MTK_HAL_REQUEST_ERROR_FRAME,       // used for error handling    //MUINT8
  MTK_HAL_REQUEST_PRECAPTURE_START,  // 4cell                 //MUINT8
  MTK_HAL_REQUEST_AF_TRIGGER_START,  // 4cell                 //MUINT8
  MTK_HAL_REQUEST_IMG_IMGO_FORMAT,   // MINT32
  MTK_HAL_REQUEST_IMG_RRZO_FORMAT,   // MINT32
  MTK_HAL_REQUEST_INDEX,             // MINT32
  MTK_HAL_REQUEST_COUNT,             // MINT32
  MTK_HAL_REQUEST_SMVR_FPS,          // MUINT8 // 0: NOT batch request
  MTK_HAL_REQUEST_REMOSAIC_ENABLE,  // MUINT8 // 0: preview mode 1: capture mode
  MTK_HAL_REQUEST_SEAMLESS_SWITCH,  // MUINT8
  MTK_HAL_REQUEST_INDEX_BSS,        // MINT32
  MTK_HAL_REQUEST_ZSD_CAPTURE_INTENT,  // MUINT8
  MTK_HAL_REQUEST_REAL_CAPTURE_SIZE,   // MSize
  MTK_HAL_REQUEST_VIDEO_SIZE,          // MSize
  MTK_HAL_REQUEST_RAW_IMAGE_INFO,  // MINT32 // index[0]: raw fmt, index[1]: raw
                                   // stride, index[2]: raw size(width),
                                   // index[3]: raw size(height)
  MTK_HAL_REQUEST_ISP_PIPELINE_MODE,                 // MINT32
  MTK_HAL_REQUEST_CAPTURE_YUV_MODE,                  // MUINT8 // 0: OFF 1: ON
  MTK_HAL_REQUEST_ADDITIONAL_TEST_FRAME_INDEX,       // MINT32
  MTK_HAL_REQUEST_ADDITIONAL_TEST_FRAME_COUNT,       // MINT32
  MTK_HAL_REQUEST_REMOSAIC_QRM_EN,                   // MUINT8
  MTK_HAL_REQUEST_REMOSAIC_QBPC_EN,                  // MUINT8
  MTK_P1NODE_SCALAR_CROP_REGION = MTK_P1NODE_START,  // MRect
  MTK_P1NODE_BIN_CROP_REGION,                        // MRect
  MTK_P1NODE_DMA_CROP_REGION,                        // MRect
  MTK_P1NODE_SEAMLESS_TARGET_SENSOR_MODE,            // MINT32
  MTK_P1NODE_SEAMLESS_ACTIVE_SENSOR_SIZE,            // MSize
  MTK_P1NODE_SEAMLESS_TARGET_SENSOR_SIZE,            // MSize
  MTK_P1NODE_SEAMLESS_TARGET_SENSOR_PATTERN,         // MUINT8
  MTK_P1NODE_SEAMLESS_SWITCH_POLICY,                 // MUINT8
  MTK_P1NODE_BIN_SIZE,                               // MSize
  MTK_P1NODE_RESIZER_SIZE,                           // MSize
  MTK_P1NODE_RESIZER_SET_SIZE,                       // MSize
  MTK_P1NODE_CTRL_RESIZE_FLUSH,                      // MBOOL
  MTK_P1NODE_CTRL_READOUT_FLUSH,                     // MBOOL
  MTK_P1NODE_CTRL_RECONFIG_SENSOR_SETTING,           // MBOOL
  MTK_P1NODE_PROCESSOR_MAGICNUM,                     // MINT32
  MTK_P1NODE_MIN_FRM_DURATION,                       // MINT64
  MTK_ISPHAL_IMGO_TYPE,                              // MINT32
  MTK_P1NODE_SENSOR_CROP_REGION,                     // MRect
  MTK_P1NODE_YUV_RESIZER1_CROP_REGION,               // MRect
  MTK_P1NODE_SENSOR_MODE,                            // MINT32
  MTK_P1NODE_SENSOR_VHDR_MODE,                       // MINT32
  MTK_P1NODE_METADATA_TAG_INDEX,                     // MINT32
  MTK_P1NODE_RSS_SIZE,                               // MSize
  MTK_P1NODE_SENSOR_STATUS,                          // MINT32
  MTK_P1NODE_SENSOR_RAW_ORDER,                       // MINT32
  MTK_P1NODE_TWIN_SWITCH,                            // MINT32
  MTK_P1NODE_TWIN_STATUS,                            // MINT32
  MTK_P1NODE_RESIZE_QUALITY_SWITCH,                  // MINT32
  MTK_P1NODE_RESIZE_QUALITY_STATUS,                  // MINT32
  MTK_P1NODE_RESIZE_QUALITY_LEVEL,                   // MINT32
  MTK_P1NODE_RESIZE_QUALITY_SWITCHING,               // MBOOL
  MTK_P1NODE_RESUME_SHUTTER_TIME_US,                 // MINT32
  MTK_P1NODE_FRAME_START_TIMESTAMP,                  // MINT64
  MTK_P1NODE_FRAME_START_TIMESTAMP_BOOT,             // MINT64
  MTK_P1NODE_FRAME_EXP_TIMESTAMP,                    // MINT64
  MTK_P1NODE_FRAME_START_TIMESTAMP_LOW_LATENCY,      // MINT64
  MTK_P1NODE_REQUEST_PROCESSED_WITHOUT_WB,           // MBOOL
  MTK_P1NODE_ISNEED_GMV,                             // MBOOL
  MTK_P1NODE_CONTROL_SENSOR_CROP_REGION,             // MINT64[]
                                                     // index 0: streamid, index
                                                     // 1: crop_x, index 2:
                                                     // crop_y, index 3: crop_w,
                                                     // index 4:crop_h, ...
  MTK_P1NODE_RESULT_STREAM_SIZE_INFO,                // MINT64[]
                                                     // index 0: streamid, index
                                                     // 1: crop_x, index 2:
                                                     // crop_y, index 3: crop_w,
                                                     // index 4:crop_h,
                                                     // index 5: dstSize_w,
                                                     // index 6: dstSize_h, ...
  MTK_P1NODE_DS_MODE,                                // MINT32
  MTK_P2NODE_HIGH_SPEED_VDO_FPS = MTK_P2NODE_START,  // MINT32
  MTK_P2NODE_HIGH_SPEED_VDO_SIZE,                    // MSize
  MTK_P2NODE_CTRL_CALTM_ENABLE,                      // MBOOL
  MTK_P2NODE_FD_CROP_REGION,                         // MRect
  MTK_P2NODE_CROP_REGION,      // MRect // for removing black edge
  MTK_P2NODE_DSDN_ENABLE,      // MBOOL // for DSDN on/off controled by Policy
  MTK_P2NODE_VIDEO_AINR_RUNMODE,      // MINT32 // for Video AINR
                                      // run mode controled by Policy
  MTK_3A_AE_HIGH_ISO_BINNING,  // MBOOL // for 3HDR high iso binning mode
  MTK_SENSOR_SCALER_CROP_REGION,                  // MRect
  MTK_SENSOR_ACTIVE_SCALER_CROP_REGION,           // MRect
  MTK_PROCESSOR_CAMINFO = MTK_3A_TUNINING_START,  // IMemory
  MTK_PROCESSOR_CAMINFO_RO,                       // IMemory
  MTK_PROCESSOR_FD,                               // IMemory
  MTK_ISP_ATMS_MAPPING_INFO,                      // IMemory
  MTK_ISP_ATMS_MAPPING_INFO_WITH_SYSTEM_INFO,     // IMemory
  MTK_ISP_MODULES_CONTROL,                        // IMemory
  MTK_3A_ISP_PROFILE,                             // MUINT8
  MTK_3A_ISP_P1_PROFILE,                          // MUINT8
  MTK_CAMINFO_LCSOUT_INFO,                        // IMemory
  MTK_3A_ISP_FULL_PROCESS_RAW,                    // MBOOL
  MTK_3A_ISP_BYPASS_LCE,                          // MBOOL
  MTK_3A_ISP_DISABLE_NR,                          // MBOOL
  MTK_3A_ISP_NR3D_SW_PARAMS,  // MINT32[14] //GMVX, GMVY, confX, confY, MAX_GMV,
                              // frameReset, GMV_Status,ISO_cutoff
  MTK_3A_ISP_NR3D_HW_PARAMS,  // IMemory
  MTK_3A_ISP_TNR_TCY_CURVE,   // IMemory
  MTK_3A_ISP_LCE_GAIN,  // MINT32, bits[0:15]: LCE gain, bits[16:31]: LCE gain
                        // confidence ratio (0-100)
  MTK_3A_ISP_FUS_NUM,  // MINT32
  MTK_3A_AE_CAP_PARAM,  // IMemory
  MTK_3A_AE_CAP_SINGLE_FRAME_HDR,     // MUINT8
  MTK_3A_AE_BV_TRIGGER,               // MBOOL
  MTK_3A_AF_LENS_POSITION,            // MINT32
  MTK_3A_FLICKER_RESULT,            // MINT32
  MTK_3A_FLICKER_RATIO,            // MINT32
  MTK_3A_FLICKER_FREQUENCY,            // MINT32
  MTK_3A_DUMMY_BEFORE_REQUEST_FRAME,  // MBOOL // Dummy frame before capture,
                                      // only for capture intent, preview don't
                                      // use
  MTK_3A_DUMMY_AFTER_REQUEST_FRAME,  // MBOOL // Dummy frame after capture, only
                                     // for capture intent, preview don't use
  MTK_3A_MANUAL_AWB_COLORTEMPERATURE_MAX,  // MINT32
  MTK_3A_MANUAL_AWB_COLORTEMPERATURE_MIN,  // MINT32
  MTK_3A_MANUAL_AWB_COLORTEMPERATURE,      // MINT32
  MTK_3A_HDR_MODE,                         // MUINT8
  MTK_3A_AE_HDR_MIXED_ISO,                 // MUINT32
  MTK_3A_AE_ZSL_STABLE,                    // MINT32 ( MBOOL )
  MTK_ISPHAL_IMGO,                         // MUINT8
  MTK_3A_SKIP_HIGH_QUALITY_CAPTURE,        // MUINT8
  MTK_3A_AE_PERFRAME_RESULT,               // IMemory
  MTK_3A_AF_PERFRAME_RESULT,               // IMemory
  MTK_3A_PD_PERFRAME_RESULT,               // IMemory
  MTK_3A_FLASH_PERFRAME_RESULT,            // IMemory
  MTK_3A_AI3A_PERFRAME_RESULT,            // IMemory
  MTK_3A_FLASH_FORCE_OFF,                  // MINT32, AI FLASH uses this
  MTK_3A_FEATURE_AE_EXPOSURE_LEVEL,        // MINT32
  MTK_3A_FEATURE_AE_TARGET_MODE,           // MINT32
  MTK_3A_FEATURE_FLASHLIGHT_TYPE,          // MUINT8
  MTK_3A_FEATURE_FACE_DETECT_ENABLE,       // MUINT8
  MTK_3A_FEATURE_AE_REQUEST_TARGET_MODE,   // MINT32
  MTK_3A_FEATURE_AE_VALID_EXPOSURE_NUM,    // MINT32
  MTK_3A_FEATURE_AE_REQUEST_EXPOSURE_NUM,  // MINT32
  MTK_3A_FEATURE_AE_REQUEST_SENSOR_MODE,   // MINT32
  MTK_3A_SYNC_INFORMATION_REQ,             // MINT32
  MTK_LSC_TBL_DATA,                        // IMemory
  MTK_LSC_TSF_DATA,                        // IMemory
  MTK_LSC_TSF_DUMP_NO,                     // IMemory
  MTK_ISP_P2_ORIGINAL_SIZE,                // MSize
  MTK_ISP_P2_CROP_REGION,                  // MRect
  MTK_ISP_P2_WRAPPING_REGION,              // WrappingParam
  MTK_ISP_P2_RESIZER_SIZE,                 // MSize
  MTK_ISP_P2_TNCS_CROP_INFO,               // MRect
  MTK_ISP_P2_TNC_TARGET_SIZE,              // MSize for TNCS
  MTK_ISP_P2_TNC_CROP_INFO,                // MRect
  MTK_ISP_BEFORE_WARP_CROP,                // MRect
  MTK_ISP_BEFORE_WARP_SIZE,                // MSize
  MTK_ISP_P2_IN_IMG_FMT,  // MINT32, 0 or not exist: RAW->YUV, 1: YUV->YUV
  MTK_ISP_P2_TUNING_UPDATE_MODE,  // MUINT8, [0 or not exist]: as default; [1]:
                                  // keep existed parameters but some parts will
                                  // be updated; [2]: keep all existed
                                  // parameters (force mode) [3] LPCNR Pass1 [4]
                                  // LPCNR Pass2
  MTK_ISP_P2_IN_IMG_RES_REVISED,  // MINT32, describes P2 input image revised
                                  // resolution. bit[0:15] width in pixel,
                                  // bit[16:31] height in pixel. May be not
                                  // exist.
  MTK_ISP_APP_TARGET_SIZE,        // MINT32, describes APP Target resolution.
                            // bit[0:15] width in pixel, bit[16:31] height in
                            // pixel. May be not exist.
  MTK_MSF_SCALE_INDEX,  // MINT32, which scale stage index, would only exist
                        // with scaling flow
  MTK_MSF_FRAME_NUM,    // MINT32, After BSS which frame number is this stage
                        // using
  MTK_TOTAL_MULTI_FRAME_NUM,  // MINT32, MSYUV fuction used this input to know
                              // frame nunber
  MTK_TOTAL_MULTI_FRAME_NUM_CAPTURED,  // MINT32, MSF function used
  MTK_SW_DSDN_VERSION,         // MINT32, distinguish different dsdn version
  MTK_ISP_COLOR_SPACE,         // MINT32
  MTK_ISP_TNR_FRAME_TOTAL,     // MINT32, total frame num for TNR FW usage
  MTK_ISP_TNR_FRAME_INDEX,     // MINT32, frame index for TNR FW usage
  MTK_ISP_TNR_SCALE_INDEX,     // MINT32, scale index for TNR FW usage
  MTK_ISP_TNR_TOTAL_SCALE,     // MINT32, total scale number for TNR
  MTK_ISP_TNR_INK_MODE,        // MINT32, enable TNR ink mode or not
  MTK_SW_DS_MODE,              // 0: DS2, 1: DS4, 2: DS42
  MTK_ISP_DRC_CURVE,           // IMemory
  MTK_ISP_DRC_CURVE_SIZE,      // MINT32
  MTK_ISP_FEO_DATA,            // IMemory
  MTK_ISP_FEO_ENABLE,          // MINT32
  MTK_ISP_FEO_INFO,            // IMemory
  MTK_ISP_HLR_RATIO,           // MINT32, which is a HDR ratio applied in HLR
  MTK_ME_L0_SIZE,              // MSize
  MTK_GYRO_MV_SIZE,            // MSize
  MTK_FOCUS_AREA_POSITION,     // MINT32
  MTK_FOCUS_AREA_SIZE,         // MSize
  MTK_FOCUS_AREA_RESULT,       // MUINT8
  MTK_FOCUS_PAUSE,             // MUINT8
  MTK_FOCUS_MZ_ON,             // MUINT8
  MTK_3A_AF_FOCUS_VALUE,       // MINT64
  MTK_3A_PRV_CROP_REGION,      // MRect
  MTK_3A_ISP_MDP_TARGET_SIZE,  // MSize
  MTK_3A_ISP_CAPTURE_TARGET_SIZE,  // MSize
  MTK_3A_REPEAT_RESULT,        // MUINT8
  MTK_3A_SKIP_PRECAPTURE,  // MBOOL //if CUST_ENABLE_FLASH_DURING_TOUCH is true,
                           // MW can skip precapture
  MTK_3A_SKIP_BAD_FRAME,   // MBOOL
  MTK_3A_FLARE_IN_MANUAL_CTRL_ENABLE,  // MBOOL
  MTK_3A_DYNAMIC_SUBSAMPLE_COUNT,  // MINT32 30fps = 1, 60fps = 2, ... , 120fps
                                   // = 4
  MTK_3A_AE_LV_VALUE,              // MINT32
  MTK_APP_CONTROL,                 // MINT32
  MTK_3A_CUST_PARAMS,              // IMemory
  MTK_3A_SETTING_CUST_PARAMS,      // IMemory
  MTK_SENSOR_MODE_INFO_ACTIVE_ARRAY_CROP_REGION,  // MRect
  MTK_3A_AE_BV,                                   // MINT32
  MTK_3A_AE_CWV,                                  // MINT32
  MTK_3A_AE_HAL_EXPOSURE_COMPENSATION,            // MINT32, HAL EVC for preven-
                                                  // ting XTS fail. HAL EVC +
                                                  // AOSP EVC = Effective EVC.
  MTK_ISP_P2_PROCESSED_RAW,                       // MINT32
  MTK_3A_AE_SENSOR_FPS_RANGE,                     // MINT32
  MTK_3A_EXIF_METADATA = MTK_3A_EXIF_START,       // IMetadata
  MTK_EIS_REGION = MTK_EIS_START,                 // MINT32
  MTK_EIS_INFO,                                   // MINT64
  MTK_EIS_VIDEO_SIZE,                             // MRect
  MTK_EIS_NEED_OVERRIDE_TIMESTAMP,                // MBOOL
  MTK_EIS_LMV_DATA,                               // IMemory
  MTK_STEREO_JPS_MAIN1_CROP = MTK_STEREO_START,   // MRect
  MTK_STEREO_JPS_MAIN2_CROP,                      // MRect
  MTK_MULTI_CAM_SYNC2A_MODE,                      // MINT32
  MTK_MULTI_CAM_SYNCAF_MODE,                      // MINT32
  MTK_MULTI_CAM_HW_FRM_SYNC_MODE,                 // MINT32
  MTK_MULTI_CAM_NOTIFY,                           // MINT32
  MTK_MULTI_CAM_SYNC2A_MASTER_SLAVE,              // MINT32[2]
  MTK_MULTI_CAM_SYNC2A_PREVIOUS_MASTER,           // MINT32
  MTK_MULTI_CAM_SYNC2A_STATUS,                    // IMemory
  MTK_MULTI_CAM_3AAWB_MASTER,                     // MINT32
  MTK_MULTI_CAM_3AAWB_PREVIOUS_MASTER,            // MINT32
  MTK_MULTI_CAM_FOV_ALIGN_RESULT,                 // IMemory
  MTK_MULTI_CAM_3AAE_DISPLAY_INVALID,             // MBOOL
  MTK_JPG_ENCODE_TYPE,                            // MINT8
  MTK_CONVERGENCE_DEPTH_OFFSET,                   // MFLOAT
  MTK_N3D_WARPING_MATRIX_SIZE,                    // MUINT32
  MTK_P1NODE_MAIN2_HAL_META,                      // IMetadata
  MTK_P2NODE_BOKEH_ISP_PROFILE,                   // MUINT8
  MTK_STEREO_FEATURE_DENOISE_MODE,                // MINT32
  MTK_STEREO_FEATURE_SENSOR_PROFILE,              // MINT32
  MTK_P1NODE_MAIN2_APP_META,                      // IMetadata
  MTK_STEREO_FEATURE_OPEN_ID,                     // MINT32
  MTK_STEREO_FRAME_PER_CAPTURE,                   // MINT32
  MTK_STEREO_ENABLE_MFB,                          // MINT32
  MTK_STEREO_BSS_RESULT,                          // MINT32
  MTK_STEREO_FEATURE_FOV_CROP_REGION,  // MINT32[6] // p.x, p.y, p.w, p.h, srcW,
                                       // srcH
  MTK_STEREO_DCMF_FEATURE_MODE,        // MINT32    //
                                 // mtk_platform_metadata_enum_dcmf_feature_mode
  MTK_STEREO_HDR_EV,                             // MINT32
  MTK_STEREO_DELAY_FRAME_COUNT,                  // MINT32
  MTK_STEREO_DCMF_DEPTHMAP_SIZE,                 // MSize
  MTK_STEREO_WITH_CAMSV,                         // MBOOL
  MTK_MULTICAM_SENSOR_STREAMING_MODE,            // MINT32
  MTK_MULTICAM_LOW_FPS,                          // MINT32
  MTK_MULTICAM_SENSOR_RECONFIG_LIST,             // MINT32
  MTK_FRAMESYNC_ID = MTK_FRAMESYNC_START,        // MINT32
  MTK_FRAMESYNC_TOLERANCE,                       // MINT64
  MTK_FRAMESYNC_FAILHANDLE,                      // MINT32
  MTK_FRAMESYNC_RESULT,                          // MINT64
  MTK_FRAMESYNC_TYPE,                            // MINT32
  MTK_VHDR_LCEI_DATA = MTK_VHDR_START,           // Memory
  MTK_VHDR_IMGO_3A_ISP_PROFILE,                  // MUINT8
  MTK_VHDR_MULTIFRAME_TIMESTAMP,                 // MINT64
  MTK_VHDR_MULTIFRAME_EXPOSURE_TIME,             // MINT64
  MTK_HDR_FEATURE_HDR_HAL_MODE,                  // MINT32
  MTK_HDR_RECONFIG_SENSOR_MODE,                  // MINT32
  MTK_HDR_PROLONG_FRAME_LENGTH,                  // MINT32
  MTK_HDR_ME_META,                               // IMetadata
  MTK_HDR_SE_META,                               // IMetadata
  MTK_HDR_BLOB_IMGO_ORDER,                       // MUINT8
  MTK_PIPELINE_UNIQUE_KEY = MTK_PIPELINE_START,  // MINT32
  MTK_PIPELINE_FRAME_NUMBER,                     // MINT32
  MTK_PIPELINE_REQUEST_NUMBER,                   // MINT32
  MTK_PIPELINE_EV_VALUE,                         // MINT32
  MTK_PIPELINE_DUMP_UNIQUE_KEY,                  // MINT32
  MTK_PIPELINE_DUMP_FRAME_NUMBER,                // MINT32
  MTK_PIPELINE_DUMP_REQUEST_NUMBER,              // MINT32
  MTK_PIPELINE_VIDEO_RECORD,                     // MINT32
  MTK_MULTIFRAME_INDEX_ORDER_UNSURE,             // MUINT8 (MBOOL)
  MTK_NR_MODE = MTK_NR_START,                    // MINT32
  MTK_NR_MNR_THRESHOLD_ISO,                      // MINT32
  MTK_NR_SWNR_THRESHOLD_ISO,                     // MINT32
  MTK_REAL_LV,                                   // MINT32
  MTK_ANALOG_GAIN,                               // MUINT32
  MTK_AWB_RGAIN,                                 // MINT32
  MTK_AWB_GGAIN,                                 // MINT32
  MTK_AWB_BGAIN,                                 // MINT32
  MTK_PLUGIN_MODE = MTK_PLUGIN_START,            // MINT64
  MTK_PLUGIN_COMBINATION_KEY,                    // MINT64
  MTK_PLUGIN_P2_COMBINATION,                     // MINT64
  MTK_PLUGIN_PROCESSED_FRAME_COUNT,              // MINT32
  MTK_PLUGIN_CUSTOM_HINT,                        // MINT32
  MTK_PLUGIN_DETACT_JOB_SYNC_TOKEN,              // MINT64, may be not exists.
  MTK_PLUGIN_UNIQUEKEY,
  MTK_DUALZOOM_DROP_REQ = MTK_DUALZOOM_START,        // MINT32
  MTK_DUALZOOM_FORCE_ENABLE_P2,                      // MINT32
  MTK_DUALZOOM_DO_FRAME_SYNC,                        // MINT32
  MTK_DUALZOOM_ZOOM_FACTOR,                          // MINT32
  MTK_DUALZOOM_DO_FOV,                               // MINT32
  MTK_DUALZOOM_FOV_RECT_INFO,                        // MINT32
  MTK_DUALZOOM_FOV_CALB_INFO,                        // MINT32
  MTK_DUALZOOM_FOV_MARGIN_PIXEL,                     // MSize
  MTK_DUALCAM_AF_STATE,                              // MUINT8
  MTK_DUALCAM_LENS_STATE,                            // MUINT8
  MTK_DUALCAM_TIMESTAMP,                             // MINT64
  MTK_DUALZOOM_3DNR_MODE,                            // MINT32
  MTK_DUALZOOM_ZOOMRATIO,                            // MINT32
  MTK_DUALZOOM_CENTER_SHIFT,                         // MINT32
  MTK_DUALZOOM_FOV_RATIO,                            // MFLOAT
  MTK_DUALZOOM_REAL_MASTER,                          // MINT32
  MTK_DUALZOOM_FD_TARGET_MASTER,                     // MINT32
  MTK_DUALZOOM_FD_REAL_MASTER,                       // MINT32  // maybe not set
  MTK_LMV_SEND_SWITCH_OUT,                           // MINT32
  MTK_LMV_SWITCH_OUT_RESULT,                         // MINT32
  MTK_LMV_VALIDITY,                                  // MINT32
  MTK_VSDOF_P1_MAIN1_ISO,                            // MINT32
  MTK_DUALZOOM_IS_STANDBY,                           // MBOOL
  MTK_DUALZOOM_CAP_CROP,                             // MRect
  MTK_DUALZOOM_MASTER_UPDATE_MODE,                   // MBOOL
  MTK_DUALZOOM_STREAMING_NR,                         // MINT32
  MTK_FEATUREPIPE_APP_MODE = MTK_FEATUREPIPE_START,  // MINT32
  MTK_POSTPROC_TYPE = MTK_POSTPROC_START,            // MINT32
  MTK_FEATURE_STREAMING = MTK_FEATURE_START,         // MINT64
  MTK_FEATURE_CAPTURE,                               // MINT64
  MTK_FEATURE_CAPTURE_PHYSICAL,                      // MINT64
  MTK_FEATURE_MULTIFRAME_UNIQUE_KEY,                 // MINT32
  MTK_FEATURE_FREE_MEMORY_MBYTE,                     // MINT32
  MTK_FEATURE_MFNR_NVRAM_QUERY_INDEX,                // MINT32
  MTK_FEATURE_MFNR_NVRAM_DECISION_ISO,               // MINT32
  MTK_FEATURE_MFNR_TUNING_INDEX_HINT,                // MINT64
  MTK_FEATURE_MFNR_FINAL_EXP,                        // MINT32
  MTK_FEATURE_MFNR_OPEN_ID,                          // MINT32
  MTK_FEATURE_AISHUTTER2_PACK,                       // IMemory

  // bypass MSTMTNCInfo from AI capture to LPNR
  MTK_FEATURE_AI_SHUTTER_TNC_INFO,                   // IMemory
  MTK_FEATURE_VHDR_STATUS,                           // MINT32
  MTK_FEATURE_BSS_SELECTED_FRAME_COUNT,              // MINT32
  MTK_FEATURE_BSS_FORCE_DROP_NUM,                    // MINT32
  MTK_FEATURE_BSS_FIXED_LSC_TBL_DATA,                // MUINT8
  MTK_FEATURE_BSS_PROCESS,                           // MINT32
  MTK_FEATURE_BSS_ISGOLDEN,                          // MBOOL
  MTK_FEATURE_BSS_REORDER,                           // MBOOL
  MTK_FEATURE_BSS_MANUAL_ORDER,                      // MUINT8
  MTK_FEATURE_BSS_RRZO_DATA,                         // MUINT8
  MTK_FEATURE_BSS_DOWNSAMPLE,                        // MBOOL
  MTK_FEATURE_PACK_RRZO,                             // MUINT8
  MTK_FEATURE_PACK_YUVOR3,                           // MUINT8

  // refer to FDInfoGroup at ICaptureFeaturePipe.h
  MTK_FEATURE_CAPTURE_FACE_INFO,                     // IMemory

  MTK_FEATURE_CAP_YUV_PROCESSING,       // MUINT8
  MTK_FEATURE_CAP_PIPE_DCE_CONTROL,     // MUINT8
  MTK_FEATURE_MULTIFRAMENODE_BYPASSED,  // MUINT8
  MTK_FEATURE_FACE_APPLIED_TCY,       // MINT32
  MTK_FEATURE_FLIP_IN_P2A,              // MINT32
  MTK_FEATURE_CAP_PQ_USERID,            // MINT64
  MTK_FEATURE_VALID_IMAGE_INDEX,        // MINT32
  MTK_FSC_CROP_DATA = MTK_FSC_START,    // IMemory
  MTK_FSC_WARP_DATA,                    // IMemory
  MTK_TUNING_FEATURE_CAPTURE_HINT = MTK_TUNING_START,  // MINT64
  MTK_TUNING_FEATURE_STREAMING_HINT,                   // MINT64
  MTK_SENSOR_FEATURE_MODE,                             // MINT64
  MTK_NDD_INFORMATION_KEY,                             // MINT64
  MTK_NDD_ACTION_VALUE,                                // MINT32
  MTK_POSTPROC_APP_RESULT,                             // IMetadata
  MTK_NDD_AAO_BUFFER,                                  // Array with push_back
  MTK_NDD_AAHO_BUFFER,                                 // Array with push_back
  MTK_NDD_AISA_BUFFER,                                 // Array with push_back
  MTK_NDD_AIMA_BUFFER,                                 // Array with push_back
  MTK_NDD_AIFA_BUFFER,                                 // Array with push_back
  MTK_NDD_LTMSO_BUFFER,                                // Array with push_back
  MTK_NDD_LTM_ALG_BUFFER,                              // Array with push_back
  MTK_NDD_AE_ALG_BUFFER,                               // Array with push_back
  MTK_NDD_CAC_TBL_BUFFER                               // Array with push_back
} mtk_platform_metadata_tag_t;

/******************************************************************************
 *
 ******************************************************************************/
typedef enum mtk_platform_3a_exif_metadata_tag {
  MTK_3A_EXIF_FNUMBER,              // MINT32
  MTK_3A_EXIF_FOCAL_LENGTH,         // MINT32
  MTK_3A_EXIF_FOCAL_LENGTH_35MM,    // MINT32
  MTK_3A_EXIF_SCENE_MODE,           // MINT32
  MTK_3A_EXIF_AWB_MODE,             // MINT32
  MTK_3A_EXIF_LIGHT_SOURCE,         // MINT32
  MTK_3A_EXIF_EXP_PROGRAM,          // MINT32
  MTK_3A_EXIF_SCENE_CAP_TYPE,       // MINT32
  MTK_3A_EXIF_FLASH_LIGHT_TIME_US,  // MINT32
  MTK_3A_EXIF_AE_METER_MODE,        // MINT32
  MTK_3A_EXIF_AE_EXP_BIAS,          // MINT32
  MTK_3A_EXIF_CAP_EXPOSURE_TIME,    // MINT32
  MTK_3A_EXIF_AE_ISO_SPEED,         // MINT32
  MTK_3A_EXIF_REAL_ISO_VALUE,       // MINT32
  MTK_3A_EXIF_AE_BRIGHTNESS_VALUE,  // MINT32
  MTK_3A_EXIF_DEBUGINFO_BEGIN,      // debug info begin
  // key: MINT32
  MTK_3A_EXIF_DBGINFO_AAA_KEY = MTK_3A_EXIF_DEBUGINFO_BEGIN,  // MINT32
  MTK_3A_EXIF_DBGINFO_AAA_DATA,
  MTK_3A_EXIF_DBGINFO_SDINFO_KEY,
  MTK_3A_EXIF_DBGINFO_SDINFO_DATA,
  MTK_3A_EXIF_DBGINFO_ISP_KEY,
  MTK_3A_EXIF_DBGINFO_ISP_DATA,
  //
  MTK_CMN_EXIF_DBGINFO_KEY,
  MTK_CMN_EXIF_DBGINFO_DATA,
  //
  MTK_MF_EXIF_DBGINFO_MF_KEY,
  MTK_MF_EXIF_DBGINFO_MF_DATA,
  //
  MTK_N3D_EXIF_DBGINFO_KEY,
  MTK_N3D_EXIF_DBGINFO_DATA,
  //
  MTK_POSTNR_EXIF_DBGINFO_NR_KEY,
  MTK_POSTNR_EXIF_DBGINFO_NR_DATA,
  //
  MTK_RESVB_EXIF_DBGINFO_KEY,
  MTK_RESVB_EXIF_DBGINFO_DATA,
  //
  MTK_RESVC_EXIF_DBGINFO_KEY,
  MTK_RESVC_EXIF_DBGINFO_DATA,
  // data: Memory
  MTK_3A_EXIF_DEBUGINFO_END,  // debug info end
} mtk_platform_3a_exif_metadata_tag_t;

/******************************************************************************
 *
 ******************************************************************************/

typedef enum mtk_platform_metadata_p1_processedraw_type {
  MTK_P1_RAW_PURE = 0,
  MTK_P1_RAW_PROCESSED
} mtk_platform_metadata_p1_processedraw_type_t;

typedef enum mtk_platform_metadata_enum_nr_mode {
  MTK_NR_MODE_OFF = 0,
  MTK_NR_MODE_MNR,
  MTK_NR_MODE_SWNR,
  MTK_NR_MODE_AUTO
} mtk_platform_metadata_enum_nr_mode_t;

typedef enum mtk_platform_metadata_enum_mfb_mode {
  MTK_MFB_MODE_OFF = 0,
  MTK_MFB_MODE_MFLL,
  MTK_MFB_MODE_AIS,
  MTK_MFB_MODE_NUM,
} mtk_platform_metadata_enum_mfb_mode_t;

typedef enum mtk_platform_metadata_enum_custom_hint {
  MTK_CUSTOM_HINT_0 = 0,
  MTK_CUSTOM_HINT_1,
  MTK_CUSTOM_HINT_2,
  MTK_CUSTOM_HINT_3,
  MTK_CUSTOM_HINT_4,
  MTK_CUSTOM_HINT_NUM,
} mtk_platform_metadata_enum_custom_hint_t;

typedef enum mtk_platform_metadata_enum_plugin_mode {
  MTK_PLUGIN_MODE_COMBINATION = 1 << 0,
  MTK_PLUGIN_MODE_NR = 1 << 1,
  MTK_PLUGIN_MODE_HDR = 1 << 2,
  MTK_PLUGIN_MODE_MFNR = 1 << 3,
  MTK_PLUGIN_MODE_COPY = 1 << 4,
  MTK_PLUGIN_MODE_TEST_PRV = 1 << 5,
  MTK_PLUGIN_MODE_BMDN = 1 << 6,
  MTK_PLUGIN_MODE_MFHR = 1 << 7,
  MTK_PLUGIN_MODE_BMDN_3rdParty = 1 << 8,
  MTK_PLUGIN_MODE_MFHR_3rdParty = 1 << 9,
  MTK_PLUGIN_MODE_FUSION_3rdParty = 1 << 10,
  MTK_PLUGIN_MODE_VSDOF_3rdParty = 1 << 11,
  MTK_PLUGIN_MODE_COLLECT = 1 << 12,
  MTK_PLUGIN_MODE_HDR_3RD_PARTY = 1 << 13,
  MTK_PLUGIN_MODE_MFNR_3RD_PARTY = 1 << 14,
  MTK_PLUGIN_MODE_BOKEH_3RD_PARTY = 1 << 15,
  MTK_PLUGIN_MODE_DCMF_3RD_PARTY = 1 << 16,
} mtk_platform_metadata_enum_plugin_mode_t;

typedef enum mtk_platform_metadata_enum_seamless_switch_policy {
      MTK_SEAMLESS_SWITCH_POLICY_ABANDON        = 0,
      MTK_SEAMLESS_SWITCH_POLICY_NON_ABANDON    = 1,
} mtk_platform_metadata_enum_seamless_switch_policy;

typedef enum mtk_platform_metadata_enum_p2_plugin_combination {
  MTK_P2_RAW_PROCESSOR = 1 << 0,
  MTK_P2_ISP_PROCESSOR = 1 << 1,
  MTK_P2_YUV_PROCESSOR = 1 << 2,
  MTK_P2_MDP_PROCESSOR = 1 << 3,
  MTK_P2_CAPTURE_REQUEST = 1 << 4,
  MTK_P2_PREVIEW_REQUEST = 1 << 5
} mtk_platform_metadata_enum_p2_plugin_combination;

typedef enum mtk_platform_metadata_enum_isp_color_space {
  MTK_ISP_COLOR_SPACE_SRGB = 0,
  MTK_ISP_COLOR_SPACE_DISPLAY_P3 = 1,
  MTK_ISP_COLOR_SPACE_CUSTOM_1 = 2
} mtk_platform_metadata_enum_isp_color_space;

typedef enum mtk_platform_metadata_enum_dualzoom_drop_req {
  MTK_DUALZOOM_DROP_NEVER_DROP = 0,
  MTK_DUALZOOM_DROP_NONE = 1,
  MTK_DUALZOOM_DROP_DIRECTLY = 2,
  MTK_DUALZOOM_DROP_NEED_P1,
  MTK_DUALZOOM_DROP_NEED_SYNCMGR,
  MTK_DUALZOOM_DROP_NEED_SYNCMGR_NEED_STREAM_F_PIPE,
} mtk_platform_metadata_enum_dualzoom_drop_req_t;

typedef enum mtk_platform_metadata_enum_p1_sensor_status {
  MTK_P1_SENSOR_STATUS_NONE = 0,
  MTK_P1_SENSOR_STATUS_STREAMING = 1,
  MTK_P1_SENSOR_STATUS_SW_STANDBY = 2,
  MTK_P1_SENSOR_STATUS_HW_STANDBY = 3,
} mtk_platform_metadata_enum_p1_sensor_status_t;

typedef enum mtk_platform_metadata_enum_p1_twin_switch {
  MTK_P1_TWIN_SWITCH_NONE = 0,
  MTK_P1_TWIN_SWITCH_ONE_TG = 1,
  MTK_P1_TWIN_SWITCH_TWO_TG = 2
} mtk_platform_metadata_enum_p1_twin_switch_t;

typedef enum mtk_platform_metadata_enum_p1_twin_status {
  MTK_P1_TWIN_STATUS_NONE = 0,
  MTK_P1_TWIN_STATUS_TG_MODE_1 = 1,
  MTK_P1_TWIN_STATUS_TG_MODE_2 = 2,
  MTK_P1_TWIN_STATUS_TG_MODE_3 = 3,
} mtk_platform_metadata_enum_p1_twin_status_t;

typedef enum mtk_platform_metadata_enum_p1_resize_quality_switch {
  MTK_P1_RESIZE_QUALITY_SWITCH_NONE = 0,
  MTK_P1_RESIZE_QUALITY_SWITCH_L_L = 1,
  MTK_P1_RESIZE_QUALITY_SWITCH_L_H = 2,
  MTK_P1_RESIZE_QUALITY_SWITCH_H_L = 3,
  MTK_P1_RESIZE_QUALITY_SWITCH_H_H = 4,
} mtk_platform_metadata_enum_p1_resize_quality_switch_t;

typedef enum mtk_platform_metadata_enum_p1_resize_quality_status {
  MTK_P1_RESIZE_QUALITY_STATUS_NONE = 0,
  MTK_P1_RESIZE_QUALITY_STATUS_ACCEPT = 1,
  MTK_P1_RESIZE_QUALITY_STATUS_IGNORE = 2,
  MTK_P1_RESIZE_QUALITY_STATUS_REJECT = 3,
  MTK_P1_RESIZE_QUALITY_STATUS_ILLEGAL = 4,
} mtk_platform_metadata_enum_p1_resize_quality_status_t;

typedef enum mtk_platform_metadata_enum_p1_resize_quality_level {
  MTK_P1_RESIZE_QUALITY_LEVEL_UNKNOWN = 0,
  MTK_P1_RESIZE_QUALITY_LEVEL_L = 1,
  MTK_P1_RESIZE_QUALITY_LEVEL_H = 2,
} mtk_platform_metadata_enum_p1_resize_quality_level_t;

typedef enum mtk_platform_metadata_enum_lmv_result {
  MTK_LMV_RESULT_OK = 0,
  MTK_LMV_RESULT_FAILED,
  MTK_LMV_RESULT_SWITCHING
} mtk_platform_metadata_enum_lmv_result_t;

typedef enum mtk_platform_metadata_enum_featurepipe_app_mode {
  MTK_FEATUREPIPE_PHOTO_PREVIEW = 0,
  MTK_FEATUREPIPE_VIDEO_PREVIEW = 1,
  MTK_FEATUREPIPE_VIDEO_RECORD = 2,
  MTK_FEATUREPIPE_VIDEO_STOP = 3,
} mtk_platform_metadata_enum_featurepipe_app_mode_t;

typedef enum mtk_platform_metadata_enum_dcmf_feature_mode {
  MTK_DCMF_FEATURE_BOKEH = 0,
  MTK_DCMF_FEATURE_MFNR_BOKEH = 1,
  MTK_DCMF_FEATURE_HDR_BOKEH = 2,
} mtk_platform_metadata_enum_dcmf_feature_mode_t;

typedef enum mtk_platform_metadata_enum_smvr_fps {
  MTK_SMVR_FPS_30 = 0,
  MTK_SMVR_FPS_120 = 1,
  MTK_SMVR_FPS_240 = 2,
  MTK_SMVR_FPS_480 = 3,
  MTK_SMVR_FPS_960 = 4,
} mtk_platform_metadata_enum_smvr_fps_t;

// MTK_FRAMESYNC_FAILHANDLE
typedef enum mtk_platform_metadata_enum_fremesync_failhandle {
  MTK_FRAMESYNC_FAILHANDLE_CONTINUE,
  MTK_FRAMESYNC_FAILHANDLE_DROP,
} mtk_platform_metadata_enum_fremesync_failhandle_t;

// MTK_FRAMESYNC_RESULT
typedef enum mtk_platform_metadata_enum_fremesync_result {
  MTK_FRAMESYNC_RESULT_PASS,
  MTK_FRAMESYNC_RESULT_FAIL_CONTINUE,
  MTK_FRAMESYNC_RESULT_FAIL_DROP,
} mtk_platform_metadata_enum_fremesync_result_t;

// MTK_FEATURE_MULTIFRAMENODE_BYPASSED
typedef enum mtk_platform_metadata_enum_multiframenode_bypassed {
  MTK_FEATURE_MULTIFRAMENODE_NOT_BYPASSED = 0,
  MTK_FEATURE_MULTIFRAMENODE_TO_BE_BYPASSED = 1
} mtk_platform_metadata_enum_mfllnode_bypassed_t;

// MTK_FEATURE_VHDR_STATUS
typedef enum mtk_platform_metadata_enum_vhdr_status {
  MTK_FEATURE_VHDR_STATUS_NON = 0,
  MTK_FEATURE_VHDR_STATUS_1EXP = 1,
  MTK_FEATURE_VHDR_STATUS_2EXP = 2,
  MTK_FEATURE_VHDR_STATUS_3EXP = 3
} mtk_platform_metadata_enum_vhdr_status_t;

// MTK_FEATURE_BSS_PROCESS
typedef enum mtk_platform_metadata_enum_bss_processing {
  MTK_FEATURE_BSS_PROCESS_ENABLE = 0,
  MTK_FEATURE_BSS_PROCESS_DISABLE = 1
} mtk_platform_metadata_enum_bss_processing_t;

// MTK_FEATURE_BSS_MANUAL_ORDER
typedef enum mtk_platform_metadata_enum_bss_manual_order {
  MTK_FEATURE_BSS_MANUAL_ORDER_OFF = 0,
  MTK_FEATURE_BSS_MANUAL_ORDER_GOLDEN = 1
} mtk_platform_metadata_enum_bss_manual_order_t;

// MTK_FEATURE_CAP_YUV_PROCESSING
typedef enum mtk_platform_metadata_enum_cap_yuv_processing {
  MTK_FEATURE_CAP_YUV_PROCESSING_NOT_NEEDED = 0,
  MTK_FEATURE_CAP_YUV_PROCESSING_NEEDED = 1
} mtk_platform_metadata_enum_cap_yuv_processing_t;

// MTK_FEATURE_CAP_PIPE_DCE_CONTROL
typedef enum mtk_platform_metadata_enum_cap_pipe_control {
  MTK_FEATURE_CAP_PIPE_DCE_ENABLE_BUT_NOT_APPLY = 2,
  MTK_FEATURE_CAP_PIPE_DCE_MANUAL_DISABLE = 1,
  MTK_FEATURE_CAP_PIPE_DCE_DEFAULT_APPLY = 0
} mtk_platform_metadata_enum_cap_pipe_dce_control_t;

// MTK_ISP_P2_PROCESSED_RAW
typedef enum mtk_platform_metadata_enum_p2_processed_raw {
  MTK_ISP_P2_PROCESSED_RAW_NOT_NEEDED = 0,
  MTK_ISP_P2_PROCESSED_RAW_NEEDED = 1
} mtk_platform_metadata_enum_p2_processed_raw_t;

// MTK_DUALZOOM_STREAMING_NR
typedef enum mtk_platform_metadata_enum_dualzoom_streaming_nr {
  MTK_DUALZOOM_STREAMING_NR_AUTO = 0,
  MTK_DUALZOOM_STREAMING_NR_OFF = 1
} mtk_platform_metadata_enum_dualzoom_streaming_nr_t;

// MTK_MULTICAM_SENSOR_STREAMING_MODE
typedef enum mtk_platform_metadata_enum_multicam_sensor_streaming_mode {
  MTK_MULTICAM_SENSOR_STREAMING_OFF = 0,
  MTK_MULTICAM_SENSOR_STREAMING_KEEP = 1,
  MTK_MULTICAM_SENSOR_STREAMING_ON = 2
} mtk_platform_metadata_enum_multicam_sensor_streaming_mode_t;

// MTK_MULTICAM_LOW_FPS
typedef enum mtk_platform_metadata_enum_multicam_low_fps {
  MTK_MULTICAM_LOW_FPS_OFF = 0,
  MTK_MULTICAM_LOW_FPS_ON = 1
} mtk_platform_metadata_enum_multicam_low_fps_t;

// MTK_3A_FEATURE_AE_EXPOSURE_LEVEL
typedef enum mtk_platform_metadata_enum_ae_exposure_level {
  MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_NONE = 0,
  MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_DUMMY,
  MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_SHORT_SHORT,
  MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_SHORT,
  MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_NORMAL,
  MTK_3A_FEATURE_AE_EXPOSURE_LEVEL_LONG,
} mtk_platform_metadata_enum_ae_exposure_level_t;

// MTK_3A_FEATURE_AE_TARGET_MODE
typedef enum mtk_platform_metadata_enum_ae_target_mode {
  MTK_3A_FEATURE_AE_TARGET_MODE_NORMAL = 0,
  MTK_3A_FEATURE_AE_TARGET_MODE_IVHDR,
  MTK_3A_FEATURE_AE_TARGET_MODE_MVHDR,
  MTK_3A_FEATURE_AE_TARGET_MODE_ZVHDR,
  MTK_3A_FEATURE_AE_TARGET_MODE_LE_FIX,
  MTK_3A_FEATURE_AE_TARGET_MODE_SE_FIX,
  MTK_3A_FEATURE_AE_TARGET_MODE_4CELL_MVHDR,
  MTK_3A_FEATURE_AE_TARGET_MODE_MSTREAM_VHDR,
  MTK_3A_FEATURE_AE_TARGET_MODE_MSTREAM_VHDR_RTO1X,
  MTK_3A_FEATURE_AE_TARGET_MODE_STAGGER_2EXP,
  MTK_3A_FEATURE_AE_TARGET_MODE_STAGGER_3EXP,
} mtk_platform_metadata_enum_ae_target_mode_t;

// MTK_3A_FEATURE_AE_VALID_EXPOSURE_NUM
typedef enum mtk_camera_metadata_enum_stagger_valid_exposure_num {
    MTK_STAGGER_VALID_EXPOSURE_NON = 0,
    MTK_STAGGER_VALID_EXPOSURE_1 = 1,
    MTK_STAGGER_VALID_EXPOSURE_2 = 2,
    MTK_STAGGER_VALID_EXPOSURE_3 = 3
} mtk_camera_metadata_enum_stagger_valid_exposure_num_t;

// MTK_3A_ISP_FUS_NUM
typedef enum mtk_camera_metadata_enum_3a_isp_fus_num {
  MTK_3A_ISP_FUS_NUM_NON = 0,
  MTK_3A_ISP_FUS_NUM_1 = 1,
  MTK_3A_ISP_FUS_NUM_2 = 2,
  MTK_3A_ISP_FUS_NUM_3 = 3,
} mtk_camera_metadata_enum_3a_isp_fus_num_t;

// MTK_HDR_BLOB_IMGO_ORDER
typedef enum mtk_platform_metadata_enum_hdr_blob_imgo_order {
    MTK_HDR_IMGO_NONE = 0,
    MTK_HDR_IMGO_NE = 1,
    MTK_HDR_IMGO_ME = 2,
    MTK_HDR_IMGO_SE = 3
} mtk_platform_metadata_enum_hdr_blob_imgo_order_t;

// MTK_P2NODE_VIDEO_AINR_RUNMODE
typedef enum mtk_camera_metadata_enum_video_ainr_runmode {
  MTK_P2NODE_VIDEO_AINR_RUNMODE_OFF = 0,
  MTK_P2NODE_VIDEO_AINR_RUNMODE_TRANSITION = 1,
  MTK_P2NODE_VIDEO_AINR_RUNMODE_ON = 2,
} mtk_camera_metadata_enum_video_ainr_runmode_t;


#endif  // INCLUDE_MTKCAM_INTERFACES_UTILS_METADATA_HAL_MTK_PLATFORM_METADATA_TAG_H_

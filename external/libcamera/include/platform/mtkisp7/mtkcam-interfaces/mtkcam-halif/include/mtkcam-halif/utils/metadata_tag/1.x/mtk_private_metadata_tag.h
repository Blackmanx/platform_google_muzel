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

#ifndef INCLUDE_MTKCAM_HALIF_UTILS_METADATA_TAG_1_X_MTK_PRIVATE_METADATA_TAG_H_
#define INCLUDE_MTKCAM_HALIF_UTILS_METADATA_TAG_1_X_MTK_PRIVATE_METADATA_TAG_H_

#include "mtkcam-halif/utils/metadata_tag/1.x/mtk_metadata_tag.h"

typedef enum mtk_private_camera_metadata_section {
 /**
  * Mediatek Camera HAL Private tags.
  *
  * The client of camera hal process must not recognize these tags. Camera hal
  * process must block and not transfer these tags to its client.
  *
  *  @par Range
  *       - 0xA0000000 - 0xAFFFFFFF
  */
  MTK_PRIVATE_SECTION         = 0xA000,
  MTK_HALCORE_FEATURE         = 0,
  MTK_POSTPROCDEV_FEATURE     = 1,
  MTK_PRIVATE_3A_FEATURE      = 2,
  MTK_PRIVATE_COUNT,
  MTK_PRIVATE_SECTION_END     = 0xAFFF,
} mtk_private_camera_metadata_section_t;

/**
 * Hierarchy positions in enum space. All vendor extension tags must be
 * defined with tag >= VENDOR_SECTION_START
 */
typedef enum mtk_private_camera_metadata_section_start {
  MTK_PRIVATE_SECTION_START = MTK_PRIVATE_SECTION << 16,
  MTK_HALCORE_FEATURE_START = (MTK_HALCORE_FEATURE +
                                MTK_PRIVATE_SECTION) << 16,

  MTK_POSTPROCDEV_FEATURE_START = (MTK_POSTPROCDEV_FEATURE +
                                   MTK_PRIVATE_SECTION) << 16,
  MTK_PRIVATE_3A_FEATURE_START = (MTK_PRIVATE_3A_FEATURE +
                          MTK_PRIVATE_SECTION) << 16,
} mtk_private_camera_metadata_section_start_t;

/**
 * Section start for postproc device
 */
typedef enum mtk_camera_metadata_postprocdev_section_start {
  /* PostProcDev VSE
   * uses range MTK_POSTPROCDEV_FEATURE_START | 0x1000 - 0x1FFF
   */
  MTK_POSTPROCDEV_VSE_FEATURE_START =
    MTK_POSTPROCDEV_FEATURE_START + 0x1000,

  /* PostProcDev WPEPQ
   * uses range MTK_POSTPROCDEV_FEATURE_START | 0x2000 - 0x2FFF
   */
  MTK_POSTPROCDEV_WPEPQ_FEATURE_START =
    MTK_POSTPROCDEV_FEATURE_START + 0x2000,

  /* PostProcDev CAPTURE
   * uses range MTK_POSTPROCDEV_FEATURE_START | 0x3000 - 0x3FFF
   */
  MTK_POSTPROCDEV_CAPTURE_FEATURE_START =
    MTK_POSTPROCDEV_FEATURE_START + 0x3000,

  /* PostProcDev BOKEH
   * uses range MTK_POSTPROCDEV_FEATURE_START | 0x4000 - 0x4FFF
   */
  MTK_POSTPROCDEV_BOKEH_FEATURE_START =
    MTK_POSTPROCDEV_FEATURE_START + 0x4000,

  /* PostProcDev EIS
   * uses range MTK_POSTPROCDEV_FEATURE_START | 0x5000 - 0x5FFF
   */
  MTK_POSTPROCDEV_EIS_FEATURE_START =
    MTK_POSTPROCDEV_FEATURE_START + 0x5000,

  /* The next PostProcDev
   * uses MTK_POSTPROCDEV_FEATURE_START | 0x6000 - 0x6FFF
  MTK_POSTPROCDEV_XXX_FEATURE_START = MTK_POSTPROCDEV_FEATURE_START +
                                       0x6000,*/
} mtk_camera_metadata_postprocdev_section_start_t;


/**
 * Main enum for defining camera metadata tags.  New entries must always go
 * before the section _END tag to preserve existing enumeration values.  In
 * addition, the name and type of the tag needs to be added to
 * ""
 */
typedef enum mtk_private_camera_metadata_tag {
  /**
   * Mediatek HALCore vendor tags.
   *
   * Available stream sources mapping to ISP DMA ports.
   *  @par Type
   *       int64_t[]
   *  @par Category
   *       static
   *  @par Valid timing
   *       launch
   *  @par Value
   *       Value from `mtk_camera_metadata_enum_halcore_stream_source`
   */
  MTK_HALCORE_AVAILABLE_STREAM_SOURCES = MTK_HALCORE_FEATURE_START,

  /**
   * Stream sources configured by user.
   *  @par Type
   *       int64_t
   *  @par Category
   *       control
   *  @par Valid timing
   *       configure
   *  @par Value
   *       Value from `mtk_camera_metadata_enum_halcore_stream_source`
   */
  MTK_HALCORE_STREAM_SOURCE,

  /**
   * Return sensor size and stride of decided sensor mode
   *  @par Type
   *       int32_t
   *  @par Category
   *       Configured result
   *  @par Valid timing
   *       configure
   *  @par Value
   *       Configured sensor information
   *  @arg [0]: sensor id
   *  @arg [1]: sensor width
   *  @arg [2]: sensor height
   *  @arg [3]: sensor stride
   *  @arg [N + 0]: sensor id
   *  @arg [N + 1]: sensor width
   *  @arg [N + 2]: sensor height
   *  @arg [N + 3]: sensor stride
   */
  MTK_HALCORE_CONFIGURED_SENSOR_INFO,

  /**
   * Allow CoreSession modify size of full raw port. Custom zone need follow
   * configured sesnor information to change full raw buffer size.
   * Default is 0.
   *  @par Type
   *       uint8_t
   *  @par Category
   *       Control
   *  @par Valid timing
   *       configure
   *  @par Value
   *  @arg 0: CoreSession must not modify full-raw setting.
   *  @arg 1: CoreSession could modify full-raw setting.
   */
  MTK_HALCORE_ALLOW_MODIFY_FULL_RAW_SETTING,

  /**
   * Allow user to set specific sensor mode for Core Session.
   * User could set the known sensor mode which predefined by MTK Camera HAL or
   * set a custom sensor mode, like 0x101 means custom1 mode and 0x102 meas
   * custom2 mode...etc.
   *  @par Type
   *       int32_t
   *  @par Category
   *       Control
   *       Configured result
   *  @par Valid timing
   *       configure
   *  @par Value
   *  @arg [0]: sensor id
   *  @arg [1]: sensor status value is in
   *    `mtk_camera_metadata_enum_halcore_sensor_mode`
   *  @arg [N + 0]: sensor id
   *  @arg [N + 1]: sensor status value is in
   *    `mtk_camera_metadata_enum_halcore_sensor_mode`
   *  @see mtk_camera_metadata_enum_halcore_sensor_mode
   */
  MTK_HALCORE_CONFIGURE_SENSOR_MODE,

  /**
   * TBD.
   */
  MTK_HALCORE_AVAILABLE_STREAM_SOURCE_FORMATS_MAP,

  /**
   * Describe crop info for specific stream.
   *  @par Type
   *       int32_t[]
   *  @par Category
   *       control
   *  @par Valid timing
   *       request
   *  @par value
   *       active array domain
   *  @arg [0]: X position
   *  @arg [1]: Y position
   *  @arg [2]: width
   *  @arg [3]: height
   */
  MTK_HALCORE_STREAM_CROP_REGION,

  /**
   * Describe physical sensor status in logical settings.
   * All sensors should be filled in this metadata.
   *  @par Type
   *       int32_t[]
   *  @par Category
   *       control
   *  @par Valid timing
   *       configure/request
   *  @par Value
   *    sensor status value is in
   *    `mtk_camera_metadata_enum_halcore_physical_sensor_status`
   *  @arg [0]: physical id
   *  @arg [1]: sensor status
   *  @arg [N + 0]: physical id
   *  @arg [N + 1]: sensor status
   */
  MTK_HALCORE_PHYSICAL_SENSOR_STATUS,

  /**
   * Describe master id for preview in logical settings.
   *  @par Type
   *       int32_t
   *  @par Category
   *       control
   *  @par Valid timing
   *       request
   *  @par Value
   *    preview master id
   */
  MTK_HALCORE_PHYSICAL_MASTER_ID,

  /**
   * Describe physical sensors which need framesync in logical settings.
   *  @par Type
   *       int32_t[]
   *  @par Category
   *       control
   *  @par Valid timing
   *       request
   *  @par Value
   *  @arg [0]: Physical id
   *  @arg [N]: Physical id
   */
  MTK_HALCORE_PHYSICAL_FRAMESYNC_ID,

  /**
   * Describe the batch size for SMVR.
   *  @par Type
   *       int32_t
   *  @par Category
   *       control
   *  @par Valid timing
   *       configure/request
   *  @par Value
   *       batch size
   */
  MTK_HALCORE_BATCH_SIZE,

  /**
   * TBD.
   */
  MTK_HALCORE_BUFFER_OFFSETS,

  /**
   * Control reselution of private real-time stream in configure time.
   *  @par Type
   *       int32_t[]
   *  @par Category
   *       control
   *  @par Valid timing
   *       configure
   *  @par Value
   *       active array domain
   *  @arg [0]: width
   *  @arg [1]: height
   */
  MTK_HALCORE_REALTIME_SOURCE_RESOLUTION,

  /**
   * Available image processing settings hint.
   *  @par Type
   *       int32_t[]
   *  @par Category
   *       static
   *  @par Valid timing
   *       launch
   *  @par Value
   *       Value in
   *       `mtk_camera_metadata_enum_halcore_available_image_processing_settings`
   */
  MTK_HALCORE_AVAILABLE_IMAGE_PROCESSING_SETTINGS,

  /**
   * Image processing settings hint for specific stream.
   *  @par Type
   *       int32_t
   *  @par Category
   *       control
   *  @par Valid timing
   *       configure/request
   *  @par Value
   *       Bitwise value from
   *       `mtk_camera_metadata_enum_halcore_available_image_processing_settings`
   */
  MTK_HALCORE_IMAGE_PROCESSING_SETTINGS,

  /**
   * Describe this control is repeating settings or not.
   *  @par Type
   *       uint8_t
   *  @par Category
   *       control
   *  @par Valid timing
   *       request
   *  @par Value
   *  @arg 0 for false
   *  @arg otherwise for true
   */
  MTK_HALCORE_ISREAPEATING_SETTING,

  MTK_HALCORE_VIDEO_AFBC,

  /**
   * Component considers that this result is urgent to callback to client.
   *  @par Type
   *       uint8_t
   *  @par Category
   *       result
   *  @par Valid timing
   *       callback
   *  @par Value
   *  @arg 0 for NONE, 1 for SOFT, 2 for HARD
   *  @arg otherwise for true
   */
  MTK_HALCORE_CALLBACK_REALTIME_MODE,

  /**
  * Configuring time bit mask to partially enable Custom Zone refine
  * MTK HAL CoreSession callback rule. The tag must be set into sessionParam
  * in configuration level.
  * @par Type
  *      int64_t
  *
  * @par Valid Timing
  *      - configure
  *
  * @par Unit
  *      bitwise
  *
  * @par Possible Values
  *      bitwise value from mtk_camera_metadata_enum_halcore_callback_rule
  */
  MTK_HALCORE_CALLBACK_RULE,

  /**
   * Request end hint for each component if there's no metadata callbacked.
   *  @par Type
   *       uint8_t
   *  @par Category
   *       result
   *  @par Valid timing
   *       callback
   *  @par Value
   *       N/A
   */
  MTK_HALCORE_REQUEST_END,

  /**
   * Describe power on stage of sensor
   *  @par Type
   *       int32_t
   *  @par Category
   *       control
   *  @par Valid timing
   *       configure/request
   *  @par Value
   *       Value in
   *       `mtk_camera_metadata_enum_halcore_poweron_stage`
   */
  MTK_HALCORE_POWERON_STAGE,

  /**
   * Describing platform support HAL buffer management or not.
   *  @par Type
   *       int32_t
   *  @par Category
   *       static
   *  @par Valid timing
   *       launch
   *  @par Value
   *  @arg 0 for not support HAL buffer management
   *  @arg 1 for support HAL buffer management
   */
  MTK_HALCORE_SUPPORT_HAL_BUFFER_MANAGEMENT,

  /**
   * The available strategies of HAL buffer management.
   * - preparatory  : request buffers at receiving requests time.
   * - immediate    : request buffers at about to write buffers time.
   * - cached       : request some extra buffers before request time.
   *                  request another buffers with immediate strategy.
   *  @par Type
   *       int32_t[]
   *  @par Category
   *       static
   *  @par Valid timing
   *       launch
   *  @par Value
   *  @arg 0 for preparatory
   *  @arg 1 for immediate
   *  @arg 2 for cached
   */
  MTK_HALCORE_HBM_AVAILABLE_STRATEGIES,

  /**
   * Describing hal buffer managemant strategy of a stream.
   *  @par Type
   *       int32_t
   *  @par Category
   *       control
   *  @par Valid timing
   *       configure
   *  @par Value
   *  @arg 0 for preparatory
   *  @arg 1 for immediate
   *  @arg 2 for cached
   */
  MTK_HALCORE_HBM_STRATEGY,

  /**
   * Describing the cach count of a stream in hal buffer cached mode.
   *  @par Type
   *       int32_t
   *  @par Category
   *       control
   *  @par Valid timing
   *       configure
   *  @par Value
   *       Non-negative integer
   */
  MTK_HALCORE_HBM_STRATEGY_CACHE_COUNT,

  /**
   *  Hint for raw type which is used by algo(ex: APU/GPU) as input
   *
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Config
   *       - Request
   *  @par Value
   *       - 0: Raw is fully processed
   *       - 1: Raw is non-fully processed
   */
  MTK_HALCORE_NON_FULL_PROCESSED_RAW_HINT,

  /**
   * User can choose Core Engine direct couple depends on APU requirements
   *
   *  @par Type
   *       int32_t
   *  @par Category
   *       control
   *  @par Valid timing
   *       - Config
   *  @par Value
   *       - 0: disable P1 direct couple
   *       - 1: enable  P1 direct couple
   */
  MTK_HALCORE_APU_DIRECT_COUPLE_HINT,

  /**
   * Hint APU driver the value of Core Engine APU Direct Couple task token from APU framework
   *
   *  @par Type
   *       int64_t
   *  @par Category
   *       control
   *  @par Valid Timing
   *       - Request
   *
   */
  MTK_HALCORE_APU_DIRECT_COUPLE_TOKEN_ID,

  MTK_HALCORE_FEATURE_END,

  //  for postproc device
  MTK_POSTPROCDEV_DEVICE_TYPE = MTK_POSTPROCDEV_FEATURE_START,

  /**
   * Describe the stream id for isp hidl adaptor.
   *  @par Type
   *  MINT64
   *
   *  @par Valid Timing
   *  @arg Config
   *
   *  @par Unit
   *  pixel in main source image
   *
   *  @par Possible Value
   *  stream id for rrzo/lcso/tuning stream/etc...
   */
  MTK_POSTPROCDEV_REAL_STREAM_ID,

  /**
   * Describe postproc device print fps log or not.
   *  @par Type
   *  MINT32
   *
   *  @par Valid Timing
   *  @arg Config
   *
   *  @par Possible Value
   *       - @c 0 Disable
   *       - @c 1 Enable
   */
  MTK_POSTPROCDEV_ENABLE_FPSLOG,

  /**
   * Describe tuning action value of postproc device.
   *  @par Type
   *  MINT32
   *
   *  @par Valid Timing
   *  @arg Per-frame
   *
   *  @par Possible Value
   *       Value from
   *       `mtk_camera_metadata_enum_postprocdev_tuning_action`
   *  @see mtk_camera_metadata_enum_postprocdev_tuning_action
   */
  MTK_POSTPROCDEV_TUNING_ACTION,
  MTK_POSTPROCDEV_FEATURE_END,

  // Mediatek Video Stream Engine vendor tags.

  /**
   * Configuring time value to describe features to be enabled, if the given
   * feature was not supported, configuration will fail.
   *
   *  @par Type
   *       int64_t
   *  @par Valid Timing
   *       - Config
   *  @par Value
   *       Value from
   *       `mtk_camera_metadata_enum_postprocdev_vse_available_features`.
   *  @see mtk_camera_metadata_enum_postprocdev_vse_available_features
   */
  MTK_POSTPROCDEV_VSE_CONFIG_FEATURES = MTK_POSTPROCDEV_VSE_FEATURE_START,

  /**
   * TBD, to report the current platform streaming feature set support list.
   *  @par Type
   *       int64_t
   *  @par Valid Timing
   *       - Config
   *  @par Value from
   *       Value from
   *       `mtk_camera_metadata_enum_postprocdev_vse_available_features`
   *  @see mtk_camera_metadata_enum_postprocdev_vse_available_features
   */
  MTK_POSTPROCDEV_VSE_AVAILABLE_FEATURES,

  /**
   * Video Stream Engine supports crop function at output image(s). Describe the
   * given output image's ROI (domain is based on the given source image), if
   * not given, use the full source image size as ROI. The destination image
   * size depends on the image buffer sent from caller, Video Stream Engine
   * will scale to the target size (unit is pixel).
   *  @par Type
   *       int64_t[]
   *  @par Valid Timing
   *       - Per-frame
   *  @par Unit
   *       Pixel in main source image.
   *  @par Value
   *       - [0]: Output stream ID
   *       - [1]: ROI's X position
   *       - [2]: ROI's Y position
   *       - [3]: ROI's width
   *       - [4]: ROI's height
   *       - [N + 0]: Output stream ID
   *       - [N + 1]: ROI's X position
   *       - [N + 2]: ROI's Y position
   *       - [N + 3]: ROI's width
   *       - [N + 4]: ROI's height
   */
  MTK_POSTPROCDEV_VSE_DESTINATION_IMAGE_ROI,

  /**
   * Video Stream Engine supports rotation (0 / 90 / 180 / 270) at the output
   * image(s). Describe the given output image's orientation. If not given, use
   * 0 as default.
   *  @par Type
   *       int64_t[]
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       - [0]: Stream ID
   *       - [1]: Orientation value
   *       - [N + 0]: Stream ID
   *       - [N + 1]: Orientation value
   */
  MTK_POSTPROCDEV_VSE_DESTINATION_IMAGE_ORIENTATION,

  /**
   * Features from Video Stream Engine may have specific controls. Caller can
   * use this tag to specify controls.
   *  @par Type
   *       int64_t[]
   *  @par Valid Timing
   *       Base on description from
   *       `mtk_camera_metadata_enum_postprocdev_vse_feature_control`.
   *  @par Value
   *       - [0]: `mtk_camera_metadata_enum_postprocdev_vse_feature_control`
   *       - [1]: value of [0].
   *       - [N]: `mtk_camera_metadata_enum_postprocdev_vse_feature_control`
   *       - [N + 1]: value of [N]
   *  @see mtk_camera_metadata_enum_postprocdev_vse_feature_control
   */
  MTK_POSTPROCDEV_VSE_FEATURE_CONTROL,

  /**
   * Video Stream Engine supports external tuning hint. This tag can be NULL,
   * use `MTK_POSTPROCDEV_VSE_TUNING_HINT_DEFAULT` as default.
   *  @par Type
   *       int64_t[]
   *  @par Valid Timing
   *       - Config
   *       - Per-frame
   *  @par Value
   *       - [0]: Stream ID
   *       - [1]: Value from
   *              `mtk_camera_metadata_enum_postprocdev_vse_tuning_hint`
   *  @see mtk_camera_metadata_enum_postprocdev_vse_tuning_hint
   */
  MTK_POSTPROCDEV_VSE_TUNING_HINT,

  /**
   * This tag describes the relationship between Stream IDs and feature-based
   * I/O. For the case of MCNR, streaming postproc device needs to know the
   * stream usage of input streams. Caller can use this tag to define each
   * stream usage in input streams during configure stage. For example, the
   * first stream is F0 and the second stream is ME.
   *  @par Type
   *       int64_t[]
   *  @par Valid Timing
   *       - Config
   *  @par Value
   *       - [0]: stream ID
   *       - [1]: value from
   *              `mtk_camera_metadata_enum_postprocdev_vse_stream_usage_map`
   *       - [N]: stream ID
   *       - [N + 1]: value from
   *              `mtk_camera_metadata_enum_postprocdev_vse_stream_usage_map`
   *  @see mtk_camera_metadata_enum_postprocdev_vse_stream_usage_map
   */
  MTK_POSTPROCDEV_VSE_STREAM_USAGE_MAP,

  /**
   * User can assign stream priority for each stream. This control is as the
   * hint to MTK ISP driver if theres concurrent requests to driver, higher
   * priority request would be executed much faster.
   * If this control doesn't be given, we will use default priority.
   *  @par Type
   *       int64_t
   *  @par Valid Timing
   *       - Config
   *  @par Value
   *       Value from
   *       `mtk_camera_metadata_enum_postprocdev_vse_stream_priority_control`
   *  @see mtk_camera_metadata_enum_postprocdev_vse_stream_priority_control
   */
  MTK_POSTPROCDEV_VSE_STREAM_PRIORITY_CONTROL,

  /**
   *  This ratio could be considered as margin ratio. If there is no margin
   *  or no EIS effect, this ratio should be 1.0. For example, the original
   *  image resolution is 800x600, and caller expect the warped image ROI is
   *  400x300 of the given image, the ratio would be: [0]: 0.5x, [1]: 0.5x.
   *   @par Type
   *        float[]
   *   @par Valid Timing
   *        - Config
   *   @par Value
   *        - [0]: width ratio. ex: 0.5
   *        - [1]: height ratio. ex: 0.5
   */
  MTK_POSTPROCDEV_VSE_STREAM_INLINE_WARP_RATIO,

  /**
   * Normal Data Dump is a mechanism to debug or for IQ (Image Quality) tuning
   * usage. To give this hint to Video Stream Engine to describe what purpose
   * that this Video Stream Engine is used for.
   *  @par Type
   *       int32_t
   *
   *  @par Valid Timing
   *       Per-frame
   *
   *  @par Value
   *       Value from
   *       `mtk_camera_metadata_enum_postprocdev_vse_ndd_control`
   *  @note If no debug or IQ tuning, this  tag can be ignored.
   *  @see mtk_camera_metadata_enum_postprocdev_vse_ndd_control
   */
  MTK_POSTPROCDEV_VSE_NDD_CONTROL,

  /**
   * This tag is used to specify the feature hint of the current request.
   * And will be mapped to a tuning feature index for tuning use.
   * Use bitwise or to combine multiple features.
   *
   *  @par Type
   *       int64_t
   *  @par Valid Timing
   *       Per-frame
   *  @par Value
   *       Value from
   *       `mtk_camera_metadata_enum_postprocdev_vse_feature_hint`
   *  @see mtk_camera_metadata_enum_postprocdev_vse_feature_hint
   */
  MTK_POSTPROCDEV_VSE_FEATURE_HINT,

  /**
   * User can choose VSE Direct Couple depends on APU requirements
   *
   *  @par Type
   *       int32_t
   *  @par Category
   *       control
   *  @par Valid Timing
   *       - Config
   *  @par Value
   *       - 0 : disable P1 direct couple
   *       - 1 : enable P1 direct couple
   *
   */
  MTK_POSTPROCDEV_VSE_APU_DIRECT_COUPLE_HINT,

  /**
   *  Hint APU driver the value of VSE-APU Direct Couple task token from APU framework
   *
   *  @par Type
   *       int64_t
   *  @par Category
   *       control
   *  @par Valid Timing
   *       - Request
   */
  MTK_POSTPROCDEV_VSE_APU_DIRECT_COUPLE_TOKEN_ID,

  /**
   *  Hint for raw type which is used by algo(ex: APU/GPU) as input, if non-full
   *  processed is true, it needs to be processed again
   *
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Config
   *  @par Value
   *       - 0: Raw is fully processed
   *       - 1: Raw is non-fully processed
   *
   */
  MTK_POSTPROCDEV_VSE_NON_FULL_PROCESSED_RAW_HINT,

  /**
   * User can hint a set of requests to multiple devices are a concurrent
   * requests set. It means, the requests between devices may have some
   * concurrent logics to be applied to these requests (for example,
   * enqueue order guarantee to MTK ISP hardware).
   *
   * While processing per frame requests, caller also has responsibility to
   *  1. Make sure this request ID between a set of requests should be the
   *     same.
   *  2. Before result callbacks of these request has been callbacked, no
   *     duplicated request ID would be given to devices. For example,
   *     increasing a counter starts from 1 for each sets of requests,
   *     and reset it after close streams.
   *
   *  @par Type
   *       int64_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       - @c 0 Disable concurrent device set (Default is disabled).
   *       - @c Otherwise An unique key generated from caller. Caller has
   *         responsibility to guarantee this key won't be duplicated
   *         against other current devices sets.
   */
  MTK_POSTPROCDEV_VSE_CONCURRENT_REQUEST_ID,

  /**
   * User has to hint the devices count to indicate how many devices were
   * in the same set. For example, caller wants to set 3 devices as the same
   * set, give this value to 3.
   *
   * Video Stream Engine may have mechanism to check if all requests were
   * collected from caller.
   *
   *  @par Type
   *       int64_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       A integer greater than 0.
   *  @note This control only valid if
   *        `MTK_POSTPROCDEV_VSE_CONCURRENT_REQUEST_ID` has been set.
   *        If `MTK_POSTPROCDEV_VSE_CONCURRENT_REQUEST_ID` has been set,
   *        this value must be given.
   */
  MTK_POSTPROCDEV_VSE_CONCURRENT_REQUEST_COUNT,

  /**
   * User has to hint the device index to device. This value is related to
   * `MTK_POSTPROCDEV_VSE_CONCURRENT_REQUEST_INDEX`. For example, if there
   * are 5 devices in a concurrent devices set, these devices should have
   * this tag with value 0, 1, 2, 3, and 4.
   *
   *  @par Type
   *       int64_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       The device index of the concurrent devices set.
   */
  MTK_POSTPROCDEV_VSE_CONCURRENT_REQUEST_INDEX,

  /**
   * The behavior hint to the given concurrent device set.
   *
   *  @par Type
   *       int64_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       Enumeration of
   *       `mtk_camera_metadata_enum_postprocdev_vse_concurrent_request_hint `.
   *       - [0]: Enum of `mtk_camera_metadata_enum_postprocdev_vse_concurrent_request_hint `.
   *       - [1]: Value of the given enumeration.
   *       - [N]: Enum of `mtk_camera_metadata_enum_postprocdev_vse_concurrent_request_hint `.
   *       - [N+1]: Value of the given enumeration.
   *  @note Only valid if `MTK_POSTPROCDEV_VSE_CONCURRENT_REQUEST_ID`
   *        was given. This control and value should be the same among
   *        each devices of the given concurrent request set.
   */
  MTK_POSTPROCDEV_VSE_CONCURRENT_REQUEST_HINT,

  /**
   * Describe if VSE should update MTK HDR10+ meta or not.
   *
   *  @par Type
   *  MINT32
   *
   *  @par Valid Timing
   *  @arg Per-frame
   *
   *  @par Possible Value
   *       - @c 0 Disable
   *       - @c 1 Enable
   *  @note Only valid if `MTK_POSTPROCDEV_VSE_FEATURE_CONTROL_YUVMCNR_PQ_DIP_ENABLE`
   *        is set to true.
   */
  MTK_POSTPROCDEV_VSE_ENABLE_HDR10_META,

  /**
   * Describe the size of Warp Map.
   *  @type: int32_t[]
   *  @valid timing: config
   *  @value:
   *    [0]: width in pixel
   *    [1]: height in pixel
   *  @note: In order to use the capability of VSE-inline-warp, user should
   *    either config a Warp Map stream, or use
   *    MTK_POSTPROCDEV_VSE_WARPMAP_SIZE to set the size of Warp Map.
   *    The width of warp map need 2 pixel alignment and while use this tag
   *    to hint warp map information and set warp map content by
   *    MTK_POSTPROCDEV_VSE_WARPMAP_SIZE, it will cost additional CPU MIPs to
   *    copy content to hardware buffer.
   */
  MTK_POSTPROCDEV_VSE_WARPMAP_SIZE,

  /**
   * Describe the content of Warp Map.
   *  @type: int32_t[]
   *  @valid timing: per-frame
   *  @value:
   *    [0]:    4-byte warp map content, x-plane: 1st part.
   *    [1]:    4-byte warp map content, x-plane: 2nd part.
   *    [N-1]:  4-byte warp map content, x-plane: Nth part.
   *    [N]:    4-byte warp map content, y-plane: 1st part.
   *    [N+1]:  4-byte warp map content, y-plane: 2nd part.
   *    [2N-1]: 4-byte warp map content, y-plane: Nth part.
   *  @note: The value of N is expected to be: N = width x height. User who
   *    sets MTK_POSTPROCDEV_VSE_WARPMAP_SIZE at config time is able to fill
   *    warp map content by MTK_POSTPROCDEV_VSE_WARPMAP_CONTENT per request.
   *    VSE device need copy content data of warp map to hardware buffer.
   */
  MTK_POSTPROCDEV_VSE_WARPMAP_CONTENT,

  MTK_POSTPROCDEV_VSE_FEATURE_END,

  /**
   * Describe the output image size of WPE module. The output image size
   * should smaller or equal to input image size.
   *  @par Type
   *       int64_t[]
   *  @par Valid Timing
   *       - Per-frame
   *  @par Unit
   *       Pixel in main source image
   *  @par Value
   *       Output image size
   *       - [0]: width in pixel
   *       - [1]: height in pixel
   */
  MTK_POSTPROCDEV_WPEPQ_WPE_OUTPUT_SIZE = MTK_POSTPROCDEV_WPEPQ_FEATURE_START,

  /**
   * Describe the tuning feature and stage of WPE_PQ device.
   *  @par Type
   *       int32_t[]
   *  @par Valid Timing
   *       Per-frame
   *  @par value
   *       - [0]: Tuning feature value
   *       - [1]: stage
   */
  MTK_POSTPROCDEV_WPEPQ_TUNING_FEATURE,

  /**
   * Describe the output image need to flip or not of WPE_PQ device.
   *  @par Type
   *       uint8_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Possible Values
   *       - @c 0 Disable
   *       - @c 1 Flip vertical
   *       - @c 2 Flip horizontal
   */
  MTK_POSTPROCDEV_WPEPQ_FLIP,

  /**
   * Describe the output image need to flip or not of WPE_PQ device.
   *  @par Type
   *       uint8_t
   *  @par Valid Timing
   *       - per-frame
   *  @par Value
   *       Value from `mtk_camera_metadata_enum_postprocdev_wpepq_rotate`.
   */
  MTK_POSTPROCDEV_WPEPQ_ROTATE,

  /**
   * Describe the crop info of WPE_PQ device.
   *  @par Type
   *       int32_t[]
   *  @par Valid Timing
   *       - Per-frame
   *  @par Unit
   *       Pixel in main source image
   *  @par value
   *       - [0]: X position
   *       - [1]: Y position
   *       - [2]: width
   *       - [3]: height
   */
  MTK_POSTPROCDEV_WPEPQ_CROP_INFO,

  /**
   * Describe enable dump flow for this warp pq.
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par value
   *       1: means enable dump flow
   */
  MTK_POSTPROCDEV_WPEPQ_ENABLE_DUMP,

  /**
   * Describe the size of Warp Map.
   *  @type: int32_t[]
   *  @valid timing: config
   *  @value:
   *    [0]: width in pixel
   *    [1]: height in pixel
   *  @note: In order to use the capability of WPE, user should either config
   *    a Warp Map stream, or use MTK_POSTPROCDEV_WPEPQ_WARPMAP_SIZE to set
   *    the size of Warp Map.
   */
  MTK_POSTPROCDEV_WPEPQ_WARPMAP_SIZE,

  /**
   * Describe the content of Warp Map.
   *  @type: int32_t[]
   *  @valid timing: per-frame
   *  @value:
   *    [0]:    4-byte warp map content, x-plane: 1st part.
   *    [1]:    4-byte warp map content, x-plane: 2nd part.
   *    [N-1]:  4-byte warp map content, x-plane: Nth part.
   *    [N]:    4-byte warp map content, y-plane: 1st part.
   *    [N+1]:  4-byte warp map content, y-plane: 2nd part.
   *    [2N-1]: 4-byte warp map content, y-plane: Nth part.
   *  @note: The value of N is expected to be: N = width x height. User who
   *    sets MTK_POSTPROCDEV_WPEPQ_WARPMAP_SIZE at config time is able to fill
   *    warp map content by MTK_POSTPROCDEV_WPEPQ_WARPMAP_CONTENT per request.
   */
  MTK_POSTPROCDEV_WPEPQ_WARPMAP_CONTENT,

  MTK_POSTPROCDEV_WPEPQ_FEATURE_END,

  /**
   * This tag describes the usage map of the given Stream IDs. Each feature of
   * Capture Engine may have different usage, caller has to follow the
   * documentation to set the corresponding usage map to the selected feature.
   *  @par Type
   *       int64_t[]
   *  @par Valid Timing
   *       - Config
   *  @par Value
   *       - [0]: Stream ID
   *       - [1]: value from
   *              `mtk_camera_metadata_enum_postprocdev_capture_stream_usage_map`
   *       - [N]: Stream ID
   *       - [N + 1]: value from
   *              `mtk_camera_metadata_enum_postprocdev_capture_stream_usage_map`
   */
  MTK_POSTPROCDEV_CAPTURE_STREAM_USAGE_MAP =
    MTK_POSTPROCDEV_CAPTURE_FEATURE_START,

  MTK_POSTPROCDEV_CAPTURE_FEATURE_END,

  /**
   * This tag describes the source of bokeh input. It can be NULL, and the enum
   * `MTK_POSTPROCDEV_BOKEH_INPUT_SOURCE_MTK_DEPTH` will be introduced as the
   * default value.
   *  @par Type
   *       uint8_t
   *  @par Valid Timing
   *       - Config
   *  @par Possible Value
   *       - [0]: Value from
   *              `mtk_camera_metadata_enum_postprocdev_bokeh_input_source`
   */
  MTK_POSTPROCDEV_BOKEH_INPUT_SOURCE = MTK_POSTPROCDEV_BOKEH_FEATURE_START,

  /**
   * This tag describes the relationship between Stream IDs and feature-based
   * I/O. Caller can use this tag to define each stream usage in input streams
   * during configure stage.
   *  @par Type
   *       int32_t[]
   *  @par Valid Timing
   *       - Config
   *  @par Possible Value
   *       - [0]: stream ID
   *       - [1]: value from
   *              `mtk_camera_metadata_enum_postprocdev_vse_stream_usage_map`
   *       - [N]: stream ID
   *       - [N + 1]: value from
   *              `mtk_camera_metadata_enum_postprocdev_bokeh_stream_usage_map`
   *  @see mtk_camera_metadata_enum_postprocdev_bokeh_stream_usage_map
   */
  MTK_POSTPROCDEV_BOKEH_STREAM_USAGE_MAP,

  /**
   * This tag describes the master sensor Id.
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Config
   *       - Per-frame
   *  @par Possible Value
   *       - [0]: Master sensor Id
   */
  MTK_POSTPROCDEV_BOKEH_MASTER_ID,

  /**
   * This tag describes the stereo scenario
   *  @par Type
   *       uint8_t
   *  @par Valid Timing
   *       - Config
   *  @par Possible Value
   *       - [0]: value from
   *            `mtk_camera_metadata_enum_postprocdev_bokeh_stereo_scenario_map`
   *  @see mtk_camera_metadata_enum_postprocdev_bokeh_stereo_scenario_map
   */
  MTK_POSTPROCDEV_BOKEH_STEREO_SCENARIO,

  /**
   * This tag describes the DOF Level
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Possible Value
   *       - [0]: DOF Level
   */
  MTK_POSTPROCDEV_BOKEH_DOF_LEVEL,

  /**
   * This tag describes the touch position
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Possible Value
   *       - [0]: position X
   *       - [1]: position Y
   */
  MTK_POSTPROCDEV_BOKEH_TOUCH_POSITION,

  MTK_POSTPROCDEV_BOKEH_FEATURE_END,

  /**
   * Describe the EIS width crop ratio and height crop ratio, for example a 100x100
   * input image with [w,h] crop ratio [0.8,0.7], after EIS cropping the output image
   * would be 80x70.
   *  @par Type
   *       float[]
   *  @par Valid Timing
   *       - Config
   *  @par Value
   *       - [0]: width crop ratio
   *       - [1]: height crop ratio
   */
  MTK_POSTPROCDEV_EIS_CROP_RATIO = MTK_POSTPROCDEV_EIS_FEATURE_START,

  /**
   * Describe the output Stream ID of the warpmap buffer calculated based on IIR
   * filter. Basically, the warpmap will be calculated immediately and return to
   * user, however, the output warpmap was synced by the timestamp of the given
   * source image metadata.
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - per-frame
   *  @par Value
   *       Stream ID of IIR warpmap.
   */
  MTK_POSTPROCDEV_EIS_STREAM_WARPMAP_IIR,

  /**
   * Describe the output Stream ID of the warpmap buffer calculated based on FIR
   * filter with fixed duration 25 frames. It means if the internal queue
   * statistics is not enough to calculate the warpmap, this output may be
   * empty. TODO: need to more tags to achieve FIR warpmap design.
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - per-frame
   *  @par Value
   *       Stream ID of FIR warpmap.
   */
  MTK_POSTPROCDEV_EIS_STREAM_WARPMAP_FIR,

  /**
   * Describe the input Stream ID of P1 output image that EIS applied to,
   * optional.
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - per-frame
   *  @par Value
   *       Stream ID of image that EIS applied to.
   */
  MTK_POSTPROCDEV_EIS_STREAM_IMAGE,

  /**
   * Describe the customization image crop/resize used for EIS calculation,
   * optional.
   *  @par Type
   *       int64_t[]
   *  @par Valid Timing
   *       - per-frame
   *  @par Value
   *       - [0]: Stream ID.
   *       - [1]: X position of crop rectangle.
   *       - [2]: Y position of crop rectangle.
   *       - [3]: Width in pixel of crop rectangle.
   *       - [4]: Height in pixel of crop rectangle.
   *       - [5]: Width of destination.
   *       - [6]: Height of destination.
   */
  MTK_POSTPROCDEV_EIS_CUSTOM_CROP,

  /**
   * Describe EIS mode for using IIR or FIR filter, if FIR mode is used the warpmap
   * would queued in EIS engine and delay the callback to user for better EIS quality,
   * while in IIR mode the warpmap would callback to user without delay
   *
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Config
   *  @par Value
   *       value from mtk_platform_metadata_enum_postprocdev_eis_mode
   */
  MTK_POSTPROCDEV_EIS_MODE,

  /**
   * Describe EIS FIR filter hint, the first element indicates to enable or
   * disable FIR filter. The second element is to hint the EIS Engine to keep
   * queuing statistics for FIR filter calculation, or to hint EIS Engine to
   * enter flush stage and return queued warpmap to user
   *
   * TODO: Caller has to identify the result info to know the "key" of the
   * output warpmap.
   *
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - per-frame
   *  @par Value
   *       value from mtk_platform_metadata_enum_postprocdev_fir_filter_hint
   */
  MTK_POSTPROCDEV_EIS_FIR_FILTER_HINT,

  /**
   * Describe EIS FIR STATE. For FIR EIS usage user need to parse MTK_POSTPROCDEV_EIS_FIR_STATE
   * callback metadata to know the FIR state of EIS engine, and match correct FIR warpmap to
   * its corresponding frame after queue process complete
   *
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - per-frame
   *  @par Value
   *       value from mtk_platform_metadata_enum_postprocdev_eis_fir_state
   */
  MTK_POSTPROCDEV_EIS_FIR_STATE,

  /**
   * Describe the size which IIR warpmap must scale to. If EIS engine calculate warpmap
   * based on larger domain size (e.g. 4k), then IIR warpmap should be scaled down to smaller domain
   * size to match buffer content
   *
   *  @par Type
   *       MSize
   *  @par Valid Timing
   *       - Config
   *  @par Value
   *       image size which IIR warpmap applied to
   */
  MTK_POSTPROCDEV_EIS_SCALE_IIR_SIZE,

  MTK_POSTPROCDEV_EIS_FEATURE_END,
  /**
   * This tag describes 3A infomation (AE/AF/AWB, including, but not limited to)
   *  @par Type
   *       IMetadata::Memory
   *  @par Valid Timing
   *       - Per-frame
   *  @par Possible Value
   *       User must cast this memory to the type of Private3AInfo for using
   *       correctly. The definition of mcam::Utils::Private3AInfo in
   *       mtkcam-halif/include/mtkcam-halif/utils/aaa/aaa_data.h
   */

  MTK_PRIVATE_3A_INFO = MTK_PRIVATE_3A_FEATURE_START,

  MTK_PRIVATE_3A_FEATURE_END,
} mtk_private_camera_metadata_tag_t;

/**
 * Enum for redefining camera metadata tags. these tags are already defined in
 * mtk_metadata_tag.h and rename them for customer could use them more
 * intuitively.
 */
typedef enum mtk_redefined_camera_metadata_tag {
  /**
   * Hint HAL to enable the given multi-frame feature.
   *
   * This tag can work with tag MTK_POSTPROCDEV_CAPTURE_HINT_FOR_CUSTOM_TUNING,
   * which acts as tuning hint to tuning framework.
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Per-frame
   *       - Config
   *  @par Value
   *       Value from `mtk_camera_metadata_enum_postprocdev_capture_hint_multiframe_feature`.
   *  @see
   *       - MTK_POSTPROCDEV_CAPTURE_HINT_MULTIFRAME_FEATURE
   *       - mtk_camera_metadata_enum_postprocdev_capture_hint_multiframe_feature
   */
  MTK_POSTPROCDEV_CAPTURE_HINT_MULTIFRAME_FEATURE =
    MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING,

  /**
   * Describe the custom tuning hint for the given Capture Engine
   * session to process the post processing progress.
   *
   * Caller could customize the tuning hints.
   *
   * This tag should work with tag MTK_POSTPROCDEV_CAPTURE_HINT_MULTIFRAME_FEATURE,
   * which acts as feature hint to indicate the feature required.
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par value
   *       any value, according to MTK's Tuning Framework
   *  @note Customized tuning hints are for customized tuning parameters which
   *        were tuned by MTK tuning framework. The tuning parameters can not
   *        be tuned or detail adjusted through MTK HAL I/F's public interfaces.
   */
  MTK_POSTPROCDEV_CAPTURE_HINT_FOR_CUSTOM_TUNING =
    MTK_CONTROL_CAPTURE_HINT_FOR_CUSTOM_TUNING,

  /**
   * Describe this Capture Device session is only used JPEG function, or bypass
   * some post processing capture features.
   *
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Configure
   *  @par value
   *       - `1`: This device session still passes requests but only outputs
   *              thumbnail.
   *       - `2`: This device session still passes requests but only outputs
   *              main YUV.
   *       - `3`: This device is only used JPEG function. App YUV will be set to
   *              JPEG directly.
   */
  MTK_POSTPROCDEV_CAPTURE_YUV_DIRECT_JPEG =
    MTK_CONFIGURE_ISP_YUV_DIRECT_JPEG,

  /**
   * Since MTK multi frame capture features need continuous and stable frames,
   * and each features may need different definitions of "continuous", and
   * "stable", this control is for Data Collect to hint caller needs the N
   * frames for the certain MTK post processing capture features.
   *
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par value
   *       number of frames
   *  @note This control tag belongs to Capture Device but is a control for
   *        Data Collect.
   *  @see MTK_POSTPROCDEV_CAPTURE_MULTIFRAME_INDEX
   */
  MTK_POSTPROCDEV_CAPTURE_MULTIFRAME_COUNT =
    MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_COUNT,

  /**
   * MTK post processing features may have different strategy to collect
   * frames, it means different frame index may indicate to different
   * policy. This tag is used to hint this request is which index of
   * multi-frame capture.
   *
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par value
   *       index of frames
   *  @note This control tag belongs to Capture Device but is a control for
   *        Data Collect.
   *  @see MTK_POSTPROCDEV_CAPTURE_MULTIFRAME_COUNT
   */
  MTK_POSTPROCDEV_CAPTURE_MULTIFRAME_INDEX =
    MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX,


  /**
   * FD engine supports face detection mode simple, full and off. Simple mode
   * indicates to return face rectangle and confidence values only; Full mode
   * indicates to return face rectangles, scores, landmarks, and face IDs.
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       - `0`: Disable FD
   *       - `1`: Simple mode
   *       - `2`: Full mode
   */
  MTK_POSTPROCDEV_FD_FACE_DETECT_MODE =
    MTK_STATISTICS_FACE_DETECT_MODE,

  /**
   * FD engine will update MTK Face IDs, this tag is an
   * output tag that indicates the IDs for detected
   * faces of corresponding request.
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       - `[N]`: ID
   */
  MTK_POSTPROCDEV_FD_FACE_IDS =
    MTK_STATISTICS_FACE_IDS,

  /**
   * FD engine will update MTK Face landmarks, this tag is an
   * output tag that indicates the face landmarks of corresponding request.
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       - `[0]`: x-coordinate of left eye
   *       - `[1]`: y-coordinate of left eye
   *       - `[2]`: x-coordinate of right eye
   *       - `[3]`: y-coordinate of right eye
   *       - `[4]`: x-coordinate of mouth
   *       - `[5]`: y-coordinate of mouth
   */
  MTK_POSTPROCDEV_FD_FACE_LANDMARKS =
    MTK_STATISTICS_FACE_LANDMARKS,

  /**
   * FD engine will update MTK Face rectangles, this tag is an
   * output tag that indicates the face retangles of corresponding request.
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       - `[0]`: x-coordinate of top left corner
   *       - `[1]`: y-coordinate of top left corner
   *       - `[2]`: x-coordinate of bottom right corner
   *       - `[3]`: y-coordinate of bottom right corner
   */
  MTK_POSTPROCDEV_FD_FACE_RECTANGLES =
    MTK_STATISTICS_FACE_RECTANGLES,

  /**
   * FD engine will update MTK Face scores, this tag is an
   * output tag that indicates the confidence scores
   * for detected faces of corresponding request.
   *  @par Type
   *       uint8_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       - `[0]`: confidence score of face 0
   *       - `[1]`: confidence score of face 1
   *       - ...
   *       - `[N]`: confidence score of face N
   */
  MTK_POSTPROCDEV_FD_FACE_SCORES =
    MTK_STATISTICS_FACE_SCORES,

  /**
   * FD engine will updateMTK Face detection additional results, this tag is
   * an output tag that indicates the FD results of corresponding request.
   *
   *  @par Type
   *       int32_t[]
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       - `[0]`: ROP
   *       - `[1]`: RIP
   *       - `[2]`: X0 of left eye
   *       - `[3]`: Y0 of left eye
   *       - `[4]`: X1 of left eye
   *       - `[5]`: Y1 of left eye
   *       - `[6]`: X0 of right eye
   *       - `[7]`: Y0 of right eye
   *       - `[8]`: X1 of right eye
   *       - `[9]`: Y1 of right eye
   *       - `[10]`: X0 of mouth
   *       - `[11]`: Y0 of mouth
   *       - `[12]`: X1 of mouth
   *       - `[13]`: Y1 of mouth
   *       - `[14]`: X of nose
   *       - `[15]`: Y of nose
   *       - `[16]`: X of upper left eyelid
   *       - `[17]`: Y of upper left eyelid
   *       - `[18]`: X of down left eyelid
   *       - `[19]`: Y of down left eyelid
   *       - `[20]`: X of upper right eyelid
   *       - `[21]`: Y of upper right eyelid
   *       - `[22]`: X of down right eyelid
   *       - `[23]`: Y of down right eyelid
   *       - `[24]`: fa_cv (confidence score of landmarks)
   *       - `[N+0]`: ROP
   *       - ...
   *       - `[N+24]`: fa_cv (confidence score of landmarks)
   */
  MTK_POSTPROCDEV_FD_FACE_ADDITIONAL_RESULT =
    MTK_FACE_FEATURE_FACE_ADDITIONAL_RESULT,


  /**
   * Refine the face detection information by the given values to FD engine.
   *  @par Type
   *       int64_t[]
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       `[0]`: Sensor ID of detected image.
   *       `[1]`: Shutter time of the detected image in us.
   */
  MTK_POSTPROCDEV_FD_REFINE_FACE_INFORMATION =
    MTK_FACE_FEATURE_3RD_PARTY_FACE_INFORMATION,

  /**
   * Refine the face detection rectangles by the given values to FD engine.
   *  @par Type
   *       MRect[]
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       - `[0]`: Rectangle of face 0
   *       - `[1]`: Rectangle of face 1
   *       - `[N]`: Rectangle of face N
   */
  MTK_POSTPROCDEV_FD_REFINE_FACE_RECTANGLES =
    MTK_FACE_FEATURE_3RD_PARTY_FACE_RECTANGLES,

  /**
   * Refine the face detection results by the given values to FD engine.
   *  @par Type
   *       int32_t[]
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       - `[0]`: ROP
   *       - `[1]`: RIP
   *       - `[2]`: X0 of left eye
   *       - `[3]`: Y0 of left eye
   *       - `[4]`: X1 of left eye
   *       - `[5]`: Y1 of left eye
   *       - `[6]`: X0 of right eye
   *       - `[7]`: Y0 of right eye
   *       - `[8]`: X1 of right eye
   *       - `[9]`: Y1 of right eye
   *       - `[10]`: X0 of mouth
   *       - `[11]`: Y0 of mouth
   *       - `[12]`: X1 of mouth
   *       - `[13]`: Y1 of mouth
   *       - `[14]`: X of nose
   *       - `[15]`: Y of nose
   *       - `[16]`: X of upper left eyelid
   *       - `[17]`: Y of upper left eyelid
   *       - `[18]`: X of down left eyelid
   *       - `[19]`: Y of down left eyelid
   *       - `[20]`: X of upper right eyelid
   *       - `[21]`: Y of upper right eyelid
   *       - `[22]`: X of down right eyelid
   *       - `[23]`: Y of down right eyelid
   *       - `[24]`: fa_cv (confidence score of landmarks)
   *       - `[N+0]`: ROP
   *       - ...
   *       - `[N+24]`: fa_cv (confidence score of landmarks)
   */
  MTK_POSTPROCDEV_FD_REFINE_FACE_RESULTS =
    MTK_FACE_FEATURE_FACE_ADDITIONAL_RESULT,

  /**
   * Refine the face detection results of YUV statistics by the given values
   * to FD engine.
   *  @par Type
   *       uint8_t
   *  @par Value
   *       - `[0]`: Y mean
   *       - `[1]`: U mean
   *       - `[2]`: V mean
   *       - `[3]`: Y average of 5% values from Y histogram (5% lowest value)
   *       - `[4]`: Y average of 95% values from Y histogram (5% highest values)
   */
  MTK_POSTPROCDEV_FD_REFINE_YUVSTS =
    MTK_FACE_FEATURE_3RD_PARTY_YUVSTS,

  /**
   * Refine the face detection results of race and gender by the given values
   * to FD engine.
   *  @par Type
   *       uint8_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       - `[0]`: Race label,
   *       - `[1]`: Gender label
   *  @par Possible Values of Race Label
   *       - `0`: Uncertain
   *       - `1`: Asian (yellow)
   *       - `2`: White
   *       - `3`: Black
   *       - `4`: Indian (brown)
   *  @par Possible Values of Gender Label
   *       - `0`: Uncertain
   *       - `1`: Male
   *       - `2`: Feamale
   */
  MTK_POSTPROCDEV_FD_REFINE_RACE_GENDER_LABEL =
    MTK_FACE_FEATURE_3RD_PARTY_RACE_GENDER_LABEL,

  /**
   * Refine the face detection results of skin map by the given values
   * to FD engine. Th maximum number of valid skin map is 3.
   * The skin map is a fixed size map (30x50 bytes) transformed from a
   * face rectangle. Each byte indicates the confidence value (0 ~ 255).
   * The lower value implies the lower confidence of skin.
   * The values of skin map are in raw major arrangement, which means
   * from [1] ~ [50] indicates row 1, [51] ~ [100] indicates raw 2,
   * and so on.
   *  @par Type
   *       uint8_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       - `[0]`: valid number of skin map,
   *       - `[1] ~ [1500]`: value of skin map 1,
   *       - `[1501] ~ [3000]`: value of skin map 2,
   *       - `[3001] ~ [4500]`: value of skin map 3
   *  @par Relation between skin maps and transform matrices
   *         Skin map rectangles can be transformed into face recatangles in
   *         active array domain within the given transform matrices described
   *         by tag `MTK_POSTPROCDEV_FD_REFINE_SKIN_MAP_TRANSFORM_MATRIX`.
   *         We can use a transform matrix times the top-left and bottom-right
   *         points of corresponding skin map to get the top-left and bottom-right
   *         points of the original face rectangle.
   */
  MTK_POSTPROCDEV_FD_REFINE_SKIN_MAP =
    MTK_FACE_FEATURE_3RD_PARTY_SKIN_MAP,

  /**
   * Refine the face detection results of skin map transform matrix (2x3)
   * by the given values to FD engine. This matrix transform face rectangles
   * from skin map domain (0, 0, 30, 50) to active array domain. The first
   * two columns indicate the rotation and scale. The last column
   * indicates the shift. Matrices are listed in row major order.
   * The number of matrices should be the same as the number of valid skin maps.
   *  @par Type
   *       double
   *  @par Valid Timing
   *       - Per-frame
   *  @par Value
   *       - `[0]`: value at row 0 column 0 of matrix 1,
   *       - `[1]`: value at row 0 column 1 of matrix 1,
   *       - `[2]`: value at row 0 column 2 of matrix 1,
   *       - `[3]`: value at row 1 column 0 of matrix 1,
   *       - `[4]`: value at row 1 column 1 of matrix 1,
   *       - `[5]`: value at row 1 column 2 of matrix 1,
   *       - `[6] ~ [11]`: transform matrix 2,
   *       - `[12] ~ [17]`: transform matrix 3
   */
  MTK_POSTPROCDEV_FD_REFINE_SKIN_MAP_TRANSFORM_MATRIX =
    MTK_FACE_FEATURE_3RD_PARTY_SKIN_MAP_TRANSFORM_MATRIX,

  /**
   * This tag is used to hint Core Session to output the raw buffer with ISP
   * processed.
   *
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par value
   *       `1`: raw buffer is processed
   */
  MTK_HALCORE_PROCESS_RAW_ENABLE =
    MTK_CONTROL_CAPTURE_PROCESS_RAW_ENABLE,

  /**
   * Describe the crop region of Capture device.
   *
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Per-frame
   *  @par Unit
   *       Pixel in source image
   *  @par value
   *       - [0]: X position
   *       - [1]: Y position
   *       - [2]: width
   *       - [3]: height
   */
  MTK_POSTPROCDEV_CAPTURE_CROP_REGION =
    MTK_CONTROL_CAPTURE_REPROCESS_CROP,

  /**
   * This tag is used to hint Core Session to disable platform FD function
   * to save power comsumption. Notice that, if disabled platform FD, platform
   * algorithms would be impacted. Caller could calculate FD results and set
   * the results back to platform through FD engine.
   *
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Config
   *  @par Value
   *       - `1`: Disable platform FD.
   */
  MTK_HALCORE_DISABLE_PLATFORM_FD =
    MTK_FACE_FEATURE_DISABLE_FDNODE,

  /**
   * This tag is used to hint Core Session to change the detection period of
   * platform FD function to save power comsumption. The value of period is
   * millisecond based. For example: value 1000 means that platform FD runs
   * once a second. In other word, 1 frame per second.
   *
   *  @par Type
   *       int32_t
   *  @par Valid Timing
   *       - Config
   *  @par Value
   *       - `N`: Run platform FD once every N ms
   */
  MTK_HALCORE_PLATFORM_FD_PERIOD =
    MTK_FACE_FEATURE_FACE_DETECTION_PERIOD
} mtk_redefined_camera_metadata_tag_t;

// Enumeration definitions for the various entries that need them

/**
 * Multi-frame feature hint for Capture Engine.
 *  @see MTK_POSTPROCDEV_CAPTURE_HINT_MULTIFRAME_FEATURE
 */
typedef enum mtk_camera_metadata_enum_postprocdev_capture_hint_multiframe_feature {
  /**
   * Hint for MTK MFNR. If the given feature was not supported, the given
   * request will failed.
   */
  MTK_POSTPROCDEV_CAPTURE_HINT_MULTIFRAME_MFNR =
    MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_MFNR,

  /**
   * Hint for MTK AINR. If the given feature was not supported, the given
   * request will failed.
   */
  MTK_POSTPROCDEV_CAPTURE_HINT_MULTIFRAME_AINR =
    MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_AINR,

  /**
   * Hint for MTK AIHDR. If the given feature was not supported, the given
   * request will failed.
   */
  MTK_POSTPROCDEV_CAPTURE_HINT_MULTIFRAME_AIHDR =
    MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_AIHDR,

  /**
   * Hint for MTK AISHUTTER. If the given value is used, then the plaform
   * would trigger either MTK AISHUTTER1 or MTK AISHUTTER2, automatically. In
   * contrast, if a specific version of MTK AISHUTTER is required, use
   * MTK AISHUTTER1 or MTK AISHUTTER2 instead. If the given feature was not
   * supported, the given request will failed.
   */
  MTK_POSTPROCDEV_CAPTURE_HINT_MULTIFRAME_AISHUTTER =
    MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_AISHUTTER,

  /**
   * Hint for MTK AISHUTTER version 1. If the given feature was not supported,
   * the given request will failed.
   */
  MTK_POSTPROCDEV_CAPTURE_HINT_MULTIFRAME_AISHUTTER1 =
    MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_AISHUTTER1,

  /**
   * Hint for MTK AISHUTTER version 2. If the given feature was not supported,
   * the given request will failed.
   */
  MTK_POSTPROCDEV_CAPTURE_HINT_MULTIFRAME_AISHUTTER2 =
    MTK_CONTROL_CAPTURE_HINT_FOR_ISP_TUNING_AISHUTTER2,
} mtk_camera_metadata_enum_postprocdev_capture_hint_multiframe_feature_t;

/**
 * This is value of tag `MTK_POSTPROCDEV_CAPTURE_STREAM_USAGE_MAP`.
 *  @sa `MTK_POSTPROCDEV_CAPTURE_STREAM_USAGE_MAP`.
 */
typedef enum mtk_camera_metadata_enum_postprocdev_capture_stream_usage_map {
  /**
   * Identify for main input stream. It can be raw or yuv.
   */
  MTK_POSTPROCDEV_CAPTURE_STREAM_USAGE_MAIN_INPUT = 0,

  /**
   * Identify for input thumbnail yuv.
   */
  MTK_POSTPROCDEV_CAPTURE_STREAM_USAGE_THUMBNAIL_INPUT,

  /**
   * Identify for scaled input yuv (rrzo), which is needed for BSS.
   */
  MTK_POSTPROCDEV_CAPTURE_STREAM_USAGE_YUV_BSS_INPUT,

  /**
   * Identify for scaled input yuv (R3), which is needed for AINR.
   */
  MTK_POSTPROCDEV_CAPTURE_STREAM_USAGE_YUV_R3_INPUT,

  /**
   * Identify for output postview buffer.
   */
  MTK_POSTPROCDEV_CAPTURE_STREAM_USAGE_POSTVIEW_OUTPUT,
} mtk_camera_metadata_enum_postprocdev_capture_stream_usage_map_t;

typedef enum mtk_camera_metadata_enum_halcore_stream_source {
  MTK_HALCORE_STREAM_SOURCE_UNDEFINED    = (0x00 << 8),  // undefined
  // high level:  0x01 - 0x6f
  MTK_HALCORE_STREAM_SOURCE_FULL_RAW     = (0x01 << 8),  // full-size raw
  MTK_HALCORE_STREAM_SOURCE_FD           = (0x02 << 8),  // face detection
  MTK_HALCORE_STREAM_SOURCE_ME           = (0x03 << 8),  // motion estimation
  MTK_HALCORE_STREAM_SOURCE_DEPTH        = (0x04 << 8),  // depth used yuv
  MTK_HALCORE_STREAM_SOURCE_FE           = (0x05 << 8),  // feature extraction
                                                          // preview used yuv
  MTK_HALCORE_STREAM_SOURCE_AI_TRACKING  = (0x06 << 8),  // ai tracking
  // low level:   0x70 - 0xdf
  MTK_HALCORE_STREAM_SOURCE_YUV          = (0x70 << 8),  // No R0 (index=0)
  MTK_HALCORE_STREAM_SOURCE_YUV_R1,                      // index=1
  MTK_HALCORE_STREAM_SOURCE_YUV_R2,
  MTK_HALCORE_STREAM_SOURCE_YUV_R3,
  MTK_HALCORE_STREAM_SOURCE_YUV_R4,
  MTK_HALCORE_STREAM_SOURCE_YUV_R5,
  // low level - isp6
  MTK_HALCORE_STREAM_SOURCE_STT          = (0xe0 << 8),  // statistics
  // global
  MTK_HALCORE_STREAM_SOURCE_TUNING_DATA  = (0xf0 << 8),
} mtk_camera_metadata_enum_halcore_stream_source_t;


typedef enum mtk_camera_metadata_enum_halcore_hbm_strategy {
  /**
   * request buffers at receiving requests time.
   */
  MTK_HALCORE_HBM_STRATEGY_PREPARATORY,
  /**
   * request buffers at about to write buffers time.
   */
  MTK_HALCORE_HBM_STRATEGY_IMMEDIATE,
  /**
   * request some extra buffers before request time.
   * request another buffers with immediate strategy.
   */
  MTK_HALCORE_HBM_STRATEGY_CACHED,
} mtk_camera_metadata_enum_halcore_hbm_strategy_t;

typedef enum mtk_camera_metadata_enum_halcore_physical_sensor_status {
  MTK_HALCORE_PHYSICAL_SENSOR_STATUS_STREAMING = 0,
  MTK_HALCORE_PHYSICAL_SENSOR_STATUS_STANDBY,
  // MTK_HALCORE_PHYSICAL_SENSOR_STATUS_LOWFPS,
} mtk_camera_metadata_enum_halcore_physical_sensor_status_t;

typedef enum
mtk_camera_metadata_enum_halcore_available_image_processing_settings {
  MTK_HALCORE_IMAGE_PROCESSING_SETTINGS_UNDEFINED = 0,
  MTK_HALCORE_IMAGE_PROCESSING_SETTINGS_FAST = (1 << 0),
  MTK_HALCORE_IMAGE_PROCESSING_SETTINGS_HIGH_QUALITY = (1 << 1),
} mtk_camera_metadata_enum_halcore_available_image_processing_settings_t;

typedef enum
mtk_camera_metadata_enum_halcore_poweron_stage {
  MTK_HALCORE_POWERON_OPEN_STAGE = 0,
  MTK_HALCORE_POWERON_PRELAUNCH_STAGE,
  MTK_HALCORE_POWERON_RUNTIME_STAGE,
} mtk_camera_metadata_enum_halcore_poweron_stage_t;

typedef enum
mtk_camera_metadata_enum_halcore_sensor_mode {
  /** For preview scenario usually */
  MTK_HALCORE_SENSOR_MODE_PREVIEW = 0,
  /** For capture scenario usually */
  MTK_HALCORE_SENSOR_MODE_CAPTURE,
  /** For video scenario usually */
  MTK_HALCORE_SENSOR_MODE_VIDEO,
  /** For slow motion video scenario usually */
  MTK_HALCORE_SENSOR_MODE_SLIM_VIDEO1,
  /** @deprecated Not used currently */
  MTK_HALCORE_SENSOR_MODE_SLIM_VIDEO2,
  /**
   * Customized sensor mode start section value, customized sensor mode
   * value should be greater then this enumerator, for example:
   *  @code
   *    CUSTOM_SENSOR_MODE_1 = MTK_HALCORE_SENSOR_MODE_CUSTOM + 1,
   *    CUSTOM_SENSOR_MODE_2,
   *    ...
   *  @endcode
   */
  MTK_HALCORE_SENSOR_MODE_CUSTOM = 0x100,
} mtk_camera_metadata_enum_halcore_sensor_mode_t;

//
typedef enum mtk_camera_metadata_enum_postprocdev_device_type {
  MTK_POSTPROCDEV_DEVICE_TYPE_VSE = 0,
  MTK_POSTPROCDEV_DEVICE_TYPE_WPEPQ,
  MTK_POSTPROCDEV_DEVICE_TYPE_CAPTURE,
  MTK_POSTPROCDEV_DEVICE_TYPE_FD,
  MTK_POSTPROCDEV_DEVICE_TYPE_EIS,
  MTK_POSTPROCDEV_DEVICE_TYPE_BOKEH,
} mtk_camera_metadata_enum_postprocdev_device_type_t;

typedef enum mtk_camera_metadata_enum_postprocdev_tuning_action {
  MTK_POSTPROCDEV_ACTION_UNDEFINED = 0,
  MTK_POSTPROCDEV_ACTION_PREVIEW,
  MTK_POSTPROCDEV_ACTION_RECORD,
  MTK_POSTPROCDEV_ACTION_CAPTURE,
} mtk_camera_metadata_enum_postprocdev_tuning_action_t;

// MTK_POSTPROC_VSE_AVAILABLE_FEATURES
typedef enum mtk_camera_metadata_enum_postprocdev_vse_available_features {
  /** Indicates invalid feature */
  MTK_POSTPROCDEV_VSE_AVAILABLE_FEATURES_NONE = 0x0000,

  /** Motion Compensation Noise Reduction for streaming. */
  MTK_POSTPROCDEV_VSE_AVAILABLE_FEATURES_YUVMCNR = 0x0001,

  /**
   * Low latency MCNR cannot be per frame enabled/disabled and conflict
   * with other features.
   */
  MTK_POSTPROCDEV_VSE_AVAILABLE_FEATURES_YUVMCNR_LOW_LATENCY = 0x0002,
  /**
   * Use XTRAW to process raw and split multiple yuv out
   */
  MTK_POSTPROCDEV_VSE_AVAILABLE_FEATURES_RAW_SPLITTER = 0x0003,
  /**
   * Use offline pipeline to support raw-in -> streaing yuv out
   */
  MTK_POSTPROCDEV_VSE_AVAILABLE_FEATURES_TK_STREAMING = 0x0004,
} mtk_camera_metadata_enum_postprocdev_vse_available_features_t;

/**
 * This enumeration is value of tag `MTK_POSTPROCDEV_VSE_TUNING_HINT`.
 */
typedef enum mtk_camera_metadata_enum_postprocdev_vse_tuning_hint {
  /** The default tuning hint. */
  MTK_POSTPROCDEV_VSE_TUNING_HINT_DEFAULT = 0,
} mtk_camera_metadata_enum_postprocdev_vse_tuning_hint_t;

// MTK_POSTPROCDEV_VSE_FEATURE_CONTROL
typedef enum mtk_camera_metadata_enum_postprocdev_vse_feature_control {
  /**
   * MCNR supports inline warp, this feature control is to enable or
   * disable inline warp function (default is disabled).
   *  @par Valid Timing
   *       - Config
   *       - Per-frame
   *  @par Possible Values
   *       - @c 0 Disable.
   *       - Otherwise enable.
   *  @note
   *  If enabled inline warp, caller has responsibility to configure
   *  `MTK_POSTPROCDEV_VSE_STREAM_USAGE_WARP_TABLE` to set warp table for
   *  each frame.
   */
  MTK_POSTPROCDEV_VSE_FEATURE_CONTROL_YUVMCNR_INLINE_WARP_ENABLE,

  /**
   * MCNR supports PQ_DIP direct-link to DIP driver. This feature control
   * is to enable or disable PQ-DIP function (default is disabled).
   *  @par Valid Timing
   *       - Config
   *       - Per-frame
   *  @par Possible Values
   *       - @c 0 Disable.
   *       - Otherwise enable.
   *  @note
   *  Default is disabled.
   */
  MTK_POSTPROCDEV_VSE_FEATURE_CONTROL_YUVMCNR_PQ_DIP_ENABLE,

  /**
   * MCNR supports down scale level for 2, 4 or 4/2 mixed. Higher level
   * indicates higher quality but slower performance.
   *  @par Valid Timing
   *       - Config
   *  @par Possible Values
   *       - @c mtk_camera_metadata_enum_postprocdev_vse_yuvmcnr_downscale_level
   *  @note Downscale level 4/2 mixed only valid for MCNR low latency mode.
   *  @sa `mtk_camera_metadata_enum_postprocdev_vse_yuvmcnr_downscale_level`
   */
  MTK_POSTPROCDEV_VSE_FEATURE_CONTROL_YUVMCNR_DOWNSCALE_LEVEL,
} mtk_camera_metadata_enum_postprocdev_vse_feature_control_t;

typedef enum mtk_camera_metadata_enum_postprocdev_vse_concurrent_request_hint {
  /**
   * Default behavior of the given concurrent device set
   *  @note Not all features support this hint.
   *  @par Possible Values
   *       Any integers are acceptable, but no effect.
   */
  MTK_POSTPROCDEV_VSE_CONCURRENT_REQUEST_HINT_DEFAULT = 0,

  /**
   * Re-order the requests from the given concurrent devices set.
   *  @note Not all features support this hint.
   *  @par Possible Values
   *       Index, starts from 0.
   */
  MTK_POSTPROCDEV_VSE_CONCURRENT_REQUEST_HINT_REORDER,
} mtk_camera_metadata_enum_postprocdev_vse_concurrent_request_hint_t;

// MTK_POSTPROCDEV_VSE_FEATURE_CONTROL_YUVMCNR_DOWNSCALE_LEVEL
typedef enum mtk_camera_metadata_enum_postprocdev_vse_yuvmcnr_downscale_level {
  /** Downscale level 2 */
  MTK_POSTPROCDEV_VSE_YUVMCNR_DOWNSCALE_LEVEL_2 = 2,
  /** Downscale level 4 */
  MTK_POSTPROCDEV_VSE_YUVMCNR_DOWNSCALE_LEVEL_4 = 4,
  /** Downscale level 4/2 mixed only valid for low latency mode MCNR */
  MTK_POSTPROCDEV_VSE_YUVMCNR_DOWNSCALE_LEVEL_4_2 = 42,
} mtk_camera_metadata_enum_postprocdev_vse_yuvmcnr_downscale_level_t;

/**
 * This is value of tag `MTK_POSTPROCDEV_VSE_STREAM_USAGE_MAP`.
 *  @sa `MTK_POSTPROCDEV_VSE_STREAM_USAGE_MAP`.
 */
typedef enum mtk_camera_metadata_enum_postprocdev_vse_stream_usage_map {
  /**
   * Identify for main input of MCNR / MCNR Low Latency, must have.
   */
  MTK_POSTPROCDEV_VSE_STREAM_USAGE_YUVMCNR_F0 = 0,

  /**
   * Identify for scaled input of MCNR / MCNR Low Latency, optional,
   * the resolution of this stream must be 1/2 width, 1/2 height of F0 if the
   * given DOWNSCALE_LEVEL is 2, must be 1/4 width, 1/4 height of F0 if the
   * given DOWNSCALE_LEVEL is 4 or 42. And, the wdith height must be the
   * multiple of 2.
   *  @sa mtk_camera_metadata_enum_postprocdev_vse_yuvmcnr_downscale_level
   */
  MTK_POSTPROCDEV_VSE_STREAM_USAGE_YUVMCNR_F1,

  /**
   * Identify for ME statistics. The width and height must be the multiple of
   * 16.
   *  @note If given ME, F1 must be given too.
   */
  MTK_POSTPROCDEV_VSE_STREAM_USAGE_YUVMCNR_ME,

  /**
   * Identify for temporary ME statistics. TME of request N is the ME of
   * request N-1 for general MCNR, N-2 for low latency MCNR.
   *  @note The first request do not need TME.
   */
  MTK_POSTPROCDEV_VSE_STREAM_USAGE_YUVMCNR_TME,

  /**
   * Identify for warp table.
   *  @note This stream must be given if
   *  `MTK_POSTPROCDEV_VSE_FEATURE_CONTROL_YUVMCNR_INLINE_WARP_ENABLE`
   *  was true.
  *   @see MTK_POSTPROCDEV_VSE_FEATURE_CONTROL_YUVMCNR_INLINE_WARP_ENABLE
   */
  MTK_POSTPROCDEV_VSE_STREAM_USAGE_WARP_TABLE,
} mtk_camera_metadata_enum_postprocdev_vse_stream_usage_map_t;

/**
 * Priority controls are for caller to specify execution priority of
 * multiple Video Stream Engines which are executing simultaneously.
 */
typedef enum mtk_camera_metadata_enum_postprocdev_vse_stream_priority_control {
  /** Default priority */
  MTK_POSTPROCDEV_VSE_STREAM_PRIORITY_DEFAULT = 0,

  /** `MTK_POSTPROCDEV_VSE_STREAM_PRIORITY_PREVIEW` is the highest priority */
  MTK_POSTPROCDEV_VSE_STREAM_PRIORITY_PREVIEW,

  /** The priority for recording. */
  MTK_POSTPROCDEV_VSE_STREAM_PRIORITY_RECORD,

  /** Lower priority for auxiliary stream 1 */
  MTK_POSTPROCDEV_VSE_STREAM_PRIORITY_AUX_1,

  /** Lower priority for auxiliary stream 2 */
  MTK_POSTPROCDEV_VSE_STREAM_PRIORITY_AUX_2,
} mtk_camera_metadata_enum_postprocdev_vse_stream_priority_control_t;

// MTK_POSTPROCDEV_VSE_NDD_CONTROL
/**
 * @see MTK_POSTPROCDEV_VSE_NDD_CONTROL
 */
typedef enum mtk_camera_metadata_enum_postprocdev_vse_ndd_control {
  /** Disable NDD */
  MTK_POSTPROCDEV_VSE_NDD_CONTROL_DISABLED = 0,

  /** Dump Preview */
  MTK_POSTPROCDEV_VSE_NDD_CONTROL_PREVIEW,

  /** Dump Analysis */
  MTK_POSTPROCDEV_VSE_NDD_CONTROL_ANALYSIS,

  /** Dump Auxiliary */
  MTK_POSTPROCDEV_VSE_NDD_CONTROL_AUXILIARY,
} mtk_camera_metadata_enum_postprocdev_vse_ndd_control_t;

// MTK_POSTPROCDEV_VSE_FEATURE_HINT,
/**
 * @see MTK_POSTPROCDEV_VSE_FEATURE_HINT
 */
typedef enum mtk_camera_metadata_enum_postprocdev_vse_feature_hint {
  /** Normal */
  MTK_POSTPROCDEV_VSE_FEATURE_HINT_NORMAL = 0ULL,

  /** Preview */
  MTK_POSTPROCDEV_VSE_FEATURE_HINT_PREVIEW = 1ULL << 1,

  /** Video */
  MTK_POSTPROCDEV_VSE_FEATURE_HINT_VIDEO = 1ULL << 2,

  /** EIS */
  MTK_POSTPROCDEV_VSE_FEATURE_HINT_EIS = 1ULL << 3,

  /** VHDR */
  MTK_POSTPROCDEV_VSE_FEATURE_HINT_VHDR = 1ULL << 4,

  /** E2EHDR */
  MTK_POSTPROCDEV_VSE_FEATURE_HINT_E2EHDR = 1ULL << 5,

  /** VSDOF */
  MTK_POSTPROCDEV_VSE_FEATURE_HINT_VSDOF = 1ULL << 6,
} mtk_camera_metadata_enum_postprocdev_vse_feature_hint_t;

// MTK_POSTPROCDEV_WPEPQ_ROTATE
typedef enum mtk_camera_metadata_enum_postprocdev_wpepq_rotate {
  MTK_POSTPROCDEV_WPEPQ_ROTATE_NONE = 0,
  MTK_POSTPROCDEV_WPEPQ_ROTATE_90,
  MTK_POSTPROCDEV_WPEPQ_ROTATE_180,
  MTK_POSTPROCDEV_WPEPQ_ROTATE_270,
} mtk_camera_metadata_enum_postprocdev_wpepq_rotate_t;

/**
 * This enumeration is value of tag `MTK_POSTPROCDEV_BOKEH_INPUT_SOURCE`.
 */
typedef enum mtk_camera_metadata_enum_postprocdev_bokeh_input_source {
  MTK_POSTPROCDEV_BOKEH_INPUT_SOURCE_MTK_DEPTH = 0,  /* default value */
  MTK_POSTPROCDEV_BOKEH_INPUT_SOURCE_CUSTOM_DEPTH,
} mtk_camera_metadata_enum_postprocdev_bokeh_input_source_t;

/**
 * This is value of tag `MTK_POSTPROCDEV_BOKEH_STREAM_USAGE_MAP`.
 *  @sa `MTK_POSTPROCDEV_BOKEH_STREAM_USAGE_MAP`.
 */
typedef enum mtk_camera_metadata_enum_postprocdev_bokeh_stream_usage_map {
  /**
   * Input Image.
   * Clean Main YUV from MCNR.
   *  @note This input is necessary.
   */
  MTK_POSTPROCDEV_BOKEH_STREAM_USAGE_MAIN_YUV = 0,

  /**
   * Input Image.
   * Depth is a output from Depth Engine.
   *  @note This input is necessary.
   */
  MTK_POSTPROCDEV_BOKEH_STREAM_USAGE_DEPTH,

  /**
   * Input Image.
   * Confidence Map is an output from Depth Engine.
   *  @note This is an optional input. The callers are expected to set this
   *        input when the capabilities of GFNode is needed.
   *        (e.g. using MTK Depth Engine)
   */
  MTK_POSTPROCDEV_BOKEH_STREAM_USAGE_CONFIDENCE_MAP,

  /**
   * Input Image.
   * Guide Image from Depth Engine.
   *  @note This is an optional input. The callers are expected to set this
   *        input when the capabilities of GFNode is needed.
   *        (e.g. using MTK Depth Engine)
   */
  MTK_POSTPROCDEV_BOKEH_STREAM_USAGE_GUIDE_IMAGE,

  /**
   * Input Image.
   * Blur Map is an output from GFNode, and used by BokehNode.
   *  @note This is an optional input. The callers are expected to set this
   *        input when the capabilities of GFNode is needed.
   *        (e.g. using MTK Depth Engine)
   */
  MTK_POSTPROCDEV_BOKEH_STREAM_USAGE_BLUR_MAP,
} mtk_camera_metadata_enum_postprocdev_bokeh_stream_usage_map_t;

/**
 * This is value of tag `MTK_POSTPROCDEV_BOKEH_STEREO_SCENARIO`.
 *  @sa `MTK_POSTPROCDEV_BOKEH_STEREO_SCENARIO`.
 */
typedef enum mtk_camera_metadata_enum_postprocdev_bokeh_stereo_scenario_map {
  // Preview
  MTK_POSTPROCDEV_BOKEH_STEREO_SCENARIO_PREVIEW = 0,

  // Record
  MTK_POSTPROCDEV_BOKEH_STEREO_SCENARIO_RECORD,
} mtk_camera_metadata_enum_postprocdev_bokeh_stereo_scenario_map_t;

typedef enum mtk_camera_metadata_enum_halcore_callback_rule {
  MTK_HALCORE_CALLBACK_RULE_DEFAULT = 0x00000000U,
  // MTK callback rule
  MTK_HALCORE_MTK_CALLBACK_RULE_METAMERGE = 0x00000001U,
  MTK_HALCORE_MTK_CALLBACK_RULE_METAPENDING = 0x00000002U,
  MTK_HALCORE_MTK_CALLBACK_RULE_MASK = 0x0000000FU,
  // AOS callbackP rule
  MTK_HALCORE_AOSP_CALLBACK_RULE_META_RULE = 0x00000010U,
  MTK_HALCORE_AOSP_CALLBACK_RULE_MASK = 0x000000F0U,
  //
  MTK_HALCORE_BYPASS_ALL = 0x10000000U,
} mtk_camera_metadata_enum_halcore_callback_rule_t;

/**
 * Configure EIS mode as IIR or FIR
 */
typedef enum mtk_platform_metadata_enum_postprocdev_eis_mode {
  /**
   * Set EIS mode as IIR, do not queue warpmap
   */
  MTK_POSTPROCDEV_EIS_MODE_IIR = 0,

  /**
   * Set EIS mode as FIR, queue warpmap in EIS engine and delay callback
   */
  MTK_POSTPROCDEV_EIS_MODE_FIR = 1,
} mtk_platform_metadata_enum_postprocdev_eis_mode_t;

/**
 * Controls the internal queue of statistics for FIR filter calculation.
 */
typedef enum mtk_platform_metadata_enum_postprocdev_fir_filter_hint {
  /**
   * Keep use the previous hint, if no hint given yet, default use
   * MTK_EIS_STATE_NONE for IIR filter.
   */
  MTK_POSTPROCDEV_EIS_FIR_FILTER_PRESERVE = 0,

  /** Ask EIS Engine to keep queuing statistics for FIR filter. */
  MTK_POSTPROCDEV_EIS_FIR_FILTER_PUSH_QUEUE = 1,

  /**
   * Stop queuing and start to return warpmap calculated from queued statistics
   * by FIR filter calculation.
   */
  MTK_POSTPROCDEV_EIS_FIR_FILTER_POP_QUEUE = 2,
} mtk_platform_metadata_enum_postprocdev_fir_filter_hint_t;

/**
 * Describe the FIR status of EIS engine
 */
typedef enum mtk_platform_metadata_enum_postprocdev_eis_fir_state {
  /**
   * IIR filter is used
   */
  MTK_POSTPROCDEV_EIS_FIR_STATE_NONE = 0,
  /**
   * EIS engine starts to queue warpmap
   */
  MTK_POSTPROCDEV_EIS_FIR_STATE_START = 1,
  /**
   * EIS engine is queueing warpmap, in this stage user cannot receive FIR warpmap
   */
  MTK_POSTPROCDEV_EIS_FIR_STATE_PUSH = 2,
  /**
   * Queue warpmap complete, EIS engine returns queued FIR warpmap to user
   */
  MTK_POSTPROCDEV_EIS_FIR_STATE_PUSH_DONE = 3,
  /**
   * EIS engine is popping queued FIR warpmap to user
   */
  MTK_POSTPROCDEV_EIS_FIR_STATE_POP = 4,
} mtk_platform_metadata_enum_postprocdev_eis_fir_state_t;

/**
 * Describe RealTime callback type
 */
typedef enum mtk_camera_metadata_enum_halcore_callback_realtime_mode {
  /**
   * No realtime property is set for this partial metadata
   */
  MTK_HALCORE_CALLBACK_NONE_REALTIME = 0x00000000U,
  /**
   * Real-time partial metadata would be called back along with
   * the most recent partials available
   */
  MTK_HALCORE_CALLBACK_SOFT_REALTIME = 0x00000001U,
  /**
   * Real-time partial metadata would be available to be called back immediately
   */
  MTK_HALCORE_CALLBACK_HARD_REALTIME = 0x00000002U,
} mtk_camera_metadata_enum_halcore_callback_realtime_mode_t;

#endif  // INCLUDE_MTKCAM_HALIF_UTILS_METADATA_TAG_1_X_MTK_PRIVATE_METADATA_TAG_H_

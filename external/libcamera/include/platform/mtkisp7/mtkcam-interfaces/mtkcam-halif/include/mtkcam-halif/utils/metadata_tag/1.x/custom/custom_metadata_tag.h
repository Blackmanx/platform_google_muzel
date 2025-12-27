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

#ifndef INCLUDE_MTKCAM_HALIF_UTILS_METADATA_TAG_1_X_CUSTOM_METADATA_TAG_H_
#define INCLUDE_MTKCAM_HALIF_UTILS_METADATA_TAG_1_X_CUSTOM_METADATA_TAG_H_

typedef enum custom_camera_metadata_section {
  /**
   * Custom Camera vendor tags.
   *
   * The client of camera hal process could recognize these tags and interact
   * with camera hal process within these tags. Interaction between hal process
   * and client might cause some transfer and IPC overhead.
   *
   * @par Range
   *      - 0x90000000 - 0x9FFFFFFF
   *
   */
  CUSTOM_VENDOR_TAG_SECTION       = 0x9000,   // WARNING!!! DO NOT MODIFY
  CUSTOM_CAMERA_MODE              = 0,
  CUSTOM_CAMERA_STREAMING_FEATURE = 1,
  CUSTOM_CAMERA_CAPTURE_FEATURE   = 2,
  CUSTOM_VENDOR_TAG_SECTION_COUNT,            // WARNING!!! DO NOT MODIFY
  CUSTOM_VENDOR_TAG_SECTION_END   = 0x9FFF,   // WARNING!!! DO NOT MODIFY
} custom_camera_metadata_section_t;

/**
 * Hierarchy positions in enum space. All vendor extension tags must be
 * defined with tag >= VENDOR_SECTION_START
 */
typedef enum custom_camera_metadata_section_start {
  CUSTOM_VENDOR_TAG_SECTION_START =           // WARNING!!! DO NOT MODIFY
            CUSTOM_VENDOR_TAG_SECTION << 16,  // WARNING!!! DO NOT MODIFY
  CUSTOM_CAMERA_MODE_START = (CUSTOM_CAMERA_MODE +
                            CUSTOM_VENDOR_TAG_SECTION) << 16,
  CUSTOM_CAMERA_STREAMING_FEATURE_START = (CUSTOM_CAMERA_STREAMING_FEATURE +
                            CUSTOM_VENDOR_TAG_SECTION) << 16,
  CUSTOM_CAMERA_CAPTURE_FEATURE_START = (CUSTOM_CAMERA_CAPTURE_FEATURE +
                            CUSTOM_VENDOR_TAG_SECTION) << 16,
} custom_camera_metadata_section_start_t;

/**
 * Main enum for defining camera metadata tags.  New entries must always go
 * before the section _END tag to preserve existing enumeration values.  In
 * addition, the name and type of the tag needs to be added to
 * ""
 */
typedef enum custom_camera_metadata_tag {
  // Example, can be removed
  CUSTOM_CAMERA_MODE_ONE = CUSTOM_CAMERA_MODE_START,
  CUSTOM_CAMERA_MODE_TWO,
  CUSTOM_CAMERA_MODE_END,

  CUSTOM_CAMERA_STREAMING_FEATURE_ONE = CUSTOM_CAMERA_STREAMING_FEATURE_START,
  CUSTOM_CAMERA_STREAMING_FEATURE_END,

  CUSTOM_CAMERA_CAPTURE_FEATURE_ONE = CUSTOM_CAMERA_CAPTURE_FEATURE_START,
  CUSTOM_CAMERA_CAPTURE_FEATURE_END,
} custom_camera_metadata_tag_t;

#endif  // INCLUDE_MTKCAM_HALIF_UTILS_METADATA_TAG_1_X_CUSTOM_METADATA_TAG_H_

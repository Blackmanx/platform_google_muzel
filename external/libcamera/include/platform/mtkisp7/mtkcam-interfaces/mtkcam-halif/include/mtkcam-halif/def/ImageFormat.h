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

#ifndef INCLUDE_MTKCAM_HALIF_DEF_IMAGEFORMAT_H_
#define INCLUDE_MTKCAM_HALIF_DEF_IMAGEFORMAT_H_

#include <cstdint>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

/**
 * Transformation definitions.
 *  @note ROT_90 is applied CLOCKWISE and AFTER TRANSFORM_FLIP_{H|V}.
 */
enum ETransform {
  /** Neither rotation nor flipping. */
  eTransform_None = 0x00,

  /** Flip source image horizontally (around the vertical axis) */
  eTransform_FLIP_H = 0x01,

  /** Flip source image vertically (around the horizontal axis)*/
  eTransform_FLIP_V = 0x02,

  /** Rotate source image 90 degrees clockwise */
  eTransform_ROT_90 = 0x04,

  /** Rotate source image 180 degrees */
  eTransform_ROT_180 = 0x03,

  /** Rotate source image 270 degrees clockwise */
  eTransform_ROT_270 = 0x07,
};

/**
 * Describe sensor output data arrangement.
 */
enum ESensorColorArrangement {
  /** Bayer format, data arrangement is BG/GR. */
  eSensorColorArrangement_BAYER_BGGR = 0x0,

  /** Bayer format, data arrangement is GB/RG. */
  eSensorColorArrangement_BAYER_GBRG,

  /** Bayer format, data arrangement is GR/BG. */
  eSensorColorArrangement_BAYER_GRBG,

  /** Bayer format, data arrangement is RG/GB. */
  eSensorColorArrangement_BAYER_RGGB,

  /** YUV format, data arrangement is UYVY. */
  eSensorColorArrangement_YUV_UYVY,

  /** YUV format, data arrangement is VYUY. */
  eSensorColorArrangement_YUV_VYUY,

  /** YUV format, data arrangement is YUYV. */
  eSensorColorArrangement_YUV_YUYV,

  /** YUV format, data arrangement is YVYU. */
  eSensorColorArrangement_YUV_YVYU,

  /** MONO stream. */
  eSensorColorArrangement_MONO,
};

/**
 * Mediatek image buffer format definitions.
 */
enum EImageFormat {
  /** HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED */
  eImgFmt_IMPLEMENTATION_DEFINED = 34,

  /** RAW 16-bit 1 plane */
  eImgFmt_RAW16 = 32,

  /** RAW 1 plane */
  eImgFmt_RAW_OPAQUE = 36,

  /** RAW 10-bit 1 plane */
  eImgFmt_RAW10 = 37,

  /**
   * This format is used to carry task-specific data which does not have a
   * standard image structure. The details of the format are left to the two
   * endpoints.
   *
   * Buffers of this format must have a height of 1, and width equal to their
   * size in bytes.
   *
   *  @note Equals to AOSP `HAL_PIXEL_FORMAT_BLOB`
   */
  eImgFmt_BLOB = 33,

  /** HAL_PIXEL_FORMAT_YCrCb_420_888 */
  eImgFmt_YCBCR_420_888 = 35,

  /** HAL_PIXEL_FORMAT_RGBA_8888 (32-bit; LSB:R, MSB:A), 1 plane */
  eImgFmt_RGBA8888 = 1,

  /** HAL_PIXEL_FORMAT_RGBX_8888 (32-bit; LSB:R, MSB:X), 1 plane */
  eImgFmt_RGBX8888 = 2,

  /** HAL_PIXEL_FORMAT_RGB_888   (24-bit), 1 plane (RGB) */
  eImgFmt_RGB888 = 3,

  /** HAL_PIXEL_FORMAT_RGB_565   (16-bit), 1 plane */
  eImgFmt_RGB565 = 4,

  /** HAL_PIXEL_FORMAT_BGRA_8888 (32-bit; LSB:B, MSB:A), 1 plane */
  eImgFmt_BGRA8888 = 5,

  /** HAL_PIXEL_FORMAT_YCbCr_422_I 422 format, 1 plane (YUYV) */
  eImgFmt_YUY2 = 20,

  /** HAL_PIXEL_FORMAT_YCbCr_422_SP 422 format, 2 plane (Y),(UV) */
  eImgFmt_NV16 = 16,

  /** HAL_PIXEL_FORMAT_YCrCb_420_SP 420 format, 2 plane (Y),(VU) */
  eImgFmt_NV21 = 17,

  /**HAL_PIXEL_FORMAT_NV12 420 format, 2 plane (Y),(UV) */
  eImgFmt_NV12 = 0x00001000,

  /** HAL_PIXEL_FORMAT_YV12 420 format, 3 plane (Y),(V),(U) */
  eImgFmt_YV12 = 842094169,

  /** HAL_PIXEL_FORMAT_Y8 8-bit Y plane */
  eImgFmt_Y8 = 538982489,

  /** @deprecated Replace it with eImgFmt_Y8 */
  eImgFmt_Y800 = eImgFmt_Y8,

  /** HAL_PIXEL_FORMAT_Y16 16-bit Y plane */
  eImgFmt_Y16 = 540422489,

  /** HAL_PIXEL_FORMAT_CAMERA_OPAQUE Opaque format, RAW10 + Metadata */
  eImgFmt_CAMERA_OPAQUE = 0x00000111,

  /** HAL_PIXEL_FORMAT_YCBCR_P010 420 format, 16bit, 2 plane (Y),(UV) = P010 */
  eImgFmt_YUV_P010 = 54,

  //
  //  0x2000 - 0x2FFF
  //
  //  This range is reserved for pixel formats that are specific to the HAL
  //  implementation.
  //
  /** Unknown */
  eImgFmt_UNKNOWN = 0x0000,

  /** Mediatek definition start hint enumeration, no used */
  eImgFmt_VENDOR_DEFINED_START = 0x2000,

  /** Indicate to YUV start enumeration hint enumeration, no used */
  eImgFmt_YUV_START = eImgFmt_VENDOR_DEFINED_START,

  /** Standard 422 format, 1 plane (YVYU) */
  eImgFmt_YVYU = eImgFmt_YUV_START,

  /** Standard 422 format, 1 plane (UYVY) */
  eImgFmt_UYVY,

  /** Standard 422 format, 1 plane (VYUY) */
  eImgFmt_VYUY,

  /** Standard 422 format, 2 plane (Y),(VU) */
  eImgFmt_NV61,

  /** 420 format block mode, 2 plane (Y),(UV) */
  eImgFmt_NV12_BLK,

  /** 420 format block mode, 2 plane (Y),(VU) */
  eImgFmt_NV21_BLK,

  /** 422 format, 3 plane (Y),(V),(U) */
  eImgFmt_YV16,

  /** 420 format, 3 plane (Y),(U),(V) */
  eImgFmt_I420,

  /** 422 format, 3 plane (Y),(U),(V) */
  eImgFmt_I422,

  /** 422 format, 10 bits data stored in 16 bits, 1 plane (YUYV) = Y210 */
  eImgFmt_YUYV_Y210,

  /** 422 format, 10 bits data stored in 16 bits, 1 plane (YVYU) */
  eImgFmt_YVYU_Y210,

  /** 422 format, 10 bits data stored in 16 bits, 1 plane (UYVY) */
  eImgFmt_UYVY_Y210,

  /** 422 format, 10 bits data stored in 16 bits, 1 plane (VYUY) */
  eImgFmt_VYUY_Y210,

  /** 422 format, 10 bits data stored in 16 bits, 2 plane (Y),(UV) = P210 */
  eImgFmt_YUV_P210,

  /** 422 format, 10 bits data stored in 16 bits, 2 plane (Y),(VU) */
  eImgFmt_YVU_P210,

  /** 422 format, 10 bits data stored in 16 bits, 3 plane (Y),(U),(V) */
  eImgFmt_YUV_P210_3PLANE,

  /** 420 format, 10 bits data stored in 16 bits, 2 plane (Y),(VU) */
  eImgFmt_YVU_P010,

  /** 420 format, 10 bits data stored in 16 bits, 3 plane (Y),(U),(V) */
  eImgFmt_YUV_P010_3PLANE,

  /**
   * Mediatek packed 10-bit 422 YUV, 1 plane. Each channel was sampled by 10-bit
   * and packed together. E.g.:
   *  @code
   *           <-- 10-bit --><-- 10-bit --><-- 10-bit --><-- 10-bit --><-->
   *   line 0: [     Y      ][     U      ][     Y      ][     V      ][  ]
   *   line 1: [     Y      ][     U      ][     Y      ][     V      ][  ]
   *           <---------------------- stride ---------------------------->
   * @endcode
   * The data accuracy is 10-bit, caller could easily truncate 2 lower bits
   * for standard 8-bit bayer (or using normalization).
   */
  eImgFmt_MTK_YUYV_Y210,

  /**
   * Mediatek packed 10-bit 422 YUV, 1 plane. Different YUV arrangement
   * between `eImgFmt_MTK_YUYV_Y210`.
   *  @sa eImgFmt_MTK_YUYV_Y210
   */
  eImgFmt_MTK_YVYU_Y210,

  /** @copydoc eImgFmt_MTK_YVYU_Y210 */
  eImgFmt_MTK_UYVY_Y210,

  /** @copydoc eImgFmt_MTK_YVYU_Y210 */
  eImgFmt_MTK_VYUY_Y210,

  /**
   * Mediatek packed 10-bit 422 YUV, 2 planes. Each channel was sampled by
   * 10-bit and packed together. E.g.:
   *  @code
   *   Y plane:
   *           <-- 10-bit --><-- 10-bit --><-- 10-bit --><-- 10-bit --><-->
   *   line 0: [     Y      ][     Y      ][     Y      ][     Y      ][  ]
   *   line 1: [     Y      ][     Y      ][     Y      ][     Y      ][  ]
   *           <---------------------- stride ---------------------------->
   *
   *
   *   UV plane:
   *           <-- 10-bit --><-- 10-bit --><-- 10-bit --><-- 10-bit --><-->
   *   line 0: [     U      ][     V      ][     U      ][     V      ][  ]
   *   line 1: [     U      ][     V      ][     U      ][     V      ][  ]
   *           <---------------------- stride ---------------------------->
   *  @endcode
   * The data accuracy is 10-bit, caller could easily truncate 2 lower bits
   * for standard 8-bit bayer (or using normalization).
   */
  eImgFmt_MTK_YUV_P210,

  /**
   * Mediatek packed 10-bit 422 YUV, 2 plane. Different YUV arrangement
   * between `eImgFmt_MTK_YUV_P210`. (Y), (VU)
   *  @sa eImgFmt_MTK_YUV_P210
   */
  eImgFmt_MTK_YVU_P210,

  /**
   * Mediatek packed 10-bit 422 YUV, 3 planes. Each channel was sampled by
   * 10-bit and packed together. E.g.:
   *  @code
   *   Y plane:
   *           <-- 10-bit --><-- 10-bit --><-- 10-bit --><-- 10-bit --><-->
   *   line 0: [     Y      ][     Y      ][     Y      ][     Y      ][  ]
   *   line 1: [     Y      ][     Y      ][     Y      ][     Y      ][  ]
   *           <---------------------- stride ---------------------------->
   *
   *
   *   U plane:
   *           <-- 10-bit --><-- 10-bit --><-- 10-bit --><-- 10-bit --><-->
   *   line 0: [     U      ][     U      ][     U      ][     U      ][  ]
   *   line 1: [     U      ][     U      ][     U      ][     U      ][  ]
   *           <---------------------- stride ---------------------------->
   *
   *   V plane:
   *           <-- 10-bit --><-- 10-bit --><-- 10-bit --><-- 10-bit --><-->
   *   line 0: [     V      ][     V      ][     V      ][     V      ][  ]
   *   line 1: [     V      ][     V      ][     V      ][     V      ][  ]
   *           <---------------------- stride ---------------------------->
   *  @endcode
   * The data accuracy is 10-bit, caller could easily truncate 2 lower bits
   * for standard 8-bit bayer (or using normalization).
   */
  eImgFmt_MTK_YUV_P210_3PLANE,

  /**
   * Mediatek packed 10-bit 420 YUV, 2-plane. (Y), (UV).
   *  @sa eImgFmt_MTK_YUV_P210
   */
  eImgFmt_MTK_YUV_P010,

  /**
   * Mediatek packed 10-bit 420 YUV, 2-plane. (Y), (VU).
   *  @sa eImgFmt_MTK_YUV_P210
   */
  eImgFmt_MTK_YVU_P010,

  /**
   * Mediatek packed 10-bit 420 YUV, 3-plane. (Y), (U), (V).
   *  @sa eImgFmt_MTK_YUV_P210_3PLANE
   */
  eImgFmt_MTK_YUV_P010_3PLANE,

  /**
   * Standard YUV 420 format, 12 bits data stored in 16 bits,
   * 2 plane (Y), (UV)
   */
  eImgFmt_YUV_P012,

  /**
   * Standard YUV 420 format, 12 bits data stored in 16 bits,
   * 2 plane (Y), (VU)
   */
  eImgFmt_YVU_P012,

  /**
   * Mediatek packed 12-bit 420 YUV, 2-plane. (Y), (UV).
   *  @sa eImgFmt_MTK_YUV_P210
   */
  eImgFmt_MTK_YUV_P012,

  /**
   * Mediatek packed 12-bit 420 YUV, 2-plane. (Y), (VU).
   *  @sa eImgFmt_MTK_YUV_P210
   */
  eImgFmt_MTK_YVU_P012,

  /** RGB format start hint enumeration, no used */
  eImgFmt_RGB_START = 0x2100,  // please add RGB format after this enum

  /** ARGB (32-bit; LSB:A, MSB:B), 1 plane */
  eImgFmt_ARGB8888 = eImgFmt_RGB_START,

  /** @deprecated Replace it with eImgFmt_ARGB8888 */
  eImgFmt_ARGB888 = eImgFmt_ARGB8888,

  /** RGB 48(16x3, 48-bit; LSB:R, MSB:B), 1 plane */
  eImgFmt_RGB48,

  /** RGB, unpacked 8-bit, 3 plane */
  eImgFmt_RGB8_UNPAK_3PLANE,

  /** RGB, unpacked 10-bit, 3 plane */
  eImgFmt_RGB10_UNPAK_3PLANE,

  /** RGB, unpacked 12-bit, 3 plane */
  eImgFmt_RGB12_UNPAK_3PLANE,

  /** RGB, packed 8-bit,  3 plane */
  eImgFmt_RGB8_PAK_3PLANE,

  /** RGB, packed 10-bit, 3 plane */
  eImgFmt_RGB10_PAK_3PLANE,

  /** RGB, packed 12-bit, 3 plane */
  eImgFmt_RGB12_PAK_3PLANE,

  /** Mediatek bayer formats start hint enumeration, no used */
  eImgFmt_RAW_START = 0x2200,  // add RAW format after this enum

  /**
   * Mediatek Bayer format, 8-bit. This data arrangement is the same as
   * MIPI 8-bit raw.
   */
  eImgFmt_BAYER8 = eImgFmt_RAW_START,

  /**
   * Mediatek Bayer format, 10-bit. Each channel was sampled by 10-bit
   * and packed together. E.g.:
   *  @code
   *           <-- 10-bit --><-- 10-bit --><-- 10-bit --><-- 10-bit --><-->
   *   line 0: [     R      ][     G      ][     R      ][     G      ][  ]
   *   line 1: [     G      ][     B      ][     G      ][     B      ][  ]
   *           <---------------------- stride ---------------------------->
   *  @endcode
   * The data accuracy is 10-bit, caller could easily truncate 2 lower bits
   * for standard 8-bit bayer (or using normalization).
   */
  eImgFmt_BAYER10,

  /**
   * Mediatek Bayer format, 12-bit. Each channel was sampled by 12-bit
   * and packed together. E.g.:
   *  @code
   *           <-- 12-bit --><-- 12-bit --><-- 12-bit --><-- 12-bit --><-->
   *   line 0: [     R      ][     G      ][     R      ][     G      ][  ]
   *   line 1: [     G      ][     B      ][     G      ][     B      ][  ]
   *           <---------------------- stride ---------------------------->
   *  @endcode
   * The data accuracy is 12-bit, caller could easily truncate 4 lower bits
   * for standard 8-bit bayer (or using normalization).
   */
  eImgFmt_BAYER12,

  /**
   * Mediatek Bayer format, 14-bit. Each channel was sampled by 14-bit
   * and packed together. E.g.:
   *  @code
   *           <-- 14-bit --><-- 14-bit --><-- 14-bit --><-- 14-bit --><-->
   *   line 0: [     R      ][     G      ][     R      ][     G      ][  ]
   *   line 1: [     G      ][     B      ][     G      ][     B      ][  ]
   *           <---------------------- stride ---------------------------->
   *  @endcode
   * The data accuracy is 14-bit, caller could easily truncate 6 lower bits
   * for standard 8-bit bayer (or using normalization).
   */
  eImgFmt_BAYER14,

  /**
   * Mediatek bayer format with double G channel data. Each channel was
   * sampled by 8-bit, 1 plane.
   * E.g.: The standard bayer order is `RG/GB`, the full-G bayer format would
   *       be `RGG/GBG`. Memory footprint is:
   *  @code
   *
   *   <- N-bit -><- N-bit -><- N-bit -><- N-bit -><- N-bit -><- N-bit -><-->
   *   [    R    ][    G    ][    G    ][    R    ][    G    ][    G    ][  ]
   *   [    G    ][    B    ][    G    ][    G    ][    B    ][    G    ][  ]
   *   <---------------------------- stride -------------------------------->
   *
   *   where N = 8, N is data accuracy.
   *
   *  @endcode
   */
  eImgFmt_FG_BAYER8,

  /**
   * Mediatek bayer format with double G channel data. Each channel was
   * sampled by 10-bit, 1 plane.
   *  @sa eImgFmt_FG_BAYER8
   */
  eImgFmt_FG_BAYER10,

  /**
   * Mediatek bayer format with double G channel data. Each channel was
   * sampled by 12-bit, 1 plane.
   *  @sa eImgFmt_FG_BAYER8
   */
  eImgFmt_FG_BAYER12,

  /**
   * Mediatek bayer format with double G channel data. Each channel was
   * sampled by 14-bit, 1 plane.
   *  @sa eImgFmt_FG_BAYER8
   */
  eImgFmt_FG_BAYER14,

  /**
   * Mediatek bayer format with full G channel data stored in plane 0,
   * and the channel R and B are stored weavely in plane 1.
   *
   *  @code
   *    plane 0
   *
   *      line 0: [G][G][G][G][]
   *      line 1: [G][G][G][G][]
   *      line 2: [G][G][G][G][]
   *      line 3: [G][G][G][G][]
   *              <-- stride -->
   *
   *    plane 1
   *
   *      line 0: [R][B][R][B][]
   *      line 1: [R][B][R][B][]
   *              <-- stride -->
   *  @endcode
   */
  eImgFmt_FG_BAYER8_PAK_2PLANE,

  /**
   * Mediatek bayer format with full G channel data and separated into
   * 2 planes. Each channel was sampled by 10-bit.
   *  @sa eImgFmt_FG_BAYER8_PAK_2PLANE
   */
  eImgFmt_FG_BAYER10_PAK_2PLANE,

  /**
   * Mediatek bayer format with full G channel data and separated into
   * 2 planes. Each channel was sampled by 12-bit.
   *  @sa eImgFmt_FG_BAYER8_PAK_2PLANE
   */
  eImgFmt_FG_BAYER12_PAK_2PLANE,

  /**
   * Mediatek bayer format with full G channel data and stored in plane 0.
   * Each channel was sampled by 8-bit. Plane 1 stores channel R, and
   * plane 2 stores channel G.
   *
   *  @code
   *    plane 0
   *
   *      line 0: [G][G][G][G][]
   *      line 1: [G][G][G][G][]
   *      line 2: [G][G][G][G][]
   *      line 3: [G][G][G][G][]
   *              <-- stride -->
   *
   *    plane 1
   *
   *      line 0: [R][R][]
   *      line 1: [R][R][]
   *              <--+--->
   *                 stride
   *
   *    plane 2
   *
   *      line 0: [B][B][]
   *      line 1: [B][B][]
   *              <--+--->
   *                 stride
   *
   *  @endcode
   */
  eImgFmt_FG_BAYER8_PAK_3PLANE,

  /**
   * Mediatek bayer format with full G channel data and separeted into 3
   * planes. Each channel was sampled by 10-bit.
   *  @sa eImgFmt_FG_BAYER8_PAK_3PLANE
   */
  eImgFmt_FG_BAYER10_PAK_3PLANE,

  /**
   * Mediatek bayer format with full G channel data and separeted into 3
   * planes. Each channel was sampled by 12-bit.
   *  @sa eImgFmt_FG_BAYER8_PAK_3PLANE
   */
  eImgFmt_FG_BAYER12_PAK_3PLANE,

  /**
   * Mediatek bayer format with full G channel data and separeted into 3
   * planes. Each channel was sampled by 8-bit stored in 2 bytes.
   *  @sa eImgFmt_FG_BAYER8_PAK_3PLANE
   */
  eImgFmt_FG_BAYER8_UNPAK_3PLANE,

  /**
   * Mediatek bayer format with full G channel data and separeted into 3
   * planes. Each channel was sampled by 10-bit stored in 2 bytes.
   * The data accuracy is 10-bit, caller could easily truncate 2 lower bits
   * for standard 8-bit bayer (or using normalization)
   *  @sa eImgFmt_FG_BAYER8_PAK_3PLANE
   */
  eImgFmt_FG_BAYER10_UNPAK_3PLANE,

  /**
   * Mediatek bayer format with full G channel data and separeted into 3
   * planes. Each channel was sampled by 12-bit stored in 2 bytes.
   * The data accuracy is 12-bit, caller could easily truncate 4 lower bits
   * for standard 8-bit bayer (or using normalization)
   *  @sa eImgFmt_FG_BAYER8_PAK_3PLANE
   */
  eImgFmt_FG_BAYER12_UNPAK_3PLANE,

  /** Bayer format, 10-bit (MIPI) */
  eImgFmt_BAYER10_MIPI,

  /** Bayer format, 8-bit data unpacked as 16-bit */
  eImgFmt_BAYER8_UNPAK,

  /** Bayer format, 10-bit data unpacked as 16-bit */
  eImgFmt_BAYER10_UNPAK,

  /** Bayer format, 12-bit data unpacked as 16-bit */
  eImgFmt_BAYER12_UNPAK,

  /** Bayer format, 14-bit data unpacked as 16-bit */
  eImgFmt_BAYER14_UNPAK,

  /** Bayer format, 15-bit data unpacked as 16-bit */
  eImgFmt_BAYER15_UNPAK,

  /** Bayer format, 16-bit data */
  eImgFmt_BAYER16_UNPAK,

  /** Bayer format, 22-bit data in 24-bit fields */
  eImgFmt_BAYER22_PAK,

  /** Warp format, 32-bit, 1 plane (X) */
  eImgFmt_WARP_1PLANE,

  /** Warp format, 32-bit, 2 plane (X), (Y) */
  eImgFmt_WARP_2PLANE,

  /** Warp format, 32-bit, 3 plane (X), (Y), (Z) */
  eImgFmt_WARP_3PLANE,

  /** Processed bayer format applied Mediatek lens shading, 16-bit, 0x2217 */
  eImgFmt_BAYER16_APPLY_LSC = eImgFmt_BAYER16_UNPAK,

  /** Blob format starts hint enumeration, no used */
  eImgFmt_BLOB_START = 0x2300,  // please add BLOB format after this enum

  /** JPEG format */
  eImgFmt_JPEG = eImgFmt_BLOB_START,

  /** JPEG 420 format, 3 plane (Y),(U),(V) */
  eImgFmt_JPG_I420,

  /** JPEG 422 format, 3 plane (Y),(U),(V) */
  eImgFmt_JPG_I422,

  /** isp tuning buffer for ISP HIDL */
  eImgFmt_ISP_HIDL_TUNING,

  /** isp tuning buffer from ISP HAL */
  eImgFmt_ISP_TUNING,

  /** Android Q HEIC for exif stream format */
  eImgFmt_JPEG_APP_SEGMENT,

  /** ISP HIDL HEIF */
  eImgFmt_HEIF,

  /** This section is used for non-image-format buffer */
  eImgFmt_STA_START = 0x2400,  // please add STATIC format after this enum

  /** statistic format, 8-bit */
  eImgFmt_STA_BYTE = eImgFmt_STA_START,

  /** statistic format, 16-bit */
  eImgFmt_STA_2BYTE,

  /** statistic format, 32-bit */
  eImgFmt_STA_4BYTE,

  /** statistic format, 10-bit */
  eImgFmt_STA_10BIT,

  /** statistic format, 12-bit */
  eImgFmt_STA_12BIT,

  /** compression data format start hint enumeration, no used */
  eImgFmt_COMPRESSION_START = 0x2500,  // please add compression data after this

  /** UFBC Start enumeration value */
  eImgFmt_UFBC_START = eImgFmt_COMPRESSION_START,

  /**
   * Mediatek compression format for 8-bit bayer.
   *  @sa struct UfbcBufferHeader
   */
  eImgFmt_UFBC_BAYER8 = eImgFmt_UFBC_START,

  /**
   * Mediatek compression format for 10-bit bayer.
   *  @sa struct UfbcBufferHeader
   */
  eImgFmt_UFBC_BAYER10,

  /**
   * Mediatek compression format for 12-bit bayer.
   *  @sa struct UfbcBufferHeader
   */
  eImgFmt_UFBC_BAYER12,

  /**
   * Mediatek compression format for 14-bit bayer.
   *  @sa struct UfbcBufferHeader
   */
  eImgFmt_UFBC_BAYER14,

  /** UFBC YUV 2-plane start */
  eImgFmt_UFBC_YUV_2P_START,

  /**
   * Mediatek compression format for 8-bit YUV 420 2 planes, (Y),(UV)
   *  @sa struct UfbcBufferHeader
   */
  eImgFmt_UFBC_NV12 = eImgFmt_UFBC_YUV_2P_START,

  /**
   * Mediatek compression format for 8-bit YUV 420 2 planes, (Y),(VU)
   *  @sa struct UfbcBufferHeader
   */
  eImgFmt_UFBC_NV21,

  /**
   * Mediatek compression format for 10-bit packed YUV 420 2 planes, (Y),(UV)
   *  @sa struct UfbcBufferHeader
   */
  eImgFmt_UFBC_YUV_P010,

  /**
   * Mediatek compression format for 10-bit packed YUV 420 2 planes, (Y),(VU)
   *  @sa struct UfbcBufferHeader
   */
  eImgFmt_UFBC_YVU_P010,

  /**
   * Mediatek compression format for 12-bit YUV 420 2 planes, (Y),(UV)
   *  @sa struct UfbcBufferHeader
   */
  eImgFmt_UFBC_YUV_P012,

  /**
   * Mediatek compression format for 12-bit YUV 420 2 planes, (Y),(VU)
   *  @sa struct UfbcBufferHeader
   */
  eImgFmt_UFBC_YVU_P012,

  /** UFBC YUV 2-plane end enumeration, no use*/
  eImgFmt_UFBC_YUV_2P_END,

  /** UFBC end hint enumeration, no use */
  eImgFmt_UFBC_END = eImgFmt_UFBC_YUV_2P_END,

  /** ARM Frame Buffer Compression format start hint, no use */
  eImgFmt_AFBC_START,

  /** AFBC 420 format, 2 plane (Y),(UV) */
  eImgFmt_AFBC_NV12 = eImgFmt_AFBC_START,

  /** AFBC 420 format, 2 plane (Y),(VU) */
  eImgFmt_AFBC_NV21,

  /** AFBC 420 format, 10bit, 2 plane (Y),(UV) = P010 */
  eImgFmt_AFBC_MTK_YUV_P010,

  /** AFBC 420 format, 10bit, 2 plane (Y),(VU) */
  eImgFmt_AFBC_MTK_YVU_P010,

  /** AFBC RGBA (32-bit; LSB:R, MSB:A), 1 plane */
  eImgFmt_AFBC_RGBA8888,

  /** ARM Frame Buffer Compression format end */
  eImgFmt_AFBC_END,

  /** Compression data end */
  eImgFmt_COMPRESSION_END = eImgFmt_AFBC_END,
};

/**
 * Describe the YUV image color space.
 *  @note Refer to dataspace defined @ graphics/common/1.0/types.hal
 */
enum EImageColorSpace : int32_t {
  /** unknown or not support colorspace */
  eImgColorSpace_UNKNOWN = 0,

  /** refer to STANDARD_MASK in types.hal */
  eImgColorSpace_STANDARD_SHIFT = 16,
  eImgColorSpace_STANDARD_MASK = 4128768,
  /** refer to STANDARD section of types.hal */
  eImgColorSpace_STANDARD_UNSPECIFIED = 0,
  eImgColorSpace_STANDARD_BT709 = 65536,      // (1 << STANDARD_SHIFT)
  eImgColorSpace_STANDARD_BT601_625 = 131072, // (2 << STANDARD_SHIFT)
  eImgColorSpace_STANDARD_BT2020 = 393216,    // (6 << STANDARD_SHIFT)
  eImgColorSpace_STANDARD_DCI_P3 = 655360,    // (10 << STANDARD_SHIFT)

  /** refer to TRANSFER_MASK in types.hal */
  eImgColorSpace_TRANSFER_SHIFT = 22,
  eImgColorSpace_TRANSFER_MASK = 130023424,

  /** refer to RANGE_MASK in types.hal */
  eImgColorSpace_RANGE_SHIFT = 27,
  eImgColorSpace_RANGE_MASK = 939524096,      // (7 << RANGE_SHIFT)
  /** refer to RANGE section of types.hal */
  eImgColorSpace_RANGE_UNSPECIFIED = 0,       // (0 << RANGE_SHIFT)
  eImgColorSpace_RANGE_FULL = 134217728,      // (1 << RANGE_SHIFT)
  eImgColorSpace_RANGE_LIMITED = 268435456,   // (2 << RANGE_SHIFT)

  /** refer to V0_JFIF defined in types.hal */
  eImgColorSpace_BT601_FULL = 146931712,

  /** refer to V0_BT601_625 defined in types.hal */
  eImgColorSpace_BT601_LIMITED = 281149440,

  /** refer to V0_SRGB_LINEAR defined in types.hal */
  eImgColorSpace_BT709_FULL = 138477568,

  /** refer to V0_BT709 defined in types.hal */
  eImgColorSpace_BT709_LIMITED = 281083904,

  /** refer to BT2020_ITU_PQ defined in types.hal */
  eImgColorSpace_BT2020_PQ_LIMITED = 298188800,

  /** MTK defined dataspace, the combination is
   *  ((STANDARD_BT2020 | TRANSFER_ST2084) | RANGE_FULL) */
  eImgColorSpace_BT2020_PQ_FULL = 163971072,

  eImgColorSpace_DISPLAY_P3 = 143261696,

  eImgColorSpace_INVALID = INT32_MAX,
};

/**
 * Describe Mediatek UFBC (Universal Frame Buffer Compression) buffer header.
 * Mediatek UFBC supports 1 plane Bayer and 2 planes Y/UV image formats so far.
 * For backward compatible, we reserve maximum 3 planes for UFBC buffer.
 * Caller must follow the formulation to calculate the bit stream buffer size
 * and length table buffer size.
 *
 * Header Size
 *
 * Fixed size of 4096 bytes. Reserved bytes will be used by Mediatek
 * ISP driver. Caller SHOULD NOT edit it.
 *
 * Bit Stream Size
 *
 *  @code
 *    // for each plane
 *    size = ((width + 63) / 64) * 64;      // width must be aligned to 64 pixel
 *    size = (size * bitsPerPixel + 7) / 8; // convert to bytes
 *    size = size * height;
 *  @endcode
 *
 * Table Size
 *
 *  @code
 *    // for each plane
 *    size = (width + 63) / 64;
 *    size = ((size + 7) & (~7));
 *    size = size * height;
 *  @endcode
 *
 *  And the memory layout should be followed as:
 *
 *  @code
 *           Bayer                  YUV2P
 *    +------------------+  +--------------------+
 *    |      Header      |  |       Header       |
 *    +------------------+  +--------------------+
 *    |                  |  |      Y Plane       |
 *    | Bayer Bit Stream |  |     Bit Stream     |
 *    |                  |  |                    |
 *    +------------------+  +--------------------+
 *    |   Length Table   |  |      UV Plane      |
 *    +------------------+  |     Bit Stream     |
 *                          |                    |
 *                          +--------------------+
 *                          |      Y Plane       |
 *                          |    Length Table    |
 *                          +--------------------+
 *                          |      UV Plane      |
 *                          |    Length Table    |
 *                          +--------------------+
 *  @endcode
 *
 *  @note Caller has responsibility to fill all the fields according the
 *        real buffer layout.
 */
struct UfbcBufferHeader {
  union {
    struct {
      /** Describe image resolution, unit in pixel. */
      uint32_t width;

      /** Describe image resolution, unit in pixel. */
      uint32_t height;

      /** Describe UFBC data plane count, UFBC supports maximum 3 planes. */
      uint32_t planeCount;

      /** Describe the original image data bits per pixel of the given plane. */
      uint32_t bitsPerPixel[3];

      /**
       * Describe the offset of the given plane bit stream data in bytes,
       * including header size.
       */
      uint32_t bitStreamOffset[3];

      /** Describe the bit stream data size in bytes of the given plane. */
      uint32_t bitStreamSize[3];

      /** Describe the encoded data size in bytes of the given plane. */
      uint32_t bitStreamDataSize[3];

      /**
       * Describe the offset of length table of the given plane, including
       * header size.
       */
      uint32_t tableOffset[3];

      /** Describe the length table size of the given plane */
      uint32_t tableSize[3];

      /** Describe the total buffer size, including buffer header. */
      uint32_t bufferSize;
    };
    uint8_t reserved[4096];
  };
} __attribute__((packed));


/**
 * Helper to check if the given format is Mediatek packed bayer format.
 *  @param fmt Format to check
 *  @return `True` for yes, `false` for not.
 */
static inline bool isHalRawFormat(EImageFormat fmt) {
  return (fmt >= eImgFmt_RAW_START && fmt < eImgFmt_BLOB_START);
}

/**
 * Helper to check if the given format is unpacked bayer format.
 *  @param fmt  Format to check.
 *  @return `True` for yes, `false` for not.
 */
static inline bool isHalUnpackRawFormat(EImageFormat fmt) {
  return (fmt >= eImgFmt_BAYER8_UNPAK && fmt <= eImgFmt_BAYER16_UNPAK);
}

/**
 * Helper to check if the given format is Mediatek packed YUV 10 bits format.
 *  @param fmt  Format to check.
 *  @return `True` for yes, `false` for not.
 */
static inline bool isHalYuvb10Format(EImageFormat fmt) {
  return (fmt >= eImgFmt_MTK_YUYV_Y210 && fmt < eImgFmt_MTK_YUV_P010_3PLANE);
}

/**
 * Helper to check if the given format is Mediatek UFBC format.
 *  @param fmt  Format to check.
 *  @return `True` for yes, `false` for not.
 */
static inline bool isHalUfbcFormat(EImageFormat fmt) {
  return (fmt >= eImgFmt_UFBC_START && fmt <= eImgFmt_UFBC_END);
}

/**
 * Helper to check if the given format is Mediatek UFBC YUV 2 plane format.
 *  @param fmt  Format to check.
 *  @return `True` for yes, `false` for not.
 */
static inline bool isHalUfbcYuv2PFormat(EImageFormat fmt) {
  return (fmt >= eImgFmt_UFBC_YUV_2P_START && fmt <= eImgFmt_UFBC_YUV_2P_END);
}

/**
 * Helper to check if the given format is Mediatek AFBC format.
 *  @param fmt  Format to check.
 *  @return `True` for yes, `false` for not.
 */
static inline bool isHalAfbcFormat(EImageFormat fmt) {
  return (fmt >= eImgFmt_AFBC_START && fmt <= eImgFmt_AFBC_END);
}

/**
 * Helper to check if the given format is RGB format.
 *  @param fmt  Format to check.
 *  @return `True` for yes, `false` for not.
 */
static inline bool isRgbFormat(EImageFormat fmt) {
  return (fmt >= eImgFmt_RGB_START && fmt < eImgFmt_RAW_START) ||
         (fmt == eImgFmt_RGBA8888) || (fmt == eImgFmt_RGBX8888) ||
         (fmt == eImgFmt_RGB888) || (fmt == eImgFmt_RGB565) ||
         (fmt == eImgFmt_BGRA8888);
}

}      // namespace NSCam
#endif  // INCLUDE_MTKCAM_HALIF_DEF_IMAGEFORMAT_H_

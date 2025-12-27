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

#ifndef INCLUDE_MTKCAM_INTERFACES_DEF_MODULE_H_
#define INCLUDE_MTKCAM_INTERFACES_DEF_MODULE_H_
//
/******************************************************************************
 *
 ******************************************************************************/
#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

/******************************************************************************
 *
 ******************************************************************************/
#define MTKCAM_GET_MODULE_GROUP_ID(module_id) (0xFFFF & (module_id >> 16))
#define MTKCAM_GET_MODULE_INDEX(module_id) (0xFFFF & (module_id))

/**
 * mtkcam module group id
 */
enum {
  MTKCAM_MODULE_GROUP_ID_DRV,
  MTKCAM_MODULE_GROUP_ID_AAA,
  MTKCAM_MODULE_GROUP_ID_CUSTOM,
  MTKCAM_MODULE_GROUP_ID_UTILS,
};

/**
 * mtkcam module id
 *
 * |     32     |      32      |
 * |  group id  | module index |
 */
enum {
  /*****
   * drv
   */
  MTKCAM_MODULE_ID_DRV_START = MTKCAM_MODULE_GROUP_ID_DRV << 16,
  MTKCAM_MODULE_ID_DRV_HAL_SENSORLIST,
  /*!< include/mtkcam-interfaces/hw/sensor/IHalSensor.h */
  MTKCAM_MODULE_ID_DRV_HW_SYNC_DRV,
  /*!< include/mtkcam/drv/IHwSyncDrv.h */
  MTKCAM_MODULE_ID_DRV_IOPIPE_CAMIO_NORMALPIPE,
  /*!< include/mtkcam/drv/iopipe/CamIO/INormalPipe.h */
  //--------------------------------------------------------------------------
  MTKCAM_MODULE_ID_DRV_END,

  /*****
   * aaa
   */
  MTKCAM_MODULE_ID_AAA_START = MTKCAM_MODULE_GROUP_ID_AAA << 16,
  MTKCAM_MODULE_ID_AAA_HAL_3A,      /*!< include/mtkcam/aaa/IHal3A.h */
  MTKCAM_MODULE_ID_AAA_HAL_ISP,     /*!< include/mtkcam/aaa/IHalISP.h */
  MTKCAM_MODULE_ID_AAA_HAL_FLASH,   /*!< include/mtkcam/aaa/IHalFlash.h */
  MTKCAM_MODULE_ID_AAA_ISP_MGR,     /*!< include/mtkcam/aaa/IIspMgr.h */
  MTKCAM_MODULE_ID_AAA_SYNC_3A_MGR, /*!< include/mtkcam/aaa/ISync3A.h */
  MTKCAM_MODULE_ID_AAA_SW_NR,       /*!< include/mtkcam/aaa/ICaptureNR.h */
  MTKCAM_MODULE_ID_AAA_DNG_INFO,    /*!< include/mtkcam/aaa/IDngInfo.h */
  MTKCAM_MODULE_ID_AAA_NVBUF_UTIL,  /*!< include/mtkcam/aaa/INvBufUtil.h */
  MTKCAM_MODULE_ID_AAA_LSC_TABLE,   /*!< include/mtkcam/aaa/ILscTable.h */
  MTKCAM_MODULE_ID_AAA_LCS_HAL,     /*!< include/mtkcam/aaa/lcs/lcs_hal.h */
  //--------------------------------------------------------------------------
  MTKCAM_MODULE_ID_AAA_END,

  /*****
   * custom
   */
  MTKCAM_MODULE_ID_CUSTOM_START = MTKCAM_MODULE_GROUP_ID_CUSTOM << 16,
  MTKCAM_MODULE_ID_CUSTOM_DEBUG_EXIF,
  /*!< include/mtkcam-interfaces/custom/ExifFactory.h */
  //--------------------------------------------------------------------------
  MTKCAM_MODULE_ID_CUSTOM_END,

  /*****
   * Utils
   */
  MTKCAM_MODULE_ID_UTILS_START = MTKCAM_MODULE_GROUP_ID_UTILS << 16,
  MTKCAM_MODULE_ID_UTILS_LOGICALDEV,
  //--------------------------------------------------------------------------
  MTKCAM_MODULE_ID_UTILS_END,
};

/******************************************************************************
 *
 ******************************************************************************/
/**
 * mtkcam module versioning control
 */

/**
 * The most significant bits (bit 24 ~ 31)
 * store the information of major version number.
 *
 * The least significant bits (bit 16 ~ 23)
 * store the information of minor version number.
 *
 * bit 0 ~ 15 are reserved for future use
 */
#define MTKCAM_MAKE_API_VERSION(major, minor) \
  ((((major)&0xff) << 24) | (((minor)&0xff) << 16))

#define MTKCAM_GET_MAJOR_API_VERSION(version) (((version) >> 24) & 0xff)

#define MTKCAM_GET_MINOR_API_VERSION(version) (((version) >> 16) & 0xff)

/**
 * mtkcam module interface
 */
struct mtkcam_module {
  /**
   * The API version of the implemented module. The module owner is
   * responsible for updating the version when a module interface has changed.
   *
   * The derived modules own and manage this field.
   * The module user must interpret the version field to decide whether or not
   * to inter-operate with the supplied module implementation.
   *
   */
  uint32_t (*get_module_api_version)();

  /** Identifier of module. */
  uint32_t (*get_module_id)();

  /**
   * Module extension.
   * Return a pointer to the derived module interface.
   */
  void* (*get_module_extension)();

  /**
   * Return the supported sub-module api versions, sorted in the range
   * [*versions, *versions + count) in ascending order, for a given camera
   * device
   *
   * versions:    The pointer to the start address of an array of supported
   *              sub-module api versions, sorted in ascending order.
   *
   * count:       The number of supported sub-module api versions.
   *
   * index:       The camera/sensor index starting from 0.
   *              Not all modules need this field; ignored if not needed.
   *
   * Return values:
   *
   * 0:           On a successful operation
   *
   * -ENODEV:     The information cannot be provided due to an internal
   *              error.
   *
   * -EINVAL:     The input arguments are invalid, i.e. the index is invalid,
   *              and/or the module is invalid.
   *
   */
  int (*get_sub_module_api_version)(uint32_t const** versions, size_t* count,
                                    int index);

  /* reserved for future use */
  void* reserved[32 - 4];
};

/******************************************************************************
 *
 ******************************************************************************/
__END_DECLS
#endif  // INCLUDE_MTKCAM_INTERFACES_DEF_MODULE_H_

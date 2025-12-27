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

/*
** $Log: cam_cal_drv.h $
*
*
*/

#ifndef INCLUDE_MTKCAM_INTERFACES_HW_MEM_CAM_CAL_DRV_H_
#define INCLUDE_MTKCAM_INTERFACES_HW_MEM_CAM_CAL_DRV_H_

#include "cam_cal_format.h"
#include <inttypes.h>

#define DRV_CAM_CAL_SUPPORT (0)

#ifndef USING_MTK_LDVT
#define CAM_CAL_SUPPORT
#endif

typedef signed int MINT32;

/*****************************************************************************
 * Enums
 *****************************************************************************/
/** @defgroup cam_cal_enum Enum
 *	@{
 */

typedef enum ENUM_CAMERA_CAM_CAL_TYPE_ENUM
    CAMERA_CAM_CAL_TYPE_ENUM;

typedef enum ENUM_CAM_CAL_DATA_VER_ENUM
    CAM_CAL_DATA_VER_ENUM;

/**
 * @}
 */

/*****************************************************************************
 * Structures
 *****************************************************************************/
/** @defgroup cam_cal_hanele_struct Struct
 *  @{
 */

/** @brief This Structure defines the CAM_CAL_Stereo_Data_STRUCT.  */

typedef struct STRUCT_CAM_CAL_Stereo_Data_STRUCT
    CAM_CAL_Stereo_Data_STRUCT, *PCAM_CAL_Stereo_Data_STRUCT;

/** @brief This Structure defines the LSC Table.  */

typedef struct STRUCT_CAM_CAL_LSC_MTK_TYPE
    CAM_CAL_LSC_MTK_TYPE;

typedef struct STRUCT_CAM_CAL_LSC_SENSOR_TYPE
    CAM_CAL_LSC_SENSOR_TYPE;

typedef union UNION_CAM_CAL_LSC_DATA
    CAM_CAL_LSC_DATA;

typedef struct STRUCT_CAM_CAL_SINGLE_LSC_STRUCT
    CAM_CAL_SINGLE_LSC_STRUCT, *PCAM_CAL_SINGLE_LSC_STRUCT;

/** @brief This Structure defines the 2A Table.  */

typedef struct STRUCT_CAM_CAL_PREGAIN_STRUCT
    CAM_CAL_PREGAIN_STRUCT, *PCAM_CAL_PREGAIN_STRUCT;

typedef struct STRUCT_CAM_CAL_AF_STRUCT
    CAM_CAL_AF_STRUCT, *PCAM_CAL_AF_STRUCT;

typedef struct STRUCT_CAM_CAL_SINGLE_2A_STRUCT
    CAM_CAL_SINGLE_2A_STRUCT, *PCAM_CAL_SINGLE_2A_STRUCT;

/** @brief This structure defines the PDAF Table.  */

typedef struct STRUCT_CAM_CAL_PDAF_STRUCT
    CAM_CAL_PDAF_STRUCT, *PCAM_CAL_PDAF_STRUCT;

/** @brief This enum defines the CAM_CAL Table.  */

typedef struct STRUCT_CAM_CAL_DATA_STRUCT
    CAM_CAL_DATA_STRUCT, *PCAM_CAL_DATA_STRUCT;

/**
 * Type define for data struct unbundle
 */
typedef struct STRUCT_CAM_CAL_MODULE_VERSION_STRUCT
    CAM_CAL_MODULE_VERSION_STRUCT, *PCAM_CAL_MODULE_VERSION_STRUCT;

typedef struct STRUCT_CAM_CAL_PART_NUM_STRUCT
    CAM_CAL_PART_NUM_STRUCT, *PCAM_CAL_PART_NUM_STRUCT;

typedef struct STRUCT_CAM_CAL_LSC_DATA_STRUCT
    CAM_CAL_LSC_DATA_STRUCT, *PCAM_CAL_LSC_DATA_STRUCT;

typedef struct STRUCT_CAM_CAL_2A_DATA_STRUCT
    CAM_CAL_2A_DATA_STRUCT, *PCAM_CAL_2A_DATA_STRUCT;

typedef struct STRUCT_CAM_CAL_PDAF_DATA_STRUCT
    CAM_CAL_PDAF_DATA_STRUCT, *PCAM_CAL_PDAF_DATA_STRUCT;

typedef struct {
  unsigned int size_of_cmd; // size of has_cmd
  unsigned char has_cmd[CAMERA_CAM_CAL_DATA_LIST]; // store the data existence of all command
  unsigned int size_of_data; // size of cali_data
  CAM_CAL_DATA_STRUCT cali_data;  // All calibration data
} CAM_CAL_VIRTUAL_DUMP_STRUCT, *PCAM_CAL_VIRTUAL_DUMP_STRUCT;

typedef struct STRUCT_CAM_CAL_STEREO_DATA_STRUCT
    CAM_CAL_STEREO_DATA_STRUCT, *PCAM_CAL_STEREO_DATA_STRUCT;

typedef struct STRUCT_CAM_CAL_LENS_ID_STRUCT
    CAM_CAL_LENS_ID_STRUCT, *PCAM_CAL_LENS_ID_STRUCT;

/*******************************************************************************
 *
 ********************************************************************************/

class CamCalDrvBase {
 public:
  /////////////////////////////////////////////////////////////////////////
  //
  // createInstance () -
  //! \brief create instance
  //
  /////////////////////////////////////////////////////////////////////////
  static CamCalDrvBase* createInstance();

  /////////////////////////////////////////////////////////////////////////
  //
  // destroyInstance () -
  //! \brief destroy instance
  //
  /////////////////////////////////////////////////////////////////////////
  virtual void destroyInstance() = 0;

  /////////////////////////////////////////////////////////////////////////
  //
  // readCamCal () -
  //! \brief
  //
  /////////////////////////////////////////////////////////////////////////
  virtual int GetCamCalCalData(uint32_t i4SensorDevId,
                               /*unsigned long u4SensorID,*/
                               CAMERA_CAM_CAL_TYPE_ENUM a_eCamCalDataType,
                               void* a_pCamCalData) = 0;

  /////////////////////////////////////////////////////////////////////////
  //
  // GetCamCalCalDataV2() -
  //! \brief Get cal data v2. Support cal data struct unbundle
  //! \param i4SensorDevId Sensor device id
  //! \param a_eCamCalDataType Cal data type
  //! \param a_pCamCalData Return the result of cal data
  //! \param calDataSize The data size of a_pCamCalData
  //! \return error code, {@code CAM_CAL_ERR_NO_ERR}
  //                                              means success without error
  //
  /////////////////////////////////////////////////////////////////////////
  virtual int GetCamCalCalDataV2(uint32_t i4SensorDevId,
                                 CAMERA_CAM_CAL_TYPE_ENUM a_eCamCalDataType,
                                 void* a_pCamCalData,
                                 unsigned int calDataSize) = 0;

 protected:
  /////////////////////////////////////////////////////////////////////////
  //
  // ~CamCalDrvBase () -
  //! \brief descontrustor
  //
  /////////////////////////////////////////////////////////////////////////
  virtual ~CamCalDrvBase() {}

 private:
};

#endif  // INCLUDE_MTKCAM_INTERFACES_HW_MEM_CAM_CAL_DRV_H_

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

#ifndef _ISP_TUNING_CAM_INFO_H_
#define _ISP_TUNING_CAM_INFO_H_

#include <tuning_mapping/cam_idx_struct_ext_pub.h>
#include <camera_custom_isp_nvram_pub.h>



#include <vector>
#include <string>
#include <mutex>  // std::mutex
#include <tuning_mapping/cam_idx_struct_query_pub.h> // mapping_info

namespace NSIspTuning
{
/*******************************************************************************
*
*******************************************************************************/
struct interpolation_sys_info {
   uint32_t idx_iso_u[NVRAM_ISP_REGS_ISO_GROUP_NUM];
   uint32_t idx_iso_l[NVRAM_ISP_REGS_ISO_GROUP_NUM];
   uint32_t idx_ct_u;
   uint32_t idx_ct_l;
   uint32_t idx_lv_u;
   uint32_t idx_lv_l;
   uint32_t idx_zoom_u;
   uint32_t idx_zoom_l;
   uint32_t idx_ratio_u;
   uint32_t idx_ratio_l;
};

typedef union _CAM_IDX_QRY_COMB_
{
    struct {
        EIspProfile_T eIspProfile;
        ESensorMode_T eSensorMode;
        EFrontBin_T eFrontBin;
        ESize_T eSize;
        EFlash_T eFlash;
        EApp_T eApp;
        EFaceDetection_T eFaceDetection;
        ECustom_00_T eCustom_00;
        ECustom_00_T eCustom_01;
        EZoom_T eZoom_Idx;
        EIspLV_T eIspLV_Idx;
        ELV_T eLV_Idx;
        ECT_T eCT_Idx;
        EISO_T eISO_Idx[NVRAM_ISP_REGS_ISO_GROUP_NUM];
    };
    MUINT32 query[EDim_dummy_NUM+NVRAM_ISP_REGS_ISO_GROUP_NUM-1];
    _CAM_IDX_QRY_COMB_() { memset(query, 0, sizeof(query));}
} CAM_IDX_QRY_COMB;

typedef struct _CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO_
{
    CAM_IDX_QRY_COMB_ISP7 mapping_info;
    int i4Iso;
    int i4RealLightValue_x10;
    int cct;
    int i4ZoomRatio_x100;
    interpolation_sys_info int_info;

    _CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO_()
        : i4Iso(100)
        , i4RealLightValue_x10(0)
        , cct(0)
        , i4ZoomRatio_x100(100)
        {}
} CAM_IDX_QRY_COMB_WITH_SYSTEM_INFO;

struct AllModuleQueryResult{
  int32_t idxBase[EModuleDB_NUM];
  int32_t isoGroup[EModuleDB_NUM];
  std::string scenario_name;
};

typedef enum {
  IDXCACHE_VALTYPE_CURRENT,
  IDXCACHE_VALTYPE_LOWERISO,
  IDXCACHE_VALTYPE_UPPERISO,
  IDXCACHE_VALTYPE_LOWERLV,
  IDXCACHE_VALTYPE_UPPERLV,
  IDXCACHE_VALTYPE_LOWERLV_LOWERCT,
  IDXCACHE_VALTYPE_LOWERLV_UPPERCT,
  IDXCACHE_VALTYPE_UPPERLV_LOWERCT,
  IDXCACHE_VALTYPE_UPPERLV_UPPERCT,
  IDXCACHE_VALTYPE_LOWERISO_LOWERZOOM,
  IDXCACHE_VALTYPE_LOWERISO_UPPERZOOM,
  IDXCACHE_VALTYPE_UPPERISO_LOWERZOOM,
  IDXCACHE_VALTYPE_UPPERISO_UPPERZOOM,
  IDXCACHE_VALTYPE_LOWERISO_LOWERRATIO,
  IDXCACHE_VALTYPE_LOWERISO_UPPERRATIO,
  IDXCACHE_VALTYPE_UPPERISO_LOWERRATIO,
  IDXCACHE_VALTYPE_UPPERISO_UPPERRATIO,
  IDXCACHE_VALTYPE_NUM
} IDXCACHE_VALTYPE;

/*******************************************************************************
*
*******************************************************************************/
}  //  NSIspTuning
#endif //  _ISP_TUNING_CAM_INFO_H_


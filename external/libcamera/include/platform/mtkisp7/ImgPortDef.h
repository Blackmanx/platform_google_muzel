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

#ifndef INCLUDE_MTKCAM_CORE_HW_IMGSTREAM_IMGPORTDEF_H_
#define INCLUDE_MTKCAM_CORE_HW_IMGSTREAM_IMGPORTDEF_H_

#define MTK_ISP_NODE_ID_BASE 0

#define MTK_ISP_CAMSYS_NODE_ID_BASE (MTK_ISP_NODE_ID_BASE + 0x00000000)
#define MTK_ISP_IMGSYS_NODE_ID_BASE (MTK_ISP_NODE_ID_BASE + 0x00020000)

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSImgStream {

enum IMG_PORT {
  IMG_PORT_IMGI = (MTK_ISP_IMGSYS_NODE_ID_BASE + 0L),
  IMG_PORT_VIPI,
  IMG_PORT_CAMSTATI,
  IMG_PORT_IMGSTATO,
  IMG_PORT_METAI,
  IMG_PORT_REGO,  // 5
  IMG_PORT_REC_DSI,
  IMG_PORT_REC_DPI,
  IMG_PORT_TNRSI,
  IMG_PORT_TNRWI,
  IMG_PORT_TNRMI,  //  10
  IMG_PORT_TNRCI,
  IMG_PORT_TNRLFDI,
  IMG_PORT_TNRVBI,
  IMG_PORT_TNRSO,
  IMG_PORT_TNRWO,  //  15
  IMG_PORT_TNRMO,
  IMG_PORT_IMG2O,
  IMG_PORT_IMG3O,
  IMG_PORT_IMG4O,
  IMG_PORT_FEO,  //  20
  IMG_PORT_TIMGO,
  IMG_PORT_CNR_BLURMAPI,
  IMG_PORT_LFEOI,
  IMG_PORT_RFEOI,
  IMG_PORT_FMO,    //  25
  IMG_PORT_TIMGI,  //  26
  IMG_PORT_TYUVO,
  IMG_PORT_TYUV2O,
  IMG_PORT_TYUV3O,
  IMG_PORT_TYUV4O,  // 30
  IMG_PORT_TYUV5O,  // 31
  IMG_PORT_LTIMGI,  // 32
  IMG_PORT_LTYUV2O,
  IMG_PORT_LTYUV3O,
  IMG_PORT_LTYUV4O,  // 35
  IMG_PORT_LTYUV5O,  // 36
  IMG_PORT_XTIMGI,
  IMG_PORT_XYUVO,
  IMG_PORT_XTYUV2O,
  IMG_PORT_XTYUV3O,  // 40
  IMG_PORT_XTYUV4O,
  IMG_PORT_XTYUV5O,
  IMG_PORT_XTIMGO,
  IMG_PORT_XTMEO,
  IMG_PORT_XTFDO,  // 45
  IMG_PORT_XTADLDBGO,
  IMG_PORT_WPE_WPEI = (MTK_ISP_IMGSYS_NODE_ID_BASE + 2000L),
  IMG_PORT_WPE_VECI,
  IMG_PORT_WPE_WPEO,
  IMG_PORT_WPE_MSKO,
  IMG_PORT_WPE_PSP_COEFFI,
  IMG_PORT_WPE_TNR_WPEI,
  IMG_PORT_WPE_TNR_VECI,
  IMG_PORT_WPE_TNR_WPEO,
  IMG_PORT_WPE_TNR_MSKO,
  IMG_PORT_WPE_TNR_PSP_COEFFI,
  IMG_PORT_PIMGI = (MTK_ISP_IMGSYS_NODE_ID_BASE + 3000L),
  IMG_PORT_WROTO,
  IMG_PORT_WDMAO,
  IMG_PORT_A_TCCSO,
  IMG_PORT_B_TCCSO,
  IMG_PORT_ME_L0_IMG0I = (MTK_ISP_IMGSYS_NODE_ID_BASE + 4000L),
  IMG_PORT_ME_L0_IMG1I,
  IMG_PORT_ME_L1_IMG0I,
  IMG_PORT_ME_L1_IMG1I,
  IMG_PORT_ME_IMGSTATI,
  IMG_PORT_ME_MEMILI,
  IMG_PORT_ME_MMG_MILO,
  IMG_PORT_ME_L0_RMVI,
  IMG_PORT_ME_L1_RMVI,
  IMG_PORT_ME_L0_WMVO,
  IMG_PORT_ME_L1_WMVO,
  IMG_PORT_ME_CONFO,
  IMG_PORT_ME_WMAPO,
  IMG_PORT_ME_FMVO,
  IMG_PORT_ME_L0_FMBI,
  IMG_PORT_ME_L1_FMBI,
  IMG_PORT_ME_L0_FMBO,
  IMG_PORT_ME_L1_FMBO,
  IMG_PORT_ME_FSTO,
  IMG_PORT_ME_LMIO,
  // Reserved for Driver Used, uer can't ue the below Port Index
  IMG_PORT_DRV_CTRLMETAI = (MTK_ISP_IMGSYS_NODE_ID_BASE + 100000L),
  IMG_PORT_DRV_SINGLEDEVICEI,
  IMG_PORT_DRV_SIGDEV_NORMI,
  IMG_PORT_DRV_PDCBUFI,
  IMG_PORT_UNKNOW
};

/******************************************************************************
 *
 ******************************************************************************/
}      // namespace NSImgStream
}      // namespace NSCam

#endif  // INCLUDE_MTKCAM_CORE_HW_IMGSTREAM_IMGPORTDEF_H_

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

#pragma once

#include "ImgPortDef.h"

#define MEDIA_DEVICE_PASS2_NAME "MTK-ISP-DIP-V4L2"
#define VIDEO_DEVICE_HUB_NAME "MTK-ISP-DIP-V4L2"

#define VIDEO_DEVICE_TIMGI_NAME "MTK-ISP-DIP-V4L2 TIMGI Input"
#define VIDEO_DEVICE_PDC_NAME "MTK-ISP-DIP-V4L2 PDC Input"
#define VIDEO_DEVICE_TYUVO_NAME "MTK-ISP-DIP-V4L2 TYUVO Output"
#define VIDEO_DEVICE_TYUV2O_NAME "MTK-ISP-DIP-V4L2 TYUV2O Output"
#define VIDEO_DEVICE_TYUV3O_NAME "MTK-ISP-DIP-V4L2 TYUV3O Output"
#define VIDEO_DEVICE_TYUV4O_NAME "MTK-ISP-DIP-V4L2 TYUV4O Output"
#define VIDEO_DEVICE_TYUV5O_NAME "MTK-ISP-DIP-V4L2 TYUV5O Output"
#define VIDEO_DEVICE_TIMGO_NAME "MTK-ISP-DIP-V4L2 TIMGO Output"
#define VIDEO_DEVICE_IMGSTATO_NAME "MTK-ISP-DIP-V4L2 IMGSTATO Outpu"

#define VIDEO_DEVICE_IMGI_NAME "MTK-ISP-DIP-V4L2 Imgi Input"
#define VIDEO_DEVICE_VIPI_NAME "MTK-ISP-DIP-V4L2 Vipi Input"
#define VIDEO_DEVICE_REC_DSI_NAME "MTK-ISP-DIP-V4L2 Rec_Dsi Input"
#define VIDEO_DEVICE_REC_DPI_NAME "MTK-ISP-DIP-V4L2 Rec_Dpi Input"
#define VIDEO_DEVICE_BOKEHI_NAME "MTK-ISP-DIP-V4L2 Bokeh Input"
#define VIDEO_DEVICE_DMGI_FM_NAME "MTK-ISP-DIP-V4L2 Dmgi_FM Input"
#define VIDEO_DEVICE_DEPI_FM_NAME "MTK-ISP-DIP-V4L2 Depi_FM Input"
#define VIDEO_DEVICE_TNRSI_NAME "MTK-ISP-DIP-V4L2 Tnrsi Input"
#define VIDEO_DEVICE_TNRWI_NAME "MTK-ISP-DIP-V4L2 Tnrwi Input"
#define VIDEO_DEVICE_TNRMI_NAME "MTK-ISP-DIP-V4L2 Tnrmi Input"
#define VIDEO_DEVICE_TNRCI_NAME "MTK-ISP-DIP-V4L2 Tnrci Input"
#define VIDEO_DEVICE_TNRLI_NAME "MTK-ISP-DIP-V4L2 Tnrli Input"
#define VIDEO_DEVICE_TNRVBI_NAME "MTK-ISP-DIP-V4L2 Tnrvbi Input"
#define VIDEO_DEVICE_IMG2O_NAME "MTK-ISP-DIP-V4L2 Img2o Output"
#define VIDEO_DEVICE_IMG3O_NAME "MTK-ISP-DIP-V4L2 Img3o Output"
#define VIDEO_DEVICE_IMG4O_NAME "MTK-ISP-DIP-V4L2 Img4o Output"
#define VIDEO_DEVICE_FMO_NAME "MTK-ISP-DIP-V4L2 FM Output"
#define VIDEO_DEVICE_TNRSO_NAME "MTK-ISP-DIP-V4L2 Tnrso Output"
#define VIDEO_DEVICE_TNRWO_NAME "MTK-ISP-DIP-V4L2 Tnrwo Output"
#define VIDEO_DEVICE_TNRMO_NAME "MTK-ISP-DIP-V4L2 Tnrmo Output"

#define VIDEO_DEVICE_PIMGI_NAME "MTK-ISP-DIP-V4L2 PIMGI Input"
#define VIDEO_DEVICE_WDMAO_NAME "MTK-ISP-DIP-V4L2 WROTO A Output"
#define VIDEO_DEVICE_WROTO_NAME "MTK-ISP-DIP-V4L2 WROTO B Output"

#define VIDEO_DEVICE_WPEI_NAME "MTK-ISP-DIP-V4L2 WPEI_E Input"
#define VIDEO_DEVICE_VECI_NAME "MTK-ISP-DIP-V4L2 VECI_E Input"
#define VIDEO_DEVICE_PSPI_NAME "MTK-ISP-DIP-V4L2 PSPI_E Input"
#define VIDEO_DEVICE_WPEO_NAME "MTK-ISP-DIP-V4L2 WPEO_E Output"
#define VIDEO_DEVICE_MSKO_NAME "MTK-ISP-DIP-V4L2 MSKO_E Output"

#define VIDEO_DEVICE_WPEI_T_NAME "MTK-ISP-DIP-V4L2 WPEI_T Input"
#define VIDEO_DEVICE_VECI_T_NAME "MTK-ISP-DIP-V4L2 VECI_T Input"
#define VIDEO_DEVICE_PSPI_T_NAME "MTK-ISP-DIP-V4L2 PSPI_T Input"
#define VIDEO_DEVICE_WPEO_T_NAME "MTK-ISP-DIP-V4L2 WPEO_T Output"
#define VIDEO_DEVICE_MSKO_T_NAME "MTK-ISP-DIP-V4L2 MSKO_T Output"

#define VIDEO_DEVICE_L0IMG0I_T_NAME "MTK-ISP-DIP-V4L2 L0I0I Input"
#define VIDEO_DEVICE_L0IMG1I_T_NAME "MTK-ISP-DIP-V4L2 L0I1I Input"
#define VIDEO_DEVICE_L1IMG0I_T_NAME "MTK-ISP-DIP-V4L2 L1I0I Input"
#define VIDEO_DEVICE_L1IMG1I_T_NAME "MTK-ISP-DIP-V4L2 L1I1I Input"
#define VIDEO_DEVICE_IMGSTATI_T_NAME "MTK-ISP-DIP-V4L2 STATI Input"
#define VIDEO_DEVICE_L0FMBI_T_NAME "MTK-ISP-DIP-V4L2 L0FMBI Input"
#define VIDEO_DEVICE_L1FMBI_T_NAME "MTK-ISP-DIP-V4L2 L1FMBI Input"
#define VIDEO_DEVICE_MEMILI_T_NAME "MTK-ISP-DIP-V4L2 MEMILI Input"
#define VIDEO_DEVICE_MMGMILO_T_NAME "MTK-ISP-DIP-V4L2 MILO Output"
#define VIDEO_DEVICE_L0RMVI_T_NAME "MTK-ISP-DIP-V4L2 L0RMVI Input"
#define VIDEO_DEVICE_L1RMVI_T_NAME "MTK-ISP-DIP-V4L2 L1RMVI Input"
#define VIDEO_DEVICE_L0WMVO_T_NAME "MTK-ISP-DIP-V4L2 L0WMVO Output"
#define VIDEO_DEVICE_L1WMVO_T_NAME "MTK-ISP-DIP-V4L2 L1WMVO Output"
#define VIDEO_DEVICE_CONFO_T_NAME "MTK-ISP-DIP-V4L2 CONFO Output"
#define VIDEO_DEVICE_WMAPO_T_NAME "MTK-ISP-DIP-V4L2 WMAPO Output"
#define VIDEO_DEVICE_FMVO_T_NAME "MTK-ISP-DIP-V4L2 FMVO Output"
#define VIDEO_DEVICE_L0FMBO_T_NAME "MTK-ISP-DIP-V4L2 L0FMBO Output"
#define VIDEO_DEVICE_L1FMBO_T_NAME "MTK-ISP-DIP-V4L2 L1FMBO Output"
#define VIDEO_DEVICE_FSTO_T_NAME "MTK-ISP-DIP-V4L2 FSTO Output"
#define VIDEO_DEVICE_LMIO_T_NAME "MTK-ISP-DIP-V4L2 LMIO Output"

#define VIDEO_DEVICE_TUNING_NAME "MTK-ISP-DIP-V4L2 Tuning"
#define VIDEO_DEVICE_CTRLMETA_NAME "MTK-ISP-DIP-V4L2 CtrlMeta"
#define VIDEO_DEVICE_SIGDEV_NAME "MTK-ISP-DIP-V4L2 Single Device"
#define VIDEO_DEVICE_SIGDEV_NORM_NAME "MTK-ISP-DIP-V4L2 SIGDEVN"

typedef struct PortDesc {
  const char* port_name;
  int port_index;
  const char* device_name;
  bool input_type;
  bool single_device;
  bool ctrl_meta;
} PortDesc;

typedef struct LinkDesc {
  const char* source_name;
  int source_index;
  const char* sink_name;
  int sink_index;
} LinkDesc;

static const PortDesc ports[] = {
    {"timgi", NSCam::NSImgStream::IMG_PORT_TIMGI, VIDEO_DEVICE_TIMGI_NAME, true,
     false, false},
    {"pdc", NSCam::NSImgStream::IMG_PORT_DRV_PDCBUFI, VIDEO_DEVICE_PDC_NAME,
     true, false, false},
    {"tyuvo", NSCam::NSImgStream::IMG_PORT_TYUVO, VIDEO_DEVICE_TYUVO_NAME,
     false, false, false},
    {"tyuv2o", NSCam::NSImgStream::IMG_PORT_TYUV2O, VIDEO_DEVICE_TYUV2O_NAME,
     false, false, false},
    {"tyuv3o", NSCam::NSImgStream::IMG_PORT_TYUV3O, VIDEO_DEVICE_TYUV3O_NAME,
     false, false, false},
    {"tyuv4o", NSCam::NSImgStream::IMG_PORT_TYUV4O, VIDEO_DEVICE_TYUV4O_NAME,
     false, false, false},
    {"tyuv5o", NSCam::NSImgStream::IMG_PORT_TYUV5O, VIDEO_DEVICE_TYUV5O_NAME,
     false, false, false},
    {"timgo", NSCam::NSImgStream::IMG_PORT_TIMGO, VIDEO_DEVICE_TIMGO_NAME,
     false, false, false},
    {"imgstato", NSCam::NSImgStream::IMG_PORT_IMGSTATO,
     VIDEO_DEVICE_IMGSTATO_NAME, false, false, false},
    // DIP
    {"imgi", NSCam::NSImgStream::IMG_PORT_IMGI, VIDEO_DEVICE_IMGI_NAME, true,
     false, false},
    {"vipi", NSCam::NSImgStream::IMG_PORT_VIPI, VIDEO_DEVICE_VIPI_NAME, true,
     false, false},
    {"rec dsi", NSCam::NSImgStream::IMG_PORT_REC_DSI, VIDEO_DEVICE_REC_DSI_NAME,
     true, false, false},
    {"rec dpi", NSCam::NSImgStream::IMG_PORT_REC_DPI, VIDEO_DEVICE_REC_DPI_NAME,
     true, false, false},
    {"cnr blurmapi", NSCam::NSImgStream::IMG_PORT_CNR_BLURMAPI,
     VIDEO_DEVICE_BOKEHI_NAME, true, false, false},
    {"lfeoi", NSCam::NSImgStream::IMG_PORT_LFEOI, VIDEO_DEVICE_DMGI_FM_NAME,
     true, false, false},
    {"rfeoi", NSCam::NSImgStream::IMG_PORT_RFEOI, VIDEO_DEVICE_DEPI_FM_NAME,
     true, false, false},
    {"tnrsi", NSCam::NSImgStream::IMG_PORT_TNRSI, VIDEO_DEVICE_TNRSI_NAME, true,
     false, false},
    {"tnrwi", NSCam::NSImgStream::IMG_PORT_TNRWI, VIDEO_DEVICE_TNRWI_NAME, true,
     false, false},
    {"tnrmi", NSCam::NSImgStream::IMG_PORT_TNRMI, VIDEO_DEVICE_TNRMI_NAME, true,
     false, false},
    {"tnrci", NSCam::NSImgStream::IMG_PORT_TNRCI, VIDEO_DEVICE_TNRCI_NAME, true,
     false, false},
    {"tnrli", NSCam::NSImgStream::IMG_PORT_TNRLFDI, VIDEO_DEVICE_TNRLI_NAME,
     true, false, false},
    {"tnrvbi", NSCam::NSImgStream::IMG_PORT_TNRVBI, VIDEO_DEVICE_TNRVBI_NAME,
     true, false, false},
    {"img2o", NSCam::NSImgStream::IMG_PORT_IMG2O, VIDEO_DEVICE_IMG2O_NAME,
     false, false, false},
    {"img3o", NSCam::NSImgStream::IMG_PORT_IMG3O, VIDEO_DEVICE_IMG3O_NAME,
     false, false, false},
    {"img4o", NSCam::NSImgStream::IMG_PORT_IMG4O, VIDEO_DEVICE_IMG4O_NAME,
     false, false, false},
    {"fmo", NSCam::NSImgStream::IMG_PORT_FMO, VIDEO_DEVICE_FMO_NAME, false,
     false, false},
    {"tnrso", NSCam::NSImgStream::IMG_PORT_TNRSO, VIDEO_DEVICE_TNRSO_NAME,
     false, false, false},
    {"tnrwo", NSCam::NSImgStream::IMG_PORT_TNRWO, VIDEO_DEVICE_TNRWO_NAME,
     false, false, false},
    {"tnrmo", NSCam::NSImgStream::IMG_PORT_TNRMO, VIDEO_DEVICE_TNRMO_NAME,
     false, false, false},
    // PQ-DIP
    {"pimgi", NSCam::NSImgStream::IMG_PORT_PIMGI, VIDEO_DEVICE_PIMGI_NAME, true,
     false, false},
    {"wdma", NSCam::NSImgStream::IMG_PORT_WDMAO, VIDEO_DEVICE_WDMAO_NAME, false,
     false, false},
    {"wrot", NSCam::NSImgStream::IMG_PORT_WROTO, VIDEO_DEVICE_WROTO_NAME, false,
     false, false},
    // ME
    {"l0img0i", NSCam::NSImgStream::IMG_PORT_ME_L0_IMG0I,
     VIDEO_DEVICE_L0IMG0I_T_NAME, true, false, false},
    {"l0img1i", NSCam::NSImgStream::IMG_PORT_ME_L0_IMG1I,
     VIDEO_DEVICE_L0IMG1I_T_NAME, true, false, false},
    {"l1img0i", NSCam::NSImgStream::IMG_PORT_ME_L1_IMG0I,
     VIDEO_DEVICE_L1IMG0I_T_NAME, true, false, false},
    {"l1img1i", NSCam::NSImgStream::IMG_PORT_ME_L1_IMG1I,
     VIDEO_DEVICE_L1IMG1I_T_NAME, false, false, false},
    {"imgstati", NSCam::NSImgStream::IMG_PORT_ME_IMGSTATI,
     VIDEO_DEVICE_IMGSTATI_T_NAME, false, false, false},
    {"mmgmilo", NSCam::NSImgStream::IMG_PORT_ME_MMG_MILO,
     VIDEO_DEVICE_MMGMILO_T_NAME, true, false, false},
    {"l0rmvi", NSCam::NSImgStream::IMG_PORT_ME_L0_RMVI,
     VIDEO_DEVICE_L0RMVI_T_NAME, true, false, false},
    {"l1rmvi", NSCam::NSImgStream::IMG_PORT_ME_L1_RMVI,
     VIDEO_DEVICE_L1RMVI_T_NAME, true, false, false},
    {"l0fmbi", NSCam::NSImgStream::IMG_PORT_ME_L0_FMBI,
     VIDEO_DEVICE_L0FMBI_T_NAME, true, false, false},
    {"l1fmbi", NSCam::NSImgStream::IMG_PORT_ME_L1_FMBI,
     VIDEO_DEVICE_L1FMBI_T_NAME, true, false, false},
    {"memili", NSCam::NSImgStream::IMG_PORT_ME_MEMILI,
     VIDEO_DEVICE_MEMILI_T_NAME, true, false, false},
    {"l0wmvo", NSCam::NSImgStream::IMG_PORT_ME_L0_WMVO,
     VIDEO_DEVICE_L0WMVO_T_NAME, false, false, false},
    {"l1wmvo", NSCam::NSImgStream::IMG_PORT_ME_L1_WMVO,
     VIDEO_DEVICE_L1WMVO_T_NAME, false, false, false},
    {"confo", NSCam::NSImgStream::IMG_PORT_ME_CONFO,
     VIDEO_DEVICE_CONFO_T_NAME, false, false, false},
    {"wmapo", NSCam::NSImgStream::IMG_PORT_ME_WMAPO,
     VIDEO_DEVICE_WMAPO_T_NAME, false, false, false},
    {"fmvo", NSCam::NSImgStream::IMG_PORT_ME_FMVO,
     VIDEO_DEVICE_FMVO_T_NAME, false, false, false},
    {"fsto", NSCam::NSImgStream::IMG_PORT_ME_FSTO,
     VIDEO_DEVICE_FSTO_T_NAME, false, false, false},
    {"lmio", NSCam::NSImgStream::IMG_PORT_ME_LMIO,
     VIDEO_DEVICE_LMIO_T_NAME, false, false, false},
    {"l0fmbo", NSCam::NSImgStream::IMG_PORT_ME_L0_FMBO,
     VIDEO_DEVICE_L0FMBO_T_NAME, false, false, false},
    {"l1fmbo", NSCam::NSImgStream::IMG_PORT_ME_L1_FMBO,
     VIDEO_DEVICE_L1FMBO_T_NAME, false, false, false},

    // WPE-EIS
    {"wpei", NSCam::NSImgStream::IMG_PORT_WPE_WPEI, VIDEO_DEVICE_WPEI_NAME,
     true, false, false},
    {"veci", NSCam::NSImgStream::IMG_PORT_WPE_VECI, VIDEO_DEVICE_VECI_NAME,
     true, false, false},
    {"veci", NSCam::NSImgStream::IMG_PORT_WPE_PSP_COEFFI,
     VIDEO_DEVICE_PSPI_NAME, true, false, false},
    {"wpeo", NSCam::NSImgStream::IMG_PORT_WPE_WPEO, VIDEO_DEVICE_WPEO_NAME,
     false, false, false},
    {"msko", NSCam::NSImgStream::IMG_PORT_WPE_MSKO, VIDEO_DEVICE_MSKO_NAME,
     false, false, false},
    // WPE-TNR
    {"wpei_t", NSCam::NSImgStream::IMG_PORT_WPE_TNR_WPEI,
     VIDEO_DEVICE_WPEI_T_NAME, true, false, false},
    {"veci_t", NSCam::NSImgStream::IMG_PORT_WPE_TNR_VECI,
     VIDEO_DEVICE_VECI_T_NAME, true, false, false},
    {"veci_t", NSCam::NSImgStream::IMG_PORT_WPE_TNR_PSP_COEFFI,
     VIDEO_DEVICE_PSPI_T_NAME, true, false, false},
    {"wpeo_t", NSCam::NSImgStream::IMG_PORT_WPE_TNR_WPEO,
     VIDEO_DEVICE_WPEO_T_NAME, false, false, false},
    {"msko_t", NSCam::NSImgStream::IMG_PORT_WPE_TNR_MSKO,
     VIDEO_DEVICE_MSKO_T_NAME, false, false, false},
    // Common
    {"tuningmeta", NSCam::NSImgStream::IMG_PORT_METAI, VIDEO_DEVICE_TUNING_NAME,
     true, false, false},
    {"ctrlmeta", NSCam::NSImgStream::IMG_PORT_DRV_CTRLMETAI,
     VIDEO_DEVICE_CTRLMETA_NAME, true, false, true},
    {"sigdev", NSCam::NSImgStream::IMG_PORT_DRV_SINGLEDEVICEI,
     VIDEO_DEVICE_SIGDEV_NAME, true, true, false},
    {"sigdevnorm", NSCam::NSImgStream::IMG_PORT_DRV_SIGDEV_NORMI,
     VIDEO_DEVICE_SIGDEV_NORM_NAME, true, true, false},
};

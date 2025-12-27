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

#include <cstdint>
#include <map>
#include <sstream>
#include <stdint.h>
#include <string>

#define IMGSYS_VER_ISP71

#include "kernel-headers/mtk_cam-meta-mt8188.h"
#include "kernel-headers/mtk_header_desc.h"
#include "kernel-headers/mtk_imgsys-vnode_id.h"
#include "kernel-headers/mtk_imgsys.h"
#include "linux/mtkisp7/camsys/camsys_videodev2.h"
#include "linux/mtkisp7/drv/7.1/ctrl_meta.h"
#include "mtkcam-halif/def/ImageFormat.h"
#include "mtkcam-halif/def/UITypes.h"
#include "platform/mtkisp7/IImgStreamDef.h"
#include "platform/mtkisp7/ImgPortDef.h"
#include "platform/mtkisp7/Tuning_Helper.h"
#include "platform/mtkisp7/eightcc.h"
#include "platform/mtkisp7/single_device_helper.h"

#define LOG_ERR(fmt, ...) printf((fmt "\n"), ##__VA_ARGS__)
#define LOG_WRN(fmt, ...)
#define LOG_ADBDBG(...)
#define LOG_INF(...)
#define LOG_VRB(...)
#define LOG_DBG(...)

#define AEE_ASSERT(Module, String)

using utype = uint32_t;

namespace NSCam {
namespace NSImgStream {

static int m_DeviceTuningEn = 0;
static TuningHelper mTuningHelper;

enum IMG_OUTPUT_SEL_ENUM {
	IMG_OUTPUT_SEL_NONE,
	IMG_OUTPUT_SEL_TIMGO_AFTER_DGN,
	IMG_OUTPUT_SEL_TIMGO_AFTER_LSC,
	IMG_OUTPUT_SEL_TIMGO_AFTER_HLR,
	IMG_OUTPUT_SEL_TIMGO_AFTER_LTM,
	IMG_OUTPUT_SEL_TIMGO_AFTER_CRNR,
	IMG_OUTPUT_SEL_TIMGO_AFTER_CCM,
	IMG_OUTPUT_SEL_TIMGO_AFTER_GGM,
	IMG_OUTPUT_SEL_MAX,
};

enum IMG_INPUT_SEL_ENUM {
	IMG_INPUT_SEL_NONE,
	IMG_INPUT_SEL_PRC_RAW_I_1,
	IMG_INPUT_SEL_PRC_RAW_I_2,
	IMG_INPUT_SEL_MAX,
};

IImageBuffer::IImageBuffer(const BufferProperty &property)
	: property(property)
{
}

MINT IImageBuffer::getImgFormat() const
{
	return property.format;
}

MSize const IImageBuffer::getImgSize() const
{
	return MSize(property.width, property.height);
}

size_t IImageBuffer::getPlaneCount() const
{
	return property.numPlanes;
}

MINT32 IImageBuffer::getColorArrangement() const
{
	return property.ColorArrangeMent;
}

MINT32 IImageBuffer::getColorSpace() const
{
	return property.colorSpace;
}

MINT32 IImageBuffer::getPlaneFD(size_t index) const
{
	return property.planes[index].fd;
}

size_t IImageBuffer::getPlaneOffsetInBytes(size_t index) const
{
	return property.planes[index].offset;
}

MINTPTR IImageBuffer::getBufVA(size_t index) const
{
	return property.planes[index].va;
}

size_t IImageBuffer::getBufSizeInBytes(size_t index) const
{
	return property.planes[index].size;
}

size_t IImageBuffer::getBufStridesInBytes(size_t index) const
{
	return property.planes[index].stride;
}

size_t IImageBuffer::getBufScanlines(size_t index) const
{
	return property.planes[index].scanline;
}

SecType IImageBuffer::getSecType() const
{
	return (SecType)0;
}

const char *portName(IMG_PORT index)
{
	switch (index) {
	case NSImgStream::IMG_PORT_IMGI:
		return "imgi";
	case NSImgStream::IMG_PORT_VIPI:
		return "vipi";
	case NSImgStream::IMG_PORT_IMG2O:
		return "img2o";
	case NSImgStream::IMG_PORT_IMG3O:
		return "img3o";
	case NSImgStream::IMG_PORT_WDMAO:
		return "wdmao";
	case NSImgStream::IMG_PORT_WROTO:
		return "wroto";
	case NSImgStream::IMG_PORT_TIMGO:
		return "timgo";
	case NSImgStream::IMG_PORT_CAMSTATI:
		return "p1stti";
	case NSImgStream::IMG_PORT_IMGSTATO:
		return "p2stto";
	case NSImgStream::IMG_PORT_METAI:
		return "tunbufi";
	case NSImgStream::IMG_PORT_TIMGI:
		return "timgi";
	case NSImgStream::IMG_PORT_PIMGI:
		return "pimgi";
	case NSImgStream::IMG_PORT_REGO:
		return "rego";
	case NSImgStream::IMG_PORT_REC_DSI:
		return "rec_dsi";
	case NSImgStream::IMG_PORT_REC_DPI:
		return "rec_dip";
	case NSImgStream::IMG_PORT_TNRSI:
		return "tnrsi";
	case NSImgStream::IMG_PORT_TNRWI:
		return "tnrwi";
	case NSImgStream::IMG_PORT_TNRMI:
		return "tnrmi";
	case NSImgStream::IMG_PORT_TNRCI:
		return "tnrci";
	case NSImgStream::IMG_PORT_TNRLFDI:
		return "tnrlfdi";
	case NSImgStream::IMG_PORT_TNRVBI:
		return "tnrvbi";
	case NSImgStream::IMG_PORT_TNRSO:
		return "tnrso";
	case NSImgStream::IMG_PORT_TNRWO:
		return "tnrwo";
	case NSImgStream::IMG_PORT_TNRMO:
		return "tnrmo";
	case NSImgStream::IMG_PORT_IMG4O:
		return "img4o";
	case NSImgStream::IMG_PORT_FEO:
		return "feo";
	case NSImgStream::IMG_PORT_CNR_BLURMAPI:
		return "cnr_blurmapi";
	case NSImgStream::IMG_PORT_LFEOI:
		return "lfeoi";
	case NSImgStream::IMG_PORT_RFEOI:
		return "rfeoi";
	case NSImgStream::IMG_PORT_FMO:
		return "fmo";
	case NSImgStream::IMG_PORT_TYUVO:
		return "tyuvo";
	case NSImgStream::IMG_PORT_TYUV2O:
		return "tyuv2o";
	case NSImgStream::IMG_PORT_TYUV3O:
		return "tyuv3o";
	case NSImgStream::IMG_PORT_TYUV4O:
		return "tyuv4o";
	case NSImgStream::IMG_PORT_TYUV5O:
		return "tyuv5o";
	case NSImgStream::IMG_PORT_LTIMGI:
		return "ltimgi";
	case NSImgStream::IMG_PORT_LTYUV2O:
		return "ltyuv2o";
	case NSImgStream::IMG_PORT_LTYUV3O:
		return "ltyuv3o";
	case NSImgStream::IMG_PORT_LTYUV4O:
		return "ltyuv4o";
	case NSImgStream::IMG_PORT_LTYUV5O:
		return "ltyuv5o";
	case NSImgStream::IMG_PORT_XTIMGI:
		return "xtimgi";
	case NSImgStream::IMG_PORT_XYUVO:
		return "xyuvo";
	case NSImgStream::IMG_PORT_XTYUV2O:
		return "xtyuv2o";
	case NSImgStream::IMG_PORT_XTYUV3O:
		return "xtyuv3o";
	case NSImgStream::IMG_PORT_XTYUV4O:
		return "xtyuv4o";
	case NSImgStream::IMG_PORT_XTYUV5O:
		return "xtyuv5o";
	case NSImgStream::IMG_PORT_XTIMGO:
		return "xtimgo";
	case NSImgStream::IMG_PORT_XTMEO:
		return "xtmeo";
	case NSImgStream::IMG_PORT_XTFDO:
		return "stfdo";
	case NSImgStream::IMG_PORT_XTADLDBGO:
		return "xtadldbgo";
	case NSImgStream::IMG_PORT_WPE_WPEI:
		return "wpe_wpei";
	case NSImgStream::IMG_PORT_WPE_VECI:
		return "wpe_veci";
	case NSImgStream::IMG_PORT_WPE_WPEO:
		return "wpe_wpeo";
	case NSImgStream::IMG_PORT_WPE_MSKO:
		return "wpe_mkso";
	case NSImgStream::IMG_PORT_WPE_PSP_COEFFI:
		return "wpe_psp_coeffi";
	case NSImgStream::IMG_PORT_WPE_TNR_WPEI:
		return "wpe_tnr_wpei";
	case NSImgStream::IMG_PORT_WPE_TNR_VECI:
		return "wpe_tnr_veci";
	case NSImgStream::IMG_PORT_WPE_TNR_WPEO:
		return "wpe_tnr_wpeo";
	case NSImgStream::IMG_PORT_WPE_TNR_MSKO:
		return "wep_tnr_msko";
	case NSImgStream::IMG_PORT_WPE_TNR_PSP_COEFFI:
		return "wpe_tnr_psp_coeffi";
	case NSImgStream::IMG_PORT_ME_L0_IMG0I:
		return "me_L0_img0i";
	case NSImgStream::IMG_PORT_ME_L0_IMG1I:
		return "me_L0_img1i";
	case NSImgStream::IMG_PORT_ME_L1_IMG0I:
		return "me_L1_img0i";
	case NSImgStream::IMG_PORT_ME_L1_IMG1I:
		return "me_L1_img1i";
	case NSImgStream::IMG_PORT_ME_IMGSTATI:
		return "me_L0_stti";
	case NSImgStream::IMG_PORT_ME_MEMILI:
		return "me_mili";
	case NSImgStream::IMG_PORT_ME_MMG_MILO:
		return "me_milo";
	case NSImgStream::IMG_PORT_ME_FMVO:
		return "me_fmvo";
	case NSImgStream::IMG_PORT_ME_L0_FMBI:
		return "me_fmb_L0i";
	case NSImgStream::IMG_PORT_ME_L1_FMBI:
		return "me_fmb_L1i";
	case NSImgStream::IMG_PORT_ME_L0_FMBO:
		return "me_fmb_L0o";
	case NSImgStream::IMG_PORT_ME_L1_FMBO:
		return "me_fmb_L1o";
	case NSImgStream::IMG_PORT_ME_FSTO:
		return "me_fsto";
	case NSImgStream::IMG_PORT_ME_LMIO:
		return "me_lmio";
	case NSImgStream::IMG_PORT_ME_L0_RMVI:
		return "me_L0_rmvi";
	case NSImgStream::IMG_PORT_ME_L1_RMVI:
		return "me_L1_rmvi";
	case NSImgStream::IMG_PORT_ME_L0_WMVO:
		return "me_L0_wmvo";
	case NSImgStream::IMG_PORT_ME_L1_WMVO:
		return "me_L1_wmvo";
	case NSImgStream::IMG_PORT_ME_CONFO:
		return "me_confo";
	case NSImgStream::IMG_PORT_ME_WMAPO:
		return "me_wmapo";
	default:
		return "unknown";
	}
	return NULL;
}

const char *colorName(EImageColorSpace color)
{
	switch (color) {
	case eImgColorSpace_UNKNOWN:
		return "eImgColorSpace_UNKNOWN";
	case eImgColorSpace_STANDARD_SHIFT:
		return "eImgColorSpace_STANDARD_SHIFT";
	case eImgColorSpace_STANDARD_MASK:
		return "eImgColorSpace_STANDARD_MASK";
	case eImgColorSpace_STANDARD_BT709:
		return "eImgColorSpace_STANDARD_BT709";
	case eImgColorSpace_STANDARD_BT601_625:
		return "eImgColorSpace_STANDARD_BT601_625";
	case eImgColorSpace_STANDARD_BT2020:
		return "eImgColorSpace_STANDARD_BT2020";
	case eImgColorSpace_STANDARD_DCI_P3:
		return "eImgColorSpace_STANDARD_DCI_P3";
	case eImgColorSpace_TRANSFER_SHIFT:
		return "eImgColorSpace_TRANSFER_SHIFT";
	case eImgColorSpace_TRANSFER_MASK:
		return "eImgColorSpace_TRANSFER_MASK";
	case eImgColorSpace_RANGE_SHIFT:
		return "eImgColorSpace_RANGE_SHIFT";
	case eImgColorSpace_RANGE_MASK:
		return "eImgColorSpace_RANGE_MASK";
	case eImgColorSpace_RANGE_FULL:
		return "eImgColorSpace_RANGE_FULL";
	case eImgColorSpace_RANGE_LIMITED:
		return "eImgColorSpace_RANGE_LIMITED";
	case eImgColorSpace_BT601_FULL:
		return "eImgColorSpace_BT601_FULL";
	case eImgColorSpace_BT601_LIMITED:
		return "eImgColorSpace_BT601_LIMITED";
	case eImgColorSpace_BT709_FULL:
		return "eImgColorSpace_BT709_FULL";
	case eImgColorSpace_BT709_LIMITED:
		return "eImgColorSpace_BT709_LIMITED";
	case eImgColorSpace_BT2020_PQ_LIMITED:
		return "eImgColorSpace_BT2020_PQ_LIMITED";
	case eImgColorSpace_BT2020_PQ_FULL:
		return "eImgColorSpace_BT2020_PQ_FULL";
	case eImgColorSpace_DISPLAY_P3:
		return "eImgColorSpace_DISPLAY_P3";
	case eImgColorSpace_INVALID:
		return "eImgColorSpace_INVALID";
	default:
		return "Unknown";
	}
}

const char *formatName(EImageFormat fmt)
{
	switch (fmt) {
	case eImgFmt_BLOB:
		return "BLOB";
	case eImgFmt_RGBA8888:
		return "RGBA8888";
	case eImgFmt_RGBX8888:
		return "RGBX8888";
	case eImgFmt_RGB888:
		return "RGB888";
	case eImgFmt_RGB565:
		return "RGB565";
	case eImgFmt_BGRA8888:
		return "BGRA8888";
	case eImgFmt_YUY2:
		return "YUY2";
	case eImgFmt_NV16:
		return "NV16";
	case eImgFmt_NV21:
		return "NV21";
	case eImgFmt_NV12:
		return "NV12";
	case eImgFmt_YV12:
		return "YV12";
	case eImgFmt_Y8:
		return "Y8";
	case eImgFmt_Y16:
		return "Y16";
	case eImgFmt_CAMERA_OPAQUE:
		return "opaque";
	case eImgFmt_YVYU:
		return "YVYU";
	case eImgFmt_UYVY:
		return "UYVY";
	case eImgFmt_VYUY:
		return "VYUY";
	case eImgFmt_NV61:
		return "NV61";
	case eImgFmt_NV12_BLK:
		return "NV12_BLK";
	case eImgFmt_NV21_BLK:
		return "NV21_BLK";
	case eImgFmt_YV16:
		return "YV16";
	case eImgFmt_I420:
		return "I420";
	case eImgFmt_I422:
		return "I422";
	case eImgFmt_YUYV_Y210:
		return "YUYV_Y210";
	case eImgFmt_YVYU_Y210:
		return "YVYU_Y210";
	case eImgFmt_UYVY_Y210:
		return "UYVY_Y210";
	case eImgFmt_VYUY_Y210:
		return "VYUY_Y210";
	case eImgFmt_YUV_P210:
		return "YUV_P210";
	case eImgFmt_YVU_P210:
		return "YVU_P210";
	case eImgFmt_YUV_P210_3PLANE:
		return "YVU_P210_3P";
	case eImgFmt_YUV_P010:
		return "YUV_P010";
	case eImgFmt_YVU_P010:
		return "YVU_P010";
	case eImgFmt_YUV_P010_3PLANE:
		return "YUV_P010_3P";
	case eImgFmt_MTK_YUYV_Y210:
		return "MTK_YUYV_Y210";
	case eImgFmt_MTK_YVYU_Y210:
		return "MTK_YVYU_Y210";
	case eImgFmt_MTK_UYVY_Y210:
		return "MTK_UYVY_Y210";
	case eImgFmt_MTK_VYUY_Y210:
		return "MTK_VYUY_Y210";
	case eImgFmt_MTK_YUV_P210:
		return "MTK_YUV_P210";
	case eImgFmt_MTK_YVU_P210:
		return "MTK_YVU_P210";
	case eImgFmt_MTK_YUV_P210_3PLANE:
		return "MTK_YVU_P210_3P";
	case eImgFmt_MTK_YUV_P010:
		return "MTK_YUV_P010";
	case eImgFmt_MTK_YVU_P010:
		return "MTK_YVU_P010";
	case eImgFmt_MTK_YUV_P010_3PLANE:
		return "MTK_YUV_P010_3P";
	case eImgFmt_YUV_P012:
		return "YUV_P012";
	case eImgFmt_YVU_P012:
		return "YVU_P012";
	case eImgFmt_MTK_YUV_P012:
		return "MTK_YUV_P012";
	case eImgFmt_MTK_YVU_P012:
		return "MTK_YVU_P012";
	case eImgFmt_ARGB8888:
		return "ARGB8888";
	case eImgFmt_RGB48:
		return "RGB48";
	case eImgFmt_BAYER8:
		return "BAYER8";
	case eImgFmt_BAYER10:
		return "BAYER10";
	case eImgFmt_BAYER12:
		return "BAYER12";
	case eImgFmt_BAYER14:
		return "BAYER14";
	case eImgFmt_BAYER12_UNPAK:
		return "BAYER12_UNPAK";
	case eImgFmt_FG_BAYER8:
		return "FG_BAYER8";
	case eImgFmt_FG_BAYER10:
		return "FG_BAYER10";
	case eImgFmt_FG_BAYER12:
		return "FG_BAYER12";
	case eImgFmt_FG_BAYER14:
		return "FG_BAYER14";
	case eImgFmt_WARP_1PLANE:
		return "WARP_1P";
	case eImgFmt_WARP_2PLANE:
		return "WARP_2P";
	case eImgFmt_WARP_3PLANE:
		return "WARP_3P";
	case eImgFmt_STA_BYTE:
		return "BYTE";
	case eImgFmt_STA_2BYTE:
		return "2BYTE";
	case eImgFmt_STA_4BYTE:
		return "4BYTE";
	case eImgFmt_STA_10BIT:
		return "10BIT";
	case eImgFmt_STA_12BIT:
		return "12BIT";
	case eImgFmt_AFBC_RGBA8888:
		return "AFBC_RGBA8888";
	case eImgFmt_AFBC_NV21:
		return "AFBC_NV21";
	case eImgFmt_AFBC_NV12:
		return "AFBC_NV12";
	case eImgFmt_UFBC_BAYER10:
		return "UFBC_BAYER10";
	case eImgFmt_UFBC_BAYER12:
		return "UFBC_BAYER12";
	case eImgFmt_UFBC_BAYER14:
		return "UFBC_BAYER14";
	case eImgFmt_UFBC_NV12:
		return "UFBC_NV12";
	case eImgFmt_UFBC_YUV_P010:
		return "UFBC_YUV_P010";
	case eImgFmt_UFBC_YUV_P012:
		return "UFBC_YUV_P012";
	case eImgFmt_ISP_TUNING:
		return "ISP_TUNING";
	default:
		return "Unkown";
	}
}

inline const char *stageName(PEU_Stage stage)
{
	switch (stage) {
	case UNKNOWN:
		return "UNKNOWN";

	/* MCNR stages */
	case HW_LTR_ME_L1:
		return "HW_LTR_ME_L1";
	case HW_ME_3PASS_MODE_0:
		return "HW_ME_3PASS_MODE_0";
	case HW_ME_3PASS_MODE_1:
		return "HW_ME_3PASS_MODE_1";
	case HW_TR_F1:
		return "HW_TR_F1";
	case HW_TR_F4:
		return "HW_TR_F4";
	case HW_TR_HWMVP:
		return "HW_TR_HWMVP";
	case HW_TR_CONF4:
		return "HW_TR_CONF4";
	case HW_TR_CONF5:
		return "HW_TR_CONF5";
	case HW_LTR_F1:
		return "HW_LTR_F1";
	case HW_LTR_F4:
		return "HW_LTR_F4";
	case HW_LTR_VBI:
		return "HW_LTR_VBI";
	case HW_WPE_W_F1:
		return "HW_WPE_W_F1";
	case HW_WPE_W_F2:
		return "HW_WPE_W_F2";
	case HW_WPE_W_F3:
		return "HW_WPE_W_F3";
	case HW_WPE_W_F4:
		return "HW_WPE_W_F4";
	case HW_WPE_W_F5:
		return "HW_WPE_W_F5";
	case HW_WPE_W_F0:
		return "HW_WPE_W_F0";
	case HW_DIP_IDI:
		return "HW_DIP_IDI";
	case HW_DIP_IDI2:
		return "HW_DIP_IDI2";
	case HW_DIP_F4:
		return "HW_DIP_F4";
	case HW_DIP_F3:
		return "HW_DIP_F3";
	case HW_DIP_F2:
		return "HW_DIP_F2";
	case HW_DIP_F1:
		return "HW_DIP_F1";
	case HW_DIP_F0:
		return "HW_DIP_F0";

	/* LPNR stages */
	case TR_R2Y:
		return "TR_R2Y";
	case P2_MS_F3:
		return "P2_MS_F3";
	case P2_MS_F2:
		return "P2_MS_F2";
	case P2_MS_F1:
		return "P2_MS_F1";
	case P2_MS_F0_PQ_DIP:
		return "P2_MS_F0_PQ_DIP";

	/* MFNR stages*/
	case BFBLD_BASE:
		return "BFBLD_BASE";
	case BFBLD_REF:
		return "BFBLD_REF";
	case BFME:
		return "BFME";
	case MCDS_F1:
		return "MCDS_F1";
	case DS:
		return "DS";
	case DS_VBI_V2:
		return "DS_VBI_V2";
	case DS_VBI_V5:
		return "DS_VBI_V5";
	case MSBLD_F0:
		return "MSBLD_F0";
	case MSBLD_F1:
		return "MSBLD_F1";
	case MSBLD_F2:
		return "MSBLD_F2";
	case MSBLD_F3:
		return "MSBLD_F3";
	case MSBLD_F4:
		return "MSBLD_F4";
	case MSBLD_F5:
		return "MSBLD_F5";
	case MSBLD_F6:
		return "MSBLD_F6";
	case AFBLD_F0:
		return "AFBLD_F0";
	case AFBLD_F1:
		return "AFBLD_F1";
	case AFBLD_F2:
		return "AFBLD_F2";
	case AFBLD_F3:
		return "AFBLD_F3";
	case AFBLD_F4:
		return "AFBLD_F4";
	case AFBLD_F5:
		return "AFBLD_F5";
	case AFBLD_F6:
		return "AFBLD_F6";
	default:
		return "NA";
	}
}

void dumpPortInfo(const PortInfo &info, int i, bool isInput)
{
	const char *ioType = isInput ? "mvIn" : "mvOut";
	const char *ioType2 = isInput ? "inputs" : "outputs";
	const char *ioType3 = isInput ? "Input" : "Output";
	(void)ioType2;
	(void)ioType3;
	IMG_PORT index = (IMG_PORT)info.mPortIdx;
	if (info.mBuffer != NULL) {
		auto &crop = info.mSrcCrop;
		auto &resizeInfo = info.mResizeInfo;

		MSize size = info.mBuffer->getImgSize();
		MINT fmt = info.mBuffer->getImgFormat();
		MINT32 fd = info.mBuffer->getPlaneFD(0);
		size_t offset = info.mBuffer->getPlaneOffsetInBytes(0);
		MINTPTR va = info.mBuffer->getBufVA(0);

		MINT32 colar = info.mBuffer->getColorArrangement();
		MINT32 colsp = info.mBuffer->getColorSpace();
		size_t planeSize = info.mBuffer->getBufSizeInBytes(0);
		size_t stride = info.mBuffer->getBufStridesInBytes(0);
		size_t scanline = info.mBuffer->getBufScanlines(0);
		auto secType = info.mBuffer->getSecType();

		LOG_ERR("   ### %s[%d] (%s) size(%dx%d) fmt(%s)"
			" fd/offset=%d_%zu va=%p",
			ioType, i, portName(index), size.w, size.h, formatName((EImageFormat)fmt),
			fd, offset, (void *)va);

		LOG_ERR("   -- ### crop [%d,%d,%d,%d]/[%d,%d,%d,%d]"
			" resizeRatio %d",
			crop.CropX, crop.CropY, crop.CropW, crop.CropH,
			crop.CropFloatX, crop.CropFloatY, crop.CropFloatW, crop.CropFloatH,
			(int)resizeInfo.mResizeRatio);

		LOG_ERR("   -- ### col(%s/%d) plane=%zu(%zu x %zu)"
			" secType %d",
			colorName((EImageColorSpace)colsp), colar, planeSize,
			stride, scanline, (int)secType);
		if (info.mBuffer->getPlaneCount() > 1) {
			planeSize = info.mBuffer->getBufSizeInBytes(1);
			stride = info.mBuffer->getBufStridesInBytes(1);
			scanline = info.mBuffer->getBufScanlines(1);
			secType = info.mBuffer->getSecType();
			LOG_ERR("   -- ### col(%s/%d) plane=%zu(%zu x %zu)",
				colorName((EImageColorSpace)colsp), colar, planeSize, stride, scanline);
		}
	} else {
		LOG_ERR("   ### Param %s[%d] idx=%d(%s) no buffer",
			ioType, i, (int)index, portName(index));
	}
}

void dumpSyncToken(const FrameParams &frameParam)
{
	std::stringstream ssWait;
	for (auto token : frameParam.mSyncTokenWaitList)
		ssWait << token << ", ";

	std::stringstream ssNotify;
	for (auto token : frameParam.mSyncTokenNotifyList)
		ssNotify << token << ", ";

	LOG_ERR("   ### Wait(%s) Notify(%s) Prev(%d) Next(%d) Stage(%s) timestamp %u",
		ssWait.str().c_str(), ssNotify.str().c_str(),
		frameParam.mSyncPrevFrameParam, frameParam.mSyncNextFrameParam,
		stageName((PEU_Stage)frameParam.mStage), frameParam.mTimestamp);

	LOG_ERR("   ### Owner %s secureFra %d scenPath %d wait %d notify %d",
		frameParam.mFrameOwner.c_str(), frameParam.mSecureFra, frameParam.mScenPath,
		frameParam.mSyncTokenWait, frameParam.mSyncTokenNotify);
}

const char *dumpOneExtraParam(const ImgExtraParam &pm)
{
	auto id = pm.mID;
	switch (id) {
	case IMG_EXTRA_PARAM_ID_DIP_MULTISCALE_INFO: {
		auto &d = pm.mData.mMutiScaleInfo;
		LOG_ERR("   ex ### IMG_EXTRA_PARAM_ID_DIP_MULTISCALE_INFO"
			" MultiScaleInfo{.mScaleRatio=%d, .mScaleIdx=%u, .mScaleTotal=%u}",
			(int)d.mScaleRatio, d.mScaleIdx, d.mScaleTotal);
		return "MultiScaleInfo";
	}
	case IMG_EXTRA_PARAM_ID_DIP_MUTLIFRAME_INFO: {
		auto &d = pm.mData.mMultiFrameInfo;
		LOG_ERR("   ex ### IMG_EXTRA_PARAM_ID_DIP_MUTLIFRAME_INFO"
			" MultiFrameInfo{.mFrameIdx=%u, .mFrameTotal=%u}",
			d.mFrameIdx, d.mFrameTotal);
		return "MultiFrameInfo";
	}
	case IMG_EXTRA_PARAM_ID_P_IMG4O_CROP_INFO: {
		auto &d = pm.mData.mPImg4oCropInfo;
		LOG_ERR("   ex ### IMG_EXTRA_PARAM_ID_P_IMG4O_CROP_INFO"
			" PImg4oCropInfo{.p_img4o_crop_x=%u, .p_img4o_crop_y=%u, "
			".p_img4o_crop_w=%u, .p_img4o_crop_h=%u, .tnrwo_scale_ratio=%u}",
			d.p_img4o_crop_x, d.p_img4o_crop_y, d.p_img4o_crop_w,
			d.p_img4o_crop_h, d.tnrwo_scale_ratio);
		return "PImg4oCropInfo";
	}
	case IMG_EXTRA_PARAM_ID_MVFRAME_INFO: {
		auto &d = pm.mData.mMVFrameInfo;
		LOG_ERR("   ex ### IMG_EXTRA_PARAM_ID_MVFRAME_INFO"
			" MVFrameInfo{.mF0Width=%u, .mF0Height=%u, .mME0Width=%u, "
			".mME0Height=%u, .mConfScaleRatio=%u}",
			d.mF0Width, d.mF0Height, d.mME0Width, d.mME0Height, d.mConfScaleRatio);
		return "MVFrameInfo";
	}
	case IMG_EXTRA_PARAM_ID_FE_INFO: {
		//auto &d = pm.mData.mFEInfo;
		LOG_ERR("   ex ### IMG_EXTRA_PARAM_ID_FE_INFO"
			" FEInfo{ To be continue }");
		return "FEInfo";
	}
	case IMG_EXTRA_PARAM_ID_FM_INFO: {
		//auto &d = pm.mData.mFMInfo;
		LOG_ERR("   ex ### IMG_EXTRA_PARAM_ID_FM_INFO"
			" FMInfo{ To be continue }");
		return "FMInfo";
	}
	case IMG_EXTRA_PARAM_ID_SRZ_INFO: {
		//auto &d = pm.mData.mSRZInfo;
		LOG_ERR("   ex ### IMG_EXTRA_PARAM_ID_SRZ_INFO"
			" SRZInfo{ To be continue }");
		return "SRZInfo";
	}
	case IMG_EXTRA_PARAM_ID_WPE_INFO:
	case IMG_EXTRA_PARAM_ID_WPE_TNR_INFO: {
		const char *name =
			(id == IMG_EXTRA_PARAM_ID_WPE_INFO) ? "IMG_EXTRA_PARAM_ID_WPE_INFO" : "IMG_EXTRA_PARAM_ID_WPE_TNR_INFO";
		auto &d = pm.mData.mWPEInfo;
		LOG_ERR("   ex ### %s"
			" WPEInfo{"
			".wpe_mode=%d, "
			".vgen_out=WPE_CrpInfo{"
			".x_start_point=%u, .x_end_point=%u, "
			".y_start_point=%u, .y_end_point=%u}, "
			".tbl_sel_v=%d, .tbl_sel_h=%d, "
			".extra_feature_index=%u, .rgb_mode=%d, ",
			name,
			d.wpe_mode,
			d.vgen_out.x_start_point, d.vgen_out.x_end_point,
			d.vgen_out.y_start_point, d.vgen_out.y_end_point,
			d.tbl_sel_v, d.tbl_sel_h,
			d.extra_feature_index, d.rgb_mode);

		LOG_ERR("   -- ex ### "
			".vgen_in=WPE_CrpOfstInfo{"
			".x_start=%u, .hr_int_ofst=%u, .hr_sub_ofst=%u, "
			".y_start=%u, .vt_int_ofst=%u, .vt_sub_ofst=%u, "
			".wd=%u, .ht=%u,}, "
			".psp_border_color_y=%u, .psp_border_color_u=%u, .psp_border_color_v=%u}",
			d.vgen_in.x_start, d.vgen_in.hr_int_ofst, d.vgen_in.hr_sub_ofst,
			d.vgen_in.y_start, d.vgen_in.vt_int_ofst, d.vgen_in.vt_sub_ofst,
			d.vgen_in.wd, d.vgen_in.ht,
			d.psp_border_color_y, d.psp_border_color_u, d.psp_border_color_v);

		return "EIS WPE Info or TNR WPE Info";
	}
	case IMG_EXTRA_PARAM_ID_ME_INFO: {
		auto &d = pm.mData.mMEInfo;
		LOG_ERR("   ex ### IMG_EXTRA_PARAM_ID_ME_INFO"
			" MEInfo{.me_mode=%d}",
			d.me_mode);
		return "ME Info";
	}
	case IMG_EXTRA_PARAM_ID_PQ_PORT_INFO: {
		auto &d = pm.mData.mPQPortInfo;
		LOG_ERR("   ex ### IMG_EXTRA_PARAM_ID_PQ_PORT_INFO"
			" PQPortInfo{.mWdmaoPQIdx=%u, .mWdmaoUserString=%lu, "
			".mWdmaoBypassCrop=%u, .mWrotoPQIdx=%u, .mWrotoUserString=%lu, "
			".mWrotoBypassCrop=%u}",
			d.mWdmaoPQIdx, d.mWdmaoUserString,
			d.mWdmaoBypassCrop, d.mWrotoPQIdx, d.mWrotoUserString,
			d.mWrotoBypassCrop);
		return "PQPortInfo";
	}
	case IMG_EXTRA_PARAM_ID_APL_INFO: {
		auto &d = pm.mData.mAPLInfo;
		LOG_ERR("   ex ### IMG_EXTRA_PARAM_ID_APL_INFO"
			" APLInfo{.mAplEnable=%u}",
			d.mAplEnable);
		return "APLInfo";
	}
	case IMG_EXTRA_PARAM_ID_COST_LEVEL_INFO: {
		auto &d = pm.mData.mCostLevel;
		LOG_ERR("   ex ### IMG_EXTRA_PARAM_ID_COST_LEVEL_INFO"
			" WPECostLevel{.costlevel=%d}",
			d.costlevel);
		return "WPECostLevel";
	}
	default:
		std::abort();
	}
}

void dumpExtraPram(const std::vector<ImgExtraParam> &extraParam)
{
	for (size_t i = 0; i < extraParam.size(); i++) {
		auto &param = extraParam[i];
		dumpOneExtraParam(param);
	}
}

void dump(const FrameParams &frameParam)
{
	std::stringstream ss;
	for (size_t i = 0; i < frameParam.mvIn.size(); i++) {
		auto &info = frameParam.mvIn[i];
		dumpPortInfo(info, i, true);
	}

	for (size_t i = 0; i < frameParam.mvOut.size(); i++) {
		auto &info = frameParam.mvOut[i];
		dumpPortInfo(info, i, false);
	}

	dumpExtraPram(frameParam.mvExtraParam);
	dumpSyncToken(frameParam);
}

IMG_PORT ImgPortMapVideoNode(IMG_PORT PortIdx)
{
	switch (PortIdx) {
	case IMG_PORT_TYUV5O:
	case IMG_PORT_FEO:
	case IMG_PORT_LTYUV5O:
	case IMG_PORT_XTYUV5O:
		return IMG_PORT_TYUV5O;
	case IMG_PORT_TIMGI:
	case IMG_PORT_LTIMGI:
	case IMG_PORT_XTIMGI:
		return IMG_PORT_TIMGI;
	case IMG_PORT_TYUVO:
	case IMG_PORT_XYUVO:
		return IMG_PORT_TYUVO;
	case IMG_PORT_TYUV2O:
	case IMG_PORT_LTYUV2O:
	case IMG_PORT_XTYUV2O:
		return IMG_PORT_TYUV2O;
	case IMG_PORT_TYUV3O:
	case IMG_PORT_LTYUV3O:
	case IMG_PORT_XTYUV3O:
		return IMG_PORT_TYUV3O;
	case IMG_PORT_TYUV4O:
	case IMG_PORT_LTYUV4O:
	case IMG_PORT_XTYUV4O:
		return IMG_PORT_TYUV4O;
	case IMG_PORT_TIMGO:
	case IMG_PORT_XTIMGO:
		return IMG_PORT_TIMGO;
	default:
		return PortIdx;
	}
}

bool IMG_PORT_MAP_HW_VIDEONODE_ID(imgsys_video_nodes_id *pNodeId,
				  IMG_PORT PortIdx)
{
	switch (PortIdx) {
		// TRAW
	case IMG_PORT_TIMGI:
	case IMG_PORT_LTIMGI:
	case IMG_PORT_XTIMGI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TIMGI_OUT;
		break;
	case IMG_PORT_DRV_PDCBUFI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_PDC_OUT;
		break;
	case IMG_PORT_TYUVO:
	case IMG_PORT_XYUVO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TYUVO_CAPTURE;
		break;
	case IMG_PORT_TYUV2O:
	case IMG_PORT_LTYUV2O:
	case IMG_PORT_XTYUV2O:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TYUV2O_CAPTURE;
		break;
	case IMG_PORT_TYUV3O:
	case IMG_PORT_LTYUV3O:
	case IMG_PORT_XTYUV3O:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TYUV3O_CAPTURE;
		break;
	case IMG_PORT_TYUV4O:
	case IMG_PORT_LTYUV4O:
	case IMG_PORT_XTYUV4O:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TYUV4O_CAPTURE;
		break;
	case IMG_PORT_TYUV5O:
	case IMG_PORT_LTYUV5O:
	case IMG_PORT_XTYUV5O:
	case IMG_PORT_FEO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TYUV5O_CAPTURE;
		break;
	case IMG_PORT_TIMGO:
	case IMG_PORT_XTIMGO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TIMGO_CAPTURE;
		break;
	case IMG_PORT_XTMEO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_XTMEO_CAPTURE;
		break;
	case IMG_PORT_XTFDO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_XTFDO_CAPTURE;
		break;
	case IMG_PORT_XTADLDBGO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_XTADLDBGO_CAPTURE;
		break;
		// DIP
	case IMG_PORT_IMGI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_IMGI_OUT;
		break;
	case IMG_PORT_VIPI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_VIPI_OUT;
		break;
	case IMG_PORT_REC_DSI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_REC_DSI_OUT;
		break;
	case IMG_PORT_REC_DPI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_REC_DPI_OUT;
		break;
	case IMG_PORT_CNR_BLURMAPI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_CNR_BLURMAPI_OUT;
		break;
	case IMG_PORT_LFEOI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_LFEI_OUT;
		break;
	case IMG_PORT_RFEOI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_RFEI_OUT;
		break;
	case IMG_PORT_TNRSI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TNRSI_OUT;
		break;
	case IMG_PORT_TNRWI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TNRWI_OUT;
		break;
	case IMG_PORT_TNRMI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TNRMI_OUT;
		break;
	case IMG_PORT_TNRCI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TNRCI_OUT;
		break;
	case IMG_PORT_TNRLFDI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TNRLI_OUT;
		break;
	case IMG_PORT_TNRVBI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TNRVBI_OUT;
		break;
	case IMG_PORT_IMG2O:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_IMG2O_CAPTURE;
		break;
	case IMG_PORT_IMG3O:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_IMG3O_CAPTURE;
		break;
	case IMG_PORT_IMG4O:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_IMG4O_CAPTURE;
		break;
	case IMG_PORT_FMO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_FMO_CAPTURE;
		break;
	case IMG_PORT_TNRSO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TNRSO_CAPTURE;
		break;
	case IMG_PORT_TNRWO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TNRWO_CAPTURE;
		break;
	case IMG_PORT_TNRMO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TNRMO_CAPTURE;
		break;
		// PQ-DIP
	case IMG_PORT_PIMGI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_PIMGI_OUT;
		break;
	case IMG_PORT_WDMAO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_WROT_A_CAPTURE;
		break;
	case IMG_PORT_WROTO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_WROT_B_CAPTURE;
		break;
	case IMG_PORT_A_TCCSO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TCCSO_A_CAPTURE;
		break;
	case IMG_PORT_B_TCCSO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_TCCSO_B_CAPTURE;
		break;
		// WPE
	case IMG_PORT_WPE_WPEI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_WWPEI_OUT;
		break;
	case IMG_PORT_WPE_VECI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_WVECI_OUT;
		break;
	case IMG_PORT_WPE_PSP_COEFFI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_WPSP_COEFI_OUT;
		break;
	case IMG_PORT_WPE_WPEO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_WWPEO_CAPTURE;
		break;
	case IMG_PORT_WPE_MSKO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_WMSKO_CAPTURE;
		break;
	case IMG_PORT_WPE_TNR_WPEI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_WTWPEI_OUT;
		break;
	case IMG_PORT_WPE_TNR_VECI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_WTVECI_OUT;
		break;
	case IMG_PORT_WPE_TNR_PSP_COEFFI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_WTPSP_COEFI_OUT;
		break;
	case IMG_PORT_WPE_TNR_WPEO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_WTWPEO_CAPTURE;
		break;
	case IMG_PORT_WPE_TNR_MSKO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_WTMSKO_CAPTURE;
		break;
		// ME
	case IMG_PORT_ME_L0_IMG0I:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEL0IMG0_OUT;
		break;
	case IMG_PORT_ME_L0_IMG1I:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEL0IMG1_OUT;
		break;
	case IMG_PORT_ME_L1_IMG0I:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEL1IMG0_OUT;
		break;
	case IMG_PORT_ME_L1_IMG1I:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEL1IMG1_OUT;
		break;
	case IMG_PORT_ME_IMGSTATI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEIMGSTATS_OUT;
		break;
	case IMG_PORT_ME_MMG_MILO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEMMGMIL_CAPTURE;
		break;
	case IMG_PORT_ME_L0_RMVI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEL0RMV_OUT;
		break;
	case IMG_PORT_ME_L1_RMVI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEL1RMV_OUT;
		break;
	case IMG_PORT_ME_L0_WMVO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEL0WMV_CAPTURE;
		break;
	case IMG_PORT_ME_L1_WMVO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEL1WMV_CAPTURE;
		break;
	case IMG_PORT_ME_CONFO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MECONF_CAPTURE;
		break;
	case IMG_PORT_ME_WMAPO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEWMAP_CAPTURE;
		break;
	case IMG_PORT_ME_FMVO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEFMV_CAPTURE;
		break;
	case IMG_PORT_ME_L0_FMBI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEL0FMB_OUT;
		break;
	case IMG_PORT_ME_L1_FMBI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEL1FMB_OUT;
		break;
	case IMG_PORT_ME_L0_FMBO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEL0WFMB_CAPTURE;
		break;
	case IMG_PORT_ME_L1_FMBO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEL1WFMB_CAPTURE;
		break;
	case IMG_PORT_ME_FSTO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEFST_CAPTURE;
		break;
	case IMG_PORT_ME_LMIO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MELMI_CAPTURE;
		break;
	case IMG_PORT_ME_MEMILI:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_MEMIL_OUT;
		break;
		// Common Part
	case IMG_PORT_IMGSTATO:
		(*pNodeId) = MTK_IMGSYS_VIDEO_NODE_ID_IMGSTATO_CAPTURE;
		break;
	default:
		LOG_ERR(
			"I can't find responding video node hw id by using PortIdx(%d)!!\n",
			PortIdx);
		return false;
	}
	return true;
}

bool TaskPriorityMapping(job_thre_pr *job_pr, IMG_PRIORITY ImgPr)
{
	switch (ImgPr) {
	case IMG_PRIORITY_PREVIEW:
	case IMG_PRIORITY_RECORD:
		*job_pr = job_thre_pr_high;
		break;
	case IMG_PRIORITY_AUX_1:
		*job_pr = job_thre_pr_middle;
		break;
	case IMG_PRIORITY_AUX_2:
		*job_pr = job_thre_pr_low;
		break;
	case IMG_PRIORITY_DEFAULT:
		*job_pr = job_thre_pr_low;
		break;
	default:
		LOG_ERR(
			"Error!! We can't find Task Priority Mapping about ImgPriority(%d)\n",
			ImgPr);
		return false;
	}
	return true;
}

bool TransformMapping(imgsysrotation &ImgRot,
		      imgsysflip &ImgFlip,
		      MINT32 Transform)
{
	switch (Transform) {
	case 0:
		ImgRot = imgsysrotation_0;
		ImgFlip = imgsysflip_off;
		break;
	case eTransform_FLIP_H:
		ImgRot = imgsysrotation_0;
		ImgFlip = imgsysflip_on;
		break;
	case eTransform_FLIP_V:
		ImgRot = imgsysrotation_180;
		ImgFlip = imgsysflip_on;
		break;
	case eTransform_ROT_180:
		ImgRot = imgsysrotation_180;
		ImgFlip = imgsysflip_off;
		break;
	case eTransform_ROT_90:
		ImgRot = imgsysrotation_90;
		ImgFlip = imgsysflip_off;
		break;
	case (eTransform_FLIP_H | eTransform_ROT_90):
		ImgRot = imgsysrotation_270;
		ImgFlip = imgsysflip_on;
		break;
	case (eTransform_FLIP_V | eTransform_ROT_90):
		ImgRot = imgsysrotation_90;
		ImgFlip = imgsysflip_on;
		break;
	case eTransform_ROT_270:
		ImgRot = imgsysrotation_270;
		ImgFlip = imgsysflip_off;
		break;
	default:
		LOG_ERR(
			"Error!! We don't support this transform, Please check enque "
			"setting!!\n");
		return false;
	}
	return true;
}

img_resize_ratio ResizeRatioMapping(IMG_RESIZE_RATIO RszRatio)
{
	switch (RszRatio) {
	case IMG_RESIZE_ANYRATIO:
		return img_resize_anyratio;
	case IMG_RESIZE_DOWN4:
		return img_resize_down4;
	case IMG_RESIZE_DOWN2:
		return img_resize_down2;
	case IMG_RESIZE_DOWN42:
		return img_resize_down42;
	default:
		LOG_ERR(
			"Error!! We don't support this resize ratio selection, Please check "
			"enque setting!!\n");
		return img_resize_anyratio;
	}
}

MUINT32 ImgBufFmtMappingToV4L2Fmt(NSCam::EImageFormat eImgFmtBuf,
				  MUINT32 ColorArrangement)
{
	switch (eImgFmtBuf) {
	// YUV Format
	case eImgFmt_NV61:
		return V4L2_PIX_FMT_NV61;
	case eImgFmt_NV16:
		return V4L2_PIX_FMT_NV16;
	case eImgFmt_NV12:
		return V4L2_PIX_FMT_NV12;
	case eImgFmt_NV21:
		return V4L2_PIX_FMT_NV21;
	case eImgFmt_YV16:
		return V4L2_PIX_FMT_YUV422P;
	case eImgFmt_I422:
		return V4L2_PIX_FMT_YUV422;
	case eImgFmt_I420:
		return V4L2_PIX_FMT_YUV420;
	case eImgFmt_YV12:
		return V4L2_PIX_FMT_YVU420;
	case eImgFmt_Y8:
		return V4L2_PIX_FMT_GREY;
	case eImgFmt_UYVY:
		return V4L2_PIX_FMT_UYVY;
	case eImgFmt_YUY2:
		return V4L2_PIX_FMT_YUYV;
	case eImgFmt_YVYU:
		return V4L2_PIX_FMT_YVYU;
	case eImgFmt_VYUY:
		return V4L2_PIX_FMT_VYUY;
	case eImgFmt_MTK_YUYV_Y210:
		return V4L2_PIX_FMT_YUYV_Y210P;
	case eImgFmt_MTK_YVYU_Y210:
		return V4L2_PIX_FMT_YVYU_Y210P;
	case eImgFmt_MTK_UYVY_Y210:
		return V4L2_PIX_FMT_UYVY_Y210P;
	case eImgFmt_MTK_VYUY_Y210:
		return V4L2_PIX_FMT_VYUY_Y210P;
	case eImgFmt_MTK_YUV_P210:
		return V4L2_PIX_FMT_YUV_2P210P;
	case eImgFmt_MTK_YVU_P210:
		return V4L2_PIX_FMT_YVU_2P210P;
	case eImgFmt_MTK_YUV_P210_3PLANE:
		return V4L2_PIX_FMT_YUV_3P210P;
	case eImgFmt_MTK_YUV_P010:
		return V4L2_PIX_FMT_YUV_2P010P;
	case eImgFmt_MTK_YVU_P010:
		return V4L2_PIX_FMT_YVU_2P010P;
	case eImgFmt_MTK_YUV_P010_3PLANE:
		return V4L2_PIX_FMT_YUV_3P010P;
	case eImgFmt_YUYV_Y210:
		return V4L2_PIX_FMT_YUYV_Y210;
	case eImgFmt_YVYU_Y210:
		return V4L2_PIX_FMT_YVYU_Y210;
	case eImgFmt_UYVY_Y210:
		return V4L2_PIX_FMT_UYVY_Y210;
	case eImgFmt_VYUY_Y210:
		return V4L2_PIX_FMT_VYUY_Y210;
	case eImgFmt_YUV_P210:
		return V4L2_PIX_FMT_YUV_2P210;
	case eImgFmt_YVU_P210:
		return V4L2_PIX_FMT_YVU_2P210;
	case eImgFmt_YUV_P210_3PLANE:
		return V4L2_PIX_FMT_YUV_3P210;
	case eImgFmt_YUV_P010:
		return V4L2_PIX_FMT_YUV_2P010;
	case eImgFmt_YVU_P010:
		return V4L2_PIX_FMT_YVU_2P010;
	case eImgFmt_YUV_P010_3PLANE:
		return V4L2_PIX_FMT_YUV_3P010;
	case eImgFmt_MTK_YUV_P012:
		return V4L2_PIX_FMT_YUV_2P012P;
	case eImgFmt_MTK_YVU_P012:
		return V4L2_PIX_FMT_YVU_2P012P;
	case eImgFmt_YUV_P012:
		return V4L2_PIX_FMT_YUV_2P012;
	case eImgFmt_YVU_P012:
		return V4L2_PIX_FMT_YVU_2P012;
	// UFBC YUV Format
	case eImgFmt_UFBC_NV12:
		return V4L2_PIX_FMT_UFBC_NV12;
	case eImgFmt_UFBC_NV21:
		return V4L2_PIX_FMT_UFBC_NV21;
	case eImgFmt_UFBC_YUV_P010:
		return V4L2_PIX_FMT_UFBC_YUV_2P010P;
	case eImgFmt_UFBC_YVU_P010:
		return V4L2_PIX_FMT_UFBC_YVU_2P010P;
	case eImgFmt_UFBC_YUV_P012:
		return V4L2_PIX_FMT_UFBC_YUV_2P012P;
	case eImgFmt_UFBC_YVU_P012:
		return V4L2_PIX_FMT_UFBC_YVU_2P012P;
	// RAW Format
	case eImgFmt_BAYER10_MIPI:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGRM10;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRGM10;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBGM10;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGBM10;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGRM10;
		}
		break;
	case eImgFmt_BAYER8:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGR8;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRG8;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBG8;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGB8;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGR8;
		}
		break;
	case eImgFmt_BAYER10:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGR10;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRG10;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBG10;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGB10;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGR10;
		}
		break;
	case eImgFmt_BAYER12:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGR12;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRG12;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBG12;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGB12;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGR12;
		}
		break;
	case eImgFmt_BAYER14:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGR14;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRG14;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBG14;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGB14;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGR14;
		}
		break;
	case eImgFmt_BAYER10_UNPAK:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGRU10;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRGU10;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBGU10;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGBU10;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGRU10;
		}
		break;
	case eImgFmt_BAYER12_UNPAK:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGRU12;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRGU12;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBGU12;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGBU12;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGRU12;
		}
		break;
	case eImgFmt_BAYER14_UNPAK:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGRU14;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRGU14;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBGU14;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGBU14;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGRU14;
		}
		break;
	case eImgFmt_BAYER15_UNPAK:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGRU15;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRGU15;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBGU15;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGBU15;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGRU15;
		}
		break;
	case eImgFmt_BAYER16_UNPAK:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGR16;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRG16;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBG16;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGB16;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGR16;
		}
		break;
	case eImgFmt_BAYER22_PAK:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGR22;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRG22;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBG22;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGB22;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGR22;
		}
		break;
	// UFBC RAW Format
	case eImgFmt_UFBC_BAYER8:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_UFBC_SBGGR8;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_UFBC_SGBRG8;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_UFBC_SGRBG8;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_UFBC_SRGGB8;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_UFBC_SBGGR8;
		}
		break;
	case eImgFmt_UFBC_BAYER10:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_UFBC_SBGGR10;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_UFBC_SGBRG10;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_UFBC_SGRBG10;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_UFBC_SRGGB10;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_UFBC_SBGGR10;
		}
		break;
	case eImgFmt_UFBC_BAYER12:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_UFBC_SBGGR12;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_UFBC_SGBRG12;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_UFBC_SGRBG12;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_UFBC_SRGGB12;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_UFBC_SBGGR12;
		}
		break;
	case eImgFmt_UFBC_BAYER14:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_UFBC_SBGGR14;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_UFBC_SGBRG14;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_UFBC_SGRBG14;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_UFBC_SRGGB14;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_UFBC_SBGGR14;
		}
		break;

	case eImgFmt_FG_BAYER8:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGR8F;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRG8F;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBG8F;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGB8F;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGR8F;
		}
		break;

	case eImgFmt_FG_BAYER10:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGR10F;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRG10F;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBG10F;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGB10F;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGR10F;
		}
		break;

	case eImgFmt_FG_BAYER12:
		switch (ColorArrangement) {
		case SENSOR_FORMAT_ORDER_RAW_B:
			return V4L2_PIX_FMT_MTISP_SBGGR12F;
		case SENSOR_FORMAT_ORDER_RAW_Gb:
			return V4L2_PIX_FMT_MTISP_SGBRG12F;
		case SENSOR_FORMAT_ORDER_RAW_Gr:
			return V4L2_PIX_FMT_MTISP_SGRBG12F;
		case SENSOR_FORMAT_ORDER_RAW_R:
			return V4L2_PIX_FMT_MTISP_SRGGB12F;
		default:
			LOG_WRN(
				"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
				"convert to v4l2 format\n",
				eImgFmtBuf, ColorArrangement);
			return V4L2_PIX_FMT_MTISP_SBGGR12F;
		}
		break;
	// RGB Format
	case eImgFmt_RGB565:
		return V4L2_PIX_FMT_RGB565;
	case eImgFmt_RGB888:
		return V4L2_PIX_FMT_RGB24;
	case eImgFmt_RGBA8888:
		return V4L2_PIX_FMT_RGBA32;
	case eImgFmt_BGRA8888:
		return V4L2_PIX_FMT_BGRA32;
	case eImgFmt_RGB48:
		return V4L2_PIX_FMT_MTISP_RGB48;
	// AFBC format
	case eImgFmt_AFBC_RGBA8888:
		return V4L2_PIX_FMT_AFBC_RGBA32;
	case eImgFmt_AFBC_NV12:
		return V4L2_PIX_FMT_AFBC_NV12;
	case eImgFmt_AFBC_NV21:
		return V4L2_PIX_FMT_AFBC_NV21;
	case eImgFmt_AFBC_MTK_YUV_P010:
		return V4L2_PIX_FMT_AFBC_YUV_2P010P;
	case eImgFmt_AFBC_MTK_YVU_P010:
		return V4L2_PIX_FMT_AFBC_YVU_2P010P;
	// RGB 3 Plane Format
	case eImgFmt_RGB8_PAK_3PLANE:
		return V4L2_PIX_FMT_MTISP_RGB3PP8;
	case eImgFmt_RGB10_PAK_3PLANE:
		return V4L2_PIX_FMT_MTISP_RGB3PP10;
	case eImgFmt_RGB12_PAK_3PLANE:
		return V4L2_PIX_FMT_MTISP_RGB3PP12;
	// FullG 2 Plane Format
	case eImgFmt_FG_BAYER8_PAK_2PLANE:
		return V4L2_PIX_FMT_MTISP_FGRBP8;
	case eImgFmt_FG_BAYER10_PAK_2PLANE:
		return V4L2_PIX_FMT_MTISP_FGRBP10;
	case eImgFmt_FG_BAYER12_PAK_2PLANE:
		return V4L2_PIX_FMT_MTISP_FGRBP12;
	// FullG 3 Plane Format
	case eImgFmt_FG_BAYER8_PAK_3PLANE:
		return V4L2_PIX_FMT_MTISP_FGRB3P8;
	case eImgFmt_FG_BAYER10_PAK_3PLANE:
		return V4L2_PIX_FMT_MTISP_FGRB3P10;
	case eImgFmt_FG_BAYER12_PAK_3PLANE:
		return V4L2_PIX_FMT_MTISP_FGRB3P12;
	// WARP Format
	case eImgFmt_WARP_2PLANE:
		return V4L2_PIX_FMT_WARP2P;
	// Tuning Data Format
	case eImgFmt_BLOB:
	case eImgFmt_ISP_TUNING:
		return V4L2_META_FMT_MTISP_DESC;
	case eImgFmt_STA_BYTE:
		return V4L2_PIX_FMT_MTISP_Y8;
	case eImgFmt_STA_2BYTE:
		return V4L2_PIX_FMT_MTISP_Y16;
	case eImgFmt_STA_4BYTE:
		return V4L2_PIX_FMT_MTISP_Y32;
	default:
		LOG_ERR(
			"Error!! don't support Imgbuf format(%x), ColorArrangement(%d) "
			"convert to v4l2 format\n",
			eImgFmtBuf, ColorArrangement);
		break;
	}

	return 0;
}

bool TRAWTIMGODumpMapping(const unsigned int OutSel,
			  TRAW_CRP_DUMP_SELECT *CrpDumpSel,
			  TRAW_TIMGO_DUMP_SELECT *TimgoDumpSel)
{
	switch (OutSel) {
	case IMG_OUTPUT_SEL_TIMGO_AFTER_DGN:
		(*CrpDumpSel) = TRAW_CRP_DGN;
		(*TimgoDumpSel) = TRAW_TIMGO_CRP;
		break;
	case IMG_OUTPUT_SEL_TIMGO_AFTER_LSC:
		(*CrpDumpSel) = TRAW_CRP_LSC;
		(*TimgoDumpSel) = TRAW_TIMGO_CRP;
		break;
	case IMG_OUTPUT_SEL_TIMGO_AFTER_HLR:
		(*CrpDumpSel) = TRAW_CRP_HLR;
		(*TimgoDumpSel) = TRAW_TIMGO_CRP;
		break;
	case IMG_OUTPUT_SEL_TIMGO_AFTER_LTM:
		(*CrpDumpSel) = TRAW_CRP_LTM;
		(*TimgoDumpSel) = TRAW_TIMGO_CRP;
		break;
	case IMG_OUTPUT_SEL_TIMGO_AFTER_CRNR:
		(*CrpDumpSel) = TRAW_CRP_CRNR;
		(*TimgoDumpSel) = TRAW_TIMGO_CRP;
		break;
	case IMG_OUTPUT_SEL_TIMGO_AFTER_CCM:
		(*CrpDumpSel) = TRAW_CRP_LTM;
		(*TimgoDumpSel) = TRAW_TIMGO_CCM;
		break;
	default:
		LOG_ERR("Error!! No Support TIMGO Dump Selection:%d\n", OutSel);
		return false;
	}
	return true;
}

TRAW_SCENARIO_TAG TRAWScenarioMapping(IMG_SCENARIO_PATH ImgScen)
{
	switch (ImgScen) {
	case IMG_SCENARIO_NORMAL:
		return TRAW_SCENARIO_NORMAL;
	case IMG_SCENARIO_FE:
		return TRAW_SCENARIO_FE;
	default:
		LOG_ERR("Error!! don't support ImgScen Scen(%d) Setting to TRAW HW\n",
			ImgScen);
		return TRAW_SCENARIO_NORMAL;
	}
}

bool TRAWTIMGISelMapping(const unsigned int InSel,
			 TrawPrcRawTypeSelect *PrcRawTypeSel)
{
	switch (InSel) {
	case IMG_INPUT_SEL_PRC_RAW_I_1:
		(*PrcRawTypeSel) = TRAW_PRC_RAW_TYPE_20B;
		break;
	case IMG_INPUT_SEL_PRC_RAW_I_2:
		(*PrcRawTypeSel) = TRAW_PRC_RAW_TYPE_16B;
		break;
	default:
		(*PrcRawTypeSel) = TRAW_PRC_RAW_TYPE_NONE;
		LOG_ERR("Error!! No Support IMGI In Selection:%d\n", InSel);
		return false;
	}
	return true;
}

TRAW_HW_ID TRAWHWIDMapping(const FrameParams &FrameParams)
{
	int i;
	for (i = 0; i < (int)FrameParams.mvIn.size(); i++) {
		switch (FrameParams.mvIn.at(i).mPortIdx) {
		case IMG_PORT_TIMGI:
			return TRAW_HW_TRAW;
		case IMG_PORT_LTIMGI:
			return TRAW_HW_LTRAW;
		case IMG_PORT_XTIMGI:
			return TRAW_HW_XTRAW;
		}
	}
	for (i = 0; i < (int)FrameParams.mvOut.size(); i++) {
		switch (FrameParams.mvOut.at(i).mPortIdx) {
		case IMG_PORT_TYUVO:
		case IMG_PORT_TYUV2O:
		case IMG_PORT_TYUV3O:
		case IMG_PORT_TYUV4O:
		case IMG_PORT_TYUV5O:
			return TRAW_HW_TRAW;
		case IMG_PORT_LTIMGI:
		case IMG_PORT_LTYUV2O:
		case IMG_PORT_LTYUV3O:
		case IMG_PORT_LTYUV4O:
		case IMG_PORT_LTYUV5O:
			return TRAW_HW_LTRAW;
		case IMG_PORT_XYUVO:
		case IMG_PORT_XTYUV2O:
		case IMG_PORT_XTYUV3O:
		case IMG_PORT_XTYUV4O:
		case IMG_PORT_XTYUV5O:
		case IMG_PORT_XTIMGO:
		case IMG_PORT_XTMEO:
		case IMG_PORT_XTFDO:
		case IMG_PORT_XTADLDBGO:
			return TRAW_HW_XTRAW;
		}
	}
	LOG_ERR("Error!! No Support HW TRAW ID\n");
	return TRAW_HW_TRAW;
}

EStreamTag DIPScenarioMapping(IMG_SCENARIO_PATH ImgScen)
{
	switch (ImgScen) {
	case IMG_SCENARIO_NORMAL:
		return EStreamTag_Normal;
	case IMG_SCENARIO_BOKEH:
		return EStreamTag_Bokeh;
	case IMG_SCENARIO_FM:
		return EStreamTag_FM;
	default:
		LOG_ERR("Error!! don't support ImgScen Scen(%d) Setting to DIP HW\n",
			ImgScen);
		return EStreamTag_Normal;
	}
}

// For PQ-DIP
IMG_PROFILE_ENUM ColorSpaceMapping(NSCam::EImageColorSpace ImgColorSpace)
{
	switch (ImgColorSpace) {
	case eImgColorSpace_BT601_FULL:
		return IMG_PROFILE_JPEG;
	case eImgColorSpace_BT601_LIMITED:
		return IMG_PROFILE_BT601;
	case eImgColorSpace_BT709_FULL:
		return IMG_PROFILE_FULL_BT709;
	case eImgColorSpace_BT709_LIMITED:
		return IMG_PROFILE_BT709;
	case eImgColorSpace_BT2020_PQ_FULL:
		return IMG_PROFILE_FULL_BT2020;
	case eImgColorSpace_BT2020_PQ_LIMITED:
		return IMG_PROFILE_BT2020;
	default:
		LOG_VRB("Error!! don't support Color Space(%d) Mapping to PQ-DIP HW\n",
			ImgColorSpace);
		return IMG_PROFILE_FULL_BT601;
	}
}

bool HandleCtrlMeta(RequestInfo *pReqInfo,
		    ImgInitParam *pUserParam,
		    const EIGHTCC &userid,
		    [[maybe_unused]] int frm,
		    [[maybe_unused]] MUINT32 TotalFrm,
		    const FrameParams &pFrmParam,
		    struct ctrl_meta_t *pCtrlMeta,
		    MUINT32 CtrlMetaOffset,
		    struct frameparams *pBufInfo,
		    int DeviceTuningEn,
		    [[maybe_unused]] bool bMEExist)
{
	std::shared_ptr<const NSCam::NSImgStream::ImgParams> pParams =
		pReqInfo->pParams;
	IMG_PORT PortIdx;
	unsigned int notifytoken_size = 0;
	unsigned int waittoken_size = 0;
	unsigned int tokenidx = 0;
	WPE_MODE wpemode = EWPE_HW_DEFAULT;
	MUINT32 CostLevel = COST_LEVEL_ORI;

	(void)wpemode;

	// Handle Common Part
	if (pReqInfo->pParams->mHWSharing == MTRUE) {
		pCtrlMeta->common.is_timeshared = 1; /* 0: normal, 1: vss */
	} else {
		pCtrlMeta->common.is_timeshared = 0;
	}
	// pCtrlMeta->common.needDump = pFrmParam.mNeedDump;
	pCtrlMeta->common.timestamp = pFrmParam.mTimestamp;
	pCtrlMeta->common.frame_no = pParams->mFrameNo;
	pCtrlMeta->common.unique_key = pParams->mRequestNo;
	pCtrlMeta->common.stage = pFrmParam.mStage;
	pCtrlMeta->common.request_fd = pReqInfo->mpRequest->GetRequestFD();
	pCtrlMeta->common.is_secureFrm = pFrmParam.mSecureFra;
	if (pFrmParam.mFrameOwner.isValid()) {
		pCtrlMeta->common.frm_owner = pFrmParam.mFrameOwner.u64();
	} else {
		pCtrlMeta->common.frm_owner = userid.u64();
	}

	if (pFrmParam.mpfnCallback != NULL) {
		pCtrlMeta->common.is_early_cb = MTRUE;
	} else {
		pCtrlMeta->common.is_early_cb = MFALSE;
	}
	pCtrlMeta->common.enquetime = pReqInfo->enque_time;
	if (pUserParam->mMaxFps == 0) {
		pCtrlMeta->common.fps = pUserParam->mMaxFps;
	} else {
		if (pReqInfo->pParams->mFps > pUserParam->mMaxFps) {
			LOG_ERR("mImgParams.mFps:%d is bigger than pUserParam.mMaxFps(%d)!!",
				pReqInfo->pParams->mFps, pUserParam->mMaxFps);
		}
		pCtrlMeta->common.fps = pReqInfo->pParams->mFps;
	}
	pCtrlMeta->common.sync_prev = pFrmParam.mSyncPrevFrameParam;
	pCtrlMeta->common.sync_next = pFrmParam.mSyncNextFrameParam;
	pCtrlMeta->common.syncid = pParams->mSyncID;
	notifytoken_size = pFrmParam.mSyncTokenNotifyList.size();
	waittoken_size = pFrmParam.mSyncTokenWaitList.size();

	if ((notifytoken_size > 0) || (waittoken_size > 0) ||
	    (pFrmParam.mSyncTokenNotify != 0) || (pFrmParam.mSyncTokenWait != 0)) {
		pCtrlMeta->common.stoken_en = true;
		pCtrlMeta->common.stoken_num = notifytoken_size + waittoken_size;
		if (pFrmParam.mSyncTokenNotify != 0) {
			pCtrlMeta->common.stoken_num = pCtrlMeta->common.stoken_num + 1;
		}
		if (pFrmParam.mSyncTokenWait != 0) {
			pCtrlMeta->common.stoken_num = pCtrlMeta->common.stoken_num + 1;
		}

		if (pCtrlMeta->common.stoken_num > STOKEN_LISTLEN) {
			LOG_ERR(
				"%s stoken_num(%d) is bigger than STOKEN_LISTLEN, please check the "
				"setting!! notifytoken_size:%d, waittoken_size:%d\n",
				__func__, pCtrlMeta->common.stoken_num, notifytoken_size,
				waittoken_size);

			return false;
		}
		for (auto const &Notify : pFrmParam.mSyncTokenNotifyList) {
			pCtrlMeta->common.stoken[tokenidx].type = token_set;
			pCtrlMeta->common.stoken[tokenidx].token_key = Notify;
			tokenidx++;
		}
		if (pFrmParam.mSyncTokenNotify != 0) {
			pCtrlMeta->common.stoken[tokenidx].type = token_set;
			pCtrlMeta->common.stoken[tokenidx].token_key = pFrmParam.mSyncTokenNotify;
			tokenidx++;
		}

		for (auto const &Wait : pFrmParam.mSyncTokenWaitList) {
			pCtrlMeta->common.stoken[tokenidx].type = token_wait;
			pCtrlMeta->common.stoken[tokenidx].token_key = Wait;
			tokenidx++;
		}
		if (pFrmParam.mSyncTokenWait != 0) {
			pCtrlMeta->common.stoken[tokenidx].type = token_wait;
			pCtrlMeta->common.stoken[tokenidx].token_key = pFrmParam.mSyncTokenWait;
			tokenidx++;
		}
		LOG_ADBDBG(
			"frm:%d RequestFD:%d request_no:%d, frame_no:%d, username(%s), "
			"userid(%llx), "
			"notifytoken_size:%d "
			"waittoken_size:%d pFrmParam.mSyncTokenNotify:%d "
			"pFrmParam.mSyncTokenWait:%d, pCtrlMeta->common.stoken_num:%d "
			"mtoken:%d\n"
			"type:%d token_key:%d type:%d token_key:%d type:%d token_key:%d "
			"type:%d token_key:%d\n"
			"type:%d token_key:%d type:%d token_key:%d type:%d token_key:%d "
			"type:%d token_key:%d\n"
			"type:%d token_key:%d type:%d token_key:%d type:%d token_key:%d "
			"type:%d token_key:%d\n"
			"type:%d token_key:%d type:%d token_key:%d type:%d token_key:%d "
			"type:%d token_key:%d\n",
			frm, pReqInfo->mpRequest->GetRequestFD(), pParams->mRequestNo,
			pParams->mFrameNo, userid.c_str(), userid.u64(), notifytoken_size,
			waittoken_size, pFrmParam.mSyncTokenNotify, pFrmParam.mSyncTokenWait,
			pCtrlMeta->common.stoken_num, pFrmParam.mDCinfo.mtoken,
			pCtrlMeta->common.stoken[0].type, pCtrlMeta->common.stoken[0].token_key,
			pCtrlMeta->common.stoken[1].type, pCtrlMeta->common.stoken[1].token_key,
			pCtrlMeta->common.stoken[2].type, pCtrlMeta->common.stoken[2].token_key,
			pCtrlMeta->common.stoken[3].type, pCtrlMeta->common.stoken[3].token_key,
			pCtrlMeta->common.stoken[4].type, pCtrlMeta->common.stoken[4].token_key,
			pCtrlMeta->common.stoken[5].type, pCtrlMeta->common.stoken[5].token_key,
			pCtrlMeta->common.stoken[6].type, pCtrlMeta->common.stoken[6].token_key,
			pCtrlMeta->common.stoken[7].type, pCtrlMeta->common.stoken[7].token_key,
			pCtrlMeta->common.stoken[8].type, pCtrlMeta->common.stoken[8].token_key,
			pCtrlMeta->common.stoken[9].type, pCtrlMeta->common.stoken[9].token_key,
			pCtrlMeta->common.stoken[10].type,
			pCtrlMeta->common.stoken[10].token_key,
			pCtrlMeta->common.stoken[11].type,
			pCtrlMeta->common.stoken[11].token_key,
			pCtrlMeta->common.stoken[12].type,
			pCtrlMeta->common.stoken[12].token_key,
			pCtrlMeta->common.stoken[13].type,
			pCtrlMeta->common.stoken[13].token_key,
			pCtrlMeta->common.stoken[14].type,
			pCtrlMeta->common.stoken[14].token_key,
			pCtrlMeta->common.stoken[15].type,
			pCtrlMeta->common.stoken[15].token_key);
	} else {
		pCtrlMeta->common.stoken_en = false;
		pCtrlMeta->common.stoken_num = 0;
	}

	pCtrlMeta->common.needDump = MFALSE;
	job_thre_pr job_pr;
	if (!TaskPriorityMapping(&job_pr, pUserParam->mPriority)) {
		LOG_ERR("Task Priority(%d) Mapping Error, please check the setting!!\n",
			pUserParam->mPriority);
		return false;
	}
	pCtrlMeta->common.priority = job_pr;
	pCtrlMeta->common.is_lowlatency = pUserParam->mLowLatency;

	// Handle PortSelP Part
	for (auto const &PortSelP : pFrmParam.mPortSel) {
		switch (PortSelP.first) {
		case IMG_PORT_TIMGO:
			NSCam::NSImgStream::TRAWTIMGODumpMapping(
				PortSelP.second, &pCtrlMeta->traw_mdata.CrpDumpSel,
				&pCtrlMeta->traw_mdata.TimgoDumpSel);
			break;
		case IMG_PORT_TIMGI:
			NSCam::NSImgStream::TRAWTIMGISelMapping(
				PortSelP.second, &pCtrlMeta->traw_mdata.PrcRawType);
			break;
		default:
			LOG_WRN("No Support Port Id :%d Selection", PortSelP.first);
			break;
		}
		// PortSelP.
	}
	// Handle mvExtraParam Part
	for (auto const &ExtP : pFrmParam.mvExtraParam) {
		struct wpe_ctrl *wpeCtl = NULL;
		switch (ExtP.mID) {
		case IMG_EXTRA_PARAM_ID_MVFRAME_INFO:
			memcpy(&pCtrlMeta->common.mvinfo, &ExtP.mData, sizeof(mvframeinfo_t));
			break;

		case IMG_EXTRA_PARAM_ID_ME_INFO:
			memcpy(&pCtrlMeta->me_meta.me_setting, &ExtP.mData,
			       sizeof(me_ctrl_setting));
			pCtrlMeta->me_meta.BATCH_NUM = pParams->mNumBatchRun;
			break;
		case IMG_EXTRA_PARAM_ID_WPE_INFO:
			wpemode = ExtP.mData.mWPEInfo.wpe_mode;
			switch (ExtP.mData.mWPEInfo.wpe_mode) {
			case EWPE_HW_TNR:
				wpeCtl = &(pCtrlMeta->wpe_mdata[HW_WPE_TNR]);
				break;
			case EWPE_HW_LITE:
				wpeCtl = &(pCtrlMeta->wpe_mdata[HW_WPE_LITE]);
				break;
			default:
				wpeCtl = &(pCtrlMeta->wpe_mdata[HW_WPE_EIS]);
				break;
			}
			memcpy(wpeCtl, &ExtP.mData, sizeof(WPEInfo));
			break;
		case IMG_EXTRA_PARAM_ID_WPE_TNR_INFO:
			wpeCtl = &(pCtrlMeta->wpe_mdata[HW_WPE_TNR]);
			memcpy(wpeCtl, &ExtP.mData, sizeof(WPEInfo));
			break;
		case IMG_EXTRA_PARAM_ID_COST_LEVEL_INFO:
			CostLevel = ExtP.mData.mCostLevel.costlevel;
			break;
		case IMG_EXTRA_PARAM_ID_FE_INFO:
			memcpy(&pCtrlMeta->traw_mdata.FEInfo, &ExtP.mData, sizeof(FEInfo));
			break;
		case IMG_EXTRA_PARAM_ID_SRZ_INFO:
			if (IMG_SRZ_ID_FESRZ == ExtP.mData.mSRZInfo.srzId) {
				memcpy(&pCtrlMeta->traw_mdata.FESrz, &ExtP.mData, sizeof(SRZInfo));
			} else if (IMG_SRZ_ID_CNRSRZ == ExtP.mData.mSRZInfo.srzId) {
				memcpy(&pCtrlMeta->dip_mdata.bokeh_rz, &ExtP.mData, sizeof(SRZInfo));
			}
			break;
		case IMG_EXTRA_PARAM_ID_APL_INFO:
			pCtrlMeta->traw_mdata.ResizerInfo.DRZH2NT2_APL_EN = 0x1;
			break;
		case IMG_EXTRA_PARAM_ID_FM_INFO:
			memcpy(&pCtrlMeta->dip_mdata.fm_info, &ExtP.mData, sizeof(FMInfo));
			break;
		case IMG_EXTRA_PARAM_ID_DIP_MULTISCALE_INFO:
			memcpy(&pCtrlMeta->dip_mdata.multiscale_info, &ExtP.mData,
			       sizeof(MultiScaleInfo));
			break;
		case IMG_EXTRA_PARAM_ID_DIP_MUTLIFRAME_INFO:
			memcpy(&pCtrlMeta->dip_mdata.multiframe_info, &ExtP.mData,
			       sizeof(MultiFrameInfo));
			break;
		case IMG_EXTRA_PARAM_ID_P_IMG4O_CROP_INFO:
			pCtrlMeta->dip_mdata.is_p_img4o_crop_info_exist = MTRUE;
			memcpy(&pCtrlMeta->common.img4o_crp, &ExtP.mData,
			       sizeof(PImg4oCropInfo));
			break;
		case IMG_EXTRA_PARAM_ID_PQ_PORT_INFO:
			memcpy(&pCtrlMeta->pqdip_mdata.pqidxinfo, &ExtP.mData,
			       sizeof(PQPortInfo));
			break;
		default:
			break;
		}
	}

	// Handle ME Part

	// Handle WPE Part

	// Handle TRAW Part
	if ((pCtrlMeta->common.dl_table[HW_TRAW].on) ||
	    (pCtrlMeta->common.dl_table[HW_LTRAW].on) ||
	    (pCtrlMeta->common.dl_table[HW_XTRAW].on)) {
		pCtrlMeta->traw_mdata.StreamTag = NSCam::NSImgStream::TRAWScenarioMapping(
			(NSCam::NSImgStream::IMG_SCENARIO_PATH)pFrmParam.mScenPath);
		pCtrlMeta->traw_mdata.HWID = NSCam::NSImgStream::TRAWHWIDMapping(pFrmParam);
	}

	// Handle Dip Part
	if (pCtrlMeta->common.dl_table[HW_DIP].on) {
		pCtrlMeta->dip_mdata.streamtag = NSCam::NSImgStream::DIPScenarioMapping(
			(NSCam::NSImgStream::IMG_SCENARIO_PATH)pFrmParam.mScenPath);
	}

	if (pCtrlMeta->common.dl_table[HW_DIP].on &&
	    pCtrlMeta->common.dl_table[HW_WPE_TNR].on) {
		pCtrlMeta->dip_mdata.costlevel =
			(CostLevel < COST_LEVEL_MAX) ? CostLevel : COST_LEVEL_MAX;
	} else {
		pCtrlMeta->dip_mdata.costlevel = COST_LEVEL_ORI;
	}

	// Handle PQ-DIP
	if ((pCtrlMeta->common.dl_table[HW_PQDIP_A].on) ||
	    (pCtrlMeta->common.dl_table[HW_PQDIP_B].on)) {
		if (pCtrlMeta->common.dl_table[HW_WPE_EIS].on) {
			for (auto const &in : pFrmParam.mvIn) {
				PortIdx = (IMG_PORT)in.mPortIdx;
				switch (PortIdx) {
				case NSCam::NSImgStream::IMG_PORT_WPE_WPEI:
					pCtrlMeta->pqdip_mdata.inProfile =
						NSCam::NSImgStream::ColorSpaceMapping(
							(NSCam::EImageColorSpace)in.mBuffer->getColorSpace());
					break;
				default:
					break;
				}
			}
		} else if (pCtrlMeta->common.dl_table[HW_WPE_TNR].on) {
			for (auto const &in : pFrmParam.mvIn) {
				PortIdx = (IMG_PORT)in.mPortIdx;
				switch (PortIdx) {
				case NSCam::NSImgStream::IMG_PORT_WPE_TNR_WPEI:
					pCtrlMeta->pqdip_mdata.inProfile =
						NSCam::NSImgStream::ColorSpaceMapping(
							(NSCam::EImageColorSpace)in.mBuffer->getColorSpace());
					break;
				default:
					break;
				}
			}
		} else if (pCtrlMeta->common.dl_table[HW_TRAW].on) {
			for (auto const &in : pFrmParam.mvIn) {
				PortIdx = (IMG_PORT)in.mPortIdx;
				switch (PortIdx) {
				case NSCam::NSImgStream::IMG_PORT_TIMGI:
					pCtrlMeta->pqdip_mdata.inProfile =
						NSCam::NSImgStream::ColorSpaceMapping(
							(NSCam::EImageColorSpace)in.mBuffer->getColorSpace());
					break;
				default:
					break;
				}
			}
		} else if (pCtrlMeta->common.dl_table[HW_DIP].on) {
			for (auto const &in : pFrmParam.mvIn) {
				PortIdx = (IMG_PORT)in.mPortIdx;
				switch (PortIdx) {
				case NSCam::NSImgStream::IMG_PORT_IMGI:
					pCtrlMeta->pqdip_mdata.inProfile =
						NSCam::NSImgStream::ColorSpaceMapping(
							(NSCam::EImageColorSpace)in.mBuffer->getColorSpace());
					break;
				default:
					break;
				}
			}
		} else {
			for (auto const &in : pFrmParam.mvIn) {
				PortIdx = (IMG_PORT)in.mPortIdx;
				switch (PortIdx) {
				case NSCam::NSImgStream::IMG_PORT_PIMGI:
					pCtrlMeta->pqdip_mdata.inProfile =
						NSCam::NSImgStream::ColorSpaceMapping(
							(NSCam::EImageColorSpace)in.mBuffer->getColorSpace());
					break;
				default:
					break;
				}
			}
		}

		for (auto const &out : pFrmParam.mvOut) {
			PortIdx = (IMG_PORT)out.mPortIdx;
			switch (PortIdx) {
			case NSCam::NSImgStream::IMG_PORT_WDMAO:
				pCtrlMeta->pqdip_mdata.outProfile_a =
					NSCam::NSImgStream::ColorSpaceMapping(
						(NSCam::EImageColorSpace)out.mBuffer->getColorSpace());
				break;
			case NSCam::NSImgStream::IMG_PORT_WROTO:
				pCtrlMeta->pqdip_mdata.outProfile_b =
					NSCam::NSImgStream::ColorSpaceMapping(
						(NSCam::EImageColorSpace)out.mBuffer->getColorSpace());
				break;
			default:
				break;
			}
		}
	}

	// Handle UFO
	for (auto const& in : pFrmParam.mvIn) {
		PortIdx = (IMG_PORT)in.mPortIdx;
		switch (PortIdx) {
		case NSCam::NSImgStream::IMG_PORT_IMGI:
			if ((in.mBuffer->getImgFormat() == eImgFmt_UFBC_YUV_P010) ||
			    (in.mBuffer->getImgFormat() == eImgFmt_UFBC_NV12) ||
			    (in.mBuffer->getImgFormat() == eImgFmt_UFBC_YUV_P012)) {
			unsigned char *ufo_buf = reinterpret_cast<unsigned char*>(in.mBuffer->getBufVA(0));
			memcpy(&pCtrlMeta->dip_ufo_meta, ufo_buf, sizeof(YUFO_META_INFO));
		}
			break;
		case NSCam::NSImgStream::IMG_PORT_WPE_WPEI:
		case NSCam::NSImgStream::IMG_PORT_WPE_TNR_WPEI:
			if ((in.mBuffer->getImgFormat() == eImgFmt_UFBC_YUV_P010) ||
			(in.mBuffer->getImgFormat() == eImgFmt_UFBC_NV12) ||
			(in.mBuffer->getImgFormat() == eImgFmt_UFBC_YUV_P012)) {
			unsigned char *ufo_buf = reinterpret_cast<unsigned char*>(in.mBuffer->getBufVA(0));
			memcpy(&pCtrlMeta->wpe_ufo_meta, ufo_buf, sizeof(YUFO_META_INFO));
			}
			break;
		default:
			break;
		}
	}

	if (DeviceTuningEn > 0) {
		LOG_DBG(
			"pReqInfo->mpCMBuf->mOffset(%d), CtrlMetaOffset(%d), RequestFd(%d), "
			"wpe-eis on:%d fmt:0x%x wd:%d ht:%d\n"
			"wpe-tnr on:%d fmt:0x%x wd:%d ht:%d traw on:%d fmt:0x%x wd:%d ht:%d\n"
			"ltraw on:%d fmt:0x%x wd:%d ht:%d dip on:%d fmt:0x%x wd:%d ht:%d\n"
			"pqdip a on:%d fmt:0x%x wd:%d ht:%d\n"
			"pqdip b on:%d fmt:0x%x wd:%d ht:%d\n"
			"adl a on:%d fmt:0x%x wd:%d ht:%d\n"
			"adl b on:%d fmt:0x%x wd:%d ht:%d\n",
			pReqInfo->mpCMBuf->mOffset, CtrlMetaOffset,
			pReqInfo->mpRequest->GetRequestFD(),
			pCtrlMeta->common.dl_table[HW_WPE_EIS].on,
			pCtrlMeta->common.dl_table[HW_WPE_EIS].src_fmt,
			pCtrlMeta->common.dl_table[HW_WPE_EIS].src_wd,
			pCtrlMeta->common.dl_table[HW_WPE_EIS].src_ht,
			pCtrlMeta->common.dl_table[HW_WPE_TNR].on,
			pCtrlMeta->common.dl_table[HW_WPE_TNR].src_fmt,
			pCtrlMeta->common.dl_table[HW_WPE_TNR].src_wd,
			pCtrlMeta->common.dl_table[HW_WPE_TNR].src_ht,
			pCtrlMeta->common.dl_table[HW_TRAW].on,
			pCtrlMeta->common.dl_table[HW_TRAW].src_fmt,
			pCtrlMeta->common.dl_table[HW_TRAW].src_wd,
			pCtrlMeta->common.dl_table[HW_TRAW].src_ht,
			pCtrlMeta->common.dl_table[HW_LTRAW].on,
			pCtrlMeta->common.dl_table[HW_LTRAW].src_fmt,
			pCtrlMeta->common.dl_table[HW_LTRAW].src_wd,
			pCtrlMeta->common.dl_table[HW_LTRAW].src_ht,
			pCtrlMeta->common.dl_table[HW_DIP].on,
			pCtrlMeta->common.dl_table[HW_DIP].src_fmt,
			pCtrlMeta->common.dl_table[HW_DIP].src_wd,
			pCtrlMeta->common.dl_table[HW_DIP].src_ht,
			pCtrlMeta->common.dl_table[HW_PQDIP_A].on,
			pCtrlMeta->common.dl_table[HW_PQDIP_A].src_fmt,
			pCtrlMeta->common.dl_table[HW_PQDIP_A].src_wd,
			pCtrlMeta->common.dl_table[HW_PQDIP_A].src_ht,
			pCtrlMeta->common.dl_table[HW_PQDIP_B].on,
			pCtrlMeta->common.dl_table[HW_PQDIP_B].src_fmt,
			pCtrlMeta->common.dl_table[HW_PQDIP_B].src_wd,
			pCtrlMeta->common.dl_table[HW_PQDIP_B].src_ht,
			pCtrlMeta->common.dl_table[HW_ADL_A].on,
			pCtrlMeta->common.dl_table[HW_ADL_A].src_fmt,
			pCtrlMeta->common.dl_table[HW_ADL_A].src_wd,
			pCtrlMeta->common.dl_table[HW_ADL_A].src_ht,
			pCtrlMeta->common.dl_table[HW_ADL_B].on,
			pCtrlMeta->common.dl_table[HW_ADL_B].src_fmt,
			pCtrlMeta->common.dl_table[HW_ADL_B].src_wd,
			pCtrlMeta->common.dl_table[HW_ADL_B].src_ht);
	}

	pBufInfo->buf.planes[0].m.dma_buf.offset =
		pReqInfo->mpCMBuf->mOffset + CtrlMetaOffset;
	pBufInfo->fmt.plane_fmt[0].bytesperline =
		pReqInfo->mpCMBuf->mBufSize;
	pBufInfo->fmt.plane_fmt[0].sizeimage = pReqInfo->mpCMBuf->mBufSize;
	pBufInfo->buf.num_planes = 1; // imgi
	pBufInfo->fmt.width = pReqInfo->mpCMBuf->mBufSize;
	pBufInfo->fmt.height = 1;
	pBufInfo->fmt.pixelformat = V4L2_META_FMT_MTISP_PARAMS;
	pBufInfo->rotation = 0;
	pBufInfo->hflip = 0; // use hflip to flip
	pBufInfo->vflip = 0;

	// crop setting
	pBufInfo->crop.c.left = 0;
	pBufInfo->crop.c.top = 0;
	pBufInfo->crop.c.width = pReqInfo->mpCMBuf->mBufSize;
	pBufInfo->crop.c.height = 1;
	pBufInfo->crop.left_subpix.numerator = 0;
	pBufInfo->crop.top_subpix.numerator = 0;
	pBufInfo->crop.width_subpix.numerator = 0;
	pBufInfo->crop.height_subpix.numerator = 0;

	LOG_DBG(
		"Frm Sync- FrmNum:%d TotalFrm:%d, request_no:%d, frame_no:%d, stage:%d,"
		"request_fd:%d\n, "
		"needDump:%d, timestamp:0x%x, is_secureFrm:%d, frm_owner:%lld, "
		"is_early_cb:%d, fps:%d sync_prev:%d, sync_next:%d, syncid:%d"
		"frm_owner:%s username(%s), wpemode(%d)\n, "
		"Ctrl Meta - num_planes:%d, width:%d, height:%d, "
		"fmt:0x%x,rot:%d, "
		"hf:%d, vf:%d, ratio:%d\n, crop(%d_%d_%d_%d), "
		"fcrop(%d_%d_%d_%d)\nPlane[0] ofset:%d, stride:%d, "
		"bufsize:%d\nPlane[1] ofset:%d, stride:%d, "
		"bufsize:%d\nPlane[2] ofset:%d, stride:%d, "
		"bufsize:%d\n",
		frm, TotalFrm, pCtrlMeta->common.unique_key, pCtrlMeta->common.frame_no,
		pCtrlMeta->common.stage, pCtrlMeta->common.request_fd,
		pCtrlMeta->common.needDump, pCtrlMeta->common.timestamp,
		pCtrlMeta->common.is_secureFrm, pCtrlMeta->common.frm_owner,
		pCtrlMeta->common.is_early_cb, pCtrlMeta->common.fps,
		pCtrlMeta->common.sync_prev, pCtrlMeta->common.sync_next,
		pCtrlMeta->common.syncid, pFrmParam.mFrameOwner.c_str(), userid.c_str(),
		wpemode, pBufInfo->buf.num_planes, pBufInfo->fmt.width,
		pBufInfo->fmt.height, pBufInfo->fmt.pixelformat,
		pBufInfo->rotation, pBufInfo->hflip, pBufInfo->vflip,
		pBufInfo->resizeratio, pBufInfo->crop.c.left, pBufInfo->crop.c.top,
		pBufInfo->crop.c.width, pBufInfo->crop.c.height,
		pBufInfo->crop.left_subpix.numerator, pBufInfo->crop.top_subpix.numerator,
		pBufInfo->crop.width_subpix.numerator,
		pBufInfo->crop.height_subpix.numerator,
		pBufInfo->buf.planes[0].m.dma_buf.offset,
		pBufInfo->fmt.plane_fmt[0].bytesperline,
		pBufInfo->fmt.plane_fmt[0].sizeimage,
		pBufInfo->buf.planes[1].m.dma_buf.offset,
		pBufInfo->fmt.plane_fmt[1].bytesperline,
		pBufInfo->fmt.plane_fmt[1].sizeimage,
		pBufInfo->buf.planes[2].m.dma_buf.offset,
		pBufInfo->fmt.plane_fmt[2].bytesperline,
		pBufInfo->fmt.plane_fmt[2].sizeimage);

	LOG_DBG("Ctrl Meta End!!\n");
	return true;
}

bool DirectLinkTableUpdate(dltb_t *pdlTable,
			   const FrameParams &frmParams,
			   bool &bMEExist,
			   int frm)
{
	int i;
	IMG_PORT PortIdx, OutputIdx;
	unsigned int dl_head_fmt = 0x0, dl_head_wd = 0x0, dl_head_ht = 0x0;
	[[maybe_unused]] bool traw_tuning_enable = false, dip_tuning_enable = false,
			      pq_dip_tuning_enable = false;
	[[maybe_unused]] bool tdshp_p1a_enable = false, tdshp_p1b_enable = false;

	MBOOL bFoundHead = MFALSE, bUserDefined = MFALSE;
	MBOOL bWPEOExist = MFALSE, bWPEmode = MFALSE;
	NSCam::EImageFormat eXTimgiImgFmt = eImgFmt_UNKNOWN;
	if (pdlTable == NULL) {
		LOG_ERR("pdlTable can't be NULL!!\n");
		return false;
	}
	for (auto const &mIn : frmParams.mvIn) {
		PortIdx = (IMG_PORT)mIn.mPortIdx;
		if (mIn.mBuffer == NULL) {
			LOG_ERR("ImageBuffer is NULL in PortIdx(%d) in Input!!\n", PortIdx);
			return false;
		}
		switch (PortIdx) {
		case IMG_PORT_IMGI: {
			if (bFoundHead == MTRUE) {
				LOG_ERR("This frame have two hw input at the same time!!\n");
				return false;
			}
			bFoundHead = MTRUE;
			dl_head_wd = mIn.mBuffer->getImgSize().w;
			dl_head_ht = mIn.mBuffer->getImgSize().h;
			dl_head_fmt = NSCam::NSImgStream::ImgBufFmtMappingToV4L2Fmt(
				(NSCam::EImageFormat)mIn.mBuffer->getImgFormat(),
				mIn.mBuffer->getColorArrangement());
			break;
		}
		case IMG_PORT_TIMGI: {
			if (bFoundHead == MTRUE) {
				LOG_ERR("This frame have two hw input at the same time!!\n");
				return false;
			}
			bFoundHead = MTRUE;
			dl_head_wd = mIn.mBuffer->getImgSize().w;
			dl_head_ht = mIn.mBuffer->getImgSize().h;
			dl_head_fmt = NSCam::NSImgStream::ImgBufFmtMappingToV4L2Fmt(
				(NSCam::EImageFormat)mIn.mBuffer->getImgFormat(),
				mIn.mBuffer->getColorArrangement());
			break;
		}
		case IMG_PORT_LTIMGI: {
			if (bFoundHead == MTRUE) {
				LOG_ERR("This frame have two hw input at the same time!!\n");
				return false;
			}
			bFoundHead = MTRUE;
			dl_head_wd = mIn.mBuffer->getImgSize().w;
			dl_head_ht = mIn.mBuffer->getImgSize().h;
			dl_head_fmt = NSCam::NSImgStream::ImgBufFmtMappingToV4L2Fmt(
				(NSCam::EImageFormat)mIn.mBuffer->getImgFormat(),
				mIn.mBuffer->getColorArrangement());
			break;
		}
		case IMG_PORT_XTIMGI: {
			if (bFoundHead == MTRUE) {
				LOG_ERR("This frame have two hw input at the same time!!\n");
				return false;
			}
			bFoundHead = MTRUE;
			dl_head_wd = mIn.mBuffer->getImgSize().w;
			dl_head_ht = mIn.mBuffer->getImgSize().h;
			eXTimgiImgFmt = (NSCam::EImageFormat)mIn.mBuffer->getImgFormat();
			dl_head_fmt = NSCam::NSImgStream::ImgBufFmtMappingToV4L2Fmt(
				eXTimgiImgFmt, mIn.mBuffer->getColorArrangement());
			break;
		}
		case IMG_PORT_PIMGI: {
			if (bFoundHead == MTRUE) {
				LOG_ERR("This frame have two hw input at the same time!!\n");
				return false;
			}
			bFoundHead = MTRUE;
			dl_head_wd = mIn.mBuffer->getImgSize().w;
			dl_head_ht = mIn.mBuffer->getImgSize().h;
			dl_head_fmt = NSCam::NSImgStream::ImgBufFmtMappingToV4L2Fmt(
				(NSCam::EImageFormat)mIn.mBuffer->getImgFormat(),
				mIn.mBuffer->getColorArrangement());
			break;
		}
		case IMG_PORT_WPE_WPEI: {
			if (bFoundHead == MTRUE) {
				LOG_ERR("This frame have two hw input at the same time!!\n");
				return false;
			}
			bFoundHead = MTRUE;
			// Check
			for (auto const &mEP : frmParams.mvExtraParam) {
				if (mEP.mID == IMG_EXTRA_PARAM_ID_WPE_INFO) {
					if (!((mEP.mData.mWPEInfo.vgen_out.x_start_point == 0) &&
					      (mEP.mData.mWPEInfo.vgen_out.x_end_point == 0) &&
					      (mEP.mData.mWPEInfo.vgen_out.y_start_point == 0) &&
					      (mEP.mData.mWPEInfo.vgen_out.y_end_point == 0))) {
						dl_head_wd = mEP.mData.mWPEInfo.vgen_out.x_end_point -
							     mEP.mData.mWPEInfo.vgen_out.x_start_point + 1;
						dl_head_ht = mEP.mData.mWPEInfo.vgen_out.y_end_point -
							     mEP.mData.mWPEInfo.vgen_out.y_start_point + 1;
						bUserDefined = MTRUE;
					}
					switch (mEP.mData.mWPEInfo.wpe_mode) {
					case EWPE_HW_TNR:
						pdlTable[HW_WPE_TNR].on = 1;
						break;
					case EWPE_HW_LITE:
						pdlTable[HW_WPE_LITE].on = 1;
						break;
					default:
						pdlTable[HW_WPE_EIS].on = 1;
						break;
					}
					bWPEmode = MTRUE;
					break;
				}
			}
			if (bUserDefined == MFALSE) {
				// Checkout WPEO exist or not ?

				for (auto const &mOut : frmParams.mvOut) {
					OutputIdx = (IMG_PORT)mOut.mPortIdx;
					if (OutputIdx == IMG_PORT_WPE_WPEO) {
						bWPEOExist = MTRUE;
						dl_head_wd = mOut.mBuffer->getImgSize().w;
						dl_head_ht = mOut.mBuffer->getImgSize().h;
						break;
					}
				}
				if (bWPEOExist == MFALSE) {
					dl_head_wd = mIn.mBuffer->getImgSize().w;
					dl_head_ht = mIn.mBuffer->getImgSize().h;
				}
			}
			dl_head_fmt = NSCam::NSImgStream::ImgBufFmtMappingToV4L2Fmt(
				(NSCam::EImageFormat)mIn.mBuffer->getImgFormat(),
				mIn.mBuffer->getColorArrangement());
			break;
		}
		case IMG_PORT_ME_L0_IMG0I:
		case IMG_PORT_ME_L0_IMG1I:
		case IMG_PORT_ME_L1_IMG0I:
		case IMG_PORT_ME_L1_IMG1I:
		case IMG_PORT_ME_IMGSTATI:
		case IMG_PORT_ME_L0_RMVI:
		case IMG_PORT_ME_L1_RMVI: {
			bMEExist = MTRUE;
			break;
		}
		default: {
			break;
		}
		}

		switch (PortIdx) {
		case IMG_PORT_TIMGI:
			pdlTable[HW_TRAW].on = 1;
			break;

		case IMG_PORT_IMGSTATO:
			pdlTable[HW_TRAW].on = 1;
			break;

		case IMG_PORT_LTIMGI:
			pdlTable[HW_LTRAW].on = 1;
			break;

		case IMG_PORT_XTIMGI:
			pdlTable[HW_XTRAW].on = 1;
			break;

		case IMG_PORT_IMGI:
		case IMG_PORT_VIPI:
		case IMG_PORT_REC_DSI:
		case IMG_PORT_REC_DPI:
		case IMG_PORT_TNRSI:
		case IMG_PORT_TNRWI:
		case IMG_PORT_TNRMI:
		case IMG_PORT_TNRCI:
		case IMG_PORT_TNRLFDI:
		case IMG_PORT_TNRVBI:
		case IMG_PORT_CNR_BLURMAPI:
		case IMG_PORT_LFEOI:
		case IMG_PORT_RFEOI:
			pdlTable[HW_DIP].on = 1;
			break;

		case IMG_PORT_WPE_WPEI:
		case IMG_PORT_WPE_VECI:
		case IMG_PORT_WPE_PSP_COEFFI:
			if (!bWPEmode)
				pdlTable[HW_WPE_EIS].on = 1;
			break;

		case IMG_PORT_WPE_TNR_WPEI:
			if (pdlTable[HW_WPE_TNR].on) {
				LOG_ERR("WPE_TNR already on by port WPEI+EWPE_HW_TNR\n");
				return false;
			} else {
				pdlTable[HW_WPE_TNR].on = 1;
			}
			break;
		case IMG_PORT_WPE_TNR_VECI:
		case IMG_PORT_WPE_TNR_PSP_COEFFI:
			pdlTable[HW_WPE_TNR].on = 1;
			break;

		default:
			break;
		}
	}

	if (bFoundHead == MFALSE) {
		if (frmParams.mScenPath == IMG_SCENARIO_FM) {
			for (auto const &mIn : frmParams.mvIn) {
				PortIdx = (IMG_PORT)mIn.mPortIdx;
				switch (PortIdx) {
				case IMG_PORT_LFEOI: {
					bFoundHead = MTRUE;
					dl_head_wd = mIn.mBuffer->getImgSize().w;
					dl_head_ht = mIn.mBuffer->getImgSize().h;
					dl_head_fmt = NSCam::NSImgStream::ImgBufFmtMappingToV4L2Fmt(
						(NSCam::EImageFormat)mIn.mBuffer->getImgFormat(),
						mIn.mBuffer->getColorArrangement());
					break;
				default:
					break;
				}
				}
			}
		} else if (bMEExist == MTRUE) {
			//  ME only Case
			return true;
		} else {
			for (auto const &mIn : frmParams.mvIn) {
				PortIdx = (IMG_PORT)mIn.mPortIdx;
				switch (PortIdx) {
				case IMG_PORT_WPE_TNR_WPEI: {
					bFoundHead = MTRUE;
					bUserDefined = MFALSE;
					for (auto const &mEP : frmParams.mvExtraParam) {
						if (mEP.mID == IMG_EXTRA_PARAM_ID_WPE_TNR_INFO) {
							if (!((mEP.mData.mWPEInfo.vgen_out.x_start_point == 0) &&
							      (mEP.mData.mWPEInfo.vgen_out.x_end_point == 0) &&
							      (mEP.mData.mWPEInfo.vgen_out.y_start_point == 0) &&
							      (mEP.mData.mWPEInfo.vgen_out.y_end_point == 0))) {
								dl_head_wd = mEP.mData.mWPEInfo.vgen_out.x_end_point -
									     mEP.mData.mWPEInfo.vgen_out.x_start_point + 1;
								dl_head_ht = mEP.mData.mWPEInfo.vgen_out.y_end_point -
									     mEP.mData.mWPEInfo.vgen_out.y_start_point + 1;
								bUserDefined = MTRUE;
							}
							break;
						}
					}

					if (bUserDefined == MFALSE) {
						// Checkout WPEO exist or not ?
						for (auto const &mOut : frmParams.mvOut) {
							OutputIdx = (IMG_PORT)mOut.mPortIdx;
							if (OutputIdx == IMG_PORT_WPE_TNR_WPEO) {
								bWPEOExist = MTRUE;
								dl_head_wd = mOut.mBuffer->getImgSize().w;
								dl_head_ht = mOut.mBuffer->getImgSize().h;
								break;
							}
						}
						if (bWPEOExist == MFALSE) {
							dl_head_wd = mIn.mBuffer->getImgSize().w;
							dl_head_ht = mIn.mBuffer->getImgSize().h;
						}
					}
					dl_head_fmt = NSCam::NSImgStream::ImgBufFmtMappingToV4L2Fmt(
						(NSCam::EImageFormat)mIn.mBuffer->getImgFormat(),
						mIn.mBuffer->getColorArrangement());

					break;
				}
				default: {
					continue;
				}
				}
			}
		}
	}

	if (bFoundHead == MFALSE) {
		LOG_ERR("This frame(%d) doesn't have any dma input!! \n", frm);

		for (auto const &mIn : frmParams.mvIn) {
			PortIdx = (IMG_PORT)mIn.mPortIdx;
			if (mIn.mBuffer == NULL) {
				LOG_ERR("ImageBuffer is NULL in PortIdx(%d) in Input!!\n", PortIdx);
				return false;
			}
			LOG_ERR(
				"frm:%d ImageBuf In width:%d, height:%d , "
				"fmt:%d\n",
				frm, mIn.mBuffer->getImgSize().w, mIn.mBuffer->getImgSize().h,
				NSCam::NSImgStream::ImgBufFmtMappingToV4L2Fmt(
					(NSCam::EImageFormat)mIn.mBuffer->getImgFormat(),
					mIn.mBuffer->getColorArrangement()));
		}

		for (auto const &mOut : frmParams.mvOut) {
			PortIdx = (IMG_PORT)mOut.mPortIdx;
			if (mOut.mBuffer == NULL) {
				LOG_ERR("ImageBuffer is NULL in PortIdx(%d) in Output!!\n", PortIdx);
				return false;
			}
			LOG_ERR(
				"frm:%d ImageBuf Output width:%d, height:%d , "
				"fmt:%d\n",
				frm, mOut.mBuffer->getImgSize().w, mOut.mBuffer->getImgSize().h,
				NSCam::NSImgStream::ImgBufFmtMappingToV4L2Fmt(
					(NSCam::EImageFormat)mOut.mBuffer->getImgFormat(),
					mOut.mBuffer->getColorArrangement()));
		}

		return false;
	}

	if ((dl_head_wd == 0) || (dl_head_ht == 0) || (dl_head_fmt == 0)) {
		LOG_ERR(
			"The DL Head frame settings are wrong!! dl_head_wd:%d, dl_head_ht:%d , "
			"dl_head_fmt:%d\n",
			dl_head_wd, dl_head_ht, dl_head_fmt);
		return false;
	}

	if (pdlTable[HW_XTRAW].on == 1 /*&& (pApuAlgoInf == NULL)*/) {
		if ((eXTimgiImgFmt == eImgFmt_BAYER10) ||
		    (eXTimgiImgFmt == eImgFmt_BAYER12)) {
			pdlTable[HW_ADL_A].on = 1;
		}
	}
	for (auto const &mOut : frmParams.mvOut) {
		PortIdx = (IMG_PORT)mOut.mPortIdx;
		if (mOut.mBuffer == NULL) {
			LOG_ERR("ImageBuffer is NULL in PortIdx(%d) in Output!!\n", PortIdx);
			return false;
		}

		switch (PortIdx) {
		case IMG_PORT_TYUVO:
		case IMG_PORT_TYUV2O:
		case IMG_PORT_TYUV3O:
		case IMG_PORT_TYUV4O:
		case IMG_PORT_TYUV5O:
		case IMG_PORT_TIMGO:
		case IMG_PORT_FEO:
			pdlTable[HW_TRAW].on = 1;
			break;

		case IMG_PORT_LTYUV2O:
		case IMG_PORT_LTYUV3O:
		case IMG_PORT_LTYUV4O:
		case IMG_PORT_LTYUV5O:
			pdlTable[HW_LTRAW].on = 1;
			break;

		case IMG_PORT_XYUVO:
		case IMG_PORT_XTYUV2O:
		case IMG_PORT_XTYUV3O:
		case IMG_PORT_XTYUV4O:
		case IMG_PORT_XTYUV5O:
		case IMG_PORT_XTIMGO:
		case IMG_PORT_XTMEO:
		case IMG_PORT_XTFDO:
		case IMG_PORT_XTADLDBGO:
			pdlTable[HW_XTRAW].on = 1;
			break;
		case IMG_PORT_TNRSO:
		case IMG_PORT_TNRWO:
		case IMG_PORT_TNRMO:
		case IMG_PORT_IMG2O:
		case IMG_PORT_IMG3O:
		case IMG_PORT_IMG4O:
		case IMG_PORT_FMO:
			pdlTable[HW_DIP].on = 1;
			break;

		case IMG_PORT_WDMAO:
			pdlTable[HW_PQDIP_A].on = 1;
			break;

		case IMG_PORT_WROTO:
			pdlTable[HW_PQDIP_B].on = 1;
			break;

		case IMG_PORT_WPE_WPEO:
		case IMG_PORT_WPE_MSKO:
			if (!pdlTable[HW_WPE_EIS].on && !pdlTable[HW_WPE_TNR].on &&
			    !pdlTable[HW_WPE_LITE].on) {
				pdlTable[HW_WPE_EIS].on = 1;
			}
			break;

		case IMG_PORT_WPE_TNR_WPEO:
		case IMG_PORT_WPE_TNR_MSKO:
			pdlTable[HW_WPE_TNR].on = 1;
			break;

		default:
			break;
		}
	}

	// Handle Metai
	for (auto const &mIn : frmParams.mvIn) {
		PortIdx = (IMG_PORT)mIn.mPortIdx;
		switch (PortIdx) {
		case IMG_PORT_METAI: {
			mtk_img_uapi_meta_raw_stats_cfg *pMetaCfg =
				reinterpret_cast<mtk_img_uapi_meta_raw_stats_cfg *>(
					mIn.mBuffer->getBufVA(0));
			if (pMetaCfg != NULL) {
				if (pMetaCfg->prot.traw_tuning_enable == MTRUE) {
					traw_tuning_enable = true;
					if ((pdlTable[HW_TRAW].on == 0) && (pdlTable[HW_LTRAW].on == 0) &&
					    (pdlTable[HW_XTRAW].on == 0)) {
						pdlTable[HW_TRAW].on = 1;
					}
				}
				if (pMetaCfg->prot.dip_tuning_enable == MTRUE) {
					pdlTable[HW_DIP].on = 1;
					dip_tuning_enable = true;
				}
				pq_dip_tuning_enable = pMetaCfg->prot.pq_dip_tuning_enable;
				if ((pMetaCfg->prot.tdshp_p1a_enable == MTRUE) &&
				    (pMetaCfg->prot.tdshp_p1b_enable == MTRUE)) {
					tdshp_p1a_enable = true;
					tdshp_p1b_enable = true;
				}
				if ((pMetaCfg->prot.tdshp_p1a_enable == MTRUE) &&
				    (pMetaCfg->prot.tdshp_p1b_enable == MFALSE)) {
					tdshp_p1a_enable = true;
				}
				if ((pMetaCfg->prot.tdshp_p1a_enable == MFALSE) &&
				    (pMetaCfg->prot.tdshp_p1b_enable == MTRUE)) {
					tdshp_p1b_enable = true;
				}
			}
			break;
		}
		default:
			break;
		}
	}

	for (i = 0; i < HW_TDR_MAX; i++) {
		if (pdlTable[i].on) {
			pdlTable[i].src_fmt = dl_head_fmt;
			pdlTable[i].src_wd = dl_head_wd;
			pdlTable[i].src_ht = dl_head_ht;
		}
	}
	if (pdlTable[HW_ADL_B].on) {
		pdlTable[HW_ADL_B].src_fmt = dl_head_fmt;
		pdlTable[HW_ADL_B].src_wd = dl_head_wd >> 1;
		pdlTable[HW_ADL_B].src_ht = dl_head_ht >> 1;
	}

	LOG_DBG(
		"DL wpe-eis on:%d fmt:0x%x wd:%d ht:%d\n"
		"DL wpe-tnr on:%d fmt:0x%x wd:%d ht:%d\n"
		"DL lite-wpe on:%d fmt:0x%x wd:%d ht:%d\n"
		"DL traw on:%d fmt:0x%x wd:%d ht:%d\n"
		"DL ltraw on:%d fmt:0x%x wd:%d ht:%d\n"
		"DL xtraw on:%d fmt:0x%x wd:%d ht:%d\n"
		"DL dip on:%d fmt:0x%x wd:%d ht:%d\n"
		"DL pq dip a on:%d fmt:0x%x wd:%d ht:%d\n"
		"DL pq dip b on:%d fmt:0x%x wd:%d ht:%d\n"
		"DL adl a on:%d fmt:0x%x wd:%d ht:%d\n"
		"DL adl b on:%d fmt:0x%x wd:%d ht:%d\n"
		"tuing meta traw_tuning_enable:%d dip_tuning_enable:%d\n"
		"tuing meta pq_dip_tuning_enable:%d, tdshp_p1a_enable:%d, "
		"tdshp_p1b_enable:%d\n",
		pdlTable[HW_WPE_EIS].on, pdlTable[HW_WPE_EIS].src_fmt,
		pdlTable[HW_WPE_EIS].src_wd, pdlTable[HW_WPE_EIS].src_ht,
		pdlTable[HW_WPE_TNR].on, pdlTable[HW_WPE_TNR].src_fmt,
		pdlTable[HW_WPE_TNR].src_wd, pdlTable[HW_WPE_TNR].src_ht,
		pdlTable[HW_WPE_LITE].on, pdlTable[HW_WPE_LITE].src_fmt,
		pdlTable[HW_WPE_LITE].src_wd, pdlTable[HW_WPE_LITE].src_ht,
		pdlTable[HW_TRAW].on, pdlTable[HW_TRAW].src_fmt, pdlTable[HW_TRAW].src_wd,
		pdlTable[HW_TRAW].src_ht, pdlTable[HW_LTRAW].on,
		pdlTable[HW_LTRAW].src_fmt, pdlTable[HW_LTRAW].src_wd,
		pdlTable[HW_LTRAW].src_ht, pdlTable[HW_XTRAW].on,
		pdlTable[HW_XTRAW].src_fmt, pdlTable[HW_XTRAW].src_wd,
		pdlTable[HW_XTRAW].src_ht, pdlTable[HW_DIP].on, pdlTable[HW_DIP].src_fmt,
		pdlTable[HW_DIP].src_wd, pdlTable[HW_DIP].src_ht, pdlTable[HW_PQDIP_A].on,
		pdlTable[HW_PQDIP_A].src_fmt, pdlTable[HW_PQDIP_A].src_wd,
		pdlTable[HW_PQDIP_A].src_ht, pdlTable[HW_PQDIP_B].on,
		pdlTable[HW_PQDIP_B].src_fmt, pdlTable[HW_PQDIP_B].src_wd,
		pdlTable[HW_PQDIP_B].src_ht, pdlTable[HW_ADL_A].on,
		pdlTable[HW_ADL_A].src_fmt, pdlTable[HW_ADL_A].src_wd,
		pdlTable[HW_ADL_A].src_ht, pdlTable[HW_ADL_B].on,
		pdlTable[HW_ADL_B].src_fmt, pdlTable[HW_ADL_B].src_wd,
		pdlTable[HW_ADL_B].src_ht,

		traw_tuning_enable, dip_tuning_enable, pq_dip_tuning_enable,
		tdshp_p1a_enable, tdshp_p1b_enable);

	return true;
}

bool HandleInputPort(RequestInfo *pReqInfo,
		     [[maybe_unused]] V4L2_MODE v4l2_modesel,
		     int frm,
		     unsigned int totalfrm,
		     const FrameParams &frmParams,
		     void *pSingleDev,
		     [[maybe_unused]] dltb_t *pdlTable)
{
	uint32_t k = 0;
	IMG_PORT PortIdx;
	[[maybe_unused]] IMG_PORT ReMapPortIdx;
	struct header_desc_norm *desc_norm = NULL;
	singlenode_desc_norm *singledevice_desc_norm = NULL;
	imgsys_video_nodes_id VidoeNodeHwId;
	struct frameparams *pBufInfo = NULL;

	for (auto const &in : frmParams.mvIn) {
		PortIdx = (IMG_PORT)in.mPortIdx;
		ReMapPortIdx = NSCam::NSImgStream::ImgPortMapVideoNode(PortIdx);

		if (pReqInfo->mMemMode == MEMORY_MODE_NORMAL) {
			singledevice_desc_norm =
				reinterpret_cast<singlenode_desc_norm *>(pSingleDev);
			if (PortIdx != IMG_PORT_METAI) {
				if (!NSCam::NSImgStream::IMG_PORT_MAP_HW_VIDEONODE_ID(&VidoeNodeHwId,
										      PortIdx)) {
					LOG_ERR(
						"I can't find video node id by using specifix "
						"IMG_PORT(%d) in Input Port!!",
						PortIdx);
					return false;
				}
				singledevice_desc_norm->dmas_enable[VidoeNodeHwId][frm] = 1;
				desc_norm = &singledevice_desc_norm->dmas[VidoeNodeHwId];
				desc_norm->fparams_tnum = totalfrm;
				pBufInfo = &desc_norm->fparams[frm];

			} else {
				singledevice_desc_norm
					->dmas_enable[MTK_IMGSYS_VIDEO_NODE_ID_TUNING_OUT][frm] = 1;

				desc_norm =
					(struct header_desc_norm *)&singledevice_desc_norm->tuning_meta;
				desc_norm->fparams_tnum = totalfrm;
				pBufInfo = &desc_norm->fparams[frm];
			}
		}

		pBufInfo->buf.num_planes = in.mBuffer->getPlaneCount();
		for (k = 0; k < pBufInfo->buf.num_planes; k++) {
			pBufInfo->buf.planes[k].m.dma_buf.offset =
				in.mBuffer->getPlaneOffsetInBytes(k); // Byte as unit
			pBufInfo->fmt.plane_fmt[k].bytesperline =
				in.mBuffer->getBufStridesInBytes(k);

			if (pBufInfo->fmt.plane_fmt[k].bytesperline == 0) {
				LOG_ERR(
					"MW Input Stride is Zero! PortIndex:%d RequestFd:%d request_no:%d, "
					"frame_no:%d",
					PortIdx, pReqInfo->mpRequest->GetRequestFD(),
					pReqInfo->pParams->mRequestNo, pReqInfo->pParams->mFrameNo);
				AEE_ASSERT(HWMODULE_STRIDE_ISZERO,
					   "MW Input Stride is Zero!! Please check MW Input Setting!!");
			}

			pBufInfo->fmt.plane_fmt[k].sizeimage =
				in.mBuffer->getBufSizeInBytes(k);
		}

		// pBufInfo->buf.num_planes = in.mBuffer->getPlaneCount();
		pBufInfo->fmt.width = in.mBuffer->getImgSize().w;
		pBufInfo->fmt.height = in.mBuffer->getImgSize().h;

		if (pBufInfo->fmt.width == 0) {
			LOG_ERR(
				"MW Input Width is Zero! PortIndex:%d RequestFd:%d request_no:%d, "
				"frame_no:%d",
				PortIdx, pReqInfo->mpRequest->GetRequestFD(),
				pReqInfo->pParams->mRequestNo, pReqInfo->pParams->mFrameNo);
			AEE_ASSERT(HWMODULE_WIDTH_ISZERO,
				   "MW Input Width is Zero!! Please check MW Input Setting!!");
		}
		if (pBufInfo->fmt.height == 0) {
			LOG_ERR(
				"MW Input Height is Zero! PortIndex:%d RequestFd:%d request_no:%d, "
				"frame_no:%d",
				PortIdx, pReqInfo->mpRequest->GetRequestFD(),
				pReqInfo->pParams->mRequestNo, pReqInfo->pParams->mFrameNo);
			AEE_ASSERT(HWMODULE_HEIGHT_ISZERO,
				   "MW Input Height is Zero!! Please check MW Input Setting!!");
		}

		pBufInfo->fmt.pixelformat =
			NSCam::NSImgStream::ImgBufFmtMappingToV4L2Fmt(
				(NSCam::EImageFormat)in.mBuffer->getImgFormat(),
				(NSCam::EImageFormat)in.mBuffer->getColorArrangement());
		pBufInfo->rotation = 0;
		pBufInfo->hflip = 0; // use hflip to flip
		pBufInfo->vflip = 0;
		pBufInfo->resizeratio =
			NSCam::NSImgStream::ResizeRatioMapping(in.mResizeInfo.mResizeRatio);

		// crop setting
		pBufInfo->crop.c.left = in.mSrcCrop.CropX;
		pBufInfo->crop.c.top = in.mSrcCrop.CropY;
		pBufInfo->crop.c.width = in.mSrcCrop.CropW;
		pBufInfo->crop.c.height = in.mSrcCrop.CropH;
		pBufInfo->crop.left_subpix.numerator = in.mSrcCrop.CropFloatX;
		pBufInfo->crop.top_subpix.numerator = in.mSrcCrop.CropFloatY;
		pBufInfo->crop.width_subpix.numerator = in.mSrcCrop.CropFloatW;
		pBufInfo->crop.height_subpix.numerator = in.mSrcCrop.CropFloatH;

		LOG_DBG(
			"Input idx:%d FrmNum:%d, RequestFD:%d, num_planes:%d, width:%d, "
			"height:%d, "
			"fmt:0x%x,rot:%d, "
			"hf:%d, vf:%d, ratio:%d, crop(%d_%d_%d_%d), "
			"fcrop(%d_%d_%d_%d) cs:%d Plane[0] ofset:%d, stride:%d, "
			"bufsize:%d Plane[1] ofset:%d, stride:%d, "
			"bufsize:%d Plane[2] ofset:%d, stride:%d, "
			"bufsize:%d, oriidx:%d",
			(PortIdx - MTK_ISP_IMGSYS_NODE_ID_BASE), frm,
			pReqInfo->mpRequest->GetRequestFD(), pBufInfo->buf.num_planes,
			pBufInfo->fmt.width, pBufInfo->fmt.height,
			pBufInfo->fmt.pixelformat, pBufInfo->rotation,
			pBufInfo->hflip, pBufInfo->vflip, pBufInfo->resizeratio,
			pBufInfo->crop.c.left, pBufInfo->crop.c.top, pBufInfo->crop.c.width,
			pBufInfo->crop.c.height, pBufInfo->crop.left_subpix.numerator,
			pBufInfo->crop.top_subpix.numerator,
			pBufInfo->crop.width_subpix.numerator,
			pBufInfo->crop.height_subpix.numerator, in.mBuffer->getColorSpace(),
			pBufInfo->buf.planes[0].m.dma_buf.offset,
			pBufInfo->fmt.plane_fmt[0].bytesperline,
			pBufInfo->fmt.plane_fmt[0].sizeimage,
			pBufInfo->buf.planes[1].m.dma_buf.offset,
			pBufInfo->fmt.plane_fmt[1].bytesperline,
			pBufInfo->fmt.plane_fmt[1].sizeimage,
			pBufInfo->buf.planes[2].m.dma_buf.offset,
			pBufInfo->fmt.plane_fmt[2].bytesperline,
			pBufInfo->fmt.plane_fmt[2].sizeimage, PortIdx);
	}
	return true;
}

bool HandleOutputPort(RequestInfo *pReqInfo,
		      int frm,
		      unsigned int totalfrm,
		      const FrameParams &frmParams,
		      void *pSingleDev)
{
	int k = 0;
	IMG_PORT PortIdx;
	[[maybe_unused]] IMG_PORT ReMapPortIdx;
	struct header_desc_norm *desc_norm = NULL;
	singlenode_desc_norm *singledevice_desc_norm = NULL;
	imgsys_video_nodes_id VidoeNodeHwId;
	struct frameparams *pBufInfo = NULL;
	imgsysrotation ImgRot = imgsysrotation_0;
	imgsysflip ImgFlip = imgsysflip_off;

	for (auto const &out : frmParams.mvOut) {
		PortIdx = (IMG_PORT)out.mPortIdx;
		ReMapPortIdx = NSCam::NSImgStream::ImgPortMapVideoNode(PortIdx);
		if (!NSCam::NSImgStream::IMG_PORT_MAP_HW_VIDEONODE_ID(&VidoeNodeHwId,
								      PortIdx)) {
			LOG_ERR(
				"I can't find video node id by using specifix "
				"IMG_PORT(%d) in Output Port!!",
				PortIdx);
			return false;
		}
		if (pReqInfo->mMemMode == MEMORY_MODE_NORMAL) {
			singledevice_desc_norm =
				reinterpret_cast<singlenode_desc_norm *>(pSingleDev);
			singledevice_desc_norm->dmas_enable[VidoeNodeHwId][frm] = 1;

			desc_norm = (struct header_desc_norm *)&singledevice_desc_norm
					    ->dmas[VidoeNodeHwId];
			desc_norm->fparams_tnum = totalfrm;
			pBufInfo = &desc_norm->fparams[frm];
		}

		pBufInfo->buf.num_planes = out.mBuffer->getPlaneCount();
		for (k = 0; (uint32_t)k < pBufInfo->buf.num_planes; k++) {
			pBufInfo->buf.planes[k].m.dma_buf.offset =
				out.mBuffer->getPlaneOffsetInBytes(k); // Byte as unit
			pBufInfo->fmt.plane_fmt[k].bytesperline =
				out.mBuffer->getBufStridesInBytes(k);

			if (pBufInfo->fmt.plane_fmt[k].bytesperline == 0) {
				LOG_ERR(
					"MW Output Stride is Zero! PortIndex:%d RequestFd:%d "
					"request_no:%d, "
					"frame_no:%d",
					PortIdx, pReqInfo->mpRequest->GetRequestFD(),
					pReqInfo->pParams->mRequestNo, pReqInfo->pParams->mFrameNo);
				AEE_ASSERT(
					HWMODULE_STRIDE_ISZERO,
					"MW Output Stride is Zero!! Please check MW Output Setting!!");
			}
			pBufInfo->fmt.plane_fmt[k].sizeimage =
				out.mBuffer->getBufSizeInBytes(k);
		}

		// pBufInfo->buf.num_planes = out.mBuffer->getPlaneCount();
		pBufInfo->fmt.width = out.mBuffer->getImgSize().w;
		pBufInfo->fmt.height = out.mBuffer->getImgSize().h;
		if (pBufInfo->fmt.width == 0) {
			LOG_ERR(
				"MW Output Width is Zero! PortIndex:%d RequestFd:%d request_no:%d, "
				"frame_no:%d",
				PortIdx, pReqInfo->mpRequest->GetRequestFD(),
				pReqInfo->pParams->mRequestNo, pReqInfo->pParams->mFrameNo);
			AEE_ASSERT(HWMODULE_WIDTH_ISZERO,
				   "MW Output Width is Zero!! Please check MW Output Setting!!");
		}
		if (pBufInfo->fmt.height == 0) {
			LOG_ERR(
				"MW Output Height is Zero! PortIndex:%d RequestFd:%d request_no:%d, "
				"frame_no:%d",
				PortIdx, pReqInfo->mpRequest->GetRequestFD(),
				pReqInfo->pParams->mRequestNo, pReqInfo->pParams->mFrameNo);
			AEE_ASSERT(HWMODULE_HEIGHT_ISZERO,
				   "MW Output Height is Zero!! Please check MW Output Setting!!");
		}

		pBufInfo->fmt.pixelformat =
			NSCam::NSImgStream::ImgBufFmtMappingToV4L2Fmt(
				(NSCam::EImageFormat)out.mBuffer->getImgFormat(),
				(NSCam::EImageFormat)out.mBuffer->getColorArrangement());

		if (!NSCam::NSImgStream::TransformMapping(ImgRot, ImgFlip,
							  out.mTransform)) {
			LOG_ERR(
				"We can't find thie Transform setting in Output Port(%d) MW Must "
				"check it!!",
				out.mTransform);
			return false;
		}
		pBufInfo->rotation = (MUINT32)ImgRot;
		pBufInfo->hflip = ImgFlip; // use hflip to flip
		pBufInfo->vflip = 0;
		pBufInfo->resizeratio =
			NSCam::NSImgStream::ResizeRatioMapping(out.mResizeInfo.mResizeRatio);

		// crop setting
		pBufInfo->crop.c.left = out.mSrcCrop.CropX;
		pBufInfo->crop.c.top = out.mSrcCrop.CropY;
		pBufInfo->crop.c.width = out.mSrcCrop.CropW;
		pBufInfo->crop.c.height = out.mSrcCrop.CropH;
		pBufInfo->crop.left_subpix.numerator = out.mSrcCrop.CropFloatX;
		pBufInfo->crop.top_subpix.numerator = out.mSrcCrop.CropFloatY;
		pBufInfo->crop.width_subpix.numerator = out.mSrcCrop.CropFloatW;
		pBufInfo->crop.height_subpix.numerator = out.mSrcCrop.CropFloatH;

		LOG_DBG(
			"Output idx:%d FrmNum:%d, RequestFD:%d num_planes:%d, width:%d, "
			"height:%d, "
			"fmt:0x%x,rot:%d, "
			"hf:%d, vf:%d, ratio:%d\n, crop(%d_%d_%d_%d), "
			"fcrop(%d_%d_%d_%d) cs:%d Plane[0] ofset:%d, stride:%d, "
			"bufsize:%d Plane[1] ofset:%d, stride:%d, "
			"bufsize:%d Plane[2] ofset:%d, stride:%d, "
			"bufsize:%d, oriidx:%d",
			(PortIdx - MTK_ISP_IMGSYS_NODE_ID_BASE), frm,
			pReqInfo->mpRequest->GetRequestFD(), pBufInfo->buf.num_planes,
			pBufInfo->fmt.width, pBufInfo->fmt.height,
			pBufInfo->fmt.pixelformat, pBufInfo->rotation,
			pBufInfo->hflip, pBufInfo->vflip, pBufInfo->resizeratio,
			pBufInfo->crop.c.left, pBufInfo->crop.c.top, pBufInfo->crop.c.width,
			pBufInfo->crop.c.height, pBufInfo->crop.left_subpix.numerator,
			pBufInfo->crop.top_subpix.numerator,
			pBufInfo->crop.width_subpix.numerator,
			pBufInfo->crop.height_subpix.numerator, out.mBuffer->getColorSpace(),
			pBufInfo->buf.planes[0].m.dma_buf.offset,
			pBufInfo->fmt.plane_fmt[0].bytesperline,
			pBufInfo->fmt.plane_fmt[0].sizeimage,
			pBufInfo->buf.planes[1].m.dma_buf.offset,
			pBufInfo->fmt.plane_fmt[1].bytesperline,
			pBufInfo->fmt.plane_fmt[1].sizeimage,
			pBufInfo->buf.planes[2].m.dma_buf.offset,
			pBufInfo->fmt.plane_fmt[2].bytesperline,
			pBufInfo->fmt.plane_fmt[2].sizeimage, PortIdx);
	}
	return true;
}

bool createSingleDevBuffer(RequestInfo *pReqInfo,
			   ImgInitParam *pUserParam,
			   const EIGHTCC &userid,
			   V4L2_MODE v4l2_modesel,
			   VNDescBuf *pVNDescBuf)
{
	int i = 0;
	struct header_desc_norm *norm_desc = NULL;
	struct singlenode_desc_norm *singledevice_desc_norm = NULL;
	void *pSingleDev = NULL;
	struct ctrl_meta_t *pCtrlMeta =
		(struct ctrl_meta_t *)pReqInfo->mpCMBuf->mpBufVa;
	struct frameparams *pBufInfo = NULL;

	if (pCtrlMeta == NULL) {
		LOG_ERR("pCtrlMeta is NULL!!");
		return false;
	}

	MUINT32 CtrlMetaOffset = 0;
	MUINT32 CtrlMetaSize = sizeof(struct ctrl_meta_t);
	std::shared_ptr<const NSCam::NSImgStream::ImgParams> pParams =
		pReqInfo->pParams;
	MUINT32 TotalFrm = pParams->mvFrameParams.size();
	if (pReqInfo->mMemMode == MEMORY_MODE_NORMAL) {
		if ((TotalFrm > TMAX) || (TotalFrm == 0)) {
			LOG_ERR(
				"ERROR!! size(%d) of mvFrameParams exceed %d or small than 0 !! We "
				"can't support now\n",
				TotalFrm, TMAX);
			return false;
		}
	} else {
		if ((TotalFrm > TMAX) || (TotalFrm == 0)) {
			LOG_ERR(
				"ERROR!! size(%d) of mvFrameParams exceed %d or small than 0 !! We "
				"can't support now\n",
				TotalFrm, TMAX);
			return false;
		}
	}
	int frameNo = pParams->mRequestNo;
	bool doLog = (frameNo == 8 || frameNo == 9 || true) && false;

	memset(reinterpret_cast<char *>(pReqInfo->mpCMBuf->mpBufVa), 0,
	       TotalFrm * sizeof(struct ctrl_meta_t));

	if (doLog)
		LOG_ERR("### Queue Request FD(%d) user %s frame no %d total frames %u!!",
			pReqInfo->mpRequest->GetRequestFD(), userid.c_str(),
			pParams->mRequestNo, TotalFrm);

	switch (v4l2_modesel) {
	case V4L2_MODE_SIGNLE_DEVICE:
		if (pVNDescBuf != NULL) {
			if (pReqInfo->mMemMode == MEMORY_MODE_NORMAL) {
				singledevice_desc_norm =
					(struct singlenode_desc_norm *)(pVNDescBuf->mpDescBufVa);
				pSingleDev = reinterpret_cast<void *>(singledevice_desc_norm);
				if (singledevice_desc_norm == NULL) {
					LOG_ERR("singledevice_desc_norm is NULL!!");
					return false;
				}

				memset(singledevice_desc_norm->dmas_enable, 0x0,
				       sizeof(singledevice_desc_norm->dmas_enable));
				norm_desc =
					(struct header_desc_norm *)&singledevice_desc_norm->ctrl_meta;
				norm_desc->fparams_tnum = TotalFrm;
				memset(reinterpret_cast<char *>(norm_desc->fparams), 0,
				       TotalFrm * sizeof(struct frameparams));
			}

		} else {
			LOG_ERR("video node:single device buffer is NULL!!\n");
			return false;
		}
		break;
	case V4L2_MODE_STANDARD:
		break;
	default:
		break;
	}

	for (auto const &pFrmParam : pParams->mvFrameParams) {
		bool bMEExist = false;

		if (doLog) {
			LOG_ERR("### ----------------- %s[%d] [%s] --------------------",
				userid.c_str(), i, stageName((PEU_Stage)pFrmParam.mStage));
			dump(pFrmParam);
		}

		// Direct Link Table Update
		if (NSCam::NSImgStream::DirectLinkTableUpdate(pCtrlMeta->common.dl_table,
							      pFrmParam, bMEExist, i) == false) {
			LOG_ERR("DirectLinkTableUpdate is fail, please check the setting!!");
			return false;
		}

		if (pFrmParam.mvIn.size() == 0) {
			LOG_ERR("No Any Input in this frame:%d !! Please check your setting!!", i);
			return false;
		}
		if (pFrmParam.mvOut.size() == 0) {
			LOG_ERR("No Any Output in this frame:%d !! Please check your setting!!", i);
			return false;
		}

		if (!HandleInputPort(pReqInfo, v4l2_modesel, i, TotalFrm, pFrmParam,
				     pSingleDev, pCtrlMeta->common.dl_table)) {
			LOG_ERR("HandleInputPort fail!!");
			return false;
		}

		if (!HandleOutputPort(pReqInfo, i, TotalFrm, pFrmParam, pSingleDev)) {
			LOG_ERR("HandleOutputPort fail!!");
			return false;
		}

		if (pReqInfo->mMemMode == MEMORY_MODE_NORMAL) {
			pBufInfo = &norm_desc->fparams[i];
		}

		// Handle Per frame control meta
		if (!NSCam::NSImgStream::HandleCtrlMeta(
			    pReqInfo, pUserParam, userid, i, TotalFrm, pFrmParam,
			    pCtrlMeta, CtrlMetaOffset, pBufInfo, m_DeviceTuningEn, bMEExist)) {
			LOG_ERR("HandleCtrlMeta fail!!");
			return false;
		}

		CtrlMetaOffset = CtrlMetaOffset + CtrlMetaSize;
		pCtrlMeta =
			reinterpret_cast<ctrl_meta_t *>((MUINTPTR)pCtrlMeta + CtrlMetaSize);
		i++;
	}

	return true;
}

uint32_t getV4L2Fmt(MINT eImgFmtBuf, MUINT32 colorArrangement)
{
	return ImgBufFmtMappingToV4L2Fmt((NSCam::EImageFormat)eImgFmtBuf, colorArrangement);
}

} // namespace NSImgStream
} // namespace NSCam

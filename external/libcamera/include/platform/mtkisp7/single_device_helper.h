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

#include <array>
#include <cstddef>
#include <stdint.h>

#include "linux/mtkisp7/drv/7.1/common.h"
#include "linux/mtkisp7/drv/7.1/hw_definition.h"
#include "mtkcam-halif/def/BuiltinTypes.h"
#include "mtkcam-halif/def/UITypes.h"
#include "mtkcam-interfaces/def/ImageFormat.h"

#include "IImgStreamDef.h"
#include "ImgPortDef.h"
#include "eightcc.h"

enum PEU_Stage {
	UNKNOWN = 0,

	/* MCNR stages */
	HW_LTR_ME_L1,
	HW_ME_3PASS_MODE_0,
	HW_ME_3PASS_MODE_1,
	HW_TR_F1,
	HW_TR_F4,
	HW_TR_HWMVP,
	HW_TR_CONF4,
	HW_TR_CONF5,
	HW_LTR_F1,
	HW_LTR_F4,
	HW_LTR_VBI,
	HW_WPE_W_F1,
	HW_WPE_W_F2,
	HW_WPE_W_F3,
	HW_WPE_W_F4,
	HW_WPE_W_F5,
	HW_WPE_W_F0,
	HW_DIP_IDI,
	HW_DIP_IDI2,
	HW_DIP_F4,
	HW_DIP_F3,
	HW_DIP_F2,
	HW_DIP_F1,
	HW_DIP_F0,

	/* LPNR stages */
	TR_R2Y,
	P2_MS_F3,
	P2_MS_F2,
	P2_MS_F1,
	P2_MS_F0_PQ_DIP,
	P2_MS_F0_H,
	P2_Y2Y_PQ_DIP,

	/* MFNR stages*/
	BFBLD_BASE,
	BFBLD_REF,
	BFME,
	MCDS_F1,
	DS,
	DS_VBI_V2,
	DS_VBI_V5,
	MSBLD_F0,
	MSBLD_F1,
	MSBLD_F2,
	MSBLD_F3,
	MSBLD_F4,
	MSBLD_F5,
	MSBLD_F6,
	AFBLD_F0,
	AFBLD_F1,
	AFBLD_F2,
	AFBLD_F3,
	AFBLD_F4,
	AFBLD_F5,
	AFBLD_F6
};

struct CtrlMetaBuf {
	MINT32 mFd;
	MUINT32 mOffset;
	MINT32 mBufSize;
	MINTPTR mpBufVa;
	intptr_t mpBufPa;
};

class IImageBuffer;

class MediaRequest
{
public:
	MediaRequest(int fd)
		: fd_(fd) {}
	int GetRequestFD() { return fd_; }
	int fd_;
};

struct RequestInfo {
	MediaRequest *mpRequest;
	CtrlMetaBuf *mpCMBuf;
	struct timeval enque_time;
	EIGHTCC mImgStreamOwner;
	MEMORY_MODE mMemMode;
	std::shared_ptr<const NSCam::NSImgStream::ImgParams> pParams;
};

// Not used, but in the arguments as a pointer
class VNDescBuf
{
public:
	MINT32 mFd;
	MINT32 mBufSize;
	MUINT32 mOffset;
	MINTPTR mpDescBufVa;
	MBOOL mbUsed;
};

enum {
	SENSOR_FORMAT_ORDER_RAW_B = 0x0,
	SENSOR_FORMAT_ORDER_RAW_Gb,
	SENSOR_FORMAT_ORDER_RAW_Gr,
	SENSOR_FORMAT_ORDER_RAW_R,
	SENSOR_FORMAT_ORDER_UYVY,
	SENSOR_FORMAT_ORDER_VYUY,
	SENSOR_FORMAT_ORDER_YUYV,
	SENSOR_FORMAT_ORDER_YVYU,
	SENSOR_FORMAT_ORDER_MONO,
	SENSOR_FORMAT_ORDER_NONE = 0xFF,
};

namespace NSCam {
namespace NSImgStream {

bool createSingleDevBuffer(RequestInfo *pReqInfo, ImgInitParam *pUserParam,
			   const EIGHTCC &userid, V4L2_MODE v4l2_modesel,
			   VNDescBuf *pVNDescBuf);

enum CacheCtrl {
	/** Flush CPU cache data to DRAM. */
	eCACHECTRL_FLUSH = 0,

	/** Invalidate cache value. */
	eCACHECTRL_INVALID = 1
};

struct BufferPlane {
	int fd;
	size_t offset;
	intptr_t va;
	size_t size;
	size_t stride;
	size_t scanline;
};

struct BufferProperty {
	int width;
	int height;

	EImageFormat format;
	int32_t ColorArrangeMent;
	int32_t colorSpace;

	int32_t numPlanes;
	std::array<BufferPlane, 3> planes;
};

/**
 *  Image Buffer implementation version.
 */
class IImageBuffer
{
public:
	/// Instantiation is disallowed.
	IImageBuffer(const BufferProperty &property);
	IImageBuffer() = default;

	/// Disallowed to directly delete a raw pointer.
	~IImageBuffer() = default;

	/// Image Attributes.
public:
	/**
   * Obtain image format of image buffer.
   *  @return The image format in `Emtkcam-interfaces/def/ImageFormat`.
   *  @see enum NSCam::Emtkcam-interfaces/def/ImageFormat.
   */
	MINT getImgFormat() const;

	/**
   * Obtain size of image buffer.
   *  @return The image size in pixel.
   */
	MSize const getImgSize() const;

	/**
   * Obtain number of planes.
   *  @return The plane count.
   */
	size_t getPlaneCount() const;

	/**
   * API to obtain the information set from `setColorArrangement`.
   *  @return MINT32 Value sets from `setColorArrangement`, default is 0.
   *  @sa NSCam::ESensorColorArrangement
   */
	MINT32 getColorArrangement() const;

	/**
   * Get the YUV color space definition. The information obtained from this
   * API might be set from IImageBufferHeap::setColorSpace. Basically, it
   * might be `eImgColorSpace_UNKNOWN`.
   *  @return The YUV color space.
   *  @sa EImageColorSpace
   */
	MINT32 getColorSpace() const;

public:
	/**
   * @copydoc getFD
   */
	MINT32 getPlaneFD(size_t index = 0) const;

	/**
   * Get the plane offset in bytes, calculated by:
   *  @code
   *    ADDR(plane) = ADDR(FD) + getPlaneOffsetInBytes(plane)
   *  @endcode
   *  @param index The plane index.
   *  @return Offset in bytes.
   */
	size_t getPlaneOffsetInBytes(size_t index) const;

	/**
   * Get the buffer VA of the given plane.
   *  @param index The plane index.
   *  @return Buffer virtual address of a given plane, `0` if failed.
   *  @note Legal only after lockBuf() with a SW usage.
   */
	MINTPTR getBufVA(size_t index) const;

	/**
   * Get the buffer size in bytes of the given plane including horizontal
   * and vertical paddings.
   *  @param index The plane index.
   *  @return Buffer size in bytes of a given plane, always legal.
   */
	size_t getBufSizeInBytes(size_t index) const;

	/**
   * Get the plane stride in bytes.
   *  @param index Index that indicates plane number
   *  @return Buffer Strides in bytes of a given plane, always legal.
   */
	size_t getBufStridesInBytes(size_t index) const;

	/**
   * Get the plane scan line count, some image buffer may need vertical padding,
   * the relationship between height and scanlines are displayed as:
   * @code
   *          ^ +-------------+--+ ^
   *          | |+++++++++++++|  | |
   *    height| |++  image  ++|  | |scanlines
   *          | |+++++++++++++|  | |
   *          | |+++++++++++++|  | |
   *          v +-------------+  | |
   *            |                | |
   *            +----------------+ v
   * @endcode
   * @param index Index that indicates plane number
   * @return Buffer scanlines of a given plane; always legal.
   */
	size_t getBufScanlines(size_t index) const;

	/**
   * Return a buffer type that indicates the buffer access permission.
   *  @return The buffer access permission enumeration.
   *  @sa enum NSCam::SecType
   */
	SecType getSecType() const;

	BufferProperty property;
};

uint32_t getV4L2Fmt(MINT eImgFmtBuf, MUINT32 colorArrangement);

} // namespace NSImgStream
} // namespace NSCam

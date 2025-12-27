/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * single_device.h - MtkISP7 ImgSys single device wrapper
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <stdint.h>
#include <vector>

#include "libcamera/internal/info_frame.h"
#include "libcamera/internal/v4l2_pixelformat.h"

#include "libcamera/framebuffer.h"
#include "libcamera/pixel_format.h"
#include "linux/mtkisp7/drv/7.1/common.h"
#include "linux/mtkisp7/drv/7.1/hw_definition.h"
#include "mtkcam-halif/def/BuiltinTypes.h"
#include "mtkcam-halif/def/ImageFormat.h"
#include "mtkcam-halif/def/UITypes.h"
#include "platform/mtkisp7/IImgStreamDef.h"
#include "platform/mtkisp7/ImgPortDef.h"
#include "platform/mtkisp7/eightcc.h"
#include "platform/mtkisp7/single_device_helper.h"

#define V4L2_STANDARD_MODE true

libcamera::V4L2PixelFormat getImgSysV4L2PixelFormat(const libcamera::PixelFormat &fmt);
NSCam::NSImgStream::BufferProperty toBufferPropery(const libcamera::InfoFrame &info);

namespace libcamera {

struct PortInfoEx {
	void set(const InfoFrame &info, uint32_t idx, int ratio, Size size)
	{
		set(info, idx, ratio, Rectangle(size));
	}

	void set(const InfoFrame &info, uint32_t idx, int ratio, Rectangle crop)
	{
		img.property = toBufferPropery(info);
		frameBuffer = info.buffer();

		portIdx = idx;
		mResizeRatio = ratio;
		CropX = crop.x;
		CropY = crop.y;
		CropW = crop.width;
		CropH = crop.height;
		CropFloatX = 0;
		CropFloatY = 0;
		CropFloatW = 0;
		CropFloatH = 0;
	}

	NSCam::NSImgStream::IImageBuffer img;
	FrameBuffer *frameBuffer;

	uint32_t portIdx;
	int mResizeRatio;
	int CropX; //! X integer start position for cropping
	int CropY; //! Y integer start position for crpping
	int CropW; //! width integer of cropped image
	int CropH; //! height integer of cropped image
	int CropFloatX; //! X float start position for cropping
	int CropFloatY; //! Y float start position for cropping
	int CropFloatW; //! width float of cropped image
	int CropFloatH; //! height float of cropped image
};

class StageEx
{
public:
	StageEx(PEU_Stage stageEnum) { stageEnum_ = stageEnum; }
	~StageEx() = default;

	void input(const InfoFrame &info, uint32_t idx, int ratio, const Rectangle &crop);
	void input(const InfoFrame &info, uint32_t idx, int ratio, const Size &size)
	{
		input(info, idx, ratio, Rectangle{ size });
	}

	void output(const InfoFrame &info, uint32_t idx, int ratio, const Rectangle &crop);
	void output(const InfoFrame &info, uint32_t idx, int ratio, const Size &size)
	{
		output(info, idx, ratio, Rectangle{ size });
	}

	void setMcdsF1WpeInfo(NSCam::NSImgStream::IMG_EXTRA_PARAM_ID id,
			      Size crop, NSCam::NSImgStream::WPE_MODE mode);
	void setWpeInfo(NSCam::NSImgStream::IMG_EXTRA_PARAM_ID id,
			Size crop, NSCam::NSImgStream::WPE_MODE mode,
			unsigned int featureIndex);

	void setMvFrame(Size f0, Size me, uint32_t scaleRatio = 4);
	void setMeInfo(NSCam::NSImgStream::ME_MODE mode);
	void setAplInfo();
	void setMultiScale(NSCam::NSImgStream::IMG_MULTI_SCALE_RATIO ratio,
			   uint32_t index, uint32_t total);
	void setPqInfo();
	void setImg4oCrop(const Rectangle &crop);
	void setCostLevel();

	void addNotify(uint32_t sync);
	void addWait(uint32_t sync);

	PEU_Stage getStageEnum() const { return stageEnum_; }

	const std::vector<PortInfoEx> &getInputs()
	{
		return inputs_;
	}

	const std::vector<PortInfoEx> &getOutputs()
	{
		return outputs_;
	}

private:
	friend class SingleDeviceRequest;

	std::vector<PortInfoEx> inputs_;
	std::vector<PortInfoEx> outputs_;
	std::vector<NSCam::NSImgStream::ImgExtraParam> extra_;

	std::list<MUINT32> mSyncTokenNotifyList;
	std::list<MUINT32> mSyncTokenWaitList;

	PEU_Stage stageEnum_;
};

class SingleDeviceRequest
{
public:
	StageEx &emplaceStage(PEU_Stage stageEnum, int frameNumber, int layer)
	{
		layer_.emplace_back(layer);
		frameNumber_.emplace_back(frameNumber);
		return stages_.emplace_back(stageEnum);
	}
	StageEx &emplaceStage(PEU_Stage stageEnum, int frameNumber)
	{
		return emplaceStage(stageEnum, frameNumber, -1);
	}
	StageEx &emplaceStage(PEU_Stage stageEnum)
	{
		return emplaceStage(stageEnum, sequence(), -1);
	}
	std::vector<StageEx> &Stages() { return stages_; }

	void init(uint32_t sequence, uint32_t timestamp, const std::string &id)
	{
		sequence_ = sequence;
		timestamp_ = timestamp;
		id_ = id;
	}

	void setSequence(uint32_t sequence) { sequence_ = sequence; }
	uint32_t sequence() { return sequence_; }
	uint32_t frameNumber(int i) { return frameNumber_[i]; }
	int layer(int i) { return layer_[i]; }

	void setTimestamp(uint32_t timestamp) { timestamp_ = timestamp; }
	uint32_t timestamp() { return timestamp_; }

	void setUserId(const std::string &id) { id_ = id; }
	const std::string &id() { return id_; }

	void fillRequestBufferForStage(const InfoFrame &infoCtrl, int requestFd, size_t stage);
	void fillRequestBuffer(const InfoFrame &infoCtrl, const InfoFrame &infoDesc, int requestFd);

	std::vector<PEU_Stage> getStageEnums() const;

private:
	void fillFrameParams(
		NSCam::NSImgStream::FrameParams &FrameParams, StageEx &stage);

	uint32_t sequence_;
	uint32_t timestamp_;
	std::string id_;
	std::vector<int> layer_;
	std::vector<int> frameNumber_;
	std::vector<StageEx> stages_;
};

} // namespace libcamera

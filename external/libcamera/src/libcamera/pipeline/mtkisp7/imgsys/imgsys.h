/*
 * Copyright (C) 2023, Google Inc.
 *
 * imgsys.h - MtkISP7 ImgSys device
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "libcamera/internal/info_frame.h"
#include "libcamera/internal/task_scheduler.h"
#include "libcamera/internal/v4l2_subdevice.h"
#include "libcamera/internal/v4l2_videodevice.h"

#include "libcamera/base/class.h"
#include "pipeline/mtkisp7/odt/on_device_tuner.h"

#include "ImgPortDef.h"
#include "single_device.h"
#include "single_device_helper.h"

namespace libcamera {

class DmaHeap;
class PipelineHandler;
class ImgSysDevice;

// TODO: Re-design the ImgSysBufferCache and ImgsysVideoDevice to merge
// the features introduced here to V4L2VideoDevice.
class ImgSysBufferCache : public V4L2BufferCache
{
public:
	~ImgSysBufferCache();

	bool isEmpty() const override;
	int get(const FrameBuffer &buffer, uint32_t userId) override;
	void put(unsigned int offsetedIdx) override;

	void setFormat(const V4L2DeviceFormat &format)
	{
		currentFormat_ = format;
	}

	bool formatReady(V4L2DeviceFormat &fmt, uint32_t userId);
	void addCurrentFormat(size_t count, uint64_t offset);
	void addFormat(V4L2DeviceFormat &fmt, uint32_t userId, size_t count,
		       uint64_t offset, bool dynamic = false);

	struct FormatCache {
	public:
		FormatCache(const V4L2DeviceFormat &format,
			    uint32_t userId, size_t count, uint64_t offset,
			    bool dynamic);
		~FormatCache();

		V4L2DeviceFormat format_;
		uint32_t userId_;
		uint64_t count_;
		uint64_t offset_;
		SimpleV4L2BufferCache *cache_;
		bool dynamic_ = false;
	};

	std::optional<uint64_t> getFormatIdx(const V4L2DeviceFormat &fmt, uint32_t userId);

	bool noCheckUserId_ = false;
	V4L2DeviceFormat currentFormat_;
	std::vector<FormatCache *> formatCaches_;
};

class ImgsysVideoDevice : public V4L2VideoDevice
{
public:
	explicit ImgsysVideoDevice(const MediaEntity *entity);
	int open();
	int configure(V4L2DeviceFormat *fmt, int resizeRatio, Rectangle crop,
		      bool forceResetCrop);
	int allocateBuffers(
		unsigned int count,
		std::vector<std::unique_ptr<FrameBuffer>> *buffers) = delete;
	int exportBuffers(
		unsigned int count,
		std::vector<std::unique_ptr<FrameBuffer>> *buffers) = delete;
	int importBuffers(unsigned int count);
	int importBuffersWithFormat(uint32_t userId, unsigned int count,
				    V4L2DeviceFormat *fmt, bool dynamic);
	int releaseBuffers();
	int setFormat(V4L2DeviceFormat *format);
	int getFormat(V4L2DeviceFormat *format);

	void resetBuffers(uint32_t userId);

private:
	int v4l2CreateBuffers(unsigned int count, V4L2DeviceFormat *format,
			      uint32_t *output_index, uint32_t *output_count);
	int resizeRatio_;
	Rectangle crop_;
	ImgSysBufferCache *getCache();
};

class ImgSysDevice
{
public:
	static Rectangle getCrop(Size inSize, Size outSize);
	static Rectangle cropNoisyBorder(const Rectangle &rect);

	const static uint32_t kUserIdMcnr = 1;
	const static uint32_t kUserIdLpnr = 2;
	const static uint32_t kUserIdMfnr = 3;

	enum FdCtrl {
		Add = 0,
		Delete,
	};

	struct Request {
		SingleDeviceRequest *sdRequest;
		size_t stage;
		int buffers_count;
		uint32_t userId;
		int frameNumber;
		uint32_t layer;
	};

	ImgSysDevice(OnDeviceTuner *odt);

	int init(MediaDevice *media, DmaHeap *dmaHeap);
	int configure(const Size sensorFullSize, const Size CamSysYuv,
		      const Size video1, const Size video2,
		      const Size still1, const Size still2,
		      const Size wrappingMapSize, const Size confMapSize);
	int start();
	int stop();

	int queueRequest(Request *request);
	int queueRequestV4L2(Request *request);
	int claimCompletedRequest(Request *request);

	int handleIova(FdCtrl fdHandle, InfoFramePool &pool);
	int handleKva(FdCtrl fdHandle, InfoFramePool &pool);

	int releaseAllBuffers();

	Signal<Request *> requestCompleted;

	TokenPool &syncPool() { return syncPool_; }

	void resetBuffers(uint32_t userId);

private:
	// 1 format x 1 port
	struct PortBuffers {
		NSCam::NSImgStream::IMG_PORT port;
		PixelFormat pixelFmt;
		Size size;
		size_t count;
		uint32_t strideAlign;
		uint32_t scanAlign;
		bool dynamic = false;
	};
	friend class ImgSysRequestHelper;

	void bufferReady(std::pair<FrameBuffer *, int> pair);

	int importBuffers(const Size sensorFullSize, const Size CamSysYuv,
			  const Size video1, const Size video2,
			  const Size still1, const Size still2,
			  const Size wrappingMapSize, const Size confMapSize);

	void importBufferByList(std::vector<PortBuffers> &portBufs, uint32_t userId);

	V4L2VideoDevice *sigdevNorm_;
	ImgsysVideoDevice *ctrlMeta_;
	std::unique_ptr<V4L2Subdevice> mtkIspDip_;
	std::unordered_map<
		NSCam::NSImgStream::IMG_PORT,
		std::unique_ptr<ImgsysVideoDevice>>
		allVideoDevices_;

	InfoFramePool descPool_;
	InfoFramePool ctrlMetaPool_;

	TokenPool syncPool_;

	Pool<int, UniqueFD> mediaRequestPool_;

	struct PendingRequest {
		Request *request;
		int mediaRequest;
		uint32_t internalRequestId;
		std::vector<PEU_Stage> stages;
		SharedMailBox<InfoFrame> ctrlMeta;
		SharedMailBox<InfoFrame> singleDevNorm;
	};

	std::list<PendingRequest> pendingRequests_;
	std::list<Request *> completedRequests_;

	MediaDevice *media_;
	DmaHeap *dmaHeap_;
	OnDeviceTuner *onDeviceTuner_;
};

class ImgSysRequestHelper
{
public:
	ImgSysRequestHelper(Task *task, Request *request, ImgSysDevice *imgSys)
		: task_(task), request_(request), imgSys_(imgSys)
	{
	}

	void queueRequest(uint32_t userId, SingleDeviceRequest &sdRequest);
	void requestReady(ImgSysDevice::Request *request);

	Task *task_;
	Request *request_;
	ImgSysDevice *imgSys_;

	std::list<ImgSysDevice::Request> imgSysRequests_;
	std::chrono::steady_clock::time_point startTime_;
};

} /* namespace libcamera */

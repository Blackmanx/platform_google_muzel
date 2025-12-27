/*
 * Copyright (C) 2023, Google Inc.
 *
 * info_frame.h - InfoFrame and InfoFramePool
 */

#pragma once

#include <unordered_map>
#include <vector>

#include "libcamera/internal/dma_heaps.h"
#include "libcamera/internal/mailbox.h"
#include "libcamera/internal/pools.h"

#include "libcamera/framebuffer.h"
#include "libcamera/geometry.h"
#include "libcamera/pixel_format.h"

namespace libcamera {

class InfoFrame
{
public:
	struct Plane {
		uint8_t *address;
	};

	InfoFrame();
	InfoFrame(const PixelFormat &format, const Size &size, FrameBuffer *buffers,
		  unsigned int strideAlign = 1, unsigned int scanAlign = 1);

	void setAddress(unsigned int plane, uint8_t *address);
	uint8_t *address(unsigned int plane) const;

	Size size() const { return size_; }
	PixelFormat format() const { return format_; }
	FrameBuffer *buffer() const { return buffer_; }
	unsigned int numPlanes() const { return numPlanes_; }
	unsigned int strideAlign() const { return strideAlign_; }
	unsigned int scanAlign() const { return scanAlign_; }

private:
	Size size_;
	PixelFormat format_;
	FrameBuffer *buffer_ = nullptr;

	unsigned int numPlanes_;
	std::array<Plane, 3> planes_;
	unsigned int strideAlign_ = 1;
	unsigned int scanAlign_ = 1;
};

class InfoFramePool
{
public:
	struct MappedBufferInfo {
		uint8_t *address = nullptr;
		size_t dmabufLength = 0;
	};

	InfoFramePool();
	~InfoFramePool();

	int createBuffers(DmaHeap *dmaHeap, const PixelFormat &format,
			  const Size &size, uint32_t count,
			  DmaHeap::Type type = DmaHeap::System,
			  unsigned int strideAlign = 1, unsigned scanAlign = 1);

	void release();

	void fetch(SharedMailBox<InfoFrame> &mailBox);

	InfoFrame get();
	void put(InfoFrame &frameInfo);

	int mmap();
	int unmap();

	std::vector<int> collectFds();

	bool mapped() const { return 0 != mappedBuffers_.size(); }

	size_t size() { return pool_.size(); }
	std::vector<std::unique_ptr<FrameBuffer>> &content()
	{
		return pool_.content();
	}

private:
	LIBCAMERA_DISABLE_COPY_AND_MOVE(InfoFramePool)

	int setBuffers(const PixelFormat &format, const Size &size,
		       std::vector<std::unique_ptr<FrameBuffer>> &buffers,
		       unsigned int align, unsigned int scanAlign);

	Size size_;
	PixelFormat format_;
	Pool<FrameBuffer *, std::unique_ptr<FrameBuffer>> pool_;
	unsigned int strideAlign_;
	unsigned int scanAlign_;

	std::unordered_map<int, MappedBufferInfo> mappedBuffers_;
};

class LazyInfoFramePool
{
public:
	LazyInfoFramePool() = default;
	~LazyInfoFramePool() = default;

	int setFormat(DmaHeap *dmaHeap, const PixelFormat &format,
		      const Size &size,
		      DmaHeap::Type type = DmaHeap::System,
		      unsigned int strideAlign = 1, unsigned scanAlign = 1);

	void release();

	void fetch(SharedMailBox<InfoFrame> &mailBox);

	InfoFrame get();
	void put(InfoFrame &frameInfo);

private:
	DmaHeap *dmaHeap_;
	DmaHeap::Type type_;
	Size size_;
	PixelFormat format_;
	unsigned int strideAlign_;
	unsigned int scanAlign_;

	std::vector<std::unique_ptr<FrameBuffer>> allocatedBuffers_;

	Mutex mutex_;
};

class ElasticInfoFramePool
{
public:
	ElasticInfoFramePool() = default;
	~ElasticInfoFramePool() = default;

	int setFormat(DmaHeap *dmaHeap, const PixelFormat &format,
		      const Size &size, uint32_t minCount = 0,
		      DmaHeap::Type type = DmaHeap::System,
		      unsigned int strideAlign = 1, unsigned scanAlign = 1);

	void release();
	void releaseElastic();

	void fetch(SharedMailBox<InfoFrame> &mailBox);

	InfoFrame get();
	void put(InfoFrame &frameInfo);

private:
	LIBCAMERA_DISABLE_COPY_AND_MOVE(ElasticInfoFramePool)

	DmaHeap *dmaHeap_;
	DmaHeap::Type type_;
	Size size_;
	uint32_t minCount_;
	PixelFormat format_;
	unsigned int strideAlign_;
	unsigned int scanAlign_;

	Pool<FrameBuffer *, std::unique_ptr<FrameBuffer>> pool_;

	Mutex mutex_;
	uint32_t idleCnt_ LIBCAMERA_TSA_GUARDED_BY(mutex_) = 0;
};

} /* namespace libcamera */

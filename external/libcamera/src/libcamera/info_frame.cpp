/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * info_frame.cpp - InfoFrame and InfoFramePool
 */

#include <sys/mman.h>
#include <unistd.h>
#include <unordered_set>

#include "libcamera/internal/dma_heaps.h"
#include "libcamera/internal/formats.h"

#include <libcamera/internal/info_frame.h>

namespace libcamera {

LOG_DEFINE_CATEGORY(InfoFrame)

InfoFrame::InfoFrame() = default;

InfoFrame::InfoFrame(const PixelFormat &format, const Size &size, FrameBuffer *buffer,
		     unsigned int strideAlign, unsigned int scanAlign)
	: size_(size), format_(format), buffer_(buffer), strideAlign_(strideAlign), scanAlign_(scanAlign)
{
	numPlanes_ = buffer->planes().size();
	for (size_t i = 0; i < 3; i++)
		planes_[i].address = nullptr;
}

void InfoFrame::setAddress(unsigned int plane, uint8_t *address)
{
	if (plane < numPlanes_)
		planes_[plane].address = address;
}

uint8_t *InfoFrame::address(unsigned int plane) const
{
	if (plane < numPlanes_)
		return planes_[plane].address;
	return nullptr;
}

InfoFramePool::InfoFramePool() = default;

InfoFramePool::~InfoFramePool()
{
	release();
}

void InfoFramePool::release()
{
	if (unmap())
		LOG(InfoFrame, Error) << "Failed to unmap mapped buffers";

	pool_.release();
}

int InfoFramePool::setBuffers(const PixelFormat &format, const Size &size,
			      std::vector<std::unique_ptr<FrameBuffer>> &buffers,
			      unsigned int strideAlign, unsigned int scanAlign)
{
	if (unmap())
		LOG(InfoFrame, Error) << "Failed to unmap buffers";

	size_ = size;
	format_ = format;
	pool_.setData(buffers);
	strideAlign_ = strideAlign;
	scanAlign_ = scanAlign;

	return 0;
}

std::vector<int> InfoFramePool::collectFds()
{
	std::unordered_set<int> fds;
	for (auto &buffer : pool_.content()) {
		for (auto &plane : buffer->planes()) {
			const int fd = plane.fd.get();
			if (fd < 0) {
				LOG(InfoFrame, Error) << "Invalid fd: " << fd;
				continue;
			}
			fds.insert(fd);
		}
	}
	return std::vector<int>(fds.begin(), fds.end());
}

int InfoFramePool::mmap()
{
	if (!mappedBuffers_.empty())
		return 0;

	for (auto &buffer : pool_.content()) {
		for (const FrameBuffer::Plane &plane : buffer->planes()) {
			const int fd = plane.fd.get();
			if (mappedBuffers_.find(fd) == mappedBuffers_.end()) {
				const size_t length = lseek(fd, 0, SEEK_END);
				mappedBuffers_[fd] = MappedBufferInfo{ nullptr, length };
			}
		}
	}

	for (auto &[fd, info] : mappedBuffers_) {
		void *address = ::mmap(nullptr, info.dmabufLength,
				       PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		info.address = static_cast<uint8_t *>(address);
	}
	return 0;
}

int InfoFramePool::unmap()
{
	if (mappedBuffers_.empty())
		return 0;

	for (auto &[_, info] : mappedBuffers_)
		munmap(info.address, info.dmabufLength);

	mappedBuffers_.clear();
	return 0;
}

InfoFrame InfoFramePool::get()
{
	FrameBuffer *buffer = pool_.get();

	InfoFrame info(format_, size_, buffer, strideAlign_, scanAlign_);
	for (size_t i = 0; i < buffer->planes().size(); i++) {
		const int fd = buffer->planes()[i].fd.get();
		const unsigned int offset = buffer->planes()[i].offset;

		if (mappedBuffers_.count(fd))
			info.setAddress(i, mappedBuffers_[fd].address + offset);
	}

	return info;
}

void InfoFramePool::put(InfoFrame &info)
{
	pool_.put(info.buffer());
}

void InfoFramePool::fetch(SharedMailBox<InfoFrame> &mailBox)
{
	auto recycler = [this](InfoFrame &info) {
		this->put(info);
	};

	mailBox->put(get(), recycler);
}

int InfoFramePool::createBuffers(DmaHeap *dmaHeap,
				 const PixelFormat &format, const Size &size,
				 unsigned int count, DmaHeap::Type type,
				 unsigned int strideAlign, unsigned scanAlign)
{
	release();
	const PixelFormatInfo &info = PixelFormatInfo::info(format);
	uint32_t bufferSize = 0;
	for (unsigned int i = 0; i < info.numPlanes(); i++)
		bufferSize += info.planeSize(size, i, strideAlign, scanAlign);

	std::vector<std::unique_ptr<FrameBuffer>> buffers;
	buffers.reserve(count);
	for (unsigned int i = 0; i < count; i++) {
		SharedFD fd(dmaHeap->alloc(bufferSize, type));
		if (!fd.isValid()) {
			buffers.clear();
			return -EBUSY;
		}

		uint32_t offset = 0;
		std::vector<FrameBuffer::Plane> planes;

		for (unsigned int j = 0; j < info.numPlanes(); j++) {
			FrameBuffer::Plane plane;
			plane.fd = fd;
			plane.offset = offset;
			plane.length = info.planeSize(size, j, strideAlign, scanAlign);
			plane.stride = info.stride(size.width, j, strideAlign);
			planes.emplace_back(plane);
			offset += plane.length;
		}

		buffers.emplace_back(std::make_unique<FrameBuffer>(planes));
	}

	return setBuffers(format, size, buffers, strideAlign, scanAlign);
}

int LazyInfoFramePool::setFormat(DmaHeap *dmaHeap, const PixelFormat &format,
				 const Size &size,
				 DmaHeap::Type type,
				 unsigned int strideAlign, unsigned scanAlign)
{
	dmaHeap_ = dmaHeap;
	type_ = type;
	size_ = size;
	format_ = format;
	strideAlign_ = strideAlign;
	scanAlign_ = scanAlign;

	return 0;
}

void LazyInfoFramePool::release()
{
	allocatedBuffers_.clear();
}

void LazyInfoFramePool::fetch(SharedMailBox<InfoFrame> &mailBox)
{
	auto recycler = [this](InfoFrame &info) {
		this->put(info);
	};

	mailBox->put(get(), recycler);
}

InfoFrame LazyInfoFramePool::get()
{
	const PixelFormatInfo &info = PixelFormatInfo::info(format_);
	uint32_t bufferSize = 0;
	for (unsigned int i = 0; i < info.numPlanes(); i++)
		bufferSize += info.planeSize(size_, i, strideAlign_, scanAlign_);

	SharedFD fd(dmaHeap_->alloc(bufferSize, type_));
	if (!fd.isValid()) {
		LOG(InfoFrame, Fatal) << "fail to allocate dma buf, size " << bufferSize;
	}

	uint32_t offset = 0;
	std::vector<FrameBuffer::Plane> planes;

	for (unsigned int j = 0; j < info.numPlanes(); j++) {
		FrameBuffer::Plane plane;
		plane.fd = fd;
		plane.offset = offset;
		plane.length = info.planeSize(size_, j, strideAlign_, scanAlign_);
		plane.stride = info.stride(size_.width, j, strideAlign_);
		planes.emplace_back(plane);
		offset += plane.length;
	}

	std::scoped_lock lock(mutex_);
	allocatedBuffers_.emplace_back(std::make_unique<FrameBuffer>(planes));

	FrameBuffer *buffer = allocatedBuffers_.back().get();
	InfoFrame infoFrame(format_, size_, buffer, strideAlign_, scanAlign_);
	return infoFrame;
}

void LazyInfoFramePool::put(InfoFrame &frameInfo)
{
	std::scoped_lock lock(mutex_);
	for (auto iter = allocatedBuffers_.begin();
	     iter != allocatedBuffers_.end(); iter++) {
		if (iter->get() == frameInfo.buffer()) {
			allocatedBuffers_.erase(iter);
			return;
		}
	}

	LOG(InfoFrame, Fatal) << "Unknown buffer returned to LazyInfoFramePool";
}

int ElasticInfoFramePool::setFormat(DmaHeap *dmaHeap, const PixelFormat &format,
				    const Size &size, uint32_t minCount,
				    DmaHeap::Type type,
				    unsigned int strideAlign, unsigned scanAlign)
{
	release();

	if (minCount > 0) {
		const PixelFormatInfo &info = PixelFormatInfo::info(format);
		uint32_t bufferSize = 0;
		for (unsigned int i = 0; i < info.numPlanes(); i++)
			bufferSize += info.planeSize(size, i, strideAlign, scanAlign);

		std::vector<std::unique_ptr<FrameBuffer>> buffers;
		buffers.reserve(minCount);
		for (unsigned int i = 0; i < minCount; i++) {
			SharedFD fd(dmaHeap->alloc(bufferSize, type));
			if (!fd.isValid()) {
				buffers.clear();
				return -EBUSY;
			}

			uint32_t offset = 0;
			std::vector<FrameBuffer::Plane> planes;

			for (unsigned int j = 0; j < info.numPlanes(); j++) {
				FrameBuffer::Plane plane;
				plane.fd = fd;
				plane.offset = offset;
				plane.length = info.planeSize(size, j, strideAlign, scanAlign);
				plane.stride = info.stride(size.width, j, strideAlign);
				planes.emplace_back(plane);
				offset += plane.length;
			}

			buffers.emplace_back(std::make_unique<FrameBuffer>(planes));
		}

		pool_.setData(buffers);

		MutexLocker lock(mutex_);
		idleCnt_ = minCount;
	}

	dmaHeap_ = dmaHeap;
	type_ = type;
	size_ = size;
	minCount_ = minCount;
	format_ = format;
	strideAlign_ = strideAlign;
	scanAlign_ = scanAlign;

	return 0;
}

void ElasticInfoFramePool::release()
{
	pool_.release();
	minCount_ = 0;

	MutexLocker lock(mutex_);
	idleCnt_ = 0;
}

void ElasticInfoFramePool::releaseElastic()
{
	size_t num = pool_.size() - minCount_;
	pool_.drop(num);

	MutexLocker lock(mutex_);
	if (idleCnt_ < num)
		LOG(InfoFrame, Fatal) << "Not enough idle buffers";

	idleCnt_ -= num;
}

void ElasticInfoFramePool::fetch(SharedMailBox<InfoFrame> &mailBox)
{
	auto recycler = [this](InfoFrame &info) {
		this->put(info);
	};

	mailBox->put(get(), recycler);
}

InfoFrame ElasticInfoFramePool::get()
{
	{
		MutexLocker lock(mutex_);
		if (idleCnt_ > 0) {
			--idleCnt_;
			return InfoFrame(format_, size_, pool_.get(),
					 strideAlign_, scanAlign_);
		}
	}

	const PixelFormatInfo &info = PixelFormatInfo::info(format_);
	uint32_t bufferSize = 0;
	for (unsigned int i = 0; i < info.numPlanes(); i++)
		bufferSize += info.planeSize(size_, i, strideAlign_, scanAlign_);

	SharedFD fd(dmaHeap_->alloc(bufferSize, type_));
	if (!fd.isValid()) {
		LOG(InfoFrame, Fatal) << "fail to allocate dma buf, size " << bufferSize;
	}

	uint32_t offset = 0;
	std::vector<FrameBuffer::Plane> planes;

	for (unsigned int j = 0; j < info.numPlanes(); j++) {
		FrameBuffer::Plane plane;
		plane.fd = fd;
		plane.offset = offset;
		plane.length = info.planeSize(size_, j, strideAlign_, scanAlign_);
		plane.stride = info.stride(size_.width, j, strideAlign_);
		planes.emplace_back(plane);
		offset += plane.length;
	}

	FrameBuffer *buffer = pool_.addAndGet(std::make_unique<FrameBuffer>(planes));

	InfoFrame infoFrame(format_, size_, buffer, strideAlign_, scanAlign_);
	return infoFrame;
}

void ElasticInfoFramePool::put(InfoFrame &frameInfo)
{
	pool_.put(frameInfo.buffer());

	MutexLocker lock(mutex_);
	++idleCnt_;
}

} /* namespace libcamera */

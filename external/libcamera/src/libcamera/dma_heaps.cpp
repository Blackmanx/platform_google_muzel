/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * dma_heaps.cpp - Helper class for dma-heap allocations
 */

#include "libcamera/internal/dma_heaps.h"

#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/dma-buf.h>
#include <linux/dma-heap.h>

#include <libcamera/base/log.h>
#include <libcamera/base/shared_fd.h>

#include <libcamera/framebuffer.h>

namespace libcamera {

LOG_DEFINE_CATEGORY(DmaHeap)

namespace {
constexpr const char *kHeapName = "/dev/dma_heap/system";
constexpr const char *kHeapCmaName = "/dev/dma_heap/scp-isp-cma-region";
} // namespace

DmaHeap::DmaHeap()
{
	valid_ = false;

	int fd = ::open(kHeapName, O_RDWR, O_RDONLY | O_CLOEXEC);
	if (fd < 0) {
		LOG(DmaHeap, Error) << "Failed to open " << kHeapName << ": "
				    << strerror(errno);
		return;
	}

	UniqueFD systemHeap(fd);

	fd = ::open(kHeapCmaName, O_RDWR, O_RDONLY | O_CLOEXEC);
	if (fd < 0) {
		LOG(DmaHeap, Error) << "Failed to open " << kHeapCmaName << ": "
				    << strerror(errno);
		return;
	}

	dmaHeapCmaHandle_ = UniqueFD(fd);
	dmaHeapHandle_ = std::move(systemHeap);

	valid_ = true;
}

DmaHeap::~DmaHeap() = default;

UniqueFD DmaHeap::alloc(std::size_t size, Type type)
{
	struct dma_heap_allocation_data heap_data{
		.len = size,
		.fd = 0,
		.fd_flags = O_RDWR | O_CLOEXEC,
		.heap_flags = 0,
	};

	int dmaFd = (type == CMA) ? dmaHeapCmaHandle_.get() : dmaHeapHandle_.get();
	int ret;
	do {
		ret = ioctl(dmaFd, DMA_HEAP_IOCTL_ALLOC, &heap_data);
	} while (!ret && (errno == EINTR || errno == EAGAIN));

	if (ret) {
		LOG(DmaHeap, Error) << "Unable to allocate from "
				    << " DMA-BUF heap " << strerror(errno)
				    << " type " << ((type == CMA) ? "CMA" : "System")
				    << " heap_data.fd_flags " << heap_data.fd_flags
				    << " len " << heap_data.len;
		return {};
	}

	return UniqueFD(heap_data.fd);
}

void DmaHeap::sync(int fd, SyncStep step, SyncType type)
{
	uint64_t flags = 0;
	switch (step) {
	case Start:
		flags = DMA_BUF_SYNC_START;
		break;
	case End:
		flags = DMA_BUF_SYNC_END;
		break;
	}

	switch (type) {
	case SyncRead:
		flags = flags | DMA_BUF_SYNC_READ;
		break;
	case SyncWrite:
		flags = flags | DMA_BUF_SYNC_WRITE;
		break;
	case SyncReadWrite:
		flags = flags | DMA_BUF_SYNC_RW;
		break;
	}

	struct dma_buf_sync sync = {
		.flags = flags
	};

	int ret;
	do {
		ret = ioctl(fd, DMA_BUF_IOCTL_SYNC, &sync);
	} while (!ret && (errno == EINTR || errno == EAGAIN));

	if (ret) {
		LOG(DmaHeap, Error) << "Unable to sync dma fd " << fd
				    << " step " << step;
	}
}

/**
 * \brief Allocate and export buffers from the DmaBufAllocator
 * \param[in] count The number of requested FrameBuffers
 * \param[in] planeSizes The sizes of planes in each FrameBuffer
 * \param[out] buffers Array of buffers successfully allocated
 *
 * Planes in a FrameBuffer are allocated with a single dma buf.
 * \todo Add the option to allocate each plane with a dma buf respectively.
 *
 * \return The number of allocated buffers on success or a negative error code
 * otherwise
 */
int DmaHeap::exportBuffers(unsigned int count,
			   const std::vector<unsigned int> &planeSizes,
			   std::vector<std::unique_ptr<FrameBuffer>> *buffers,
			   Type type)
{
	for (unsigned int i = 0; i < count; ++i) {
		std::unique_ptr<FrameBuffer> buffer =
			createBuffer(planeSizes, type);
		if (!buffer) {
			LOG(DmaHeap, Error) << "Unable to create buffer";

			buffers->clear();
			return -EINVAL;
		}

		buffers->push_back(std::move(buffer));
	}

	return count;
}

std::unique_ptr<FrameBuffer>
DmaHeap::createBuffer(const std::vector<unsigned int> &planeSizes, Type type)
{
	std::vector<FrameBuffer::Plane> planes;

	unsigned int frameSize = 0, offset = 0;
	for (auto planeSize : planeSizes)
		frameSize += planeSize;

	SharedFD fd(alloc(frameSize, type));
	if (!fd.isValid())
		return nullptr;

	for (auto planeSize : planeSizes) {
		planes.emplace_back(FrameBuffer::Plane{ fd, offset, planeSize, 0 });
		offset += planeSize;
	}

	return std::make_unique<FrameBuffer>(planes);
}

} /* namespace libcamera */

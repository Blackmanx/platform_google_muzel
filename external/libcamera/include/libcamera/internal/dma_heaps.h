/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * dma_heaps.h - Helper class for dma-heap allocations
 */

#pragma once

#include <memory>
#include <vector>

#include <libcamera/base/unique_fd.h>

namespace libcamera {

class FrameBuffer;

class DmaHeap
{
public:
	enum Type {
		System,
		CMA,
	};

	enum SyncStep {
		Start = 0,
		End,
	};

	enum SyncType {
		SyncRead,
		SyncWrite,
		SyncReadWrite,
	};

	static void sync(int fd, SyncStep step, SyncType type);

	DmaHeap();
	~DmaHeap();
	bool isValid() { return valid_; }
	UniqueFD alloc(std::size_t size, Type type);

	int exportBuffers(unsigned int count,
			  const std::vector<unsigned int> &planeSizes,
			  std::vector<std::unique_ptr<FrameBuffer>> *buffers,
			  Type type);

private:
	std::unique_ptr<FrameBuffer> createBuffer(
		const std::vector<unsigned int> &planeSizes, Type type);

	bool valid_;
	UniqueFD dmaHeapHandle_;
	UniqueFD dmaHeapCmaHandle_;
};

class DmaSyncer final
{
public:
	explicit DmaSyncer(int fd, DmaHeap::SyncType type = DmaHeap::SyncReadWrite)
		: fd_(fd)
	{
		DmaHeap::sync(fd_, DmaHeap::Start, type);
	}

	~DmaSyncer()
	{
		DmaHeap::sync(fd_, DmaHeap::End, DmaHeap::SyncReadWrite);
	}

private:
	int fd_;
};

} /* namespace libcamera */

/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * pools.h - Template class for generic pool
 *
 * Common usage:
 *
 * Unique tokens:
 * Pool<uint32_t, BasicContainer<uint32_t>>;
 *
 * Auto closed Fds:
 * Pool<int, UniqueFD>;
 *
 * Auto destructed FrameBuffers:
 * Pool<FrameBuffer *, std::unique_ptr<FrameBuffer>>;
 */

#pragma once

#include <deque>
#include <memory>
#include <string>

#include <libcamera/base/log.h>
#include <libcamera/base/mutex.h>

namespace libcamera {

template<typename T>
class BasicContainer
{
public:
	BasicContainer(T &value)
		: value_(value) {}
	T get() { return value_; }

private:
	T value_;
};

template<typename T, typename UniquePtr>
class Pool : public Loggable
{
public:
	Pool() = default;
	~Pool() = default;

	/* Loggable */
	std::string logPrefix() const override { return "Pool"; }

	int setData(std::vector<UniquePtr> &pool)
	{
		pool_.swap(pool);
		free_.clear();
		inUse_.clear();

		for (auto &fd : pool_) {
			free_.emplace_back(fd.get());
		}

		return 0;
	}

	T addAndGet(UniquePtr obj)
	{
		std::scoped_lock lock(mutex_);
		T fd = obj.get();
		inUse_.emplace_back(fd);
		pool_.insert(pool_.end(), std::move(obj));

		return fd;
	}

	void drop(size_t num)
	{
		std::scoped_lock lock(mutex_);
		if (free_.size() < num) {
			LOG(Fatal) << "Failed to drop " << num
				   << " buffers. Free: " << free_.size();
		}

		for (size_t i = 0; i < num; ++i) {
			auto it = std::find_if(pool_.begin(), pool_.end(),
					       [&](const UniquePtr &ptr) { return ptr.get() == free_.back(); });

			if (it == pool_.end())
				LOG(Fatal) << "Failed to find buffer in pool.";

			pool_.erase(it);
			free_.pop_back();
		}
	}

	void release()
	{
		pool_.clear();
		free_.clear();
		inUse_.clear();
	}

	std::vector<UniquePtr> &content()
	{
		return pool_;
	}

	size_t size()
	{
		return pool_.size();
	}

	T get()
	{
		std::scoped_lock lock(mutex_);

		if (free_.empty())
			LOG(Fatal) << "not enough data in the Pool";

		T fd = free_.front();
		free_.pop_front();
		inUse_.emplace_back(fd);

		return fd;
	}

	void put(T data)
	{
		std::scoped_lock lock(mutex_);

		auto iter = std::find(inUse_.begin(), inUse_.end(), data);
		if (iter == inUse_.end())
			LOG(Fatal) << "return data not belonging to the pool";

		inUse_.erase(iter);
		free_.emplace_front(data);
	}

private:
	LIBCAMERA_DISABLE_COPY_AND_MOVE(Pool)

	std::vector<UniquePtr> pool_;
	std::deque<T> free_;
	std::deque<T> inUse_;
	Mutex mutex_;
};

using TokenPool = Pool<uint32_t, BasicContainer<uint32_t>>;

} /* namespace libcamera */

/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2024, Google Inc.
 *
 * history.h - Template class to store a list of structs.
 */
#pragma once

#include <cstdint>
#include <deque>

#include "libcamera/base/log.h"
#include "libcamera/base/mutex.h"

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

template<typename T>
class History
{
public:
	static const uint32_t kMaxSize = 32;

	History(int size = kMaxSize)
		: size_(size)
	{
	}

	void add(uint32_t id, T t)
	{
		MutexLocker locker(lock_);

		resultHistory_.push_back(std::make_pair(id, std::move(t)));
		if (resultHistory_.size() > size_)
			resultHistory_.pop_front();
	}

	uint32_t lastId()
	{
		MutexLocker locker(lock_);

		return resultHistory_.back().first;
	}

	T *query(uint32_t id)
	{
		MutexLocker locker(lock_);

		ASSERT(!resultHistory_.empty());

		for (auto &[resultId, result] : resultHistory_) {
			if (id == resultId)
				return &result;
		}

		LOG(MtkISP7, Error) << " Cannot find result of id " << id
				    << ". Use the latest one.";

		return &resultHistory_.back().second;
	}

	void release()
	{
		MutexLocker locker(lock_);

		resultHistory_.clear();
	}

private:
	uint32_t size_;

	Mutex lock_;
	std::deque<std::pair<uint32_t, T>> resultHistory_
		LIBCAMERA_TSA_GUARDED_BY(lock_);
};

} // namespace libcamera

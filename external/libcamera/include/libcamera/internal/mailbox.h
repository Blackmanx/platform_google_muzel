/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * mailbox.h - Template class for generic mailbox
 */

#pragma once

#include <functional>

#include <libcamera/base/log.h>

namespace libcamera {

template<class T>
class MailBox
{
public:
	using Recycler = std::function<void(T &)>;

	MailBox()
		: valid_(false) {}
	~MailBox()
	{
		if (valid_ && recycler_)
			recycler_(item_);
	}

	void put(const T &item, Recycler recycler)
	{
		ASSERT(!valid_);

		valid_ = true;
		recycler_ = recycler;
		item_ = item;
	}

	const T &get()
	{
		ASSERT(valid_);
		return item_;
	}

	bool valid() { return valid_; }

private:
	T item_;
	bool valid_;
	std::function<void(T &)> recycler_;
};

template<class T>
using SharedMailBox = std::shared_ptr<MailBox<T>>;

template<class T>
SharedMailBox<T> makeMailBox()
{
	return std::make_shared<MailBox<T>>();
}

template<class T>
std::vector<SharedMailBox<T>> makeMailBoxVector(unsigned int count)
{
	std::vector<SharedMailBox<T>> mailBoxes;
	mailBoxes.resize(count);
	for (unsigned int i = 0; i < count; i++)
		mailBoxes[i] = std::make_shared<MailBox<T>>();

	return mailBoxes;
}

} /* namespace libcamera */

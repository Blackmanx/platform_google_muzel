// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "allocator.h"

#include <memory>

#include "minigbm_allocator.h"

namespace crosLibcamera {

// static
std::unique_ptr<Allocator> Allocator::Create(Allocator::Backend backend)
{
	switch (backend) {
	case Allocator::Backend::kMinigbm:
		return CreateMinigbmAllocator();
	}
	return nullptr;
}

} // namespace crosLibcamera

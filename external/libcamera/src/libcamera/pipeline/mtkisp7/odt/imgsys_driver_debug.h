/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * imgsys_driver_debug.h - Imgsys driver debugfs wrapper.
 */

#pragma once

#include <cstdint>
#include <memory>

#include <libcamera/base/unique_fd.h>

namespace libcamera {

constexpr const size_t kDumpFileNameLength = 256;

class ImgsysDebug
{
public:
        void exportDump(int mediaRequestFd, size_t stageCount);

private:
        void exportDumpIfExists(char fileName[kDumpFileNameLength],
                                uint8_t *dump, size_t size);

        UniqueFD debugFsFd_;
};

} // namespace libcamera

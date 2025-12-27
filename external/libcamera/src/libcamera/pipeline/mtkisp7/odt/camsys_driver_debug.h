/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * camsys_driver_debug.h - Camsys driver debugfs wrapper.
 */

#pragma once

#include <filesystem>
#include <memory>

#include <libcamera/base/unique_fd.h>

namespace libcamera {

class CamsysDebug
{
public:
	static std::unique_ptr<CamsysDebug> create(int index);

	CamsysDebug(int index, UniqueFD ctrlFd, std::filesystem::path dumpPath);
	~CamsysDebug();

	void exportDump(
		int sequenceNumber, int requestNumber,
		std::filesystem::path destinationPrefix);

private:
	static const std::filesystem::path kDebugCtrl;
	static const std::filesystem::path kDebugDump;
	static const std::filesystem::path kDebugFsRoot;
	static const std::string kExportDumpBeginPrefix;
	static const std::string kExportDumpEndPrefix;
	static const std::string kStartDebugCmd;
	static const std::string kStopDebugCmd;

	const int camsysIndex_;
	UniqueFD ctrlFd_;
	const std::filesystem::path dumpPath_;
};

} // namespace libcamera

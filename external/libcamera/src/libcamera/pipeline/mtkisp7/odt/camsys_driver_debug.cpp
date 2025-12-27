/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * camsys_driver_debug.cpp - Camsys driver debugfs wrapper.
 */

#include "pipeline/mtkisp7/odt/camsys_driver_debug.h"

#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

#include <libcamera/base/log.h>
#include <libcamera/base/unique_fd.h>

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

const std::filesystem::path CamsysDebug::kDebugCtrl = "ctrl";
const std::filesystem::path CamsysDebug::kDebugDump = "data";
const std::filesystem::path CamsysDebug::kDebugFsRoot =
	"/sys/kernel/debug/mtk_cam_dbg";

const std::string CamsysDebug::kExportDumpBeginPrefix = "r:s:";
const std::string CamsysDebug::kExportDumpEndPrefix = "r:e:";
const std::string CamsysDebug::kStartDebugCmd = "d:s";
const std::string CamsysDebug::kStopDebugCmd = "d:e";

CamsysDebug::CamsysDebug(
	int index, UniqueFD ctrlFd, std::filesystem::path dumpPath)
	: camsysIndex_(index), ctrlFd_(std::move(ctrlFd)), dumpPath_(dumpPath)
{
}

CamsysDebug::~CamsysDebug()
{
	int ret = write(ctrlFd_.get(), kStartDebugCmd.c_str(), kStartDebugCmd.length());
	if (ret == -1) {
		LOG(MtkISP7, Error) << "Failed to stop camsys debug: " << errno;
		LOG(MtkISP7, Fatal) << "Failed to disable camsys debug, if debug is no "
				    << "longer wanted, please reboot device.";
	}
	LOG(MtkISP7, Info) << "Camsys driver debug disabled";
}

// static
std::unique_ptr<CamsysDebug> CamsysDebug::create(int index)
{
	const std::filesystem::path ctrlPath =
		kDebugFsRoot / std::to_string(index) / kDebugCtrl;
	UniqueFD ctrlFd(open(ctrlPath.c_str(), O_RDWR, 0660));
	if (!ctrlFd.isValid()) {
		LOG(MtkISP7, Error) << "Failed to open camsys debug control: " << errno;
		return nullptr;
	}

	int ret = write(
		ctrlFd.get(), kStartDebugCmd.c_str(), kStartDebugCmd.length());
	if (ret == -1) {
		LOG(MtkISP7, Error) << "Failed to start camsys debug: " << errno;
		return nullptr;
	}

	LOG(MtkISP7, Warning) << "Camsys driver debug enabled on index: "
			      << std::to_string(index);

	std::filesystem::path dumpPath =
		kDebugFsRoot / std::to_string(index) / kDebugDump;

	return std::make_unique<CamsysDebug>(
		index, std::move(ctrlFd), dumpPath);
}

void CamsysDebug::exportDump(
	int sequenceNumber, int requestNumber,
	std::filesystem::path destinationPrefix)
{
	std::string seqNumStr = std::to_string(sequenceNumber);
	std::string reqNumStr = std::to_string(requestNumber);
	std::filesystem::path destination =
		destinationPrefix.string() + "_" + reqNumStr + "_raw_" +
		std::to_string(camsysIndex_) + "_" + seqNumStr + "_p1.dump";
	std::string exportBeginCmd = kExportDumpBeginPrefix + seqNumStr;

	int ret = write(ctrlFd_.get(), exportBeginCmd.c_str(), exportBeginCmd.length());
	if (ret == -1) {
		LOG(MtkISP7, Error) << "Failed to export camsys debug dump: " << errno;
		return;
	}

	std::ifstream input(dumpPath_, std::ios::binary);
	std::ofstream output(destination, std::ios::binary);

	// Cannot seek, not implemented by the driver.
	// Cannot std::filesystem::copy, maybe also using seek.
	// Can only read until EOF.
	std::vector<char> chunks;
	while (!input.eof()) {
		chunks.assign(256, 0);
		input.read(chunks.data(), chunks.size());
		if (input.gcount() > 0) {
			output.write(chunks.data(), chunks.size());
		}
	}

	std::string exportEndCmd = kExportDumpEndPrefix + seqNumStr;
	ret = write(ctrlFd_.get(), exportEndCmd.c_str(), exportEndCmd.length());
	if (ret == -1) {
		LOG(MtkISP7, Error) << "Failed to flush camsys debug dump: " << errno;
		return;
	}

	LOG(MtkISP7, Info) << "Flushed camsys registers to: " << destination
			   << " ; size: " << output.tellp();
}

} // namespace libcamera

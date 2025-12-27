/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * imgsys_driver_debug.h - Imgsys driver debugfs wrapper.
 */

#include "pipeline/mtkisp7/odt/imgsys_driver_debug.h"

#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include <vector>

#include <libcamera/base/log.h>
#include <libcamera/base/unique_fd.h>

namespace libcamera {

LOG_DECLARE_CATEGORY(MtkISP7)

namespace {

constexpr const size_t kWpeDumpCount = 3;
constexpr const size_t kTrawDumpCount = 3;
constexpr const size_t kDipDumpCount = 1;
constexpr const size_t kPqDipDumpCount = 2;
constexpr const size_t kMeDumpCount = 1;

constexpr const size_t kWpeDumpSize = 0x1000;
constexpr const size_t kTrawDumpSize = 0xC000;
constexpr const size_t kDipDumpSize = 0x5F000;
constexpr const size_t kPqDipDumpSize = 0x6000;
constexpr const size_t kMeDumpSize = 0xA20;

struct ndd_wpe_info_t {
	char fp[kDumpFileNameLength];
	uint8_t data[kWpeDumpSize];
};

struct ndd_traw_info_t {
	char fp[kDumpFileNameLength];
	uint8_t data[kTrawDumpSize];
};

struct ndd_dip_info_t {
	char fp[kDumpFileNameLength];
	uint8_t data[kDipDumpSize];
};

struct ndd_pqdip_info_t {
	char fp[kDumpFileNameLength];
	uint8_t data[kPqDipDumpSize];
};

struct ndd_me_info_t {
	char fp[kDumpFileNameLength];
	uint8_t data[kMeDumpSize];
};

struct ndd_frm_info_t {
	struct ndd_wpe_info_t wpe[kWpeDumpCount];
	struct ndd_traw_info_t traw[kTrawDumpCount];
	struct ndd_dip_info_t dip[kDipDumpCount];
	struct ndd_pqdip_info_t pqdip[kPqDipDumpCount];
	struct ndd_me_info_t me[kMeDumpCount];
} __attribute__((__packed__));

struct ndd_request_t {
	int fd;
	int total_frmnum;
} __attribute__((__packed__));

const std::filesystem::path kDebugFsPath =
	"/sys/kernel/debug/mtk_imgsys_debug/mtk_imgsys_ndd";

} // namespace

void ImgsysDebug::exportDumpIfExists(char fileName[kDumpFileNameLength],
				     uint8_t *dump, size_t size)
{
	if (fileName == nullptr || fileName[0] == '\0') {
		return;
	}
	std::ofstream out(fileName, std::ios::binary);
	out.write(reinterpret_cast<char *>(dump), size);
	if (!out.good()) {
		LOG(MtkISP7, Error) << "Failed to export dump: " << fileName;
	}
}

void ImgsysDebug::exportDump(int mediaRequestFd, size_t stageCount)
{
	if (!debugFsFd_.isValid()) {
		debugFsFd_.reset(open(kDebugFsPath.c_str(), O_RDWR));
		if (!debugFsFd_.isValid()) {
			LOG(MtkISP7, Error) << "Failed to open imgsys debugFs";
			return;
		}
	}
	ndd_request_t nddRequest{
		.fd = mediaRequestFd,
		.total_frmnum = static_cast<int>(stageCount)
	};
	size_t bytesWritten =
		write(debugFsFd_.get(), &nddRequest, sizeof(ndd_request_t));
	if (bytesWritten != sizeof(ndd_request_t)) {
		LOG(MtkISP7, Error) << "Failed to write to imgsys debugFs. "
				    << "bytesWritten: " << bytesWritten;
		return;
	}
	std::vector<ndd_frm_info_t> imgsysDrvNdd(stageCount);
	size_t bytesRead = read(debugFsFd_.get(), imgsysDrvNdd.data(),
				stageCount * sizeof(ndd_frm_info_t));
	if (bytesRead != stageCount * sizeof(ndd_frm_info_t)) {
		LOG(MtkISP7, Error) << "Failed to read from imgsys debugFs. "
				    << "bytesRead: " << bytesRead;
		return;
	}
	for (size_t i = 0; i < stageCount; i++) {
		for (size_t j = 0; j < kWpeDumpCount; j++) {
			exportDumpIfExists(imgsysDrvNdd[i].wpe[j].fp,
					   imgsysDrvNdd[i].wpe[j].data,
					   kWpeDumpSize);
		}
		for (size_t j = 0; j < kTrawDumpCount; j++) {
			exportDumpIfExists(imgsysDrvNdd[i].traw[j].fp,
					   imgsysDrvNdd[i].traw[j].data,
					   kTrawDumpSize);
		}
		for (size_t j = 0; j < kDipDumpCount; j++) {
			exportDumpIfExists(imgsysDrvNdd[i].dip[j].fp,
					   imgsysDrvNdd[i].dip[j].data,
					   kDipDumpSize);
		}
		for (size_t j = 0; j < kPqDipDumpCount; j++) {
			exportDumpIfExists(imgsysDrvNdd[i].pqdip[j].fp,
					   imgsysDrvNdd[i].pqdip[j].data,
					   kPqDipDumpSize);
		}
		for (size_t j = 0; j < kMeDumpCount; j++) {
			exportDumpIfExists(imgsysDrvNdd[i].me[j].fp,
					   imgsysDrvNdd[i].me[j].data,
					   kMeDumpSize);
		}
	}
}

} // namespace libcamera

/*
 * Copyright (C) 2024, Google Inc.
 *
 * img_meta_request_data_helper.h - Helper of ImgMetaRequestData.
 */

#pragma once

#include <libcamera/ipa/mtkisp7_ipa_interface.h>

namespace libcamera {

ipa::mtkisp7::ImgMetaRequestData makeImgMetaRequestDataNonMfnr(
	bool _isCapture, uint32_t _stage, uint32_t _tuningBufferId,
	uint32_t _statisticsBufferId, uint32_t _swHistBufferId,
	const Size &_inputSize, const Size &_outputSize,
	const Size &_outputSize2, const Size &_fullDipSize,
	const std::map<uint32_t, uint32_t> &_reserved);

} /* namespace libcamera */

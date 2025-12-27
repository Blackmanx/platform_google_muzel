/*
 * Copyright (C) 2024, Google Inc.
 *
 * img_meta_request_data_helper.cpp - Helper of ImgMetaRequestData.
 */

#include "img_meta_request_data_helper.h"

namespace libcamera {

ipa::mtkisp7::ImgMetaRequestData makeImgMetaRequestDataNonMfnr(
	bool _isCapture, uint32_t _stage, uint32_t _tuningBufferId,
	uint32_t _statisticsBufferId, uint32_t _swHistBufferId,
	const Size &_inputSize, const Size &_outputSize,
	const Size &_outputSize2, const Size &_fullDipSize,
	const std::map<uint32_t, uint32_t> &_reserved)
{
	return ipa::mtkisp7::ImgMetaRequestData(
		_isCapture, _stage, _tuningBufferId, _statisticsBufferId,
		_swHistBufferId, _inputSize, _outputSize, _outputSize2,
		_fullDipSize, _reserved, false, 0, 1, 0, false, false, 0);
}

} /* namespace libcamera */

/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * feature.h - MtkISP7 on device tuner feature enum.
 */
#pragma once

#include <map>
#include <string>

namespace libcamera {

// todo(yerlandinata): yaml autogen

enum class Feature {
	Preview,
	Preview_eis,
	Preview_vhdr,
	Preview_e2ehdr,
	Preview_eis_vhdr,
	Preview_vhdr_e2ehdr,
	Preview_eis_vhdr_e2ehdr,
	Preview_vsdof_bayerbayer,
	Preview_vsdof_bayermono,
	Preview_vsdof_eis_bayerbayer,
	Preview_vsdof_eis_bayermono,
	Video,
	Video_eis,
	Video_vhdr,
	Video_e2ehdr,
	Video_eis_vhdr,
	Video_vhdr_e2ehdr,
	Video_eis_vhdr_e2ehdr,
	Video_vsdof_bayerbayer,
	Video_vsdof_bayermono,
	Video_vsdof_eis_bayerbayer,
	Video_vsdof_eis_bayermono,
	Capture_default,
	Capture_simple,
	Capture_cshot,
	Capture_cshot_lpnr,
	Capture_single_nr,
	Capture_remosaic,
	Capture_lpnr,
	Capture_mfnr,
	Capture_remosaic_mfnr,
	Capture_ainr,
	Capture_ainr_single_nr,
	Capture_aihdr,
	Capture_aihdr_single_nr,
	Capture_yuv_reprocessing_default,
	Capture_yuv_reprocessing_with_single_nr,
	Capture_yuv_encode_jpeg_default,
	Capture_yuv_encode_jpeg_with_single_nr,
	Capture_remosaic_single_nr,
	Capture_mfnr_single_nr,
	Capture_remosaic_mfnr_single_nr,
	Capture_ainr14b_lpnr,
	Capture_ainr16b_lpnr,
	Capture_aihdr16b_lpnr,
	NDD_default,
	Capture_aishutter1,
	Capture_aishutter2,
	Video_ainr_vhdr,
	Video_ainr,
	Video_ainr_vhdr_freerun,
	Video_ainr_freerun,
	AOV,
	NUM,
};

const std::map<Feature, std::string> kFeatureStrMap{
	{ Feature::Preview, "Preview" },
	{ Feature::Preview_eis, "Preview_eis" },
	{ Feature::Preview_vhdr, "Preview_vhdr" },
	{ Feature::Preview_e2ehdr, "Preview_e2ehdr" },
	{ Feature::Preview_eis_vhdr, "Preview_eis_vhdr" },
	{ Feature::Preview_vhdr_e2ehdr, "Preview_vhdr_e2ehdr" },
	{ Feature::Preview_eis_vhdr_e2ehdr, "Preview_eis_vhdr_e2ehdr" },
	{ Feature::Preview_vsdof_bayerbayer, "Preview_vsdof_bayerbayer" },
	{ Feature::Preview_vsdof_bayermono, "Preview_vsdof_bayermono" },
	{ Feature::Preview_vsdof_eis_bayerbayer, "Preview_vsdof_eis_bayerbayer" },
	{ Feature::Preview_vsdof_eis_bayermono, "Preview_vsdof_eis_bayermono" },
	{ Feature::Video, "Video" },
	{ Feature::Video_eis, "Video_eis" },
	{ Feature::Video_vhdr, "Video_vhdr" },
	{ Feature::Video_e2ehdr, "Video_e2ehdr" },
	{ Feature::Video_eis_vhdr, "Video_eis_vhdr" },
	{ Feature::Video_vhdr_e2ehdr, "Video_vhdr_e2ehdr" },
	{ Feature::Video_eis_vhdr_e2ehdr, "Video_eis_vhdr_e2ehdr" },
	{ Feature::Video_vsdof_bayerbayer, "Video_vsdof_bayerbayer" },
	{ Feature::Video_vsdof_bayermono, "Video_vsdof_bayermono" },
	{ Feature::Video_vsdof_eis_bayerbayer, "Video_vsdof_eis_bayerbayer" },
	{ Feature::Video_vsdof_eis_bayermono, "Video_vsdof_eis_bayermono" },
	{ Feature::Capture_default, "Capture_default" },
	{ Feature::Capture_simple, "Capture_simple" },
	{ Feature::Capture_cshot, "Capture_cshot" },
	{ Feature::Capture_cshot_lpnr, "Capture_cshot_lpnr" },
	{ Feature::Capture_single_nr, "Capture_single_nr" },
	{ Feature::Capture_remosaic, "Capture_remosaic" },
	{ Feature::Capture_lpnr, "Capture_lpnr" },
	{ Feature::Capture_mfnr, "Capture_mfnr" },
	{ Feature::Capture_remosaic_mfnr, "Capture_remosaic_mfnr" },
	{ Feature::Capture_ainr, "Capture_ainr" },
	{ Feature::Capture_ainr_single_nr, "Capture_ainr_single_nr" },
	{ Feature::Capture_aihdr, "Capture_aihdr" },
	{ Feature::Capture_aihdr_single_nr, "Capture_aihdr_single_nr" },
	{ Feature::Capture_yuv_reprocessing_default, "Capture_yuv_reprocessing_default" },
	{ Feature::Capture_yuv_reprocessing_with_single_nr, "Capture_yuv_reprocessing_with_single_nr" },
	{ Feature::Capture_yuv_encode_jpeg_default, "Capture_yuv_encode_jpeg_default" },
	{ Feature::Capture_yuv_encode_jpeg_with_single_nr, "Capture_yuv_encode_jpeg_with_single_nr" },
	{ Feature::Capture_remosaic_single_nr, "Capture_remosaic_single_nr" },
	{ Feature::Capture_mfnr_single_nr, "Capture_mfnr_single_nr" },
	{ Feature::Capture_remosaic_mfnr_single_nr, "Capture_remosaic_mfnr_single_nr" },
	{ Feature::Capture_ainr14b_lpnr, "Capture_ainr14b_lpnr" },
	{ Feature::Capture_ainr16b_lpnr, "Capture_ainr16b_lpnr" },
	{ Feature::Capture_aihdr16b_lpnr, "Capture_aihdr16b_lpnr" },
	{ Feature::NDD_default, "NDD_default" },
	{ Feature::Capture_aishutter1, "Capture_aishutter1" },
	{ Feature::Capture_aishutter2, "Capture_aishutter2" },
	{ Feature::Video_ainr_vhdr, "Video_ainr_vhdr" },
	{ Feature::Video_ainr, "Video_ainr" },
	{ Feature::Video_ainr_vhdr_freerun, "Video_ainr_vhdr_freerun" },
	{ Feature::Video_ainr_freerun, "Video_ainr_freerun" },
	{ Feature::AOV, "AOV" },
	{ Feature::NUM, "NUM" },
};

const std::map<std::string, Feature> kStrFeatureMap =
	[] {
		std::map<std::string, Feature> result;
		for (const auto &[feature, name] : kFeatureStrMap) {
			result[name] = feature;
		}
		return result;
	}();

} // namespace libcamera

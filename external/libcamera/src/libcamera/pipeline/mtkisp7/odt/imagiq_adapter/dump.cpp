/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2023, Google Inc.
 *
 * dump.cpp - MtkISP7 on device tuner dump information.
 */

#include "pipeline/mtkisp7/odt/imagiq_adapter/dump.h"

namespace libcamera {

const std::array<Dump::Id, 6> Dump::kWpeInputImageDumpIds{
	Dump::Id::WPE_WghtMap_WPEI_F0,
	Dump::Id::WPE_WghtMap_WPEI_F1,
	Dump::Id::WPE_WghtMap_WPEI_F2,
	Dump::Id::WPE_WghtMap_WPEI_F3,
	Dump::Id::WPE_WghtMap_WPEI_F4,
	Dump::Id::WPE_WghtMap_WPEI_F5
};
const std::array<Dump::Id, 6> Dump::kWpeWeightMapDumpIds{
	Dump::Id::WPE_WghtMap_WPE_MAP_F0,
	Dump::Id::WPE_WghtMap_WPE_MAP_F1,
	Dump::Id::WPE_WghtMap_WPE_MAP_F2,
	Dump::Id::WPE_WghtMap_WPE_MAP_F3,
	Dump::Id::WPE_WghtMap_WPE_MAP_F4,
	Dump::Id::WPE_WghtMap_WPE_MAP_F5
};
const std::array<Dump::Id, 6> Dump::kWpeOutputImageDumpIds{
	Dump::Id::WPE_WghtMap_WPEO_F0,
	Dump::Id::WPE_WghtMap_WPEO_F1,
	Dump::Id::WPE_WghtMap_WPEO_F2,
	Dump::Id::WPE_WghtMap_WPEO_F3,
	Dump::Id::WPE_WghtMap_WPEO_F4,
	Dump::Id::WPE_WghtMap_WPEO_F5
};
const std::array<Dump::Id, 6> Dump::kDip1ImgiDumpIds{
	Dump::Id::P2_MS_F1_IMGI_D1_MCNR,
	Dump::Id::P2_MS_F2_IMGI_D1_MCNR,
	Dump::Id::P2_MS_F3_IMGI_D1_MCNR,
	Dump::Id::P2_MS_F4_IMGI_D1,
	Dump::Id::P2_MS_F_SMALL_IMGI_D1,
	Dump::Id::P2_IDI_IMGI_D1
};
const std::array<Dump::Id, 6> Dump::kDip1VipiDumpIds{
	Dump::Id::P2_MS_F1_VIPI,
	Dump::Id::P2_MS_F2_VIPI,
	Dump::Id::P2_MS_F3_VIPI,
	Dump::Id::P2_MS_F4_VIPI,
	Dump::Id::P2_MS_F_SMALL_VIPI,
	Dump::Id::P2_IDI_VIPI
};
const std::array<Dump::Id, 6> Dump::kDip1TnrsiDumpIds{
	Dump::Id::P2_MS_F1_TNRSI,
	Dump::Id::P2_MS_F2_TNRSI,
	Dump::Id::P2_MS_F3_TNRSI,
	Dump::Id::P2_MS_F4_TNRSI,
	Dump::Id::P2_MS_F_SMALL_TNRSI,
	Dump::Id::P2_IDI_TNRSI
};
const std::array<Dump::Id, 6> Dump::kDip1TnrsoDumpIds{
	Dump::Id::P2_MS_F1_TNRSO,
	Dump::Id::P2_MS_F2_TNRSO,
	Dump::Id::P2_MS_F3_TNRSO,
	Dump::Id::P2_MS_F4_TNRSO,
	Dump::Id::P2_MS_F_SMALL_TNRSO,
	Dump::Id::P2_IDI_TNRSO
};
const std::array<Dump::Id, 6> Dump::kDip1Img3oDumpIds{
	Dump::Id::P2_MS_F1_IMG3O_MCNR,
	Dump::Id::P2_MS_F2_IMG3O_MCNR,
	Dump::Id::P2_MS_F3_IMG3O_MCNR,
	Dump::Id::P2_MS_F4_IMG3O,
	Dump::Id::P2_MS_F_SMALL_IMG3O,
	Dump::Id::P2_IDI_IMG3O
};
const std::array<Dump::Id, 6> Dump::kDip1MetaP2DumpIds{
	Dump::Id::P2_MS_F1_META_P2_MCNR,
	Dump::Id::P2_MS_F2_META_P2_MCNR,
	Dump::Id::P2_MS_F3_META_P2_MCNR,
	Dump::Id::P2_MS_F4_META_P2,
	Dump::Id::P2_MS_F_SMALL_META_P2,
	Dump::Id::P2_IDI_META_P2
};
const std::array<Dump::Id, 5> Dump::kDip1TnrwiDumpIds{
	Dump::Id::P2_MS_F1_TNRWI,
	Dump::Id::P2_MS_F2_TNRWI,
	Dump::Id::P2_MS_F3_TNRWI,
	Dump::Id::P2_MS_F4_TNRWI,
	Dump::Id::P2_MS_F_SMALL_TNRWI
};
const std::array<Dump::Id, 5> Dump::kDip1TnrciDumpIds{
	Dump::Id::P2_MS_F1_TNRCI,
	Dump::Id::P2_MS_F2_TNRCI,
	Dump::Id::P2_MS_F3_TNRCI,
	Dump::Id::P2_MS_F4_TNRCI,
	Dump::Id::P2_MS_F_SMALL_TNRCI
};
const std::array<Dump::Id, 5> Dump::kDip1TnrliDumpIds{
	Dump::Id::P2_MS_F1_TNRLI,
	Dump::Id::P2_MS_F2_TNRLI,
	Dump::Id::P2_MS_F3_TNRLI,
	Dump::Id::P2_MS_F4_TNRLI,
	Dump::Id::P2_MS_F_SMALL_TNRLI
};
const std::array<Dump::Id, 5> Dump::kDip1TnrvbiDumpIds{
	Dump::Id::P2_MS_F1_TNRVBI,
	Dump::Id::P2_MS_F2_TNRVBI,
	Dump::Id::P2_MS_F3_TNRVBI,
	Dump::Id::P2_MS_F4_TNRVBI,
	Dump::Id::P2_MS_F_SMALL_TNRVBI
};
const std::array<Dump::Id, 5> Dump::kDip1TnrmoDumpIds{
	Dump::Id::P2_MS_F1_TNRMO,
	Dump::Id::P2_MS_F2_TNRMO,
	Dump::Id::P2_MS_F3_TNRMO,
	Dump::Id::P2_MS_F4_TNRMO,
	Dump::Id::P2_MS_F_SMALL_TNRMO
};
const std::array<Dump::Id, 5> Dump::kDip1TnrwoDumpIds{
	Dump::Id::P2_MS_F1_TNRWO,
	Dump::Id::P2_MS_F2_TNRWO,
	Dump::Id::P2_MS_F3_TNRWO,
	Dump::Id::P2_MS_F4_TNRWO,
	Dump::Id::P2_MS_F_SMALL_TNRWO
};
const std::array<Dump::Id, 5> Dump::kDip1ReciDumpIds{
	Dump::Id::P2_MS_F1_RECI_D1_MCNR,
	Dump::Id::P2_MS_F2_RECI_D1_MCNR,
	Dump::Id::P2_MS_F3_RECI_D1,
	Dump::Id::P2_MS_F4_RECI_D1,
	Dump::Id::P2_MS_F_SMALL_RECI_D1
};
const std::array<Dump::Id, 4> Dump::kDip1TnrmiDumpIds{
	Dump::Id::P2_MS_F1_TNRMI,
	Dump::Id::P2_MS_F2_TNRMI,
	Dump::Id::P2_MS_F3_TNRMI,
	Dump::Id::P2_MS_F4_TNRMI
};

} // namespace libcamera

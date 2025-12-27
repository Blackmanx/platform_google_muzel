/*
 * This file is part of the flashrom project.
 *
 * Copyright 2021 Google LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <string.h>
#include "flash.h"

/* Cros flashrom needs to be able to automatically identify the board's flash
 * without additional input. If multiple flashchip entries are considered to be
 * suitable matches, flashrom will fail to identify the chip and be unable to
 * perform the requested operations.
 *
 * This function allows flashrom to always pick a single flashchip entry, by
 * filtering out all unwanted duplicate flashchip entries and leaving only the
 * one we want to use.
 */
bool is_chipname_duplicate(const struct flashchip *chip)
{
	/* The "GD25B128B/GD25Q128B" and "GD25Q127C/GD25Q128C" chip entries
	 * have the same vendor and model IDs.
	 *
	 * Historically cros flashrom only had the "C" entry; an initial
	 * attempt to import the "B" from upstream entry resulted in flashrom
	 * being unable to identify the flash on Atlas and Nocturne boards,
	 * causing flashrom failures documented in b/168943314.
	 *
	 * After introducing GD25Q128E, we need to split GD25Q127C/GD25Q128C
	 * into GD25Q127C/GD25Q128E and GD25Q128C since 127C and 128C actually
	 * have different features (mainly QPI).
	 * GD25Q128C is phased out now, so we mark GD25Q128C as duplicated.
	 */
	if(!strcmp(chip->name, "GD25B128B/GD25Q128B") ||
	   !strcmp(chip->name, "GD25Q128C"))
		return true;

	/* MX25L128... has been split into several entries:
	 * "MX25L12805D" (b/190574697), "MX25L12833F",
	 * "MX25L12835F/MX25L12873F", "MX25L12845E/MX25L12865E" (b/332486637),
	 * and "MX25L12850F" (CB:81350),
	 * We are actually using MX25L12833F, so mark the others as duplicate.
	 */
	if(!strcmp(chip->name, "MX25L12805D") ||
	   !strcmp(chip->name, "MX25L12835F/MX25L12873F") ||
	   !strcmp(chip->name, "MX25L12845E/MX25L12865E") ||
	   !strcmp(chip->name, "MX25L12850F"))
		return true;

	/* W25Q256.V has been split into two entries: W25Q256FV and
	 * W25Q256JV_Q.
	 * Marking the latter as duplicate.
	 */
	if(!strcmp(chip->name, "W25Q256JV_Q")) return true;

	/* W25Q32.W has been split into three entries:
	 * W25Q32BW/W25Q32CW/W25Q32DW, W25Q32FW and W25Q32JW...Q
	 * We are actually using W25Q32DW, so mark the last two as duplicate.
	 */
	if(!strcmp(chip->name, "W25Q32FW") ||
	   !strcmp(chip->name, "W25Q32JW...Q"))
		return true;

	/* The "GD25LQ255E" and "GD25LB256F/GD25LR256F" and chip entries
	 * have the same vendor and model IDs.
	 * Marking the latter as duplicate.
	 */
	if(!strcmp(chip->name, "GD25LB256F/GD25LR256F")) return true;

	/* FIXME(b/412502949): Felino Proto 0.
	 * The "MX77U51250F" and "MX77U51250F_32" and chip entries
	 * have the same vendor and model IDs.
	 * Marking the latter as duplicate.
	 */
	if(!strcmp(chip->name, "MX77U51250F")) return true;

	return false;
}

/*
 * This file is part of the flashrom project.
 *
 * Copyright (C) 2022 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*
 * For entertainment purposes of MP devices only.
 * with thx, Edward O'Callaghan.
 */

#include <stdint.h>
#include <stdlib.h>

#include "flash.h"
#include "programmer.h"

/* spi25_statusreg.c stubs. */
int spi_write_register(const struct flashctx *flash, enum flash_reg reg, uint8_t value)
{
	fprintf(stderr, " write register call unexpected!\n");
	abort();
	return 0;
}
int spi_read_register(const struct flashctx *flash, enum flash_reg reg, uint8_t *value)
{
	/* Assume all status registers have a default value of zero. */
	*value = 0;
	return 0;
}

/* flashrom stubs. */
void programmer_delay(const struct flashctx *flash, unsigned int usecs) {}
void update_progress(struct flashrom_flashctx *flashctx, enum flashrom_progress_stage stage, size_t current, size_t total) {}

/* jedec.c */
uint8_t oddparity(uint8_t val)
{
	val = (val ^ (val >> 4)) & 0xf;
	val = (val ^ (val >> 2)) & 0x3;
	return (val ^ (val >> 1)) & 0x1;
}

/* helpers.c */
int max(int a, int b)
{
	return (a > b) ? a : b;
}
int min(int a, int b)
{
	return (a < b) ? a : b;
}

/* libflashrom.c */
int print(enum flashrom_log_level level, const char *fmt, ...)
{
	/*
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, fmt, args);
	va_end(args);
	*/
	return 0;
}
size_t flashrom_flash_getsize(const struct flashrom_flashctx *const flashctx)
{
	return flashctx->chip->total_size * 1024;
}
enum flashrom_wp_result flashrom_wp_cfg_new(struct flashrom_wp_cfg **cfg)
{
	*cfg = calloc(1, sizeof(**cfg));
	return *cfg ? 0 : FLASHROM_WP_ERR_OTHER;
}

void flashrom_wp_cfg_release(struct flashrom_wp_cfg *cfg)
{
	free(cfg);
}

void flashrom_wp_set_mode(struct flashrom_wp_cfg *cfg, enum flashrom_wp_mode mode)
{
	cfg->mode = mode;
}

void flashrom_wp_set_range(struct flashrom_wp_cfg *cfg, size_t start, size_t len)
{
	cfg->range.start = start;
	cfg->range.len = len;
}

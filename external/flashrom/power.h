/*
 * This file is part of the flashrom project.
 *
 * Copyright (C) 2011 The Chromium OS Authors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 * power.h: header file for power management routines
 */

#ifndef __POWER_H__
#define __POWER_H__ 1

/* Disable power management. */
extern int disable_power_management(void);

/* Re-enable power management. */
extern int restore_power_management(void);

#endif	/* __POWER_H__ */

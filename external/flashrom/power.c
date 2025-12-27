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
 * power.c: power management routines
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

#include "flash.h"	/* for msg_* */
#include "power.h"

/*
 * Path of a lock file which flashrom's PID should be written to instruct
 * powerd not to suspend or shut down. powerd checks for arbitrary lock files
 * within /run/lock/power_override.
 */
#define POWERD_LOCK_FILE_PATH "/run/lock/power_override/flashrom.lock"

/* Files created by powerd to announce an imminent shutdown or suspend. */
#define POWERD_SUSPEND_ANNOUNCED_PATH  "/run/power_manager/power/suspend_announced"
#define POWERD_SHUTDOWN_ANNOUNCED_PATH  "/run/power_manager/power/shutdown_announced"

/*
 * Env var set by powerd's setuid helper. If running from powerd, ignore
 * suspend/shutdown announced files.
 */
#define POWERD_SETUID_HELPER_ENV "POWERD_SETUID_HELPER"

static bool check_suspend_imminent(void)
{
	if (getenv(POWERD_SETUID_HELPER_ENV)) {
		msg_pdbg("%s env var is set: skipping check for suspend/shutdown"
			 "announcment files.\n", POWERD_SETUID_HELPER_ENV);
		return false;
	}

	struct stat s;
	if ((stat(POWERD_SUSPEND_ANNOUNCED_PATH, &s) == 0) ||
	    (stat(POWERD_SHUTDOWN_ANNOUNCED_PATH, &s) == 0)) {
		msg_perr("Cannot disable power management, the system "
			 "is already preparing to suspend/shutdown. Aborting.\n");
		return true;
	}
	return false;
}

int disable_power_management(void)
{
	FILE *lock_file = NULL;
	int rc = 0;
	mode_t old_umask;

	msg_pdbg("%s: Disabling power management.\n", __func__);

	old_umask = umask(022);
	lock_file = fopen(POWERD_LOCK_FILE_PATH, "w");
	umask(old_umask);
	if (!lock_file) {
		msg_perr("%s: Failed to open %s for writing: %s\n",
			__func__, POWERD_LOCK_FILE_PATH, strerror(errno));
		return 1;
	}

	if (fprintf(lock_file, "%ld", (long)getpid()) < 0) {
		msg_perr("%s: Failed to write PID to %s: %s\n",
			__func__, POWERD_LOCK_FILE_PATH, strerror(errno));
		rc = 1;
	}

	if (fclose(lock_file) != 0) {
		msg_perr("%s: Failed to close %s: %s\n",
			__func__, POWERD_LOCK_FILE_PATH, strerror(errno));
	}

	/* Check after creating lock file to avoid race with powerd. */
	if (check_suspend_imminent()) {
		restore_power_management();
		return 2;
	}

	return rc;
}

int restore_power_management(void)
{
	int result = 0;

	msg_pdbg("%s: Re-enabling power management.\n", __func__);

	result = unlink(POWERD_LOCK_FILE_PATH);
	if (result != 0 && errno != ENOENT)  {
		msg_perr("%s: Failed to unlink %s: %s\n",
			__func__, POWERD_LOCK_FILE_PATH, strerror(errno));
		return 1;
	}
	return 0;
}

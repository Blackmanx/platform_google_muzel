/* Copyright 2012, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of Google Inc. nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdbool.h>

#include "big_lock.h"
#include "ipc_lock.h"
#include "flash.h"

#define LOCKFILE_NAME		"firmware_utility_lock"
static struct ipc_lock big_lock = LOCKFILE_INIT(LOCKFILE_NAME);
/** Big lock acquisition status. */
static bool g_big_lock_acquired = false;

int acquire_big_lock(int timeout_secs)
{
	msg_gdbg("Acquiring lock (timeout=%d sec)...\n", timeout_secs);
	int ret = acquire_lock(&big_lock, timeout_secs * 1000);
	if (ret < 0) {
		msg_gerr("Could not acquire lock.\n");
	} else {
		g_big_lock_acquired = true;
		msg_gdbg("Lock acquired.\n");
	}
	return ret;
}

int release_big_lock(void)
{
	int ret = 0;
	if (g_big_lock_acquired) {
		ret = release_lock(&big_lock);
		g_big_lock_acquired = false;
	}
	return ret;
}

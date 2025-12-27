/* SPDX-License-Identifier: MIT */
#ifndef LIBURING_COMPAT_H
#define LIBURING_COMPAT_H

#include <stdint.h>
#include <inttypes.h>
#include <linux/openat2.h>
#include <linux/ioctl.h>

typedef int __kernel_rwf_t;

#ifndef BLOCK_URING_CMD_DISCARD
#define BLOCK_URING_CMD_DISCARD                        _IO(0x12, 0)
#endif

#ifndef __ANDROID__

struct __kernel_timespec {
        int64_t         tv_sec;
        long long       tv_nsec;
};

#endif

/* <linux/time_types.h> is not available, so it can't be included */
#define UAPI_LINUX_IO_URING_H_SKIP_LINUX_TIME_TYPES_H 1

#endif

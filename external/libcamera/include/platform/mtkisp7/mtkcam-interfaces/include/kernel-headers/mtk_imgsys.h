/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ***   To edit the content of this header, modify the corresponding
 ***   source file (e.g. under external/kernel-headers/original/) then
 ***   run bionic/libc/kernel/tools/update_all.py
 ***
 ***   Any manual change here will be lost the next time this script will
 ***   be run. You've been warned!
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef _MTK_IMGSYS_H
#define _MTK_IMGSYS_H

#include <stddef.h>
#include <linux/ioctl.h>
#include <linux/v4l2-controls.h>

#define BASE_VIDIOC_PRIVATE 192
#define FD_MAX (32)

struct fd_info {
	uint8_t fd_num;
	unsigned int fds[FD_MAX];
	unsigned int fds_size[FD_MAX];
	unsigned long long fds_phys_addr[FD_MAX];
} __attribute__ ((__packed__));


#define MTKDIP_IOC_ADD_KVA _IOW('V', BASE_VIDIOC_PRIVATE + 8, struct fd_info)
#define MTKDIP_IOC_DEL_KVA _IOW('V', BASE_VIDIOC_PRIVATE + 9, struct fd_info)

struct fd_tbl {
	uint8_t fd_num;
	uint64_t fds;    // user address, point to unsigned int array
} __attribute__ ((__packed__));
#define MTKDIP_IOC_ADD_IOVA _IOW('V', BASE_VIDIOC_PRIVATE + 10, struct fd_tbl)
#define MTKDIP_IOC_DEL_IOVA _IOW('V', BASE_VIDIOC_PRIVATE + 11, struct fd_tbl)

struct sensor_info {
	uint16_t full_wd;
	uint16_t full_ht;
};

struct init_info {
	struct sensor_info sensor;
	uint32_t sec_tag;
	uint32_t is_smvr;
};

#define MTKDIP_IOC_S_INIT_INFO \
			_IOW('V', BASE_VIDIOC_PRIVATE + 12, struct init_info)

#define V4L2_CID_IMGSYS_OFFSET	(0xC000)
#define V4L2_CID_IMGSYS_BASE    (V4L2_CID_USER_BASE + V4L2_CID_IMGSYS_OFFSET)

#define V4L2_CID_IMGSYS_APU_DC  (V4L2_CID_IMGSYS_BASE + 1)

struct ctrl_info {
  uint32_t id;
  int32_t value;
} __attribute__ ((__packed__));

#define MTKDIP_IOC_SET_CONTROL _IOW('V', BASE_VIDIOC_PRIVATE + 13, struct ctrl_info)
#define MTKDIP_IOC_GET_CONTROL _IOWR('V', BASE_VIDIOC_PRIVATE + 14, struct ctrl_info)

#endif

# Copyright (C) 2025 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

HOST_TOOL_NAME := depmod
LOCAL_DIR := $(GET_LOCAL_DIR)

HOST_INCLUDE_DIRS += \
	$(LOCAL_DIR) \

HOST_SRCS := \
	$(LOCAL_DIR)/tools/depmod.c \
	$(LOCAL_DIR)/tools/kmod.c \
	$(LOCAL_DIR)/tools/modinfo.c \
	$(LOCAL_DIR)/tools/rmmod.c \
	$(LOCAL_DIR)/tools/log.c \
	$(LOCAL_DIR)/tools/modprobe.c \
	$(LOCAL_DIR)/tools/static-nodes.c \
	$(LOCAL_DIR)/tools/insmod.c \
	$(LOCAL_DIR)/tools/lsmod.c \
	$(LOCAL_DIR)/libkmod/libkmod.c \
	$(LOCAL_DIR)/libkmod/libkmod-builtin.c \
	$(LOCAL_DIR)/libkmod/libkmod-config.c \
	$(LOCAL_DIR)/libkmod/libkmod-elf.c \
	$(LOCAL_DIR)/libkmod/libkmod-file.c \
	$(LOCAL_DIR)/libkmod/libkmod-index.c \
	$(LOCAL_DIR)/libkmod/libkmod-list.c \
	$(LOCAL_DIR)/libkmod/libkmod-module.c \
	$(LOCAL_DIR)/libkmod/libkmod-signature.c \
	$(LOCAL_DIR)/shared/array.c \
	$(LOCAL_DIR)/shared/hash.c \
	$(LOCAL_DIR)/shared/scratchbuf.c \
	$(LOCAL_DIR)/shared/strbuf.c \
	$(LOCAL_DIR)/shared/util.c \

HOST_FLAGS := \
	-include android/port.h \
	-Wall \
	-Werror \
	-Wno-format \
	-Wno-unused-parameter \
	-Wno-unused-variable \
	-Dsecure_getenv=getenv \
	-DHAVE_CONFIG_H \
	-include config.h \
	-DHAVE_DECL_STRNDUPA \
	-DMODULE_DIRECTORY=\"/lib/modules\" \
	-DSYSCONFDIR=\"/tmp\" \
	-DDISTCONFDIR=\"/lib\" \
	-UNDEBUG \

HOST_STATIC_LINK := true

HOST_LIBS := c++

include make/host_tool.mk

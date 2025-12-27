#
# Copyright (C) 2011 The Android Open Source Project
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

LOCAL_PATH := $(call my-dir)

art_path := $(LOCAL_PATH)

include $(art_path)/build/Android.common_path.mk

########################################################################
# cpplint rules to style check art source files

include $(art_path)/build/Android.cpplint.mk

########################################################################
# product rules

ART_HOST_DEPENDENCIES := \
  $(ART_HOST_EXECUTABLES) \
  $(ART_HOST_DEX_DEPENDENCIES) \
  $(ART_HOST_SHARED_LIBRARY_DEPENDENCIES)

ifeq ($(ART_BUILD_HOST_DEBUG),true)
ART_HOST_DEPENDENCIES += $(ART_HOST_SHARED_LIBRARY_DEBUG_DEPENDENCIES)
endif

ART_TARGET_DEPENDENCIES := \
  $(ART_TARGET_DEX_DEPENDENCIES)

########################################################################
# test rules

# All the dependencies that must be built ahead of sync-ing them onto the target device.
TEST_ART_TARGET_SYNC_DEPS := $(ADB_EXECUTABLE)

include $(art_path)/build/Android.common_test.mk
include $(art_path)/build/Android.gtest.mk
include $(art_path)/test/Android.run-test.mk

TEST_ART_TARGET_SYNC_DEPS += $(ART_TEST_TARGET_GTEST_DEPENDENCIES) $(ART_TEST_TARGET_RUN_TEST_DEPENDENCIES)

# "mm test-art" to build and run all tests on host and device
.PHONY: test-art
test-art:
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-gtest
test-art-gtest:
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-run-test
test-art-run-test:
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

########################################################################
# host test rules

test-art: test-art-host
test-art-gtest: test-art-host-gtest
test-art-run-test: test-art-host-run-test

VIXL_TEST_DEPENDENCY :=
# We can only run the vixl tests on 64-bit hosts (vixl testing issue) when its a
# top-level build (to declare the vixl test rule).
ifneq ($(HOST_PREFER_32_BIT),true)
ifeq ($(ONE_SHOT_MAKEFILE),)
VIXL_TEST_DEPENDENCY := run-vixl-tests
endif
endif

.PHONY: test-art-host-vixl
test-art-host-vixl: $(VIXL_TEST_DEPENDENCY)

# "mm test-art-host" to build and run all host tests.
.PHONY: test-art-host
test-art-host: test-art-host-gtest test-art-host-run-test \
               test-art-host-vixl test-art-host-dexdump
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

# All host tests that run solely with the default compiler.
.PHONY: test-art-host-default
test-art-host-default: test-art-host-run-test-default
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

# All host tests that run solely with the optimizing compiler.
.PHONY: test-art-host-optimizing
test-art-host-optimizing: test-art-host-run-test-optimizing
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

# All host tests that run solely on the interpreter.
.PHONY: test-art-host-interpreter
test-art-host-interpreter: test-art-host-run-test-interpreter
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

# All host tests that run solely on the jit.
.PHONY: test-art-host-jit
test-art-host-jit: test-art-host-run-test-jit
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

# Primary host architecture variants:
.PHONY: test-art-host$(ART_PHONY_TEST_HOST_SUFFIX)
test-art-host$(ART_PHONY_TEST_HOST_SUFFIX): test-art-host-gtest$(ART_PHONY_TEST_HOST_SUFFIX) \
    test-art-host-run-test$(ART_PHONY_TEST_HOST_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-host-default$(ART_PHONY_TEST_HOST_SUFFIX)
test-art-host-default$(ART_PHONY_TEST_HOST_SUFFIX): test-art-host-run-test-default$(ART_PHONY_TEST_HOST_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-host-optimizing$(ART_PHONY_TEST_HOST_SUFFIX)
test-art-host-optimizing$(ART_PHONY_TEST_HOST_SUFFIX): test-art-host-run-test-optimizing$(ART_PHONY_TEST_HOST_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-host-interpreter$(ART_PHONY_TEST_HOST_SUFFIX)
test-art-host-interpreter$(ART_PHONY_TEST_HOST_SUFFIX): test-art-host-run-test-interpreter$(ART_PHONY_TEST_HOST_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-host-jit$(ART_PHONY_TEST_HOST_SUFFIX)
test-art-host-jit$(ART_PHONY_TEST_HOST_SUFFIX): test-art-host-run-test-jit$(ART_PHONY_TEST_HOST_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

# Secondary host architecture variants:
ifneq ($(HOST_PREFER_32_BIT),true)
.PHONY: test-art-host$(2ND_ART_PHONY_TEST_HOST_SUFFIX)
test-art-host$(2ND_ART_PHONY_TEST_HOST_SUFFIX): test-art-host-gtest$(2ND_ART_PHONY_TEST_HOST_SUFFIX) \
    test-art-host-run-test$(2ND_ART_PHONY_TEST_HOST_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-host-default$(2ND_ART_PHONY_TEST_HOST_SUFFIX)
test-art-host-default$(2ND_ART_PHONY_TEST_HOST_SUFFIX): test-art-host-run-test-default$(2ND_ART_PHONY_TEST_HOST_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-host-optimizing$(2ND_ART_PHONY_TEST_HOST_SUFFIX)
test-art-host-optimizing$(2ND_ART_PHONY_TEST_HOST_SUFFIX): test-art-host-run-test-optimizing$(2ND_ART_PHONY_TEST_HOST_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-host-interpreter$(2ND_ART_PHONY_TEST_HOST_SUFFIX)
test-art-host-interpreter$(2ND_ART_PHONY_TEST_HOST_SUFFIX): test-art-host-run-test-interpreter$(2ND_ART_PHONY_TEST_HOST_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-host-jit$(2ND_ART_PHONY_TEST_HOST_SUFFIX)
test-art-host-jit$(2ND_ART_PHONY_TEST_HOST_SUFFIX): test-art-host-run-test-jit$(2ND_ART_PHONY_TEST_HOST_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)
endif

# Dexdump/list regression test.
.PHONY: test-art-host-dexdump
test-art-host-dexdump: $(addprefix $(HOST_OUT_EXECUTABLES)/, dexdump dexlist)
	ANDROID_HOST_OUT=$(realpath $(HOST_OUT)) art/test/dexdump/run-all-tests

########################################################################
# target test rules

test-art: test-art-target
test-art-gtest: test-art-target-gtest
test-art-run-test: test-art-target-run-test

# "mm test-art-target" to build and run all target tests.
.PHONY: test-art-target
test-art-target: test-art-target-gtest test-art-target-run-test
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

# All target tests that run solely with the default compiler.
.PHONY: test-art-target-default
test-art-target-default: test-art-target-run-test-default
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

# All target tests that run solely with the optimizing compiler.
.PHONY: test-art-target-optimizing
test-art-target-optimizing: test-art-target-run-test-optimizing
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

# All target tests that run solely on the interpreter.
.PHONY: test-art-target-interpreter
test-art-target-interpreter: test-art-target-run-test-interpreter
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

# All target tests that run solely on the jit.
.PHONY: test-art-target-jit
test-art-target-jit: test-art-target-run-test-jit
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

# Primary target architecture variants:
.PHONY: test-art-target$(ART_PHONY_TEST_TARGET_SUFFIX)
test-art-target$(ART_PHONY_TEST_TARGET_SUFFIX): test-art-target-gtest$(ART_PHONY_TEST_TARGET_SUFFIX) \
    test-art-target-run-test$(ART_PHONY_TEST_TARGET_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-target-default$(ART_PHONY_TEST_TARGET_SUFFIX)
test-art-target-default$(ART_PHONY_TEST_TARGET_SUFFIX): test-art-target-run-test-default$(ART_PHONY_TEST_TARGET_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-target-optimizing$(ART_PHONY_TEST_TARGET_SUFFIX)
test-art-target-optimizing$(ART_PHONY_TEST_TARGET_SUFFIX): test-art-target-run-test-optimizing$(ART_PHONY_TEST_TARGET_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-target-interpreter$(ART_PHONY_TEST_TARGET_SUFFIX)
test-art-target-interpreter$(ART_PHONY_TEST_TARGET_SUFFIX): test-art-target-run-test-interpreter$(ART_PHONY_TEST_TARGET_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-target-jit$(ART_PHONY_TEST_TARGET_SUFFIX)
test-art-target-jit$(ART_PHONY_TEST_TARGET_SUFFIX): test-art-target-run-test-jit$(ART_PHONY_TEST_TARGET_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

# Secondary target architecture variants:
ifdef 2ND_ART_PHONY_TEST_TARGET_SUFFIX
.PHONY: test-art-target$(2ND_ART_PHONY_TEST_TARGET_SUFFIX)
test-art-target$(2ND_ART_PHONY_TEST_TARGET_SUFFIX): test-art-target-gtest$(2ND_ART_PHONY_TEST_TARGET_SUFFIX) \
    test-art-target-run-test$(2ND_ART_PHONY_TEST_TARGET_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-target-default$(2ND_ART_PHONY_TEST_TARGET_SUFFIX)
test-art-target-default$(2ND_ART_PHONY_TEST_TARGET_SUFFIX): test-art-target-run-test-default$(2ND_ART_PHONY_TEST_TARGET_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-target-optimizing$(2ND_ART_PHONY_TEST_TARGET_SUFFIX)
test-art-target-optimizing$(2ND_ART_PHONY_TEST_TARGET_SUFFIX): test-art-target-run-test-optimizing$(2ND_ART_PHONY_TEST_TARGET_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-target-interpreter$(2ND_ART_PHONY_TEST_TARGET_SUFFIX)
test-art-target-interpreter$(2ND_ART_PHONY_TEST_TARGET_SUFFIX): test-art-target-run-test-interpreter$(2ND_ART_PHONY_TEST_TARGET_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)

.PHONY: test-art-target-jit$(2ND_ART_PHONY_TEST_TARGET_SUFFIX)
test-art-target-jit$(2ND_ART_PHONY_TEST_TARGET_SUFFIX): test-art-target-run-test-jit$(2ND_ART_PHONY_TEST_TARGET_SUFFIX)
	$(hide) $(call ART_TEST_PREREQ_FINISHED,$@)
endif

#######################
# Reset LOCAL_PATH because previous includes may override its value.
# Keep this after all "include $(art_path)/..." are done, and before any
# "include $(BUILD_...)".
LOCAL_PATH := $(art_path)

########################################################################
# "m build-art" for quick minimal build
.PHONY: build-art

build-art: build-art-host

# For host, we extract the ICU data from the apex and install it to HOST_OUT/I18N_APEX.
$(HOST_I18N_DATA): $(TARGET_OUT)/apex/$(I18N_APEX).apex $(HOST_OUT)/bin/deapexer $(HOST_OUT)/bin/debugfs_static
	$(call extract-from-apex,$(I18N_APEX))
	rm -rf $(HOST_OUT)/$(I18N_APEX)
	mkdir -p $(HOST_OUT)/$(I18N_APEX)/
	cp -R $(TARGET_OUT)/apex/$(I18N_APEX)/etc/ $(HOST_OUT)/$(I18N_APEX)/
	touch $@

$(HOST_TZDATA_DATA): $(TARGET_OUT)/apex/$(TZDATA_APEX).apex $(HOST_OUT)/bin/deapexer $(HOST_OUT)/bin/debugfs_static
	$(call extract-from-apex,$(TZDATA_APEX))
	rm -rf $(HOST_OUT)/$(TZDATA_APEX)
	mkdir -p $(HOST_OUT)/$(TZDATA_APEX)/
	cp -R $(TARGET_OUT)/apex/$(TZDATA_APEX)/etc/ $(HOST_OUT)/$(TZDATA_APEX)/
	touch $@

.PHONY: build-art-host
build-art-host:   $(HOST_OUT_EXECUTABLES)/art $(ART_HOST_DEPENDENCIES) $(HOST_CORE_IMG_OUTS) $(HOST_I18N_DATA) $(HOST_TZDATA_DATA)

build-art: build-art-target

.PHONY: build-art-target
build-art-target: $(TARGET_OUT_EXECUTABLES)/art $(ART_TARGET_DEPENDENCIES) $(TARGET_CORE_IMG_OUTS)

TARGET_BOOT_IMAGE_SYSTEM_DIR := $(PRODUCT_OUT)/system/apex/art_boot_images
TARGET_ART_APEX_SYSTEM := $(PRODUCT_OUT)/system/apex/com.android.art
TARGET_BOOT_IMAGE_PROFILE := $(TARGET_ART_APEX_SYSTEM).testing/etc/boot-image.prof

.PHONY: build-art-simulator-profile
build-art-simulator-profile: $(HOST_OUT_EXECUTABLES)/profmand $(TARGET_CORE_IMG_DEX_FILES) \
		$(PRODUCT_DEX_PREOPT_BOOT_IMAGE_PROFILE_LOCATION)
	mkdir -p $(dir $(TARGET_BOOT_IMAGE_PROFILE))
	# Generate a profile from the core boot jars. This allows the simulator and boot image to use a
	# stable profile that is generated on the host.
	$(HOST_OUT_EXECUTABLES)/profmand \
	  --output-profile-type=boot \
	  --create-profile-from=$(PRODUCT_DEX_PREOPT_BOOT_IMAGE_PROFILE_LOCATION) \
	  $(foreach jar,$(TARGET_CORE_IMG_DEX_FILES),--apk=$(jar)) \
	  $(foreach jar,$(TARGET_CORE_IMG_DEX_LOCATIONS),--dex-location=$(jar)) \
	  --reference-profile-file=$(TARGET_BOOT_IMAGE_PROFILE)

.PHONY: build-art-simulator-boot-image
build-art-simulator-boot-image: $(HOST_OUT_EXECUTABLES)/generate-boot-image64 \
		$(HOST_OUT_EXECUTABLES)/dex2oatd $(TARGET_CORE_IMG_DEX_FILES) build-art-simulator-profile
	# Note: The target boot image needs to be in a trusted system directory to be used by the
	# zygote or if -Xonly-use-system-oat-files is passed to the runtime.
	rm -rf $(TARGET_BOOT_IMAGE_SYSTEM_DIR)
	mkdir -p $(TARGET_BOOT_IMAGE_SYSTEM_DIR)/javalib
	mkdir -p $(TARGET_ART_APEX_SYSTEM)/javalib
	# Copy the core boot jars to the expected directory for generate-boot-image.
	$(foreach i,$(call int_range_list, 1, $(words $(TARGET_CORE_IMG_JARS))), \
	  cp $(word $(i),$(TARGET_CORE_IMG_DEX_FILES)) \
	    $(TARGET_ART_APEX_SYSTEM)/javalib/$(word $(i),$(TARGET_CORE_IMG_JARS)).jar;)
	# Generate a target boot image using the host dex2oat. Note: a boot image using a profile is
	# required for certain run tests to pass.
	$(HOST_OUT_EXECUTABLES)/generate-boot-image64 \
	  --output-dir=$(TARGET_BOOT_IMAGE_SYSTEM_DIR)/javalib \
	  --compiler-filter=speed-profile \
	  --use-profile=true \
	  --profile-file=$(TARGET_BOOT_IMAGE_PROFILE) \
	  --dex2oat-bin=$(HOST_OUT_EXECUTABLES)/dex2oatd \
	  --android-root=$(TARGET_OUT) \
	  --android-root-for-location=true \
	  --core-only=true \
	  --instruction-set=$(TARGET_ARCH)

# For simulator, build a target profile and boot image on the host.
.PHONY: build-art-simulator
build-art-simulator: build-art-simulator-profile build-art-simulator-boot-image

# Extracts files from an APEX into a location. The APEX can be either a .apex or
# .capex file in $(TARGET_OUT)/apex, or a directory in the same location. Files
# are extracted to $(TARGET_OUT) with the same relative paths as under the APEX
# root.
# $(1): APEX base name
# $(2): List of files to extract, with paths relative to the APEX root
#
# "cp -d" below doesn't work on Darwin, but this is only used for tests and
# won't run on mac anyway.
define extract-from-apex
  apex_root=$(TARGET_OUT)/apex && \
  apex_file=$$apex_root/$(1).apex && \
  apex_dir=$$apex_root/$(1) && \
  if [ ! -f $$apex_file ]; then \
    apex_file=$$apex_root/$(1).capex; \
  fi && \
  if [ -f $$apex_file ]; then \
    rm -rf $$apex_dir && \
    mkdir -p $$apex_dir && \
    debugfs=$(HOST_OUT)/bin/debugfs_static && \
    fsckerofs=$(HOST_OUT)/bin/fsck.erofs && \
    $(HOST_OUT)/bin/deapexer --debugfs_path $$debugfs \
		--fsckerofs_path $$fsckerofs extract $$apex_file $$apex_dir; \
  fi && \
  for f in $(2); do \
    sf=$$apex_dir/$$f && \
    df=$(TARGET_OUT)/$$f && \
    if [ -f $$sf -o -h $$sf ]; then \
      mkdir -p $$(dirname $$df) && \
      cp -fd $$sf $$df; \
    fi || exit 1; \
  done
endef

########################################################################
# Rules for building all dependencies for tests.

ART_TARGET_PLATFORM_LIBS := \
  libcutils \
  libprocessgroup \
  libprocinfo \
  libselinux \
  libtombstoned_client \
  libz \

ART_TARGET_PLATFORM_DEPENDENCIES := \
  $(TARGET_OUT)/etc/public.libraries.txt \
  $(foreach lib,$(ART_TARGET_PLATFORM_LIBS), $(TARGET_OUT_SHARED_LIBRARIES)/$(lib).so)
ifdef TARGET_2ND_ARCH
ART_TARGET_PLATFORM_DEPENDENCIES += \
  $(foreach lib,$(ART_TARGET_PLATFORM_LIBS), $(2ND_TARGET_OUT_SHARED_LIBRARIES)/$(lib).so)
endif

.PHONY: build-art-host-gtests build-art-host-run-tests build-art-host-tests

build-art-host-gtests: build-art-host $(ART_TEST_HOST_GTEST_DEPENDENCIES)

build-art-host-run-tests: build-art-host \
                          $(TEST_ART_RUN_TEST_DEPENDENCIES) \
                          $(ART_TEST_HOST_RUN_TEST_DEPENDENCIES) \
                          art-run-test-host-data \
                          art-run-test-jvm-data

build-art-host-tests: build-art-host-gtests build-art-host-run-tests

.PHONY: build-art-target-gtests build-art-target-run-tests build-art-target-tests

build-art-target-gtests: build-art-target \
                         $(ART_TEST_TARGET_GTEST_DEPENDENCIES) \
                         $(ART_TARGET_PLATFORM_DEPENDENCIES)

build-art-target-run-tests: build-art-target \
                            $(TEST_ART_RUN_TEST_DEPENDENCIES) \
                            $(ART_TEST_TARGET_RUN_TEST_DEPENDENCIES) \
                            $(ART_TARGET_PLATFORM_DEPENDENCIES) \
                            art-run-test-target-data

build-art-target-tests: build-art-target-gtests build-art-target-run-tests

########################################################################

# Clear locally used variables.
TEST_ART_TARGET_SYNC_DEPS :=

# These files only exist if this flag is off. WITH_DEXPREOPT_ART_BOOT_IMG_ONLY is the
# minimal dexpreopt mode we use on eng builds for build speed.
ifneq ($(WITH_DEXPREOPT_ART_BOOT_IMG_ONLY),true)

# Helper target that depends on boot image creation.
#
# Can be used, for example, to dump initialization failures:
#   m art-boot-image ART_BOOT_IMAGE_EXTRA_ARGS=--dump-init-failures=fails.txt
.PHONY: art-boot-image
art-boot-image:  $(DEXPREOPT_IMAGE_boot_$(TARGET_ARCH))

.PHONY: art-job-images
art-job-images: \
  art-boot-image \
  $(2ND_DEFAULT_DEX_PREOPT_BUILT_IMAGE_FILENAME) \
  $(HOST_OUT_EXECUTABLES)/dex2oats \
  $(HOST_OUT_EXECUTABLES)/dex2oatds \
  $(HOST_OUT_EXECUTABLES)/profman

endif # TARGET_BUILD_VARIANT == eng

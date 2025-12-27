#!/bin/bash
#
# Copyright (C) 2020 The Android Open Source Project
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
# This script creates the framework boot image and system server profiles
# (suitable to include in the platform build).
# The input to the script are:
#   1) the boot.zip file which contains the boot classpath and system server jars.
#      This file can be obtained from running `m dist` or by configuring the device with
#      the `art/tools/boot-image-profile-configure-device.sh` script.
#   2) the preloaded classes denylist which specify what classes should not be preloaded
#      in Zygote. Usually located in frameworks/base/config/preloaded-classes-denylist
#   3) a list of raw boot image profiles extracted from devices. An example how to do that is
#      by running `art/tools/boot-image-profile-extract-profile.sh` script.
#
# It is strongly recommended that you make use of extensive critical user journeys flows in order
# to capture the raw boot image profiles described in #3.
#
# NOTE: by default, the script uses default arguments for producing the boot image profiles.
# You might want to adjust the default generation arguments based on the shape of profile
# and based on the metrics that matter for each product.
#

if [[ -z "$ANDROID_BUILD_TOP" ]]; then
  echo "You must run on this after running envsetup.sh and launch target"
  exit 1
fi

if [[ "$#" -lt 4 ]]; then
  echo "Usage $0 <output-dir> <boot.zip-location> <preloaded-denylist-location> <profile-input1> <profile-input2> ... <profman args>"
  echo "Without any profman args the script will use defaults."
  echo "Example: $0 output-dir boot.zip frameworks/base/config/preloaded-classes-denylist android1.prof android2.prof"
  echo "         $0 output-dir boot.zip frameworks/base/config/preloaded-classes-denylist android.prof --profman-arg --upgrade-startup-to-hot=true"
  echo "preloaded-deny-list-location is usually frameworks/base/config/preloaded-classes-denylist"
  exit 1
fi

echo "Creating work dir"
WORK_DIR=/tmp/android-bcp
mkdir -p "$WORK_DIR"

OUT_DIR="$1"
BOOT_ZIP="$2"
PRELOADED_DENYLIST="$3"
shift 3

# Read the profile input args.
profman_profile_input_args=()
boot_image_profman_args=()
system_server_profman_args=()
while [[ "$#" -ge 1 ]]; do
  # Read the profman args.
  if [[ "$#" -ge 2 ]]; then
    if [[ "$1" = '--profman-arg' ]]; then
      boot_image_profman_args+=("$2")
      system_server_profman_args+=("$2")
      shift 2
      continue
    fi

    if [[ "$1" = '--boot-image-profman-arg' ]]; then
      boot_image_profman_args+=("$2")
      shift 2
      continue
    fi

    if [[ "$1" = '--system-server-profman-arg' ]]; then
      system_server_profman_args+=("$2")
      shift 2
      continue
    fi
  fi
  profman_profile_input_args+=("--profile-file=$1")
  shift
done

OUT_BOOT_PROFILE="$OUT_DIR"/boot-image-profile.txt
OUT_PRELOADED_CLASSES="$OUT_DIR"/preloaded-classes
OUT_SYSTEM_SERVER="$OUT_DIR"/art-profile
ART_JARS=("core-oj.jar core-libart.jar okhttp.jar bouncycastle.jar apache-xml.jar")

echo "Changing dirs to the build top"
cd "$ANDROID_BUILD_TOP"

echo "Unziping boot.zip"
BOOT_UNZIP_DIR="$WORK_DIR"/boot-dex
BOOT_JARS="$BOOT_UNZIP_DIR"/dex_bootjars_input
SYSTEM_SERVER_JAR="$BOOT_UNZIP_DIR"/system/framework/services.jar

unzip -o "$BOOT_ZIP" -d "$BOOT_UNZIP_DIR"

echo "Processing boot image jar files"
jar_args=()
for entry in "$BOOT_JARS"/*
do
  # Ignore ART jars, since we want fromework related jars only.
  jar_name=$(basename "$entry")
  if [[ " ${ART_JARS[*]} " != *\ ${jar_name}\ * ]]; then
    jar_args+=("--apk=$entry")
  fi
done

echo "Running profman for boot image profiles"
# NOTE:
# You might want to adjust the default generation arguments based on the data
# For example, to update the selection thresholds you could specify:
# - For boot image profile:
#  --boot-image-profman-arg --method-threshold=10 \
#  --boot-image-profman-arg --class-threshold=10 \
#  --boot-image-profman-arg --preloaded-class-threshold=10 \
#  --boot-image-profman-arg --special-package=android:1 \
#  --boot-image-profman-arg --special-package=com.android.systemui:1 \
# - For System Server:
#  --system-server-profman-arg --method-threshold=10 \
#  --system-server-profman-arg --class-threshold=10 \
#  --system-server-profman-arg --special-package=android:1 \
#  --system-server-profman-arg --special-package=com.android.systemui:1 \
# You can use --profman-arg to specify the arguments for both boot image
# and system server.
#
# The threshold is percentage of total aggregation, that is, a method/class is
# included in the profile only if it's used by at least x% of the packages.
# (from 0% - include everything to 100% - include only the items that
# are used by all packages on device).
# The --special-package allows you to give a prioriority to certain packages,
# meaning, if the methods is used by that package then the algorithm will use a
# different selection thresholds.
# (system server is identified as the "android" package)
if [[ "${#boot_image_profman_args[*]}" -eq 0 ]]; then
  boot_image_profman_args+=("--special-package=android:1")
  boot_image_profman_args+=("--special-package=com.android.systemui:1")
fi
boot_image_profman_args+=("${jar_args[@]}")
profman \
  --generate-boot-image-profile \
  "${profman_profile_input_args[@]}" \
  --out-profile-path="$OUT_BOOT_PROFILE" \
  --out-preloaded-classes-path="$OUT_PRELOADED_CLASSES" \
  --preloaded-classes-denylist="$PRELOADED_DENYLIST" \
  "${boot_image_profman_args[@]}"

echo "Done boot image profile"

echo "Running profman for system server"
# We don't have a preloaded-classes nor a denylist files for System Server, so we ignore the arguments.
# We also set the thresholds to 0 to include everything if no args are specified.
if [[ "${#system_server_profman_args[*]}" -eq 0 ]]; then
  system_server_profman_args+=("--method-threshold=0")
  system_server_profman_args+=("--class-threshold=0")
fi
profman \
  --generate-boot-image-profile \
  "${profman_profile_input_args[@]}" \
  --out-profile-path="$OUT_SYSTEM_SERVER" \
  --apk="$SYSTEM_SERVER_JAR" \
  "${system_server_profman_args[@]}"

echo "Done system server"

echo ""
echo "Boot profile methods+classes count:          $(wc -l $OUT_BOOT_PROFILE)"
echo "Preloaded classes count:                     $(wc -l $OUT_PRELOADED_CLASSES)"
echo "System server profile methods+classes count: $(wc -l $OUT_SYSTEM_SERVER)"

CLEAN_UP="${CLEAN_UP:-true}"
if [[ "$CLEAN_UP" = "true" ]]; then
  rm -rf "$WORK_DIR"
fi

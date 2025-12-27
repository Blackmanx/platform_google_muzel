#!/bin/bash
#
# Copyright (C) 2025 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Declare a map of relative paths from the libcore directory to checkstyle
# configuration files to apply.
#
# This script removes all instances of @android.annotation.FlaggedApi(FLAG_NAME)

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <FLAG_NAME>"
    exit 1
fi

if [[ -z "${ANDROID_BUILD_TOP}" ]]
then
  die "ANDROID_BUILD_TOP not found. You need to run lunch first."
fi

FLAG_NAME=$1

find ${ANDROID_BUILD_TOP}/libcore/ojluni/annotations/flagged_api/ -type f -name "*.java" -exec sed -i "/@\(android.annotation.\)\?FlaggedApi($FLAG_NAME)/d" {} +
find ${ANDROID_BUILD_TOP}/libcore/luni/src/main/java/ -type f -name "*.java" -exec sed -i "/@\(android.annotation.\)\?FlaggedApi($FLAG_NAME)/d" {} +

echo "Done."

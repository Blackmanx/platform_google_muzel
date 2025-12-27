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

# This script scans for enabled aconfig flags for a given release and processes them.
# It is designed to be run from the root of the Android source tree.

set -e

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <RELEASE_NAME>"
    echo "Example: $0 bp2a"
    exit 1
fi

RELEASE_NAME="$1"
SCRIPT_DIR=$(dirname "$0")
# Assuming the script is in libcore/tools/ojluni-annotations and run from the project root.
PROJECT_ROOT=$(pwd)
ACONFIG_DIR="$PROJECT_ROOT/build/release/aconfig/$RELEASE_NAME/com.android.libcore/"
REMOVE_API_SCRIPT="$PROJECT_ROOT/libcore/tools/ojluni-annotations/remove_flagged_api.sh"

if [ ! -d "$ACONFIG_DIR" ]; then
    echo "Error: Aconfig directory not found for release '$RELEASE_NAME' at: $ACONFIG_DIR"
    exit 1
fi

if [ ! -f "$REMOVE_API_SCRIPT" ]; then
    echo "Error: The remove_flagged_api.sh script was not found at: $REMOVE_API_SCRIPT"
    exit 1
fi

echo "Scanning for enabled flags in: $ACONFIG_DIR"

# Find all .textproto files, then use awk to find the 'name' associated with 'state: ENABLED'.
# This handles multi-line blocks where name and state are not on the same line.
find "$ACONFIG_DIR" -name "*.textproto" -exec awk '
    /name:/ { current_name = $2; gsub(/"/, "", current_name) }
    /state: ENABLED/ { if (current_name != "") { print current_name; current_name = "" } }
' {} + | while read -r flag; do
    if [ -z "$flag" ]; then
        continue
    fi

    echo "Found enabled flag: '$flag'"

    # Transform the flag name to the required format, e.g.,
    # "native_metrics" -> "com.android.libcore.Flags.FLAG_NATIVE_METRICS"
    UPPERCASE_FLAG=$(echo "$flag" | tr 'a-z' 'A-Z')
    FULL_FLAG_NAME="com.android.libcore.Flags.FLAG_$UPPERCASE_FLAG"

    echo "Executing: $REMOVE_API_SCRIPT $FULL_FLAG_NAME"
    "$REMOVE_API_SCRIPT" "$FULL_FLAG_NAME"
    echo "--------------------------------------------------"
done

echo "@FlaggedApi annotations are removed."

echo "Start regenerating the API files, e.g. libcore/api/current.txt"
m art.module.public.api.stubs.source-update-current-api art.module.public.api.stubs.source.module_lib-update-current-api



#
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
#!/bin/bash

# This script automates the setup of a local Compiler Explorer instance
# using compilers built from the local Android source tree.
# It is based on the guide at art/tools/compiler-explorer/compiler-explorer.md.

set -e

# Prerequisite Check:
# In an Android checkout, build/envsetup.sh and lunch set these variables.
if [[ -z "$ANDROID_BUILD_TOP" || -z "$ANDROID_HOST_OUT" || -z "$ANDROID_PRODUCT_OUT" ]]; then
    echo "ERROR: Build environment is not set up correctly." >&2
    echo "Please run 'source build/envsetup.sh' and 'lunch' in your Android source tree." >&2
    exit 1
fi

# Prerequisite check for Node.js
if ! command -v node &> /dev/null; then
    echo "ERROR: Node.js not found. Compiler Explorer requires Node.js." >&2
    echo "Please install it, for example using Node Version Manager (nvm)," >&2
    echo "or ensure that the 'node' executable is in your PATH." >&2
    echo "Refer to art/tools/compiler-explorer/compiler-explorer.md for more details." >&2
    exit 1
fi

# The script should be run from the root of the Android source tree.
cd "$ANDROID_BUILD_TOP"
echo "Running from Android source tree: $PWD"

# Determine default instruction sets based on lunch target's TARGET_ARCH.
# This needs to be done early so show_help can display the correct default.
LUNCH_TARGET_ARCH_VAR=$(build/soong/soong_ui.bash --dumpvars-mode --vars=TARGET_ARCH)
LUNCH_TARGET_ARCH=$(echo "$LUNCH_TARGET_ARCH_VAR" | cut -d'=' -f2 | tr -d "'")

# Map TARGET_ARCH to a set of instruction sets for boot image generation.
declare -a DEFAULT_INSTRUCTION_SETS # Declare as array to be safe
case "$LUNCH_TARGET_ARCH" in
  "arm")
    DEFAULT_INSTRUCTION_SETS=("arm")
    ;;
  "arm64")
    DEFAULT_INSTRUCTION_SETS=("arm" "arm64")
    ;;
  "x86")
    DEFAULT_INSTRUCTION_SETS=("x86")
    ;;
  "x86_64")
    DEFAULT_INSTRUCTION_SETS=("x86" "x86_64")
    ;;
  "riscv64")
    DEFAULT_INSTRUCTION_SETS=("riscv64")
    ;;
  *)
    echo "WARNING: Could not determine TARGET_ARCH from lunch target ('$LUNCH_TARGET_ARCH'). Defaulting to all common instruction sets." >&2
    DEFAULT_INSTRUCTION_SETS=("arm" "arm64" "x86" "x86_64" "riscv64")
    ;;
esac

# Default directory to install Compiler Explorer.
# Can be overridden with the --dir flag.
# We default to a directory inside the Android out directory to ensure it's within the workspace.
DEFAULT_COMPILER_EXPLORER_DIR="$ANDROID_BUILD_TOP/out/compiler-explorer"
COMPILER_EXPLORER_DIR="$DEFAULT_COMPILER_EXPLORER_DIR"
INSTRUCTION_SETS=("${DEFAULT_INSTRUCTION_SETS[@]}")

# Flags for script behavior
ACTION_FULL_SETUP=false
ACTION_REBUILD_DEX2OAT=false
ACTION_RESTART=false
SKIP_COMPILER_BUILD=false

# Helper Functions

function verify_compiler_paths() {
  echo "### Verifying compiler paths ###"
  local config_dir="$1"
  local has_error=false

  # Get all lines with path assignments from the config files. We use mapfile
  # to read the lines into an array to avoid subshell scoping issues with the
  # 'has_error' variable. We add '|| true' to the grep command to prevent the
  # script from exiting if grep finds no matches.
  local lines_to_check
  mapfile -t lines_to_check < <(grep -r -E "=${COMPILER_EXPLORER_DIR}" "$config_dir" || true)

  for line in "${lines_to_check[@]}"; do
    local file="${line%%:*}"
    local assignment="${line#*:}"
    local path="${assignment#*=}"

    # Strip potential quotes.
    path=${path//\"/}
    if [[ ! -e "$path" ]]; then
      echo "ERROR: Missing compiler artifact in '$file'." >&2
      echo "  Path specified in config: $path" >&2
      echo "  This file or directory does not exist." >&2
      echo "  This could be due to a build failure, a copy error, or an incorrect path in the .properties files." >&2
      has_error=true
    fi
  done

  if $has_error; then
    echo "ERROR: Path verification failed. Please check the errors above and try again." >&2
    exit 1
  fi
  echo "### All compiler paths verified successfully. ###"
}

function show_help() {
  echo "Usage: $0 [options]"
  echo
  echo "Automates the setup and management of a local Compiler Explorer instance."
  echo
  echo "Options:"
  echo "  --dir <path>                Set the directory for Compiler Explorer installation."
  echo "                              (Default: $DEFAULT_COMPILER_EXPLORER_DIR)"
  echo "  --instruction-sets \"<set>\"  Space-separated list of instruction sets for boot images."
  echo "                              (Default: Derived from lunch target's TARGET_ARCH, currently: \"${INSTRUCTION_SETS[*]}\")"
  echo "  -h, --help                  Show this help message."
  echo
  echo "Actions:"
  echo "  --rebuild-dex2oat           Cleanly rebuilds and copies dex2oat."
  echo "  --restart                   Stop and restart the Compiler Explorer server."
  echo "  --skip-compiler-build       Skip the 'm' build command. Useful for re-running copy/generation steps."
  echo
  echo "If no action is specified, the script will perform a full, one-time setup."
}

# Argument Parsing
while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
    --dir)
      COMPILER_EXPLORER_DIR="$2"
      shift; shift
      ;;
    --instruction-sets)
      INSTRUCTION_SETS=($2)
      shift; shift
      ;;
    --rebuild-dex2oat)
      ACTION_REBUILD_DEX2OAT=true
      shift
      ;;
    --restart)
      ACTION_RESTART=true
      shift
      ;;
    --skip-compiler-build)
      SKIP_COMPILER_BUILD=true
      shift
      ;;
    -h|--help)
      show_help
      exit 0
      ;;
    *)
      echo "Unknown option: $1"
      show_help
      exit 1
      ;;
  esac
done

# Determine the main action if no specific action flag was provided.
if ! $ACTION_REBUILD_DEX2OAT && ! $ACTION_RESTART; then
  ACTION_FULL_SETUP=true
fi

# A single variable to hold all build targets for the 'm' command.
BUILD_TARGETS=""

# Set up directories and check out Compiler Explorer.
# This is part of the one-time full setup.
if $ACTION_FULL_SETUP; then
  echo "### Creating directory and checking out Compiler Explorer ###"
  echo "Compiler Explorer installation directory: $COMPILER_EXPLORER_DIR"
  mkdir -p "$COMPILER_EXPLORER_DIR"
  if [ -d "$COMPILER_EXPLORER_DIR/compiler-explorer" ]; then
    echo "Compiler Explorer directory already exists. Skipping clone."
  else
    (cd "$COMPILER_EXPLORER_DIR" && git clone https://github.com/compiler-explorer/compiler-explorer.git)
  fi
fi



# Build Compiler Explorer.
# This is part of the one-time full setup.
if $ACTION_FULL_SETUP; then
  echo "### Building Compiler Explorer frontend ###"
  (cd "$COMPILER_EXPLORER_DIR/compiler-explorer" && make prebuild)
fi

# Determine what needs to be built.
# We collect all targets and run 'm' only once.
echo "### Determining build targets... ###"
if $ACTION_FULL_SETUP; then
  # Full setup builds everything.
  # r8 and smali-baksmali for d8/baksmali, 'dist out/dist/art_release.zip' for dex2oat, 'generate-boot-image' for boot images.
  BUILD_TARGETS="r8 smali-baksmali dist out/dist/art_release.zip generate-boot-image"
elif $ACTION_REBUILD_DEX2OAT; then
  # Rebuilding dex2oat only requires the 'dist out/dist/art_release.zip' target for art_release.zip.
  BUILD_TARGETS="dist out/dist/art_release.zip generate-boot-image"
fi

if [[ -n "$BUILD_TARGETS" ]] && ! $SKIP_COMPILER_BUILD; then
  echo "### Building Android components: $BUILD_TARGETS ###"
  m $BUILD_TARGETS
elif $SKIP_COMPILER_BUILD; then
  echo "### Skipping Android component build as requested. ###"
else
  echo "### No Android components need to be built for the requested action. ###"
fi

# Build and copy compilers except dex2oat.
# This is part of the one-time full setup.
if $ACTION_FULL_SETUP; then
  echo "### Copying compilers (Java, Kotlin, D8, Baksmali) ###"
  rm -rf "$COMPILER_EXPLORER_DIR/compilers"
  mkdir -p "$COMPILER_EXPLORER_DIR/compilers"
  # Note: The guide uses jdk21, but some checkouts might have other versions.
  # We find the latest available JDK prebuilt.
  JDK_PATH=$(find prebuilts/jdk/ -name "linux-x86" -type d | sort -V | tail -n 1)
  if [[ -z "$JDK_PATH" ]]; then
    echo "ERROR: Could not find a prebuilt JDK."
    exit 1
  fi
  echo "Found JDK at: $JDK_PATH"
  cp -r "$JDK_PATH" "$COMPILER_EXPLORER_DIR/compilers/java-local"
  cp -r "external/kotlinc" "$COMPILER_EXPLORER_DIR/compilers/kotlinc-local"
  mkdir -p "$COMPILER_EXPLORER_DIR/compilers/d8-local"
  cp "out/host/linux-x86/framework/r8.jar" "$COMPILER_EXPLORER_DIR/compilers/d8-local/r8.jar"
  unzip "$COMPILER_EXPLORER_DIR/compilers/d8-local/r8.jar" r8-version.properties -d "$COMPILER_EXPLORER_DIR/compilers/d8-local/"
  chmod +x "$COMPILER_EXPLORER_DIR/compilers/d8-local/r8.jar"
  mkdir -p "$COMPILER_EXPLORER_DIR/compilers/baksmali-local"
  cp "out/host/linux-x86/framework/smali-baksmali.jar" "$COMPILER_EXPLORER_DIR/compilers/baksmali-local"
fi

if $ACTION_FULL_SETUP || $ACTION_REBUILD_DEX2OAT; then
  echo "### Preparing ART host tools ###"
  DEX2OAT_DIR="$COMPILER_EXPLORER_DIR/compilers/dex2oat-local"
  echo "Cleaning up old dex2oat directory: $DEX2OAT_DIR"
  rm -rf "$DEX2OAT_DIR"
  echo "Unzipping art_release.zip to get ART artifacts..."
  unzip -q -d "$DEX2OAT_DIR" "out/dist/art_release.zip"

  # Find the host-specific bin directory containing dex2oat64.
  HOST_BIN_DIR=$(find "$DEX2OAT_DIR" -type f -name "dex2oat64" -exec dirname {} \; | head -n 1)
  if [[ -z "$HOST_BIN_DIR" ]]; then
    echo "ERROR: Could not find dex2oat host binaries in art_release.zip." >&2
    exit 1
  fi
  echo "Found host binaries in: $HOST_BIN_DIR"

  # Move binaries to a consistent, top-level bin/ directory for simplicity.
  echo "Moving host binaries to a top-level bin/ directory..."
  mkdir -p "$DEX2OAT_DIR/bin"
  mv "$HOST_BIN_DIR"/* "$DEX2OAT_DIR/bin/"

  # Clean up the now-empty platform-specific directories.
  find "$DEX2OAT_DIR" -mindepth 1 -maxdepth 1 -type d ! -name "bin" ! -name "bootjars" -exec rm -rf {} +
fi

# Generate boot images.
if $ACTION_FULL_SETUP || $ACTION_REBUILD_DEX2OAT; then
  echo "### Generating boot images ###"
  DEX2OAT_DIR="$COMPILER_EXPLORER_DIR/compilers/dex2oat-local"
  APP_DIR="$DEX2OAT_DIR/app"
  # Although this step is not required, having boot images makes dex2oat generate better code.
  echo "Preparing directories for boot image generation..."
  rm -rf "$APP_DIR"
  mkdir -p "$APP_DIR/apex/com.android.art/javalib"
  cp $DEX2OAT_DIR/bootjars/* "$APP_DIR/apex/com.android.art/javalib"
  mkdir -p "$APP_DIR/system/framework"

  echo "Generating boot images for instruction sets: ${INSTRUCTION_SETS[*]}"
  for instruction_set in "${INSTRUCTION_SETS[@]}"; do
    echo "Generating for $instruction_set..."
  # The generate-boot-image script is architecture-dependent, but the 64-bit version can
  # generate for all architectures.
    "$ANDROID_HOST_OUT/bin/generate-boot-image64" \
        --output-dir="$APP_DIR/system/framework" \
        --compiler-filter=speed \
        --use-profile=false \
        --dex2oat-bin="$DEX2OAT_DIR/bin/dex2oat64" \
        --android-root="$APP_DIR" \
        --core-only=true \
        --instruction-set="$instruction_set" \
        -- \
        --runtime-arg \
        -Xgc:CMC
  done
fi

# Configure and start Compiler Explorer server.
# This runs on full setup or when restarting.
if $ACTION_FULL_SETUP || $ACTION_RESTART; then
  echo "### Configuring Compiler Explorer ###"
  CE_CONFIG_DIR="$COMPILER_EXPLORER_DIR/compiler-explorer/etc/config"
  mkdir -p "$CE_CONFIG_DIR"
  cp art/tools/compiler-explorer/config/* "$CE_CONFIG_DIR"
  # Replace placeholders in the config files with the actual paths.
  echo "Setting paths in config files..."
  find "$CE_CONFIG_DIR" -type f -name '*.local.properties' -print0 | xargs -0 sed -i \
    -e "s?{{compilersDir}}?$COMPILER_EXPLORER_DIR/compilers?g"

  # After setting paths, verify that they all point to existing files/directories.
  verify_compiler_paths "$CE_CONFIG_DIR"

  echo "### Starting Compiler Explorer server ###"
  echo "To stop the server, press Ctrl+C in the terminal where it's running."
  # Attempt to stop any running server before starting.
  (cd "$COMPILER_EXPLORER_DIR/compiler-explorer" && make stop &> /dev/null || true)
  (cd "$COMPILER_EXPLORER_DIR/compiler-explorer" && make run)
fi

echo "### Script finished. ###"
if $ACTION_FULL_SETUP; then
  echo "Your local Compiler Explorer should be running at http://localhost:10240/"
fi
if $ACTION_REBUILD_DEX2OAT; then
  echo "Rebuild complete. Please restart the server with the --restart flag to apply changes."
fi
if $ACTION_RESTART; then
  echo "Compiler Explorer restarted."
fi

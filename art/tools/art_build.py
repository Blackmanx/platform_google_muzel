#!/usr/bin/env python3
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

import argparse
import os
import shlex
import shutil
import subprocess
import sys
from typing import Callable, Dict, List, Optional


I18N_APEX = "com.android.i18n"
TZDATA_APEX = "com.android.tzdata"
CORE_IMG_JARS: List[str] = [
    "core-oj",
    "core-libart",
    "okhttp",
    "bouncycastle",
    "apache-xml",
]
HOSTDEX_JARS: List[tuple[str, str]] = [
    ("com.android.conscrypt", "conscrypt"),
    ("com.android.i18n", "core-icu4j"),
]
ART_CORE_EXECUTABLES: List[str] = [
    "dalvikvm",
    "dexlist",
]
ART_CORE_DEBUGGABLE_EXECUTABLES_COMMON: List[str] = [
    "dex2oat",
    "dexoptanalyzer",
    "imgdiag",
    "oatdump",
    "profman",
]
ART_CORE_DEBUGGABLE_EXECUTABLES_HOST: List[str] = (
    ART_CORE_DEBUGGABLE_EXECUTABLES_COMMON
)
ART_CORE_SHARED_LIBRARIES: List[str] = [
    "libjavacore",
    "libopenjdk",
    "libopenjdkjvm",
    "libopenjdkjvmti",
    "libjdwp",
]
ART_CORE_SHARED_DEBUG_LIBRARIES: List[str] = [
    "libopenjdkd",
    "libopenjdkjvmd",
    "libopenjdkjvmtid",
]
ART_HOST_CORE_SHARED_LIBRARIES: List[str] = ART_CORE_SHARED_LIBRARIES + [
    "libicuuc-host",
    "libicui18n-host",
    "libicu_jni",
]
# Define a more specific type for build variables, which are strings.
BuildVarsDict = Dict[str, str]
# A list of build variables that are essential for this script.
# The script will exit if these variables are not found or are empty.
REQUIRED_BUILD_VARS: List[str] = [
    "OUT_DIR",
    "HOST_OUT",
    "HOST_OUT_EXECUTABLES",
    "HOST_OUT_JAVA_LIBRARIES",
    "HOST_OUT_SHARED_LIBRARIES",
    "PRODUCT_OUT",
    "TARGET_OUT",
    "TARGET_ARCH",
    "TARGET_OUT_SHARED_LIBRARIES",
]
# A list of optional build variables. These are retrieved but not required
# to be present in the build environment.
OPTIONAL_BUILD_VARS: List[str] = [
    "HOST_2ND_ARCH",
    "2ND_HOST_OUT_SHARED_LIBRARIES",
    "TARGET_2ND_ARCH",
    "2ND_TARGET_OUT_SHARED_LIBRARIES",
    "PRODUCT_DEX_PREOPT_BOOT_IMAGE_PROFILE_LOCATION",
]
# Platform libraries that must be specified through their full target paths,
# because the prebuilts used on master-art don't have all the variants to allow
# using their name-only build targets.
ART_TARGET_PLATFORM_LIBS_WITH_FULL_PATH: List[str] = [
    "libcutils",
    "libprocinfo",
    "libprocessgroup",
    "libselinux",
    "libtombstoned_client",
    "libz",
]


def using_thin_manifest():
  """Returns true if the source tree is checked out from a thin manifest."""
  return not os.path.isdir("frameworks/base")


def run_subprocess(
    command: List[str],
    cwd: Optional[str] = None,
    env: Optional[Dict[str, str]] = None,
    capture_stdout: bool = False,
) -> subprocess.CompletedProcess:
  """Runs a subprocess command (always with shell=False). Exits on failure.

  Args:
      command: The command to execute as a list of strings. The first element is
        the program, subsequent are arguments.
      cwd: The current working directory for the subprocess.
      env: Environment variables to set for the subprocess.
      capture_stdout: If True, stdout will be captured. stderr passes through.
        Defaults to False (both stdout/stderr pass through).
  """
  current_env = os.environ.copy()
  if env:
    current_env.update(env)

  stdout_setting = subprocess.PIPE if capture_stdout else None
  stderr_setting = None

  try:
    # The command is constructed from trusted sources (hardcoded values
    # or build system variables). With shell=False, this call is not
    # vulnerable to command injection.
    # nosemgrep: googleplex-android.python.lang.security.audit.dangerous-subprocess-use-audit
    result = subprocess.run(
        command,
        cwd=cwd,
        env=current_env,
        shell=False,
        stdout=stdout_setting,
        stderr=stderr_setting,
        text=True,
        check=True,
    )
    return result
  except subprocess.CalledProcessError as e:
    print(f"Error: Command failed with return code {e.returncode}")
    print(f"  Command: {shlex.join(e.cmd)}")
    if capture_stdout:
      if e.stdout:
        print(f"  Stdout (captured): {e.stdout.strip()}")
      print("  (Stderr from the subprocess should have already printed")
      print("   to the console above.)")
    else:
      print("  (Stdout and stderr from the subprocess should have already")
      print("   printed to the console above.)")
    sys.exit(1)
  except KeyboardInterrupt:
    sys.exit(1)
  except Exception as e:
    print(f"An unexpected error occurred during subprocess execution: {e}")
    sys.exit(1)


def _build_dumpvars_command(vars_to_get: List[str]) -> List[str]:
  """Constructs the command list for soong_ui.bash --dumpvars-mode.

  Assumes CWD is the Android root.
  """
  soong_ui_path: str = "build/soong/soong_ui.bash"

  vars_string: str = " ".join(vars_to_get)

  command_list: List[str] = [
      soong_ui_path,
      "--dumpvars-mode",
      f"--vars={vars_string}",
  ]
  return command_list


def _parse_dumpvars_output(output_string: str) -> BuildVarsDict:
  """Parses the standard output from soong_ui.bash --dumpvars-mode.

  Args:
      output_string: The raw string output from soong_ui.bash --dumpvars-mode.

  Returns:
      A BuildVarsDict (a dictionary with a predefined structure) containing
      the requested build variables as string key-value pairs. Refer to
      the BuildVarsDict type definition for the specific expected keys.
  """
  parsed_vars: BuildVarsDict = {}
  output_lines: List[str] = output_string.strip().splitlines()

  for line in output_lines:
    # Dumpvars output looks like KEY='value' or KEY="value" or KEY=value
    # Simple split on first '=' is usually sufficient.
    if "=" in line:
      key: str
      value_with_quotes: str
      key, value_with_quotes = line.split("=", 1)

      # Remove potential quotes around the value and strip whitespace
      value = value_with_quotes.strip()
      if value.startswith("'") and value.endswith("'"):
        value = value[1:-1]
      elif value.startswith('"') and value.endswith('"'):
        value = value[1:-1]

      parsed_vars[key.strip()] = value
    else:
      # Lines without '=' are unexpected for variable output, might be
      # warnings/info from the build system itself.
      print(f"Error: Unparseable line from dumpvars output: '{line}'")
      sys.exit(1)

  return parsed_vars


def get_android_build_vars() -> BuildVarsDict:
  """Dumps variables from the Android build system.

  Retrieves variables defined in the global REQUIRED_BUILD_VARS and
  OPTIONAL_BUILD_VARS, and validates that the required ones are present.
  Assumes CWD is already the Android root and necessary TARGET_* env vars are
  set.
  """
  command_list = _build_dumpvars_command(
      vars_to_get=REQUIRED_BUILD_VARS + OPTIONAL_BUILD_VARS
  )

  process_result = run_subprocess(command_list, capture_stdout=True)
  parsed_vars: BuildVarsDict = _parse_dumpvars_output(process_result.stdout)

  for var_name in REQUIRED_BUILD_VARS:
    if var_name not in parsed_vars or not parsed_vars.get(var_name):
      print(
          f"Error: Essential variable '{var_name}' not found or "
          "empty in retrieved build variables. This indicates a "
          "problem with the build setup or dumpvars output."
      )
      print("\nRetrieved Variables:")
      for k, v in parsed_vars.items():
        print(f"  {k}='{v}'")
      sys.exit(1)

  return parsed_vars


def extract_from_apex(apex_name: str, build_vars: BuildVarsDict):
  """Extract files from an APEX file"""
  host_out = build_vars.get("HOST_OUT")
  target_out = build_vars.get("TARGET_OUT")

  apex_root = os.path.join(target_out, "apex")
  apex_input_file = os.path.join(apex_root, f"{apex_name}.apex")
  apex_out_dir = os.path.join(apex_root, apex_name)

  if not os.path.exists(apex_input_file):
    apex_input_file = os.path.join(apex_root, f"{apex_name}.capex")

  if not os.path.exists(apex_input_file):
    print(f"Error: APEX file for '{apex_name}' not found in '{apex_root}'.")
    sys.exit(1)

  shutil.rmtree(apex_out_dir, ignore_errors=True)
  os.makedirs(apex_out_dir, exist_ok=True)

  deapexer_path = os.path.join(host_out, "bin", "deapexer")
  debugfs_path = os.path.join(host_out, "bin", "debugfs")
  fsckerofs_path = os.path.join(host_out, "bin", "fsck.erofs")

  deapexer_command = [
      deapexer_path,
      "--debugfs_path",
      debugfs_path,
      "--fsckerofs_path",
      fsckerofs_path,
      "extract",
      apex_input_file,
      apex_out_dir,
  ]
  print(f"$ {shlex.join(deapexer_command)}")
  run_subprocess(deapexer_command)

  host_apex_out_dir = os.path.join(host_out, apex_name)
  shutil.rmtree(host_apex_out_dir, ignore_errors=True)
  os.makedirs(host_apex_out_dir, exist_ok=True)

  etc_src = os.path.join(apex_out_dir, "etc")
  etc_dest = os.path.join(host_apex_out_dir, "etc")
  if os.path.exists(etc_src):
    shutil.copytree(
        etc_src, etc_dest, dirs_exist_ok=True, copy_function=perform_copy
    )
  else:
    print(f"No 'etc' directory found in extracted {apex_name}.")
    sys.exit(1)


def perform_copy(source_path: str, target_path: str) -> None:
  """Performs a single file copy operation. Overwrites target.

  Exits the script with status 1 if any error occurs.

  Args:
      source_path (str): The path to the source file.
      target_path (str): The path to the target file.

  Returns:
      None
  """
  try:
    if not os.path.exists(source_path):
      print(
          f"  ERROR: Source file not found! ({source_path})",
          file=sys.stderr,
      )
      sys.exit(1)

    target_dir: str = os.path.dirname(target_path)
    os.makedirs(target_dir, exist_ok=True)

    print(f"$ cp {source_path} {target_path}")
    shutil.copy(source_path, target_path)

  except Exception as e:
    err_msg = (
        "  ERROR: An unexpected error occurred during copy "
        f"({source_path} -> {target_path}): {e}"
    )
    print(err_msg, file=sys.stderr)
    sys.exit(1)


def host_i18n_data_action(build_vars: BuildVarsDict):
  """Custom action to process i18n data."""
  extract_from_apex(I18N_APEX, build_vars)


def host_tzdata_data_action(build_vars: BuildVarsDict):
  """Custom action to process tzdata data."""
  extract_from_apex(TZDATA_APEX, build_vars)


def _get_target_art_apex_system_path(build_vars: BuildVarsDict) -> str:
  """Returns the path corresponding to TARGET_ART_APEX_SYSTEM."""
  product_out: str = build_vars["PRODUCT_OUT"]
  return os.path.join(product_out, "system/apex/com.android.art")


def _get_target_boot_image_profile_path(build_vars: BuildVarsDict) -> str:
  """Returns the path corresponding to TARGET_BOOT_IMAGE_PROFILE."""
  target_art_apex_system = _get_target_art_apex_system_path(build_vars)
  # This mimics the '.testing' suffix addition from the makefile.
  return os.path.join(
      target_art_apex_system + ".testing", "etc/boot-image.prof"
  )


def _get_target_core_img_dex_files_paths(
    build_vars: BuildVarsDict,
) -> List[str]:
  "Generates paths to intermediate core image JARs for testing."
  out_dir: str = build_vars["OUT_DIR"]
  intermediates_dir_base: str = os.path.join(
      out_dir, "target/common/obj/JAVA_LIBRARIES"
  )
  return [
      os.path.join(
          intermediates_dir_base,
          f"{jar}.com.android.art.testing_intermediates/javalib.jar",
      )
      for jar in CORE_IMG_JARS
  ]


def generate_simulator_profile_action(build_vars: BuildVarsDict):
  """Generates the simulator boot image profile using profmand."""

  # Construct all necessary paths and variables
  host_out_execs: str = build_vars["HOST_OUT_EXECUTABLES"]
  profmand_path: str = os.path.join(host_out_execs, "profmand")
  # TARGET_BOOT_IMAGE_PROFILE
  reference_profile_file: str = _get_target_boot_image_profile_path(build_vars)

  # Determine dex locations based on ART_TEST_ANDROID_ROOT
  dex_location_base: str = os.environ.get(
      "ART_TEST_ANDROID_ROOT", "/apex/com.android.art/javalib"
  )
  dex_locations = [f"{dex_location_base}/{jar}.jar" for jar in CORE_IMG_JARS]

  os.makedirs(os.path.dirname(reference_profile_file), exist_ok=True)
  # Generate a profile from the core boot jars. This allows the simulator
  # and boot image to use a stable profile that is generated on the host.
  profmand_command: List[str] = [
      profmand_path,
      "--output-profile-type=boot",
  ]

  profiles_str = build_vars.get(
      "PRODUCT_DEX_PREOPT_BOOT_IMAGE_PROFILE_LOCATION", ""
  )
  profmand_command.append(f"--create-profile-from={profiles_str}")
  # Add --apk arguments from TARGET_CORE_IMG_DEX_FILES
  apk_paths = _get_target_core_img_dex_files_paths(build_vars)
  profmand_command.extend([f"--apk={path}" for path in apk_paths])
  # Add --dex-location arguments
  profmand_command.extend([f"--dex-location={loc}" for loc in dex_locations])
  # Add the final reference profile file argument
  profmand_command.append(f"--reference-profile-file={reference_profile_file}")

  print(f"$ {shlex.join(profmand_command)}")
  run_subprocess(profmand_command)


def generate_simulator_boot_image_action(build_vars: BuildVarsDict):
  """Generates the simulator boot image using the host's dex2oat."""

  # Define all necessary paths from build variables
  product_out: str = build_vars["PRODUCT_OUT"]
  host_out_execs: str = build_vars["HOST_OUT_EXECUTABLES"]

  target_boot_image_system_dir = os.path.join(
      product_out, "system/apex/art_boot_images"
  )
  target_art_apex_system_dir: str = _get_target_art_apex_system_path(build_vars)
  profile_file: str = _get_target_boot_image_profile_path(build_vars)
  android_root_for_boot_image: str = build_vars["TARGET_OUT"]

  # Note: The target boot image needs to be in a trusted system directory
  # to be used by the zygote or if -Xonly-use-system-oat-files is passed
  # to the runtime.
  shutil.rmtree(target_boot_image_system_dir, ignore_errors=True)
  os.makedirs(
      os.path.join(target_boot_image_system_dir, "javalib"), exist_ok=True
  )
  os.makedirs(
      os.path.join(target_art_apex_system_dir, "javalib"), exist_ok=True
  )

  # Copy the core boot jars to the expected directory for
  # generate-boot-image.
  target_core_img_dex_files = _get_target_core_img_dex_files_paths(build_vars)
  for i, src_jar in enumerate(target_core_img_dex_files):
    dest_jar_name = CORE_IMG_JARS[i] + ".jar"
    dest_path = os.path.join(
        target_art_apex_system_dir, "javalib", dest_jar_name
    )
    perform_copy(src_jar, dest_path)

  # Generate a target boot image using the host dex2oat. Note: a boot
  # image using a profile is required for certain run tests to pass.
  command = [
      os.path.join(host_out_execs, "generate-boot-image64"),
      f"--output-dir={os.path.join(target_boot_image_system_dir, 'javalib')}",
      "--compiler-filter=speed-profile",
      "--use-profile=true",
      f"--profile-file={profile_file}",
      f"--dex2oat-bin={os.path.join(host_out_execs, 'dex2oatd')}",
      f"--android-root={android_root_for_boot_image}",
      "--android-root-for-location=true",
      "--core-only=true",
      f'--instruction-set={build_vars["TARGET_ARCH"]}',
  ]
  print(f"$ {shlex.join(command)}")
  run_subprocess(command)


def _get_core_img_jar_source_path(
    build_vars: BuildVarsDict, jar_basename: str
) -> str:
  """Returns the full source path for a given core image JAR.

  This path is where the build system places JARs for dexpreopting.
  """
  out_dir = build_vars.get("OUT_DIR")
  target_arch = build_vars.get("TARGET_ARCH")
  # Path corresponds to art/build/Android.common_path.mk variable
  # CORE_IMG_JAR_DIR.
  core_img_jar_dir = os.path.join(
      out_dir, "soong", f"dexpreopt_{target_arch}", "dex_artjars_input"
  )
  return os.path.join(core_img_jar_dir, f"{jar_basename}.jar")


def _get_hostdex_jar_source_path(
    build_vars: BuildVarsDict, jar_basename: str
) -> str:
  """Returns the source path for a given hostdex JAR."""
  host_out_java_libs = build_vars.get("HOST_OUT_JAVA_LIBRARIES")
  return os.path.join(host_out_java_libs, f"{jar_basename}-hostdex.jar")


def copy_core_img_jars_action(build_vars: BuildVarsDict):
  """Copies JARs listed in the global CORE_IMG_JARS."""
  target_base_dir = os.path.join(
      build_vars.get("HOST_OUT"), "apex/com.android.art/javalib"
  )

  for jar_base_name in CORE_IMG_JARS:
    source: str = _get_core_img_jar_source_path(build_vars, jar_base_name)
    target: str = os.path.join(target_base_dir, f"{jar_base_name}.jar")
    perform_copy(source, target)


def copy_hostdex_jars_action(build_vars: BuildVarsDict):
  """Copies hostdex JARs to their respective APEX javalib dirs."""
  # We can still use the host variant of `conscrypt` and `core-icu4j`
  # because they don't go into the primary boot image that is used in
  # host gtests, and hence can't lead to checksum mismatches.
  host_out = build_vars.get("HOST_OUT")
  for apex_name, jar_basename in HOSTDEX_JARS:
    source = _get_hostdex_jar_source_path(build_vars, jar_basename)
    target = os.path.join(
        host_out, f"apex/{apex_name}/javalib/{jar_basename}.jar"
    )
    perform_copy(source, target)


def copy_all_host_boot_image_jars_action(build_vars: BuildVarsDict):
  """Composite action to copy all necessary JARs for the host boot image.

  This includes CORE_IMG_JARS, conscrypt, and i18n JARs.
  """
  copy_core_img_jars_action(build_vars)
  copy_hostdex_jars_action(build_vars)


class Target:
  """Represents an internal build target with potential actions/dependencies.

  Attributes:
      name: The unique identifier for this internal target.
      make_targets: List of actual Soong/Make targets required by this target.
      dependencies: List of names of other internal Targets this one depends on.
      action: An optional function to execute after make_targets are built.
  """

  def __init__(
      self,
      name: str,
      make_targets: Optional[List[str]] = None,
      dependencies: Optional[List[str]] = None,
      action: Optional[Callable] = None,
  ):
    """Initializes a Target."""
    self.name = name
    self.make_targets = make_targets or []
    self.dependencies = dependencies or []
    self.action = action

  def execute_post_build_action(self, build_vars: BuildVarsDict):
    """Executes the target's post-build action, if defined."""
    if self.action:
      self.action(build_vars)


class Builder:
  """Manages internal target definitions and orchestrates the build process."""

  def __init__(self):
    """Initializes the Builder."""
    self.targets: dict[str, Target] = {}
    self.enabled_internal_targets: List[str] = []
    self.positional_make_targets: List[str] = []
    self.build_vars: Optional[BuildVarsDict] = None

  def _get_art_host_executables_make_targets(self) -> List[str]:
    """Generates the list of make_targets for ART_HOST_EXECUTABLES."""
    make_targets: List[str] = []

    art_build_host_ndebug: bool = (
        os.environ.get("ART_BUILD_HOST_NDEBUG", "true") != "false"
    )
    art_build_host_debug: bool = (
        os.environ.get("ART_BUILD_HOST_DEBUG", "true") != "false"
    )

    # ART_HOST_EXECUTABLES
    if art_build_host_ndebug:
      # Names for non-debug host executables
      execs_for_ndebug: List[str] = (
          ART_CORE_EXECUTABLES + ART_CORE_DEBUGGABLE_EXECUTABLES_HOST
      )
      for name in execs_for_ndebug:
        make_targets.append(f"{name}-host")

    if art_build_host_debug:
      # Names for debug host executables
      for name in ART_CORE_DEBUGGABLE_EXECUTABLES_HOST:
        make_targets.append(f"{name}d-host")

    return list(set(make_targets))

  def _get_art_host_dex_dependencies_make_targets(self) -> List[str]:
    """Generates the list of make_targets for ART_HOST_DEX_DEPENDENCIES."""
    host_out_java_libs: str = self.build_vars["HOST_OUT_JAVA_LIBRARIES"]
    make_targets: List[str] = []

    # HOST_TEST_CORE_JARS :=
    # $(addsuffix -hostdex, $(CORE_IMG_JARS) core-icu4j conscrypt)
    host_test_core_jars_base_names: List[str] = CORE_IMG_JARS + [
        name for _, name in HOSTDEX_JARS
    ]
    host_test_core_jars: List[str] = [
        f"{name}-hostdex" for name in host_test_core_jars_base_names
    ]

    # ART_HOST_DEX_DEPENDENCIES :=
    # $(foreach jar,$(HOST_TEST_CORE_JARS),$(HOST_OUT_JAVA_LIBRARIES)/$(jar).jar)
    for jar_name in host_test_core_jars:
      full_jar_path: str = os.path.join(host_out_java_libs, f"{jar_name}.jar")
      make_targets.append(full_jar_path)

    return make_targets

  def _get_art_host_shared_library_dependencies(
      self,
  ) -> tuple[List[str], List[str]]:
    """Generates make_targets for host shared libraries.

    Considers HOST_PREFER_32_BIT and returns both non-debug and debug
    libraries.

    Returns:
        A tuple containing two lists of strings:
        (non_debug_make_targets, debug_make_targets)
    """
    make_targets: List[str] = []
    make_targets_debug: List[str] = []

    host_out_shared_libs: str = self.build_vars["HOST_OUT_SHARED_LIBRARIES"]
    second_host_out_shared_libs: Optional[str] = self.build_vars.get(
        "2ND_HOST_OUT_SHARED_LIBRARIES"
    )
    actual_host_2nd_arch: Optional[str] = self.build_vars.get("HOST_2ND_ARCH")

    host_prefer_32_bit_env: str = os.environ.get("HOST_PREFER_32_BIT", "")
    host_prefer_32_bit: bool = host_prefer_32_bit_env == "true"

    primary_libs_path: Optional[str]
    if host_prefer_32_bit:
      primary_libs_path = second_host_out_shared_libs
      if not primary_libs_path:
        print(
            "Error: HOST_PREFER_32_BIT=true but "
            "2ND_HOST_OUT_SHARED_LIBRARIES is not set.",
            file=sys.stderr,
        )
        sys.exit(1)
    else:
      primary_libs_path = host_out_shared_libs

    # Non-debug libraries
    make_targets.extend(
        os.path.join(primary_libs_path, f"{name}.so")
        for name in ART_HOST_CORE_SHARED_LIBRARIES
    )
    # Debug libraries
    make_targets_debug.extend(
        os.path.join(primary_libs_path, f"{name}.so")
        for name in ART_CORE_SHARED_DEBUG_LIBRARIES
    )

    if actual_host_2nd_arch:
      if not second_host_out_shared_libs:
        err_msg = (
            f"Error: HOST_2ND_ARCH ('{actual_host_2nd_arch}') is set, "
            "but 2ND_HOST_OUT_SHARED_LIBRARIES is missing."
        )
        print(err_msg, file=sys.stderr)
        sys.exit(1)

      # Non-debug 2nd arch libraries
      make_targets.extend(
          os.path.join(second_host_out_shared_libs, f"{name}.so")
          for name in ART_HOST_CORE_SHARED_LIBRARIES
      )
      # Debug 2nd arch libraries
      make_targets_debug.extend(
          os.path.join(second_host_out_shared_libs, f"{name}.so")
          for name in ART_CORE_SHARED_DEBUG_LIBRARIES
      )

    return list(set(make_targets)), list(set(make_targets_debug))

  def _get_art_host_run_test_dep_make_targets(self) -> List[str]:
    """Gets make targets for ART_TEST_HOST_RUN_TEST_DEPENDENCIES."""
    make_targets: List[str] = []

    # ART_HOST_EXECUTABLES
    make_targets.extend(self._get_art_host_executables_make_targets())
    # ART_HOST_DEX_DEPENDENCIES
    make_targets.extend(self._get_art_host_dex_dependencies_make_targets())

    # Simple make targets
    make_targets.append("art_test_host_run_test_dependencies")

    # Special handling for hprof-conv. On the master-art branch, it
    # requires a full path to build correctly. This is because its
    # dependency, libwinpthread, references a Windows-specific variant
    # that is not available in the branch's source tree.
    host_out_execs: str = self.build_vars["HOST_OUT_EXECUTABLES"]
    make_targets.append(os.path.join(host_out_execs, "hprof-conv"))

    return list(set(make_targets))

  def _get_copy_core_img_make_targets(self) -> List[str]:
    """Generates the list of make_targets (file paths) for CORE_IMG_JARS.

    These are the source JARs that the copy_core_img_jars_action will copy.
    """
    make_targets = []
    for jar_base_name in CORE_IMG_JARS:
      source_file_path = _get_core_img_jar_source_path(
          self.build_vars, jar_base_name
      )
      make_targets.append(source_file_path)
    return make_targets

  def _get_art_target_platform_libs_make_targets(self) -> List[str]:
    """Generates make_targets for ART_TARGET_PLATFORM_DEPENDENCIES."""
    target_out: str = self.build_vars["TARGET_OUT"]
    target_out_shared_libs: str = self.build_vars["TARGET_OUT_SHARED_LIBRARIES"]
    second_target_out_shared_libs: Optional[str] = self.build_vars.get(
        "2ND_TARGET_OUT_SHARED_LIBRARIES"
    )
    target_2nd_arch: Optional[str] = self.build_vars.get("TARGET_2ND_ARCH")

    make_targets: List[str] = [
        os.path.join(target_out, "etc", "public.libraries.txt")
    ]

    make_targets.extend(
        os.path.join(target_out_shared_libs, f"{lib}.so")
        for lib in ART_TARGET_PLATFORM_LIBS_WITH_FULL_PATH
    )

    if target_2nd_arch:
      if not second_target_out_shared_libs:
        print(
            "Error: TARGET_2ND_ARCH is set, but "
            "2ND_TARGET_OUT_SHARED_LIBRARIES is missing.",
            file=sys.stderr,
        )
        sys.exit(1)

      make_targets.extend(
          os.path.join(second_target_out_shared_libs, f"{lib}.so")
          for lib in ART_TARGET_PLATFORM_LIBS_WITH_FULL_PATH
      )

    return list(set(make_targets))

  def _get_art_simulator_profile_dep_make_targets(self) -> List[str]:
    """Gets make targets required by the simulator profile action."""
    # The action depends on profmand and the boot image profile text.
    make_targets: List[str] = [
        os.path.join(self.build_vars["HOST_OUT_EXECUTABLES"], "profmand"),
    ]
    profiles_str = self.build_vars.get(
        "PRODUCT_DEX_PREOPT_BOOT_IMAGE_PROFILE_LOCATION", ""
    )
    make_targets.append(profiles_str)

    # TARGET_CORE_IMG_DEX_FILES
    make_targets.extend(_get_target_core_img_dex_files_paths(self.build_vars))

    return make_targets

  def _get_art_simulator_boot_image_dep_make_targets(
      self,
  ) -> List[str]:
    """Gets make targets required by the simulator boot image action."""
    host_out_execs: str = self.build_vars["HOST_OUT_EXECUTABLES"]
    make_targets: List[str] = [
        os.path.join(host_out_execs, "generate-boot-image64"),
        os.path.join(host_out_execs, "dex2oatd"),
    ]
    make_targets.extend(_get_target_core_img_dex_files_paths(self.build_vars))
    return make_targets

  def setup_default_targets(self, build_vars: BuildVarsDict):
    """Defines built-in targets using build_vars and stores build_vars."""
    self.build_vars = build_vars

    # ART_HOST_DEPENDENCIES
    all_art_host_deps_make_targets = []
    all_art_host_deps_make_targets.extend(
        # ART_HOST_EXECUTABLES
        self._get_art_host_executables_make_targets()
    )
    all_art_host_deps_make_targets.extend(
        # ART_HOST_DEX_DEPENDENCIES
        self._get_art_host_dex_dependencies_make_targets()
    )
    # Get both non-debug and debug shared library dependencies
    (
        shared_libs,
        debug_libs,
    ) = self._get_art_host_shared_library_dependencies()
    all_art_host_deps_make_targets.extend(shared_libs)
    # Conditionally add debug dependencies.
    art_build_host_debug: bool = (
        os.environ.get("ART_BUILD_HOST_DEBUG", "true") != "false"
    )
    if art_build_host_debug:
      all_art_host_deps_make_targets.extend(debug_libs)
    self.add_target(
        Target(
            name="art_host_dependencies",
            make_targets=list(set(all_art_host_deps_make_targets)),
        )
    )

    # HOST_CORE_IMG_OUTS
    copy_core_img_make_targets = self._get_copy_core_img_make_targets()
    hostdex_jar_make_targets = [
        _get_hostdex_jar_source_path(self.build_vars, basename)
        for _, basename in HOSTDEX_JARS
    ]
    all_boot_image_make_targets = (
        copy_core_img_make_targets + hostdex_jar_make_targets
    )
    self.add_target(
        Target(
            name="host_core_img_outs",
            action=copy_all_host_boot_image_jars_action,
            make_targets=all_boot_image_make_targets,
        )
    )

    build_art_host_dependencies = [
        "art_host_dependencies",
        "host_core_img_outs",
    ]
    if using_thin_manifest():
      # These dependencies are only necessary when building on the thin manifest
      # where prebuilts are used for many libraries. In a full platform tree
      # their source variants will ensure these files get built. Also, doing
      # this in a full platform tree tends to overwrite the built files in the
      # post-build actions, which can make the next incremental build invalidate
      # a lot of other targets and hence do excessive rebuilding.
      # HOST_I18N_DATA
      self.add_target(
          Target(
              name="extract-host-i18n-data",
              action=host_i18n_data_action,
              make_targets=[I18N_APEX, "deapexer", "debugfs", "fsck.erofs"],
          )
      )
      build_art_host_dependencies.append("extract-host-i18n-data")
      # HOST_TZDATA_DATA
      self.add_target(
          Target(
              name="extract-host-tzdata-data",
              action=host_tzdata_data_action,
              make_targets=[TZDATA_APEX, "deapexer", "debugfs", "fsck.erofs"],
          )
      )
      build_art_host_dependencies.append("extract-host-tzdata-data")

    self.add_target(
        Target(
            name="build-art-host",
            make_targets=["art-script"],
            dependencies=build_art_host_dependencies,
        )
    )
    # build-art-host-gtests depends on build-art-host  and
    #    $(ART_TEST_HOST_GTEST_DEPENDENCIES)
    # ART_TEST_HOST_GTEST_DEPENDENCIES := $(HOST_I18N_DATA)
    self.add_target(
        Target(
            name="build-art-host-gtests",
            dependencies=["build-art-host"],
        )
    )
    # build-art-host-run-tests
    art_test_host_run_test_deps = self._get_art_host_run_test_dep_make_targets()
    self.add_target(
        Target(
            name="build-art-host-run-tests",
            dependencies=["build-art-host"],
            make_targets=(
                art_test_host_run_test_deps
                + ["art-run-test-host-data", "art-run-test-jvm-data"]
            ),
        )
    )
    self.add_target(
        Target(
            name="build-art-target",
            make_targets=([
                "art-script",
                "com.android.art.testing",
                "com.android.conscrypt",
                "com.android.i18n",
            ]),
        )
    )
    # ART_TARGET_PLATFORM_DEPENDENCIES
    self.add_target(
        Target(
            name="art_target_platform_dependencies",
            make_targets=self._get_art_target_platform_libs_make_targets(),
        )
    )
    self.add_target(
        Target(
            name="build-art-target-gtests",
            make_targets=["art_test_target_gtest_dependencies"],
            dependencies=[
                "build-art-target",
                "art_target_platform_dependencies",
            ],
        )
    )
    self.add_target(
        Target(
            name="build-art-target-run-tests",
            dependencies=[
                "build-art-target",
                # ART_TARGET_PLATFORM_DEPENDENCIES
                "art_target_platform_dependencies",
            ],
            make_targets=[
                "art_test_target_run_test_dependencies",
                "art-run-test-target-data",
            ],
        )
    )
    self.add_target(
        Target(
            name="build-art-simulator-profile",
            action=generate_simulator_profile_action,
            make_targets=self._get_art_simulator_profile_dep_make_targets(),
        )
    )
    self.add_target(
        Target(
            name="build-art-simulator-boot-image",
            dependencies=["build-art-simulator-profile"],
            action=generate_simulator_boot_image_action,
            make_targets=self._get_art_simulator_boot_image_dep_make_targets(),
        )
    )
    # For simulator, build a target profile and boot image on the host.
    self.add_target(
        Target(
            name="build-art-simulator",
            dependencies=[
                "build-art-simulator-profile",
                "build-art-simulator-boot-image",
            ],
        )
    )
    self.add_target(
        Target(
            name="build-art",
            dependencies=["build-art-host", "build-art-target"],
        )
    )

  def add_target(self, target: Target):
    """Adds or updates an internal target definition.

    Args:
        target: The Target object to add.
    """
    if target.name in self.targets:
      print(f"Error: Redefining internal target '{target.name}'.")
      sys.exit(1)
    self.targets[target.name] = target

  def _collect_recursive(
      self,
      target_name: str,
      collected_make_targets: List[str],
      collected_actions: List[Target],
      recursion_stack: set[str],
      processed_nodes: set[str],
  ):
    """Recursively collects dependencies, detecting cycles.

    Internal helper for collect_targets. Modifies lists in place.

    Args:
        target_name: The name of the internal target to collect.
        collected_make_targets: List to store collected make targets.
        collected_actions: List to store collected Target objects with actions.
        recursion_stack: A set of targets in the current traversal path, used to
          detect actual cycles.
        processed_nodes: A set of targets that have already been fully
          processed, used to handle shared dependencies (like diamond
          dependencies) efficiently.
    """
    if target_name in processed_nodes:
      return

    # If we encounter a node that is already in our current recursion
    # stack, we have found a genuine cycle.
    if target_name in recursion_stack:
      print(
          "Error: Cycle detected in internal target dependencies "
          f"involving '{target_name}'."
      )
      sys.exit(1)

    if target_name not in self.targets:
      print(
          f"Error: Definition error - Internal target '{target_name}' "
          "(specified as a dependency for another internal target) "
          "was not found in the defined internal targets"
          f" {self.targets}."
      )
      sys.exit(1)

    # Add the current target to the recursion stack for this path.
    recursion_stack.add(target_name)
    target = self.targets[target_name]

    for dependency_name in target.dependencies:
      self._collect_recursive(
          dependency_name,
          collected_make_targets,
          collected_actions,
          recursion_stack,
          processed_nodes,
      )

    # After traversing all children, remove from the recursion stack and
    # mark as fully processed.
    recursion_stack.remove(target_name)
    processed_nodes.add(target_name)

    collected_make_targets.extend(target.make_targets)

    if target.action:
      collected_actions.append(target)

  def collect_targets(
      self,
      target_name: str,
      collected_make_targets: List[str],
      collected_actions: List[Target],
      processed_nodes: set[str],
  ):
    """Collects make targets and actions for an internal target.

    Handles the top-level call for recursive collection.

    Args:
        target_name: The name of the internal target to collect.
        collected_make_targets: List to store collected make targets.
        collected_actions: List to store collected Target objects with actions.
        processed_nodes: A set of targets already processed in this build.
    """
    # The recursion stack is specific to each top-level traversal.
    recursion_stack = set()
    self._collect_recursive(
        target_name,
        collected_make_targets,
        collected_actions,
        recursion_stack,
        processed_nodes,
    )

  def build(self, args: argparse.Namespace):
    """Builds targets based on enabled internal and positional targets."""
    if self.build_vars is None:
      print(
          "Error: build_vars not set in Builder. "
          "setup_default_targets must be called first.",
          file=sys.stderr,
      )
      sys.exit(1)

    all_make_targets: List[str] = []
    all_actions: List[Target] = []
    # This set will track all nodes processed during this build run
    # to avoid redundant work. It's passed through the collectors.
    processed_nodes_for_build = set()

    for internal_target_name in self.enabled_internal_targets:
      if internal_target_name in self.targets:
        self.collect_targets(
            internal_target_name,
            all_make_targets,
            all_actions,
            processed_nodes_for_build,
        )
      else:
        print(
            "Error: Enabled internal target "
            f"'{internal_target_name}' not found in definitions. "
            "Exiting."
        )
        sys.exit(1)

    if self.positional_make_targets:
      all_make_targets.extend(self.positional_make_targets)

    unique_make_targets = list(dict.fromkeys(all_make_targets))

    if unique_make_targets:
      env_for_make = os.environ.copy()
      make_command = ["./build/soong/soong_ui.bash", "--make-mode"]
      if args.jobs:
        make_command.append(f"-j{args.jobs}")
      make_command.extend(unique_make_targets)

      print_env_parts = []
      # Only show a few key env vars for the log to avoid clutter
      for key in [
          "TARGET_PRODUCT",
          "TARGET_RELEASE",
          "TARGET_BUILD_VARIANT",
          "SOONG_ALLOW_MISSING_DEPENDENCIES",
          "TARGET_BUILD_UNBUNDLED",
      ]:
        if key in env_for_make:
          print_env_parts.append(f"{key}={env_for_make[key]}")
      print(f"$ {shlex.join(print_env_parts + make_command)}")

      run_subprocess(make_command, env=env_for_make)
    else:
      print("No make targets specified or collected.")

    # dict.fromkeys preserves the order, ensuring actions are executed
    # in dependency order.
    unique_actions = list(dict.fromkeys(all_actions))

    if unique_actions:
      print("Executing post-build actions...")
      for target_obj in unique_actions:
        target_obj.execute_post_build_action(self.build_vars)
    else:
      print("No post-build actions to execute.")


def _setup_env_and_get_primary_build_vars(
    args: argparse.Namespace,
) -> BuildVarsDict:
  """Sets up the script's execution environment and retrieves build vars.

  This includes changing CWD, setting ANDROID_BUILD_TOP, and retrieving
  primary build variables from dumpvars.
  """
  actual_android_root = os.path.abspath(args.android_root)
  print(f"Info: Effective Android build root: {actual_android_root}")
  os.environ["ANDROID_BUILD_TOP"] = actual_android_root
  try:
    os.chdir(actual_android_root)
  except Exception as e:
    print(f"Error: Could not change CWD to {actual_android_root}: {e}")
    sys.exit(1)

  required_env_vars = [
      "TARGET_PRODUCT",
      "TARGET_RELEASE",
      "TARGET_BUILD_VARIANT",
  ]
  missing_vars = [v for v in required_env_vars if not os.environ.get(v)]
  if missing_vars:
    error_msg1 = (
        "Error: The following essential environment variables must be set:"
    )
    error_msg_part2 = f"        {', '.join(missing_vars)}."
    print(error_msg1)
    print(error_msg_part2)
    sys.exit(1)

  if using_thin_manifest():
    # This is often necessary for reduced manifest branches (e.g.,
    # master-art) to allow them to build successfully when certain
    # framework dependencies are not present in the source tree.
    print(
        "Info: Using thin manifest - setting"
        " SOONG_ALLOW_MISSING_DEPENDENCIES=true and"
        " TARGET_BUILD_UNBUNDLED=true."
    )
    os.environ["SOONG_ALLOW_MISSING_DEPENDENCIES"] = "true"
    os.environ["TARGET_BUILD_UNBUNDLED"] = "true"

  final_build_vars: BuildVarsDict = get_android_build_vars()

  return final_build_vars


def parse_command_line_arguments(builder: Builder) -> argparse.ArgumentParser:
  """Parses args, populates builder lists, and returns the parser.

  Args:
      builder: The Builder instance to populate with parsed targets.

  Returns:
      The configured ArgumentParser object.
  """
  parser = argparse.ArgumentParser(
      description="Builds ART targets with Soong, handling APEX extractions."
  )
  parser.add_argument(
      "-j",
      dest="jobs",
      type=int,
      help="Set the number of parallel jobs for the build.",
  )
  # Arguments for enabling internal targets
  parser.add_argument(
      "--build-art-host",
      action="store_true",
      help="Build build-art-host components (activates internal target).",
  )
  parser.add_argument(
      "--build-art-host-gtests",
      action="store_true",
      help="Build build-art-host-gtests components (internal target).",
  )
  parser.add_argument(
      "--build-art-host-run-tests",
      action="store_true",
      help="Build build-art-host-run-tests components (internal target).",
  )
  parser.add_argument(
      "--build-art-target",
      action="store_true",
      help="Build build-art-target components (activates internal target).",
  )
  parser.add_argument(
      "--build-art-target-gtests",
      action="store_true",
      help="Build build-art-target-gtests components (internal target).",
  )
  parser.add_argument(
      "--build-art-target-run-tests",
      action="store_true",
      help="Build build-art-target-run-tests components (internal target).",
  )
  parser.add_argument(
      "--build-art-simulator",
      action="store_true",
      help="Build all ART simulator components (internal target).",
  )
  parser.add_argument(
      "--build-art",
      action="store_true",
      help="Build build-art components (activates internal target).",
  )
  parser.add_argument(
      "--android-root",
      default=os.environ.get("ANDROID_BUILD_TOP", "."),
      help=(
          "Path to the Android root directory. Overrides "
          "ANDROID_BUILD_TOP environment variable if set. "
          "Defaults to $ANDROID_BUILD_TOP or '.' if not set."
      ),
  )
  # Argument for positional real build targets
  parser.add_argument(
      "positional_targets",
      metavar="MAKE_TARGET",
      nargs="*",
      help="Additional Soong/Make build targets to build.",
  )

  args = parser.parse_args()

  if args.build_art_host:
    builder.enabled_internal_targets.append("build-art-host")
  if args.build_art_host_gtests:
    builder.enabled_internal_targets.append("build-art-host-gtests")
  if args.build_art_host_run_tests:
    builder.enabled_internal_targets.append("build-art-host-run-tests")
  if args.build_art_target:
    builder.enabled_internal_targets.append("build-art-target")
  if args.build_art_target_gtests:
    builder.enabled_internal_targets.append("build-art-target-gtests")
  if args.build_art_target_run_tests:
    builder.enabled_internal_targets.append("build-art-target-run-tests")
  if args.build_art_simulator:
    builder.enabled_internal_targets.append("build-art-simulator")
  if args.build_art:
    builder.enabled_internal_targets.append("build-art")

  builder.positional_make_targets.extend(args.positional_targets)

  return parser


def main():
  """Main execution function."""
  builder = Builder()
  parser = parse_command_line_arguments(builder)
  args = parser.parse_args()

  build_vars = _setup_env_and_get_primary_build_vars(args)
  builder.setup_default_targets(build_vars)

  if builder.enabled_internal_targets or builder.positional_make_targets:
    builder.build(args)
  else:
    print("No build targets specified. Printing help:")
    parser.print_help()
    sys.exit(1)


if __name__ == "__main__":
  main()

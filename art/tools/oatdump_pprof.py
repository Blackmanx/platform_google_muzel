#!/usr/bin/env python3
#
# Copyright 2025, The Android Open Source Project
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
"""A script to generate a pprof report from Android OAT (oatdump) files."""

import argparse
import gzip
import os
import re
import subprocess
import sys

# BEGIN profile_pb2 import
try:
  import profile_pb2
except ImportError:
  # Allow running the script from a checkout, not as a python_binary_host
  ANDROID_BUILD_TOP = os.environ.get('ANDROID_BUILD_TOP')
  if ANDROID_BUILD_TOP:
    import_path = ANDROID_BUILD_TOP + '/system/extras/simpleperf/scripts'
  else:
    import_path = os.getcwd() + '/system/extras/simpleperf/scripts'
  sys.path.append(import_path)
  import profile_pb2
# END profile_pb2 import

# Common file extensions for OAT/ODEX files.
OAT_FILE_EXTENSIONS = (
    '.odex',
    '.oat',
    '@classes.dex',
    '@classes.odex',
)


def run_command(command, decode=True):
  """Executes a shell command and returns its output, handling errors."""
  try:
    process = subprocess.run(
        command, check=True, capture_output=True, shell=True
    )
    if decode:
      return process.stdout.decode('utf-8', errors='ignore').strip()
    return process.stdout
  except subprocess.CalledProcessError as e:
    sys.stderr.write(
        f"Error running command: '{command}'\nStderr:"
        f" {e.stderr.decode('utf-8', errors='ignore')}\n"
    )
    return None


def load_proguard_map(map_file_path):
  """Parse a Proguard map file and return a deobfuscation mapping."""
  mapping = {'classes': {}, 'methods': {}}
  current_class_map = None
  class_regex = re.compile(r'^(.*) -> (.*):')
  method_regex = re.compile(r'^\s+\S+\s+([^\s(]+)\(.*\)\s+->\s+(.*)')

  with open(map_file_path, 'r', encoding='utf-8', errors='ignore') as f:
    for line in f:
      class_match = class_regex.match(line)
      if class_match:
        original_name, obfuscated_name = class_match.groups()
        mapping['classes'][obfuscated_name] = original_name
        current_class_map = {
            'obfuscated': obfuscated_name,
            'original': original_name,
        }
      elif current_class_map:
        method_match = method_regex.match(line)
        if method_match:
          original_method, obfuscated_method = method_match.groups()
          full_obfuscated_key = (
              f"{current_class_map['obfuscated']}.{obfuscated_method}"
          )
          mapping['methods'][full_obfuscated_key] = original_method
  return mapping


def stream_and_parse_oatdump(paths, proguard_map=None, is_host_file=False):
  """Yields (oat_file, dex_file, package, class, method, size) tuples from oatdump."""
  current_dex_file = None
  current_obfuscated_class_name = None
  current_method_name = None

  dex_file_regex = re.compile(r'^location:\s*(.*?)$')
  class_regex = re.compile(r'^\s*[0-9a-fA-F]+:\s+L([^;]+);')
  method_regex = re.compile(
      r'\s(?:[^\s.]+\.)*([a-zA-Z0-9_$<>]+\(.*\))\s+\(dex_method_idx='
  )
  code_size_regex = re.compile(r'^\s+CODE:.*size=(\d+)')

  for path in paths:
    print(f'Parsing {path}...')
    current_oat_file = path
    process = None

    if is_host_file:
      try:
        lines_iterator = open(path, 'r', encoding='utf-8', errors='ignore')
      except FileNotFoundError:
        sys.stderr.write(f'Error: Oatdump file not found at {path}.\n')
        continue
    else:
      oatdump_command = (
          f'adb shell oatdump --oat-file={path} --no-disassemble --no-dump:vmap'
      )
      process = subprocess.Popen(
          oatdump_command,
          shell=True,
          stdout=subprocess.PIPE,
          stderr=subprocess.PIPE,
          text=True,
          errors='ignore',
      )
      lines_iterator = process.stdout

    for line in lines_iterator:
      oat_dex_file_match = dex_file_regex.search(line)
      if oat_dex_file_match:
        current_dex_file = oat_dex_file_match.group(1)
        current_obfuscated_class_name = None
        current_method_name = None
        continue

      if current_dex_file:
        class_match = class_regex.search(line)
        if class_match:
          current_obfuscated_class_name = class_match.group(1).replace('/', '.')
          current_method_name = None
          continue

      if current_obfuscated_class_name:
        method_match = method_regex.search(line)
        if method_match:
          current_method_name = method_match.group(1)
          continue

      if not current_method_name:
        continue

      size_match = code_size_regex.search(line)
      if not size_match:
        continue

      code_size = int(size_match.group(1))
      if code_size > 0:
        class_to_process = current_obfuscated_class_name
        method_to_process = current_method_name

        if proguard_map:
          class_to_process = proguard_map['classes'].get(
              current_obfuscated_class_name, current_obfuscated_class_name
          )

          method_name_part = method_to_process.split('(')[0]
          params = method_to_process[len(method_name_part) :]
          full_obfuscated_key = (
              f'{current_obfuscated_class_name}.{method_name_part}'
          )

          deobfuscated_method_name = proguard_map['methods'].get(
              full_obfuscated_key
          )
          if deobfuscated_method_name:
            method_to_process = deobfuscated_method_name + params

        package, class_name = (
            (class_to_process.rsplit('.', 1))
            if '.' in class_to_process
            else ('default', class_to_process)
        )
        if package and class_name and method_to_process:
          yield (
              current_oat_file,
              current_dex_file,
              package,
              class_name,
              method_to_process,
              code_size,
          )

      current_method_name = None

    if is_host_file and lines_iterator:
      lines_iterator.close()

    if process:
      stderr_output = process.stderr.read()
      if process.wait() != 0:
        sys.stderr.write(
            f'Warning: Failed to get oatdump for {path}.\nStderr:'
            f' {stderr_output}\n'
        )


def generate_pprof_report(data, output_path, dex_file_oriented, argv):
  """Generates a pprof file from the parsed oatdump data."""
  profile = profile_pb2.Profile()
  # Use a dictionary for efficient string-to-ID mapping and a list to maintain
  # the order for the protobuf string table.
  string_to_id = {'': 0}
  id_to_string = ['']
  functions = {}
  locations = {}
  mappings = {}

  def get_string_id(s):
    """Adds a string to the string table and returns its ID."""
    string_id = string_to_id.get(s)
    if string_id is None:
      new_id = len(id_to_string)
      string_to_id[s] = new_id
      id_to_string.append(s)
      string_id = new_id
    return string_id

  st_size = profile.sample_type.add()
  st_size.type = get_string_id('space')
  st_size.unit = get_string_id('bytes')

  for oat_file, dex_file, package_name, class_name, method_name, size in data:
    if size == 0:
      continue

    sample = profile.sample.add()
    sample.value.append(size)

    # Generate a stack trace for the sample with the following hierarchy:
    #   - Oat file path (to identify where the code comes from)
    #   - [Optional] Dex file path (to identify where the dex code comes from)
    #   - Package name parts, e.g. 'android', 'app'
    #   - Class name, e.g. 'Activity'
    #   - Fully qualified method name, e.g. 'android.app.Activity.onCreate(android.os.Bundle)'
    stack_frames = []
    stack_frames.append((oat_file, os.path.basename(oat_file)))
    if dex_file_oriented:
      stack_frames.append((dex_file, dex_file))
    package_parts = package_name.split('.')
    current_package = ''
    for part in package_parts:
      current_package = f'{current_package}.{part}' if current_package else part
      stack_frames.append((current_package, current_package))
    full_class_name = f'{current_package}.{class_name}'
    stack_frames.append((full_class_name, full_class_name))
    full_method_name = f'{package_name}.{class_name}.{method_name}'
    stack_frames.append((full_method_name, full_method_name))

    location_ids = []

    if oat_file not in mappings:
      mapping = profile.mapping.add()
      mapping.id = len(profile.mapping)
      mapping.filename = get_string_id(oat_file)
      mappings[oat_file] = mapping.id
    mapping_id = mappings[oat_file]

    for i, (unique_id, label) in enumerate(stack_frames):
      if unique_id not in functions:
        func = profile.function.add()
        func.id = len(profile.function)
        func.name = get_string_id(label)
        func.filename = get_string_id(oat_file)
        functions[unique_id] = func.id

      func_id = functions[unique_id]

      loc_key = (func_id, mapping_id)
      if loc_key not in locations:
        loc = profile.location.add()
        loc.id = len(profile.location)
        loc.mapping_id = mapping_id
        line = loc.line.add()
        line.function_id = func_id
        locations[loc_key] = loc.id

      location_ids.append(locations[loc_key])

    sample.location_id.extend(reversed(location_ids))

  comment = f"{os.path.basename(argv[0])} {' '.join(argv[1:])}"
  profile.comment.append(get_string_id(comment))
  profile.string_table.extend(id_to_string)

  with gzip.open(output_path, 'wb') as f:
    f.write(profile.SerializeToString())


def get_oat_files_for_package(package_name):
  """Finds all OAT/ODEX files for a given package under the primary ABI."""
  command_output = run_command(f'adb shell pm art dump {package_name}')
  if not command_output:
    sys.stderr.write(f'Error: Could not find oat file for {package_name}.\n')
    return []

  paths = set()
  expect_location_for_primary_abi = False
  location_regex = re.compile(r'^\s*\[location is (.*?)\]$')

  for line in command_output.splitlines():
    if expect_location_for_primary_abi:
      location_match = location_regex.search(line)
      if location_match:
        location = location_match.group(1)
        if location.endswith(OAT_FILE_EXTENSIONS):
          paths.add(location)
        expect_location_for_primary_abi = False
    elif '[primary-abi]' in line:
      expect_location_for_primary_abi = True
      continue

  if not paths:
    sys.stderr.write(f'Error: Failed to parse art dump for {package_name}:\n')
    sys.stderr.write(command_output)
    sys.stderr.write('\n')
    return None
  return list(paths)


def get_oat_files_for_process(process_identifier):
  """Finds all .oat and .odex files for a given process name or PID."""
  pid = run_command(f'adb shell pidof -s {process_identifier}')
  if not pid:
    sys.stderr.write(f'Error: Could not find PID for "{process_identifier}".\n')
    return None

  maps_content = run_command(f'adb shell cat /proc/{pid}/maps')
  if not maps_content:
    sys.stderr.write(f'Error: Could not read maps for PID {pid}.\n')
    return None

  oat_files = set()
  for line in maps_content.splitlines():
    path = line.split()[-1]
    if path.endswith(OAT_FILE_EXTENSIONS):
      oat_files.add(path)
  return list(oat_files)


def ensure_adb_root():
  """Ensures adb connection has root access.

  Raises:
    subprocess.CalledProcessError: If an adb command fails.
    FileNotFoundError: If 'adb' command is not found.
    SystemExit: If root access cannot be obtained.
  """
  try:
    # Attempt to restart adbd as root. This command will block until adbd restarts.
    run_command('adb root')
    # Wait for the device to reconnect after adb root.
    run_command('adb wait-for-device')
    # Verify root access by checking the user ID.
    result_id = run_command('adb shell id -u')
    if result_id != '0':
      sys.stderr.write(
          'Error: Failed to get root access via adb. Please ensure your device'
          ' is rooted or adbd can be restarted as root.\n'
      )
      sys.exit(1)
  except subprocess.CalledProcessError as e:
    command = e.cmd if isinstance(e.cmd, str) else ' '.join(e.cmd)
    sys.stderr.write(
        f'Error: adb command failed during root check. Is adb installed and'
        f' device connected?\n'
    )
    sys.stderr.write(f'Command: {command}\n')
    sys.stderr.write(f'Stderr: {e.stderr}\n')
    sys.exit(1)
  except FileNotFoundError:
    sys.stderr.write(
        "Error: 'adb' command not found. Please ensure adb is in your PATH.\n"
    )
    sys.exit(1)


def main(argv):
  parser = argparse.ArgumentParser(
      description='Generate a pprof report from Android OAT files.'
  )
  group = parser.add_mutually_exclusive_group(required=True)
  group.add_argument(
      '--package', help='Package name of the app on the connected device.'
  )
  group.add_argument(
      '--process', help='Process name or PID on the connected device.'
  )
  group.add_argument(
      '--oatdump-file', help='Path to a single oatdump output file on the host.'
  )

  parser.add_argument(
      '--output',
      default='oatdump_report.pprof',
      help='Path to save the generated pprof file.',
  )
  parser.add_argument(
      '--map', help='Optional path to a Proguard deobfuscation map file.'
  )
  parser.add_argument(
      '--dex-file-oriented',
      action='store_true',
      help='If set, groups the flame graph by dex files.',
  )
  args = parser.parse_args(argv)

  oat_files = []
  is_host = False
  if args.oatdump_file:
    oat_files = [args.oatdump_file]
    is_host = True
  else:
    ensure_adb_root()
    if args.package:
      oat_files = get_oat_files_for_package(args.package)
    elif args.process:
      oat_files = get_oat_files_for_process(args.process)

  if not oat_files:
    sys.stderr.write('Error: Could not find any OAT/ODEX files to process.\n')
    sys.exit(1)

  proguard_map = load_proguard_map(args.map) if args.map else None

  parsed_data = list(
      stream_and_parse_oatdump(oat_files, proguard_map, is_host_file=is_host)
  )

  if not parsed_data:
    print('No methods found in oatdump. The report will be empty.')
  else:
    print(f'Generating pprof report to {args.output}...')
    generate_pprof_report(parsed_data, args.output, args.dex_file_oriented, sys.argv)
    print('Done.')
    print(f'Report saved to: {os.path.abspath(args.output)}')
    print(f'Tip: To upload to pprof web UI, run:')
    print(f'pprof -flame {os.path.abspath(args.output)}')


if __name__ == '__main__':
  main(sys.argv[1:])

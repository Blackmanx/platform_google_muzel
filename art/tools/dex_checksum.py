#!/usr/bin/python3
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
# Script to calculate (and maybe update) the checksum of a file.
#

import sys
import zlib
import struct

def calculate_dex_checksum(data):
    if len(data) < 12:
        raise ValueError("Data is too small to be a valid DEX file.")
    # The checksum is calculated from byte 12 to the end of the file.
    checksum = zlib.adler32(data[12:])
    return checksum & 0xffffffff  # Ensure unsigned 32-bit

def update_dex_checksum(file_path):
    try:
        with open(file_path, 'rb') as f:
            content = bytearray(f.read())

        if len(content) < 12:
            print("Error: File is too small to be a valid DEX file.", file=sys.stderr)
            return False

        original_checksum = struct.unpack('<I', content[8:12])[0]
        new_checksum = calculate_dex_checksum(content)

        if original_checksum == new_checksum:
            print(f"Checksum is already correct: {new_checksum:08x}")
            return True

        print(f"Updating checksum from {original_checksum:08x} to {new_checksum:08x}")
        content[8:12] = struct.pack('<I', new_checksum)

        with open(file_path, 'wb') as f:
            f.write(content)
        print(f"Successfully updated checksum in {file_path}")
        return True

    except FileNotFoundError:
        print(f"Error: File not found: {file_path}", file=sys.stderr)
        return False
    except Exception as e:
        print(f"Error updating checksum: {e}", file=sys.stderr)
        return False

def print_dex_checksum(file_path):
    try:
        with open(file_path, 'rb') as f:
            content = f.read()
        checksum = calculate_dex_checksum(content)
        print(f"{checksum:08x}")
    except FileNotFoundError:
        print(f"Error: File not found: {file_path}", file=sys.stderr)
    except Exception as e:
        print(f"Error calculating checksum: {e}", file=sys.stderr)

if __name__ == "__main__":
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        print("Usage:", file=sys.stderr)
        print("  python dex_checksum.py <file_path>           "
              "(Calculate and print checksum)", file=sys.stderr)
        print("  python dex_checksum.py --update <file_path>  "
              "(Calculate and update checksum in file)", file=sys.stderr)
        sys.exit(1)

    if sys.argv[1] == "--update":
        if len(sys.argv) != 3:
            print("Usage: python dex_checksum.py --update <file_path>", file=sys.stderr)
            sys.exit(1)
        file_path = sys.argv[2]
        update_dex_checksum(file_path)
    else:
        if len(sys.argv) != 2:
            print("Usage: python dex_checksum.py <file_path>", file=sys.stderr)
            sys.exit(1)
        file_path = sys.argv[1]
        print_dex_checksum(file_path)

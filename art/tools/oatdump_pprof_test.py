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

import gzip
import os
import sys
import tempfile
import unittest
import oatdump_pprof

try:
  import profile_pb2
except ImportError:
  # Allow running the script from a checkout, not as a python_binary_host
  ANDROID_BUILD_TOP = os.environ.get('ANDROID_BUILD_TOP')
  sys.path.append(ANDROID_BUILD_TOP + '/system/extras/simpleperf/scripts')
  import profile_pb2


class OatdumpPprofTest(unittest.TestCase):

  def test_oatdump_pprof_smoke(self):
    with tempfile.TemporaryDirectory() as temp_dir:
      oatdump_file_path = os.path.join(temp_dir, 'oatdump.txt')
      map_file_path = os.path.join(temp_dir, 'proguard.map')
      output_file_path = os.path.join(temp_dir, 'output.pprof')

      with open(oatdump_file_path, 'w') as f:
        f.write("""
OatDexFile:
location: /system_ext/priv-app/SystemUIGoogle/SystemUIGoogle.apk!classes2.dex
1: Landroid/app/Activity; (2 methods)
  1: void android.app.Activity.<init>() (dex_method_idx=0)
    CODE: (offset=0x00001234 size=64)
  2: void android.app.Activity.onCreate(android.os.Bundle) (dex_method_idx=1)
    CODE: (offset=0x00005678 size=128)
a: La; (1 method)
  1: void a.a() (dex_method_idx=3)
    CODE: (offset=0x0000def0 size=256)
""")

      with open(map_file_path, 'w') as f:
        f.write("""com.example.MyClass -> a:
    void myMethod() -> a
""")

      oatdump_pprof.main([
          '--oatdump-file',
          oatdump_file_path,
          '--map',
          map_file_path,
          '--output',
          output_file_path,
      ])

      self.assertTrue(os.path.exists(output_file_path))
      self.assertGreater(os.path.getsize(output_file_path), 0)

      with gzip.open(output_file_path, 'rb') as f:
        profile = profile_pb2.Profile()
        profile.ParseFromString(f.read())
        string_table = profile.string_table
        self.assertIn(os.path.basename(oatdump_file_path), string_table)
        self.assertIn('android', string_table)
        self.assertIn('android.app', string_table)
        self.assertIn('android.app.Activity', string_table)
        self.assertIn(
            'android.app.Activity.onCreate(android.os.Bundle)', string_table
        )
        self.assertIn('com.example.MyClass', string_table)
        self.assertIn('com.example.MyClass.myMethod()', string_table)
        self.assertNotIn('a', string_table)
        self.assertNotIn('a()', string_table)
        self.assertNotIn('a.a()', string_table)


if __name__ == '__main__':
  unittest.main()

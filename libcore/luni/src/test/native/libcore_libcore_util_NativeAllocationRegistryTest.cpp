/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include <atomic>
#include <string>

#include <android-base/macros.h>
#include <jni.h>
#include <nativehelper/JNIHelp.h>
#include <nativehelper/ScopedUtfChars.h>

#if defined(__BIONIC__)
#include <sys/system_properties.h>
#endif

extern "C"
jboolean Java_libcore_libcore_util_NativeAllocationRegistryTest_isNativeBridgedABI(JNIEnv*, jclass) {
#if defined(__BIONIC__)
  return __system_property_find("ro.dalvik.vm.isa." ABI_STRING) != nullptr;
#else
  return false;
#endif
}

std::atomic<uint64_t> gNumNativeBytesAllocated = 0;

static void finalize(uint64_t* ptr) {
  gNumNativeBytesAllocated -= *ptr;
  delete ptr;
}

extern "C"
jlong Java_libcore_libcore_util_NativeAllocationRegistryTest_getNativeFinalizer(JNIEnv*, jclass) {
  return static_cast<jlong>(reinterpret_cast<uintptr_t>(&finalize));
}

extern "C"
jlong Java_libcore_libcore_util_NativeAllocationRegistryTest_doNativeAllocation(JNIEnv*,
                                                                        jclass,
                                                                        jlong size) {
  gNumNativeBytesAllocated += size;

  // The actual allocation is a pointer to the pretend size of the allocation.
  uint64_t* ptr = new uint64_t;
  *ptr = static_cast<uint64_t>(size);
  return static_cast<jlong>(reinterpret_cast<uintptr_t>(ptr));
}

extern "C"
jlong Java_libcore_libcore_util_NativeAllocationRegistryTest_getNumNativeBytesAllocated(JNIEnv*, jclass) {
  return static_cast<jlong>(gNumNativeBytesAllocated);
}

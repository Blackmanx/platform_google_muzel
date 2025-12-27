/*
 * Copyright (C) 2025 The Android Open Source Project
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

// An implementation of the native-bridge interface for testing.

#include "NativeBridge8IdentifyTrampolines_lib.h"
#include "nativebridge/native_bridge.h"

// NativeBridgeCallbacks implementations
extern "C" bool native_bridge8_initialize(
    const android::NativeBridgeRuntimeCallbacks* /* art_cbs */,
    const char* /* app_code_cache_dir */,
    const char* /* isa */) {
  return true;
}

extern "C" void* native_bridge8_loadLibrary(const char* /* libpath */, int /* flag */) {
  return nullptr;
}

extern "C" void* native_bridge8_getTrampoline(void* /* handle */,
                                              const char* /* name */,
                                              const char* /* shorty */,
                                              uint32_t /* len */) {
  return nullptr;
}

extern "C" void* native_bridge8_getTrampoline2(void* /* handle */,
                                               const char* /* name */,
                                               const char* /* shorty */,
                                               uint32_t /* len */,
                                               android::JNICallType /* jni_call_type */) {
  return nullptr;
}

extern "C" void* native_bridge8_getTrampolineForFunctionPointer(
    const void* /* method */,
    const char* /* shorty */,
    uint32_t /* len */,
    android::JNICallType /* jni_call_type */) {
  return nullptr;
}

extern "C" bool native_bridge8_isSupported(const char* /* libpath */) { return false; }

extern "C" const struct android::NativeBridgeRuntimeValues* native_bridge8_getAppEnv(
    const char* /* abi */) {
  return nullptr;
}

extern "C" bool native_bridge8_isCompatibleWith(uint32_t version) {
  // For testing, allow 1-8, but disallow 9+.
  return version <= 8;
}

extern "C" android::NativeBridgeSignalHandlerFn native_bridge8_getSignalHandler(int /* signal */) {
  return nullptr;
}

extern "C" int native_bridge8_unloadLibrary(void* /* handle */) { return 0; }

extern "C" const char* native_bridge8_getError() { return nullptr; }

extern "C" bool native_bridge8_isPathSupported(const char* /* path */) { return true; }

extern "C" android::native_bridge_namespace_t* native_bridge8_createNamespace(
    const char* /* name */,
    const char* /* ld_library_path */,
    const char* /* default_library_path */,
    uint64_t /* type */,
    const char* /* permitted_when_isolated_path */,
    android::native_bridge_namespace_t* /* parent_ns */) {
  return nullptr;
}

extern "C" bool native_bridge8_linkNamespaces(android::native_bridge_namespace_t* /* from */,
                                              android::native_bridge_namespace_t* /* to */,
                                              const char* /* shared_libs_soname */) {
  return true;
}

extern "C" void* native_bridge8_loadLibraryExt(const char* /* libpath */,
                                               int /* flag */,
                                               android::native_bridge_namespace_t* /* ns */) {
  return nullptr;
}

extern "C" android::native_bridge_namespace_t* native_bridge8_getVendorNamespace() {
  return nullptr;
}

extern "C" android::native_bridge_namespace_t* native_bridge8_getExportedNamespace(
    const char* /* name */) {
  return nullptr;
}

extern "C" bool native_bridge8_isNativeBridgeFunctionPointer(const void* ptr) {
  android::SetIsNativeBridgeFunctionPointerCalledFor(ptr);
  return true;
}

extern "C" void native_bridge8_preZygoteFork() {}

android::NativeBridgeCallbacks NativeBridgeItf{
    // v1
    .version = 8,
    .initialize = &native_bridge8_initialize,
    .loadLibrary = &native_bridge8_loadLibrary,
    .getTrampoline = &native_bridge8_getTrampoline,
    .isSupported = &native_bridge8_isSupported,
    .getAppEnv = &native_bridge8_getAppEnv,
    // v2
    .isCompatibleWith = &native_bridge8_isCompatibleWith,
    .getSignalHandler = &native_bridge8_getSignalHandler,
    // v3
    .unloadLibrary = &native_bridge8_unloadLibrary,
    .getError = &native_bridge8_getError,
    .isPathSupported = &native_bridge8_isPathSupported,
    .unused_initAnonymousNamespace = nullptr,
    .createNamespace = &native_bridge8_createNamespace,
    .linkNamespaces = &native_bridge8_linkNamespaces,
    .loadLibraryExt = &native_bridge8_loadLibraryExt,
    // v4
    &native_bridge8_getVendorNamespace,
    // v5
    &native_bridge8_getExportedNamespace,
    // v6
    &native_bridge8_preZygoteFork,
    // v7
    &native_bridge8_getTrampoline2,
    &native_bridge8_getTrampolineForFunctionPointer,
    // v8
    &native_bridge8_isNativeBridgeFunctionPointer,
};

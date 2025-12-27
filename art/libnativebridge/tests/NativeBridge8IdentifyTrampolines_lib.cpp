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

#include "NativeBridge8IdentifyTrampolines_lib.h"

namespace android {

static const void* g_is_native_bridge_function_pointer_called_for = nullptr;

void SetIsNativeBridgeFunctionPointerCalledFor(const void* ptr) {
  g_is_native_bridge_function_pointer_called_for = ptr;
}

bool IsNativeBridgeFunctionPointerCalledFor(const void* ptr) {
  return g_is_native_bridge_function_pointer_called_for == ptr;
}

}  // namespace android

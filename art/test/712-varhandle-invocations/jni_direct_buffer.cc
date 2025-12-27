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

#include "jni.h"
#include <string.h>
#include <cstdlib>

namespace art {

static constexpr size_t SIZE = 64;

extern "C" JNIEXPORT jobject JNICALL Java_VarHandleArrayTests_getNativeBuffer(
    JNIEnv* env, [[maybe_unused]] jclass klass) {
  void* buffer = malloc(SIZE);
  memset(buffer, 0, SIZE);
  return env->NewDirectByteBuffer(buffer, SIZE);
}

extern "C" JNIEXPORT void JNICALL Java_VarHandleArrayTests_free(
    JNIEnv* env, [[maybe_unused]] jclass klass, jobject jbuffer) {
  free(static_cast<void*>(env->GetDirectBufferAddress(jbuffer)));
}

}  // namespace art

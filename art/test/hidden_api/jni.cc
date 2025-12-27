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

#include <jni.h>
#include <log/log.h>

extern "C" JNIEXPORT void JNICALL
Java_android_art_HiddenApiSdk28AppTest_00024NativeRunnable_run(JNIEnv* env, jobject) {
  ALOGI("HiddenApiSdk28AppTest$NativeRunnable.run enter");

  // Check some APIs that are accessible from an app with an old enough target
  // SDK but not from platform. The 674-hiddenapi run test checks that they
  // aren't actually in the core platform API and would work regardless.

  {
    jclass byte_class = env->FindClass("java/lang/Byte");
    LOG_ALWAYS_FATAL_IF(byte_class == nullptr);
    jfieldID value_field = env->GetFieldID(byte_class, "value", "B");
    LOG_ALWAYS_FATAL_IF(value_field == nullptr);
  }

  {
    jclass unsafe_class = env->FindClass("sun/misc/Unsafe");
    LOG_ALWAYS_FATAL_IF(unsafe_class == nullptr);
    jmethodID method_field =
        env->GetMethodID(unsafe_class, "getAndAddInt", "(Ljava/lang/Object;JI)I");
    LOG_ALWAYS_FATAL_IF(method_field == nullptr);
  }

  ALOGI("HiddenApiSdk28AppTest$NativeRunnable.run ok");
}

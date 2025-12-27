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

// This code goes into libarttest(d)_external.so, which means:
// -  It's in the default linker namespace and does not have access to ART
//    internals, but it can access the libraries on
//    NATIVELOADER_DEFAULT_NAMESPACE_LIBS (which includes libarttest(d).so)
//    after addDefaultNamespaceLibsLinkToSystemLinkerNamespace has been called.
// -  It's under /data, so hiddenapi classifies it as app domain.

#include "android-base/logging.h"
#include "jni.h"
#include "ti-agent/scoped_utf_chars.h"

namespace art {
namespace Test674HiddenApi {

namespace AppJniApiCallers {
#include "jni_api_callers.h"
}  // namespace AppJniApiCallers

extern "C" JNIEXPORT void JNICALL Java_ChildClass_registerAppJniApiCallers(JNIEnv* env,
                                                                           jclass,
                                                                           jclass target_class) {
  AppJniApiCallers::registerNatives(env, target_class);
}

}  // namespace Test674HiddenApi
}  // namespace art

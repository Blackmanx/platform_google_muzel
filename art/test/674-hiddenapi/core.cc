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

// This code goes into libarttest(d).so, which means:
// -  It's in the com_android_art linker namespace and has full access to ART internals.
// -  It's under /apex/com.android.art, so hiddenapi classifies it as
//    core-platform domain.

#include <dlfcn.h>

#include "base/sdk_version.h"
#include "dex/art_dex_file_loader.h"
#include "hidden_api.h"
#include "jni.h"
#include "runtime.h"
#include "ti-agent/scoped_utf_chars.h"

#ifdef ART_TARGET_ANDROID
#include <bionic/dlext_namespaces.h>
#endif

namespace art {
namespace Test674HiddenApi {

// Should be the same as dalvik.system.VMRuntime.PREVENT_META_REFLECTION_BLOCKLIST_ACCESS
static constexpr uint64_t kPreventMetaReflectionBlocklistAccess = 142365358;

std::vector<std::vector<std::unique_ptr<const DexFile>>> opened_dex_files;

// The JNI entrypoints below end up in libarttest(d).so, but for the native
// methods in src-ex (Java_ChildClass_* and Java_Reflection_*) the test makes
// copies of libarttest(d)_external.so and loads them instead. Those libs have
// DT_NEEDED dependencies on libarttest(d).so, so its exported symbols become
// visible directly in them too. Hence we don't need to create wrappers for them
// in libarttest(d)_external.so.

extern "C" JNIEXPORT void JNICALL
Java_Main_addDefaultNamespaceLibsLinkToSystemLinkerNamespace(JNIEnv*, jclass) {
#ifdef ART_TARGET_ANDROID
  const char* links = getenv("NATIVELOADER_DEFAULT_NAMESPACE_LIBS");
  if (links == nullptr || *links == 0) {
    LOG(FATAL) << "Expected NATIVELOADER_DEFAULT_NAMESPACE_LIBS to be set";
  }
  struct android_namespace_t* system_ns = android_get_exported_namespace("system");
  if (system_ns == nullptr) {
    LOG(FATAL) << "Failed to retrieve system namespace";
  }
  if (!android_link_namespaces(system_ns, nullptr, links)) {
    LOG(FATAL) << "Error adding linker namespace link from system to default for " << links << ": "
               << dlerror();
  }
#endif
}

// Note that this method is used by several hiddenapi-related run tests.
extern "C" JNIEXPORT void JNICALL Java_Main_init(JNIEnv* env, jclass) {
  Runtime* runtime = Runtime::Current();
  runtime->SetHiddenApiEnforcementPolicy(hiddenapi::EnforcementPolicy::kEnabled);
  runtime->SetCorePlatformApiEnforcementPolicy(hiddenapi::EnforcementPolicy::kEnabled);
  runtime->SetDedupeHiddenApiWarnings(false);

  // Set targetSdkVersion through the Java method, to get the same setting in
  // both the Java and native instances.
  ScopedLocalRef<jclass> runtime_class(env, env->FindClass("dalvik/system/VMRuntime"));
  CHECK_NE(runtime_class.get(), nullptr);
  jmethodID get_runtime =
      env->GetStaticMethodID(runtime_class.get(), "getRuntime", "()Ldalvik/system/VMRuntime;");
  CHECK_NE(get_runtime, nullptr);
  ScopedLocalRef<jobject> runtime_obj(
      env, env->CallStaticObjectMethod(runtime_class.get(), get_runtime));
  CHECK_NE(runtime_obj.get(), nullptr);
  jmethodID set_target_sdk_version =
      env->GetMethodID(runtime_class.get(), "setTargetSdkVersion", "(I)V");
  CHECK_NE(set_target_sdk_version, nullptr);
  uint32_t target_sdk_version =
      static_cast<uint32_t>(hiddenapi::ApiList::MaxTargetO().GetMaxAllowedSdkVersion());
  env->CallVoidMethod(runtime_obj.get(), set_target_sdk_version, target_sdk_version);
}

extern "C" JNIEXPORT jboolean JNICALL Java_Main_runningOnHost(JNIEnv*, jclass) {
#ifdef ART_TARGET_ANDROID
  return JNI_FALSE;
#else
  return JNI_TRUE;
#endif
}

extern "C" JNIEXPORT void JNICALL Java_Main_setDexDomain(JNIEnv*,
                                                         jclass,
                                                         jint int_index,
                                                         jboolean is_core_platform) {
  size_t index = static_cast<size_t>(int_index);
  CHECK_LT(index, opened_dex_files.size());
  for (std::unique_ptr<const DexFile>& dex_file : opened_dex_files[index]) {
    const_cast<DexFile*>(dex_file.get())
        ->SetHiddenapiDomain((is_core_platform == JNI_FALSE) ? hiddenapi::Domain::kPlatform
                                                             : hiddenapi::Domain::kCorePlatform);
  }
}

extern "C" JNIEXPORT jint JNICALL Java_Main_appendToBootClassLoader(JNIEnv* env,
                                                                    jclass klass,
                                                                    jstring jpath,
                                                                    jboolean is_core_platform) {
  ScopedUtfChars utf(env, jpath);
  const char* path = utf.c_str();
  CHECK(path != nullptr);

  const size_t index = opened_dex_files.size();
  const jint int_index = static_cast<jint>(index);
  opened_dex_files.push_back(std::vector<std::unique_ptr<const DexFile>>());

  DexFileLoader dex_loader(path);
  std::string error_msg;

  if (!dex_loader.Open(/* verify */ false,
                       /* verify_checksum */ true,
                       &error_msg,
                       &opened_dex_files[index])) {
    LOG(FATAL) << "Could not open " << path << " for boot classpath extension: " << error_msg;
    UNREACHABLE();
  }

  Java_Main_setDexDomain(env, klass, int_index, is_core_platform);

  Runtime::Current()->AppendToBootClassPath(path, path, opened_dex_files[index]);

  return int_index;
}

extern "C" JNIEXPORT void JNICALL Java_Main_setSdkAll(JNIEnv*, jclass, jboolean value) {
  std::vector<std::string> exemptions;
  if (value != JNI_FALSE) {
    exemptions.push_back("L");
  }
  Runtime::Current()->SetHiddenApiExemptions(exemptions);
}

namespace CorePlatformJniApiCallers {
#include "jni_api_callers.h"
}  // namespace CorePlatformJniApiCallers

extern "C" JNIEXPORT void JNICALL
Java_ChildClass_registerCorePlatformJniApiCallers(JNIEnv* env, jclass, jclass target_class) {
  CorePlatformJniApiCallers::registerNatives(env, target_class);
}

extern "C" JNIEXPORT jint JNICALL Java_Reflection_getHiddenApiAccessFlags(JNIEnv*, jclass) {
  return static_cast<jint>(kAccHiddenapiBits);
}

extern "C" JNIEXPORT void JNICALL Java_Reflection_setHiddenApiCheckHardening(JNIEnv*,
                                                                             jclass,
                                                                             jboolean value) {
  CompatFramework& compat_framework = Runtime::Current()->GetCompatFramework();
  std::set<uint64_t> disabled_changes = compat_framework.GetDisabledCompatChanges();
  if (value == JNI_TRUE) {
    // If hidden api check hardening is enabled, remove it from the set of disabled changes.
    disabled_changes.erase(kPreventMetaReflectionBlocklistAccess);
  } else {
    // If hidden api check hardening is disabled, add it to the set of disabled changes.
    disabled_changes.insert(kPreventMetaReflectionBlocklistAccess);
  }
  compat_framework.SetDisabledCompatChanges(disabled_changes);
}

}  // namespace Test674HiddenApi
}  // namespace art

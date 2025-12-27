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

// This isn't really a header file, but add a guard anyway just to make cpplint happy.
#ifndef ART_TEST_674_HIDDENAPI_JNI_API_CALLERS_H_
#define ART_TEST_674_HIDDENAPI_JNI_API_CALLERS_H_

// Implementations for the native methods in src-ex/JNI.java which test accesses
// through the JNI API. These are defined twice to put them physically in two
// different .so libs:
// -  Once in core.cc to go into libarttest(d).so, which classifies them as
//    core-platform domain.
// -  Once in app.cc to go into libarttest(d)_external.so, which gets copied to
//    libhiddenapitest_xxx.so in the run test directory, which classifies them
//    as app domain.
// There is no easy way to put them in a .so lib in a location that classifies
// them as platform domain, so we don't test that.

static jobject NewInstance(JNIEnv* env, jclass klass) {
  jmethodID constructor = env->GetMethodID(klass, "<init>", "()V");
  if (constructor == nullptr) {
    return nullptr;
  }
  return env->NewObject(klass, constructor);
}

static jboolean JNICALL
canDiscoverField(JNIEnv* env, jclass, jclass klass, jstring name, jboolean is_static) {
  ScopedUtfChars utf_name(env, name);
  jfieldID field = is_static ? env->GetStaticFieldID(klass, utf_name.c_str(), "I")
                             : env->GetFieldID(klass, utf_name.c_str(), "I");
  if (field == nullptr) {
    env->ExceptionClear();
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

static jboolean JNICALL
canGetField(JNIEnv* env, jclass, jclass klass, jstring name, jboolean is_static) {
  ScopedUtfChars utf_name(env, name);
  jfieldID field = is_static ? env->GetStaticFieldID(klass, utf_name.c_str(), "I")
                             : env->GetFieldID(klass, utf_name.c_str(), "I");
  if (field == nullptr) {
    env->ExceptionClear();
    return JNI_FALSE;
  }
  if (is_static) {
    env->GetStaticIntField(klass, field);
  } else {
    jobject obj = NewInstance(env, klass);
    if (obj == nullptr) {
      env->ExceptionDescribe();
      env->ExceptionClear();
      return JNI_FALSE;
    }
    env->GetIntField(obj, field);
  }

  if (env->ExceptionOccurred()) {
    env->ExceptionDescribe();
    env->ExceptionClear();
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

static jboolean JNICALL
canSetField(JNIEnv* env, jclass, jclass klass, jstring name, jboolean is_static) {
  ScopedUtfChars utf_name(env, name);
  jfieldID field = is_static ? env->GetStaticFieldID(klass, utf_name.c_str(), "I")
                             : env->GetFieldID(klass, utf_name.c_str(), "I");
  if (field == nullptr) {
    env->ExceptionClear();
    return JNI_FALSE;
  }
  if (is_static) {
    env->SetStaticIntField(klass, field, 42);
  } else {
    jobject obj = NewInstance(env, klass);
    if (obj == nullptr) {
      env->ExceptionDescribe();
      env->ExceptionClear();
      return JNI_FALSE;
    }
    env->SetIntField(obj, field, 42);
  }

  if (env->ExceptionOccurred()) {
    env->ExceptionDescribe();
    env->ExceptionClear();
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

static jboolean JNICALL
canDiscoverMethod(JNIEnv* env, jclass, jclass klass, jstring name, jboolean is_static) {
  ScopedUtfChars utf_name(env, name);
  jmethodID method = is_static ? env->GetStaticMethodID(klass, utf_name.c_str(), "()I")
                               : env->GetMethodID(klass, utf_name.c_str(), "()I");
  if (method == nullptr) {
    env->ExceptionClear();
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

static jboolean JNICALL
canInvokeMethodA(JNIEnv* env, jclass, jclass klass, jstring name, jboolean is_static) {
  ScopedUtfChars utf_name(env, name);
  jmethodID method = is_static ? env->GetStaticMethodID(klass, utf_name.c_str(), "()I")
                               : env->GetMethodID(klass, utf_name.c_str(), "()I");
  if (method == nullptr) {
    env->ExceptionClear();
    return JNI_FALSE;
  }

  if (is_static) {
    env->CallStaticIntMethodA(klass, method, nullptr);
  } else {
    jobject obj = NewInstance(env, klass);
    if (obj == nullptr) {
      env->ExceptionDescribe();
      env->ExceptionClear();
      return JNI_FALSE;
    }
    env->CallIntMethodA(obj, method, nullptr);
  }

  if (env->ExceptionOccurred()) {
    env->ExceptionDescribe();
    env->ExceptionClear();
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

static jboolean JNICALL
canInvokeMethodV(JNIEnv* env, jclass, jclass klass, jstring name, jboolean is_static) {
  ScopedUtfChars utf_name(env, name);
  jmethodID method = is_static ? env->GetStaticMethodID(klass, utf_name.c_str(), "()I")
                               : env->GetMethodID(klass, utf_name.c_str(), "()I");
  if (method == nullptr) {
    env->ExceptionClear();
    return JNI_FALSE;
  }

  if (is_static) {
    env->CallStaticIntMethod(klass, method);
  } else {
    jobject obj = NewInstance(env, klass);
    if (obj == nullptr) {
      env->ExceptionDescribe();
      env->ExceptionClear();
      return JNI_FALSE;
    }
    env->CallIntMethod(obj, method);
  }

  if (env->ExceptionOccurred()) {
    env->ExceptionDescribe();
    env->ExceptionClear();
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

static constexpr size_t kConstructorSignatureLength = 5;  // e.g. (IZ)V
static constexpr size_t kNumConstructorArgs = kConstructorSignatureLength - 3;

static jboolean JNICALL canDiscoverConstructor(JNIEnv* env, jclass, jclass klass, jstring args) {
  ScopedUtfChars utf_args(env, args);
  jmethodID constructor = env->GetMethodID(klass, "<init>", utf_args.c_str());
  if (constructor == nullptr) {
    env->ExceptionClear();
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

static jboolean JNICALL canInvokeConstructorA(JNIEnv* env, jclass, jclass klass, jstring args) {
  ScopedUtfChars utf_args(env, args);
  jmethodID constructor = env->GetMethodID(klass, "<init>", utf_args.c_str());
  if (constructor == nullptr) {
    env->ExceptionClear();
    return JNI_FALSE;
  }

  // CheckJNI won't allow out-of-range values, so just zero everything.
  CHECK_EQ(strlen(utf_args.c_str()), kConstructorSignatureLength);
  size_t initargs_size = sizeof(jvalue) * kNumConstructorArgs;
  jvalue* initargs = reinterpret_cast<jvalue*>(alloca(initargs_size));
  memset(initargs, 0, initargs_size);

  env->NewObjectA(klass, constructor, initargs);
  if (env->ExceptionOccurred()) {
    env->ExceptionDescribe();
    env->ExceptionClear();
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

static jboolean JNICALL canInvokeConstructorV(JNIEnv* env, jclass, jclass klass, jstring args) {
  ScopedUtfChars utf_args(env, args);
  jmethodID constructor = env->GetMethodID(klass, "<init>", utf_args.c_str());
  if (constructor == nullptr) {
    env->ExceptionClear();
    return JNI_FALSE;
  }

  // CheckJNI won't allow out-of-range values, so just zero everything.
  CHECK_EQ(strlen(utf_args.c_str()), kConstructorSignatureLength);
  size_t initargs_size = sizeof(jvalue) * kNumConstructorArgs;
  jvalue* initargs = reinterpret_cast<jvalue*>(alloca(initargs_size));
  memset(initargs, 0, initargs_size);

  static_assert(kNumConstructorArgs == 2, "Change the varargs below if you change the constant");
  env->NewObject(klass, constructor, initargs[0], initargs[1]);
  if (env->ExceptionOccurred()) {
    env->ExceptionDescribe();
    env->ExceptionClear();
    return JNI_FALSE;
  }

  return JNI_TRUE;
}

static void registerNatives(JNIEnv* env, jclass target_class) {
  static const char* csb_sig = "(Ljava/lang/Class;Ljava/lang/String;Z)Z";
  static const char* cs_sig = "(Ljava/lang/Class;Ljava/lang/String;)Z";
  static const JNINativeMethod methods[] = {
      {"canDiscoverField", csb_sig, reinterpret_cast<void*>(canDiscoverField)},
      {"canGetField", csb_sig, reinterpret_cast<void*>(canGetField)},
      {"canSetField", csb_sig, reinterpret_cast<void*>(canSetField)},
      {"canDiscoverMethod", csb_sig, reinterpret_cast<void*>(canDiscoverMethod)},
      {"canInvokeMethodA", csb_sig, reinterpret_cast<void*>(canInvokeMethodA)},
      {"canInvokeMethodV", csb_sig, reinterpret_cast<void*>(canInvokeMethodV)},
      {"canDiscoverConstructor", cs_sig, reinterpret_cast<void*>(canDiscoverConstructor)},
      {"canInvokeConstructorA", cs_sig, reinterpret_cast<void*>(canInvokeConstructorA)},
      {"canInvokeConstructorV", cs_sig, reinterpret_cast<void*>(canInvokeConstructorV)},
  };
  CHECK_EQ(env->RegisterNatives(target_class, methods, sizeof(methods) / sizeof(methods[0])), 0);
}

#endif  // ART_TEST_674_HIDDENAPI_JNI_API_CALLERS_H_

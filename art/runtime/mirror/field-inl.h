/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef ART_RUNTIME_MIRROR_FIELD_INL_H_
#define ART_RUNTIME_MIRROR_FIELD_INL_H_

#include "class_root.h"
#include "field.h"

#include "art_field-inl.h"
#include "class-alloc-inl.h"
#include "class_root-inl.h"
#include "object-inl.h"
#include "base/sdk_version.h"
#include "runtime.h"
#include "well_known_classes.h"

namespace art HIDDEN {

namespace mirror {

inline ObjPtr<mirror::Class> Field::GetDeclaringClass() REQUIRES_SHARED(Locks::mutator_lock_) {
  return GetFieldObject<Class>(OFFSET_OF_OBJECT_MEMBER(Field, declaring_class_));
}

inline bool Field::IsUnmodifiable() REQUIRES_SHARED(Locks::mutator_lock_) {
  if (!IsFinal()) {
    return false;
  }

  ObjPtr<mirror::Class> declaring_class = GetDeclaringClass();
  DCHECK(declaring_class != nullptr);

  // Write-protected fields are `static final`, but can be modified nevertheless.
  if (IsWriteProtected()) {
    return false;
  }

  if ((GetAccessFlags() & kAccMonotonic) != 0) {
    return true;
  }

  // Before and on Android B any field could be overwritten using reflection with final fields in
  // record classes being the only exception. For compatibility purposes allow apps targeting B
  // or an older release to overwrite such fields.
  uint32_t target_sdk_version = Runtime::Current()->GetTargetSdkVersion();
  if (IsSdkVersionSetAndAtMost(target_sdk_version, SdkVersion::kB)) {
    return false;
  }

  // `static final` fields with declared VarHandle, MethodHandle or Atomic*FieldUpdater types
  // are unmodifiable on apps targeting C or higher.
  ObjPtr<mirror::Class> field_type = GetType();
  if (IsStatic() && field_type->IsBootStrapClassLoaded()) {
    // These classes are abstract and exact implementations are exposed neither to apps
    // nor in the platform, hence plain comparison instead of subtype checks.
    if (field_type == GetClassRoot(ClassRoot::kJavaLangInvokeMethodHandle) ||
        field_type == GetClassRoot(ClassRoot::kJavaLangInvokeVarHandle) ||
        field_type == WellKnownClasses::ToClass(
            WellKnownClasses::java_util_concurrent_atomic_AIFU) ||
        field_type == WellKnownClasses::ToClass(
            WellKnownClasses::java_util_concurrent_atomic_ALFU) ||
        field_type == WellKnownClasses::ToClass(
            WellKnownClasses::java_util_concurrent_atomic_ARFU)) {
      return true;
    }
  }

  // Make sure that OEMs code in bootclasspath won't be affected after ART module update.
  uint32_t sdk_version = Runtime::Current()->GetSdkVersion();
  if (IsSdkVersionSetAndAtMost(sdk_version, SdkVersion::kB)) {
    return false;
  }

  // static final fields can't be modified once initialized.
  return IsStatic();
}

inline bool Field::IsWriteProtected() {
  ArtField* art_field = GetArtField();
  if (art_field == WellKnownClasses::java_lang_System_in ||
      art_field == WellKnownClasses::java_lang_System_out ||
      art_field == WellKnownClasses::java_lang_System_err) {
    return true;
  }
  // TODO(b/423809429): some `static final` fields defined in android.os.Build and
  // android.os.Build$VERSION are overwritten for App Compat reasons on dogfood builds.
  // Once these fields are no longer modified checks below could be removed altogether as released
  // Android versions should not modify these fields.
  if (IsStatic() && IsFinal()) {
    ObjPtr<mirror::Class> declaring_class = GetDeclaringClass();
    if (!declaring_class->IsBootStrapClassLoaded()) {
      return false;
    }
    if (declaring_class->DescriptorEquals("Landroid/os/Build;") ||
        declaring_class->DescriptorEquals("Landroid/os/Build$VERSION;")) {
      return true;
    }
  }
  return false;
}

inline Primitive::Type Field::GetTypeAsPrimitiveType() {
  return GetType()->GetPrimitiveType();
}

inline ObjPtr<mirror::Class> Field::GetType() {
  return GetFieldObject<mirror::Class>(OFFSET_OF_OBJECT_MEMBER(Field, type_));
}

template<bool kTransactionActive, bool kCheckTransaction>
inline void Field::SetDeclaringClass(ObjPtr<Class> c) {
  SetFieldObject<kTransactionActive, kCheckTransaction>(DeclaringClassOffset(), c);
}

template<bool kTransactionActive, bool kCheckTransaction>
inline void Field::SetType(ObjPtr<Class> type) {
  SetFieldObject<kTransactionActive, kCheckTransaction>(TypeOffset(), type);
}

}  // namespace mirror
}  // namespace art

#endif  // ART_RUNTIME_MIRROR_FIELD_INL_H_

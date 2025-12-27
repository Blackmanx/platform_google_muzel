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

#include "assume_value_signatures.h"

#include <sstream>

#include "art_field.h"
#include "base/logging.h"
#include "mirror/class.h"
#include "runtime.h"
#include "scoped_thread_state_change-inl.h"
#include "scoped_thread_state_change.h"

namespace art_flags = com::android::art::flags;

namespace art HIDDEN {

ArtField* AssumeValueSignature::LookupField() const {
  ClassLinker* class_linker = Runtime::Current()->GetClassLinker();
  Thread* self = Thread::Current();
  ScopedObjectAccess soa(self);

  std::string class_descriptor_str(class_descriptor_);
  ObjPtr<mirror::Class> klass =
      class_linker->LookupClass(self, class_descriptor_str.c_str(), /*class_loader=*/nullptr);
  if (klass == nullptr) {
    // Some assumed values may be for members of classes that aren't always present during
    // compilation, e.g., framework classes *not* in the base ART boot image. Such missing classes
    // should not cause lookup failures during compilation.
    return nullptr;
  }

  ArtField* field = is_static_ ? klass->FindDeclaredStaticField(member_name_, type_descriptor_)
                               : klass->FindDeclaredInstanceField(member_name_, type_descriptor_);
  if (UNLIKELY(field == nullptr)) {
    std::ostringstream os;
    klass->DumpClass(os, mirror::Class::kDumpClassFullDetail);
    LOG(ERROR) << "Couldn't find " << (is_static_ ? "static" : "instance") << " field \""
               << member_name_ << "\" with signature \"" << type_descriptor_ << "\": " << os.str();
  }
  return field;
}

}  // namespace art

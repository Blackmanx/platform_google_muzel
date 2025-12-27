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

#include "assume_value_options.h"

#include "assume_value_signatures.h"
#include "base/logging.h"
#include "com_android_art_rw_flags.h"
#include "runtime.h"

namespace art_rw_flags = com::android::art::rw::flags;

namespace art HIDDEN {

bool AssumeValueOptions::MaybeGetAssumedValue(ArtField* field, int32_t* value) const {
  if (UNLIKELY(!assumed_value_overrides_.empty())) {
    const auto it = assumed_value_overrides_.find(field);
    if (it != assumed_value_overrides_.end()) {
      *value = it->second;
      return true;
    }
  }

  const auto signature = Runtime::Current()->LookupAssumeValueSignature(field);
  if (signature == AssumeValueSignatures::kSdkInt) {
    if (art_rw_flags::assume_value_sdk_int() && HasValidSdkInt()) {
      *value = sdk_int_;
      return true;
    }
  }

  return false;
}

void AssumeValueOptions::SetSdkInt(uint32_t sdk_int) {
  DCHECK(art_rw_flags::assume_value_sdk_int());
  VLOG(compiler) << "Setting assumed value for SDK_INT: " << sdk_int;
  sdk_int_ = sdk_int;
}

}  // namespace art

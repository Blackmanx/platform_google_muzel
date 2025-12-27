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

#ifndef ART_COMPILER_DRIVER_ASSUME_VALUE_OPTIONS_H_
#define ART_COMPILER_DRIVER_ASSUME_VALUE_OPTIONS_H_

#include <cstdint>
#include <map>

#include "base/macros.h"
#include "base/sdk_version.h"

namespace art HIDDEN {

class ArtField;

// A helper class containing configured values that can be safely assumed during compile time.
class AssumeValueOptions final {
 public:
  static constexpr uint32_t kUnsetSdkInt = static_cast<uint32_t>(SdkVersion::kUnset);

  AssumeValueOptions() = default;

  // Conditionally gets the assumed value for a given member, if defined.
  // Returns whether the field has a corresponding assumed value.
  bool MaybeGetAssumedValue(ArtField* field, int32_t* value) const;

  // The assumed Build.VERSION.SDK_INT value to use for compilation.
  // Defaults to kUnsetSdkInt unless explicitly configured.
  uint32_t SdkInt() const { return sdk_int_; }

  // Whether a valid, explicit SDK_INT has been set for compilation.
  bool HasValidSdkInt() const { return sdk_int_ != kUnsetSdkInt; }

  // Sets the assumed Build.VERSION.SDK_INT value to user for compilation, if supported.
  EXPORT void SetSdkInt(uint32_t sdk_int);

 private:
  // The assumed android.os.Build.VERSION.SDK_INT value to use during compilation.
  uint32_t sdk_int_ = kUnsetSdkInt;

  // Only used for testing.
  std::map<ArtField*, int32_t> assumed_value_overrides_;

  friend class CommonCompilerTestImpl;
};

}  // namespace art

#endif  // ART_COMPILER_DRIVER_ASSUME_VALUE_OPTIONS_H_

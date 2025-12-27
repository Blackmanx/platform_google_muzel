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

#ifndef ART_RUNTIME_ASSUME_VALUE_SIGNATURES_H_
#define ART_RUNTIME_ASSUME_VALUE_SIGNATURES_H_

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "base/macros.h"
#include "base/value_object.h"

namespace art HIDDEN {

class ArtField;
class AssumeValueSignatures;

// A helper class defining the signature associated with a members that can take an assumed value.
class AssumeValueSignature : public ValueObject {
 public:
  constexpr bool operator==(const AssumeValueSignature& other) const {
    return class_descriptor_ == other.class_descriptor_ &&
        member_name_ == other.member_name_ &&
        type_descriptor_ == other.type_descriptor_ &&
        is_static_ == other.is_static_;
  }
  constexpr bool operator!=(const AssumeValueSignature& other) const { return !(*this == other); }

  // Returns a string that can be used as a key associated with the member's signature.
  constexpr std::string AsKey() const {
    return std::string(class_descriptor_) + "->" + std::string(member_name_);
  }

  // Attempts to look up the field associated with the current signature.
  // If unavailable, returns nullptr.
  ArtField* LookupField() const;

 private:
  friend class AssumeValueSignatures;

  consteval AssumeValueSignature(std::string_view class_descriptor,
                                 std::string_view member_name,
                                 std::string_view type_descriptor,
                                 bool is_static)
      : class_descriptor_(class_descriptor),
        member_name_(member_name),
        type_descriptor_(type_descriptor),
        is_static_(is_static) {}

  const std::string_view class_descriptor_;
  const std::string_view member_name_;
  const std::string_view type_descriptor_;
  const bool is_static_;
};

// A helper class containing signatures for members that can take assumed values during compilation.
class AssumeValueSignatures final {
 public:
  static constexpr AssumeValueSignature kSdkInt{
      "Landroid/os/Build$VERSION;", "SDK_INT", "I", /*is_static=*/true};

  static constexpr std::array<AssumeValueSignature, 1> kSignatures{kSdkInt};

  // Looks up a possible declared signature associated with the given descriptor and member.
  // Returns nullptr if there's no such matching signature.
  static constexpr std::optional<AssumeValueSignature> Lookup(std::string_view class_descriptor,
                                                              std::string_view member_name) {
    for (const auto& signature : kSignatures) {
      if (signature.class_descriptor_ == class_descriptor &&
          signature.member_name_ == member_name) {
        return signature;
      }
    }
    return std::nullopt;
  }
};

}  // namespace art

#endif  // ART_RUNTIME_ASSUME_VALUE_SIGNATURES_H_

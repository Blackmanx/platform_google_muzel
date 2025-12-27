/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef ART_RUNTIME_VERIFIER_REG_TYPE_INL_H_
#define ART_RUNTIME_VERIFIER_REG_TYPE_INL_H_

#include "reg_type.h"

#include "base/arena_allocator.h"
#include "base/casts.h"
#include "method_verifier.h"
#include "mirror/class.h"
#include "verifier_deps.h"

namespace art HIDDEN {
namespace verifier {

inline ObjPtr<mirror::Class> RegType::GetClass() const {
  DCHECK(IsReference()) << Dump();
  return down_cast<const ReferenceType&>(*this).GetClassImpl();
}

inline Handle<mirror::Class> RegType::GetClassHandle() const {
  DCHECK(IsReference()) << Dump();
  return down_cast<const ReferenceType&>(*this).GetClassHandleImpl();
}

namespace detail {

class RegTypeAssignabilityImpl final : RegType {
 public:
  explicit constexpr RegTypeAssignabilityImpl(RegType::Kind kind)
      : RegType("", /* unused cache id */ 0, kind) {}

  static constexpr Assignability AssignabilityFrom(RegType::Kind lhs_kind, RegType::Kind rhs_kind);
};

constexpr RegType::Assignability RegTypeAssignabilityImpl::AssignabilityFrom(
    RegType::Kind lhs_kind, RegType::Kind rhs_kind) {
  RegTypeAssignabilityImpl lhs(lhs_kind);
  RegTypeAssignabilityImpl rhs(rhs_kind);
  auto maybe_narrowing_conversion = [&rhs]() constexpr {
    return rhs.IsIntegralTypes() ? Assignability::kNarrowingConversion
                                 : Assignability::kNotAssignable;
  };
  if (lhs.IsBoolean()) {
    return rhs.IsBooleanTypes() ? Assignability::kAssignable : maybe_narrowing_conversion();
  } else if (lhs.IsByte()) {
    return rhs.IsByteTypes() ? Assignability::kAssignable : maybe_narrowing_conversion();
  } else if (lhs.IsShort()) {
    return rhs.IsShortTypes() ? Assignability::kAssignable : maybe_narrowing_conversion();
  } else if (lhs.IsChar()) {
    return rhs.IsCharTypes() ? Assignability::kAssignable : maybe_narrowing_conversion();
  } else if (lhs.IsInteger()) {
    return rhs.IsIntegralTypes() ? Assignability::kAssignable : Assignability::kNotAssignable;
  } else if (lhs.IsFloat()) {
    return rhs.IsFloatTypes() ? Assignability::kAssignable : Assignability::kNotAssignable;
  } else if (lhs.IsLongLo()) {
    return rhs.IsLongTypes() ? Assignability::kAssignable : Assignability::kNotAssignable;
  } else if (lhs.IsDoubleLo()) {
    return rhs.IsDoubleTypes() ? Assignability::kAssignable : Assignability::kNotAssignable;
  } else if (lhs.IsConflict()) {
    // TODO: The `MethodVerifier` is doing a `lhs` category check for `return{,-wide,-object}`
    // before the assignability check, so a `Conflict` (`void`) is not a valid `lhs`. We could
    // speed up the verification by removing the category check and relying on the assignability
    // check. Then we would need to return `NotAssignable` here as the result would be used
    // if a value is returned from a `void` method.
    return Assignability::kInvalid;
  } else if (lhs.IsUninitializedTypes() || lhs.IsUnresolvedMergedReference()) {
    // These reference kinds are not valid `lhs`.
    return Assignability::kInvalid;
  } else if (lhs.IsNonZeroReferenceTypes()) {
    if (rhs.IsZeroOrNull()) {
      return Assignability::kAssignable;  // All reference types can be assigned null.
    } else if (!rhs.IsNonZeroReferenceTypes()) {
      return Assignability::kNotAssignable;  // Expect rhs to be a reference type.
    } else if (rhs.IsUninitializedTypes()) {
      // References of uninitialized types can be copied but not assigned.
      return Assignability::kNotAssignable;
    } else if (lhs.IsJavaLangObject()) {
      return Assignability::kAssignable;  // All reference types can be assigned to Object.
    } else {
      // Use `Reference` to tell the caller to process a reference assignability check.
      // This check requires more information than the kinds available here.
      return Assignability::kReference;
    }
  } else {
    DCHECK(lhs.IsUndefined() || lhs.IsHighHalf() || lhs.IsConstantTypes());
    return Assignability::kInvalid;
  }
}

}  // namespace detail

inline RegType::Assignability RegType::AssignabilityFrom(Kind lhs, Kind rhs) {
  static constexpr size_t kNumKinds = NumberOfKinds();
  using AssignabilityTable = std::array<std::array<Assignability, kNumKinds>, kNumKinds>;
  static constexpr AssignabilityTable kAssignabilityTable = []() constexpr {
    AssignabilityTable result;
    for (size_t lhs = 0u; lhs != kNumKinds; ++lhs) {
      for (size_t rhs = 0u; rhs != kNumKinds; ++rhs) {
        result[lhs][rhs] = detail::RegTypeAssignabilityImpl::AssignabilityFrom(
            enum_cast<RegType::Kind>(lhs), enum_cast<RegType::Kind>(rhs));
      }
    }
    return result;
  }();

  return kAssignabilityTable[lhs][rhs];
}

inline void* RegType::operator new(size_t size, ArenaAllocator* allocator) {
  return allocator->Alloc(size, kArenaAllocMisc);
}

}  // namespace verifier
}  // namespace art

#endif  // ART_RUNTIME_VERIFIER_REG_TYPE_INL_H_

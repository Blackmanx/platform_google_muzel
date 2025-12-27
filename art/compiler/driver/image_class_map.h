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

#ifndef ART_COMPILER_DRIVER_IMAGE_CLASS_MAP_H_
#define ART_COMPILER_DRIVER_IMAGE_CLASS_MAP_H_

#include "base/hash_map.h"
#include "base/macros.h"
#include "dex/type_reference.h"

namespace art HIDDEN {

// The `ImageClassMap` maps a `TypeReference` to the maximum needed array dimension.
//
// Note: There can be identical descriptors in different dex files.
// These are treated as different types as they may be loaded by different
// class loaders, for example when `DelegateLastClassLoader` is involved.
class ImageClassMap {
 public:
  // Add an image class with the required maximum array dimensions if not yet present.
  // All lower dimensions are implicitly added. For example, if `type_ref` represents
  // `LCls;` and `array_dim` is 2, this effectively adds `[[LCls;`, `[LCls;` and `LCls;`.
  // Pass `array_dim = 0` to add only the non-array class.
  //
  // The `type_ref` should reference a class that has a class definition in the dex file,
  // or a primitive type descriptor for the `core-oj` component of the primary boot image.
  void Add(TypeReference type_ref, size_t array_dim);

  // Remove an image class. The `type_ref` should reference a non-array class and all
  // its array classes are implicitly removed.
  //
  // Note: This can be used to remove primitive types but we should never need to do that.
  // For `core-oj`, we should include all primitive types and for other dex files, we
  // should never add primitive types at all.
  void Remove(TypeReference type_ref);

  // Check if the class referenced by `type_ref` is present with at least `array_dim` dimensions.
  // The `type_ref` should reference a non-array class, see `Add()`.
  bool Contains(TypeReference type_ref, size_t array_dim) const;

  // Check if the class referenced by `dex_file` and `descriptor` is present with at least
  // `array_dim` dimensions. The `descriptor` should be a non-array descriptor.
  // Lookup by descriptor is supported to avoid searching for the `TypeId` in the dex file.
  bool Contains(const DexFile* dex_file, std::string_view descriptor, size_t array_dim) const;

  // Visit each entry, passing the `TypeReference type_ref` and `size_t array_dim` to `visitor`.
  template <typename Visitor>
  void ForEach(Visitor&& visitor) const;

  size_t size() const {
    return map_.size();
  }

  bool empty() const {
    return map_.empty();
  }

 private:
  // Using local version of `TypeReference` that can be default-constructed.
  struct TypeRef {
    TypeRef() : dex_file(nullptr), type_index(0u) {}
    explicit TypeRef(TypeReference type_ref)
        : dex_file(type_ref.dex_file), type_index(type_ref.TypeIndex()) {}

    const DexFile* dex_file;
    dex::TypeIndex type_index;
  };

  using DexFileAndDescriptor = std::pair<const DexFile*, std::string_view>;

  struct TypeRefEmpty {
    void MakeEmpty(std::pair<TypeRef, size_t>& item) const {
      item.first.dex_file = nullptr;
    }
    bool IsEmpty(const std::pair<TypeRef, size_t>& item) const {
      return item.first.dex_file == nullptr;
    }
  };

  struct TypeRefHash {
    size_t operator()(const TypeRef& type_ref) const;
    size_t operator()(const DexFileAndDescriptor& dex_file_and_descriptor) const;
  };

  struct TypeRefEqual {
    bool operator()(const TypeRef& lhs, const TypeRef& rhs) const;
    bool operator()(const TypeRef& lhs, const DexFileAndDescriptor& rhs) const;
  };

  HashMap<TypeRef, size_t, TypeRefEmpty, TypeRefHash, TypeRefEqual> map_;
};

}  // namespace art

#endif  // ART_COMPILER_DRIVER_IMAGE_CLASS_MAP_H_

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

#ifndef ART_COMPILER_DRIVER_IMAGE_CLASS_MAP_INL_H_
#define ART_COMPILER_DRIVER_IMAGE_CLASS_MAP_INL_H_

#include "image_class_map.h"

#include "dex/dex_file-inl.h"

namespace art HIDDEN {

inline size_t ImageClassMap::TypeRefHash::operator()(const TypeRef& type_ref) const {
  DCHECK(type_ref.dex_file != nullptr);
  return DataHash()(type_ref.dex_file->GetTypeDescriptorView(type_ref.type_index));
}

inline size_t ImageClassMap::TypeRefHash::operator()(
    const DexFileAndDescriptor& dex_file_and_descriptor) const {
  DCHECK(dex_file_and_descriptor.first != nullptr);
  return DataHash()(dex_file_and_descriptor.second);
}

inline bool ImageClassMap::TypeRefEqual::operator()(const TypeRef& lhs, const TypeRef& rhs) const {
  DCHECK(lhs.dex_file != nullptr);
  DCHECK(rhs.dex_file != nullptr);
  return lhs.dex_file == rhs.dex_file && lhs.type_index == rhs.type_index;
}

inline bool ImageClassMap::TypeRefEqual::operator()(const TypeRef& lhs,
                                                    const DexFileAndDescriptor& rhs) const {
  DCHECK(lhs.dex_file != nullptr);
  DCHECK(rhs.first != nullptr);
  return lhs.dex_file == rhs.first &&
         lhs.dex_file->GetTypeDescriptorView(lhs.type_index) == rhs.second;
}

inline void ImageClassMap::Add(TypeReference type_ref, size_t array_dim) {
  DCHECK(type_ref.dex_file != nullptr);
  auto [it, inserted] = map_.insert({TypeRef(type_ref), array_dim});
  if (!inserted && it->second < array_dim) {
    it->second = array_dim;
  }
}

inline void ImageClassMap::Remove(TypeReference type_ref) {
  DCHECK(type_ref.dex_file != nullptr);
  auto it = map_.find(TypeRef(type_ref));
  if (it != map_.end()) {
    map_.erase(it);
  }
}

inline bool ImageClassMap::Contains(TypeReference type_ref, size_t array_dim) const {
  DCHECK(type_ref.dex_file != nullptr);
  auto it = map_.find(TypeRef(type_ref));
  return it != map_.end() && it->second >= array_dim;
}

inline bool ImageClassMap::Contains(const DexFile* dex_file,
                                    std::string_view descriptor,
                                    size_t array_dim) const {
  DCHECK(dex_file != nullptr);
  DCHECK(!descriptor.starts_with('['));
  auto it = map_.find(DexFileAndDescriptor{dex_file, descriptor});
  return it != map_.end() && it->second >= array_dim;
}

template <typename Visitor>
inline void ImageClassMap::ForEach(Visitor&& visitor) const {
  for (const auto& entry : map_) {
    DCHECK(entry.first.dex_file != nullptr);
    visitor(TypeReference(entry.first.dex_file, entry.first.type_index), entry.second);
  }
}

}  // namespace art

#endif  // ART_COMPILER_DRIVER_IMAGE_CLASS_MAP_INL_H_

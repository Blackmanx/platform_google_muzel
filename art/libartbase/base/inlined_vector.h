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

#ifndef ART_LIBARTBASE_BASE_INLINED_VECTOR_H_
#define ART_LIBARTBASE_BASE_INLINED_VECTOR_H_

#include <vector>

#include "array_ref.h"
#include "logging.h"
#include "macros.h"

namespace art HIDDEN {

// Helper class for collecting values in a stack-allocated array of size `kMaxStackEntries`,
// or in heap-allocated `std::vector<>` storage if `kMaxStackEntries` is exceeded.
template <typename T, size_t kMaxStackEntries>
class InlinedVector {
 public:
  void push_back(const T& value) {
    if (LIKELY(size_ < kMaxStackEntries)) {
      stack_entries_[size_] = value;
    } else {
      if (size_ == kMaxStackEntries) {
        heap_entries_.reserve(2u * kMaxStackEntries);
        heap_entries_.assign(stack_entries_, stack_entries_ + kMaxStackEntries);
      }
      DCHECK_EQ(heap_entries_.size(), size_);
      heap_entries_.push_back(value);
    }
    ++size_;
  }

  size_t size() const {
    return size_;
  }

  ArrayRef<T const> GetArray() const {
    return {size_ <= kMaxStackEntries ? stack_entries_ : heap_entries_.data(), size_};
  }

 private:
  T stack_entries_[kMaxStackEntries];
  std::vector<T> heap_entries_;
  size_t size_;
};

}  // namespace art

#endif  // ART_LIBARTBASE_BASE_INLINED_VECTOR_H_

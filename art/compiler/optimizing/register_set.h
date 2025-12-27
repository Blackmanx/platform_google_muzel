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

#ifndef ART_COMPILER_OPTIMIZING_REGISTER_SET_H_
#define ART_COMPILER_OPTIMIZING_REGISTER_SET_H_

#include "base/logging.h"
#include "base/macros.h"
#include "base/bit_utils.h"
#include "base/value_object.h"
#include "physical_register_type.h"

namespace art HIDDEN {

class RegisterSet : public ValueObject {
 public:
  static RegisterSet Empty() { return RegisterSet(); }
  static RegisterSet AllFpu() { return RegisterSet(0u, ~0u, 0u); }

  void AddCoreRegisterSet(uint32_t registers) {
    core_register_set_ |= registers;
  }

  void AddFpuRegisterSet(uint32_t registers) {
    fpu_register_set_ |= registers;
  }

  void AddVecRegisterSet(uint32_t registers) {
    vec_register_set_ |= registers;
  }

  void AddRegisterSet(PhysicalRegisterType register_type, uint32_t registers) {
    (this->*GetRegisterSetAccessor(register_type)) |= registers;
  }

  void AddCoreRegister(uint32_t reg) {
    DCHECK_LT(reg, BitSizeOf<uint32_t>());
    core_register_set_ |= 1u << reg;
  }

  void AddFpuRegister(uint32_t reg) {
    DCHECK_LT(reg, BitSizeOf<uint32_t>());
    fpu_register_set_ |= 1u << reg;
  }

  void AddVecRegister(uint32_t reg) {
    DCHECK_LT(reg, BitSizeOf<uint32_t>());
    vec_register_set_ |= 1u << reg;
  }

  void AddRegister(PhysicalRegisterType register_type, uint32_t reg) {
    (this->*GetRegisterSetAccessor(register_type)) |= 1u << reg;
  }

  bool ContainsCoreRegister(uint32_t id) const {
    return Contains(core_register_set_, id);
  }

  bool ContainsFpuRegister(uint32_t id) const {
    return Contains(fpu_register_set_, id);
  }

  bool ContainsVecRegister(uint32_t id) const {
    return Contains(vec_register_set_, id);
  }

  static bool Contains(uint32_t register_set, uint32_t reg) {
    return (register_set & (1 << reg)) != 0;
  }

  size_t GetNumberOfRegisters() const {
    return POPCOUNT(core_register_set_) +
           POPCOUNT(fpu_register_set_) +
           POPCOUNT(vec_register_set_);
  }

  uint32_t GetCoreRegisterSet() const {
    return core_register_set_;
  }

  uint32_t GetFpuRegisterSet() const {
    return fpu_register_set_;
  }

  uint32_t GetVecRegisterSet() const {
    return vec_register_set_;
  }

  uint32_t GetRegisterSet(PhysicalRegisterType register_type) const {
    return this->*GetRegisterSetAccessor(register_type);
  }

  static uint32_t RegisterSet::* GetCoreRegisterSetAccessor() {
    return &RegisterSet::core_register_set_;
  }

  static uint32_t RegisterSet::* GetFpuRegisterSetAccessor() {
    return &RegisterSet::fpu_register_set_;
  }

  static uint32_t RegisterSet::* GetVecRegisterSetAccessor() {
    return &RegisterSet::vec_register_set_;
  }

  static uint32_t RegisterSet::* GetRegisterSetAccessor(PhysicalRegisterType register_type) {
    // Note: clang++ can optimize this to a linear expression before inlining this to other
    // functions. Other similar `switch` expressions may not be optimized that well, so it's
    // preferable to use this function in implementing `PhysicalRegisterType`-base operations.
    switch (register_type) {
      case PhysicalRegisterType::kCoreRegister:
        return GetCoreRegisterSetAccessor();
      case PhysicalRegisterType::kFpuRegister:
        return GetFpuRegisterSetAccessor();
      case PhysicalRegisterType::kVectorRegister:
        return GetVecRegisterSetAccessor();
    }
  }

  RegisterSet Union(const RegisterSet& other) const {
    return {core_register_set_ | other.core_register_set_,
            fpu_register_set_ | other.fpu_register_set_,
            vec_register_set_ | other.vec_register_set_};
  }

  RegisterSet Intersect(const RegisterSet& other) const {
    return {core_register_set_ & other.core_register_set_,
            fpu_register_set_ & other.fpu_register_set_,
            vec_register_set_ & other.vec_register_set_};
  }

  RegisterSet Subtract(const RegisterSet& other) const {
    return {core_register_set_ & ~other.core_register_set_,
            fpu_register_set_ & ~other.fpu_register_set_,
            vec_register_set_ & ~other.vec_register_set_};
  }

 private:
  RegisterSet() : core_register_set_(0u), fpu_register_set_(0u), vec_register_set_(0u) {}
  RegisterSet(uint32_t core, uint32_t fp, uint32_t vec)
      : core_register_set_(core), fpu_register_set_(fp), vec_register_set_(vec) {}

  uint32_t core_register_set_;
  uint32_t fpu_register_set_;
  uint32_t vec_register_set_;
};

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_REGISTER_SET_H_

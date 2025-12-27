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

#ifndef ART_SIMULATOR_INCLUDE_CODE_SIMULATOR_H_
#define ART_SIMULATOR_INCLUDE_CODE_SIMULATOR_H_

#include "arch/instruction_set.h"
#include "runtime.h"

namespace art {

class ArtMethod;
union JValue;
class Thread;

//
// Classes in this file model simulator executors - per thread entities with their own contexts:
// simulated stack, registers, etc.
//

// This class implements a basic simulator executor which is capable of executing sequences
// of simulated ISA instructions. It is not aware of ART runtime so it can manage only trivial
// ART methods.
//
// Currently only used by code generator tests.
class BasicCodeSimulator {
 public:
  BasicCodeSimulator() {}
  virtual ~BasicCodeSimulator() {}

  // Starts simulating instructions of a target isa from a buffer.
  virtual void RunFrom(intptr_t code_buffer) = 0;

  // Get return value according to C ABI.
  virtual bool GetCReturnBool() const = 0;
  virtual int32_t GetCReturnInt32() const = 0;
  virtual int64_t GetCReturnInt64() const = 0;

  // Can the target_isa be simulated on the current host ISA?
  static constexpr bool CanSimulate(InstructionSet target_isa) {
    // We currently support only 64-bit hosts as the simulator is designed around the fact
    // that host/target pointer size is the same.
    if constexpr (kRuntimeISA == InstructionSet::kX86_64) {
      switch (target_isa) {
        case InstructionSet::kArm64:
          return true;
        default:
          return false;
      }
    } else {
      return false;
    }
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(BasicCodeSimulator);
};

// libart(d)-simulator is only included as a dependency on device targets.
#ifndef ART_TARGET
// Create a basic code simulator object.
extern "C" BasicCodeSimulator* CreateBasicCodeSimulator(InstructionSet target_isa,
                                                        size_t stack_size);
#else
[[maybe_unused]] static BasicCodeSimulator* CreateBasicCodeSimulator(InstructionSet, size_t) {
  return nullptr;
}
#endif

#ifdef ART_USE_SIMULATOR

// This class implements an ART runtime aware simulator executor which can execute all quick ABI
// code: is aware of ART entrypoints, ABI/ISA transitions, etc.
class CodeSimulator {
 public:
  CodeSimulator() {}
  virtual ~CodeSimulator() {}

  // Invokes (starts to simulate) a method; this should follow the semantics of
  // art_quick_invoke_stub.
  virtual void Invoke(ArtMethod* method,
                      uint32_t* args,
                      uint32_t args_size,
                      Thread* self,
                      JValue* result,
                      const char* shorty,
                      bool isStatic) REQUIRES_SHARED(Locks::mutator_lock_) = 0;

  // Get the current stack pointer from the simulator.
  virtual int64_t GetStackPointer() = 0;
  // Get the stack base from the simulator.
  virtual uint8_t* GetStackBaseInternal() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(CodeSimulator);
};

// Create a runtime aware code simulator object.
extern "C" CodeSimulator* CreateCodeSimulator(InstructionSet target_isa, size_t stack_size);

#endif

}  // namespace art

#endif  // ART_SIMULATOR_INCLUDE_CODE_SIMULATOR_H_

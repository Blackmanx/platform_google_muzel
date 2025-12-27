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

#ifndef ART_COMPILER_OPTIMIZING_INSTRUCTION_SIMPLIFIER_SHARED_H_
#define ART_COMPILER_OPTIMIZING_INSTRUCTION_SIMPLIFIER_SHARED_H_

#include "base/macros.h"
#include "base/scoped_arena_containers.h"
#include "nodes.h"

namespace art HIDDEN {

class CodeGenerator;

namespace helpers {

inline bool CanFitInShifterOperand(HInstruction* instruction) {
  if (instruction->IsTypeConversion()) {
    HTypeConversion* conversion = instruction->AsTypeConversion();
    DataType::Type result_type = conversion->GetResultType();
    DataType::Type input_type = conversion->GetInputType();
    // We don't expect to see the same type as input and result.
    return DataType::IsIntegralType(result_type) && DataType::IsIntegralType(input_type) &&
        (result_type != input_type);
  } else {
    return (instruction->IsShl() && instruction->AsShl()->InputAt(1)->IsIntConstant()) ||
        (instruction->IsShr() && instruction->AsShr()->InputAt(1)->IsIntConstant()) ||
        (instruction->IsUShr() && instruction->AsUShr()->InputAt(1)->IsIntConstant());
  }
}

inline bool HasShifterOperand(HInstruction* instr, InstructionSet isa) {
  // On ARM64 `neg` instructions are an alias of `sub` using the zero register
  // as the first register input.
  bool res = instr->IsAdd() || instr->IsAnd() ||
      (isa == InstructionSet::kArm64 && instr->IsNeg()) ||
      instr->IsOr() || instr->IsSub() || instr->IsXor();
  return res;
}

// Check the specified sub is the last operation of the sequence:
//   t1 = Shl
//   t2 = Sub(t1, *)
//   t3 = Sub(*, t2)
inline bool IsSubRightSubLeftShl(HSub *sub) {
  HInstruction* right = sub->GetRight();
  return right->IsSub() && right->AsSub()->GetLeft()->IsShl();
}

// Helper class for mapping instructions to their inputs that should be merged to shifter operand.
// This class maintains its own `ScopedArenaAllocator` and should be wrapped in `std::optional` to
// avoid the construction/destruction overhead unless we actually find something to record.
class ShifterOperandMap {
 public:
  explicit ShifterOperandMap(ArenaStack* stack)
      : allocator_(stack),
        instruction_map_(allocator_.Adapter(kArenaAllocMisc)) {}

  void Add(HInstruction* use, HInstruction* bitfield_op) {
    DCHECK(!Contains(use));
    instruction_map_.Overwrite(use, bitfield_op);
  }

  bool Contains(HInstruction* use) const {
    return instruction_map_.find(use) != instruction_map_.end();
  }

  HInstruction* TryTakingBitFieldOp(HInstruction* use) {
    auto it = instruction_map_.find(use);
    if (it == instruction_map_.end()) {
      return nullptr;
    }
    HInstruction* bitfield_op = it->second;
    instruction_map_.erase(it);  // The caller is about to replace the `use`, so remove the entry.
    return bitfield_op;
  }

  bool IsEmpty() const {
    return instruction_map_.empty();
  }

 private:
  ScopedArenaAllocator allocator_;
  // Maps the shift/extend instruction's use to the shift/extend instruction to merge.
  ScopedArenaHashMap<HInstruction*, HInstruction*> instruction_map_;
};

// Is an instruction before another one in reverse post order?
//
// Used to determine if an instruction was already visited, os shall be visited later
// during reverse post order visit. Not applicable to Phis.
bool IsBeforeInReversePostOrder(HGraph* graph, HInstruction* lhs, HInstruction* rhs);

}  // namespace helpers

bool TrySimpleMultiplyAccumulatePatterns(HMul* mul, InstructionSet isa);
bool TryCombineMultiplyAccumulate(HInstruction* use, HMul* mul, InstructionSet isa);

bool TryExtractArrayAccessAddress(CodeGenerator* codegen,
                                  HInstruction* access,
                                  HInstruction* array,
                                  HInstruction* index,
                                  size_t data_offset);

bool TryExtractVecArrayAccessAddress(HVecMemoryOperation* access, HInstruction* index);

// Try to replace
//   Sub(c, Sub(a, b))
// with
//   Add(c, Sub(b, a))
bool TryReplaceSubSubWithSubAdd(HSub* last_sub);

// ARM does not contain instruction ROL so replace
//   ROL dest, a, distance
// with
//   NEG neg, distance
//   ROR dest, a, neg
// before GVN to give it a chance to deduplicate the instructions, if it's able.
void UnfoldRotateLeft(HRol* rol);

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_INSTRUCTION_SIMPLIFIER_SHARED_H_

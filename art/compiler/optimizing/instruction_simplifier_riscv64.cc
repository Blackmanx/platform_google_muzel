/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "instruction_simplifier_riscv64.h"

#include "instruction_simplifier.h"
#include "nodes.h"

namespace art HIDDEN {

namespace riscv64 {

class InstructionSimplifierRiscv64Visitor final
    : public CRTPGraphVisitor<InstructionSimplifierRiscv64Visitor> {
 public:
  InstructionSimplifierRiscv64Visitor(HGraph* graph, OptimizingCompilerStats* stats)
      : CRTPGraphVisitor(graph), stats_(stats) {}

 private:
  void RecordSimplification() {
    MaybeRecordStat(stats_, MethodCompilationStat::kInstructionSimplificationsArch);
  }

  // Replace Add which has Shl with distance of 1 or 2 or 3 with Riscv64ShiftAdd
  bool TryReplaceAddShlWithShiftAdd(HAdd* add, HShl* shl, HInstruction* add_other_input) {
    // There is no reason to replace Int32 Shl+Add with ShiftAdd because of
    // additional sign-extension required.
    if (shl->GetType() != DataType::Type::kInt64) {
      return false;
    }

    DCHECK_EQ(add->GetType(), DataType::Type::kInt64)
        << "Add must be the same 64 bit type as the input";

    // The bytecode does not permit the shift distance to come from a wide variable.
    DCHECK_EQ(shl->GetRight()->IsConstant(), shl->GetRight()->IsIntConstant());
    if (!shl->GetRight()->IsIntConstant()) {
      return false;
    }

    const int32_t distance = shl->GetRight()->AsIntConstant()->GetValue();
    if (distance < 1 || distance > 3) {
      return false;
    }

    // If the HAdd to replace has both inputs the same HShl<1|2|3>, then
    // don't perform the optimization. The processor will not be able to execute
    // these shifts parallel which is the purpose of the replace below.
    if (shl == add_other_input) {
      return false;
    }

    HRiscv64ShiftAdd* shift_add = new (GetGraph()->GetAllocator())
        HRiscv64ShiftAdd(shl->GetLeft(), add_other_input, distance);

    add->GetBlock()->ReplaceAndRemoveInstructionWith(add, shift_add);

    if (!shl->HasUses()) {
      shl->GetBlock()->RemoveInstruction(shl);
    }

    return true;
  }

  // Keep `ForwardVisit()` functions from base class visible except for those we replace below.
  using CRTPGraphVisitor::ForwardVisit;

  // Forward `Or` and `Xor` to `HandleOrXor()`.
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HOr*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitOr);
    return &InstructionSimplifierRiscv64Visitor::HandleOrXor;
  }
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HXor*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitXor);
    return &InstructionSimplifierRiscv64Visitor::HandleOrXor;
  }

  void HandleOrXor(HBinaryOperation* bitwise_op) {
    DCHECK(bitwise_op->IsOr() || bitwise_op->IsXor());
    if (TryMergeNegatedInput(bitwise_op)) {
      RecordSimplification();
    } else if (TryOptimizeWithBitManipulation(bitwise_op)) {
      RecordSimplification();
    }
  }

  // Replace code looking like
  //    SHL tmp, a, 1 or 2 or 3
  //    ADD dst, tmp, b
  // with
  //    Riscv64ShiftAdd dst, a, b
  void VisitAdd(HAdd* add) {
    HInstruction* left = add->GetLeft();
    HInstruction* right = add->GetRight();
    if ((left->IsShl() && TryReplaceAddShlWithShiftAdd(add, left->AsShl(), right)) ||
        (right->IsShl() && TryReplaceAddShlWithShiftAdd(add, right->AsShl(), left))) {
      RecordSimplification();
    }
  }

  bool TryReplaceShrAndWithBitExtract(HAnd* op) {
    HInstruction* left = op->GetLeft();
    HInstruction* right = op->GetRight();
    DCHECK_EQ(op->GetType(), DataType::Type::kInt64);

    HInstruction* op_value = (!left->IsShr()) ? left : right;
    if (!op_value->IsConstant()) {
      return false;
    }

    if (!op_value->AsConstant()->IsOne()) {
      return false;
    }

    HShr* shr = (left->IsShr()) ? left->AsShr() : right->AsShr();

    HInstruction* shamt = shr->GetRight();
    HInstruction* shr_op = shr->GetLeft();

    HRiscv64BitExtract* bit_extract =
        new (GetGraph()->GetAllocator()) HRiscv64BitExtract(shr_op, shamt);
    op->GetBlock()->ReplaceAndRemoveInstructionWith(op, bit_extract);

    if (!shr->HasUses()) {
      shr->GetBlock()->RemoveInstruction(shr);
    }

    return true;
  }

  bool TryOptimizeBinaryOpWithConstWithBitManipulation(HBinaryOperation* op) {
    DCHECK(op->IsOr() || op->IsXor() || op->IsAnd());
    HInstruction* left = op->GetLeft();
    HInstruction* right = op->GetRight();
    DCHECK(op->GetType() == DataType::Type::kInt64);

    HInstruction* op_value;
    uint64_t imm;
    if (left->IsConstant()) {
      imm = left->AsConstant()->GetValueAsUint64();
      op_value = right;
    } else {
      imm = right->AsConstant()->GetValueAsUint64();
      op_value = left;
    }

    if (op->IsAnd()) {
      // We expect this pattern in case of And operation so we need to invert
      // the constant value to get the bit mask
      //    SHL tmp, 1, b
      //    NOT tmp, tmp
      //    AND dst, a, tmp
      //
      // For example, to clear bit 3, we need to have
      // AND with 0xFFFFFFF7, which is ~0x8.
      imm = ~imm;
    }

    if (imm == 0 || !IsPowerOfTwo(imm)) {
      return false;
    }

    int32_t power = WhichPowerOf2(imm);
    HConstant* shamt = GetGraph()->GetConstant(DataType::Type::kInt32, power);
    ArenaAllocator* allocator = GetGraph()->GetAllocator();
    HInstruction* replacement;

    if (op->IsOr()) {
      replacement = new (allocator) HRiscv64BitSet(op_value, shamt);
    } else if (op->IsXor()) {
      replacement = new (allocator) HRiscv64BitInvert(op_value, shamt);
    } else {
      DCHECK(op->IsAnd());
      replacement = new (allocator) HRiscv64BitClear(op_value, shamt);
    }
    op->GetBlock()->ReplaceAndRemoveInstructionWith(op, replacement);

    return true;
  }

  // Replace code looking like
  //    SHR tmp, a, b
  //    AND dst, 1, tmp
  // with
  //    Riscv64BitExtract dst, a, b
  // Replace code looking like
  //    SHL tmp, 1, b
  //    OR dst, a, tmp
  // with
  //    Riscv64BitSet dst, a, b
  // Replace code looking like
  //    SHL tmp, 1, b
  //    XOR dst, a, tmp
  // with
  //    Riscv64BitInvert dst, a, b
  // Replace code looking like
  //    SHL tmp, 1, b
  //    NOT tmp, tmp
  //    AND dst, a, tmp
  // with
  //    Riscv64BitClear dst, a, b

  bool TryOptimizeWithBitManipulation(HBinaryOperation* op) {
    DCHECK(op->IsAnd() || op->IsOr() || op->IsXor());

    if (!op->HasUses()) {
      DCHECK(op->IsDeadAndRemovable());
      op->GetBlock()->RemoveInstruction(op);
      return false;
    }

    DataType::Type result_type = op->GetType();
    if (result_type != DataType::Type::kInt64) {
      return false;
    }

    HInstruction* left = op->GetLeft();
    HInstruction* right = op->GetRight();

    if (left->IsShr() || right->IsShr()) {
      return op->IsAnd() ? TryReplaceShrAndWithBitExtract(op->AsAnd()) : false;
    }

    if (left->IsConstant() || right->IsConstant()) {
      return TryOptimizeBinaryOpWithConstWithBitManipulation(op);
    }

    HShl* shl;
    HNot* hnot = nullptr;
    HInstruction* op_value;

    if (left->IsNot() || right->IsNot()) {
      hnot = (left->IsNot()) ? left->AsNot() : right->AsNot();
      if (!hnot->GetInput()->IsShl()) {
        return false;
      }
      shl = hnot->GetInput()->AsShl();
      op_value = (!left->IsNot()) ? left : right;
    } else {
      if (!left->IsShl() && !right->IsShl()) {
        return false;
      }
      if (left->IsShl()) {
        shl = left->AsShl();
        op_value = right;
      } else {
        shl = right->AsShl();
        op_value = left;
      }
    }

    HInstruction* shl_op = shl->GetLeft();
    if (!shl_op->IsConstant()) {
      return false;
    } else if (!shl_op->AsConstant()->IsOne()) {
      return false;
    }

    HInstruction* shamt = shl->GetRight();
    ArenaAllocator* allocator = GetGraph()->GetAllocator();
    HInstruction* replacement;

    if (op->IsAnd() != (hnot != nullptr)) {
      return false;
    }

    if (op->IsOr()) {
      replacement = new (allocator) HRiscv64BitSet(op_value, shamt);
    } else if (op->IsXor()) {
      replacement = new (allocator) HRiscv64BitInvert(op_value, shamt);
    } else {
      DCHECK(op->IsAnd());
      replacement = new (allocator) HRiscv64BitClear(op_value, shamt);
    }

    op->GetBlock()->ReplaceAndRemoveInstructionWith(op, replacement);

    if (hnot != nullptr && !hnot->HasUses()) {
      hnot->GetBlock()->RemoveInstruction(hnot);
    }

    if (!shl->HasUses()) {
      shl->GetBlock()->RemoveInstruction(shl);
    }

    return true;
  }

  void VisitAnd(HAnd* inst) {
    if (TryOptimizeWithBitManipulation(inst)) {
      RecordSimplification();
    } else if (TryMergeNegatedInput(inst)) {
      RecordSimplification();
    }
  }

  void VisitSub(HSub* inst) {
    if (TryMergeWithAnd(inst)) {
      RecordSimplification();
    }
  }

  OptimizingCompilerStats* stats_ = nullptr;

  template <typename T> friend class art::CRTPGraphVisitor;
};

bool InstructionSimplifierRiscv64::Run() {
  auto visitor = InstructionSimplifierRiscv64Visitor(graph_, stats_);
  visitor.VisitReversePostOrder();
  return true;
}

}  // namespace riscv64
}  // namespace art

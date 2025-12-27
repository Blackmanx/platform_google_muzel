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

#include "instruction_simplifier_arm.h"

#include "code_generator.h"
#include "common_arm.h"
#include "instruction_simplifier.h"
#include "instruction_simplifier_shared.h"
#include "mirror/array-inl.h"
#include "mirror/string.h"
#include "nodes.h"

namespace art HIDDEN {

using helpers::CanFitInShifterOperand;
using helpers::HasShifterOperand;
using helpers::IsBeforeInReversePostOrder;
using helpers::IsSubRightSubLeftShl;
using helpers::ShifterOperandMap;

namespace arm {

class InstructionSimplifierArmVisitor final
    : public CRTPGraphVisitor<InstructionSimplifierArmVisitor> {
 public:
  InstructionSimplifierArmVisitor(
      HGraph* graph, CodeGenerator* codegen, OptimizingCompilerStats* stats)
      : CRTPGraphVisitor(graph),
        codegen_(codegen),
        stats_(stats),
        shifter_operand_map_(std::nullopt) {}

  ~InstructionSimplifierArmVisitor() {
    DCHECK_IMPLIES(shifter_operand_map_.has_value(), shifter_operand_map_->IsEmpty());
  }

 private:
  void RecordSimplification() {
    MaybeRecordStat(stats_, MethodCompilationStat::kInstructionSimplificationsArch);
  }

  bool TryMarkingShifterOperand(HInstruction* bitfield_op);
  bool TryMergingShifterOperand(HInstruction* use);
  bool TryMergeIntoShifterOperand(HInstruction* use, HInstruction* bitfield_op, bool do_merge);
  bool CanMergeIntoShifterOperand(HInstruction* use, HInstruction* bitfield_op) {
    return TryMergeIntoShifterOperand(use, bitfield_op, /* do_merge= */ false);
  }
  bool MergeIntoShifterOperand(HInstruction* use, HInstruction* bitfield_op) {
    DCHECK(CanMergeIntoShifterOperand(use, bitfield_op));
    return TryMergeIntoShifterOperand(use, bitfield_op, /* do_merge= */ true);
  }

  bool TryMultiplyAccumulateSimplification(HInstruction* use, HInstruction* maybe_mul) {
    return maybe_mul->IsMul() &&
           maybe_mul->HasOnlyOneNonEnvironmentUse() &&
           TryCombineMultiplyAccumulate(use, maybe_mul->AsMul(), InstructionSet::kArm);
  }

  // Keep `ForwardVisit()` functions from base class visible except for those we replace below.
  using CRTPGraphVisitor::ForwardVisit;

  // Forward `Shl`, `Shr` and `UShr` to `HandleShiftForShifterOperand()`.
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HShl*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitShl);
    return &InstructionSimplifierArmVisitor::HandleShiftForShifterOperand;
  }
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HShr*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitShr);
    return &InstructionSimplifierArmVisitor::HandleShiftForShifterOperand;
  }
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HUShr*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitUShr);
    return &InstructionSimplifierArmVisitor::HandleShiftForShifterOperand;
  }

  // Forward `And` and `Or` to `HandleAndOr()`.
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HAnd*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitAnd);
    return &InstructionSimplifierArmVisitor::HandleAndOr;
  }
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HOr*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitOr);
    return &InstructionSimplifierArmVisitor::HandleAndOr;
  }

  void HandleShiftForShifterOperand(HBinaryOperation* shift);
  void HandleAndOr(HBinaryOperation* bitwise_op);

  void VisitAdd(HAdd* instruction);
  void VisitArrayGet(HArrayGet* instruction);
  void VisitArraySet(HArraySet* instruction);
  void VisitMul(HMul* instruction);
  void VisitRol(HRol* instruction);
  void VisitSub(HSub* instruction);
  void VisitTypeConversion(HTypeConversion* instruction);
  void VisitXor(HXor* instruction);

  CodeGenerator* codegen_;
  OptimizingCompilerStats* stats_;
  std::optional<ShifterOperandMap> shifter_operand_map_;

  template <typename T> friend class art::CRTPGraphVisitor;
};

bool InstructionSimplifierArmVisitor::TryMergeIntoShifterOperand(HInstruction* use,
                                                                 HInstruction* bitfield_op,
                                                                 bool do_merge) {
  DCHECK(HasShifterOperand(use, InstructionSet::kArm));
  DCHECK(use->IsBinaryOperation());
  DCHECK(CanFitInShifterOperand(bitfield_op));
  DCHECK(!bitfield_op->HasEnvironmentUses());

  DataType::Type type = use->GetType();
  if (type != DataType::Type::kInt32 && type != DataType::Type::kInt64) {
    return false;
  }

  HInstruction* left = use->InputAt(0);
  HInstruction* right = use->InputAt(1);
  DCHECK(left == bitfield_op || right == bitfield_op);

  if (left == right) {
    // TODO: Handle special transformations in this situation?
    // For example should we transform `(x << 1) + (x << 1)` into `(x << 2)`?
    // Or should this be part of a separate transformation logic?
    return false;
  }

  bool is_commutative = use->AsBinaryOperation()->IsCommutative();
  HInstruction* other_input;
  if (bitfield_op == right) {
    other_input = left;
  } else {
    if (is_commutative) {
      other_input = right;
    } else {
      return false;
    }
  }

  HDataProcWithShifterOp::OpKind op_kind;
  int shift_amount = 0;

  HDataProcWithShifterOp::GetOpInfoFromInstruction(bitfield_op, &op_kind, &shift_amount);
  shift_amount &= use->GetType() == DataType::Type::kInt32
      ? kMaxIntShiftDistance
      : kMaxLongShiftDistance;

  if (HDataProcWithShifterOp::IsExtensionOp(op_kind)) {
    if (!use->IsAdd() && (!use->IsSub() || use->GetType() != DataType::Type::kInt64)) {
      return false;
    }
  // Shift by 1 is a special case that results in the same number and type of instructions
  // as this simplification, but potentially shorter code.
  } else if (type == DataType::Type::kInt64 && shift_amount == 1) {
    return false;
  }

  if (do_merge) {
    HDataProcWithShifterOp* alu_with_op =
        new (GetGraph()->GetAllocator()) HDataProcWithShifterOp(use,
                                                                other_input,
                                                                bitfield_op->InputAt(0),
                                                                op_kind,
                                                                shift_amount,
                                                                use->GetDexPc());
    use->GetBlock()->ReplaceAndRemoveInstructionWith(use, alu_with_op);
    if (bitfield_op->GetUses().empty()) {
      bitfield_op->GetBlock()->RemoveInstruction(bitfield_op);
    }
  }

  return true;
}

// Mark a bitfield move instruction for merging into its uses if it can be merged in all of them.
bool InstructionSimplifierArmVisitor::TryMarkingShifterOperand(HInstruction* bitfield_op) {
  DCHECK(CanFitInShifterOperand(bitfield_op));

  if (bitfield_op->HasEnvironmentUses()) {
    return false;
  }

  const HUseList<HInstruction*>& uses = bitfield_op->GetUses();

  // Check whether we can merge the instruction in all its users' shifter operand.
  for (const HUseListNode<HInstruction*>& use : uses) {
    HInstruction* user = use.GetUser();
    if (!HasShifterOperand(user, InstructionSet::kArm)) {
      return false;
    }
    if (!CanMergeIntoShifterOperand(user, bitfield_op)) {
      return false;
    }
    if (shifter_operand_map_.has_value() && shifter_operand_map_->Contains(user)) {
      return false;  // The user shall already have shifter operand merged.
    }
  }

  // Mark the instruction for merging into its uses. The merging is done when we visit those uses.
  if (!shifter_operand_map_.has_value()) {
    shifter_operand_map_.emplace(GetGraph()->GetArenaStack());
  }
  for (const HUseListNode<HInstruction*>& use : uses) {
    shifter_operand_map_->Add(use.GetUser(), bitfield_op);
  }
  return true;
}

bool InstructionSimplifierArmVisitor::TryMergingShifterOperand(HInstruction* user) {
  if (!shifter_operand_map_.has_value()) {
    return false;
  }
  HInstruction* bitfield_op = shifter_operand_map_->TryTakingBitFieldOp(user);
  if (bitfield_op == nullptr) {
    return false;
  }
  bool merged = MergeIntoShifterOperand(user, bitfield_op);
  DCHECK(merged);
  return true;
}

void InstructionSimplifierArmVisitor::HandleShiftForShifterOperand(HBinaryOperation* shift) {
  DCHECK(shift->IsShl() || shift->IsShr() || shift->IsUShr());
  DCHECK_EQ(shift->GetRight()->IsConstant(), shift->GetRight()->IsIntConstant());
  if (shift->GetRight()->IsIntConstant()) {
    TryMarkingShifterOperand(shift);
  }
}

void InstructionSimplifierArmVisitor::HandleAndOr(HBinaryOperation* bitwise_op) {
  DCHECK(bitwise_op->IsAnd() || bitwise_op->IsOr());
  if (TryMergingShifterOperand(bitwise_op) ||
      TryMergeNegatedInput(bitwise_op)) {
    RecordSimplification();
  }
}

void InstructionSimplifierArmVisitor::VisitAdd(HAdd* instruction) {
  if (TryMergingShifterOperand(instruction)  ||
      TryMultiplyAccumulateSimplification(instruction, instruction->GetLeft()) ||
      TryMultiplyAccumulateSimplification(instruction, instruction->GetRight())) {
    RecordSimplification();
  }
}

void InstructionSimplifierArmVisitor::VisitArrayGet(HArrayGet* instruction) {
  size_t data_offset = CodeGenerator::GetArrayDataOffset(instruction);
  DataType::Type type = instruction->GetType();

  // TODO: Implement reading (length + compression) for String compression feature from
  // negative offset (count_offset - data_offset). Thumb2Assembler (now removed) did
  // not support T4 encoding of "LDR (immediate)", but ArmVIXLMacroAssembler might.
  // Don't move array pointer if it is charAt because we need to take the count first.
  if (mirror::kUseStringCompression && instruction->IsStringCharAt()) {
    return;
  }

  // TODO: Support intermediate address for object arrays on arm.
  if (type == DataType::Type::kReference) {
    return;
  }

  if (type == DataType::Type::kInt64
      || type == DataType::Type::kFloat32
      || type == DataType::Type::kFloat64) {
    // T32 doesn't support ShiftedRegOffset mem address mode for these types
    // to enable optimization.
    return;
  }

  if (TryExtractArrayAccessAddress(codegen_,
                                   instruction,
                                   instruction->GetArray(),
                                   instruction->GetIndex(),
                                   data_offset)) {
    RecordSimplification();
  }
}

void InstructionSimplifierArmVisitor::VisitArraySet(HArraySet* instruction) {
  size_t access_size = DataType::Size(instruction->GetComponentType());
  size_t data_offset = mirror::Array::DataOffset(access_size).Uint32Value();
  DataType::Type type = instruction->GetComponentType();

  if (type == DataType::Type::kInt64
      || type == DataType::Type::kFloat32
      || type == DataType::Type::kFloat64) {
    // T32 doesn't support ShiftedRegOffset mem address mode for these types
    // to enable optimization.
    return;
  }

  if (TryExtractArrayAccessAddress(codegen_,
                                   instruction,
                                   instruction->GetArray(),
                                   instruction->GetIndex(),
                                   data_offset)) {
    RecordSimplification();
  }
}

void InstructionSimplifierArmVisitor::VisitMul(HMul* instruction) {
  if (instruction->HasOnlyOneNonEnvironmentUse()) {
    HInstruction* use = instruction->GetUses().front().GetUser();
    if (use->IsAdd() || (use->IsSub() && instruction == use->AsSub()->GetRight())) {
      // Shall be simplified when visiting the `use` unless the `use` is simplified in another way.
      return;
    }
  }
  if (TrySimpleMultiplyAccumulatePatterns(instruction, InstructionSet::kArm)) {
    RecordSimplification();
  }
}

void InstructionSimplifierArmVisitor::VisitRol(HRol* instruction) {
  UnfoldRotateLeft(instruction);
  RecordSimplification();
}

void InstructionSimplifierArmVisitor::VisitSub(HSub* instruction) {
  if (TryMergingShifterOperand(instruction) ||
      TryMultiplyAccumulateSimplification(instruction, instruction->GetRight())) {
    RecordSimplification();
    return;
  }
  if (IsSubRightSubLeftShl(instruction)) {
    HSub* right_sub = instruction->GetRight()->AsSub();
    HInstruction* shl = right_sub->InputAt(0);
    if (shl->InputAt(1)->IsConstant() && TryReplaceSubSubWithSubAdd(instruction)) {
      DCHECK(!instruction->IsInBlock());
      DCHECK(shl->IsInBlock());
      DCHECK(right_sub->IsInBlock());
      DCHECK(right_sub->HasOnlyOneNonEnvironmentUse());
      if (TryMarkingShifterOperand(shl)) {
        HInstruction* replacement = right_sub->GetUses().front().GetUser();
        bool success = TryMergingShifterOperand(right_sub);
        DCHECK(success);
        for (auto it = shl->GetUses().begin(), end = shl->GetUses().end(); it != end; ) {
          HInstruction* use = it->GetUser();
          ++it;  // Move to next use early as the current use node can be removed below.
          if (IsBeforeInReversePostOrder(GetGraph(), use, replacement)) {
            // This use shall not be visited again, so make the replacement now.
            success = TryMergingShifterOperand(use);
            DCHECK(success);
          }
        }
        return;
      }
    }
  }

  if (TryMergeWithAnd(instruction)) {
    return;
  }
}

void InstructionSimplifierArmVisitor::VisitTypeConversion(HTypeConversion* instruction) {
  DataType::Type result_type = instruction->GetResultType();
  DataType::Type input_type = instruction->GetInputType();

  if (input_type == result_type) {
    // We let the arch-independent code handle this.
    return;
  }

  if (DataType::IsIntegralType(result_type) && DataType::IsIntegralType(input_type)) {
    TryMarkingShifterOperand(instruction);
  }
}

void InstructionSimplifierArmVisitor::VisitXor(HXor* instruction) {
  if (TryMergingShifterOperand(instruction)) {
    RecordSimplification();
  }
}

bool InstructionSimplifierArm::Run() {
  InstructionSimplifierArmVisitor visitor(graph_, codegen_, stats_);
  visitor.VisitReversePostOrder();
  return true;
}

}  // namespace arm
}  // namespace art

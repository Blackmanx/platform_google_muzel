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

#include "instruction_simplifier_arm64.h"

#include "common_arm64.h"
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

namespace arm64 {

using helpers::ShifterOperandSupportsExtension;

class InstructionSimplifierArm64Visitor final
    : public CRTPGraphVisitor<InstructionSimplifierArm64Visitor> {
 public:
  InstructionSimplifierArm64Visitor(
      HGraph* graph, CodeGenerator* codegen, OptimizingCompilerStats* stats)
      : CRTPGraphVisitor(graph),
        codegen_(codegen),
        stats_(stats),
        shifter_operand_map_(std::nullopt) {}

  ~InstructionSimplifierArm64Visitor() {
    DCHECK_IMPLIES(shifter_operand_map_.has_value(), shifter_operand_map_->IsEmpty());
  }

 private:
  void RecordSimplification() {
    MaybeRecordStat(stats_, MethodCompilationStat::kInstructionSimplificationsArch);
  }

  bool TryMarkingShifterOperand(HInstruction* bitfield_op);
  bool TryMergingShifterOperand(HInstruction* use);
  bool TryMergeIntoShifterOperand(HInstruction* use,
                                  HInstruction* bitfield_op,
                                  bool do_merge);
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
           TryCombineMultiplyAccumulate(use, maybe_mul->AsMul(), InstructionSet::kArm64);
  }

  // Keep `ForwardVisit()` functions from base class visible except for those we replace below.
  using CRTPGraphVisitor::ForwardVisit;

  // Forward `Shl`, `Shr` and `UShr` to `HandleShiftForShifterOperand()`.
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HShl*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitShl);
    return &InstructionSimplifierArm64Visitor::HandleShiftForShifterOperand;
  }
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HShr*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitShr);
    return &InstructionSimplifierArm64Visitor::HandleShiftForShifterOperand;
  }
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HUShr*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitUShr);
    return &InstructionSimplifierArm64Visitor::HandleShiftForShifterOperand;
  }

  // Forward `And`, `Or` and `Xor` to `HandleBitwiseOp()`.
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HAnd*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitAnd);
    return &InstructionSimplifierArm64Visitor::HandleBitwiseOp;
  }
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HOr*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitOr);
    return &InstructionSimplifierArm64Visitor::HandleBitwiseOp;
  }
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HXor*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitXor);
    return &InstructionSimplifierArm64Visitor::HandleBitwiseOp;
  }

  void HandleShiftForShifterOperand(HBinaryOperation* shift);
  void HandleBitwiseOp(HBinaryOperation* bitwise_op);

  // HInstruction visitors, sorted alphabetically.
  void VisitAdd(HAdd* instruction);
  void VisitArrayGet(HArrayGet* instruction);
  void VisitArraySet(HArraySet* instruction);
  void VisitMul(HMul* instruction);
  void VisitNeg(HNeg* instruction);
  void VisitRol(HRol* instruction);
  void VisitSub(HSub* instruction);
  void VisitTypeConversion(HTypeConversion* instruction);
  void VisitVecLoad(HVecLoad* instruction);
  void VisitVecStore(HVecStore* instruction);
  void VisitVecAdd(HVecAdd* instruction);
  void VisitVecSub(HVecSub* instruction);

  bool TryCombineVecMultiplyAccumulate(HVecMul* mul, HVecBinaryOperation* binop);

  CodeGenerator* codegen_;
  OptimizingCompilerStats* stats_;
  std::optional<ShifterOperandMap> shifter_operand_map_;

  template <typename T> friend class art::CRTPGraphVisitor;
};

bool InstructionSimplifierArm64Visitor::TryMergeIntoShifterOperand(HInstruction* use,
                                                                   HInstruction* bitfield_op,
                                                                   bool do_merge) {
  DCHECK(HasShifterOperand(use, InstructionSet::kArm64));
  DCHECK(use->IsBinaryOperation() || use->IsNeg());
  DCHECK(CanFitInShifterOperand(bitfield_op));
  DCHECK(!bitfield_op->HasEnvironmentUses());

  DataType::Type type = use->GetType();
  if (type != DataType::Type::kInt32 && type != DataType::Type::kInt64) {
    return false;
  }

  HInstruction* left;
  HInstruction* right;
  if (use->IsBinaryOperation()) {
    left = use->InputAt(0);
    right = use->InputAt(1);
  } else {
    DCHECK(use->IsNeg());
    right = use->AsNeg()->InputAt(0);
    left = GetGraph()->GetConstant(right->GetType(), 0);
  }
  DCHECK(left == bitfield_op || right == bitfield_op);

  if (left == right) {
    // TODO: Handle special transformations in this situation?
    // For example should we transform `(x << 1) + (x << 1)` into `(x << 2)`?
    // Or should this be part of a separate transformation logic?
    return false;
  }

  bool is_commutative = use->IsBinaryOperation() && use->AsBinaryOperation()->IsCommutative();
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

  if (HDataProcWithShifterOp::IsExtensionOp(op_kind) && !ShifterOperandSupportsExtension(use)) {
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
bool InstructionSimplifierArm64Visitor::TryMarkingShifterOperand(HInstruction* bitfield_op) {
  DCHECK(CanFitInShifterOperand(bitfield_op));

  if (bitfield_op->HasEnvironmentUses()) {
    return false;
  }

  const HUseList<HInstruction*>& uses = bitfield_op->GetUses();

  // Check whether we can merge the instruction in all its users' shifter operand.
  for (const HUseListNode<HInstruction*>& use : uses) {
    HInstruction* user = use.GetUser();
    if (!HasShifterOperand(user, InstructionSet::kArm64)) {
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

bool InstructionSimplifierArm64Visitor::TryMergingShifterOperand(HInstruction* user) {
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

void InstructionSimplifierArm64Visitor::HandleShiftForShifterOperand(HBinaryOperation* shift) {
  DCHECK(shift->IsShl() || shift->IsShr() || shift->IsUShr());
  DCHECK_EQ(shift->GetRight()->IsConstant(), shift->GetRight()->IsIntConstant());
  if (shift->GetRight()->IsIntConstant()) {
    TryMarkingShifterOperand(shift);
  }
}

void InstructionSimplifierArm64Visitor::HandleBitwiseOp(HBinaryOperation* bitwise_op) {
  DCHECK(bitwise_op->IsAnd() || bitwise_op->IsOr() || bitwise_op->IsXor());
  if (TryMergingShifterOperand(bitwise_op) ||
      TryMergeNegatedInput(bitwise_op)) {
    RecordSimplification();
  }
}

void InstructionSimplifierArm64Visitor::VisitAdd(HAdd* instruction) {
  if (TryMergingShifterOperand(instruction) ||
      TryMultiplyAccumulateSimplification(instruction, instruction->GetLeft()) ||
      TryMultiplyAccumulateSimplification(instruction, instruction->GetRight())) {
    RecordSimplification();
  }
}

void InstructionSimplifierArm64Visitor::VisitArrayGet(HArrayGet* instruction) {
  size_t data_offset = CodeGenerator::GetArrayDataOffset(instruction);
  if (TryExtractArrayAccessAddress(codegen_,
                                   instruction,
                                   instruction->GetArray(),
                                   instruction->GetIndex(),
                                   data_offset)) {
    RecordSimplification();
  }
}

void InstructionSimplifierArm64Visitor::VisitArraySet(HArraySet* instruction) {
  size_t access_size = DataType::Size(instruction->GetComponentType());
  size_t data_offset = mirror::Array::DataOffset(access_size).Uint32Value();
  if (TryExtractArrayAccessAddress(codegen_,
                                   instruction,
                                   instruction->GetArray(),
                                   instruction->GetIndex(),
                                   data_offset)) {
    RecordSimplification();
  }
}

void InstructionSimplifierArm64Visitor::VisitMul(HMul* instruction) {
  if (instruction->HasOnlyOneNonEnvironmentUse()) {
    HInstruction* use = instruction->GetUses().front().GetUser();
    if (use->IsAdd() || use->IsNeg() || (use->IsSub() && instruction == use->AsSub()->GetRight())) {
      // Shall be simplified when visiting the user (unless the user is simplified in another way).
      return;
    }
  }
  if (TrySimpleMultiplyAccumulatePatterns(instruction, InstructionSet::kArm64)) {
    RecordSimplification();
  }
}

void InstructionSimplifierArm64Visitor::VisitNeg(HNeg* instruction) {
  if (TryMergingShifterOperand(instruction) ||
      TryMultiplyAccumulateSimplification(instruction, instruction->GetInput())) {
    RecordSimplification();
  }
}

void InstructionSimplifierArm64Visitor::VisitRol(HRol* rol) {
  UnfoldRotateLeft(rol);
  RecordSimplification();
}

void InstructionSimplifierArm64Visitor::VisitSub(HSub* instruction) {
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

void InstructionSimplifierArm64Visitor::VisitTypeConversion(HTypeConversion* instruction) {
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

void InstructionSimplifierArm64Visitor::VisitVecLoad(HVecLoad* instruction) {
  if (!instruction->IsPredicated() && !instruction->IsStringCharAt() &&
      TryExtractVecArrayAccessAddress(instruction, instruction->GetIndex())) {
    RecordSimplification();
  } else if (instruction->IsPredicated()) {
    size_t size = DataType::Size(instruction->GetPackedType());
    size_t offset = mirror::Array::DataOffset(size).Uint32Value();
    if (TryExtractArrayAccessAddress(
            codegen_, instruction, instruction->GetArray(), instruction->GetIndex(), offset)) {
      RecordSimplification();
    }
  }
}

void InstructionSimplifierArm64Visitor::VisitVecStore(HVecStore* instruction) {
  if (!instruction->IsPredicated() &&
      TryExtractVecArrayAccessAddress(instruction, instruction->GetIndex())) {
    RecordSimplification();
  } else if (instruction->IsPredicated()) {
    size_t size = DataType::Size(instruction->GetPackedType());
    size_t offset = mirror::Array::DataOffset(size).Uint32Value();
    if (TryExtractArrayAccessAddress(
            codegen_, instruction, instruction->GetArray(), instruction->GetIndex(), offset)) {
      RecordSimplification();
    }
  }
}

void InstructionSimplifierArm64Visitor::VisitVecAdd(HVecAdd* instruction) {
  HInstruction* right = instruction->GetRight();
  HInstruction* left = instruction->GetLeft();
  if ((right->IsVecMul() && TryCombineVecMultiplyAccumulate(right->AsVecMul(), instruction)) ||
      (left->IsVecMul() && TryCombineVecMultiplyAccumulate(left->AsVecMul(), instruction))) {
    RecordSimplification();
  }
}

void InstructionSimplifierArm64Visitor::VisitVecSub(HVecSub* instruction) {
  HInstruction* right = instruction->GetRight();
  if (right->IsVecMul() && TryCombineVecMultiplyAccumulate(right->AsVecMul(), instruction)) {
    RecordSimplification();
  }
}

bool InstructionSimplifierArm64Visitor::TryCombineVecMultiplyAccumulate(
    HVecMul* mul, HVecBinaryOperation* binop) {
  DCHECK(binop->IsVecAdd() || binop->IsVecSub());
  DCHECK_IMPLIES(binop->IsVecSub(), mul == binop->GetRight());

  DataType::Type type = mul->GetPackedType();
  if (!(type == DataType::Type::kUint8 ||
        type == DataType::Type::kInt8 ||
        type == DataType::Type::kUint16 ||
        type == DataType::Type::kInt16 ||
        type == DataType::Type::kInt32)) {
    return false;
  }

  if (!mul->HasOnlyOneNonEnvironmentUse()) {
    return false;
  }

  // Replace code looking like
  //    VECMUL tmp, x, y
  //    VECADD/SUB dst, acc, tmp
  // with
  //    VECMULACC dst, acc, x, y
  // Note that we do not want to (unconditionally) perform the merge when the
  // multiplication has multiple uses and it can be merged in all of them.
  // Multiple uses could happen on the same control-flow path, and we would
  // then increase the amount of work. In the future we could try to evaluate
  // whether all uses are on different control-flow paths (using dominance and
  // reverse-dominance information) and only perform the merge when they are.
  HInstruction* accumulator = nullptr;
  HVecBinaryOperation* vec_binop = binop->AsVecBinaryOperation();
  HInstruction* binop_left = vec_binop->GetLeft();
  HInstruction* binop_right = vec_binop->GetRight();
  // This is always true since the `HVecMul` has only one use (which is checked above).
  DCHECK_NE(binop_left, binop_right);
  if (binop_right == mul) {
    accumulator = binop_left;
  } else {
    DCHECK_EQ(binop_left, mul);
    DCHECK(binop->IsVecAdd());  // Only addition is commutative.
    accumulator = binop_right;
  }

  DCHECK(accumulator != nullptr);
  HInstruction::InstructionKind kind =
      binop->IsVecAdd() ? HInstruction::kAdd : HInstruction::kSub;

  bool predicated_simd = vec_binop->IsPredicated();
  if (predicated_simd && !HVecOperation::HaveSamePredicate(vec_binop, mul)) {
    return false;
  }

  ArenaAllocator* allocator = GetGraph()->GetAllocator();
  HVecMultiplyAccumulate* mulacc =
      new (allocator) HVecMultiplyAccumulate(allocator,
                                             kind,
                                             accumulator,
                                             mul->GetLeft(),
                                             mul->GetRight(),
                                             vec_binop->GetPackedType(),
                                             vec_binop->GetVectorLength(),
                                             vec_binop->GetDexPc());

  vec_binop->GetBlock()->ReplaceAndRemoveInstructionWith(vec_binop, mulacc);
  if (predicated_simd) {
    mulacc->SetGoverningPredicate(vec_binop->GetGoverningPredicate(),
                                  vec_binop->GetPredicationKind());
  }

  DCHECK(!mul->HasUses());
  mul->GetBlock()->RemoveInstruction(mul);
  return true;
}

bool InstructionSimplifierArm64::Run() {
  InstructionSimplifierArm64Visitor visitor(graph_, codegen_, stats_);
  visitor.VisitReversePostOrder();
  return true;
}

}  // namespace arm64
}  // namespace art

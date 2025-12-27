/*
 * Copyright (C) 2014 The Android Open Source Project
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
#include "nodes.h"

#include <algorithm>
#include <cfloat>
#include <functional>
#include <optional>

#include "art_method-inl.h"
#include "base/arena_allocator.h"
#include "base/arena_bit_vector.h"
#include "base/bit_utils.h"
#include "base/bit_vector-inl.h"
#include "base/bit_vector.h"
#include "base/iteration_range.h"
#include "base/logging.h"
#include "base/calloc_arena_pool.h"
#include "base/scoped_arena_allocator.h"
#include "base/scoped_arena_containers.h"
#include "base/stl_util.h"
#include "class_linker-inl.h"
#include "class_root-inl.h"
#include "code_generator.h"
#include "common_dominator.h"
#include "intrinsic_objects.h"
#include "intrinsics.h"
#include "intrinsics_list.h"
#include "loop_information-inl.h"
#include "mirror/class-inl.h"
#include "scoped_thread_state_change-inl.h"
#include "ssa_builder.h"

namespace art HIDDEN {

// Enable floating-point static evaluation during constant folding
// only if all floating-point operations and constants evaluate in the
// range and precision of the type used (i.e., 32-bit float, 64-bit
// double).
static constexpr bool kEnableFloatingPointStaticEvaluation = (FLT_EVAL_METHOD == 0);

// Remove the environment use records of the instruction for users.
void HInstruction::RemoveEnvironmentUses() {
  for (HEnvironment* environment = GetEnvironment();
       environment != nullptr;
       environment = environment->GetParent()) {
    for (size_t i = 0, e = environment->Size(); i < e; ++i) {
      if (environment->GetInstructionAt(i) != nullptr) {
        environment->RemoveAsUserOfInput(i);
      }
    }
  }
}

// Return whether the instruction has an environment and it's used by others.
bool HasEnvironmentUsedByOthers(HInstruction* instruction) {
  for (HEnvironment* environment = instruction->GetEnvironment();
       environment != nullptr;
       environment = environment->GetParent()) {
    for (size_t i = 0, e = environment->Size(); i < e; ++i) {
      HInstruction* user = environment->GetInstructionAt(i);
      if (user != nullptr) {
        return true;
      }
    }
  }
  return false;
}

// Reset environment records of the instruction itself.
void ResetEnvironmentInputRecords(HInstruction* instruction) {
  for (HEnvironment* environment = instruction->GetEnvironment();
       environment != nullptr;
       environment = environment->GetParent()) {
    for (size_t i = 0, e = environment->Size(); i < e; ++i) {
      DCHECK(environment->GetHolder() == instruction);
      if (environment->GetInstructionAt(i) != nullptr) {
        environment->SetRawEnvAt(i, nullptr);
      }
    }
  }
}

// This method assumes `insn` has been removed from all users with the exception of catch
// phis because of missing exceptional edges in the graph. It removes the
// instruction from catch phi uses, together with inputs of other catch phis in
// the catch block at the same index, as these must be dead too.
static void RemoveCatchPhiUsesOfDeadInstruction(HInstruction* insn) {
  DCHECK(!insn->HasEnvironmentUses());
  while (insn->HasNonEnvironmentUses()) {
    const HUseListNode<HInstruction*>& use = insn->GetUses().front();
    size_t use_index = use.GetIndex();
    HBasicBlock* user_block = use.GetUser()->GetBlock();
    DCHECK(use.GetUser()->IsPhi());
    DCHECK(user_block->IsCatchBlock());
    for (HInstructionIteratorPrefetchNext phi_it(user_block->GetPhis()); !phi_it.Done();
         phi_it.Advance()) {
      phi_it.Current()->AsPhi()->RemoveInputAt(use_index);
    }
  }
}

void HBasicBlock::ClearDominanceInformation() {
  dominated_blocks_.clear();
  dominator_ = nullptr;
}

HInstruction* HBasicBlock::GetFirstInstructionDisregardMoves() const {
  HInstruction* instruction = GetFirstInstruction();
  while (instruction->IsParallelMove()) {
    instruction = instruction->GetNext();
  }
  return instruction;
}

bool HBasicBlock::Dominates(const HBasicBlock* other) const {
  // Walk up the dominator tree from `other`, to find out if `this`
  // is an ancestor.
  const HBasicBlock* current = other;
  while (current != nullptr) {
    if (current == this) {
      return true;
    }
    current = current->GetDominator();
  }
  return false;
}

static void UpdateInputsUsers(HGraph* graph, HInstruction* instruction) {
  HInputsRef inputs = instruction->GetInputs();
  if (inputs.size() != 0u) {
    ArenaAllocator* allocator = graph->GetAllocator();
    for (size_t i = 0; i < inputs.size(); ++i) {
      inputs[i]->AddUseAt(allocator, instruction, i);
    }
  }
  // Environment should be created later.
  DCHECK(!instruction->HasEnvironment());
}

void HBasicBlock::ReplaceAndRemovePhiWith(HPhi* initial, HPhi* replacement) {
  DCHECK(initial->GetBlock() == this);
  InsertPhiAfter(replacement, initial);
  initial->ReplaceWith(replacement);
  RemovePhi(initial);
}

void HBasicBlock::ReplaceAndRemoveInstructionWith(HInstruction* initial,
                                                  HInstruction* replacement) {
  DCHECK(initial->GetBlock() == this);
  if (initial->IsControlFlow()) {
    // We can only replace a control flow instruction with another control flow instruction.
    DCHECK(replacement->IsControlFlow());
    DCHECK_EQ(replacement->GetId(), -1);
    DCHECK_EQ(replacement->GetType(), DataType::Type::kVoid);
    DCHECK_EQ(initial->GetBlock(), this);
    DCHECK_EQ(initial->GetType(), DataType::Type::kVoid);
    DCHECK(initial->GetUses().empty());
    DCHECK(initial->GetEnvUses().empty());
    replacement->SetBlock(this);
    HGraph* graph = GetGraph();
    replacement->SetId(graph->AllocateInstructionId());
    instructions_.InsertInstructionBefore(replacement, initial);
    UpdateInputsUsers(graph, replacement);
  } else {
    InsertInstructionBefore(replacement, initial);
    initial->ReplaceWith(replacement);
  }
  RemoveInstruction(initial);
}

static void Add(HInstructionList* instruction_list,
                HBasicBlock* block,
                HInstruction* instruction) {
  DCHECK(instruction->GetBlock() == nullptr);
  DCHECK_EQ(instruction->GetId(), -1);
  instruction->SetBlock(block);
  HGraph* graph = block->GetGraph();
  instruction->SetId(graph->AllocateInstructionId());
  UpdateInputsUsers(graph, instruction);
  instruction_list->AddInstruction(instruction);
}

void HBasicBlock::AddInstruction(HInstruction* instruction) {
  Add(&instructions_, this, instruction);
}

void HBasicBlock::AddPhi(HPhi* phi) {
  Add(&phis_, this, phi);
}

void HBasicBlock::InsertInstructionBefore(HInstruction* instruction, HInstruction* cursor) {
  DCHECK(!cursor->IsPhi());
  DCHECK(!instruction->IsPhi());
  DCHECK_EQ(instruction->GetId(), -1);
  DCHECK_NE(cursor->GetId(), -1);
  DCHECK_EQ(cursor->GetBlock(), this);
  DCHECK(!instruction->IsControlFlow());
  instruction->SetBlock(this);
  HGraph* graph = GetGraph();
  instruction->SetId(graph->AllocateInstructionId());
  UpdateInputsUsers(graph, instruction);
  instructions_.InsertInstructionBefore(instruction, cursor);
}

void HBasicBlock::InsertInstructionAfter(HInstruction* instruction, HInstruction* cursor) {
  DCHECK(!cursor->IsPhi());
  DCHECK(!instruction->IsPhi());
  DCHECK_EQ(instruction->GetId(), -1);
  DCHECK_NE(cursor->GetId(), -1);
  DCHECK_EQ(cursor->GetBlock(), this);
  DCHECK(!instruction->IsControlFlow());
  DCHECK(!cursor->IsControlFlow());
  instruction->SetBlock(this);
  HGraph* graph = GetGraph();
  instruction->SetId(graph->AllocateInstructionId());
  UpdateInputsUsers(graph, instruction);
  instructions_.InsertInstructionAfter(instruction, cursor);
}

void HBasicBlock::InsertPhiAfter(HPhi* phi, HPhi* cursor) {
  DCHECK_EQ(phi->GetId(), -1);
  DCHECK_NE(cursor->GetId(), -1);
  DCHECK_EQ(cursor->GetBlock(), this);
  phi->SetBlock(this);
  HGraph* graph = GetGraph();
  phi->SetId(graph->AllocateInstructionId());
  UpdateInputsUsers(graph, phi);
  phis_.InsertInstructionAfter(phi, cursor);
}

static void Remove(HInstructionList* instruction_list,
                   HBasicBlock* block,
                   HInstruction* instruction,
                   bool ensure_safety) {
  DCHECK_EQ(block, instruction->GetBlock());
  instruction->SetBlock(nullptr);
  instruction_list->RemoveInstruction(instruction);
  if (ensure_safety) {
    DCHECK(instruction->GetUses().empty());
    DCHECK(instruction->GetEnvUses().empty());
    instruction->RemoveAsUser();
  }
}

void HBasicBlock::RemoveInstruction(HInstruction* instruction, bool ensure_safety) {
  DCHECK(!instruction->IsPhi());
  Remove(&instructions_, this, instruction, ensure_safety);
}

void HBasicBlock::RemovePhi(HPhi* phi, bool ensure_safety) {
  Remove(&phis_, this, phi, ensure_safety);
}

void HBasicBlock::RemoveInstructionOrPhi(HInstruction* instruction, bool ensure_safety) {
  if (instruction->IsPhi()) {
    RemovePhi(instruction->AsPhi(), ensure_safety);
  } else {
    RemoveInstruction(instruction, ensure_safety);
  }
}

void HEnvironment::CopyFrom(ArenaAllocator* allocator, ArrayRef<HInstruction* const> locals) {
  DCHECK_EQ(locals.size(), Size());
  for (size_t i = 0; i < locals.size(); i++) {
    HInstruction* instruction = locals[i];
    SetRawEnvAt(i, instruction);
    if (instruction != nullptr) {
      instruction->AddEnvUseAt(allocator, this, i);
    }
  }
}

void HEnvironment::CopyFrom(ArenaAllocator* allocator, const HEnvironment* env) {
  DCHECK_EQ(env->Size(), Size());
  for (size_t i = 0; i < env->Size(); i++) {
    HInstruction* instruction = env->GetInstructionAt(i);
    SetRawEnvAt(i, instruction);
    if (instruction != nullptr) {
      instruction->AddEnvUseAt(allocator, this, i);
    }
  }
}

void HEnvironment::CopyFromWithLoopPhiAdjustment(ArenaAllocator* allocator,
                                                 HEnvironment* env,
                                                 HBasicBlock* loop_header) {
  DCHECK(loop_header->IsLoopHeader());
  for (size_t i = 0; i < env->Size(); i++) {
    HInstruction* instruction = env->GetInstructionAt(i);
    SetRawEnvAt(i, instruction);
    if (instruction == nullptr) {
      continue;
    }
    if (instruction->IsLoopHeaderPhi() && (instruction->GetBlock() == loop_header)) {
      // At the end of the loop pre-header, the corresponding value for instruction
      // is the first input of the phi.
      HInstruction* initial = instruction->AsPhi()->InputAt(0);
      SetRawEnvAt(i, initial);
      initial->AddEnvUseAt(allocator, this, i);
    } else {
      instruction->AddEnvUseAt(allocator, this, i);
    }
  }
}

void HEnvironment::RemoveAsUserOfInput(size_t index) const {
  const HUserRecord<HEnvironment*>& env_use = GetVRegs()[index];
  HInstruction* user = env_use.GetInstruction();
  auto before_env_use_node = env_use.GetBeforeUseNode();
  user->env_uses_.erase_after(before_env_use_node);
  user->FixUpUserRecordsAfterEnvUseRemoval(before_env_use_node);
}

void HEnvironment::ReplaceInput(HInstruction* replacement, size_t index) {
  const HUserRecord<HEnvironment*>& env_use_record = GetVRegs()[index];
  HInstruction* orig_instr = env_use_record.GetInstruction();

  DCHECK(orig_instr != replacement);

  HUseList<HEnvironment*>::iterator before_use_node = env_use_record.GetBeforeUseNode();
  // Note: fixup_end remains valid across splice_after().
  auto fixup_end = replacement->env_uses_.empty() ? replacement->env_uses_.begin()
                                                  : ++replacement->env_uses_.begin();
  replacement->env_uses_.splice_after(replacement->env_uses_.before_begin(),
                                      env_use_record.GetInstruction()->env_uses_,
                                      before_use_node);
  replacement->FixUpUserRecordsAfterEnvUseInsertion(fixup_end);
  orig_instr->FixUpUserRecordsAfterEnvUseRemoval(before_use_node);
}

std::ostream& HInstruction::Dump(std::ostream& os, bool dump_args) {
  // Note: Handle the case where the instruction has been removed from
  // the graph to support debugging output for failed gtests.
  HGraph* graph = (GetBlock() != nullptr) ? GetBlock()->GetGraph() : nullptr;
  HGraphVisualizer::DumpInstruction(&os, graph, this);
  if (dump_args) {
    // Allocate memory from local ScopedArenaAllocator.
    std::optional<CallocArenaPool> local_arena_pool;
    std::optional<ArenaStack> local_arena_stack;
    if (UNLIKELY(graph == nullptr)) {
      local_arena_pool.emplace();
      local_arena_stack.emplace(&local_arena_pool.value());
    }
    ScopedArenaAllocator allocator(
        graph != nullptr ? graph->GetArenaStack() : &local_arena_stack.value());
    // Instructions that we already visited. We print each instruction only once.
    ArenaBitVector visited(&allocator,
                           (graph != nullptr) ? graph->GetCurrentInstructionId() : 0u,
                           /* expandable= */ (graph == nullptr),
                           kArenaAllocMisc);
    visited.SetBit(GetId());
    // Keep a queue of instructions with their indentations.
    ScopedArenaDeque<std::pair<HInstruction*, size_t>> queue(allocator.Adapter(kArenaAllocMisc));
    auto add_args = [&queue](HInstruction* instruction, size_t indentation) {
      for (HInstruction* arg : ReverseRange(instruction->GetInputs())) {
        queue.emplace_front(arg, indentation);
      }
    };
    add_args(this, /*indentation=*/ 1u);
    while (!queue.empty()) {
      HInstruction* instruction;
      size_t indentation;
      std::tie(instruction, indentation) = queue.front();
      queue.pop_front();
      if (!visited.IsBitSet(instruction->GetId())) {
        visited.SetBit(instruction->GetId());
        os << '\n';
        for (size_t i = 0; i != indentation; ++i) {
          os << "  ";
        }
        HGraphVisualizer::DumpInstruction(&os, graph, instruction);
        add_args(instruction, indentation + 1u);
      }
    }
  }
  return os;
}

HInstruction* HInstruction::GetNextDisregardingMoves() const {
  HInstruction* next = GetNext();
  while (next != nullptr && next->IsParallelMove()) {
    next = next->GetNext();
  }
  return next;
}

HInstruction* HInstruction::GetPreviousDisregardingMoves() const {
  HInstruction* previous = GetPrevious();
  while (previous != nullptr && previous->IsParallelMove()) {
    previous = previous->GetPrevious();
  }
  return previous;
}

bool HInstruction::Dominates(HInstruction* other_instruction) const {
  return other_instruction == this || StrictlyDominates(other_instruction);
}

bool HInstruction::StrictlyDominates(HInstruction* other_instruction) const {
  if (other_instruction == this) {
    // An instruction does not strictly dominate itself.
    return false;
  }
  HBasicBlock* block = GetBlock();
  HBasicBlock* other_block = other_instruction->GetBlock();
  if (block != other_block) {
    return GetBlock()->Dominates(other_instruction->GetBlock());
  } else {
    // If both instructions are in the same block, ensure this
    // instruction comes before `other_instruction`.
    if (IsPhi()) {
      if (!other_instruction->IsPhi()) {
        // Phis appear before non phi-instructions so this instruction
        // dominates `other_instruction`.
        return true;
      } else {
        // There is no order among phis.
        LOG(FATAL) << "There is no dominance between phis of a same block.";
        UNREACHABLE();
      }
    } else {
      // `this` is not a phi.
      if (other_instruction->IsPhi()) {
        // Phis appear before non phi-instructions so this instruction
        // does not dominate `other_instruction`.
        return false;
      } else {
        // Check whether this instruction comes before
        // `other_instruction` in the instruction list.
        return block->GetInstructions().FoundBefore(this, other_instruction);
      }
    }
  }
}

void HInstruction::RemoveEnvironment() {
  RemoveEnvironmentUses();
  environment_ = nullptr;
}

void HInstruction::ReplaceWith(HInstruction* other) {
  DCHECK(other != nullptr);
  // Note: fixup_end remains valid across splice_after().
  auto fixup_end = other->uses_.empty() ? other->uses_.begin() : ++other->uses_.begin();
  other->uses_.splice_after(other->uses_.before_begin(), uses_);
  other->FixUpUserRecordsAfterUseInsertion(fixup_end);

  // Note: env_fixup_end remains valid across splice_after().
  auto env_fixup_end =
      other->env_uses_.empty() ? other->env_uses_.begin() : ++other->env_uses_.begin();
  other->env_uses_.splice_after(other->env_uses_.before_begin(), env_uses_);
  other->FixUpUserRecordsAfterEnvUseInsertion(env_fixup_end);

  DCHECK(uses_.empty());
  DCHECK(env_uses_.empty());
}

void HInstruction::ReplaceUsesDominatedBy(HInstruction* dominator,
                                          HInstruction* replacement,
                                          bool strictly_dominated) {
  HBasicBlock* dominator_block = dominator->GetBlock();
  BitVectorView<size_t> visited_blocks;

  // Lazily compute the dominated blocks to faster calculation of domination afterwards.
  auto maybe_generate_visited_blocks = [&visited_blocks, this, dominator_block]() {
    if (visited_blocks.SizeInBits() != 0u) {
      DCHECK_EQ(visited_blocks.SizeInBits(), GetBlock()->GetGraph()->GetBlocks().size());
      return;
    }
    HGraph* graph = GetBlock()->GetGraph();
    const size_t size = graph->GetBlocks().size();
    visited_blocks = ArenaBitVector::CreateFixedSize(graph->GetAllocator(), size, kArenaAllocMisc);
    ScopedArenaAllocator allocator(graph->GetArenaStack());
    ScopedArenaVector<const HBasicBlock*> worklist(allocator.Adapter(kArenaAllocMisc));
    worklist.reserve(size);
    worklist.push_back(dominator_block);
    visited_blocks.SetBit(dominator_block->GetBlockId());

    while (!worklist.empty()) {
      const HBasicBlock* current = worklist.back();
      worklist.pop_back();
      for (HBasicBlock* dominated : current->GetDominatedBlocks()) {
        DCHECK(!visited_blocks.IsBitSet(dominated->GetBlockId()));
        visited_blocks.SetBit(dominated->GetBlockId());
        worklist.push_back(dominated);
      }
    }
  };

  const HUseList<HInstruction*>& uses = GetUses();
  for (auto it = uses.begin(), end = uses.end(); it != end; /* ++it below */) {
    HInstruction* user = it->GetUser();
    HBasicBlock* block = user->GetBlock();
    size_t index = it->GetIndex();
    // Increment `it` now because `*it` may disappear thanks to user->ReplaceInput().
    ++it;
    bool dominated = false;
    if (dominator_block == block) {
      // Trickier case, call the other methods.
      dominated =
          strictly_dominated ? dominator->StrictlyDominates(user) : dominator->Dominates(user);
    } else {
      // Block domination.
      maybe_generate_visited_blocks();
      dominated = visited_blocks.IsBitSet(block->GetBlockId());
    }

    if (dominated) {
      user->ReplaceInput(replacement, index);
    } else if (user->IsPhi() && !user->AsPhi()->IsCatchPhi()) {
      // If the input flows from a block dominated by `dominator`, we can replace it.
      // We do not perform this for catch phis as we don't have control flow support
      // for their inputs.
      HBasicBlock* predecessor = block->GetPredecessors()[index];
      maybe_generate_visited_blocks();
      if (visited_blocks.IsBitSet(predecessor->GetBlockId())) {
        user->ReplaceInput(replacement, index);
      }
    }
  }
}

void HInstruction::ReplaceEnvUsesDominatedBy(HInstruction* dominator, HInstruction* replacement) {
  const HUseList<HEnvironment*>& uses = GetEnvUses();
  for (auto it = uses.begin(), end = uses.end(); it != end; /* ++it below */) {
    HEnvironment* user = it->GetUser();
    size_t index = it->GetIndex();
    // Increment `it` now because `*it` may disappear thanks to user->ReplaceInput().
    ++it;
    if (dominator->StrictlyDominates(user->GetHolder())) {
      user->ReplaceInput(replacement, index);
    }
  }
}

void HInstruction::ReplaceInput(HInstruction* replacement, size_t index) {
  HUserRecord<HInstruction*> input_use = InputRecordAt(index);
  if (input_use.GetInstruction() == replacement) {
    // Nothing to do.
    return;
  }
  HUseList<HInstruction*>::iterator before_use_node = input_use.GetBeforeUseNode();
  // Note: fixup_end remains valid across splice_after().
  auto fixup_end =
      replacement->uses_.empty() ? replacement->uses_.begin() : ++replacement->uses_.begin();
  replacement->uses_.splice_after(replacement->uses_.before_begin(),
                                  input_use.GetInstruction()->uses_,
                                  before_use_node);
  replacement->FixUpUserRecordsAfterUseInsertion(fixup_end);
  input_use.GetInstruction()->FixUpUserRecordsAfterUseRemoval(before_use_node);
}

size_t HInstruction::EnvironmentSize() const {
  return HasEnvironment() ? environment_->Size() : 0;
}

void HVariableInputSizeInstruction::AddInput(HInstruction* input) {
  DCHECK(input->GetBlock() != nullptr);
  inputs_.push_back(HUserRecord<HInstruction*>(input));
  input->AddUseAt(GetBlock()->GetGraph()->GetAllocator(), this, inputs_.size() - 1);
}

void HVariableInputSizeInstruction::InsertInputAt(size_t index, HInstruction* input) {
  inputs_.insert(inputs_.begin() + index, HUserRecord<HInstruction*>(input));
  // Update indexes in use nodes of inputs that have been pushed further back by the insert().
  for (size_t i = index + 1u, e = inputs_.size(); i < e; ++i) {
    DCHECK_EQ(inputs_[i].GetUseNode()->GetIndex(), i - 1u);
    inputs_[i].GetUseNode()->SetIndex(i);
  }
  // Add the use after updating the indexes. If the `input` is already used by `this`,
  // the fixup after use insertion can use those indexes.
  input->AddUseAt(GetBlock()->GetGraph()->GetAllocator(), this, index);
}

void HVariableInputSizeInstruction::RemoveInputAt(size_t index) {
  RemoveAsUserOfInput(index);
  inputs_.erase(inputs_.begin() + index);
  // Update indexes in use nodes of inputs that have been pulled forward by the erase().
  for (size_t i = index, e = inputs_.size(); i < e; ++i) {
    DCHECK_EQ(inputs_[i].GetUseNode()->GetIndex(), i + 1u);
    inputs_[i].GetUseNode()->SetIndex(i);
  }
}

void HVariableInputSizeInstruction::RemoveAllInputs() {
  RemoveAsUserOfAllInputs();
  DCHECK(!HasNonEnvironmentUses());

  inputs_.clear();
  DCHECK_EQ(0u, InputCount());
}

size_t HConstructorFence::RemoveConstructorFences(HInstruction* instruction) {
  DCHECK(instruction->GetBlock() != nullptr);
  // Removing constructor fences only makes sense for instructions with an object return type.
  DCHECK_EQ(DataType::Type::kReference, instruction->GetType());

  // Return how many instructions were removed for statistic purposes.
  size_t remove_count = 0;

  // Efficient implementation that simultaneously (in one pass):
  // * Scans the uses list for all constructor fences.
  // * Deletes that constructor fence from the uses list of `instruction`.
  // * Deletes `instruction` from the constructor fence's inputs.
  // * Deletes the constructor fence if it now has 0 inputs.

  const HUseList<HInstruction*>& uses = instruction->GetUses();
  // Warning: Although this is "const", we might mutate the list when calling RemoveInputAt.
  for (auto it = uses.begin(), end = uses.end(); it != end; ) {
    const HUseListNode<HInstruction*>& use_node = *it;
    HInstruction* const use_instruction = use_node.GetUser();

    // Advance the iterator immediately once we fetch the use_node.
    // Warning: If the input is removed, the current iterator becomes invalid.
    ++it;

    if (use_instruction->IsConstructorFence()) {
      HConstructorFence* ctor_fence = use_instruction->AsConstructorFence();
      size_t input_index = use_node.GetIndex();

      // Process the candidate instruction for removal
      // from the graph.

      // Constructor fence instructions are never
      // used by other instructions.
      //
      // If we wanted to make this more generic, it
      // could be a runtime if statement.
      DCHECK(!ctor_fence->HasUses());

      // A constructor fence's return type is "kPrimVoid"
      // and therefore it can't have any environment uses.
      DCHECK(!ctor_fence->HasEnvironmentUses());

      // Remove the inputs first, otherwise removing the instruction
      // will try to remove its uses while we are already removing uses
      // and this operation will fail.
      DCHECK_EQ(instruction, ctor_fence->InputAt(input_index));

      // Removing the input will also remove the `use_node`.
      // (Do not look at `use_node` after this, it will be a dangling reference).
      ctor_fence->RemoveInputAt(input_index);

      // Once all inputs are removed, the fence is considered dead and
      // is removed.
      if (ctor_fence->InputCount() == 0u) {
        ctor_fence->GetBlock()->RemoveInstruction(ctor_fence);
        ++remove_count;
      }
    }
  }

  if (kIsDebugBuild) {
    // Post-condition checks:
    // * None of the uses of `instruction` are a constructor fence.
    // * The `instruction` itself did not get removed from a block.
    for (const HUseListNode<HInstruction*>& use_node : instruction->GetUses()) {
      CHECK(!use_node.GetUser()->IsConstructorFence());
    }
    CHECK(instruction->GetBlock() != nullptr);
  }

  return remove_count;
}

void HConstructorFence::Merge(HConstructorFence* other) {
  // Do not delete yourself from the graph.
  DCHECK(this != other);
  // Don't try to merge with an instruction not associated with a block.
  DCHECK(other->GetBlock() != nullptr);
  // A constructor fence's return type is "kPrimVoid"
  // and therefore it cannot have any environment uses.
  DCHECK(!other->HasEnvironmentUses());

  auto has_input = [](HInstruction* haystack, HInstruction* needle) {
    // Check if `haystack` has `needle` as any of its inputs.
    for (size_t input_count = 0; input_count < haystack->InputCount(); ++input_count) {
      if (haystack->InputAt(input_count) == needle) {
        return true;
      }
    }
    return false;
  };

  // Add any inputs from `other` into `this` if it wasn't already an input.
  for (size_t input_count = 0; input_count < other->InputCount(); ++input_count) {
    HInstruction* other_input = other->InputAt(input_count);
    if (!has_input(this, other_input)) {
      AddInput(other_input);
    }
  }

  other->GetBlock()->RemoveInstruction(other);
}

HInstruction* HConstructorFence::GetAssociatedAllocation(bool ignore_inputs) {
  HInstruction* new_instance_inst = GetPrevious();
  // Check if the immediately preceding instruction is a new-instance/new-array.
  // Otherwise this fence is for protecting final fields.
  if (new_instance_inst != nullptr &&
      (new_instance_inst->IsNewInstance() || new_instance_inst->IsNewArray())) {
    if (ignore_inputs) {
      // If inputs are ignored, simply check if the predecessor is
      // *any* HNewInstance/HNewArray.
      //
      // Inputs are normally only ignored for prepare_for_register_allocation,
      // at which point *any* prior HNewInstance/Array can be considered
      // associated.
      return new_instance_inst;
    } else {
      // Normal case: There must be exactly 1 input and the previous instruction
      // must be that input.
      if (InputCount() == 1u && InputAt(0) == new_instance_inst) {
        return new_instance_inst;
      }
    }
  }
  return nullptr;
}

void HGraphVisitor::VisitInsertionOrder() {
  for (HBasicBlock* block : graph_->GetActiveBlocks()) {
    VisitBasicBlock(block);
  }
}

void HGraphVisitor::VisitReversePostOrder() {
  for (HBasicBlock* block : graph_->GetReversePostOrder()) {
    VisitBasicBlock(block);
  }
}

void HGraphVisitor::VisitBasicBlock(HBasicBlock* block) {
  VisitPhis(block);
  VisitNonPhiInstructions(block);
}

void HGraphVisitor::VisitPhis(HBasicBlock* block) {
  for (HInstructionIteratorPrefetchNext it(block->GetPhis()); !it.Done(); it.Advance()) {
    DCHECK(it.Current()->IsPhi());
    VisitPhi(it.Current()->AsPhi());
  }
}

void HGraphVisitor::VisitNonPhiInstructions(HBasicBlock* block) {
  for (HInstructionIteratorPrefetchNext it(block->GetInstructions()); !it.Done(); it.Advance()) {
    DCHECK(!it.Current()->IsPhi());
    Dispatch(it.Current());
  }
}

HConstant* HTypeConversion::TryStaticEvaluation() const { return TryStaticEvaluation(GetInput()); }

HConstant* HTypeConversion::TryStaticEvaluation(HInstruction* input) const {
  HGraph* graph = input->GetBlock()->GetGraph();
  if (input->IsIntConstant()) {
    int32_t value = input->AsIntConstant()->GetValue();
    switch (GetResultType()) {
      case DataType::Type::kInt8:
        return graph->GetIntConstant(static_cast<int8_t>(value));
      case DataType::Type::kUint8:
        return graph->GetIntConstant(static_cast<uint8_t>(value));
      case DataType::Type::kInt16:
        return graph->GetIntConstant(static_cast<int16_t>(value));
      case DataType::Type::kUint16:
        return graph->GetIntConstant(static_cast<uint16_t>(value));
      case DataType::Type::kInt64:
        return graph->GetLongConstant(static_cast<int64_t>(value));
      case DataType::Type::kFloat32:
        return graph->GetFloatConstant(static_cast<float>(value));
      case DataType::Type::kFloat64:
        return graph->GetDoubleConstant(static_cast<double>(value));
      default:
        return nullptr;
    }
  } else if (input->IsLongConstant()) {
    int64_t value = input->AsLongConstant()->GetValue();
    switch (GetResultType()) {
      case DataType::Type::kInt8:
        return graph->GetIntConstant(static_cast<int8_t>(value));
      case DataType::Type::kUint8:
        return graph->GetIntConstant(static_cast<uint8_t>(value));
      case DataType::Type::kInt16:
        return graph->GetIntConstant(static_cast<int16_t>(value));
      case DataType::Type::kUint16:
        return graph->GetIntConstant(static_cast<uint16_t>(value));
      case DataType::Type::kInt32:
        return graph->GetIntConstant(static_cast<int32_t>(value));
      case DataType::Type::kFloat32:
        return graph->GetFloatConstant(static_cast<float>(value));
      case DataType::Type::kFloat64:
        return graph->GetDoubleConstant(static_cast<double>(value));
      default:
        return nullptr;
    }
  } else if (input->IsFloatConstant()) {
    float value = input->AsFloatConstant()->GetValue();
    switch (GetResultType()) {
      case DataType::Type::kInt32:
        if (std::isnan(value))
          return graph->GetIntConstant(0);
        if (value >= static_cast<float>(kPrimIntMax))
          return graph->GetIntConstant(kPrimIntMax);
        if (value <= kPrimIntMin)
          return graph->GetIntConstant(kPrimIntMin);
        return graph->GetIntConstant(static_cast<int32_t>(value));
      case DataType::Type::kInt64:
        if (std::isnan(value))
          return graph->GetLongConstant(0);
        if (value >= static_cast<float>(kPrimLongMax))
          return graph->GetLongConstant(kPrimLongMax);
        if (value <= kPrimLongMin)
          return graph->GetLongConstant(kPrimLongMin);
        return graph->GetLongConstant(static_cast<int64_t>(value));
      case DataType::Type::kFloat64:
        return graph->GetDoubleConstant(static_cast<double>(value));
      default:
        return nullptr;
    }
  } else if (input->IsDoubleConstant()) {
    double value = input->AsDoubleConstant()->GetValue();
    switch (GetResultType()) {
      case DataType::Type::kInt32:
        if (std::isnan(value))
          return graph->GetIntConstant(0);
        if (value >= kPrimIntMax)
          return graph->GetIntConstant(kPrimIntMax);
        if (value <= kPrimLongMin)
          return graph->GetIntConstant(kPrimIntMin);
        return graph->GetIntConstant(static_cast<int32_t>(value));
      case DataType::Type::kInt64:
        if (std::isnan(value))
          return graph->GetLongConstant(0);
        if (value >= static_cast<double>(kPrimLongMax))
          return graph->GetLongConstant(kPrimLongMax);
        if (value <= kPrimLongMin)
          return graph->GetLongConstant(kPrimLongMin);
        return graph->GetLongConstant(static_cast<int64_t>(value));
      case DataType::Type::kFloat32:
        return graph->GetFloatConstant(static_cast<float>(value));
      default:
        return nullptr;
    }
  }
  return nullptr;
}

HConstant* HUnaryOperation::TryStaticEvaluation() const { return TryStaticEvaluation(GetInput()); }

HConstant* HUnaryOperation::TryStaticEvaluation(HInstruction* input) const {
  if (input->IsIntConstant()) {
    return Evaluate(input->AsIntConstant());
  } else if (input->IsLongConstant()) {
    return Evaluate(input->AsLongConstant());
  } else if (kEnableFloatingPointStaticEvaluation) {
    if (input->IsFloatConstant()) {
      return Evaluate(input->AsFloatConstant());
    } else if (input->IsDoubleConstant()) {
      return Evaluate(input->AsDoubleConstant());
    }
  }
  return nullptr;
}

HConstant* HBinaryOperation::TryStaticEvaluation() const {
  return TryStaticEvaluation(GetLeft(), GetRight());
}

HConstant* HBinaryOperation::TryStaticEvaluation(HInstruction* left, HInstruction* right) const {
  if (left->IsIntConstant() && right->IsIntConstant()) {
    return Evaluate(left->AsIntConstant(), right->AsIntConstant());
  } else if (left->IsLongConstant()) {
    if (right->IsIntConstant()) {
      // The binop(long, int) case is only valid for shifts and rotations.
      DCHECK(IsShl() || IsShr() || IsUShr() || IsRol() || IsRor()) << DebugName();
      return Evaluate(left->AsLongConstant(), right->AsIntConstant());
    } else if (right->IsLongConstant()) {
      return Evaluate(left->AsLongConstant(), right->AsLongConstant());
    }
  } else if (left->IsNullConstant() && right->IsNullConstant()) {
    // The binop(null, null) case is only valid for equal and not-equal conditions.
    DCHECK(IsEqual() || IsNotEqual()) << DebugName();
    return Evaluate(left->AsNullConstant(), right->AsNullConstant());
  } else if (kEnableFloatingPointStaticEvaluation) {
    if (left->IsFloatConstant() && right->IsFloatConstant()) {
      return Evaluate(left->AsFloatConstant(), right->AsFloatConstant());
    } else if (left->IsDoubleConstant() && right->IsDoubleConstant()) {
      return Evaluate(left->AsDoubleConstant(), right->AsDoubleConstant());
    }
  }
  return nullptr;
}

HConstant* HBinaryOperation::GetConstantRight() const {
  if (GetRight()->IsConstant()) {
    return GetRight()->AsConstant();
  } else if (IsCommutative() && GetLeft()->IsConstant()) {
    return GetLeft()->AsConstant();
  } else {
    return nullptr;
  }
}

// If `GetConstantRight()` returns one of the input, this returns the other
// one. Otherwise it returns null.
HInstruction* HBinaryOperation::GetLeastConstantLeft() const {
  HInstruction* most_constant_right = GetConstantRight();
  if (most_constant_right == nullptr) {
    return nullptr;
  } else if (most_constant_right == GetLeft()) {
    return GetRight();
  } else {
    return GetLeft();
  }
}

std::ostream& operator<<(std::ostream& os, ComparisonBias rhs) {
  // TODO: Replace with auto-generated operator<<.
  switch (rhs) {
    case ComparisonBias::kNoBias:
      return os << "none";
    case ComparisonBias::kGtBias:
      return os << "gt";
    case ComparisonBias::kLtBias:
      return os << "lt";
  }
}

HCondition* HCondition::Create(HGraph* graph,
                               IfCondition cond,
                               HInstruction* lhs,
                               HInstruction* rhs,
                               uint32_t dex_pc) {
  ArenaAllocator* allocator = graph->GetAllocator();
  switch (cond) {
    case kCondEQ: return new (allocator) HEqual(lhs, rhs, dex_pc);
    case kCondNE: return new (allocator) HNotEqual(lhs, rhs, dex_pc);
    case kCondLT: return new (allocator) HLessThan(lhs, rhs, dex_pc);
    case kCondLE: return new (allocator) HLessThanOrEqual(lhs, rhs, dex_pc);
    case kCondGT: return new (allocator) HGreaterThan(lhs, rhs, dex_pc);
    case kCondGE: return new (allocator) HGreaterThanOrEqual(lhs, rhs, dex_pc);
    case kCondB:  return new (allocator) HBelow(lhs, rhs, dex_pc);
    case kCondBE: return new (allocator) HBelowOrEqual(lhs, rhs, dex_pc);
    case kCondA:  return new (allocator) HAbove(lhs, rhs, dex_pc);
    case kCondAE: return new (allocator) HAboveOrEqual(lhs, rhs, dex_pc);
    default:
      LOG(FATAL) << "Unexpected condition " << cond;
      UNREACHABLE();
  }
}

bool HCondition::IsBeforeWhenDisregardMoves(HInstruction* instruction) const {
  return this == instruction->GetPreviousDisregardingMoves();
}

bool HInstruction::Equals(const HInstruction* other) const {
  if (GetKind() != other->GetKind()) return false;
  if (GetType() != other->GetType()) return false;
  if (!InstructionDataEquals(other)) return false;
  HConstInputsRef inputs = GetInputs();
  HConstInputsRef other_inputs = other->GetInputs();
  if (inputs.size() != other_inputs.size()) return false;
  for (size_t i = 0; i != inputs.size(); ++i) {
    if (inputs[i] != other_inputs[i]) return false;
  }

  DCHECK_EQ(ComputeHashCode(), other->ComputeHashCode());
  return true;
}

std::ostream& operator<<(std::ostream& os, HInstruction::InstructionKind rhs) {
#define DECLARE_CASE(type, super) case HInstruction::k##type: os << #type; break;
  switch (rhs) {
    FOR_EACH_CONCRETE_INSTRUCTION(DECLARE_CASE)
    default:
      os << "Unknown instruction kind " << static_cast<int>(rhs);
      break;
  }
#undef DECLARE_CASE
  return os;
}

std::ostream& operator<<(std::ostream& os, const HInstruction::NoArgsDump rhs) {
  // TODO Really this should be const but that would require const-ifying
  // graph-visualizer and HGraphVisitor which are tangled up everywhere.
  return const_cast<HInstruction*>(rhs.ins)->Dump(os, /* dump_args= */ false);
}

std::ostream& operator<<(std::ostream& os, const HInstruction::ArgsDump rhs) {
  // TODO Really this should be const but that would require const-ifying
  // graph-visualizer and HGraphVisitor which are tangled up everywhere.
  return const_cast<HInstruction*>(rhs.ins)->Dump(os, /* dump_args= */ true);
}

std::ostream& operator<<(std::ostream& os, const HInstruction& rhs) {
  return os << rhs.DumpWithoutArgs();
}

std::ostream& operator<<(std::ostream& os, const HUseList<HInstruction*>& lst) {
  os << "Instructions[";
  bool first = true;
  for (const auto& hi : lst) {
    if (!first) {
      os << ", ";
    }
    first = false;
    os << hi.GetUser()->DebugName() << "[id: " << hi.GetUser()->GetId()
       << ", blk: " << hi.GetUser()->GetBlock()->GetBlockId() << "]@" << hi.GetIndex();
  }
  os << "]";
  return os;
}

std::ostream& operator<<(std::ostream& os, const HUseList<HEnvironment*>& lst) {
  os << "Environments[";
  bool first = true;
  for (const auto& hi : lst) {
    if (!first) {
      os << ", ";
    }
    first = false;
    os << *hi.GetUser()->GetHolder() << "@" << hi.GetIndex();
  }
  os << "]";
  return os;
}

void HInstruction::MoveBefore(HInstruction* cursor, bool do_checks) {
  if (do_checks) {
    DCHECK(!IsPhi());
    DCHECK(!IsControlFlow());
    DCHECK(CanBeMoved() ||
           // HShouldDeoptimizeFlag can only be moved by CHAGuardOptimization.
           IsShouldDeoptimizeFlag());
    DCHECK(!cursor->IsPhi());
  }

  next_->previous_ = previous_;
  if (previous_ != nullptr) {
    previous_->next_ = next_;
  }
  if (block_->instructions_.first_instruction_ == this) {
    block_->instructions_.first_instruction_ = next_;
  }
  DCHECK_NE(block_->instructions_.last_instruction_, this);

  previous_ = cursor->previous_;
  if (previous_ != nullptr) {
    previous_->next_ = this;
  }
  next_ = cursor;
  cursor->previous_ = this;
  block_ = cursor->block_;

  if (block_->instructions_.first_instruction_ == cursor) {
    block_->instructions_.first_instruction_ = this;
  }
}

void HInstruction::MoveBeforeFirstUserAndOutOfLoops() {
  DCHECK(!CanThrow());
  DCHECK(!HasSideEffects());
  DCHECK(!HasEnvironmentUses());
  DCHECK(HasNonEnvironmentUses());
  DCHECK(!IsPhi());  // Makes no sense for Phi.
  DCHECK_EQ(InputCount(), 0u);

  // Find the target block.
  auto uses_it = GetUses().begin();
  auto uses_end = GetUses().end();
  HBasicBlock* target_block = uses_it->GetUser()->GetBlock();
  ++uses_it;
  while (uses_it != uses_end && uses_it->GetUser()->GetBlock() == target_block) {
    ++uses_it;
  }
  if (uses_it != uses_end) {
    // This instruction has uses in two or more blocks. Find the common dominator.
    CommonDominator finder(target_block);
    for (; uses_it != uses_end; ++uses_it) {
      finder.Update(uses_it->GetUser()->GetBlock());
    }
    target_block = finder.Get();
    DCHECK(target_block != nullptr);
  }
  // Move to the first dominator not in a loop.
  while (target_block->IsInLoop()) {
    target_block = target_block->GetDominator();
    DCHECK(target_block != nullptr);
  }

  // Find insertion position.
  HInstruction* insert_pos = nullptr;
  for (const HUseListNode<HInstruction*>& use : GetUses()) {
    if (use.GetUser()->GetBlock() == target_block &&
        (insert_pos == nullptr || use.GetUser()->StrictlyDominates(insert_pos))) {
      insert_pos = use.GetUser();
    }
  }
  if (insert_pos == nullptr) {
    // No user in `target_block`, insert before the control flow instruction.
    insert_pos = target_block->GetLastInstruction();
    DCHECK(insert_pos->IsControlFlow());
    // Avoid splitting HCondition from HIf to prevent unnecessary materialization.
    if (insert_pos->IsIf()) {
      HInstruction* if_input = insert_pos->AsIf()->InputAt(0);
      if (if_input == insert_pos->GetPrevious()) {
        insert_pos = if_input;
      }
    }
  }
  MoveBefore(insert_pos);
}

HBasicBlock* HBasicBlock::SplitBefore(HInstruction* cursor) {
  DCHECK_EQ(cursor->GetBlock(), this);

  HBasicBlock* new_block =
      HBasicBlock::Create(GetGraph()->GetAllocator(), GetGraph(), cursor->GetDexPc());
  new_block->instructions_.first_instruction_ = cursor;
  new_block->instructions_.last_instruction_ = instructions_.last_instruction_;
  instructions_.last_instruction_ = cursor->previous_;
  if (cursor->previous_ == nullptr) {
    instructions_.first_instruction_ = nullptr;
  } else {
    cursor->previous_->next_ = nullptr;
    cursor->previous_ = nullptr;
  }

  new_block->instructions_.SetBlockOfInstructions(new_block);
  AddInstruction(new (GetGraph()->GetAllocator()) HGoto(new_block->GetDexPc()));

  for (HBasicBlock* successor : GetSuccessors()) {
    successor->predecessors_[successor->GetPredecessorIndexOf(this)] = new_block;
  }
  new_block->successors_.swap(successors_);
  DCHECK(successors_.empty());
  AddSuccessor(new_block);

  GetGraph()->AddBlock(new_block);
  return new_block;
}

HBasicBlock* HBasicBlock::CreateImmediateDominator() {
  DCHECK(!graph_->IsInSsaForm()) << "Support for SSA form not implemented.";
  DCHECK(!IsCatchBlock()) << "Support for updating try/catch information not implemented.";

  HBasicBlock* new_block =
      HBasicBlock::Create(GetGraph()->GetAllocator(), GetGraph(), GetDexPc());

  for (HBasicBlock* predecessor : GetPredecessors()) {
    predecessor->successors_[predecessor->GetSuccessorIndexOf(this)] = new_block;
  }
  new_block->predecessors_.swap(predecessors_);
  DCHECK(predecessors_.empty());
  AddPredecessor(new_block);

  GetGraph()->AddBlock(new_block);
  return new_block;
}

HBasicBlock* HBasicBlock::SplitBeforeForInlining(HInstruction* cursor) {
  DCHECK_EQ(cursor->GetBlock(), this);

  HBasicBlock* new_block =
      HBasicBlock::Create(GetGraph()->GetAllocator(), GetGraph(), cursor->GetDexPc());
  new_block->instructions_.first_instruction_ = cursor;
  new_block->instructions_.last_instruction_ = instructions_.last_instruction_;
  instructions_.last_instruction_ = cursor->previous_;
  if (cursor->previous_ == nullptr) {
    instructions_.first_instruction_ = nullptr;
  } else {
    cursor->previous_->next_ = nullptr;
    cursor->previous_ = nullptr;
  }

  new_block->instructions_.SetBlockOfInstructions(new_block);

  for (HBasicBlock* successor : GetSuccessors()) {
    successor->predecessors_[successor->GetPredecessorIndexOf(this)] = new_block;
  }
  new_block->successors_.swap(successors_);
  DCHECK(successors_.empty());

  for (HBasicBlock* dominated : GetDominatedBlocks()) {
    dominated->dominator_ = new_block;
  }
  new_block->dominated_blocks_.swap(dominated_blocks_);
  DCHECK(dominated_blocks_.empty());
  return new_block;
}

HBasicBlock* HBasicBlock::SplitAfterForInlining(HInstruction* cursor) {
  DCHECK(!cursor->IsControlFlow());
  DCHECK_NE(instructions_.last_instruction_, cursor);
  DCHECK_EQ(cursor->GetBlock(), this);

  HBasicBlock* new_block =
      HBasicBlock::Create(GetGraph()->GetAllocator(), GetGraph(), GetDexPc());
  new_block->instructions_.first_instruction_ = cursor->GetNext();
  new_block->instructions_.last_instruction_ = instructions_.last_instruction_;
  cursor->next_->previous_ = nullptr;
  cursor->next_ = nullptr;
  instructions_.last_instruction_ = cursor;

  new_block->instructions_.SetBlockOfInstructions(new_block);
  for (HBasicBlock* successor : GetSuccessors()) {
    successor->predecessors_[successor->GetPredecessorIndexOf(this)] = new_block;
  }
  new_block->successors_.swap(successors_);
  DCHECK(successors_.empty());

  for (HBasicBlock* dominated : GetDominatedBlocks()) {
    dominated->dominator_ = new_block;
  }
  new_block->dominated_blocks_.swap(dominated_blocks_);
  DCHECK(dominated_blocks_.empty());
  return new_block;
}

const HTryBoundary* HBasicBlock::ComputeTryEntryOfSuccessors() const {
  if (EndsWithTryBoundary()) {
    HTryBoundary* try_boundary = GetLastInstruction()->AsTryBoundary();
    if (try_boundary->IsEntry()) {
      DCHECK(!IsTryBlock());
      return try_boundary;
    } else {
      DCHECK(IsTryBlock());
      DCHECK(try_catch_information_->GetTryEntry().HasSameExceptionHandlersAs(*try_boundary));
      return nullptr;
    }
  } else if (IsTryBlock()) {
    return &try_catch_information_->GetTryEntry();
  } else {
    return nullptr;
  }
}

bool HBasicBlock::HasThrowingInstructions() const {
  for (HInstructionIteratorPrefetchNext it(GetInstructions()); !it.Done(); it.Advance()) {
    if (it.Current()->CanThrow()) {
      return true;
    }
  }
  return false;
}

static bool HasOnlyOneInstruction(const HBasicBlock& block) {
  return block.GetPhis().IsEmpty()
      && !block.GetInstructions().IsEmpty()
      && block.GetFirstInstruction() == block.GetLastInstruction();
}

bool HBasicBlock::IsSingleGoto() const {
  return HasOnlyOneInstruction(*this) && GetLastInstruction()->IsGoto();
}

bool HBasicBlock::IsSingleReturn() const {
  return HasOnlyOneInstruction(*this) && GetLastInstruction()->IsReturn();
}

bool HBasicBlock::IsSingleReturnOrReturnVoidAllowingPhis() const {
  return (GetFirstInstruction() == GetLastInstruction()) &&
         (GetLastInstruction()->IsReturn() || GetLastInstruction()->IsReturnVoid());
}

bool HBasicBlock::IsSingleTryBoundary() const {
  return HasOnlyOneInstruction(*this) && GetLastInstruction()->IsTryBoundary();
}

bool HBasicBlock::EndsWithControlFlowInstruction() const {
  return !GetInstructions().IsEmpty() && GetLastInstruction()->IsControlFlow();
}

bool HBasicBlock::EndsWithReturn() const {
  return !GetInstructions().IsEmpty() &&
      (GetLastInstruction()->IsReturn() || GetLastInstruction()->IsReturnVoid());
}

bool HBasicBlock::EndsWithIf() const {
  return !GetInstructions().IsEmpty() && GetLastInstruction()->IsIf();
}

bool HBasicBlock::EndsWithTryBoundary() const {
  return !GetInstructions().IsEmpty() && GetLastInstruction()->IsTryBoundary();
}

bool HBasicBlock::HasSinglePhi() const {
  return !GetPhis().IsEmpty() && GetFirstPhi()->GetNext() == nullptr;
}

ArrayRef<HBasicBlock* const> HBasicBlock::GetNormalSuccessors() const {
  if (EndsWithTryBoundary()) {
    // The normal-flow successor of HTryBoundary is always stored at index zero.
    DCHECK_EQ(successors_[0], GetLastInstruction()->AsTryBoundary()->GetNormalFlowSuccessor());
    return ArrayRef<HBasicBlock* const>(successors_).SubArray(0u, 1u);
  } else {
    // All successors of blocks not ending with TryBoundary are normal.
    return ArrayRef<HBasicBlock* const>(successors_);
  }
}

ArrayRef<HBasicBlock* const> HBasicBlock::GetExceptionalSuccessors() const {
  if (EndsWithTryBoundary()) {
    return ArrayRef<HBasicBlock* const>(GetSuccessors()).SubArray(1u);
  } else {
    // Blocks not ending with TryBoundary do not have exceptional successors.
    return ArrayRef<HBasicBlock* const>();
  }
}

bool HTryBoundary::HasSameExceptionHandlersAs(const HTryBoundary& other) const {
  ArrayRef<HBasicBlock* const> handlers1 = GetExceptionHandlers();
  ArrayRef<HBasicBlock* const> handlers2 = other.GetExceptionHandlers();

  size_t length = handlers1.size();
  if (length != handlers2.size()) {
    return false;
  }

  // Exception handlers need to be stored in the same order.
  for (size_t i = 0; i < length; ++i) {
    if (handlers1[i] != handlers2[i]) {
      return false;
    }
  }
  return true;
}

void HBasicBlock::DisconnectAndDelete() {
  // Dominators must be removed after all the blocks they dominate. This way
  // a loop header is removed last, a requirement for correct loop information
  // iteration.
  DCHECK(dominated_blocks_.empty());

  // The following steps gradually remove the block from all its dependants in
  // post order (b/27683071).

  // (1) Store a basic block that we'll use in step (5) to find loops to be updated.
  //     We need to do this before step (4) which destroys the predecessor list.
  HBasicBlock* loop_update_start = this;
  if (IsLoopHeader()) {
    HLoopInformation* loop_info = GetLoopInformation();
    // All other blocks in this loop should have been removed because the header
    // was their dominator.
    // Note that we do not remove `this` from `loop_info` as it is unreachable.
    DCHECK(!loop_info->IsIrreducible());
    DCHECK_EQ(loop_info->GetBlockMask().NumSetBits(), 1u);
    DCHECK_EQ(static_cast<uint32_t>(loop_info->GetBlockMask().GetHighestBitSet()), GetBlockId());
    loop_update_start = loop_info->GetPreHeader();
  }

  // (2) Disconnect the block from its successors and update their phis.
  DisconnectFromSuccessors();

  // (3) Remove instructions and phis. Instructions should have no remaining uses
  //     except in catch phis. If an instruction is used by a catch phi at `index`,
  //     remove `index`-th input of all phis in the catch block since they are
  //     guaranteed dead. Note that we may miss dead inputs this way but the
  //     graph will always remain consistent.
  RemoveCatchPhiUsesAndInstruction(/* building_dominator_tree = */ false);

  // (4) Disconnect the block from its predecessors and update their
  //     control-flow instructions.
  for (HBasicBlock* predecessor : predecessors_) {
    // We should not see any back edges as they would have been removed by step (3).
    DCHECK_IMPLIES(IsInLoop(), !GetLoopInformation()->IsBackEdge(*predecessor));

    HInstruction* last_instruction = predecessor->GetLastInstruction();
    if (last_instruction->IsTryBoundary() && !IsCatchBlock()) {
      // This block is the only normal-flow successor of the TryBoundary which
      // makes `predecessor` dead. Since DCE removes blocks in post order,
      // exception handlers of this TryBoundary were already visited and any
      // remaining handlers therefore must be live. We remove `predecessor` from
      // their list of predecessors.
      DCHECK_EQ(last_instruction->AsTryBoundary()->GetNormalFlowSuccessor(), this);
      while (predecessor->GetSuccessors().size() > 1) {
        HBasicBlock* handler = predecessor->GetSuccessors()[1];
        DCHECK(handler->IsCatchBlock());
        predecessor->RemoveSuccessor(handler);
        handler->RemovePredecessor(predecessor);
      }
    }

    predecessor->RemoveSuccessor(this);
    uint32_t num_pred_successors = predecessor->GetSuccessors().size();
    if (num_pred_successors == 1u) {
      // If we have one successor after removing one, then we must have
      // had an HIf, HPackedSwitch or HTryBoundary, as they have more than one
      // successor. Replace those with a HGoto.
      DCHECK(last_instruction->IsIf() ||
             last_instruction->IsPackedSwitch() ||
             (last_instruction->IsTryBoundary() && IsCatchBlock()));
      predecessor->RemoveInstruction(last_instruction);
      predecessor->AddInstruction(new (graph_->GetAllocator()) HGoto(last_instruction->GetDexPc()));
    } else if (num_pred_successors == 0u) {
      // The predecessor has no remaining successors and therefore must be dead.
      // We deliberately leave it without a control-flow instruction so that the
      // GraphChecker fails unless it is not removed during the pass too.
      predecessor->RemoveInstruction(last_instruction);
    } else {
      // There are multiple successors left. The removed block might be a successor
      // of a PackedSwitch which will be completely removed (perhaps replaced with
      // a Goto), or we are deleting a catch block from a TryBoundary. In either
      // case, leave `last_instruction` as is for now.
      DCHECK(last_instruction->IsPackedSwitch() ||
             (last_instruction->IsTryBoundary() && IsCatchBlock()));
    }
  }
  predecessors_.clear();

  // (5) Remove the block from all loops it is included in. Skip the inner-most
  //     loop if this is the loop header (see definition of `loop_update_start`)
  //     because the loop header's predecessor list has been destroyed in step (4).
  for (HLoopInformationOutwardIterator it(*loop_update_start); !it.Done(); it.Advance()) {
    HLoopInformation* loop_info = it.Current();
    loop_info->Remove(this);
    if (loop_info->IsBackEdge(*this)) {
      // If this was the last back edge of the loop, we deliberately leave the
      // loop in an inconsistent state and will fail GraphChecker unless the
      // entire loop is removed during the pass.
      loop_info->RemoveBackEdge(this);
    }
  }

  // (6) Disconnect from the dominator.
  dominator_->RemoveDominatedBlock(this);
  SetDominator(nullptr);

  // (7) Delete from the graph, update reverse post order.
  graph_->DeleteDeadEmptyBlock(this);
}

void HBasicBlock::DisconnectFromSuccessors(BitVectorView<const size_t> visited) {
  DCHECK_IMPLIES(visited.SizeInBits() != 0u, visited.SizeInBits() == graph_->GetBlocks().size());
  for (HBasicBlock* successor : successors_) {
    // Delete this block from the list of predecessors.
    size_t this_index = successor->GetPredecessorIndexOf(this);
    successor->predecessors_.erase(successor->predecessors_.begin() + this_index);

    if (visited.SizeInBits() != 0u && !visited.IsBitSet(successor->GetBlockId())) {
      // `successor` itself is dead. Therefore, there is no need to update its phis.
      continue;
    }

    DCHECK(!successor->predecessors_.empty());

    // Remove this block's entries in the successor's phis. Skips exceptional
    // successors because catch phi inputs do not correspond to predecessor
    // blocks but throwing instructions. They are removed in `RemoveCatchPhiUses`.
    if (!successor->IsCatchBlock()) {
      if (successor->predecessors_.size() == 1u) {
        // The successor has just one predecessor left. Replace phis with the only
        // remaining input.
        for (HInstructionIteratorPrefetchNext phi_it(successor->GetPhis()); !phi_it.Done();
             phi_it.Advance()) {
          HPhi* phi = phi_it.Current()->AsPhi();
          phi->ReplaceWith(phi->InputAt(1 - this_index));
          successor->RemovePhi(phi);
        }
      } else {
        for (HInstructionIteratorPrefetchNext phi_it(successor->GetPhis()); !phi_it.Done();
             phi_it.Advance()) {
          phi_it.Current()->AsPhi()->RemoveInputAt(this_index);
        }
      }
    }
  }
  successors_.clear();
}

void HBasicBlock::RemoveCatchPhiUsesAndInstruction(bool building_dominator_tree) {
  for (HBackwardInstructionIteratorPrefetchNext it(GetInstructions()); !it.Done(); it.Advance()) {
    HInstruction* insn = it.Current();
    RemoveCatchPhiUsesOfDeadInstruction(insn);

    // If we are building the dominator tree, we removed all input records previously.
    // `RemoveInstruction` will try to remove them again but that's not something we support and we
    // will crash. We check here since we won't be checking that in RemoveInstruction.
    if (building_dominator_tree) {
      DCHECK(insn->GetUses().empty());
      DCHECK(insn->GetEnvUses().empty());
    }
    RemoveInstruction(insn, /* ensure_safety= */ !building_dominator_tree);
  }
  for (HInstructionIteratorPrefetchNext it(GetPhis()); !it.Done(); it.Advance()) {
    HPhi* insn = it.Current()->AsPhi();
    RemoveCatchPhiUsesOfDeadInstruction(insn);

    // If we are building the dominator tree, we removed all input records previously.
    // `RemovePhi` will try to remove them again but that's not something we support and we
    // will crash. We check here since we won't be checking that in RemovePhi.
    if (building_dominator_tree) {
      DCHECK(insn->GetUses().empty());
      DCHECK(insn->GetEnvUses().empty());
    }
    RemovePhi(insn, /* ensure_safety= */ !building_dominator_tree);
  }
}

void HBasicBlock::MergeInstructionsWith(HBasicBlock* other) {
  DCHECK(EndsWithControlFlowInstruction());
  RemoveInstruction(GetLastInstruction());
  instructions_.Add(other->GetInstructions());
  other->instructions_.SetBlockOfInstructions(this);
  other->instructions_.Clear();
}

void HBasicBlock::MergeWith(HBasicBlock* other) {
  DCHECK_EQ(GetGraph(), other->GetGraph());
  DCHECK(ContainsElement(dominated_blocks_, other));
  DCHECK_EQ(GetSingleSuccessor(), other);
  DCHECK_EQ(other->GetSinglePredecessor(), this);
  DCHECK(other->GetPhis().IsEmpty());

  // Move instructions from `other` to `this`.
  MergeInstructionsWith(other);

  // Remove `other` from the loops it is included in.
  for (HLoopInformationOutwardIterator it(*other); !it.Done(); it.Advance()) {
    HLoopInformation* loop_info = it.Current();
    loop_info->Remove(other);
    if (loop_info->IsBackEdge(*other)) {
      loop_info->ReplaceBackEdge(other, this);
    }
  }

  // Update links to the successors of `other`.
  successors_.clear();
  for (HBasicBlock* successor : other->GetSuccessors()) {
    successor->predecessors_[successor->GetPredecessorIndexOf(other)] = this;
  }
  successors_.swap(other->successors_);
  DCHECK(other->successors_.empty());

  // Update the dominator tree.
  RemoveDominatedBlock(other);
  for (HBasicBlock* dominated : other->GetDominatedBlocks()) {
    dominated->SetDominator(this);
  }
  dominated_blocks_.insert(
      dominated_blocks_.end(), other->dominated_blocks_.begin(), other->dominated_blocks_.end());
  other->dominated_blocks_.clear();
  other->dominator_ = nullptr;

  // Clear the list of predecessors of `other` in preparation of deleting it.
  other->predecessors_.clear();

  // Delete `other` from the graph. The function updates reverse post order.
  graph_->DeleteDeadEmptyBlock(other);
}

void HBasicBlock::MergeWithInlined(HBasicBlock* other) {
  DCHECK_NE(GetGraph(), other->GetGraph());
  DCHECK(GetDominatedBlocks().empty());
  DCHECK(GetSuccessors().empty());
  DCHECK(!EndsWithControlFlowInstruction());
  DCHECK(other->GetGraph()->IsEntryBlock(other->GetSinglePredecessor()));
  DCHECK(other->GetPhis().IsEmpty());
  DCHECK(!other->IsInLoop());

  // Move instructions from `other` to `this`.
  instructions_.Add(other->GetInstructions());
  other->instructions_.SetBlockOfInstructions(this);

  // Update links to the successors of `other`.
  successors_.clear();
  for (HBasicBlock* successor : other->GetSuccessors()) {
    successor->predecessors_[successor->GetPredecessorIndexOf(other)] = this;
  }
  successors_.swap(other->successors_);
  DCHECK(other->successors_.empty());

  // Update the dominator tree.
  for (HBasicBlock* dominated : other->GetDominatedBlocks()) {
    dominated->SetDominator(this);
  }
  dominated_blocks_.insert(
      dominated_blocks_.end(), other->dominated_blocks_.begin(), other->dominated_blocks_.end());
  other->dominated_blocks_.clear();
  other->dominator_ = nullptr;
  other->graph_ = nullptr;
}

void HBasicBlock::ReplaceWith(HBasicBlock* other) {
  while (!GetPredecessors().empty()) {
    HBasicBlock* predecessor = GetPredecessors()[0];
    predecessor->ReplaceSuccessor(this, other);
  }
  while (!GetSuccessors().empty()) {
    HBasicBlock* successor = GetSuccessors()[0];
    successor->ReplacePredecessor(this, other);
  }
  for (HBasicBlock* dominated : GetDominatedBlocks()) {
    other->AddDominatedBlock(dominated);
  }
  GetDominator()->ReplaceDominatedBlock(this, other);
  other->SetDominator(GetDominator());
  dominator_ = nullptr;
  graph_ = nullptr;
}

static void CheckAgainstUpperBound(ReferenceTypeInfo rti, ReferenceTypeInfo upper_bound_rti) {
  if (rti.IsValid()) {
    ScopedObjectAccess soa(Thread::Current());
    DCHECK(upper_bound_rti.IsSupertypeOf(rti))
        << " upper_bound_rti: " << upper_bound_rti
        << " rti: " << rti;
    DCHECK_IMPLIES(upper_bound_rti.GetTypeHandle()->CannotBeAssignedFromOtherTypes(), rti.IsExact())
        << " upper_bound_rti: " << upper_bound_rti
        << " rti: " << rti;
  }
}

void HInstruction::SetReferenceTypeInfo(ReferenceTypeInfo rti) {
  if (kIsDebugBuild) {
    DCHECK_EQ(GetType(), DataType::Type::kReference);
    DCHECK(rti.IsValid()) << "Invalid RTI for " << DebugName();
    if (IsBoundType()) {
      // Having the test here spares us from making the method virtual just for
      // the sake of a DCHECK.
      CheckAgainstUpperBound(rti, AsBoundType()->GetUpperBound());
    }
  }
  reference_type_handle_ = rti.GetTypeHandle();
  SetPackedFlag<kFlagReferenceTypeIsExact>(rti.IsExact());
}

void HInstruction::SetReferenceTypeInfoIfValid(ReferenceTypeInfo rti) {
  if (rti.IsValid()) {
    SetReferenceTypeInfo(rti);
  }
}

bool HBoundType::InstructionDataEquals(const HInstruction* other) const {
  const HBoundType* other_bt = other->AsBoundType();
  ScopedObjectAccess soa(Thread::Current());
  return GetUpperBound().IsEqual(other_bt->GetUpperBound()) &&
         GetUpperCanBeNull() == other_bt->GetUpperCanBeNull() &&
         CanBeNull() == other_bt->CanBeNull();
}

void HBoundType::SetUpperBound(const ReferenceTypeInfo& upper_bound, bool can_be_null) {
  if (kIsDebugBuild) {
    DCHECK(upper_bound.IsValid());
    DCHECK(!upper_bound_.IsValid()) << "Upper bound should only be set once.";
    CheckAgainstUpperBound(GetReferenceTypeInfo(), upper_bound);
  }
  upper_bound_ = upper_bound;
  SetPackedFlag<kFlagUpperCanBeNull>(can_be_null);
}

bool HInstruction::HasAnyEnvironmentUseBefore(HInstruction* other) {
  // For now, assume that instructions in different blocks may use the
  // environment.
  // TODO: Use the control flow to decide if this is true.
  if (GetBlock() != other->GetBlock()) {
    return true;
  }

  // We know that we are in the same block. Walk from 'this' to 'other',
  // checking to see if there is any instruction with an environment.
  HInstruction* current = this;
  for (; current != other && current != nullptr; current = current->GetNext()) {
    // This is a conservative check, as the instruction result may not be in
    // the referenced environment.
    if (current->HasEnvironment()) {
      return true;
    }
  }

  // We should have been called with 'this' before 'other' in the block.
  // Just confirm this.
  DCHECK(current != nullptr);
  return false;
}

void HInvoke::SetIntrinsic(Intrinsics intrinsic,
                           IntrinsicNeedsEnvironment needs_env,
                           IntrinsicSideEffects side_effects,
                           IntrinsicExceptions exceptions) {
  intrinsic_ = intrinsic;
  IntrinsicOptimizations opt(this);

  // Adjust method's side effects from intrinsic table.
  switch (side_effects) {
    case kNoSideEffects: SetSideEffects(SideEffects::None()); break;
    case kReadSideEffects: SetSideEffects(SideEffects::AllReads()); break;
    case kWriteSideEffects: SetSideEffects(SideEffects::AllWrites()); break;
    case kAllSideEffects: SetSideEffects(SideEffects::AllExceptGCDependency()); break;
  }

  if (needs_env == kNoEnvironment) {
    opt.SetDoesNotNeedEnvironment();
  } else {
    // If we need an environment, that means there will be a call, which can trigger GC.
    SetSideEffects(GetSideEffects().Union(SideEffects::CanTriggerGC()));
  }
  // Adjust method's exception status from intrinsic table.
  SetCanThrow(exceptions == kCanThrow);
}

bool HNewInstance::IsStringAlloc() const {
  return GetEntrypoint() == kQuickAllocStringObject;
}

bool HInvoke::NeedsEnvironment() const {
  if (!IsIntrinsic()) {
    return true;
  }
  IntrinsicOptimizations opt(*this);
  return !opt.GetDoesNotNeedEnvironment();
}

const DexFile& HInvokeStaticOrDirect::GetDexFileForPcRelativeDexCache() const {
  ArtMethod* caller = GetEnvironment()->GetMethod();
  ScopedObjectAccess soa(Thread::Current());
  // `caller` is null for a top-level graph representing a method whose declaring
  // class was not resolved.
  return caller == nullptr ? GetBlock()->GetGraph()->GetDexFile() : *caller->GetDexFile();
}

std::ostream& operator<<(std::ostream& os, HInvokeStaticOrDirect::ClinitCheckRequirement rhs) {
  switch (rhs) {
    case HInvokeStaticOrDirect::ClinitCheckRequirement::kExplicit:
      return os << "explicit";
    case HInvokeStaticOrDirect::ClinitCheckRequirement::kImplicit:
      return os << "implicit";
    case HInvokeStaticOrDirect::ClinitCheckRequirement::kNone:
      return os << "none";
  }
}

bool HInvokeStaticOrDirect::CanBeNull() const {
  if (IsStringInit()) {
    return false;
  }
  return HInvoke::CanBeNull();
}

bool HInvoke::CanBeNull() const {
  switch (GetIntrinsic()) {
    case Intrinsics::kThreadCurrentThread:
    case Intrinsics::kStringBufferAppend:
    case Intrinsics::kStringBufferToString:
    case Intrinsics::kStringBuilderAppendObject:
    case Intrinsics::kStringBuilderAppendString:
    case Intrinsics::kStringBuilderAppendCharSequence:
    case Intrinsics::kStringBuilderAppendCharArray:
    case Intrinsics::kStringBuilderAppendBoolean:
    case Intrinsics::kStringBuilderAppendChar:
    case Intrinsics::kStringBuilderAppendInt:
    case Intrinsics::kStringBuilderAppendLong:
    case Intrinsics::kStringBuilderAppendFloat:
    case Intrinsics::kStringBuilderAppendDouble:
    case Intrinsics::kStringBuilderToString:
#define DEFINE_BOXED_CASE(name, unused1, unused2, unused3, unused4) \
    case Intrinsics::k##name##ValueOf:
    BOXED_TYPES(DEFINE_BOXED_CASE)
#undef DEFINE_BOXED_CASE
      return false;
    default:
      return GetType() == DataType::Type::kReference;
  }
}

bool HInvokeVirtual::CanDoImplicitNullCheckOn(HInstruction* obj) const {
  if (obj != InputAt(0)) {
    return false;
  }
  switch (GetIntrinsic()) {
    case Intrinsics::kNone:
      return true;
    case Intrinsics::kReferenceRefersTo:
      return true;
    default:
      // TODO: Add implicit null checks in more intrinsics.
      return false;
  }
}

bool HLoadClass::InstructionDataEquals(const HInstruction* other) const {
  const HLoadClass* other_load_class = other->AsLoadClass();
  // TODO: To allow GVN for HLoadClass from different dex files, we should compare the type
  // names rather than type indexes. However, we shall also have to re-think the hash code.
  if (type_index_ != other_load_class->type_index_ ||
      GetPackedFields() != other_load_class->GetPackedFields()) {
    return false;
  }
  switch (GetLoadKind()) {
    case LoadKind::kBootImageRelRo:
    case LoadKind::kJitBootImageAddress:
    case LoadKind::kJitTableAddress: {
      ScopedObjectAccess soa(Thread::Current());
      return GetClass().Get() == other_load_class->GetClass().Get();
    }
    default:
      DCHECK(HasTypeReference(GetLoadKind()));
      return IsSameDexFile(GetDexFile(), other_load_class->GetDexFile());
  }
}

bool HLoadString::InstructionDataEquals(const HInstruction* other) const {
  const HLoadString* other_load_string = other->AsLoadString();
  // TODO: To allow GVN for HLoadString from different dex files, we should compare the strings
  // rather than their indexes. However, we shall also have to re-think the hash code.
  if (string_index_ != other_load_string->string_index_ ||
      GetPackedFields() != other_load_string->GetPackedFields()) {
    return false;
  }
  switch (GetLoadKind()) {
    case LoadKind::kBootImageRelRo:
    case LoadKind::kJitBootImageAddress:
    case LoadKind::kJitTableAddress: {
      ScopedObjectAccess soa(Thread::Current());
      return GetString().Get() == other_load_string->GetString().Get();
    }
    default:
      return IsSameDexFile(GetDexFile(), other_load_string->GetDexFile());
  }
}

void HInstruction::RemoveEnvironmentUsers() {
  for (const HUseListNode<HEnvironment*>& use : GetEnvUses()) {
    HEnvironment* user = use.GetUser();
    user->SetRawEnvAt(use.GetIndex(), nullptr);
  }
  env_uses_.clear();
}

HInstruction* ReplaceInstrOrPhiByClone(HInstruction* instr) {
  HInstruction* clone = instr->Clone(instr->GetBlock()->GetGraph()->GetAllocator());
  HBasicBlock* block = instr->GetBlock();

  if (instr->IsPhi()) {
    HPhi* phi = instr->AsPhi();
    DCHECK(!phi->HasEnvironment());
    HPhi* phi_clone = clone->AsPhi();
    block->ReplaceAndRemovePhiWith(phi, phi_clone);
  } else {
    block->ReplaceAndRemoveInstructionWith(instr, clone);
    if (instr->HasEnvironment()) {
      clone->CopyEnvironmentFrom(instr->GetEnvironment());
      HLoopInformation* loop_info = block->GetLoopInformation();
      if (instr->IsSuspendCheck() && loop_info != nullptr) {
        loop_info->SetSuspendCheck(clone->AsSuspendCheck());
      }
    }
  }
  return clone;
}

std::ostream& operator<<(std::ostream& os, const MoveOperands& rhs) {
  os << "["
     << " source=" << rhs.GetSource()
     << " destination=" << rhs.GetDestination()
     << " type=" << rhs.GetType()
     << " instruction=";
  if (rhs.GetInstruction() != nullptr) {
    os << rhs.GetInstruction()->DebugName() << ' ' << rhs.GetInstruction()->GetId();
  } else {
    os << "null";
  }
  os << " ]";
  return os;
}

std::ostream& operator<<(std::ostream& os, TypeCheckKind rhs) {
  switch (rhs) {
    case TypeCheckKind::kUnresolvedCheck:
      return os << "unresolved_check";
    case TypeCheckKind::kExactCheck:
      return os << "exact_check";
    case TypeCheckKind::kClassHierarchyCheck:
      return os << "class_hierarchy_check";
    case TypeCheckKind::kAbstractClassCheck:
      return os << "abstract_class_check";
    case TypeCheckKind::kInterfaceCheck:
      return os << "interface_check";
    case TypeCheckKind::kArrayObjectCheck:
      return os << "array_object_check";
    case TypeCheckKind::kArrayCheck:
      return os << "array_check";
    case TypeCheckKind::kBitstringCheck:
      return os << "bitstring_check";
  }
}

// Check that intrinsic enum values fit within space set aside in ArtMethod modifier flags.
#define CHECK_INTRINSICS_ENUM_VALUES(Name, InvokeType, _, SideEffects, Exceptions, ...) \
  static_assert( \
    static_cast<uint32_t>(Intrinsics::k ## Name) <= (kAccIntrinsicBits >> CTZ(kAccIntrinsicBits)), \
    "Intrinsics enumeration space overflow.");
  ART_INTRINSICS_LIST(CHECK_INTRINSICS_ENUM_VALUES)
#undef CHECK_INTRINSICS_ENUM_VALUES

// Function that returns whether an intrinsic needs an environment or not.
static inline IntrinsicNeedsEnvironment NeedsEnvironmentIntrinsic(Intrinsics i) {
  switch (i) {
    case Intrinsics::kNone:
      return kNeedsEnvironment;  // Non-sensical for intrinsic.
#define OPTIMIZING_INTRINSICS(Name, InvokeType, NeedsEnv, SideEffects, Exceptions, ...) \
    case Intrinsics::k ## Name: \
      return NeedsEnv;
      ART_INTRINSICS_LIST(OPTIMIZING_INTRINSICS)
#undef OPTIMIZING_INTRINSICS
  }
  return kNeedsEnvironment;
}

// Function that returns whether an intrinsic has side effects.
static inline IntrinsicSideEffects GetSideEffectsIntrinsic(Intrinsics i) {
  switch (i) {
    case Intrinsics::kNone:
      return kAllSideEffects;
#define OPTIMIZING_INTRINSICS(Name, InvokeType, NeedsEnv, SideEffects, Exceptions, ...) \
    case Intrinsics::k ## Name: \
      return SideEffects;
      ART_INTRINSICS_LIST(OPTIMIZING_INTRINSICS)
#undef OPTIMIZING_INTRINSICS
  }
  return kAllSideEffects;
}

// Function that returns whether an intrinsic can throw exceptions.
static inline IntrinsicExceptions GetExceptionsIntrinsic(Intrinsics i) {
  switch (i) {
    case Intrinsics::kNone:
      return kCanThrow;
#define OPTIMIZING_INTRINSICS(Name, InvokeType, NeedsEnv, SideEffects, Exceptions, ...) \
    case Intrinsics::k ## Name: \
      return Exceptions;
      ART_INTRINSICS_LIST(OPTIMIZING_INTRINSICS)
#undef OPTIMIZING_INTRINSICS
  }
  return kCanThrow;
}

void HInvoke::SetResolvedMethod(ArtMethod* method, bool enable_intrinsic_opt) {
  if (method != nullptr && method->IsIntrinsic() && enable_intrinsic_opt) {
    Intrinsics intrinsic = method->GetIntrinsic();
    SetIntrinsic(intrinsic,
                 NeedsEnvironmentIntrinsic(intrinsic),
                 GetSideEffectsIntrinsic(intrinsic),
                 GetExceptionsIntrinsic(intrinsic));
  }
  resolved_method_ = method;
}

bool IsGEZero(HInstruction* instruction) {
  DCHECK(instruction != nullptr);
  if (instruction->IsArrayLength()) {
    return true;
  } else if (instruction->IsMin()) {
    // Instruction MIN(>=0, >=0) is >= 0.
    return IsGEZero(instruction->InputAt(0)) &&
           IsGEZero(instruction->InputAt(1));
  } else if (instruction->IsAbs()) {
    // Instruction ABS(>=0) is >= 0.
    // NOTE: ABS(minint) = minint prevents assuming
    //       >= 0 without looking at the argument.
    return IsGEZero(instruction->InputAt(0));
  }
  int64_t value = -1;
  return IsInt64AndGet(instruction, &value) && value >= 0;
}

}  // namespace art

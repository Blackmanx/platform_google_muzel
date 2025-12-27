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

#include "ssa_liveness_analysis.h"

#include "base/arena_bit_vector.h"
#include "base/bit_vector-inl.h"
#include "base/bit_utils_iterator.h"
#include "code_generator.h"
#include "com_android_art_flags.h"
#include "linear_order.h"
#include "loop_information-inl.h"
#include "nodes.h"

namespace art HIDDEN {

inline BlockInfo::BlockInfo(ScopedArenaAllocator* allocator, size_t number_of_ssa_values)
    : live_in_(ArenaBitVector::CreateFixedSize(
          allocator, number_of_ssa_values, kArenaAllocSsaLiveness)),
      live_out_(ArenaBitVector::CreateFixedSize(
          allocator, number_of_ssa_values, kArenaAllocSsaLiveness)),
      kill_(ArenaBitVector::CreateFixedSize(
          allocator, number_of_ssa_values, kArenaAllocSsaLiveness)) {
}

void SsaLivenessAnalysis::Analyze() {
  // Compute the linear order directly in the graph's data structure
  // (there are no more following graph mutations).
  LinearizeGraph(graph_, &graph_->linear_order_);

  // Liveness analysis.
  NumberInstructions();
  ComputeLiveness();
}

// Local flags for location processing.
static constexpr uint32_t kFlagNotLeaf = 1u;
static constexpr uint32_t kFlagNeedsSuspendCheckEntry = 2u;
static constexpr uint32_t kFlagRequiresCurrentMethod = 4u;

template <bool kIsPhi>
static inline uint32_t AllocateLocations(HGraphVisitor* location_builder,
                                         HInstruction* instruction,
                                         uint32_t flags,
                                         /*out*/ HSuspendCheck** entry_suspend_check) {
  DCHECK_EQ(kIsPhi, instruction->IsPhi());
  if (kIsPhi) {
    DCHECK(instruction->GetEnvironment() == nullptr);
    location_builder->VisitPhi(instruction->AsPhi());
  } else {
    ArenaAllocator* allocator = location_builder->GetGraph()->GetAllocator();
    for (HEnvironment* env = instruction->GetEnvironment();
         env != nullptr;
         env = env->GetParent()) {
      env->AllocateLocations(allocator);
    }
    location_builder->Dispatch(instruction);
  }
  DCHECK(CodeGenerator::CheckTypeConsistency(instruction));
  if (kIsPhi) {
    DCHECK(instruction->GetLocations() != nullptr);
    DCHECK(!instruction->GetLocations()->CanCall());
    DCHECK(!instruction->NeedsCurrentMethod());
  } else if (instruction->IsSuspendCheck() &&
             location_builder->GetGraph()->IsEntryBlock(instruction->GetBlock())) {
    DCHECK(*entry_suspend_check == nullptr);  // At most one entry suspend check.
    *entry_suspend_check = instruction->AsSuspendCheck();
  } else {
    LocationSummary* locations = instruction->GetLocations();
    if (locations != nullptr) {
      if (locations->CanCall()) {
        flags |= kFlagNotLeaf | kFlagRequiresCurrentMethod;
        if (locations->NeedsSuspendCheckEntry()) {
          flags |= kFlagNeedsSuspendCheckEntry;
        }
      } else if (locations->Intrinsified() &&
                 instruction->IsInvokeStaticOrDirect() &&
                 !instruction->AsInvokeStaticOrDirect()->HasCurrentMethodInput()) {
        // A static method call that has been fully intrinsified, and cannot call on the slow
        // path or refer to the current method directly, no longer needs current method.
        return flags;
      }
    }
    if (instruction->NeedsCurrentMethod()) {
      flags |= kFlagRequiresCurrentMethod;
    }
  }
  return flags;
}

void SsaLivenessAnalysis::NumberInstructions() {
  HGraphVisitor* location_builder = codegen_->GetLocationBuilder();
  HSuspendCheck* entry_suspend_check = nullptr;
  uint32_t flags = 0u;
  size_t ssa_index = 0;
  size_t lifetime_position = 0;
  // Each instruction gets a lifetime position, and a block gets a lifetime
  // start and end position. Non-phi instructions have a distinct lifetime position than
  // the block they are in. Phi instructions have the lifetime start of their block as
  // lifetime position.
  //
  // Because the register allocator will insert moves in the graph, we need
  // to differentiate between the start and end of an instruction. Adding 2 to
  // the lifetime position for each instruction ensures the start of an
  // instruction is different than the end of the previous instruction.
  for (HBasicBlock* block : graph_->GetLinearOrder()) {
    block->SetLifetimeStart(lifetime_position);

    for (HInstructionIterator inst_it(block->GetPhis()); !inst_it.Done(); inst_it.Advance()) {
      HInstruction* current = inst_it.Current();
      flags = AllocateLocations</*kIsPhi=*/ true>(
          location_builder, current, flags, &entry_suspend_check);
      // Phis always have a valid output location, namely `Location::Any()`.
      DCHECK(current->GetLocations() != nullptr);
      DCHECK(current->GetLocations()->Out().IsValid());
      instructions_from_ssa_index_.push_back(current);
      current->SetSsaIndex(ssa_index++);
      bool is_pair = codegen_->NeedsTwoRegisters(current->GetType());
      current->SetLiveInterval(
          LiveInterval::MakeInterval(allocator_, current->GetType(), is_pair, current));
      current->SetLifetimePosition(lifetime_position);
    }
    lifetime_position += kLivenessPositionsPerInstruction;

    // Add a null marker to notify we are starting a block.
    instructions_from_lifetime_position_.push_back(nullptr);

    for (HInstructionIterator inst_it(block->GetInstructions()); !inst_it.Done();
         inst_it.Advance()) {
      HInstruction* current = inst_it.Current();
      flags = AllocateLocations</*kIsPhi=*/ false>(
          location_builder, current, flags, &entry_suspend_check);
      LocationSummary* locations = current->GetLocations();
      if (locations != nullptr && locations->Out().IsValid()) {
        instructions_from_ssa_index_.push_back(current);
        current->SetSsaIndex(ssa_index++);
        bool is_pair = codegen_->NeedsTwoRegisters(current->GetType());
        current->SetLiveInterval(
            LiveInterval::MakeInterval(allocator_, current->GetType(), is_pair, current));
      }
      instructions_from_lifetime_position_.push_back(current);
      current->SetLifetimePosition(lifetime_position);
      lifetime_position += kLivenessPositionsPerInstruction;
    }

    block->SetLifetimeEnd(lifetime_position);
  }
  DCHECK_EQ(GetNumberOfSsaValues(), ssa_index);

  DCHECK(codegen_->IsLeafMethod());  // Initial value.
  codegen_->SetIsLeaf((flags & kFlagNotLeaf) == 0u);
  // Update current method requirement.
  DCHECK_EQ(codegen_->RequiresCurrentMethod(), codegen_->GetGraph()->IsCompilingBaseline());
  codegen_->SetRequiresCurrentMethod(
      codegen_->RequiresCurrentMethod() || (flags & kFlagRequiresCurrentMethod) != 0u);
  if ((flags & kFlagNeedsSuspendCheckEntry) == 0u && entry_suspend_check != nullptr) {
    // We do this here because we do not want the suspend check to artificially create live
    // registers. This is the earliest point where we know the suspend chech is not required.
    DCHECK_EQ(entry_suspend_check->GetLocations()->GetTempCount(), 0u);
    entry_suspend_check->GetBlock()->RemoveInstruction(entry_suspend_check);
  }
}

void SsaLivenessAnalysis::ComputeLiveness() {
  size_t number_of_ssa_values = GetNumberOfSsaValues();
  for (HBasicBlock* block : graph_->GetLinearOrder()) {
    block_infos_[block->GetBlockId()] =
        new (allocator_) BlockInfo(allocator_, number_of_ssa_values);
  }

  // Compute the live ranges, as well as the initial live_in, live_out, and kill sets.
  // This method does not handle backward branches for the sets, therefore live_in
  // and live_out sets are not yet correct.
  ComputeLiveRanges();

  // Do a fixed point calculation to take into account backward branches,
  // that will update live_in of loop headers, and therefore live_out and live_in
  // of blocks in the loop.
  ComputeLiveInAndLiveOutSets();
}

void SsaLivenessAnalysis::RecursivelyProcessInputs(HInstruction* current,
                                                   HBasicBlock* block,
                                                   HInstruction* actual_user,
                                                   BitVectorView<size_t> live_in) {
  DCHECK(current->GetBlock() == block);
  HInputsRef inputs = current->GetInputs();
  for (size_t i = 0; i < inputs.size(); ++i) {
    HInstruction* input = inputs[i];
    bool has_in_location = current->GetLocations()->InAt(i).IsValid();
    bool has_out_location = input->GetLocations()->Out().IsValid();

    if (has_in_location) {
      DCHECK(has_out_location)
          << "Instruction " << current->DebugName() << current->GetId()
          << " expects an input value at index " << i << " but "
          << input->DebugName() << input->GetId() << " does not produce one.";
      DCHECK(input->HasSsaIndex());
      // `input` generates a result used by `current`. Add use and update
      // the live-in set.
      input->GetLiveInterval()->AddUse</*kEnvironmentUse=*/ false>(
          current, block, /* environment= */ nullptr, i, actual_user);
      live_in.SetBit(input->GetSsaIndex());
    } else if (has_out_location) {
      // `input` generates a result but it is not used by `current`.
    } else {
      // `input` is inlined into `current`. Walk over its inputs and record
      // uses at `current`.
      DCHECK(input->IsEmittedAtUseSite());
      // Check that the inlined input is not a phi. Recursing on loop phis could
      // lead to an infinite loop.
      DCHECK(!input->IsPhi());
      DCHECK(!input->HasEnvironment());
      RecursivelyProcessInputs(input, input->GetBlock(), actual_user, live_in);
    }
  }
}

inline bool SsaLivenessAnalysis::ShouldAllBeLiveForEnvironment(HInstruction* env_holder,
                                                               HGraph* graph) {
  // A value that's not live in compiled code may still be needed in interpreter,
  // due to code motion, etc.
  if (env_holder->IsDeoptimize()) {
    return true;
  }

  // A value live at a throwing instruction in a try block may be copied by
  // the exception handler to its location at the top of the catch block.
  if (env_holder->CanThrowIntoCatchBlock()) {
    return true;
  }

  // For debuggable graphs, we keep all values live.
  if (graph->IsDebuggable()) {
    return true;
  }

  // When compiling in OSR mode, all loops in the compiled method may be entered
  // from the interpreter via SuspendCheck; thus we need to preserve the environment.
  if (env_holder->IsSuspendCheck() && graph->IsCompilingOsr()) {
    return true;
  }

  return false;
}

inline bool SsaLivenessAnalysis::ShouldBeLiveForEnvironment(HInstruction* instruction,
                                                            bool is_dead_reference_safe) {
  DCHECK(instruction != nullptr);
  if (is_dead_reference_safe) {
    return false;
  }
  return instruction->GetType() == DataType::Type::kReference;
}

void SsaLivenessAnalysis::ProcessEnvironment(HInstruction* current,
                                             HBasicBlock* block,
                                             HInstruction* actual_user,
                                             BitVectorView<size_t> live_in) {
  DCHECK(current->GetBlock() == block);
  if (current->GetEnvironment() == nullptr) {
    return;
  }
  // Handle environment uses. See statements (b), (c) and (d) of the SsaLivenessAnalysis.
  auto process_environment = [&](auto should_be_live) ALWAYS_INLINE {
    for (HEnvironment* environment = current->GetEnvironment();
         environment != nullptr;
         environment = environment->GetParent()) {
      for (size_t i = 0, e = environment->Size(); i < e; ++i) {
        HInstruction* instruction = environment->GetInstructionAt(i);
        if (instruction == nullptr) {
          continue;
        }
        // If this environment use does not keep the instruction live, it does not
        // affect the live range of that instruction.
        if (should_be_live(instruction)) {
          CHECK(instruction->HasSsaIndex()) << instruction->DebugName();
          live_in.SetBit(instruction->GetSsaIndex());
          instruction->GetLiveInterval()->AddUse</*kEnvironmentUse=*/ true>(
              current, block, environment, i, actual_user);
        }
      }
    }
  };
  if (ShouldAllBeLiveForEnvironment(current, block->GetGraph())) {
    process_environment([]([[maybe_unused]] HInstruction* instruction) { return true; });
  } else if (block->GetGraph()->IsDeadReferenceSafe()) {
    // Nothing to do. In debug build check that `ShouldBeLiveForEnvironment()` is always false.
    if (kIsDebugBuild) {
      process_environment([](HInstruction* instruction) {
        CHECK(!ShouldBeLiveForEnvironment(instruction, /*is_dead_reference_safe=*/ true));
        return false;
      });
    }
  } else {
    process_environment([](HInstruction* instruction) {
      return ShouldBeLiveForEnvironment(instruction, /*is_dead_reference_safe=*/ false);
    });
  }
}

void SsaLivenessAnalysis::ComputeLiveRanges() {
  // Do a post order visit, adding inputs of instructions live in the block where
  // that instruction is defined, and killing instructions that are being visited.
  for (HBasicBlock* block : ReverseRange(graph_->GetLinearOrder())) {
    BitVectorView kill = GetKillSet(*block);
    BitVectorView live_in = GetLiveInSet(*block);

    // Set phi inputs of successors of this block corresponding to this block
    // as live_in.
    for (HBasicBlock* successor : block->GetSuccessors()) {
      live_in.Union(GetLiveInSet(*successor));
      if (successor->IsCatchBlock()) {
        // Inputs of catch phis will be kept alive through their environment
        // uses, allowing the runtime to copy their values to the corresponding
        // catch phi spill slots when an exception is thrown.
        // The only instructions which may not be recorded in the environments
        // are constants created by the SSA builder as typed equivalents of
        // untyped constants from the bytecode, or phis with only such constants
        // as inputs (verified by GraphChecker). Their raw binary value must
        // therefore be the same and we only need to keep alive one.
      } else {
        size_t phi_input_index = successor->GetPredecessorIndexOf(block);
        for (HInstructionIteratorPrefetchNext phi_it(successor->GetPhis()); !phi_it.Done();
             phi_it.Advance()) {
          HInstruction* phi = phi_it.Current();
          HInstruction* input = phi->InputAt(phi_input_index);
          if (com::android::art::flags::reg_alloc_spill_slot_reuse() &&
              input->GetLiveInterval()->GetUses().empty()) {
            // If the `input` has no recorded uses yet, the `phi` use shall be its last use
            // (we visit blocks in reverse linear order) and the `input` dies at the end of
            // the `block`. Record the `phi` interval as a hint to try using the same spill
            // slot in order to avoid excessive moves if both `input` and `phi` get spilled.
            input->GetLiveInterval()->SetHintPhiInterval(phi->GetLiveInterval());
          }
          input->GetLiveInterval()->AddPhiUse(phi, phi_input_index, block);
          // A phi input whose last user is the phi dies at the end of the predecessor block,
          // and not at the phi's lifetime position.
          live_in.SetBit(input->GetSsaIndex());
        }
      }
    }

    // Add a range that covers this block to all instructions live_in because of successors.
    // Instructions defined in this block will have their start of the range adjusted.
    for (uint32_t idx : live_in.Indexes()) {
      HInstruction* current = GetInstructionFromSsaIndex(idx);
      current->GetLiveInterval()->AddRange(block->GetLifetimeStart(), block->GetLifetimeEnd());
    }

    for (HBackwardInstructionIteratorPrefetchNext back_it(block->GetInstructions());
         !back_it.Done();
         back_it.Advance()) {
      HInstruction* current = back_it.Current();
      if (current->HasSsaIndex()) {
        // Kill the instruction and shorten its interval.
        kill.SetBit(current->GetSsaIndex());
        live_in.ClearBit(current->GetSsaIndex());
        current->GetLiveInterval()->SetFrom(current->GetLifetimePosition());
      }

      // Process inputs of instructions.
      if (current->IsEmittedAtUseSite()) {
        if (kIsDebugBuild) {
          CHECK(!current->GetLocations()->Out().IsValid());
          CHECK(!current->HasEnvironmentUses());
          if (current->IsNullCheck()) {
            // Implicit null check is replaced by its input in all users before register
            // allocation, so it does not have any uses at this point.
            CHECK(current->GetUses().empty());
          } else {
            // TODO: Should we allow dead instructions marked as "emitted at use site"?
            CHECK(!current->GetUses().empty());
            for (const HUseListNode<HInstruction*>& use : current->GetUses()) {
              HInstruction* user = use.GetUser();
              size_t index = use.GetIndex();
              CHECK(!user->GetLocations()->InAt(index).IsValid());
            }
            if (!current->GetUses().HasExactlyOneElement()) {
              // If there is more than one user, there can be no unallocated locations.
              // We do not have a way to record different locations for different use sites.
              for (size_t i : Range(current->GetLocations()->GetInputCount())) {
                CHECK(!current->GetLocations()->InAt(i).IsUnallocated());
              }
              for (size_t i : Range(current->GetLocations()->GetTempCount())) {
                CHECK(!current->GetLocations()->GetTemp(i).IsUnallocated());
              }
            }
          }
        }
      } else {
        // Process the environment first, because we know their uses come after
        // or at the same liveness position of inputs.
        ProcessEnvironment(current, block, current, live_in);

        // Special case implicit null checks. We want their environment uses to be
        // emitted at the instruction doing the actual null check.
        HNullCheck* check = current->GetImplicitNullCheck();
        if (check != nullptr) {
          ProcessEnvironment(check, block, current, live_in);
        }
        RecursivelyProcessInputs(current, block, current, live_in);
      }
    }

    // Kill phis defined in this block.
    for (HInstructionIteratorPrefetchNext inst_it(block->GetPhis()); !inst_it.Done();
         inst_it.Advance()) {
      HInstruction* current = inst_it.Current();
      if (current->HasSsaIndex()) {
        kill.SetBit(current->GetSsaIndex());
        live_in.ClearBit(current->GetSsaIndex());
        LiveInterval* interval = current->GetLiveInterval();
        DCHECK((interval->GetFirstRange() == nullptr)
               || (interval->GetStart() == current->GetLifetimePosition()));
        interval->SetFrom(current->GetLifetimePosition());
      }
    }

    if (block->IsLoopHeader()) {
      if (kIsDebugBuild) {
        CheckNoLiveInIrreducibleLoop(*block);
      }
      size_t last_position = GetLoopLifetimeEnd(block->GetLoopInformation());
      // For all live_in instructions at the loop header, we need to create a range
      // that covers the full loop.
      for (uint32_t idx : live_in.Indexes()) {
        HInstruction* current = GetInstructionFromSsaIndex(idx);
        current->GetLiveInterval()->AddLoopRange(block->GetLifetimeStart(), last_position);
      }
    }
  }
}

void SsaLivenessAnalysis::ComputeLiveInAndLiveOutSets() {
  bool changed;
  do {
    changed = false;

    for (const HBasicBlock* block : graph_->GetPostOrder()) {
      // The live_in set depends on the kill set (which does not
      // change in this loop), and the live_out set.  If the live_out
      // set does not change, there is no need to update the live_in set.
      if (UpdateLiveOut(*block) && UpdateLiveIn(*block)) {
        if (kIsDebugBuild) {
          CheckNoLiveInIrreducibleLoop(*block);
        }
        changed = true;
      }
    }
  } while (changed);
}

bool SsaLivenessAnalysis::UpdateLiveOut(const HBasicBlock& block) {
  BitVectorView<size_t> live_out = GetLiveOutSet(block);
  bool changed = false;
  // The live_out set of a block is the union of live_in sets of its successors.
  for (HBasicBlock* successor : block.GetSuccessors()) {
    if (live_out.Union(GetLiveInSet(*successor))) {
      changed = true;
    }
  }
  return changed;
}

bool SsaLivenessAnalysis::UpdateLiveIn(const HBasicBlock& block) {
  BitVectorView<size_t> live_out = GetLiveOutSet(block);
  BitVectorView<size_t> kill = GetKillSet(block);
  BitVectorView<size_t> live_in = GetLiveInSet(block);
  // If live_out is updated (because of backward branches), we need to make
  // sure instructions in live_out are also in live_in, unless they are killed
  // by this block.
  return live_in.UnionIfNotIn(live_out, kill);
}

void SsaLivenessAnalysis::DoCheckNoLiveInIrreducibleLoop(const HBasicBlock& block) const {
  DCHECK(block.IsLoopHeader());
  DCHECK(block.GetLoopInformation()->IsIrreducible());
  BitVectorView<size_t> live_in = GetLiveInSet(block);
  // To satisfy our liveness algorithm, we need to ensure loop headers of
  // irreducible loops do not have any live-in instructions, except constants
  // and the current method, which can be trivially re-materialized.
  for (uint32_t idx : live_in.Indexes()) {
    HInstruction* instruction = GetInstructionFromSsaIndex(idx);
    DCHECK(graph_->IsEntryBlock(instruction->GetBlock())) << instruction->DebugName();
    DCHECK(!instruction->IsParameterValue());
    DCHECK(instruction->IsCurrentMethod() || instruction->IsConstant())
        << instruction->DebugName();
  }
}

size_t SsaLivenessAnalysis::GetLoopLifetimeEnd(const HLoopInformation* loop_info) {
  DCHECK(loop_info != nullptr);
  size_t last_position = 0;
  for (HBasicBlock* back_edge : loop_info->GetBackEdges()) {
    last_position = std::max(back_edge->GetLifetimeEnd(), last_position);
  }
  return last_position;
}

template <bool kEnvironmentUse>
ALWAYS_INLINE inline void LiveInterval::AddUse(HInstruction* instruction,
                                               HBasicBlock* block,
                                               HEnvironment* environment,
                                               size_t input_index,
                                               HInstruction* actual_user) {
  DCHECK(instruction->GetBlock() == block);
  DCHECK_EQ(kEnvironmentUse, environment != nullptr);
  DCHECK(actual_user != nullptr);

  // Set the use within the instruction.
  size_t position = actual_user->GetLifetimePosition() + kLivenessPositionOfNormalUse;
  if (!kEnvironmentUse) {
    LocationSummary* locations = instruction->GetLocations();
    if (locations->IsFixedInput(input_index) || locations->OutputUsesSameAs(input_index)) {
      // For fixed inputs and output same as input, the register allocator
      // requires to have inputs die at the instruction, so that input moves use the
      // location of the input just before that instruction (and not potential moves due
      // to splitting).
      DCHECK_EQ(instruction, actual_user);
      position = actual_user->GetLifetimePosition();
    } else if (!locations->InAt(input_index).IsValid()) {
      return;
    }
  }

  if (!kEnvironmentUse && block->IsInLoop()) {
    AddBackEdgeUses(*block);
  }

  if (kEnvironmentUse) {
    DCHECK(env_uses_.empty() || position <= env_uses_.front().GetPosition());
    DCHECK(uses_.empty() || position < uses_.front().GetPosition());
    EnvUsePosition* new_env_use =
        new (allocator_) EnvUsePosition(environment, input_index, position);
    env_uses_.push_front(*new_env_use);
  } else if (!uses_.empty() && uses_.front().GetPosition() < position) {
    DCHECK(uses_.front().GetUser() == actual_user);
    // The user uses the instruction multiple times, and one use dies before the other.
    // We update the use list so that the latter is first.
    DCHECK(uses_.front().GetPosition() + kLivenessPositionOfNormalUse == position);
    UsePositionList::iterator next_pos = uses_.begin();
    UsePositionList::iterator insert_pos;
    do {
      insert_pos = next_pos;
      ++next_pos;
    } while (next_pos != uses_.end() && next_pos->GetPosition() < position);
    UsePosition* new_use = new (allocator_) UsePosition(instruction, input_index, position);
    uses_.insert_after(insert_pos, *new_use);
    if (first_range_->GetEnd() == uses_.front().GetPosition()) {
      first_range_->end_ = position;
    }
    return;
  } else {
    DCHECK(uses_.empty() || position <= uses_.front().GetPosition());
    UsePosition* new_use = new (allocator_) UsePosition(instruction, input_index, position);
    uses_.push_front(*new_use);
  }

  size_t start_block_position = block->GetLifetimeStart();
  if (first_range_ == nullptr) {
    // First time we see a use of that interval.
    first_range_ = last_range_ = range_search_start_ =
        new (allocator_) LiveRange(start_block_position, position, nullptr);
  } else if (first_range_->GetStart() == start_block_position) {
    // There is a use later in the same block or in a following block.
    // Note that in such a case, `AddRange` for the whole blocks has been called
    // before arriving in this method, and this is the reason the start of
    // `first_range_` is before the given `position`.
    DCHECK_LE(position, first_range_->GetEnd());
  } else {
    DCHECK(first_range_->GetStart() > position);
    // There is a hole in the interval. Create a new range.
    // Note that the start of `first_range_` can be equal to `end`: two blocks
    // having adjacent lifetime positions are not necessarily
    // predecessor/successor. When two blocks are predecessor/successor, the
    // liveness algorithm has called `AddRange` before arriving in this method,
    // and the check line 205 would succeed.
    first_range_ = range_search_start_ =
        new (allocator_) LiveRange(start_block_position, position, first_range_);
  }
}

LiveInterval* LiveInterval::SplitAt(size_t position) {
  DCHECK(!IsTemp());
  DCHECK(!IsFixed());
  DCHECK_GT(position, GetStart());

  if (GetEnd() <= position) {
    // This range dies before `position`, no need to split.
    return nullptr;
  }

  LiveInterval* new_interval =
      new (allocator_) LiveInterval(allocator_, type_, is_pair_, defined_by_);

  new_interval->next_sibling_ = next_sibling_;
  next_sibling_ = new_interval;
  new_interval->parent_ = parent_;

  LiveRange* current = first_range_;
  LiveRange* previous = nullptr;
  // Iterate over the ranges, and either find a range that covers this position, or
  // two ranges in between this position (that is, the position is in a lifetime hole).
  do {
    if (position >= current->GetEnd()) {
      // Move to next range.
      previous = current;
      current = current->next_;
    } else if (position <= current->GetStart()) {
      // If the previous range did not cover this position, we know position is in
      // a lifetime hole. We can just break the first_range_ and last_range_ links
      // and return the new interval.
      DCHECK(previous != nullptr);
      DCHECK(current != first_range_);
      new_interval->last_range_ = last_range_;
      last_range_ = previous;
      previous->next_ = nullptr;
      new_interval->first_range_ = current;
      if (range_search_start_ != nullptr && range_search_start_->GetEnd() >= current->GetEnd()) {
        // Search start point is inside `new_interval`. Change it to null
        // (i.e. the end of the interval) in the original interval.
        range_search_start_ = nullptr;
      }
      new_interval->range_search_start_ = new_interval->first_range_;
      return new_interval;
    } else {
      // This range covers position. We create a new `first_range_` for the `new_interval`
      // that covers the range from the `position`. We also shorten the current
      // range and make it the last range of this interval.
      // Note: We must not do it the other way around as the `current` range may be cached
      // as the search start position by the register allocator and updating its start
      // position can break certain invariants.
      DCHECK(position < current->GetEnd() && position > current->GetStart());
      new_interval->first_range_ =
          new (allocator_) LiveRange(position, current->end_, current->next_);
      new_interval->range_search_start_ = new_interval->first_range_;
      new_interval->last_range_ =
          (last_range_ != current) ? last_range_ : new_interval->first_range_;
      current->next_ = nullptr;
      current->end_ = position;
      last_range_ = current;
      if (range_search_start_ != nullptr && range_search_start_->GetEnd() >= position) {
        // Search start point is inside `new_interval`. Change it to `last_range`
        // in the original interval. This is conservative but always correct.
        range_search_start_ = last_range_;
      }
      return new_interval;
    }
  } while (current != nullptr);

  LOG(FATAL) << "Unreachable";
  return nullptr;
}

void LiveInterval::Dump(std::ostream& stream) const {
  stream << "ranges: { ";
  LiveRange* current = first_range_;
  while (current != nullptr) {
    current->Dump(stream);
    stream << " ";
    current = current->GetNext();
  }
  stream << "}, uses: { ";
  for (const UsePosition& use : GetUses()) {
    use.Dump(stream);
    stream << " ";
  }
  stream << "}, { ";
  for (const EnvUsePosition& env_use : GetEnvironmentUses()) {
    env_use.Dump(stream);
    stream << " ";
  }
  stream << "}";
  stream << " fixed:" << is_fixed_ << " temp:" << IsTemp() << " split:" << IsSplit();
  stream << " pair:" << IsPair();
}

bool LiveInterval::SameRegisterKind(Location other) const {
  if (IsFloatingPoint()) {
    if (IsPair()) {
      return other.IsFpuRegisterPair();
    } else {
      return other.IsFpuRegister();
    }
  } else {
    if (IsPair()) {
      return other.IsCoreRegisterPair();
    } else {
      return other.IsCoreRegister();
    }
  }
}

size_t LiveInterval::NumberOfSpillSlotsNeeded() const {
  DCHECK(!IsTemp());
  DCHECK(!IsFixed());
  // For a SIMD operation, compute the number of needed spill slots.
  // TODO: do through vector type?
  HInstruction* definition = GetDefinedBy();
  DCHECK(definition != nullptr);
  if (HVecOperation::ReturnsSIMDValue(definition)) {
    if (definition->IsPhi()) {
      definition = definition->InputAt(1);  // SIMD always appears on back-edge
    }
    return definition->AsVecOperation()->GetVectorNumberOfBytes() / kVRegSize;
  }
  // Return number of needed spill slots based on type.
  return (type_ == DataType::Type::kInt64 || type_ == DataType::Type::kFloat64) ? 2 : 1;
}

LiveInterval* LiveInterval::GetSiblingAt(size_t position) {
  LiveInterval* current = this;
  while (current != nullptr && !current->IsDefinedAt(position)) {
    current = current->GetNextSibling();
  }
  return current;
}

void LiveInterval::AddBackEdgeUses(const HBasicBlock& block_at_use) {
  DCHECK(block_at_use.IsInLoop());
  if (block_at_use.GetGraph()->HasIrreducibleLoops()) {
    // Linear order may not be well formed when irreducible loops are present,
    // i.e. loop blocks may not be adjacent and a back edge may not be last,
    // which violates assumptions made in this method.
    return;
  }

  // Add synthesized uses at the back edge of loops to help the register allocator.
  // Note that this method is called in decreasing liveness order, to facilitate adding
  // uses at the head of the `uses_` list. Because below
  // we iterate from inner-most to outer-most, which is in increasing liveness order,
  // we need to add subsequent entries after the last inserted entry.
  const UsePositionList::iterator old_begin = uses_.begin();
  UsePositionList::iterator insert_pos = uses_.before_begin();
  for (HLoopInformationOutwardIterator it(block_at_use); !it.Done(); it.Advance()) {
    HLoopInformation* current = it.Current();
    if (GetDefinedBy()->GetLifetimePosition() >= current->GetHeader()->GetLifetimeStart()) {
      // This interval is defined in the loop. We can stop going outward.
      break;
    }

    // We're only adding a synthesized use at the last back edge. Adding synthesized uses on
    // all back edges is not necessary: anything used in the loop will have its use at the
    // last back edge. If we want branches in a loop to have better register allocation than
    // another branch, then it is the linear order we should change.
    size_t back_edge_use_position = SsaLivenessAnalysis::GetLoopLifetimeEnd(current);
    if ((old_begin != uses_.end()) && (old_begin->GetPosition() <= back_edge_use_position)) {
      // There was a use already seen in this loop. Therefore the previous call to `AddUse`
      // already inserted the backedge use. We can stop going outward.
      DCHECK(HasSynthesizeUseAt(back_edge_use_position));
      break;
    }

    DCHECK(insert_pos != uses_.before_begin()
           ? back_edge_use_position > insert_pos->GetPosition()
           : current == block_at_use.GetLoopInformation())
        << std::distance(uses_.before_begin(), insert_pos);

    UsePosition* new_use = new (allocator_) UsePosition(back_edge_use_position);
    insert_pos = uses_.insert_after(insert_pos, *new_use);
  }
}

}  // namespace art

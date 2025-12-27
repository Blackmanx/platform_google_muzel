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

#ifndef ART_COMPILER_OPTIMIZING_SSA_LIVENESS_ANALYSIS_H_
#define ART_COMPILER_OPTIMIZING_SSA_LIVENESS_ANALYSIS_H_

#include <iostream>

#include "base/array_ref.h"
#include "base/bit_vector.h"
#include "base/intrusive_forward_list.h"
#include "base/iteration_range.h"
#include "base/macros.h"
#include "base/scoped_arena_allocator.h"
#include "base/scoped_arena_containers.h"
#include "nodes.h"

namespace art HIDDEN {

class CodeGenerator;
class SsaLivenessAnalysis;

static constexpr uint32_t kNoRegisters = 0u;

// Constants describing positions assigned to various data for an instruction.
//
//                              0     1     2     3     4
// temporary                    +-----------------+
// blocked                      +-----------------+
// fixed register input   ------+
// normal input           ------------+
// fixed register output                          +-----------
// overlapping output           +-----------------------------
// non-overlapping output                   +-----------------
//
// If the output is requested in the same register as first input using the
// `Location::kSameAsFirstInput`, the first input is considered used at
// position 0 even if it's not requested in a fixed register.
//
// Note: Three positions per instruction would be enough as the non-overlapping output
// can start at position 1 without any change to the results. However, we prefer to use
// a power of two for faster division.
static constexpr size_t kLivenessPositionsPerInstruction = 4u;
static constexpr size_t kLivenessPositionsForTemp = kLivenessPositionsPerInstruction - 1u;
static constexpr size_t kLivenessPositionsToBlock = kLivenessPositionsPerInstruction - 1u;
static constexpr size_t kLivenessPositionOfNormalUse = 1u;  // Inside instruction.
static constexpr size_t kLivenessPositionOfFixedOutput = kLivenessPositionsPerInstruction - 1u;
static constexpr size_t kLivenessPositionOfNonOverlappingOutput =
    com::android::art::flags::reg_alloc_no_output_overlap() ? 2u : 0u;
static constexpr size_t kLivenessPositionForMoveAfter = kLivenessPositionsPerInstruction - 1u;

class BlockInfo : public ArenaObject<kArenaAllocSsaLiveness> {
 public:
  BlockInfo(ScopedArenaAllocator* allocator, size_t number_of_ssa_values);

 private:
  BitVectorView<size_t> live_in_;
  BitVectorView<size_t> live_out_;
  BitVectorView<size_t> kill_;

  friend class SsaLivenessAnalysis;

  DISALLOW_COPY_AND_ASSIGN(BlockInfo);
};

/**
 * A live range contains the start and end of a range where an instruction or a temporary
 * is live.
 */
class LiveRange final : public ArenaObject<kArenaAllocSsaLiveness> {
 public:
  LiveRange(size_t start, size_t end, LiveRange* next) : start_(start), end_(end), next_(next) {
    DCHECK_LT(start, end);
    DCHECK(next_ == nullptr || next_->GetStart() > GetEnd());
  }

  size_t GetStart() const { return start_; }
  size_t GetEnd() const { return end_; }
  LiveRange* GetNext() const { return next_; }

  bool IntersectsWith(const LiveRange& other) const {
    return (start_ >= other.start_ && start_ < other.end_)
        || (other.start_ >= start_ && other.start_ < end_);
  }

  bool IsBefore(const LiveRange& other) const {
    return end_ <= other.start_;
  }

  void Dump(std::ostream& stream) const {
    stream << "[" << start_ << "," << end_ << ")";
  }

  LiveRange* Dup(ScopedArenaAllocator* allocator) const {
    return new (allocator) LiveRange(
        start_, end_, next_ == nullptr ? nullptr : next_->Dup(allocator));
  }

  LiveRange* GetLastRange() {
    return next_ == nullptr ? this : next_->GetLastRange();
  }

 private:
  size_t start_;
  size_t end_;
  LiveRange* next_;

  friend class LiveInterval;

  DISALLOW_COPY_AND_ASSIGN(LiveRange);
};

/**
 * A use position represents a live interval use at a given position.
 */
class UsePosition : public ArenaObject<kArenaAllocSsaLiveness>,
                    public IntrusiveForwardListNode<UsePosition> {
 public:
  UsePosition(HInstruction* user, size_t input_index, size_t position)
      : user_(user),
        input_index_(input_index),
        position_(position) {
  }

  explicit UsePosition(size_t position)
      : user_(nullptr),
        input_index_(kNoInput),
        position_(dchecked_integral_cast<uint32_t>(position)) {
  }

  size_t GetPosition() const { return position_; }

  HInstruction* GetUser() const { return user_; }

  bool IsSynthesized() const { return user_ == nullptr; }

  size_t GetInputIndex() const { return input_index_; }

  void Dump(std::ostream& stream) const {
    stream << position_;
  }

  HLoopInformation* GetLoopInformation() const {
    return user_->GetBlock()->GetLoopInformation();
  }

  bool RequiresCoreRegister() const {
    if (IsSynthesized()) return false;
    Location location = GetUser()->GetLocations()->InAt(GetInputIndex());
    return location.IsUnallocated() && location.RequiresRegisterKind();
  }

 private:
  static constexpr uint32_t kNoInput = static_cast<uint32_t>(-1);

  HInstruction* const user_;
  const size_t input_index_;
  const size_t position_;

  DISALLOW_COPY_AND_ASSIGN(UsePosition);
};
using UsePositionList = IntrusiveForwardList<UsePosition>;

/**
 * An environment use position represents a live interval for environment use at a given position.
 */
class EnvUsePosition : public ArenaObject<kArenaAllocSsaLiveness>,
                       public IntrusiveForwardListNode<EnvUsePosition> {
 public:
  EnvUsePosition(HEnvironment* environment,
                 size_t input_index,
                 size_t position)
      : environment_(environment),
        input_index_(input_index),
        position_(position) {
    DCHECK(environment != nullptr);
  }

  size_t GetPosition() const { return position_; }

  HEnvironment* GetEnvironment() const { return environment_; }
  size_t GetInputIndex() const { return input_index_; }

  void Dump(std::ostream& stream) const {
    stream << position_;
  }

 private:
  HEnvironment* const environment_;
  const size_t input_index_;
  const size_t position_;

  DISALLOW_COPY_AND_ASSIGN(EnvUsePosition);
};
using EnvUsePositionList = IntrusiveForwardList<EnvUsePosition>;

template <typename Iterator>
inline Iterator FindUseAtOrAfterPosition(Iterator first, Iterator last, size_t position) {
  using value_type = const typename Iterator::value_type;
  static_assert(std::is_same<value_type, const UsePosition>::value ||
                    std::is_same<value_type, const EnvUsePosition>::value,
                "Expecting value type UsePosition or EnvUsePosition.");
  Iterator ret = std::find_if(
      first, last, [position](const value_type& use) { return use.GetPosition() >= position; });
  // Check that the processed range is sorted. Do not check the rest of the range to avoid
  // increasing the complexity of callers from O(n) to O(n^2).
  DCHECK(std::is_sorted(
      first,
      ret,
      [](const value_type& lhs, const value_type& rhs) {
          return lhs.GetPosition() < rhs.GetPosition();
      }));
  return ret;
}

template <typename Iterator>
inline IterationRange<Iterator> FindMatchingUseRange(Iterator first,
                                                     Iterator last,
                                                     size_t position_begin,
                                                     size_t position_end) {
  Iterator begin = FindUseAtOrAfterPosition(first, last, position_begin);
  Iterator end = FindUseAtOrAfterPosition(begin, last, position_end);
  return MakeIterationRange(begin, end);
}

/**
 * An interval is a list of disjoint live ranges where an instruction is live.
 * Each instruction that has uses gets an interval.
 */
class LiveInterval : public ArenaObject<kArenaAllocSsaLiveness> {
 public:
  static LiveInterval* MakeInterval(ScopedArenaAllocator* allocator,
                                    DataType::Type type,
                                    bool is_pair,
                                    HInstruction* instruction) {
    return new (allocator) LiveInterval(allocator, type, is_pair, instruction);
  }

  static LiveInterval* MakeFixedInterval(ScopedArenaAllocator* allocator,
                                         uint32_t regs,
                                         DataType::Type type) {
    return new (allocator) LiveInterval(
        allocator, type, /*is_pair=*/ false, /*defined_by*/ nullptr, /*is_fixed=*/ true, regs);
  }

  static LiveInterval* MakeTempInterval(ScopedArenaAllocator* allocator,
                                        DataType::Type type,
                                        bool is_pair,
                                        size_t temp_index,
                                        size_t position,
                                        HInstruction* instruction) {
    int8_t checked_index = dchecked_integral_cast<int8_t>(temp_index);
    LiveInterval* temp = new (allocator) LiveInterval(allocator,
                                                      type,
                                                      is_pair,
                                                      /*defined_by*/ instruction,
                                                      /*is_fixed=*/ false,
                                                      /*regs=*/ kNoRegisters,
                                                      checked_index);
    temp->AddRange(position, position + kLivenessPositionsForTemp);
    return temp;
  }

  bool IsTemp() const {
    static_assert(kNoTempIndex < 0);
    return temp_index_ >= 0;
  }

  size_t GetTempIndex() const {
    DCHECK(IsTemp());
    return dchecked_integral_cast<size_t>(temp_index_);
  }

  bool IsFixed() const { return is_fixed_; }
  // This interval is the result of a split.
  bool IsSplit() const { return parent_ != this; }

  // Record use of an input. The use will be recorded as an environment use if `kEnvironmentUse`
  // is true (which must correspond to `environment` being non-null) and as register use otherwise.
  // The use will be recorded at `actual_user`'s lifetime position.
  template <bool kEnvironmentUse>
  void AddUse(HInstruction* instruction,
              HBasicBlock* block,
              HEnvironment* environment,
              size_t input_index,
              HInstruction* actual_user);

  void AddPhiUse(HInstruction* instruction, size_t input_index, HBasicBlock* block) {
    DCHECK(instruction->IsPhi());
    if (block->IsInLoop()) {
      AddBackEdgeUses(*block);
    }
    UsePosition* new_use =
        new (allocator_) UsePosition(instruction, input_index, block->GetLifetimeEnd());
    uses_.push_front(*new_use);
  }

  ALWAYS_INLINE void AddRange(size_t start, size_t end) {
    if (first_range_ == nullptr) {
      first_range_ = last_range_ = range_search_start_ =
          new (allocator_) LiveRange(start, end, first_range_);
    } else if (first_range_->GetStart() == end) {
      // There is a use in the following block.
      first_range_->start_ = start;
    } else if (first_range_->GetStart() == start && first_range_->GetEnd() == end) {
      DCHECK(is_fixed_);
    } else {
      DCHECK_GT(first_range_->GetStart(), end);
      // There is a hole in the interval. Create a new range.
      first_range_ = range_search_start_ = new (allocator_) LiveRange(start, end, first_range_);
    }
  }

  void AddLoopRange(size_t start, size_t end) {
    DCHECK(first_range_ != nullptr);
    DCHECK_LE(start, first_range_->GetStart());
    // Find the range that covers the positions after the loop.
    LiveRange* after_loop = first_range_;
    LiveRange* last_in_loop = nullptr;
    while (after_loop != nullptr && after_loop->GetEnd() < end) {
      DCHECK_LE(start, after_loop->GetStart());
      last_in_loop = after_loop;
      after_loop = after_loop->GetNext();
    }
    if (after_loop == nullptr) {
      // Uses are only in the loop.
      first_range_ = last_range_ = range_search_start_ =
          new (allocator_) LiveRange(start, end, nullptr);
    } else if (after_loop->GetStart() <= end) {
      first_range_ = range_search_start_ = after_loop;
      // There are uses after the loop.
      first_range_->start_ = start;
    } else {
      // The use after the loop is after a lifetime hole.
      DCHECK(last_in_loop != nullptr);
      first_range_ = range_search_start_ = last_in_loop;
      first_range_->start_ = start;
      first_range_->end_ = end;
    }
  }

  bool HasSpillSlot() const {
    static_assert(kNoSpillSlot == -1);
    return spill_slot_or_hint_ >= 0;
  }
  void SetSpillSlot(int slot) {
    DCHECK(!IsFixed());
    DCHECK(!IsTemp());
    spill_slot_or_hint_ = slot;
    DCHECK(HasSpillSlot());
  }
  int GetSpillSlot() const { return spill_slot_or_hint_; }

  bool HasSpillSlotOrHint() const {
    return spill_slot_or_hint_ != kNoSpillSlot;
  }
  bool HasSpillSlotHint() const {
    return spill_slot_or_hint_ < kNoSpillSlot;
  }
  void SetSpillSlotHint(int hint) {
    static_assert(kNoSpillSlot == -1);
    DCHECK(!HasSpillSlotOrHint());
    DCHECK_GE(hint, 0);
    spill_slot_or_hint_ = -2 - hint;
  }
  int GetSpillSlotHint() const {
    DCHECK(HasSpillSlotOrHint());
    return HasSpillSlot() ? GetSpillSlot() : -(spill_slot_or_hint_ + 2);
  }

  void SetFrom(size_t from) {
    if (first_range_ != nullptr) {
      first_range_->start_ = from;
    } else {
      // Instruction without uses.
      DCHECK(uses_.empty());
      DCHECK(from == defined_by_->GetLifetimePosition());
      // TODO: The `kLivenessPositionsPerInstruction` below looks like a bug for calls coming
      // from `RegisterAllocatorLinearScan::CheckForFixedOutput()` as the new range reaches
      // into the next instruction. However, those call always take the `first_range_ != nullptr`
      // path above. We should use another, simpler function for that.
      first_range_ = last_range_ = range_search_start_ =
          new (allocator_) LiveRange(from, from + kLivenessPositionsPerInstruction, nullptr);
    }
  }

  LiveInterval* GetParent() const { return parent_; }

  // Returns whether this interval is the parent interval, that is, the interval
  // that starts where the HInstruction is defined.
  bool IsParent() const { return parent_ == this; }

  LiveRange* GetFirstRange() const { return first_range_; }
  LiveRange* GetLastRange() const { return last_range_; }

  uint32_t GetRegisters() const { return registers_; }
  void SetRegisters(int regs) { registers_ = regs; }
  void ClearRegisters() { registers_ = kNoRegisters; }
  bool HasRegisters() const { return registers_ != kNoRegisters; }

  uint32_t GetRegisterOrLowRegister() const {
    DCHECK(HasRegisters());
    uint32_t regs = GetRegisters();
    DCHECK_EQ(IsPair(), !IsPowerOfTwo(regs));
    DCHECK_IMPLIES(IsPair(), IsPowerOfTwo(regs & (regs - 1u)));
    return CTZ(regs);
  }

  uint32_t GetHighRegister() {
    DCHECK(IsPair());
    DCHECK(HasRegisters());
    uint32_t regs = GetRegisters();
    DCHECK(!IsPowerOfTwo(regs));
    DCHECK(IsPowerOfTwo(regs & (regs - 1u)));
    return BitSizeOf<uint32_t>() - 1u - CLZ(regs);
  }

  bool IsDeadAt(size_t position) const {
    return GetEnd() <= position;
  }

  bool IsDefinedAt(size_t position) const {
    return GetStart() <= position && !IsDeadAt(position);
  }

  // Returns true if the interval contains a LiveRange covering `position`.
  // The range at or immediately after the current position of linear scan
  // is cached for better performance. If `position` can be smaller than
  // that, CoversSlow should be used instead.
  bool Covers(size_t position) {
    LiveRange* candidate = FindRangeAtOrAfter(position, range_search_start_);
    range_search_start_ = candidate;
    return (candidate != nullptr && candidate->GetStart() <= position);
  }

  // Same as Covers but always tests all ranges.
  bool CoversSlow(size_t position) const {
    LiveRange* candidate = FindRangeAtOrAfter(position, first_range_);
    return candidate != nullptr && candidate->GetStart() <= position;
  }

  // Returns the first intersection of this interval with `current`, which
  // must be the interval currently being allocated by linear scan.
  size_t FirstIntersectionWith(LiveInterval* current) const {
    // Find the first range after the start of `current`. We use the search
    // cache to improve performance.
    DCHECK(GetStart() <= current->GetStart() || IsFixed());
    LiveRange* other_range = current->first_range_;
    LiveRange* my_range = FindRangeAtOrAfter(other_range->GetStart(), range_search_start_);
    if (my_range == nullptr) {
      return kNoLifetime;
    }

    // Advance both intervals and find the first matching range start in
    // this interval.
    do {
      if (my_range->IsBefore(*other_range)) {
        my_range = my_range->GetNext();
        if (my_range == nullptr) {
          return kNoLifetime;
        }
      } else if (other_range->IsBefore(*my_range)) {
        other_range = other_range->GetNext();
        if (other_range == nullptr) {
          return kNoLifetime;
        }
      } else {
        DCHECK(my_range->IntersectsWith(*other_range));
        return std::max(my_range->GetStart(), other_range->GetStart());
      }
    } while (true);
  }

  size_t GetStart() const {
    return first_range_->GetStart();
  }

  size_t GetEnd() const {
    return last_range_->GetEnd();
  }

  size_t GetLength() const {
    return GetEnd() - GetStart();
  }

  size_t FirstRegisterUseAfter(size_t position) const {
    DCHECK(!IsTemp());

    size_t end = GetEnd();
    for (const UsePosition& use : GetUses()) {
      size_t use_position = use.GetPosition();
      if (use_position > end) {
        break;
      }
      if (use_position > position) {
        if (use.RequiresCoreRegister()) {
          return use_position;
        }
      }
    }
    return kNoLifetime;
  }

  // Returns the location of the first register use for this live interval.
  size_t FirstRegisterUse() const {
    return FirstRegisterUseAfter(GetStart());
  }

  size_t FirstUseAfter(size_t position) const {
    DCHECK(!IsTemp());
    DCHECK(!IsDefiningPosition(position));

    size_t end = GetEnd();
    for (const UsePosition& use : GetUses()) {
      size_t use_position = use.GetPosition();
      if (use_position > end) {
        break;
      }
      if (use_position > position) {
        return use_position;
      }
    }
    return kNoLifetime;
  }

  const UsePositionList& GetUses() const {
    return parent_->uses_;
  }

  const EnvUsePositionList& GetEnvironmentUses() const {
    return parent_->env_uses_;
  }

  DataType::Type GetType() const {
    return type_;
  }

  HInstruction* GetDefinedBy() const {
    DCHECK(parent_->defined_by_ == defined_by_);
    return defined_by_;
  }

  bool HasWillCallSafepoint(ArrayRef<HInstruction* const> safepoints) const {
    bool result = false;
    ForCoveredSafepoints(
        safepoints,
        GetParent()->GetNumSafepointsAfter(),
        [&result](HInstruction* safepoint) ALWAYS_INLINE {
          if (safepoint->GetLocations()->WillCall()) {
            result = true;
            return false;
          } else {
            return true;  // Continue iterating.
          }
        });
    return result;
  }

  /**
   * Split this interval at `position`. This interval is changed to:
   * [start ... position).
   *
   * The new interval covers:
   * [position ... end)
   */
  LiveInterval* SplitAt(size_t position);

  bool StartsBeforeOrAt(LiveInterval* other) const {
    return GetStart() <= other->GetStart();
  }

  bool StartsAfter(LiveInterval* other) const {
    return GetStart() > other->GetStart();
  }

  void Dump(std::ostream& stream) const;

  LiveInterval* GetNextSibling() const { return next_sibling_; }
  LiveInterval* GetLastSibling() {
    LiveInterval* result = this;
    while (result->next_sibling_ != nullptr) {
      result = result->next_sibling_;
    }
    return result;
  }

  // Returns the number of required spilling slots (measured as a multiple of the
  // Dex virtual register size `kVRegSize`).
  size_t NumberOfSpillSlotsNeeded() const;

  bool IsFloatingPoint() const {
    return type_ == DataType::Type::kFloat32 || type_ == DataType::Type::kFloat64;
  }

  // Finds the sibling that is defined at `position`.
  LiveInterval* GetSiblingAt(size_t position);

  // Returns whether `other` and `this` share the same kind of register.
  bool SameRegisterKind(Location other) const;
  bool SameRegisterKind(const LiveInterval& other) const {
    return IsFloatingPoint() == other.IsFloatingPoint();
  }

  bool IsPair() const {
    return is_pair_;
  }

  // Returns whether an interval, when it is non-split, is using
  // the same register of one of its input. This function should
  // be used only for DCHECKs.
  bool IsUsingInputRegister(uint32_t reg) const {
    if (defined_by_ != nullptr && !IsSplit()) {
      DCHECK_NE(GetRegisters() & (1u << reg), 0u);
      for (const HInstruction* input : defined_by_->GetInputs()) {
        LiveInterval* interval = input->GetLiveInterval();

        // Find the interval that covers `defined_by`_. Calls to this function
        // are made outside the linear scan, hence we need to use CoversSlow.
        while (interval != nullptr && !interval->CoversSlow(defined_by_->GetLifetimePosition())) {
          interval = interval->GetNextSibling();
        }

        // Check if both intervals have the same register of the same kind.
        if (interval != nullptr
            && interval->SameRegisterKind(*this)
            && (interval->GetRegisters() & (1u << reg)) != 0u) {
          return true;
        }
      }
    }
    return false;
  }

  // Returns whether an interval, when it is non-split, can safely use
  // the same register of one of its input. Note that this method requires
  // IsUsingInputRegister() to be true. This function should be used only
  // for DCHECKs.
  bool CanUseInputRegister(uint32_t reg) const {
    DCHECK(IsUsingInputRegister(reg));
    if (defined_by_ != nullptr && !IsSplit()) {
      DCHECK_NE(GetRegisters() & (1u << reg), 0u);
      LocationSummary* locations = defined_by_->GetLocations();
      if (locations->OutputCanOverlapWithInputs()) {
        return false;
      }
      for (const HInstruction* input : defined_by_->GetInputs()) {
        LiveInterval* interval = input->GetLiveInterval();

        // Find the interval that covers `defined_by`_. Calls to this function
        // are made outside the linear scan, hence we need to use CoversSlow.
        while (interval != nullptr && !interval->CoversSlow(defined_by_->GetLifetimePosition())) {
          interval = interval->GetNextSibling();
        }

        if (interval != nullptr
            && interval->SameRegisterKind(*this)
            && (interval->GetRegisters() & (1u << reg)) != 0u) {
          // We found the input that has the register `reg`. Check if it is live after
          // `defined_by_`.
          return !interval->CoversSlow(
              defined_by_->GetLifetimePosition() + kLivenessPositionOfNormalUse);
        }
      }
    }
    LOG(FATAL) << "Unreachable";
    UNREACHABLE();
  }

  // Resets the starting point for range-searching queries to the first range.
  // Intervals must be reset prior to starting a new linear scan over them.
  void ResetSearchCache() {
    range_search_start_ = first_range_;
  }

  bool RequiresRegisterForDefinitionAt(size_t position) const {
    DCHECK(!IsTemp());
    if (!IsDefiningPosition(position)) {
      return false;
    }
    DCHECK(IsParent());
    LocationSummary* locations = defined_by_->GetLocations();
    Location location = locations->Out();
    // This interval is the first interval of the instruction. If the output
    // of the instruction requires a register, we return the position of that instruction
    // as the first register use.
    if (location.Equals(Location::SameAsFirstInput())) {
      location = locations->InAt(0);
      DCHECK(!location.Equals(Location::SameAsFirstInput()));
    }
    return location.IsRegisterKind() ||
           (location.IsUnallocated() && location.RequiresRegisterKind());
  }

  void SetHintPhiInterval(LiveInterval* hint_phi_interval) {
    DCHECK(hint_phi_interval->GetDefinedBy() != nullptr);
    DCHECK(hint_phi_interval->GetDefinedBy()->IsPhi());
    hint_phi_interval_ = hint_phi_interval;
  }

  LiveInterval* GetHintPhiInterval() {
    return hint_phi_interval_;
  }

  void SetNumSafepointsAfter(size_t num_safepoints_after) {
    DCHECK(!IsSplit());
    num_safepoints_after_ = num_safepoints_after;
  }

  size_t GetNumSafepointsAfter() const {
    DCHECK(!IsSplit());
    return num_safepoints_after_;
  }

  // Call `fn()` for all covered safepoints. Stop if the `fn()` returns false.
  //
  // The `remiaining_safepoints` arguments specifies how many of the `safepoints`
  // we need to process. It can be `GetParent()->GetNumSafepointsAfter()` or less
  // if some of the safepoints are known to be before this `LiveInterval`. This
  // function returns the updated number of remaining safepoints to process for
  // the next sibling, except when the iteration is aborted by `fn()` returning
  // false and remaining safepoint count is reported as zero.
  template <typename Function>
  size_t ForCoveredSafepoints(ArrayRef<HInstruction* const> safepoints,
                              size_t remaining_safepoints,
                              Function&& fn) const ALWAYS_INLINE {
    DCHECK_LE(remaining_safepoints, GetParent()->GetNumSafepointsAfter());
    LiveRange* range = GetFirstRange();
    DCHECK(range != nullptr);
    DCHECK_IMPLIES(
        remaining_safepoints < GetParent()->GetNumSafepointsAfter(),
        ComputeSafepointPosition(safepoints[remaining_safepoints]) < range->GetStart());
    for (; remaining_safepoints != 0u; --remaining_safepoints) {
      HInstruction* safepoint = safepoints[remaining_safepoints - 1u];
      size_t safepoint_position = ComputeSafepointPosition(safepoint);
      // Safepoints are ordered by lifetime position in decreasing order.
      DCHECK_IMPLIES(
          remaining_safepoints < safepoints.size(),
          safepoint_position >= ComputeSafepointPosition(safepoints[remaining_safepoints]));
      while (range->GetEnd() <= safepoint_position) {
        range = range->GetNext();
        if (range == nullptr) {
          return remaining_safepoints;
        }
      }
      if (range->GetStart() <= safepoint_position) {
        if (!fn(safepoint)) {
          return 0u;
        }
      }
    }
    return 0u;
  }

 private:
  LiveInterval(ScopedArenaAllocator* allocator,
               DataType::Type type,
               bool is_pair,
               HInstruction* defined_by,
               bool is_fixed = false,
               uint32_t regs = kNoRegisters,
               int8_t temp_index = kNoTempIndex)
      : allocator_(allocator),
        first_range_(nullptr),
        last_range_(nullptr),
        range_search_start_(nullptr),
        num_safepoints_after_(0u),
        uses_(),
        env_uses_(),
        next_sibling_(nullptr),
        parent_(this),
        defined_by_(defined_by),
        hint_phi_interval_(nullptr),
        registers_(regs),
        spill_slot_or_hint_(kNoSpillSlot),
        type_(type),
        temp_index_(temp_index),
        is_fixed_(is_fixed),
        is_pair_(is_pair) {}

  // Searches for a LiveRange that either covers the given position or is the
  // first next LiveRange. Returns null if no such LiveRange exists. Ranges
  // known to end before `position` can be skipped with `search_start`.
  LiveRange* FindRangeAtOrAfter(size_t position, LiveRange* search_start) const {
    if (kIsDebugBuild) {
      if (search_start != first_range_) {
        // If we are not searching the entire list of ranges, make sure we do
        // not skip the range we are searching for.
        if (search_start == nullptr) {
          DCHECK(IsDeadAt(position));
        } else if (search_start->GetStart() > position) {
          DCHECK_EQ(search_start, FindRangeAtOrAfter(position, first_range_));
        }
      }
    }

    LiveRange* range;
    for (range = search_start;
         range != nullptr && range->GetEnd() <= position;
         range = range->GetNext()) {
      continue;
    }
    return range;
  }

  bool IsDefiningPosition(size_t position) const {
    return IsParent() && (position == GetStart());
  }

  bool HasSynthesizeUseAt(size_t position) const {
    for (const UsePosition& use : GetUses()) {
      size_t use_position = use.GetPosition();
      if ((use_position == position) && use.IsSynthesized()) {
        return true;
      }
      if (use_position > position) break;
    }
    return false;
  }

  static size_t ComputeSafepointPosition(HInstruction* instruction) {
    // We special case instructions emitted at use site, as their
    // safepoint position needs to be at their use.
    if (instruction->IsEmittedAtUseSite()) {
      // Currently only applies to implicit null checks, which are emitted
      // at the next instruction.
      DCHECK(instruction->IsNullCheck()) << instruction->DebugName();
      return instruction->GetLifetimePosition() + kLivenessPositionsPerInstruction;
    } else {
      return instruction->GetLifetimePosition();
    }
  }

  void AddBackEdgeUses(const HBasicBlock& block_at_use);

  ScopedArenaAllocator* const allocator_;

  // Ranges of this interval. We need a quick access to the last range to test
  // for liveness (see `IsDeadAt`).
  LiveRange* first_range_;
  LiveRange* last_range_;

  // The first range at or after the current position of a linear scan. It is
  // used to optimize range-searching queries.
  LiveRange* range_search_start_;

  size_t num_safepoints_after_;

  // Uses of this interval. Only the parent interval keeps these lists.
  UsePositionList uses_;
  EnvUsePositionList env_uses_;

  // Live interval that is the result of a split.
  LiveInterval* next_sibling_;

  // The first interval from which split intervals come from.
  LiveInterval* parent_;

  // The instruction represented by this interval.
  HInstruction* const defined_by_;

  // If the last use of the instruction is a Phi, keep a record of that Phi's interval
  // for hints, except if the Phi is a loop Phi in an irreducible loop.
  LiveInterval* hint_phi_interval_;

  // The registers allocated to this interval, if any, otherwise `kNoRegisters`.
  // A register is recorded by setting the appropriate bit, register pair by setting two bits.
  uint32_t registers_;

  // The spill slot allocated to this interval, or a spill slot hint, `kNoSpillSlot` if neither.
  //
  // Values >= 0 represent an actual spill slot, -1 is reserved for `kNoSpillSlot`
  // and values <= -2 encode a non-negative spill slot hint as `-2 - hint`.
  int spill_slot_or_hint_;

  // The instruction type this interval corresponds to.
  const DataType::Type type_;

  // The index of the temporary, `kNoTempIndex` if not a temporary.
  // Currently, we support only 32 core and 32 FP registers. We should never request more
  // temps than that, so `int8_t` is enough. (Even if we added another register type.)
  const int8_t temp_index_;

  // Whether the interval is for a fixed register.
  const bool is_fixed_;

  // Whether this interval represents a register pair.
  const bool is_pair_;

  static constexpr int kNoSpillSlot = -1;
  static constexpr int8_t kNoTempIndex = -1;

  friend class RegisterAllocatorTest;

  DISALLOW_COPY_AND_ASSIGN(LiveInterval);
};

/**
 * Analysis that computes the liveness of instructions:
 *
 * (a) Non-environment uses of an instruction always make the instruction live.
 * (b) Environment uses of an instruction whose type is object (that is, non-primitive), make the
 *     instruction live, unless the class has an @DeadReferenceSafe annotation.
 *     This avoids unexpected premature reference enqueuing or finalization, which could
 *     result in premature deletion of native objects.  In the presence of @DeadReferenceSafe,
 *     object references are treated like primitive types.
 * (c) When the graph has the debuggable property, environment uses
 *     of an instruction that has a primitive type make the instruction live.
 *     If the graph does not have the debuggable property, the environment
 *     use has no effect, and may get a 'none' value after register allocation.
 * (d) When compiling in OSR mode, all loops in the compiled method may be entered
 *     from the interpreter via SuspendCheck; such use in SuspendCheck makes the instruction
 *     live.
 *
 * (b) is implemented through `SsaLivenessAnalysis::ShouldBeLiveForEnvironment()`.
 * (c) and (d) are implemented through `SsaLivenessAnalysis::ShouldAllBeLiveForEnvironment()`.
 */
class SsaLivenessAnalysis : public ValueObject {
 public:
  SsaLivenessAnalysis(HGraph* graph, CodeGenerator* codegen, ScopedArenaAllocator* allocator)
      : graph_(graph),
        codegen_(codegen),
        allocator_(allocator),
        block_infos_(graph->GetBlocks().size(),
                     nullptr,
                     allocator_->Adapter(kArenaAllocSsaLiveness)),
        instructions_from_ssa_index_(allocator_->Adapter(kArenaAllocSsaLiveness)),
        instructions_from_lifetime_position_(allocator_->Adapter(kArenaAllocSsaLiveness)) {
  }

  void Analyze();

  BitVectorView<size_t> GetLiveInSet(const HBasicBlock& block) const {
    return block_infos_[block.GetBlockId()]->live_in_;
  }

  BitVectorView<size_t> GetLiveOutSet(const HBasicBlock& block) const {
    return block_infos_[block.GetBlockId()]->live_out_;
  }

  BitVectorView<size_t> GetKillSet(const HBasicBlock& block) const {
    return block_infos_[block.GetBlockId()]->kill_;
  }

  HInstruction* GetInstructionFromSsaIndex(size_t index) const {
    return instructions_from_ssa_index_[index];
  }

  ArrayRef<HInstruction* const> GetInstructionsFromSsaIndexes() const {
    return ArrayRef<HInstruction* const>(instructions_from_ssa_index_);
  }

  HInstruction* GetInstructionFromPosition(size_t index) const {
    return instructions_from_lifetime_position_[index];
  }

  ArrayRef<HInstruction* const> GetInstructionsFromPositions() const {
    return ArrayRef<HInstruction* const>(instructions_from_lifetime_position_);
  }

  static HBasicBlock* GetBlockFromPosition(
      size_t index, ArrayRef<HInstruction* const> instructions_from_positions) {
    HInstruction* instruction = instructions_from_positions[index];
    if (instruction == nullptr) {
      // If we are at a block boundary, get the block following.
      instruction = instructions_from_positions[index + 1];
    }
    return instruction->GetBlock();
  }

  static bool IsAtBlockBoundary(
      size_t index, ArrayRef<HInstruction* const> instructions_from_positions) {
    return instructions_from_positions[index] == nullptr;
  }

  HInstruction* GetTempUser(LiveInterval* temp) const {
    // A temporary shares the same lifetime start as the instruction that requires it.
    DCHECK(temp->IsTemp());
    HInstruction* user =
        GetInstructionFromPosition(temp->GetStart() / kLivenessPositionsPerInstruction);
    DCHECK(user != nullptr);
    DCHECK_EQ(temp->GetStart(), user->GetLifetimePosition());
    return user;
  }

  size_t GetNumberOfSsaValues() const {
    return instructions_from_ssa_index_.size();
  }

  // Returns the lifetime position of the back edge that has the
  // greatest lifetime position.
  static size_t GetLoopLifetimeEnd(const HLoopInformation* loop_info);

  static constexpr const char* kLivenessPassName = "liveness";

 private:
  // Give an SSA number to each instruction that defines a value used by another instruction,
  // and setup the lifetime information of each instruction and block.
  void NumberInstructions();

  // Compute live ranges of instructions, as well as live_in, live_out and kill sets.
  void ComputeLiveness();

  // Compute the live ranges of instructions, as well as the initial live_in, live_out and
  // kill sets, that do not take into account backward branches.
  void ComputeLiveRanges();

  // After computing the initial sets, this method does a fixed point
  // calculation over the live_in and live_out set to take into account
  // backwards branches.
  void ComputeLiveInAndLiveOutSets();

  // Update the live_in set of the block and returns whether it has changed.
  bool UpdateLiveIn(const HBasicBlock& block);

  // Update the live_out set of the block and returns whether it has changed.
  bool UpdateLiveOut(const HBasicBlock& block);

  static void ProcessEnvironment(HInstruction* instruction,
                                 HBasicBlock* block,
                                 HInstruction* actual_user,
                                 BitVectorView<size_t> live_in);
  static void RecursivelyProcessInputs(HInstruction* instruction,
                                       HBasicBlock* block,
                                       HInstruction* actual_user,
                                       BitVectorView<size_t> live_in);

  // Returns whether all instructions held by the `HEnvironment` of `env_holder` should be
  // kept live by that `HEnvironment`
  static bool ShouldAllBeLiveForEnvironment(HInstruction* env_holder, HGraph* graph);

  // Returns whether `instruction` in an `HEnvironment` should be kept live by that `HEnvironment`.
  static bool ShouldBeLiveForEnvironment(HInstruction* instruction, bool is_dead_reference_safe);

  void CheckNoLiveInIrreducibleLoop(const HBasicBlock& block) const {
    if (!block.IsLoopHeader() || !block.GetLoopInformation()->IsIrreducible()) {
      return;
    }
    DoCheckNoLiveInIrreducibleLoop(block);
  }

  void DoCheckNoLiveInIrreducibleLoop(const HBasicBlock& block) const;

  HGraph* const graph_;
  CodeGenerator* const codegen_;

  // Use a local ScopedArenaAllocator for allocating memory.
  // This allocator must remain alive while doing register allocation.
  ScopedArenaAllocator* const allocator_;

  ScopedArenaVector<BlockInfo*> block_infos_;

  // Temporary array used when computing live_in, live_out, and kill sets.
  ScopedArenaVector<HInstruction*> instructions_from_ssa_index_;

  // Compressed map from lifetime position to instruction (nullptr for block start).
  // Indexed by the lifetime position divided by `kLivenessPositionsPerInstruction`.
  ScopedArenaVector<HInstruction*> instructions_from_lifetime_position_;

  friend class RegisterAllocatorTest;

  DISALLOW_COPY_AND_ASSIGN(SsaLivenessAnalysis);
};

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_SSA_LIVENESS_ANALYSIS_H_

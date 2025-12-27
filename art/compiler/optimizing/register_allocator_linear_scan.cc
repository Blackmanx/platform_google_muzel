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

#include "register_allocator_linear_scan.h"

#include <iostream>
#include <sstream>

#include "base/bit_utils_iterator.h"
#include "base/bit_vector-inl.h"
#include "base/pointer_size.h"
#include "code_generator.h"
#include "com_android_art_flags.h"
#include "linear_order.h"
#include "register_allocation_resolver.h"
#include "register_allocator-inl.h"
#include "ssa_liveness_analysis.h"

namespace art HIDDEN {

static constexpr size_t kMaxLifetimePosition = -1;
static constexpr size_t kDefaultNumberOfSpillSlots = 4;

// For simplicity, we implement register pairs as (reg, reg + 1).
// Note that this is a requirement for double registers on ARM, since we
// allocate SRegister.
static int GetHighForLowRegister(int reg) { return reg + 1; }
static bool IsLowRegister(int reg) { return (reg & 1) == 0; }

class RegisterAllocatorLinearScan::SpillSlotData {
 public:
  SpillSlotData(LiveInterval* interval, size_t end)
      : end_(end),
        gap_start_(interval->GetFirstRange()->GetEnd()),
        interval_(interval),
        range_(interval->GetFirstRange()) {
    DCHECK_EQ(end, interval->GetLastSibling()->GetEnd());
  }

  size_t GetEnd() const {
    return end_;
  }

  // Determine if the spill slot can be used for another interval `parent`.
  // Returns `true` if the spill slot can be reused, `false` otherwise.
  // The `parent`'s `start` and `end` are passed as arguments as a performance optimization.
  //
  // This is a heuristic which does not find all spill slot reuse opportunities. For that,
  // we would need to keep track of all lifetime positions used for the spill slot, either as
  // a bit mask, or as a list of all intervals using it, and that could take a lot of memory.
  //
  // Instead, we keep only the `interval_` with the longest lifetime (ending at `end_`) and
  // the `gap_start_`, the earlier position that can be reused. When we reuse the slot for
  // another interval that falls within `[gap_start_, end_)` and does not overlap the current
  // `interval_`'s lifetime, we update `gap_start_` to the end of that interval.
  bool CanUseFor(LiveInterval* parent, size_t start, size_t end) {
    DCHECK(!parent->IsSplit());
    DCHECK_EQ(start, parent->GetStart());
    DCHECK_EQ(end, parent->GetLastSibling()->GetEnd());
    // Check if the spill slot use has ended at the `start` position.
    if (start >= end_) {
      return true;
    }
    // Check if the `parent` interval can fit in the gap.
    if (start < gap_start_ || end > end_) {
      return false;
    }
    // Update search start position based on `gap_start_`.
    while (gap_start_ >= interval_->GetEnd()) {
      if (interval_->GetNextSibling() == nullptr) {
        return true;  // The entire range `[gap_start_, end_)` can be used.
      }
      interval_ = interval_->GetNextSibling();
      range_ = interval_->GetFirstRange();
    }
    while (gap_start_ >= range_->GetEnd()) {
      range_ = range_->GetNext();
      DCHECK(range_ != nullptr);
    }
    // Check if there are any overlapping ranges.
    // This is similar to `LiveInterval::FirstIntersectionWith()` but covers sibling intervals.
    auto move_to_next_range = [](LiveInterval*& interval, LiveRange*& range) ALWAYS_INLINE {
      range = range->GetNext();
      if (range == nullptr) {
        if (interval->GetNextSibling() == nullptr) {
          return false;
        }
        interval = interval->GetNextSibling();
        range = interval->GetFirstRange();
      }
      return true;
    };
    LiveInterval* interval = interval_;
    LiveRange* range = range_;
    LiveInterval* other_interval = parent;
    LiveRange* other_range = parent->GetFirstRange();
    while (true) {
      if (range->IsBefore(*other_range)) {
        if (!move_to_next_range(interval, range)) {
          return true;  // No more ranges to check.
        }
      } else if (other_range->IsBefore(*range)) {
        if (!move_to_next_range(other_interval, other_range)) {
          return true;  // No more ranges to check.
        }
      } else {
        DCHECK(range->IntersectsWith(*other_range));
        return false;
      }
    }
  }

  void UseFor(LiveInterval* interval, size_t end) {
    DCHECK_EQ(end, interval->GetLastSibling()->GetEnd());
    if (end > end_) {
      DCHECK_GE(interval->GetParent()->GetStart(), end_);
      *this = SpillSlotData(interval, end);
    } else {
      DCHECK(CanUseFor(interval->GetParent(), interval->GetParent()->GetStart(), end));
      DCHECK_GT(end, gap_start_);
      gap_start_ = end;
    }
  }

 private:
  size_t end_;
  size_t gap_start_;
  LiveInterval* interval_;
  LiveRange* range_;
};

class RegisterAllocatorLinearScan::LinearScan {
 public:
  struct CoreRegisterTag {};
  struct FpRegisterTag {};
  struct VectorRegisterTag {};

  LinearScan(RegisterAllocatorLinearScan* register_allocator, CoreRegisterTag /*tag*/)
      : codegen_(register_allocator->codegen_),
        number_of_registers_(codegen_->GetNumberOfCoreRegisters()),
        register_type_(PhysicalRegisterType::kCoreRegister),
        registers_blocked_for_call_(
            register_allocator->registers_blocked_for_call_.GetCoreRegisterSet()),
        available_registers_(register_allocator->available_registers_.GetCoreRegisterSet()),
        spill_slots_(&register_allocator->int_spill_slots_),
        wide_spill_slots_(&register_allocator->long_spill_slots_),
        unhandled_(TakeUnhandledIntervals(&register_allocator->unhandled_core_intervals_)),
        handled_(register_allocator->allocator_->Adapter(kArenaAllocRegisterAllocator)),
        active_(register_allocator->allocator_->Adapter(kArenaAllocRegisterAllocator)),
        inactive_(register_allocator->allocator_->Adapter(kArenaAllocRegisterAllocator)),
        instructions_from_positions_(register_allocator->liveness_.GetInstructionsFromPositions()),
        registers_array_(register_allocator->allocator_->AllocArray<size_t>(
            number_of_registers_, kArenaAllocRegisterAllocator)),
        safepoints_(register_allocator->safepoints_) {
    Init(register_allocator, register_allocator->physical_core_register_intervals_);
  }

  LinearScan(RegisterAllocatorLinearScan* register_allocator, FpRegisterTag /*tag*/)
      : codegen_(register_allocator->codegen_),
        number_of_registers_(codegen_->GetNumberOfFloatingPointRegisters()),
        register_type_(PhysicalRegisterType::kFpuRegister),
        registers_blocked_for_call_(
            register_allocator->registers_blocked_for_call_.GetFpuRegisterSet()),
        available_registers_(register_allocator->available_registers_.GetFpuRegisterSet()),
        spill_slots_(&register_allocator->float_spill_slots_),
        wide_spill_slots_(&register_allocator->double_spill_slots_),
        unhandled_(TakeUnhandledIntervals(&register_allocator->unhandled_fp_intervals_)),
        handled_(register_allocator->allocator_->Adapter(kArenaAllocRegisterAllocator)),
        active_(register_allocator->allocator_->Adapter(kArenaAllocRegisterAllocator)),
        inactive_(register_allocator->allocator_->Adapter(kArenaAllocRegisterAllocator)),
        instructions_from_positions_(register_allocator->liveness_.GetInstructionsFromPositions()),
        registers_array_(register_allocator->allocator_->AllocArray<size_t>(
            number_of_registers_, kArenaAllocRegisterAllocator)),
        safepoints_(register_allocator->safepoints_) {
    Init(register_allocator, register_allocator->physical_fp_register_intervals_);
  }

  LinearScan(RegisterAllocatorLinearScan* register_allocator, VectorRegisterTag /*tag*/)
      : codegen_(register_allocator->codegen_),
        number_of_registers_(codegen_->GetNumberOfVectorRegisters()),
        register_type_(PhysicalRegisterType::kVectorRegister),
        registers_blocked_for_call_(
            register_allocator->registers_blocked_for_call_.GetVecRegisterSet()),
        available_registers_(register_allocator->available_registers_.GetVecRegisterSet()),
        spill_slots_(nullptr),  // Unused, all vector intervals have type `kFloat64`.
        wide_spill_slots_(&register_allocator->vector_spill_slots_),
        unhandled_(TakeUnhandledIntervals(&register_allocator->unhandled_vector_intervals_)),
        handled_(register_allocator->allocator_->Adapter(kArenaAllocRegisterAllocator)),
        active_(register_allocator->allocator_->Adapter(kArenaAllocRegisterAllocator)),
        inactive_(register_allocator->allocator_->Adapter(kArenaAllocRegisterAllocator)),
        instructions_from_positions_(register_allocator->liveness_.GetInstructionsFromPositions()),
        registers_array_(register_allocator->allocator_->AllocArray<size_t>(
            number_of_registers_, kArenaAllocRegisterAllocator)),
        safepoints_(register_allocator->safepoints_) {
    Init(register_allocator, register_allocator->physical_vector_register_intervals_);
  }

  void Run();

 private:
  void Init(RegisterAllocatorLinearScan* register_allocator,
            const ScopedArenaVector<LiveInterval*>& physical_register_intervals);

  static ScopedArenaVector<LiveInterval*> TakeUnhandledIntervals(
      ScopedArenaVector<LiveInterval*>* source) {
    ScopedArenaVector<LiveInterval*> result(source->get_allocator());
    result.swap(*source);
    return result;
  }

  ALWAYS_INLINE ScopedArenaVector<SpillSlotData>* GetSpillSlotsForType(DataType::Type type) {
    switch (type) {
      case DataType::Type::kFloat64:
      case DataType::Type::kInt64:
        DCHECK(wide_spill_slots_ != nullptr);
        return wide_spill_slots_;
      case DataType::Type::kUint32:
      case DataType::Type::kUint64:
      case DataType::Type::kVoid:
        // Let the compiler optimize this away in release build.
        DCHECK(false) << "Unexpected type for interval " << type;
        FALLTHROUGH_INTENDED;
      case DataType::Type::kFloat32:
      case DataType::Type::kReference:
      case DataType::Type::kInt32:
      case DataType::Type::kUint16:
      case DataType::Type::kUint8:
      case DataType::Type::kInt8:
      case DataType::Type::kBool:
      case DataType::Type::kInt16:
        DCHECK(spill_slots_ != nullptr);
        return spill_slots_;
    }
  }

  bool TryUsingSpillSlotHint(LiveInterval* interval);
  bool TryAllocateFreeReg(LiveInterval* interval, uint32_t available_registers);

  // Try to allocate a blocked register. Returns true on success, false otherwise.
  // The second returned value indicates registers freed to accommodate the allocation;
  // in some cases, registers can be freed even if the function reports failure.
  std::pair<bool, uint32_t> AllocateBlockedReg(LiveInterval* interval);

  int FindAvailableRegisterPair(
      uint32_t available_registers, ArrayRef<size_t> next_use, size_t starting_at) const;
  int FindAvailableRegister(
      uint32_t available_registers, ArrayRef<size_t> next_use, LiveInterval* current) const;

  // Allocate a spill slot for the given interval. Should be called in linear
  // order of interval starting positions.
  void AllocateSpillSlotFor(LiveInterval* interval);

  void DumpInterval(std::ostream& stream, LiveInterval* interval) const;
  void DumpAllIntervals(std::ostream& stream) const;

  // Try splitting an active non-pair or unaligned pair interval at the given `position`.
  // Returns the registers that were freed, `kNoRegisters` on failure.
  uint32_t TrySplitNonPairOrUnalignedPairIntervalAt(size_t position,
                                                    size_t first_register_use,
                                                    ArrayRef<size_t> next_use);

  LiveInterval* SplitBetween(LiveInterval* interval, size_t from, size_t to) const {
    return RegisterAllocator::SplitBetween(interval, from, to, instructions_from_positions_);
  }

  bool IsBlocked(int reg) const {
    DCHECK_LT(static_cast<size_t>(reg), number_of_registers_);
    return (available_registers_ & (1u << reg)) == 0u;
  }

  bool IsCallerSaveRegister(int reg) const {
    DCHECK_LT(static_cast<size_t>(reg), BitSizeOf<uint32_t>());
    return (registers_blocked_for_call_ & (1u << reg)) != 0u;
  }

  ArrayRef<size_t> GetRegistersArray() {
    return ArrayRef<size_t>(registers_array_, number_of_registers_);
  }

  uint32_t GetRegisterMask(LiveInterval* interval) {
    return interval->HasRegisters()
        ? RegisterAllocator::GetNormalRegisterMask(interval, register_type_)
        : RegisterAllocator::GetBlockedRegistersMask(interval,
                                                     instructions_from_positions_,
                                                     available_registers_,
                                                     registers_blocked_for_call_);
  }

  // Returns the first register hint that is at least free before the value
  // contained in `free_until`. If none is found, returns `kNoRegister`.
  int FindFirstRegisterHint(
      LiveInterval* interval, uint32_t available_registers, ArrayRef<size_t> free_until) const;

  // If there is enough at the definition site to find a register (for example
  // it uses the same input as the first input), returns the register as a hint.
  // Returns `kNoRegister` otherwise.
  int FindHintAtDefinition(LiveInterval* interval) const;

  static constexpr int kNoRegister = -1;

  CodeGenerator* const codegen_;

  // Number of registers for the current register kind (core or floating point).
  size_t number_of_registers_;

  // The register type processed by this `LinearScan` object.
  const PhysicalRegisterType register_type_;

  // Mask of registers blocked for a call.
  const uint32_t registers_blocked_for_call_;

  // All available registers, as decided by the code generator.
  const uint32_t available_registers_;

  // Spill slots for normal and wide intervals, pointing to appropriately typed slots
  // in the `RegisterAllocatorLinearScan`.
  ScopedArenaVector<SpillSlotData>* const spill_slots_;
  ScopedArenaVector<SpillSlotData>* const wide_spill_slots_;

  // Currently processed list of unhandled intervals. Retrieved either from
  // `unhandled_core_intervals_` or `unhandled_fp_intervals_`.
  ScopedArenaVector<LiveInterval*> unhandled_;

  // List of intervals that have been processed.
  ScopedArenaVector<LiveInterval*> handled_;

  // List of intervals that are currently active when processing a new live interval.
  // That is, they have a live range that spans the start of the new interval.
  ScopedArenaVector<LiveInterval*> active_;

  // List of intervals that are currently inactive when processing a new live interval.
  // That is, they have a lifetime hole that spans the start of the new interval.
  ScopedArenaVector<LiveInterval*> inactive_;

  // Instructions indexed by lifetime positions, cached from `SsaLivenessAnalysis`.
  const ArrayRef<HInstruction* const> instructions_from_positions_;

  // Temporary array, allocated ahead of time for simplicity.
  size_t* registers_array_;

  ArrayRef<HInstruction* const> safepoints_;
};

RegisterAllocatorLinearScan::RegisterAllocatorLinearScan(ScopedArenaAllocator* allocator,
                                                         CodeGenerator* codegen,
                                                         const SsaLivenessAnalysis& liveness)
      : RegisterAllocator(allocator, codegen, liveness),
        unhandled_core_intervals_(allocator->Adapter(kArenaAllocRegisterAllocator)),
        unhandled_fp_intervals_(allocator->Adapter(kArenaAllocRegisterAllocator)),
        unhandled_vector_intervals_(allocator->Adapter(kArenaAllocRegisterAllocator)),
        physical_core_register_intervals_(allocator->Adapter(kArenaAllocRegisterAllocator)),
        physical_fp_register_intervals_(allocator->Adapter(kArenaAllocRegisterAllocator)),
        physical_vector_register_intervals_(allocator->Adapter(kArenaAllocRegisterAllocator)),
        block_registers_for_call_interval_(
            LiveInterval::MakeFixedInterval(allocator, kNoRegisters, DataType::Type::kVoid)),
        block_registers_special_interval_(
            LiveInterval::MakeFixedInterval(allocator, kNoRegisters, DataType::Type::kVoid)),
        temp_intervals_(allocator->Adapter(kArenaAllocRegisterAllocator)),
        int_spill_slots_(allocator->Adapter(kArenaAllocRegisterAllocator)),
        long_spill_slots_(allocator->Adapter(kArenaAllocRegisterAllocator)),
        float_spill_slots_(allocator->Adapter(kArenaAllocRegisterAllocator)),
        double_spill_slots_(allocator->Adapter(kArenaAllocRegisterAllocator)),
        vector_spill_slots_(allocator->Adapter(kArenaAllocRegisterAllocator)),
        catch_phi_spill_slots_(0),
        safepoints_(allocator->Adapter(kArenaAllocRegisterAllocator)),
        reserved_out_slots_(0) {
  temp_intervals_.reserve(4);
  int_spill_slots_.reserve(kDefaultNumberOfSpillSlots);
  long_spill_slots_.reserve(kDefaultNumberOfSpillSlots);
  float_spill_slots_.reserve(kDefaultNumberOfSpillSlots);
  double_spill_slots_.reserve(kDefaultNumberOfSpillSlots);

  physical_core_register_intervals_.resize(codegen->GetNumberOfCoreRegisters(), nullptr);
  physical_fp_register_intervals_.resize(codegen->GetNumberOfFloatingPointRegisters(), nullptr);
  physical_vector_register_intervals_.resize(codegen->GetNumberOfVectorRegisters(), nullptr);
}

RegisterAllocatorLinearScan::~RegisterAllocatorLinearScan() {}

void RegisterAllocatorLinearScan::AllocateRegisters() {
  AllocateRegistersInternal();
  RegisterAllocationResolver(codegen_, liveness_)
      .Resolve(ArrayRef<HInstruction* const>(safepoints_),
               reserved_out_slots_,
               int_spill_slots_.size(),
               long_spill_slots_.size(),
               float_spill_slots_.size(),
               double_spill_slots_.size(),
               catch_phi_spill_slots_,
               ArrayRef<LiveInterval* const>(temp_intervals_));

  if (kIsDebugBuild) {
    ValidateInternal(PhysicalRegisterType::kCoreRegister, /*log_fatal_on_failure=*/ true);
    ValidateInternal(PhysicalRegisterType::kFpuRegister, /*log_fatal_on_failure=*/ true);
    // Check that the linear order is still correct with regards to lifetime positions.
    // Since only parallel moves have been inserted during the register allocation,
    // these checks are mostly for making sure these moves have been added correctly.
    size_t current_liveness = 0;
    for (HBasicBlock* block : codegen_->GetGraph()->GetLinearOrder()) {
      for (HInstructionIteratorPrefetchNext inst_it(block->GetPhis()); !inst_it.Done();
           inst_it.Advance()) {
        HInstruction* instruction = inst_it.Current();
        DCHECK_LE(current_liveness, instruction->GetLifetimePosition());
        current_liveness = instruction->GetLifetimePosition();
      }
      for (HInstructionIteratorPrefetchNext inst_it(block->GetInstructions());
           !inst_it.Done();
           inst_it.Advance()) {
        HInstruction* instruction = inst_it.Current();
        DCHECK_LE(current_liveness, instruction->GetLifetimePosition()) << instruction->DebugName();
        current_liveness = instruction->GetLifetimePosition();
      }
    }
  }
}

bool RegisterAllocatorLinearScan::Validate(bool log_fatal_on_failure) {
  return ValidateInternal(PhysicalRegisterType::kCoreRegister, log_fatal_on_failure) &&
         ValidateInternal(PhysicalRegisterType::kFpuRegister, log_fatal_on_failure);
}

inline ScopedArenaVector<LiveInterval*>* RegisterAllocatorLinearScan::GetPhysicalRegisterIntervals(
    PhysicalRegisterType register_type) {
  switch (register_type) {
    case PhysicalRegisterType::kCoreRegister:
      return &physical_core_register_intervals_;
    case PhysicalRegisterType::kFpuRegister:
      return &physical_fp_register_intervals_;
    case PhysicalRegisterType::kVectorRegister:
      return &physical_vector_register_intervals_;
  }
}

void RegisterAllocatorLinearScan::BlockRegister(PhysicalRegisterType register_type,
                                                int reg,
                                                size_t position,
                                                bool will_call) {
  // Ensure that an explicit input/output/temporary register is marked as being allocated.
  codegen_->AddAllocatedRegister(register_type, reg);
  if (will_call) {
    uint32_t registers_blocked_for_call = registers_blocked_for_call_.GetRegisterSet(register_type);
    if ((registers_blocked_for_call & (1u << reg)) != 0u) {
      // Register is already marked as blocked by the `block_registers_for_call_interval_`.
      return;
    }
  }
  ArrayRef<LiveInterval*> physical_register_intervals(*GetPhysicalRegisterIntervals(register_type));
  LiveInterval* interval = physical_register_intervals[reg];
  if (interval == nullptr) {
    DataType::Type type = (register_type == PhysicalRegisterType::kCoreRegister)
        ? DataType::Type::kInt32
        : (register_type == PhysicalRegisterType::kFpuRegister) ? DataType::Type::kFloat32
                                                                : DataType::Type::kFloat64;
    interval = LiveInterval::MakeFixedInterval(allocator_, 1u << reg, type);
    physical_register_intervals[reg] = interval;
  }
  DCHECK_EQ(interval->GetRegisters(), 1u << reg);
  interval->AddRange(position, position + kLivenessPositionsToBlock);
}

inline void RegisterAllocatorLinearScan::BlockRegisters(Location location,
                                                        size_t position,
                                                        bool will_call) {
  DCHECK(location.IsRegisterKind());
  PhysicalRegisterType register_type = location.GetRegisterType();
  if (location.IsRegister()) {
    BlockRegister(register_type, location.reg(), position, will_call);
  } else {
    BlockRegister(register_type, location.low(), position, will_call);
    BlockRegister(register_type, location.high(), position, will_call);
  }
}

void RegisterAllocatorLinearScan::AllocateRegistersInternal() {
  // Iterate post-order, to ensure the list is sorted, and the last added interval
  // is the one with the lowest start position.
  for (HBasicBlock* block : codegen_->GetGraph()->GetLinearPostOrder()) {
    for (HBackwardInstructionIteratorPrefetchNext back_it(block->GetInstructions());
         !back_it.Done();
         back_it.Advance()) {
      ProcessInstruction(back_it.Current());
    }
    for (HInstructionIteratorPrefetchNext inst_it(block->GetPhis()); !inst_it.Done();
         inst_it.Advance()) {
      ProcessInstruction(inst_it.Current());
    }

    if (block->IsCatchBlock() ||
        (block->IsLoopHeader() && block->GetLoopInformation()->IsIrreducible())) {
      // By blocking all registers at the top of each catch block or irreducible loop, we force
      // intervals belonging to the live-in set of the catch/header block to be spilled.
      // TODO(ngeoffray): Phis in this block could be allocated in register.
      size_t position = block->GetLifetimeStart();
      DCHECK_EQ(liveness_.GetInstructionFromPosition(position / kLivenessPositionsPerInstruction),
                nullptr);
      block_registers_special_interval_->AddRange(position, position + kLivenessPositionsToBlock);
    }
  }

  // Add the current method to the `reserved_out_slots_`. ArtMethod* takes 2 vregs for 64 bits.
  PointerSize pointer_size = InstructionSetPointerSize(codegen_->GetInstructionSet());
  reserved_out_slots_ += static_cast<size_t>(pointer_size) / kVRegSize;

  // Most methods have some core register intervals, so run the core register pass unconditionally.
  LinearScan(this, LinearScan::CoreRegisterTag()).Run();
  // Most methods do not have any FP register intervals, so try to avoid the overhead
  // of constructing the `LinearScan` object for the FP registers pass.
  if (!unhandled_fp_intervals_.empty()) {
    LinearScan(this, LinearScan::FpRegisterTag()).Run();
  }
  if (!unhandled_vector_intervals_.empty()) {
    LinearScan(this, LinearScan::VectorRegisterTag()).Run();
  }
}

void RegisterAllocatorLinearScan::LinearScan::Init(
    RegisterAllocatorLinearScan* register_allocator,
    const ScopedArenaVector<LiveInterval*>& physical_register_intervals) {
  // Add intervals representing groups of physical registers blocked for calls,
  // catch blocks and irreducible loop headers.
  LiveInterval* block_registers_intervals[] = {
      register_allocator->block_registers_for_call_interval_,
      register_allocator->block_registers_special_interval_
  };
  for (LiveInterval* block_registers_interval : block_registers_intervals) {
    if (block_registers_interval->GetFirstRange() != nullptr) {
      block_registers_interval->ResetSearchCache();
      inactive_.push_back(block_registers_interval);
    }
  }
  for (LiveInterval* fixed : physical_register_intervals) {
    if (fixed != nullptr) {
      // Fixed interval is added to inactive_ instead of unhandled_.
      // It's also the only type of inactive interval whose start position
      // can be after the current interval during linear scan.
      // Fixed interval is never split and never moves to unhandled_.
      inactive_.push_back(fixed);
    }
  }
}

void RegisterAllocatorLinearScan::ProcessInstruction(HInstruction* instruction) {
  LocationSummary* locations = instruction->GetLocations();

  // Check for early returns.
  if (locations == nullptr) {
    return;
  }

  if (locations->CanCall()) {
    // Update the `reserved_out_slots_` for invokes that make a call, including intrinsics
    // that make the call only on the slow-path. Same for the `HStringBuilderAppend`.
    if (instruction->IsInvoke()) {
      reserved_out_slots_ = std::max<size_t>(
          reserved_out_slots_, instruction->AsInvoke()->GetNumberOfOutVRegs());
    } else if (instruction->IsStringBuilderAppend()) {
      reserved_out_slots_ = std::max<size_t>(
          reserved_out_slots_, instruction->AsStringBuilderAppend()->GetNumberOfOutVRegs());
    }
  }
  bool will_call = locations->WillCall();
  if (will_call) {
    // If a call will happen, add the range to a fixed interval that represents all the
    // caller-save registers blocked at call sites.
    const size_t position = instruction->GetLifetimePosition();
    DCHECK_NE(liveness_.GetInstructionFromPosition(position / kLivenessPositionsPerInstruction),
              nullptr);
    block_registers_for_call_interval_->AddRange(position, position + kLivenessPositionsToBlock);
  }
  CheckForTempLiveIntervals(instruction, will_call);
  size_t num_safepoints_after = safepoints_.size();
  CheckForSafepoint(instruction);
  CheckForFixedInputs(instruction, will_call);

  LiveInterval* current = instruction->GetLiveInterval();
  if (current == nullptr) {
    return;
  }

  current->SetNumSafepointsAfter(num_safepoints_after);
  const bool core_register = !DataType::IsFloatingPointType(instruction->GetType());
  ScopedArenaVector<LiveInterval*>& unhandled =
      core_register ? unhandled_core_intervals_ : unhandled_fp_intervals_;

  DCHECK(unhandled.empty() || current->StartsBeforeOrAt(unhandled.back()));

  DCHECK_EQ(codegen_->NeedsTwoRegisters(current->GetType()), current->IsPair());

  current->ResetSearchCache();
  CheckForFixedOutput(instruction, will_call);

  if (instruction->IsPhi() && instruction->AsPhi()->IsCatchPhi()) {
    AllocateSpillSlotForCatchPhi(instruction->AsPhi());
  }

  // If needed, add interval to the list of unhandled intervals.
  if (current->HasSpillSlot() || instruction->IsConstant()) {
    // Split just before first register use.
    DCHECK(!current->RequiresRegisterForDefinitionAt(current->GetStart()));
    size_t first_register_use = current->FirstRegisterUse();
    if (first_register_use != kNoLifetime) {
      LiveInterval* split = SplitBetween(current,
                                         current->GetStart(),
                                         first_register_use - 1,
                                         liveness_.GetInstructionsFromPositions());
      // Don't add directly to `unhandled`, it needs to be sorted and the start
      // of this new interval might be after intervals already in the list.
      AddSorted(&unhandled, split);
    } else {
      // Nothing to do, we won't allocate a register for this value.
    }
  } else {
    // Don't add directly to `unhandled`, temp or safepoint intervals
    // for this instruction may have been added, and those can be
    // processed first.
    AddSorted(&unhandled, current);
  }
}

void RegisterAllocatorLinearScan::CheckForTempLiveIntervals(HInstruction* instruction,
                                                            bool will_call) {
  LocationSummary* locations = instruction->GetLocations();
  size_t position = instruction->GetLifetimePosition();

  // Create synthesized intervals for temporaries.
  for (size_t i = 0; i < locations->GetTempCount(); ++i) {
    Location temp = locations->GetTemp(i);
    if (temp.IsRegister()) {
      BlockRegister(temp.GetRegisterType(), temp.reg(), position, will_call);
    } else {
      DCHECK(temp.IsUnallocated());
      switch (temp.GetPolicy()) {
        case Location::kRequiresCoreRegister: {
          LiveInterval* interval = LiveInterval::MakeTempInterval(
              allocator_, DataType::Type::kInt32, /*is_pair=*/ false, i, position, instruction);
          temp_intervals_.push_back(interval);
          unhandled_core_intervals_.push_back(interval);
          break;
        }

        case Location::kRequiresFpuRegister: {
          bool is_pair = codegen_->NeedsTwoRegisters(DataType::Type::kFloat64);
          LiveInterval* interval = LiveInterval::MakeTempInterval(
              allocator_, DataType::Type::kFloat64, is_pair, i, position, instruction);
          temp_intervals_.push_back(interval);
          unhandled_fp_intervals_.push_back(interval);
          break;
        }

        case Location::kRequiresVecRegister: {
          LiveInterval* interval = LiveInterval::MakeTempInterval(
              allocator_, DataType::Type::kFloat64, /*is_pair=*/ false, i, position, instruction);
          temp_intervals_.push_back(interval);
          unhandled_vector_intervals_.push_back(interval);
          break;
        }

        default:
          LOG(FATAL) << "Unexpected policy for temporary location " << temp.GetPolicy();
      }
    }
  }
}

void RegisterAllocatorLinearScan::CheckForSafepoint(HInstruction* instruction) {
  LocationSummary* locations = instruction->GetLocations();
  if (locations->NeedsSafepoint()) {
    safepoints_.push_back(instruction);
  }
}

void RegisterAllocatorLinearScan::CheckForFixedInputs(HInstruction* instruction, bool will_call) {
  LocationSummary* locations = instruction->GetLocations();
  size_t position = instruction->GetLifetimePosition();
  for (size_t i = 0; i < locations->GetInputCount(); ++i) {
    Location input = locations->InAt(i);
    if (input.IsRegisterKind()) {
      BlockRegisters(input, position, will_call);
    }
  }
}

void RegisterAllocatorLinearScan::CheckForFixedOutput(HInstruction* instruction, bool will_call) {
  LocationSummary* locations = instruction->GetLocations();
  size_t position = instruction->GetLifetimePosition();
  LiveInterval* current = instruction->GetLiveInterval();
  // Some instructions define their output in fixed register/stack slot. We need
  // to ensure we know these locations before doing register allocation. For a
  // given register, we create an interval that covers these locations. The register
  // will be unavailable at these locations when trying to allocate one for an
  // interval.
  //
  // The backwards walking ensures the ranges are ordered on increasing start positions.
  Location output = locations->Out();
  if (output.IsUnallocated()) {
    if (output.GetPolicy() == Location::kSameAsFirstInput) {
      DCHECK(locations->OutputCanOverlapWithInputs());
      Location first = locations->InAt(0);
      if (first.IsRegisterKind()) {
        current->SetFrom(position + kLivenessPositionOfFixedOutput);
        current->SetRegisters(first.GetRegisterSet());
      }
    } else if (com::android::art::flags::reg_alloc_no_output_overlap() &&
               !locations->OutputCanOverlapWithInputs()) {
      current->SetFrom(position + kLivenessPositionOfNonOverlappingOutput);
    }
  } else if (output.IsRegisterKind()) {
    // Shift the interval's start by one to account for the blocked register.
    current->SetFrom(position + kLivenessPositionOfFixedOutput);
    current->SetRegisters(output.GetRegisterSet());
    BlockRegisters(output, position, will_call);
  } else if (output.IsStackSlot() || output.IsDoubleStackSlot()) {
    current->SetSpillSlot(output.GetStackIndex());
  } else {
    DCHECK(output.IsConstant());
  }
}

class AllRangesIterator : public ValueObject {
 public:
  explicit AllRangesIterator(LiveInterval* interval)
      : current_interval_(interval),
        current_range_(interval->GetFirstRange()) {}

  bool Done() const { return current_interval_ == nullptr; }
  LiveRange* CurrentRange() const { return current_range_; }
  LiveInterval* CurrentInterval() const { return current_interval_; }

  void Advance() {
    current_range_ = current_range_->GetNext();
    if (current_range_ == nullptr) {
      current_interval_ = current_interval_->GetNextSibling();
      if (current_interval_ != nullptr) {
        current_range_ = current_interval_->GetFirstRange();
      }
    }
  }

 private:
  LiveInterval* current_interval_;
  LiveRange* current_range_;

  DISALLOW_COPY_AND_ASSIGN(AllRangesIterator);
};

inline size_t RegisterAllocatorLinearScan::GetNumberOfSpillSlots() const {
  return int_spill_slots_.size() +
         long_spill_slots_.size() +
         float_spill_slots_.size() +
         double_spill_slots_.size() +
         catch_phi_spill_slots_;
}

bool RegisterAllocatorLinearScan::ValidateInternal(PhysicalRegisterType current_register_type,
                                                   bool log_fatal_on_failure) const {
  auto should_process = [current_register_type](LiveInterval* interval) {
    if (interval == nullptr) {
      return false;
    }
    PhysicalRegisterType register_type = DataType::IsFloatingPointType(interval->GetType())
        ? PhysicalRegisterType::kFpuRegister
        : PhysicalRegisterType::kCoreRegister;
    return register_type == current_register_type;
  };

  // To simplify unit testing, we eagerly create the array of intervals, and
  // call the helper method.
  ScopedArenaAllocator allocator(allocator_->GetArenaStack());
  ScopedArenaVector<LiveInterval*> intervals(
      allocator.Adapter(kArenaAllocRegisterAllocatorValidate));
  for (size_t i = 0; i < liveness_.GetNumberOfSsaValues(); ++i) {
    HInstruction* instruction = liveness_.GetInstructionFromSsaIndex(i);
    if (should_process(instruction->GetLiveInterval())) {
      intervals.push_back(instruction->GetLiveInterval());
    }
  }

  for (LiveInterval* block_registers_interval : { block_registers_for_call_interval_,
                                                  block_registers_special_interval_ }) {
    if (block_registers_interval->GetFirstRange() != nullptr) {
      intervals.push_back(block_registers_interval);
    }
  }
  const ScopedArenaVector<LiveInterval*>* physical_register_intervals =
      (current_register_type == PhysicalRegisterType::kCoreRegister)
          ? &physical_core_register_intervals_
          : &physical_fp_register_intervals_;
  for (LiveInterval* fixed : *physical_register_intervals) {
    if (fixed != nullptr) {
      intervals.push_back(fixed);
    }
  }

  for (LiveInterval* temp : temp_intervals_) {
    if (should_process(temp)) {
      intervals.push_back(temp);
    }
  }

  return ValidateIntervals(ArrayRef<LiveInterval* const>(intervals),
                           GetNumberOfSpillSlots(),
                           reserved_out_slots_,
                           *codegen_,
                           &liveness_,
                           current_register_type,
                           log_fatal_on_failure);
}

void RegisterAllocatorLinearScan::LinearScan::DumpInterval(std::ostream& stream,
                                                           LiveInterval* interval) const {
  interval->Dump(stream);
  stream << ": ";
  if (interval->HasRegisters()) {
    const char* delim = "";
    for (uint32_t reg : LowToHighBits(interval->GetRegisters())) {
      stream << delim;
      delim = ",";
      if (interval->IsFloatingPoint()) {
        codegen_->DumpFloatingPointRegister(stream, reg);
      } else {
        codegen_->DumpCoreRegister(stream, reg);
      }
    }
  } else if (interval->IsFixed()) {
    DCHECK_EQ(interval->GetType(), DataType::Type::kVoid);
    size_t start = interval->GetFirstRange()->GetStart();
    bool blocked_for_call =
        instructions_from_positions_[start / kLivenessPositionsPerInstruction] != nullptr;
    stream << (blocked_for_call ? "block-for-call" : "block-special");
  } else {
    stream << "spilled";
  }
  stream << std::endl;
}

void RegisterAllocatorLinearScan::LinearScan::DumpAllIntervals(std::ostream& stream) const {
  stream << "inactive: " << std::endl;
  for (LiveInterval* inactive_interval : inactive_) {
    DumpInterval(stream, inactive_interval);
  }
  stream << "active: " << std::endl;
  for (LiveInterval* active_interval : active_) {
    DumpInterval(stream, active_interval);
  }
  stream << "unhandled: " << std::endl;
  for (LiveInterval* unhandled_interval : unhandled_) {
    DumpInterval(stream, unhandled_interval);
  }
  stream << "handled: " << std::endl;
  for (LiveInterval* handled_interval : handled_) {
    DumpInterval(stream, handled_interval);
  }
}

// By the book implementation of a linear scan register allocator.
void RegisterAllocatorLinearScan::LinearScan::Run() {
  uint32_t available_registers = com::android::art::flags::reg_alloc_no_output_overlap()
      ? available_registers_
      : 0u;  // Unused.
  uint32_t allocated_registers = 0u;
  size_t last_position = std::numeric_limits<size_t>::max();
  while (!unhandled_.empty()) {
    // Remove interval with the lowest start position from unhandled.
    LiveInterval* current = unhandled_.back();
    unhandled_.pop_back();

    // Make sure the interval is an expected state.
    DCHECK(!current->IsFixed() && !current->HasSpillSlot());
    // Make sure we are going in the right order.
    DCHECK(unhandled_.empty() || unhandled_.back()->GetStart() >= current->GetStart());

    size_t position = current->GetStart();
    if (position != last_position) {
      // Remember the inactive_ size here since the ones moved to inactive_ from
      // active_ below shouldn't need to be re-checked.
      size_t inactive_intervals_to_handle = inactive_.size();

      // Remove currently active intervals that are dead at this position.
      // Move active intervals that have a lifetime hole at this position to inactive.
      // Note: While the intent would be better expressed with `std::remove_if()`, we use
      // `std::copy_if()` with the opposite condition instead because it has a simpler
      // implementation and calls the somewhat complicated lambda only once. Simplified
      // code flow outweighs the benefits of avoiding some unnecessary pointer writes.
      auto active_kept_end = std::copy_if(
          active_.begin(),
          active_.end(),
          active_.begin(),
          [this, position, &available_registers](LiveInterval* interval) {
            if (interval->IsDeadAt(position)) {
              handled_.push_back(interval);
            } else if (!interval->Covers(position)) {
              inactive_.push_back(interval);
            } else {
              return true;  // Keep this interval.
            }
            if (com::android::art::flags::reg_alloc_no_output_overlap()) {
              available_registers |= GetRegisterMask(interval);
              DCHECK_EQ(available_registers & ~available_registers_, 0u);
            }
            return false;
          });
      active_.erase(active_kept_end, active_.end());

      // Remove currently inactive intervals that are dead at this position.
      // Move inactive intervals that cover this position to active.
      // Note: Using `std::copy_if()` instead of `std::remove_if()` as above.
      auto inactive_to_handle_end = inactive_.begin() + inactive_intervals_to_handle;
      auto inactive_kept_end = std::copy_if(
          inactive_.begin(),
          inactive_to_handle_end,
          inactive_.begin(),
          [this, position, &available_registers](LiveInterval* interval) {
            DCHECK(interval->GetStart() < position || interval->IsFixed());
            if (interval->IsDeadAt(position)) {
              handled_.push_back(interval);
              return false;
            } else if (interval->Covers(position)) {
              active_.push_back(interval);
              if (com::android::art::flags::reg_alloc_no_output_overlap()) {
                available_registers &= ~GetRegisterMask(interval);
              }
              return false;
            } else {
              return true;  // Keep this interval.
            }
          });
      inactive_.erase(inactive_kept_end, inactive_to_handle_end);

      last_position = position;
    } else {
      // Active and inactive intervals should not change for the same position.
      DCHECK(std::none_of(active_.begin(),
                          active_.end(),
                          [position](LiveInterval* interval) {
                            return interval->IsDeadAt(position) || !interval->Covers(position);
                          }));
      DCHECK(std::none_of(inactive_.begin(),
                          inactive_.end(),
                          [position](LiveInterval* interval) {
                            return interval->IsDeadAt(position) || interval->Covers(position);
                          }));
    }

    // For a Phi which has all inputs in the same spill slot as its spill slot hint, use that hint.
    if (com::android::art::flags::reg_alloc_spill_slot_reuse() &&
        current->HasSpillSlotHint() &&
        TryUsingSpillSlotHint(current)) {
      continue;
    }

    // Try to find an available register, or two registers for a pair interval.
    // Note: We cannot check for available aligned register pairs here (the only pairs
    // that can be returned by `FindAvailableRegisterPair()`) because the interval can
    // have an unaligned register pair already set, or provided as a hint.
    auto has_available_registers = [=]() ALWAYS_INLINE {
      // Use branch-less expression to clear lowest bit (if any) for pairs.
      uint32_t adjusted_available_registers =
          available_registers & (available_registers - (current->IsPair() ? 1u : 0u));
      return adjusted_available_registers != kNoRegisters;
    };
    bool success = false;
    if (com::android::art::flags::reg_alloc_no_output_overlap() && !has_available_registers()) {
      // Not enough registers for `TryAllocateFreeReg()` to succeed.
      DCHECK(!current->HasRegisters());
      DCHECK(!TryAllocateFreeReg(current, available_registers));
    } else {
      success = TryAllocateFreeReg(current, available_registers);
    }

    // If no register could be found, we need to spill.
    if (!success) {
      uint32_t evicted_registers;
      std::tie(success, evicted_registers) = AllocateBlockedReg(current);
      if (com::android::art::flags::reg_alloc_no_output_overlap()) {
        available_registers |= evicted_registers;
        DCHECK_EQ(available_registers & ~available_registers_, 0u);
      }
    }

    // If the interval had a register allocated, add it to the list of active intervals.
    if (success) {
      allocated_registers |= current->GetRegisters();
      active_.push_back(current);
      if (com::android::art::flags::reg_alloc_no_output_overlap()) {
        DCHECK_EQ(available_registers & current->GetRegisters(), current->GetRegisters());
        available_registers &= ~current->GetRegisters();
      }
    }
  }
  codegen_->AddAllocatedRegisterSet(register_type_, allocated_registers);
}

static uint32_t FreeIfNotCoverAt(
    LiveInterval* interval, size_t position, ArrayRef<size_t> free_until) {
  // Note that the same instruction may occur multiple times in the input list,
  // so `free_until` may have changed already.
  // Since `position` is not the current scan position, we need to use CoversSlow.
  if (interval->IsDeadAt(position)) {
    // Set the register to be free. Note that inactive intervals might later
    // update the corresponding `free_until[.]`.
    DCHECK_EQ(free_until[interval->GetRegisterOrLowRegister()], kMaxLifetimePosition);
    DCHECK_IMPLIES(interval->IsPair(),
                   free_until[interval->GetHighRegister()] == kMaxLifetimePosition);
    return interval->GetRegisters();
  } else if (!interval->CoversSlow(position)) {
    // The interval becomes inactive at `defined_by`. We make its register
    // available only until the next use strictly after `defined_by`.
    uint32_t reg = interval->GetRegisterOrLowRegister();
    free_until[reg] = interval->FirstUseAfter(position);
    if (interval->IsPair()) {
      free_until[interval->GetHighRegister()] = free_until[reg];
    }
    return interval->GetRegisters();
  } else {
    return 0u;
  }
}

bool RegisterAllocatorLinearScan::LinearScan::TryUsingSpillSlotHint(LiveInterval* current) {
  DCHECK(current->HasSpillSlotHint());
  int hint = current->GetSpillSlotHint();
  DCHECK(current->GetDefinedBy() != nullptr);
  HBasicBlock* block = current->GetDefinedBy()->GetBlock();
  DCHECK(current->GetDefinedBy()->IsPhi());
  auto inputs = current->GetDefinedBy()->AsPhi()->GetInputs();
  DCHECK_EQ(inputs.size(), block->GetPredecessors().size());

  // Check if all inputs have the same spill slot as `hint` and that none of them
  // has a register allocated before the incoming edge.
  for (auto [predecessor, input_index] : ZipCount(block->GetPredecessors())) {
    DCHECK_EQ(predecessor->GetNormalSuccessors().size(), 1u);
    LiveInterval* input_li = inputs[input_index]->GetLiveInterval();
    if (input_li->GetSpillSlot() != hint ||
        input_li->GetSiblingAt(predecessor->GetLifetimeEnd() - 1)->HasRegisters()) {
      return false;
    }
  }

  // Check that the required spill slots are available at the start of the `current` interval.
  ScopedArenaVector<SpillSlotData>* spill_slots = GetSpillSlotsForType(current->GetType());
  size_t number_of_spill_slots_needed = current->NumberOfSpillSlotsNeeded();
  DCHECK_LE(hint + number_of_spill_slots_needed, spill_slots->size());
  DCHECK(current->GetParent() == current);
  size_t start = current->GetStart();
  size_t end = current->GetLastSibling()->GetEnd();
  ArrayRef<SpillSlotData> range =
      ArrayRef<SpillSlotData>(*spill_slots).SubArray(hint, number_of_spill_slots_needed);
  if (std::any_of(range.begin(),
                  range.end(),
                  [=](const SpillSlotData& data) { return data.GetEnd() > start; })) {
    return false;
  }

  // Use the spill slots and split the `current` interval if there is any register use.
  SpillSlotData new_data(current, end);
  for (SpillSlotData& data : range) {
    DCHECK_LE(data.GetEnd(), start);
    data = new_data;
  }
  current->SetSpillSlot(hint);
  DCHECK(!current->RequiresRegisterForDefinitionAt(current->GetStart()));
  size_t first_register_use = current->FirstRegisterUse();
  if (first_register_use != kNoLifetime) {
    LiveInterval* split = SplitBetween(current, current->GetStart(), first_register_use - 1);
    DCHECK(current != split);
    AddSorted(&unhandled_, split);
  }
  handled_.push_back(current);
  return true;
}

static int RegisterOrLowRegister(Location location) {
  return location.IsRegisterPair() ? location.low() : location.reg();
}

int RegisterAllocatorLinearScan::LinearScan::FindFirstRegisterHint(
    LiveInterval* interval, uint32_t available_registers, ArrayRef<size_t> free_until) const {
  if (interval->IsTemp()) {
    return kNoRegister;
  }

  if (interval->GetParent() == interval && interval->GetDefinedBy() != nullptr) {
    // This is the first interval for the instruction. Try to find
    // a register based on its definition.
    DCHECK_EQ(interval->GetDefinedBy()->GetLiveInterval(), interval);
    int hint = FindHintAtDefinition(interval);
    if (hint != kNoRegister && (available_registers & (1u << hint)) != 0u) {
      DCHECK_GT(free_until[hint], interval->GetStart());
      return hint;
    }
  }

  auto is_free_until = [=](int reg, size_t pos) ALWAYS_INLINE {
    return (available_registers & (1u << reg)) != 0u && free_until[reg] >= pos;
  };
  if (interval->IsSplit() &&
      SsaLivenessAnalysis::IsAtBlockBoundary(
          interval->GetStart() / kLivenessPositionsPerInstruction, instructions_from_positions_)) {
    // If the start of this interval is at a block boundary, we look at the
    // location of the interval in blocks preceding the block this interval
    // starts at. If one location is a register we return it as a hint. This
    // will avoid a move between the two blocks.
    HBasicBlock* block = SsaLivenessAnalysis::GetBlockFromPosition(
        interval->GetStart() / kLivenessPositionsPerInstruction, instructions_from_positions_);
    DCHECK(!interval->RequiresRegisterForDefinitionAt(interval->GetStart()));
    size_t next_register_use = interval->FirstRegisterUse();
    for (HBasicBlock* predecessor : block->GetPredecessors()) {
      size_t position = predecessor->GetLifetimeEnd() - 1;
      // We know positions above GetStart() do not have a location yet.
      if (position < interval->GetStart()) {
        LiveInterval* existing = interval->GetParent()->GetSiblingAt(position);
        if (existing != nullptr &&
            existing->HasRegisters() &&
            // It's worth using that register if it is available until the next use.
            is_free_until(existing->GetRegisterOrLowRegister(), next_register_use)) {
          return existing->GetRegisterOrLowRegister();
        }
      }
    }
  }

  size_t start = interval->GetStart();
  size_t end = interval->GetEnd();
  for (const UsePosition& use : interval->GetUses()) {
    size_t use_position = use.GetPosition();
    if (use_position > end) {
      break;
    }
    if (use_position >= start && !use.IsSynthesized()) {
      HInstruction* user = use.GetUser();
      size_t input_index = use.GetInputIndex();
      if (user->IsPhi()) {
        // If the phi has a register, try to use the same.
        DCHECK(interval->SameRegisterKind(*user->GetLiveInterval()));
        DCHECK_EQ(interval->IsPair(), user->GetLiveInterval()->IsPair());
        if (user->GetLiveInterval()->HasRegisters()) {
          int reg = user->GetLiveInterval()->GetRegisterOrLowRegister();
          if (is_free_until(reg, use_position)) {
            return reg;
          }
        }
        // If the instruction dies at the phi assignment, we can try having the
        // same register.
        if (end == user->GetBlock()->GetPredecessors()[input_index]->GetLifetimeEnd()) {
          HInputsRef inputs = user->GetInputs();
          for (size_t i = 0; i < inputs.size(); ++i) {
            if (i == input_index) {
              continue;
            }
            LiveInterval* sibling = inputs[i]->GetLiveInterval()->GetSiblingAt(
                user->GetBlock()->GetPredecessors()[i]->GetLifetimeEnd() - 1);
            DCHECK(sibling != nullptr);
            DCHECK(interval->SameRegisterKind(*sibling));
            DCHECK_EQ(interval->IsPair(), sibling->IsPair());
            if (sibling->HasRegisters()) {
              int reg = sibling->GetRegisterOrLowRegister();
              if (is_free_until(reg, use_position)) {
                return reg;
              }
            }
          }
        }
      } else {
        // If the instruction is expected in a register, try to use it.
        LocationSummary* locations = user->GetLocations();
        Location expected = locations->InAt(use.GetInputIndex());
        // We use the user's lifetime position - 1 (and not `use_position`) because the
        // register is blocked at the beginning of the user.
        size_t position = user->GetLifetimePosition() - 1;
        if (expected.IsRegisterKind()) {
          DCHECK(interval->SameRegisterKind(expected));
          int reg = RegisterOrLowRegister(expected);
          if (is_free_until(reg, position)) {
            return reg;
          }
        }
      }
    }
  }

  return kNoRegister;
}

int RegisterAllocatorLinearScan::LinearScan::FindHintAtDefinition(LiveInterval* interval) const {
  HInstruction* defined_by = interval->GetDefinedBy();
  DCHECK(defined_by != nullptr);
  if (defined_by->IsPhi()) {
    // Try to use the same register as one of the inputs.
    const ArenaVector<HBasicBlock*>& predecessors = defined_by->GetBlock()->GetPredecessors();
    HInputsRef inputs = defined_by->AsPhi()->GetInputs();
    for (size_t i = 0; i < inputs.size(); ++i) {
      size_t end = predecessors[i]->GetLifetimeEnd();
      LiveInterval* input_interval = inputs[i]->GetLiveInterval()->GetSiblingAt(end - 1);
      if (input_interval->GetEnd() == end) {
        // If the input dies at the end of the predecessor, we know its register can
        // be reused.
        DCHECK(interval->SameRegisterKind(*input_interval));
        DCHECK_EQ(interval->IsPair(), input_interval->IsPair());
        if (input_interval->HasRegisters()) {
          return input_interval->GetRegisterOrLowRegister();
        }
      }
    }
  } else {
    LocationSummary* locations = defined_by->GetLocations();
    Location out = locations->Out();
    if (out.IsUnallocated() && out.GetPolicy() == Location::kSameAsFirstInput) {
      // Try to use the same register as the first input.
      LiveInterval* input_interval =
          defined_by->InputAt(0)->GetLiveInterval()->GetSiblingAt(interval->GetStart() - 1);
      if (input_interval->GetEnd() == interval->GetStart()) {
        // If the input dies at the start of this instruction, we know its register can
        // be reused.
        DCHECK(interval->SameRegisterKind(*input_interval));
        DCHECK_EQ(interval->IsPair(), input_interval->IsPair());
        if (input_interval->HasRegisters()) {
          return input_interval->GetRegisterOrLowRegister();
        }
      }
    }
  }
  return kNoRegister;
}

// Find a free register. If multiple are found, pick the register that
// is free the longest.
bool RegisterAllocatorLinearScan::LinearScan::TryAllocateFreeReg(LiveInterval* current,
                                                                 uint32_t available_registers) {
  // First set all registers to be free.
  ArrayRef<size_t> free_until = GetRegistersArray();
  std::fill_n(free_until.begin(), free_until.size(), kMaxLifetimePosition);

  if (kIsDebugBuild || !com::android::art::flags::reg_alloc_no_output_overlap()) {
    uint32_t current_available_registers = available_registers_;
    // For each active interval, set its register(s) to not free.
    for (LiveInterval* interval : active_) {
      DCHECK(interval->HasRegisters() || interval->IsFixed());
      uint32_t register_mask = GetRegisterMask(interval);
      DCHECK_NE(register_mask, 0u);
      current_available_registers &= ~register_mask;
    }
    if (com::android::art::flags::reg_alloc_no_output_overlap()) {
      CHECK_EQ(available_registers, current_available_registers);
    } else {
      available_registers = current_available_registers;
    }
  }

  if (!com::android::art::flags::reg_alloc_no_output_overlap()) {
    // An interval that starts an instruction (that is, it is not split), may
    // re-use the registers used by the inputs of that instruciton, based on the
    // location summary.
    if (!current->IsTemp() && !current->IsSplit()) {
      HInstruction* defined_by = current->GetDefinedBy();
      DCHECK(defined_by != nullptr);
      LocationSummary* locations = defined_by->GetLocations();
      if (!locations->OutputCanOverlapWithInputs() && locations->Out().IsUnallocated()) {
        HInputsRef inputs = defined_by->GetInputs();
        for (size_t i = 0; i < inputs.size(); ++i) {
          if (locations->InAt(i).IsValid()) {
            // Take the last interval of the input. It is the location of that interval
            // that will be used at `defined_by`.
            LiveInterval* interval = inputs[i]->GetLiveInterval()->GetLastSibling();
            // Note that interval may have not been processed yet.
            // TODO: Handle non-split intervals last in the work list.
            if (interval->HasRegisters() && interval->SameRegisterKind(*current)) {
              // The input must be live until the end of `defined_by`, to comply to
              // the linear scan algorithm. So we use `defined_by`'s end lifetime
              // position to check whether the input is dead or is inactive after
              // `defined_by`.
              DCHECK(interval->CoversSlow(defined_by->GetLifetimePosition()));
              size_t position = defined_by->GetLifetimePosition() + kLivenessPositionOfNormalUse;
              available_registers |= FreeIfNotCoverAt(interval, position, free_until);
            }
          }
        }
      }
    }
  }

  // For each inactive interval, set its register to be free until
  // the next intersection with `current`.
  for (LiveInterval* inactive : inactive_) {
    // Temp/Slow-path-safepoint interval has no holes.
    DCHECK(!inactive->IsTemp());
    if (!current->IsSplit() && !inactive->IsFixed()) {
      // Neither current nor inactive are fixed.
      // Thanks to SSA, a non-split interval starting in a hole of an
      // inactive interval should never intersect with that inactive interval.
      // Only if it's not fixed though, because fixed intervals don't come from SSA.
      DCHECK_EQ(inactive->FirstIntersectionWith(current), kNoLifetime);
      continue;
    }

    DCHECK(inactive->HasRegisters() || inactive->IsFixed());
    uint32_t register_mask = GetRegisterMask(inactive);
    DCHECK_NE(register_mask, 0u);
    register_mask &= available_registers;
    if (register_mask != 0u) {
      size_t next_intersection = inactive->FirstIntersectionWith(current);
      if (next_intersection != kNoLifetime) {
        for (uint32_t reg : LowToHighBits(register_mask)) {
          free_until[reg] = std::min(free_until[reg], next_intersection);
        }
      }
    }
  }

  if (current->HasRegisters()) {
    // Some instructions have a fixed register output.
    DCHECK_EQ(available_registers & current->GetRegisters(), current->GetRegisters());
  } else {
    int hint = FindFirstRegisterHint(current, available_registers, free_until);
    if ((hint != kNoRegister) &&
        // For simplicity, if the hint we are getting for a pair cannot be used,
        // we are just going to allocate a new pair.
        !(current->IsPair() && (available_registers & (1u << GetHighForLowRegister(hint))) == 0u)) {
      DCHECK(!IsBlocked(hint));
    } else if (current->IsPair()) {
      hint = FindAvailableRegisterPair(available_registers, free_until, current->GetStart());
    } else {
      hint = FindAvailableRegister(available_registers, free_until, current);
    }
    // If we could not find a register (or pair), we need to spill.
    if (hint == kNoRegister) {
      return false;
    }
    DCHECK_EQ(GetHighForLowRegister(hint), hint + 1);
    uint32_t regs = (current->IsPair() ? 3u : 1u) << hint;
    DCHECK_EQ(available_registers & regs, regs);
    current->SetRegisters(regs);
  }

  DCHECK_NE(current->GetRegisters(), kNoRegisters);
  size_t free_end = free_until[current->GetRegisterOrLowRegister()];
  if (current->IsPair()) {
    free_end = std::min(free_end, free_until[current->GetHighRegister()]);
  }
  if (!current->IsDeadAt(free_end)) {
    // If the register is only available for a subset of live ranges
    // covered by `current`, split `current` before the position where
    // the register is not available anymore.
    LiveInterval* split = SplitBetween(current, current->GetStart(), free_end);
    DCHECK(split != nullptr);
    AddSorted(&unhandled_, split);
  }
  return true;
}

int RegisterAllocatorLinearScan::LinearScan::FindAvailableRegisterPair(
    uint32_t available_registers, ArrayRef<size_t> next_use, size_t starting_at) const {
  // Extract low registers of available pairs.
  available_registers = (available_registers & (available_registers >> 1)) & 0x55555555u;

  int reg = kNoRegister;
  // Pick the register pair that is used the last.
  for (size_t i : LowToHighBits(available_registers)) {
    DCHECK(IsLowRegister(i));
    DCHECK(!IsBlocked(i));
    int high_register = GetHighForLowRegister(i);
    DCHECK(!IsBlocked(high_register));
    int existing_high_register = GetHighForLowRegister(reg);
    if ((reg == kNoRegister) || (next_use[i] >= next_use[reg]
                        && next_use[high_register] >= next_use[existing_high_register])) {
      reg = i;
      if (next_use[i] == kMaxLifetimePosition
          && next_use[high_register] == kMaxLifetimePosition) {
        break;
      }
    } else if (next_use[reg] <= starting_at || next_use[existing_high_register] <= starting_at) {
      // If one of the current register is known to be unavailable, just unconditionally
      // try a new one.
      reg = i;
    }
  }
  return reg;
}

int RegisterAllocatorLinearScan::LinearScan::FindAvailableRegister(
    uint32_t available_registers, ArrayRef<size_t> next_use, LiveInterval* current) const {
  // We special case intervals that do not span a safepoint to try to find a caller-save
  // register if one is available. We iterate from 0 to the number of registers,
  // so if there are caller-save registers available at the end, we continue the iteration.
  bool prefers_caller_save_checked = false;
  int reg = kNoRegister;
  for (size_t i : LowToHighBits(available_registers)) {
    DCHECK(!IsBlocked(i));

    // Best case: we found a register fully available.
    if (next_use[i] == kMaxLifetimePosition) {
      // If we have previously checked that we prefer a caller-save register for this interval,
      // the `!HasWillCallSafepoint()` must be true, otherwise we would have left the loop.
      DCHECK_IMPLIES(prefers_caller_save_checked, !current->HasWillCallSafepoint(safepoints_));
      if (!IsCallerSaveRegister(i) &&
          (prefers_caller_save_checked || !current->HasWillCallSafepoint(safepoints_))) {
        prefers_caller_save_checked = true;
        // We can get shorter encodings on some platforms by using
        // small register numbers. So only update the candidate if the previous
        // one was not available for the whole method.
        if (reg == kNoRegister || next_use[reg] != kMaxLifetimePosition) {
          reg = i;
        }
        // Continue the iteration in the hope of finding a caller save register.
        continue;
      }
      // We know the register is good enough. Return it.
      reg = i;
      break;
    }

    // If we had no register before, take this one as a reference.
    if (reg == kNoRegister) {
      reg = i;
      continue;
    }

    // Pick the register that is used the last.
    if (next_use[i] > next_use[reg]) {
      reg = i;
      continue;
    }
  }
  return reg;
}

uint32_t RegisterAllocatorLinearScan::LinearScan::TrySplitNonPairOrUnalignedPairIntervalAt(
    size_t position, size_t first_register_use, ArrayRef<size_t> next_use) {
  for (auto it = active_.begin(), end = active_.end(); it != end; ++it) {
    LiveInterval* active = *it;
    // Special fixed intervals that represent multiple registers do not report having a register.
    if (active->IsFixed()) continue;
    DCHECK(active->HasRegisters());
    if (first_register_use > next_use[active->GetRegisterOrLowRegister()]) continue;

    // Split the first interval found that is either:
    // 1) A non-pair interval.
    // 2) A pair interval whose high is not low + 1.
    // 3) A pair interval whose low is not even.
    if (!active->IsPair() ||
        active->GetHighRegister() != active->GetRegisterOrLowRegister() + 1u ||
        !IsLowRegister(active->GetRegisterOrLowRegister())) {
      uint32_t evicted_registers = active->GetRegisters();
      DCHECK_NE(evicted_registers, kNoRegisters);
      LiveInterval* split = Split(active, position);
      if (split != active) {
        handled_.push_back(active);
      }
      active_.erase(it);
      AddSorted(&unhandled_, split);
      return evicted_registers;
    }
  }
  return kNoRegisters;
}

// Find the register that is used the last, and spill the interval
// that holds it. If the first use of `current` is after that register
// we spill `current` instead.
std::pair<bool, uint32_t> RegisterAllocatorLinearScan::LinearScan::AllocateBlockedReg(
    LiveInterval* current) {
  size_t first_register_use =
      (current->IsTemp() || current->RequiresRegisterForDefinitionAt(current->GetStart()))
          ? current->GetStart()
          : current->FirstRegisterUse();
  if (first_register_use == kNoLifetime) {
    AllocateSpillSlotFor(current);
    return {false, 0u};
  }

  // First set all registers as not being used.
  ArrayRef<size_t> next_use = GetRegistersArray();
  std::fill_n(next_use.begin(), next_use.size(), kMaxLifetimePosition);

  // For each active interval, find the next use of its register after the start of `current`.
  size_t start = current->GetStart();
  for (LiveInterval* active : active_) {
    uint32_t register_mask = GetRegisterMask(active);
    DCHECK_NE(register_mask, 0u);
    size_t use = start;
    if (active->HasRegisters()) {
      if (!active->IsFixed() &&
          !active->IsTemp() &&
          !active->RequiresRegisterForDefinitionAt(use)) {
        use = active->FirstRegisterUseAfter(use);
        if (use == kNoLifetime) {
          continue;
        }
      }
    } else {
      DCHECK(active->IsFixed());
    }
    for (uint32_t reg : LowToHighBits(register_mask)) {
      next_use[reg] = use;
    }
  }

  // For each inactive interval, find the next use of its register after the
  // start of current.
  for (LiveInterval* inactive : inactive_) {
    // Temp/Slow-path-safepoint interval has no holes.
    DCHECK(!inactive->IsTemp());
    if (!current->IsSplit() && !inactive->IsFixed()) {
      // Neither current nor inactive are fixed.
      // Thanks to SSA, a non-split interval starting in a hole of an
      // inactive interval should never intersect with that inactive interval.
      // Only if it's not fixed though, because fixed intervals don't come from SSA.
      DCHECK_EQ(inactive->FirstIntersectionWith(current), kNoLifetime);
      continue;
    }
    DCHECK(inactive->HasRegisters() || inactive->IsFixed());
    size_t next_intersection = inactive->FirstIntersectionWith(current);
    if (next_intersection != kNoLifetime) {
      size_t use = next_intersection;
      if (!inactive->IsFixed()) {
        use = inactive->FirstUseAfter(start);
        if (use == kNoLifetime) {
          continue;
        }
      }
      uint32_t register_mask = GetRegisterMask(inactive);
      DCHECK_NE(register_mask, 0u);
      for (uint32_t reg : LowToHighBits(register_mask)) {
        next_use[reg] = std::min(use, next_use[reg]);
      }
    }
  }

  int reg = kNoRegister;
  bool should_spill = false;
  if (current->IsPair()) {
    reg = FindAvailableRegisterPair(available_registers_, next_use, first_register_use);
    DCHECK_NE(reg, kNoRegister);
    // We should spill if both registers are not available.
    should_spill = (first_register_use >= next_use[reg]) ||
                   (first_register_use >= next_use[GetHighForLowRegister(reg)]);
  } else {
    reg = FindAvailableRegister(available_registers_, next_use, current);
    DCHECK_NE(reg, kNoRegister);
    should_spill = (first_register_use >= next_use[reg]);
  }

  if (should_spill) {
    bool is_allocation_at_use_site = (start >= (first_register_use - 1));
    if (is_allocation_at_use_site) {
      if (!current->IsPair()) {
        DumpInterval(std::cerr, current);
        DumpAllIntervals(std::cerr);
        // This situation has the potential to infinite loop, so we make it a non-debug CHECK.
        HInstruction* at =
            instructions_from_positions_[first_register_use / kLivenessPositionsPerInstruction];
        CHECK(false) << "There is not enough registers available for "
            << current->GetDefinedBy()->DebugName() << " "
            << current->GetDefinedBy()->GetId()
            << " at " << first_register_use - 1 << " "
            << (at == nullptr ? "" : at->DebugName());
      }

      // If we're allocating a register for `current` because the instruction at
      // that position requires it, but we think we should spill, then there are
      // non-pair intervals or unaligned pair intervals blocking the allocation.
      // We split the first interval found, and put ourselves first in the
      // `unhandled_` list.
      uint32_t evicted_registers =
          TrySplitNonPairOrUnalignedPairIntervalAt(start, first_register_use, next_use);
      DCHECK_NE(evicted_registers, kNoRegisters);
      unhandled_.push_back(current);
      return {false, evicted_registers};
    } else {
      // If the first use of that instruction is after the last use of the found
      // register, we split this interval just before its first register use.
      AllocateSpillSlotFor(current);
      LiveInterval* split = SplitBetween(current, start, first_register_use - 1);
      DCHECK(current != split);
      AddSorted(&unhandled_, split);
      return {false, 0u};
    }
  } else {
    // Use this register and spill the active and inactive intervals that have that register.
    DCHECK_EQ(GetHighForLowRegister(reg), reg + 1);
    uint32_t regs = (current->IsPair() ? 3u : 1u) << reg;
    current->SetRegisters(regs);
    uint32_t evicted_registers = 0u;
    static constexpr size_t kNothingToRemove = std::numeric_limits<size_t>::max();
    size_t active_to_remove_for_pair = kNothingToRemove;

    for (auto it = active_.begin(), end = active_.end(); it != end; ++it) {
      LiveInterval* active = *it;
      DCHECK_IMPLIES(active->IsFixed(), (GetRegisterMask(active) & regs) == 0u);
      if ((active->GetRegisters() & regs) != 0u) {
        DCHECK(!active->IsFixed());
        evicted_registers |= active->GetRegisters();
        LiveInterval* split = Split(active, start);
        if (split != active) {
          handled_.push_back(active);
        }
        if (com::android::art::flags::reg_alloc_no_output_overlap() &&
            start % kLivenessPositionsPerInstruction == kLivenessPositionOfNonOverlappingOutput) {
          // If we're splitting an active interval in the middle of instruction for non-overlapping
          // output, we cannot insert parallel moves in the middle of that instruction. Going back
          // to the start of the instruction would be difficult (bringing back already handled
          // inputs that die at the instruction, treating the output as live before its lifetime
          // starts), so we force spilling the active interval, avoiding a register for the second
          // half of the instruction's lifetime. If it has a register use later, we split it twice.
          // Note: This can result in suboptimal register allocation only if register pairs are
          // involved. For single-register intervals, the `split` would be spilled later anyway.
          DCHECK(split != active);
          DCHECK_EQ(start, split->GetStart());
          AllocateSpillSlotFor(split);
          handled_.push_back(split);
          DCHECK(!split->RequiresRegisterForDefinitionAt(start));
          size_t register_use = split->FirstRegisterUseAfter(start);
          if (register_use != kNoLifetime) {
            LiveInterval* second_split = SplitBetween(split, start, register_use - 1);
            DCHECK(second_split != split);
            DCHECK_LT(start, second_split->GetStart());
            AddSorted(&unhandled_, second_split);
          }
        } else {
          AddSorted(&unhandled_, split);
        }
        if ((regs & ~evicted_registers) != 0u) {
          // Delay removal of the `active` interval until we evict the other half or
          // process all active intervals (if the other half was not currently used).
          DCHECK(current->IsPair());
          DCHECK_EQ(active_to_remove_for_pair, kNothingToRemove);
          active_to_remove_for_pair = std::distance(active_.begin(), it);
        } else {
          DCHECK_IMPLIES(
              active_to_remove_for_pair != kNothingToRemove,
              active_to_remove_for_pair < static_cast<size_t>(std::distance(active_.begin(), it)));
          active_.erase(it);
          break;
        }
      }
    }
    if (active_to_remove_for_pair != kNothingToRemove) {
      active_.erase(active_.begin() + active_to_remove_for_pair);
    }

    // NOTE: Retrieve end() on each iteration because we're removing elements in the loop body.
    for (auto it = inactive_.begin(); it != inactive_.end(); ) {
      LiveInterval* inactive = *it;
      bool erased = false;
      if ((inactive->HasRegisters() || inactive->IsFixed()) &&
          (GetRegisterMask(inactive) & regs) != 0u) {
        if (!inactive->IsFixed() && !current->IsSplit()) {
          // Neither current nor inactive are fixed.
          // Thanks to SSA, a non-split interval starting in a hole of an
          // inactive interval should never intersect with that inactive interval.
          // Only if it's not fixed though, because fixed intervals don't come from SSA.
          DCHECK_EQ(inactive->FirstIntersectionWith(current), kNoLifetime);
        } else {
          size_t next_intersection = inactive->FirstIntersectionWith(current);
          if (next_intersection != kNoLifetime) {
            if (inactive->IsFixed()) {
              LiveInterval* split = Split(current, next_intersection);
              DCHECK_NE(split, current);
              AddSorted(&unhandled_, split);
            } else {
              // Split at the start of `current`, which will lead to splitting
              // at the end of the lifetime hole of `inactive`.
              LiveInterval* split = Split(inactive, start);
              // If it's inactive, it must start before the current interval.
              DCHECK_NE(split, inactive);
              it = inactive_.erase(it);
              erased = true;
              handled_.push_back(inactive);
              AddSorted(&unhandled_, split);
            }
          }
        }
      }
      // If we have erased the element, `it` already points to the next element.
      // Otherwise we need to move to the next element.
      if (!erased) {
        ++it;
      }
    }

    return {true, evicted_registers};
  }
}

void RegisterAllocatorLinearScan::AddSorted(ScopedArenaVector<LiveInterval*>* array,
                                            LiveInterval* interval) {
  DCHECK(!interval->IsFixed() && !interval->HasSpillSlot());
  size_t insert_at = 0;
  for (size_t i = array->size(); i > 0; --i) {
    LiveInterval* current = (*array)[i - 1u];
    // High intervals must be processed right after their low equivalent.
    if (current->StartsAfter(interval)) {
      insert_at = i;
      break;
    }
  }

  array->insert(array->begin() + insert_at, interval);
}

void RegisterAllocatorLinearScan::LinearScan::AllocateSpillSlotFor(LiveInterval* interval) {
  LiveInterval* parent = interval->GetParent();

  // An instruction gets a spill slot for its entire lifetime. If the parent
  // of this interval already has a spill slot, there is nothing to do.
  if (parent->HasSpillSlot()) {
    return;
  }

  HInstruction* defined_by = parent->GetDefinedBy();
  DCHECK_IMPLIES(defined_by->IsPhi(), !defined_by->AsPhi()->IsCatchPhi());

  if (defined_by->IsParameterValue()) {
    // Parameters have their own stack slot.
    parent->SetSpillSlot(codegen_->GetStackSlotOfParameter(defined_by->AsParameterValue()));
    return;
  }

  if (defined_by->IsCurrentMethod()) {
    parent->SetSpillSlot(0);
    return;
  }

  if (defined_by->IsConstant()) {
    // Constants don't need a spill slot.
    return;
  }

  ScopedArenaVector<SpillSlotData>* spill_slots = GetSpillSlotsForType(interval->GetType());
  size_t number_of_spill_slots_needed = parent->NumberOfSpillSlotsNeeded();
  size_t start = parent->GetStart();
  size_t end = interval->GetLastSibling()->GetEnd();
  size_t slot = 0;
  bool used_hint = false;

  if (com::android::art::flags::reg_alloc_spill_slot_reuse()) {
    LiveInterval* hint_phi_interval = parent->GetHintPhiInterval();
    // If the immediate hint Phi does not have a spill hint, we could try to follow the
    // hint Phi chain to a Phi that does. However, we would need to make sure we don't go
    // over a Phi loop forever. And we would need to investigate if the additional spill
    // slot sharing we can find this way is worth the increase in compilation time.
    if (hint_phi_interval != nullptr && hint_phi_interval->HasSpillSlotOrHint()) {
      size_t hint = hint_phi_interval->GetSpillSlotHint();
      DCHECK_LE(hint + number_of_spill_slots_needed, spill_slots->size());
      ArrayRef<SpillSlotData> range =
          ArrayRef<SpillSlotData>(*spill_slots).SubArray(hint, number_of_spill_slots_needed);
      if (std::all_of(range.begin(),
                      range.end(),
                      [=](SpillSlotData& data) { return data.CanUseFor(parent, start, end); })) {
        // Update slots and use the hint.
        for (SpillSlotData& data : range) {
          data.UseFor(interval, end);
        }
        used_hint = true;
        slot = hint;
      }
    }
  }

  // Find first available spill slots.
  if (!used_hint) {
    for (size_t e = spill_slots->size(); slot < e; ++slot) {
      bool found = true;
      for (size_t s = slot, u = std::min(slot + number_of_spill_slots_needed, e); s < u; s++) {
        if ((*spill_slots)[s].GetEnd() > start) {
          found = false;  // failure
          break;
        }
      }
      if (found) {
        break;  // success
      }
    }

    // Need new spill slots?
    SpillSlotData new_data(interval, end);
    size_t num_old_slots = spill_slots->size() - slot;
    if (num_old_slots < number_of_spill_slots_needed) {
      spill_slots->resize(slot + number_of_spill_slots_needed, new_data);
      // Update only old slots below.
      number_of_spill_slots_needed = num_old_slots;
    }

    // Set slots to end.
    for (size_t s : Range(slot, slot + number_of_spill_slots_needed)) {
      SpillSlotData& data = (*spill_slots)[s];
      DCHECK_LE(data.GetEnd(), start);
      data = new_data;
    }
  }

  // Note that the exact spill slot location will be computed when we resolve,
  // that is when we know the number of spill slots for each type.
  parent->SetSpillSlot(slot);

  if (com::android::art::flags::reg_alloc_spill_slot_reuse()) {
    LiveInterval* hint_phi_interval = parent->GetHintPhiInterval();
    while (hint_phi_interval != nullptr && !hint_phi_interval->HasSpillSlotOrHint()) {
      hint_phi_interval->SetSpillSlotHint(slot);
      hint_phi_interval = hint_phi_interval->GetHintPhiInterval();
    }
  }
}

void RegisterAllocatorLinearScan::AllocateSpillSlotForCatchPhi(HPhi* phi) {
  LiveInterval* interval = phi->GetLiveInterval();

  HInstruction* previous_phi = phi->GetPrevious();
  DCHECK(previous_phi == nullptr || previous_phi->AsPhi()->GetRegNumber() <= phi->GetRegNumber())
      << "Phis expected to be sorted by vreg number, so that equivalent phis are adjacent.";

  if (phi->IsVRegEquivalentOf(previous_phi)) {
    // This is an equivalent of the previous phi. We need to assign the same
    // catch phi slot.
    DCHECK(previous_phi->GetLiveInterval()->HasSpillSlot());
    interval->SetSpillSlot(previous_phi->GetLiveInterval()->GetSpillSlot());
  } else {
    // Allocate a new spill slot for this catch phi.
    // TODO: Reuse spill slots when intervals of phis from different catch
    //       blocks do not overlap.
    interval->SetSpillSlot(catch_phi_spill_slots_);
    catch_phi_spill_slots_ += interval->NumberOfSpillSlotsNeeded();
  }
}

}  // namespace art

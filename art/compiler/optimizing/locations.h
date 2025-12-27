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

#ifndef ART_COMPILER_OPTIMIZING_LOCATIONS_H_
#define ART_COMPILER_OPTIMIZING_LOCATIONS_H_

#include "base/arena_bit_vector.h"
#include "base/arena_containers.h"
#include "base/arena_object.h"
#include "base/array_ref.h"
#include "base/bit_field.h"
#include "base/bit_utils.h"
#include "base/bit_vector.h"
#include "base/casts.h"
#include "base/macros.h"
#include "base/value_object.h"
#include "physical_register_type.h"
#include "register_set.h"

namespace art HIDDEN {

class HConstant;
class HInstruction;
class Location;

std::ostream& operator<<(std::ostream& os, const Location& location);

/**
 * A Location is an abstraction over the potential location
 * of an instruction. It could be in register or stack.
 */
class Location : public ValueObject {
 public:
  enum OutputOverlap : uint8_t {
    // The liveness of the output overlaps the liveness of one or
    // several input(s); the register allocator cannot reuse an
    // input's location for the output's location.
    kOutputOverlap,
    // The liveness of the output does not overlap the liveness of any
    // input; the register allocator is allowed to reuse an input's
    // location for the output's location.
    kNoOutputOverlap
  };

  enum Kind {
    kInvalid = 0,
    kConstant = 1,
    kStackSlot = 2,  // 32bit stack slot.
    kDoubleStackSlot = 3,  // 64bit stack slot.
    kSIMDStackSlot = 4,  // 128bit stack slot. TODO: generalize with encoded #bytes?

    // We do not use the value 5 because it conflicts with kLocationConstantMask.
    kDoNotUse5 = 5,

    kCoreRegister = 6,  // Core register.
    kFpuRegister = 7,  // Float register.
    kVecRegister = 8,  // Vector register.

    // We do not use the value 9 because it conflicts with kLocationConstantMask.
    kDoNotUse9 = 9,

    kCoreRegisterPair = 10,  // Long register.
    kFpuRegisterPair = 11,  // Double register.

    // Unallocated location represents a location that is not fixed and can be
    // allocated by a register allocator.  Each unallocated location has
    // a policy that specifies what kind of location is suitable. Payload
    // contains register allocation policy.
    kUnallocated = 12,

    // We do not use the value 13 because it conflicts with kLocationConstantMask.
    kDoNotUse13 = 13,
  };

  static constexpr PhysicalRegisterType RegisterTypeForKind(Kind kind) {
    // The values in the `Kind` enumeration were selected in a way that makes
    // conversions between the `Kind` and `PhysicalRegisterType` simple.
    // The correctness is ensured by `static_assert()`s in `GetRegisterType()`.
    return enum_cast<PhysicalRegisterType>((kind - kCoreRegister) & 3u);
  }

  constexpr Location() : ValueObject(), value_(kInvalid) {
    // Verify that non-constant location kinds do not interfere with kConstant.
    static_assert((kInvalid & kLocationConstantMask) != kConstant, "TagError");
    static_assert((kStackSlot & kLocationConstantMask) != kConstant, "TagError");
    static_assert((kDoubleStackSlot & kLocationConstantMask) != kConstant, "TagError");
    static_assert((kSIMDStackSlot & kLocationConstantMask) != kConstant, "TagError");
    static_assert((kCoreRegister & kLocationConstantMask) != kConstant, "TagError");
    static_assert((kFpuRegister & kLocationConstantMask) != kConstant, "TagError");
    static_assert((kVecRegister & kLocationConstantMask) != kConstant, "TagError");
    static_assert((kCoreRegisterPair & kLocationConstantMask) != kConstant, "TagError");
    static_assert((kFpuRegisterPair & kLocationConstantMask) != kConstant, "TagError");
    static_assert((kUnallocated & kLocationConstantMask) != kConstant, "TagError");
    static_assert((kConstant & kLocationConstantMask) == kConstant, "TagError");

    DCHECK(!IsValid());
  }

  constexpr Location(const Location& other) = default;

  Location& operator=(const Location& other) = default;

  bool IsConstant() const {
    return (value_ & kLocationConstantMask) == kConstant;
  }

  static Location ConstantLocation(HInstruction* constant) {
    DCHECK(constant != nullptr);
    if (kIsDebugBuild) {
      // Call out-of-line helper to avoid circular dependency with `nodes.h`.
      DCheckInstructionIsConstant(constant);
    }
    return Location(kConstant | reinterpret_cast<uintptr_t>(constant));
  }

  HConstant* GetConstant() const {
    DCHECK(IsConstant());
    return reinterpret_cast<HConstant*>(value_ & ~kLocationConstantMask);
  }

  bool IsValid() const {
    return value_ != kInvalid;
  }

  bool IsInvalid() const {
    return !IsValid();
  }

  // Empty location. Used if there the location should be ignored.
  static constexpr Location NoLocation() {
    return Location();
  }

  // Register locations.
  static constexpr Location CoreRegister(int reg) {
    return Location(kCoreRegister, reg);
  }

  static constexpr Location FpuRegister(int reg) {
    return Location(kFpuRegister, reg);
  }

  static constexpr Location VecRegister(int reg) {
    return Location(kVecRegister, reg);
  }

  static constexpr Location CoreRegisterPair(int low, int high) {
    return Location(kCoreRegisterPair, low << 16 | high);
  }

  static constexpr Location FpuRegisterPair(int low, int high) {
    return Location(kFpuRegisterPair, low << 16 | high);
  }

  bool IsCoreRegister() const {
    return GetKind() == kCoreRegister;
  }

  bool IsFpuRegister() const {
    return GetKind() == kFpuRegister;
  }

  bool IsVecRegister() const {
    return GetKind() == kVecRegister;
  }

  bool IsCoreRegisterPair() const {
    return GetKind() == kCoreRegisterPair;
  }

  bool IsFpuRegisterPair() const {
    return GetKind() == kFpuRegisterPair;
  }

  bool IsRegister() const {
    return IsCoreRegister() || IsFpuRegister() || IsVecRegister();
  }

  bool IsRegisterPair() const {
    return IsCoreRegisterPair() || IsFpuRegisterPair();
  }

  bool IsRegisterKind() const {
    return IsRegister() || IsRegisterPair();
  }

  int reg() const {
    DCHECK(IsRegister());
    return GetPayload();
  }

  int low() const {
    DCHECK(IsRegisterPair());
    return GetPayload() >> 16;
  }

  int high() const {
    DCHECK(IsRegisterPair());
    return GetPayload() & 0xFFFF;
  }

  template <typename T>
  T AsRegister() const {
    DCHECK(IsCoreRegister());
    return static_cast<T>(reg());
  }

  template <typename T>
  T AsFpuRegister() const {
    DCHECK(IsFpuRegister());
    return static_cast<T>(reg());
  }

  template <typename T>
  T AsVecRegister() const {
    DCHECK(IsVecRegister());
    return static_cast<T>(reg());
  }

  template <typename T>
  T AsRegisterPairLow() const {
    DCHECK(IsCoreRegisterPair());
    return static_cast<T>(low());
  }

  template <typename T>
  T AsRegisterPairHigh() const {
    DCHECK(IsCoreRegisterPair());
    return static_cast<T>(high());
  }

  template <typename T>
  T AsFpuRegisterPairLow() const {
    DCHECK(IsFpuRegisterPair());
    return static_cast<T>(low());
  }

  template <typename T>
  T AsFpuRegisterPairHigh() const {
    DCHECK(IsFpuRegisterPair());
    return static_cast<T>(high());
  }

  Location ToLow() const {
    if (IsCoreRegisterPair()) {
      return CoreRegister(low());
    } else if (IsFpuRegisterPair()) {
      return FpuRegister(low());
    } else {
      DCHECK(IsDoubleStackSlot());
      return StackSlot(GetStackIndex());
    }
  }

  Location ToHigh() const {
    if (IsCoreRegisterPair()) {
      return CoreRegister(high());
    } else if (IsFpuRegisterPair()) {
      return FpuRegister(high());
    } else {
      DCHECK(IsDoubleStackSlot());
      return StackSlot(GetHighStackIndex(4));
    }
  }

  uint32_t GetRegisterSet() const {
    if (IsRegister()) {
      return 1u << reg();
    } else {
      DCHECK(IsRegisterPair());
      return (1u << low()) | (1u << high());
    }
  }

  PhysicalRegisterType GetRegisterType() const {
    DCHECK(IsRegisterKind());
    static_assert(RegisterTypeForKind(kCoreRegister) == PhysicalRegisterType::kCoreRegister);
    static_assert(RegisterTypeForKind(kFpuRegister) == PhysicalRegisterType::kFpuRegister);
    static_assert(RegisterTypeForKind(kVecRegister) == PhysicalRegisterType::kVectorRegister);
    static_assert(RegisterTypeForKind(kCoreRegisterPair) == PhysicalRegisterType::kCoreRegister);
    static_assert(RegisterTypeForKind(kFpuRegisterPair) == PhysicalRegisterType::kFpuRegister);
    return RegisterTypeForKind(GetKind());
  }

  static uintptr_t EncodeStackIndex(intptr_t stack_index) {
    DCHECK(-kStackIndexBias <= stack_index);
    DCHECK(stack_index < kStackIndexBias);
    return static_cast<uintptr_t>(kStackIndexBias + stack_index);
  }

  static Location StackSlot(intptr_t stack_index) {
    uintptr_t payload = EncodeStackIndex(stack_index);
    Location loc(kStackSlot, payload);
    // Ensure that sign is preserved.
    DCHECK_EQ(loc.GetStackIndex(), stack_index);
    return loc;
  }

  bool IsStackSlot() const {
    return GetKind() == kStackSlot;
  }

  static Location DoubleStackSlot(intptr_t stack_index) {
    uintptr_t payload = EncodeStackIndex(stack_index);
    Location loc(kDoubleStackSlot, payload);
    // Ensure that sign is preserved.
    DCHECK_EQ(loc.GetStackIndex(), stack_index);
    return loc;
  }

  bool IsDoubleStackSlot() const {
    return GetKind() == kDoubleStackSlot;
  }

  static Location SIMDStackSlot(intptr_t stack_index) {
    uintptr_t payload = EncodeStackIndex(stack_index);
    Location loc(kSIMDStackSlot, payload);
    // Ensure that sign is preserved.
    DCHECK_EQ(loc.GetStackIndex(), stack_index);
    return loc;
  }

  bool IsSIMDStackSlot() const {
    return GetKind() == kSIMDStackSlot;
  }

  static Location StackSlotByNumOfSlots(size_t num_of_slots, int spill_slot) {
    DCHECK_NE(num_of_slots, 0u);
    switch (num_of_slots) {
      case 1u:
        return StackSlot(spill_slot);
      case 2u:
        return DoubleStackSlot(spill_slot);
      default:
        // Assume all other stack slot sizes correspond to SIMD slot size.
        return SIMDStackSlot(spill_slot);
    }
  }

  intptr_t GetStackIndex() const {
    DCHECK(IsStackSlot() || IsDoubleStackSlot() || IsSIMDStackSlot());
    // Decode stack index manually to preserve sign.
    return GetPayload() - kStackIndexBias;
  }

  intptr_t GetHighStackIndex(uintptr_t word_size) const {
    DCHECK(IsDoubleStackSlot());
    // Decode stack index manually to preserve sign.
    return GetPayload() - kStackIndexBias + word_size;
  }

  Kind GetKind() const {
    return IsConstant() ? kConstant : KindField::Decode(value_);
  }

  bool Equals(Location other) const {
    return value_ == other.value_;
  }

  bool Contains(Location other) const {
    if (Equals(other)) {
      return true;
    } else if (IsRegisterPair() || IsDoubleStackSlot()) {
      return ToLow().Equals(other) || ToHigh().Equals(other);
    }
    return false;
  }

  bool OverlapsWith(Location other) const {
    // Only check the overlapping case that can happen with our register allocation algorithm.
    bool overlap = Contains(other) || other.Contains(*this);
    if (kIsDebugBuild && !overlap) {
      // Note: These are also overlapping cases. But we are not able to handle them in
      // ParallelMoveResolverWithSwap. Make sure that we do not meet such case with our compiler.
      if ((IsRegisterPair() && other.IsRegisterPair()) ||
          (IsDoubleStackSlot() && other.IsDoubleStackSlot())) {
        DCHECK(!Contains(other.ToLow()));
        DCHECK(!Contains(other.ToHigh()));
      }
    }
    return overlap;
  }

  const char* DebugString() const {
    switch (GetKind()) {
      case kInvalid: return "I";
      case kConstant: return "C";
      case kStackSlot: return "S";
      case kDoubleStackSlot: return "DS";
      case kSIMDStackSlot: return "SIMD";
      case kCoreRegister: return "R";
      case kFpuRegister: return "F";
      case kVecRegister: return "V";
      case kCoreRegisterPair: return "RP";
      case kFpuRegisterPair: return "FP";
      case kUnallocated: return "U";
      case kDoNotUse5:  // fall-through
      case kDoNotUse9:
      case kDoNotUse13:
        LOG(FATAL) << "Should not use this location kind";
    }
    UNREACHABLE();
  }

  // Unallocated locations.
  enum Policy {
    kAny,
    kRequiresCoreRegister,
    kRequiresFpuRegister,
    kRequiresVecRegister,
    kSameAsFirstInput,
  };

  bool IsUnallocated() const {
    return GetKind() == kUnallocated;
  }

  static Location UnallocatedLocation(Policy policy) {
    return Location(kUnallocated, PolicyField::Encode(policy));
  }

  // Any free register is suitable to replace this unallocated location.
  static Location Any() {
    return UnallocatedLocation(kAny);
  }

  static Location RequiresCoreRegister() {
    return UnallocatedLocation(kRequiresCoreRegister);
  }

  static Location RequiresFpuRegister() {
    return UnallocatedLocation(kRequiresFpuRegister);
  }

  static Location RequiresVecRegister() {
    return UnallocatedLocation(kRequiresVecRegister);
  }

  static Location RegisterOrConstant(HInstruction* instruction);
  static Location RegisterOrInt32Constant(HInstruction* instruction);
  static Location ByteRegisterOrConstant(int reg, HInstruction* instruction);
  static Location FpuRegisterOrConstant(HInstruction* instruction);
  static Location FpuRegisterOrInt32Constant(HInstruction* instruction);

  // The location of the first input to the instruction will be
  // used to replace this unallocated location.
  static Location SameAsFirstInput() {
    return UnallocatedLocation(kSameAsFirstInput);
  }

  Policy GetPolicy() const {
    DCHECK(IsUnallocated());
    return PolicyField::Decode(GetPayload());
  }

  bool RequiresRegisterKind() const {
    return GetPolicy() == kRequiresCoreRegister ||
           GetPolicy() == kRequiresFpuRegister ||
           GetPolicy() == kRequiresVecRegister;
  }

  uintptr_t GetEncoding() const {
    return GetPayload();
  }

 private:
  // Number of bits required to encode Kind value.
  static constexpr uint32_t kBitsForKind = 4;
  static constexpr uint32_t kBitsForPayload = kBitsPerIntPtrT - kBitsForKind;
  static constexpr uintptr_t kLocationConstantMask = 0x3;

  explicit Location(uintptr_t value) : value_(value) {}

  constexpr Location(Kind kind, uintptr_t payload)
      : value_(KindField::Encode(kind) | PayloadField::Encode(payload)) {}

  uintptr_t GetPayload() const {
    return PayloadField::Decode(value_);
  }

  static void DCheckInstructionIsConstant(HInstruction* instruction);

  using KindField = BitField<Kind, 0, kBitsForKind>;
  using PayloadField = BitField<uintptr_t, kBitsForKind, kBitsForPayload>;

  // Layout for kUnallocated locations payload.
  using PolicyField = BitField<Policy, 0, 3>;

  // Layout for stack slots.
  static const intptr_t kStackIndexBias =
      static_cast<intptr_t>(1) << (kBitsForPayload - 1);

  // Location either contains kind and payload fields or a tagged handle for
  // a constant locations. Values of enumeration Kind are selected in such a
  // way that none of them can be interpreted as a kConstant tag.
  uintptr_t value_;
};
std::ostream& operator<<(std::ostream& os, Location::Kind rhs);
std::ostream& operator<<(std::ostream& os, Location::Policy rhs);

static constexpr bool kIntrinsified = true;

/**
 * The code generator computes LocationSummary for each instruction so that
 * the instruction itself knows what code to generate: where to find the inputs
 * and where to place the result.
 *
 * The intent is to have the code for generating the instruction independent of
 * register allocation. A register allocator just has to provide a LocationSummary.
 */
class LocationSummary : public ArenaObject<kArenaAllocLocationSummary> {
 public:
  enum CallKind : uint8_t {
    kNoCall,
    kCallOnMainAndSlowPath,
    kCallOnSlowPath,
    kCallOnMainOnly
  };

  // The `Create` function is parametrized by the instruction type to allow devirtualizing
  // the underlying virtual call of the inlineable `instruction->InputCount()`.
  // Note: Making this template-parameter-dependent also helps us avoid `#include "nodes.h"`
  // which would lead to a circular dependency we would have to resolve.
  template <typename InstructionType>
  ALWAYS_INLINE
  static LocationSummary* Create(ArenaAllocator* allocator,
                                 InstructionType* instruction,
                                 CallKind call_kind,
                                 bool intrinsified = false) {
    return CreateImpl(allocator, instruction, call_kind, intrinsified, instruction->InputCount());
  }

  template <typename InstructionType>
  ALWAYS_INLINE
  static LocationSummary* CreateNoCall(ArenaAllocator* allocator,
                                       InstructionType* instruction,
                                       bool intrinsified = false) {
    return Create(allocator, instruction, kNoCall, intrinsified);
  }

  ArrayRef<Location> Inputs() {
    return ArrayRef<Location>(&inputs_[0], input_count_);
  }

  ArrayRef<const Location> Inputs() const {
    return ArrayRef<const Location>(&inputs_[0], input_count_);
  }

  void SetInAt(uint32_t at, Location location) {
    Inputs()[at] = location;
  }

  Location InAt(uint32_t at) const {
    return Inputs()[at];
  }

  size_t GetInputCount() const {
    return input_count_;
  }

  // Set the output location.  Argument `overlaps` tells whether the
  // output overlaps any of the inputs (if so, it cannot share the
  // same register as one of the inputs); it is set to
  // `Location::kOutputOverlap` by default for safety.
  void SetOut(Location location, Location::OutputOverlap overlaps = Location::kOutputOverlap) {
    DCHECK(output_.IsInvalid());
    output_overlaps_ = overlaps;
    output_ = location;
  }

  void UpdateOut(Location location) {
    // There are two reasons for updating an output:
    // 1) Parameters, where we only know the exact stack slot after
    //    doing full register allocation.
    // 2) Unallocated location.
    DCHECK(output_.IsStackSlot() || output_.IsDoubleStackSlot() || output_.IsUnallocated());
    output_ = location;
  }

  void AddTemp(Location location) {
    temps_.push_back(location);
  }

  void AddRegisterTemps(size_t count) {
    for (size_t i = 0; i < count; ++i) {
      AddTemp(Location::RequiresCoreRegister());
    }
  }

  Location GetTemp(uint32_t at) const {
    return temps_[at];
  }

  void SetTempAt(uint32_t at, Location location) {
    DCHECK(temps_[at].IsUnallocated() || temps_[at].IsInvalid());
    temps_[at] = location;
  }

  size_t GetTempCount() const {
    return temps_.size();
  }

  bool HasTemps() const { return !temps_.empty(); }

  Location Out() const { return output_; }

  bool CanCall() const {
    return call_kind_ != kNoCall;
  }

  bool WillCall() const {
    return call_kind_ == kCallOnMainOnly || call_kind_ == kCallOnMainAndSlowPath;
  }

  bool CallsOnSlowPath() const {
    return OnlyCallsOnSlowPath() || CallsOnMainAndSlowPath();
  }

  bool OnlyCallsOnSlowPath() const {
    return call_kind_ == kCallOnSlowPath;
  }

  bool NeedsSuspendCheckEntry() const {
    // Slow path calls do not need a SuspendCheck at method entry since they go into the runtime,
    // which we expect to either do a suspend check or return quickly.
    return WillCall();
  }

  bool CallsOnMainAndSlowPath() const {
    return call_kind_ == kCallOnMainAndSlowPath;
  }

  bool NeedsSafepoint() const {
    return CanCall();
  }

  void SetCustomSlowPathCallerSaves(const RegisterSet& caller_saves) {
    DCHECK(OnlyCallsOnSlowPath());
    call_data_->has_custom_slow_path_calling_convention = true;
    call_data_->custom_slow_path_caller_saves = caller_saves;
  }

  bool HasCustomSlowPathCallingConvention() const {
    // Meaningful only for `kCallOnSlowPath`. Allow checking also for `kCallOnMainAndSlowPath`.
    DCHECK(CallsOnSlowPath());
    DCHECK_IMPLIES(CallsOnMainAndSlowPath(), !call_data_->has_custom_slow_path_calling_convention);
    return call_data_->has_custom_slow_path_calling_convention;
  }

  const RegisterSet& GetCustomSlowPathCallerSaves() const {
    DCHECK(HasCustomSlowPathCallingConvention());
    return call_data_->custom_slow_path_caller_saves;
  }

  void SetStackBit(uint32_t index) {
    DCHECK(CanCall());
    call_data_->stack_mask.SetBit(index);
  }

  void ClearStackBit(uint32_t index) {
    DCHECK(CanCall());
    call_data_->stack_mask.ClearBit(index);
  }

  void SetRegisterBit(uint32_t reg_id) {
    DCHECK(CanCall());
    call_data_->register_mask |= (1 << reg_id);
  }

  uint32_t GetRegisterMask() const {
    DCHECK(CanCall());
    return call_data_->register_mask;
  }

  bool RegisterContainsObject(uint32_t reg_id) {
    DCHECK(CanCall());
    return RegisterSet::Contains(call_data_->register_mask, reg_id);
  }

  BitVector* GetStackMask() const {
    DCHECK(CanCall());
    return &call_data_->stack_mask;
  }

  RegisterSet* GetLiveRegisters() {
    DCHECK(CanCall());
    return &call_data_->live_registers;
  }

  size_t GetNumberOfLiveRegisters() const {
    DCHECK(CanCall());
    return call_data_->live_registers.GetNumberOfRegisters();
  }

  bool OutputUsesSameAs(uint32_t input_index) const {
    return (input_index == 0)
        && output_.IsUnallocated()
        && (output_.GetPolicy() == Location::kSameAsFirstInput);
  }

  bool IsFixedInput(uint32_t input_index) const {
    Location input = Inputs()[input_index];
    return input.IsCoreRegister()
        || input.IsFpuRegister()
        || input.IsRegisterPair()
        || input.IsStackSlot()
        || input.IsDoubleStackSlot();
  }

  bool OutputCanOverlapWithInputs() const {
    return output_overlaps_ == Location::kOutputOverlap;
  }

  bool Intrinsified() const {
    return intrinsified_;
  }

 private:
  struct CallData : public ArenaObject<kArenaAllocLocationSummary> {
   public:
    explicit CallData(ArenaAllocator* allocator);

    // Mask of objects that live in the stack.
    ArenaBitVector stack_mask;

    // Whether the slow path has default or custom calling convention.
    bool has_custom_slow_path_calling_convention;

    // Mask of objects that live in register.
    uint32_t register_mask;

    // Registers that are in use at this position.
    RegisterSet live_registers;

    // Custom slow path caller saves. Valid only if indicated by
    // `has_custom_slow_path_calling_convention`.
    RegisterSet custom_slow_path_caller_saves;
  };

  LocationSummary(HInstruction* instruction,
                  CallKind call_kind,
                  bool intrinsified,
                  ArenaAllocator* allocator,
                  size_t input_count);

  static LocationSummary* CreateImpl(ArenaAllocator* allocator,
                                     HInstruction* instruction,
                                     CallKind call_kind,
                                     bool intrinsified,
                                     size_t input_count);

  ArenaVector<Location> temps_;
  Location output_;

  // Data asociated with a call, null for `kNoCall`.
  CallData* call_data_;

  const CallKind call_kind_;
  // Whether these are locations for an intrinsified call.
  const bool intrinsified_;
  // Whether the output overlaps with any of the inputs. If it overlaps, then it cannot
  // share the same register as the inputs.
  Location::OutputOverlap output_overlaps_;

  // The number of inputs.
  const uint32_t input_count_;

  // Inputs array allocated together with the `LocationSummary`.
  Location inputs_[0];

  friend class RegisterAllocatorTest;
  DISALLOW_COPY_AND_ASSIGN(LocationSummary);
};

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_LOCATIONS_H_

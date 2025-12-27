/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "fast_compiler.h"

// TODO(VIXL): Make VIXL compile cleanly with -Wshadow, -Wdeprecated-declarations.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "aarch64/disasm-aarch64.h"
#include "aarch64/macro-assembler-aarch64.h"
#include "aarch64/disasm-aarch64.h"
#pragma GCC diagnostic pop

#include "code_generation_data.h"
#include "code_generator_arm64.h"
#include "data_type-inl.h"
#include "dex/bytecode_utils.h"
#include "dex/code_item_accessors-inl.h"
#include "dex/dex_file_exception_helpers.h"
#include "dex/dex_instruction-inl.h"
#include "driver/dex_compilation_unit.h"
#include "entrypoints/entrypoint_utils-inl.h"
#include "jit_patches_arm64.h"
#include "nodes.h"
#include "thread-inl.h"
#include "utils/arm64/assembler_arm64.h"


using namespace vixl::aarch64;  // NOLINT(build/namespaces)
using vixl::ExactAssemblyScope;
using vixl::CodeBufferCheckScope;
using vixl::EmissionCheckScope;

#ifdef __
#error "ARM64 Codegen VIXL macro-assembler macro already defined."
#endif
#define __ GetVIXLAssembler()->

namespace art HIDDEN {
namespace arm64 {

using helpers::CPURegisterFrom;
using helpers::HeapOperand;
using helpers::LocationFrom;
using helpers::StackOperandFrom;
using helpers::RegisterFrom;
using helpers::DRegisterFrom;
using helpers::SRegisterFrom;
using helpers::WRegisterFrom;
using helpers::XRegisterFrom;

// The maximum (meaningful) distance (31) that can be used in an integer shift/rotate operation.
static constexpr int32_t kMaxIntShiftDistance = 0x1f;

static const vixl::aarch64::Register kAvailableCalleeSaveRegisters[] = {
  vixl::aarch64::x22,
  vixl::aarch64::x23,
  vixl::aarch64::x24,
  vixl::aarch64::x25,
  vixl::aarch64::x26,
  vixl::aarch64::x27,
  vixl::aarch64::x28,
  vixl::aarch64::x29,
};

static const vixl::aarch64::Register kAvailableTempRegisters[] = {
  vixl::aarch64::x8,
  vixl::aarch64::x9,
  vixl::aarch64::x10,
  vixl::aarch64::x11,
  vixl::aarch64::x12,
  vixl::aarch64::x13,
  vixl::aarch64::x14,
  vixl::aarch64::x15,
};

static const vixl::aarch64::VRegister kAvailableCalleeSaveFpuRegisters[] = {
  vixl::aarch64::d8,
  vixl::aarch64::d9,
  vixl::aarch64::d10,
  vixl::aarch64::d11,
  vixl::aarch64::d12,
  vixl::aarch64::d13,
  vixl::aarch64::d14,
  vixl::aarch64::d15,
};

static const vixl::aarch64::VRegister kAvailableTempFpuRegisters[] = {
  vixl::aarch64::d16,
  vixl::aarch64::d17,
  vixl::aarch64::d18,
  vixl::aarch64::d19,
  vixl::aarch64::d20,
  vixl::aarch64::d21,
  vixl::aarch64::d22,
  vixl::aarch64::d23,
};

static constexpr size_t kMaximumRegisters = arraysize(kAvailableCalleeSaveRegisters);
static_assert(kMaximumRegisters == arraysize(kAvailableTempRegisters));
static_assert(kMaximumRegisters == arraysize(kAvailableTempFpuRegisters));
static_assert(kMaximumRegisters == arraysize(kAvailableCalleeSaveFpuRegisters));

// The core register or FPU register number we are going to use when a dex
// register is stored in stack, and the operation that stores to it needs a
// temporary register.
static constexpr size_t kResultRegisterForSpill = 0u;

class FastCompilerARM64 : public FastCompiler {
 public:
  FastCompilerARM64(ArtMethod* method,
                    ArenaAllocator* allocator,
                    ArenaStack* arena_stack,
                    VariableSizedHandleScope* handles,
                    const CompilerOptions& compiler_options,
                    const DexCompilationUnit& dex_compilation_unit)
      : method_(method),
        allocator_(allocator),
        handles_(handles),
        assembler_(allocator,
                   compiler_options.GetInstructionSetFeatures()->AsArm64InstructionSetFeatures()),
        jit_patches_(&assembler_, allocator),
        compiler_options_(compiler_options),
        dex_compilation_unit_(dex_compilation_unit),
        code_generation_data_(CodeGenerationData::Create(arena_stack, InstructionSet::kArm64)),
        processed_(ArenaBitVector::CreateFixedSize(
                       allocator_,
                       GetCodeItemAccessor().InsnsSizeInCodeUnits())),
        work_queue_(allocator->Adapter()),
        vreg_locations_(dex_compilation_unit.GetCodeItemAccessor().RegistersSize(),
                        allocator->Adapter()),
        branch_targets_(dex_compilation_unit.GetCodeItemAccessor().InsnsSizeInCodeUnits(),
                        allocator->Adapter()),
        object_register_masks_(dex_compilation_unit.GetCodeItemAccessor().InsnsSizeInCodeUnits(),
                               allocator->Adapter()),
        object_stack_masks_(dex_compilation_unit.GetCodeItemAccessor().InsnsSizeInCodeUnits(),
                               allocator->Adapter()),
        is_non_null_masks_(dex_compilation_unit.GetCodeItemAccessor().InsnsSizeInCodeUnits(),
                           allocator->Adapter()),
        catch_pcs_(ArenaBitVector::CreateFixedSize(
                       allocator,
                       dex_compilation_unit.GetCodeItemAccessor().InsnsSizeInCodeUnits())),
        branch_pcs_(ArenaBitVector::CreateFixedSize(
                             allocator,
                             dex_compilation_unit.GetCodeItemAccessor().InsnsSizeInCodeUnits())),
        catch_stack_maps_(allocator->Adapter()),
        has_frame_(false),
        needs_vreg_info_(false),
        core_spill_mask_(0u),
        fpu_spill_mask_(0u),
        object_register_mask_(0u),
        object_stack_mask_(allocator, /*start_bits=*/ 0, /*expandable=*/ true),
        is_non_null_mask_(0u) {
    ResetTempRegisters();
    ResetRegisterToSpill();
    memset(is_non_null_masks_.data(), ~0, is_non_null_masks_.size() * sizeof(uint64_t));
    memset(object_register_masks_.data(), ~0, object_register_masks_.size() * sizeof(uint64_t));
    memset(object_stack_masks_.data(), 0, object_stack_masks_.size() * sizeof(ArenaBitVector*));
    GetAssembler()->cfi().SetEnabled(compiler_options.GenerateAnyDebugInfo());
  }

  void ResetTempRegisters() {
    temp_core_register_ = 0u;
    temp_fpu_register_ = 0u;
  }

  uint32_t GetTempCoreRegister() {
    DCHECK(has_frame_);
    DCHECK_LT(temp_core_register_, kMaximumRegisters);
    return kAvailableTempRegisters[temp_core_register_++].GetCode();
  }

  uint32_t GetTempFpuRegister() {
    DCHECK(has_frame_);
    DCHECK_LT(temp_fpu_register_, kMaximumRegisters);
    return kAvailableTempFpuRegisters[temp_fpu_register_++].GetCode();
  }

  void ResetRegisterToSpill() {
    register_to_spill_ = std::make_pair(-1, DataType::Type::kVoid);
  }

  bool NeedsToSpill() const {
    return register_to_spill_.first != static_cast<uint32_t>(-1);
  }

  // Top-level method to generate code for `method_`.
  bool Compile(bool with_loop_support = false);

  ArrayRef<const uint8_t> GetCode() const override {
    return ArrayRef<const uint8_t>(assembler_.CodeBufferBaseAddress(), assembler_.CodeSize());
  }

  ScopedArenaVector<uint8_t> BuildStackMaps() const override {
    return code_generation_data_->GetStackMapStream()->Encode();
  }

  ArrayRef<const uint8_t> GetCfiData() const override {
    return ArrayRef<const uint8_t>(*assembler_.cfi().data());
  }

  int32_t GetFrameSize() const override {
    if (!has_frame_) {
      return 0;
    }
    uint16_t number_of_vregs = GetCodeItemAccessor().RegistersSize();
    uint16_t number_of_parameters = GetCodeItemAccessor().InsSize();
    uint32_t extra_stack_slots = 0u;
    if ((number_of_vregs - number_of_parameters) > kMaximumRegisters) {
      extra_stack_slots += (number_of_vregs - number_of_parameters - kMaximumRegisters);
    }
    size_t size = FrameEntrySpillSize() +
        /* method */ static_cast<size_t>(kArm64PointerSize) +
        extra_stack_slots * kVRegSize +
        GetCodeItemAccessor().OutsSize() * kVRegSize;
    return RoundUp(size, kStackAlignment);
  }

  uint32_t GetNumberOfJitRoots() const override {
    return code_generation_data_->GetNumberOfJitRoots();
  }

  void EmitJitRoots(uint8_t* code,
                    const uint8_t* roots_data,
                    /*out*/std::vector<Handle<mirror::Object>>* roots) override
       REQUIRES_SHARED(Locks::mutator_lock_) {
    code_generation_data_->EmitJitRoots(roots);
    jit_patches_.EmitJitRootPatches(code, roots_data, *code_generation_data_);
  }

  ~FastCompilerARM64() override {
    GetVIXLAssembler()->Reset();
  }

  const char* GetUnimplementedReason() const {
    return unimplemented_reason_;
  }

  bool ShouldRecompileWithLoopSupport() const {
    DCHECK(!branch_pcs_.IsAnyBitSet());
    return recompile_with_loop_support_;
  }

 private:
  // Go over each instruction of the method, and generate code for them.
  bool ProcessInstructions();
  bool ProcessBlock(uint32_t dex_pc);

  // Initialize the locations of parameters for this method.
  bool InitializeParameters();

  // Generate code for the frame entry, as well as the suspend check.
  // Only called when needed. If the frame entry has already been generated, do nothing.
  bool EnsureHasFrame();

  bool GenerateFrame();
  void GenerateSuspendCheck(uint32_t dex_pc);
  void GenerateSuspendCheckAndIncrementHotness(uint32_t dex_pc, Register temp);
  void IncrementHotness(Register method);

  // Generate code for a frame exit.
  void PopFrameAndReturn();

  // Record a stack map at the given dex_pc.
  void RecordPcInfo(uint32_t dex_pc);

  // Generate code to move from one location to another. Note that `hint_type`
  // is only used for the size of the type (64 or 32 bits), as some operations
  // come untyped and we default to integer for such operations. The
  // destination and the source will know the type.
  bool MoveLocation(Location destination, Location source, DataType::Type hint_type);

  // Get a register location for the dex register `reg`. Saves the location into
  // `vreg_locations_` for next uses of `reg`.
  // `next` should be the next dex instruction, to help choose the register.
  Location CreateNewRegisterLocation(uint32_t reg, DataType::Type type, const Instruction* next);
  Location CreateNewLocation(uint32_t reg, DataType::Type type);

  // Return the existing register location for `reg`.
  Location GetExistingRegisterLocation(uint32_t reg, DataType::Type type);

  // Move dex registers holding constants and fpu registers into core registers. Used when
  // branching.
  void MoveConstantsAndFpusToRegisters();

  // Reset all locations to core registers. Used for branch targets.
  void ResetLocations();

  // Handle the beginning of a branch target.
  void StartBranchTarget(bool flow_continues, uint32_t dex_pc);

  // Whether the branch target is initialized, which means we've already seen a
  // branch to it.
  bool BranchTargetIsInitialized(uint32_t dex_pc);

  // Update the masks associated to the given dex_pc. Used when dex_pc is a
  // branch target.
  void UpdateMasks(uint32_t dex_pc);

  // Generate code for one instruction.
  bool ProcessDexInstruction(const Instruction& instruction,
                             uint32_t dex_pc,
                             const Instruction* next);

  // Setup the arguments for an invoke.
  bool SetupArguments(InvokeType invoke_type,
                      const InstructionOperands& operands,
                      const char* shorty,
                      /* out */ uint32_t* obj_reg);

  // Generate code for doing a Java invoke.
  bool HandleInvoke(const Instruction& instruction, uint32_t dex_pc, InvokeType invoke_type);

  // Generate code for IF_* instructions.
  template<vixl::aarch64::Condition kCond, bool kCompareWithZero>
  bool If_21_22t(const Instruction& instruction, uint32_t dex_pc);

  // Generate code for doing a runtime invoke.
  void InvokeRuntime(QuickEntrypointEnum entrypoint, uint32_t dex_pc);
  bool BuildInvokeRuntime11x(
      QuickEntrypointEnum entrypoint, const Instruction& isntruction, uint32_t dex_pc);

  bool BuildLoadClass(uint32_t vreg, dex::TypeIndex type_index, const Instruction* next);
  bool BuildLoadString(uint32_t vreg, dex::StringIndex string_index, const Instruction* next);
  bool BuildNewInstance(
      uint32_t vreg, dex::TypeIndex string_index, uint32_t dex_pc, const Instruction* next);
  bool BuildNewArray(const Instruction& instruction, uint32_t dex_pc, const Instruction* next);
  bool BuildNewArray(dex::TypeIndex type_index, Location length_location, uint32_t dex_pc);
  bool BuildFilledNewArray(uint32_t dex_pc,
                           dex::TypeIndex type_index,
                           const InstructionOperands& operands);
  bool BuildCheckCast(uint32_t vreg, dex::TypeIndex type_index, uint32_t dex_pc);
  bool BuildInstanceOf(uint32_t vreg,
                       uint32_t vreg_result,
                       dex::TypeIndex type_index,
                       uint32_t dex_pc,
                       const Instruction* next);
  void SetIntConstant(uint32_t register_index, int32_t constant, const Instruction* next);
  void SetLongConstant(uint32_t register_index, int64_t constant, const Instruction* next);
  bool BuildMove(
      uint32_t dest_reg, uint32_t src_reg, DataType::Type type, const Instruction* next);
  bool LoadMethod(Register reg, ArtMethod* method);
  void DoReadBarrierOn(Register reg, vixl::aarch64::Label* exit = nullptr, bool do_mr_check = true);
  void DoWriteBarrierOn(Register holder,
                        UseScratchRegisterScope& temps,
                        bool overwrite_holder = false);
  bool DoGet(const MemOperand& mem,
             uint16_t field_index,
             Instruction::Code opcode,
             uint32_t dest_reg,
             bool can_receiver_be_null,
             bool is_object,
             bool is_volatile,
             uint32_t dex_pc,
             const Instruction* next);
  bool DoPut(const MemOperand& mem,
             Register holder,
             ArtField* field,
             Instruction::Code opcode,
             int32_t source_reg,
             bool can_receiver_be_null,
             bool is_object,
             bool is_volatile,
             uint32_t dex_pc);
  bool BuildArrayAccess(const Instruction& instruction,
                        uint32_t dex_pc,
                        bool is_put,
                        DataType::Type type,
                        const Instruction* next);
  bool BuildArrayLength(const Instruction& insttruction, uint32_t dex_pc, const Instruction* next);
  bool BuildCompare(const Instruction& instruction, const Instruction* next);
  bool BuildReturn(const Instruction& instruction);
  bool BuildMoveResult(const Instruction& instruction, bool is_object, const Instruction* next);
  bool BuildInstanceFieldGet(const Instruction& instruction,
                             uint32_t dex_pc,
                             bool is_object,
                             const Instruction* next);
  bool BuildInstanceFieldSet(const Instruction& instruction,
                             uint32_t dex_pc,
                             bool is_object);
  bool BuildStaticFieldAccess(const Instruction& instruction,
                              uint32_t dex_pc,
                              bool is_object,
                              bool is_put,
                              const Instruction* next);
  void Div(Register dst, Register first, Register second, uint32_t dex_pc);
  void Rem(Register dst, Register first, Register second, uint32_t dex_pc);
  bool Frem(CPURegister dst,
            CPURegister first,
            CPURegister second,
            uint32_t dex_pc,
            DataType::Type type);
  bool BuildSwitch(const Instruction& instruction, uint32_t dex_pc);
  bool BuildFillArrayData(const Instruction& instruction, uint32_t dex_pc);

  // Loop support
  bool ProcessDexInstructionForMasks(const Instruction& instruction);
  bool ProcessBlockForMasks(uint32_t dex_pc);
  bool UpdateObjectMasks(uint32_t dex_pc);
  bool AnalyzeBranches();

  void AddToWorkQueue(uint32_t dex_pc) {
    if (!processed_.IsBitSet(dex_pc)) {
      work_queue_.push(dex_pc);
    }
  }

  // Update registers and masks for the merge point.
  void PrepareToBranch(uint32_t dex_pc) {
    // We are going to branch, move all constants to registers to make the merge
    // point use the same locations.
    MoveConstantsAndFpusToRegisters();
    UpdateMasks(dex_pc);
    AddToWorkQueue(dex_pc);
  }

  // Mark whether dex register `vreg_index` is an object.
  void UpdateRegisterMask(uint32_t vreg_index, bool is_object, bool is_wide) {
    DCHECK(!(is_object && is_wide));
    // A `vreg_index` greater than kMaximumRegisters means the dex register is
    // stored in stack.
    if (vreg_index < kMaximumRegisters) {
      // Note that the register mask is only useful when there is a frame, so we
      // use the callee save registers for the mask.
      if (is_object) {
        object_register_mask_ |= (1 << kAvailableCalleeSaveRegisters[vreg_index].GetCode());
      } else {
        object_register_mask_ &= ~(1 << kAvailableCalleeSaveRegisters[vreg_index].GetCode());
      }
    } else {
      if (is_object) {
        object_stack_mask_.SetBit(GetStackSlot(vreg_index) / kVRegSize);
      } else {
        object_stack_mask_.ClearBit(GetStackSlot(vreg_index) / kVRegSize);
        if (is_wide) {
          object_stack_mask_.ClearBit(GetStackSlot(vreg_index + 1) / kVRegSize);
        }
      }
    }
  }

  // Mark whether dex register `vreg_index` can be null.
  void UpdateNonNullMask(uint32_t vreg_index, bool can_be_null) {
    if (can_be_null) {
      is_non_null_mask_ &= ~(1 << vreg_index);
    } else {
      is_non_null_mask_ |= (1 << vreg_index);
    }
  }

  // Update information about dex register `vreg_index`.
  void UpdateLocal(uint32_t vreg_index, bool is_object, bool is_wide, bool can_be_null = true) {
    DCHECK(!(is_object && is_wide));
    UpdateRegisterMask(vreg_index, is_object, is_wide);
    UpdateNonNullMask(vreg_index, can_be_null);
  }

  // Whether dex register `vreg_index` can be null.
  bool CanBeNull(uint32_t vreg_index) const {
    return (is_non_null_mask_ & (1 << vreg_index)) == 0;
  }

  // Get the label associated with the given `dex_pc`.
  vixl::aarch64::Label* GetLabelOf(uint32_t dex_pc) {
    return &branch_targets_[dex_pc];
  }

  // If we need to abort compilation, clear branch targets, required by vixl.
  void AbortCompilation() {
    for (vixl::aarch64::Label& label : branch_targets_) {
      if (label.IsLinked()) {
        __ Bind(&label);
      }
    }
  }

  bool IsParameter(uint32_t vreg) const {
    uint16_t number_of_vregs = GetCodeItemAccessor().RegistersSize();
    uint16_t number_of_parameters = GetCodeItemAccessor().InsSize();
    return vreg >= (number_of_vregs - number_of_parameters);
  }

  uint32_t GetStackSlot(uint32_t vreg) const {
    if (IsParameter(vreg)) {
      // Parameters are stored in the caller stack, as per our ABI.
      uint16_t number_of_vregs = GetCodeItemAccessor().RegistersSize();
      uint16_t number_of_parameters = GetCodeItemAccessor().InsSize();
      uint16_t first_parameter_index = number_of_vregs - number_of_parameters;
      return GetFrameSize() + sizeof(ArtMethod*) + (vreg - first_parameter_index) * kVRegSize;
    } else {
      return sizeof(ArtMethod*) +
          GetCodeItemAccessor().OutsSize() * kVRegSize +
          (vreg - kMaximumRegisters) * kVRegSize;
    }
  }

  // Compiler utilities.
  //
  Arm64Assembler* GetAssembler() { return &assembler_; }
  vixl::aarch64::MacroAssembler* GetVIXLAssembler() { return GetAssembler()->GetVIXLAssembler(); }
  const DexFile& GetDexFile() const { return *dex_compilation_unit_.GetDexFile(); }
  const CodeItemDataAccessor& GetCodeItemAccessor() const {
    return dex_compilation_unit_.GetCodeItemAccessor();
  }
  bool HitUnimplemented() const {
    return unimplemented_reason_ != nullptr;
  }

  // Frame related utilities.
  //
  uint32_t GetCoreSpillSize() const {
    return GetFramePreservedCoreRegisters().GetTotalSizeInBytes();
  }
  uint32_t FrameEntrySpillSize() const {
    return GetFramePreservedFPRegisters().GetTotalSizeInBytes() + GetCoreSpillSize();
  }
  CPURegList GetFramePreservedCoreRegisters() const {
    return CPURegList(CPURegister::kRegister, kXRegSize, core_spill_mask_);
  }
  CPURegList GetFramePreservedFPRegisters() const {
    return CPURegList(CPURegister::kVRegister, kDRegSize, fpu_spill_mask_);
  }

  bool IsBranchTarget(uint32_t dex_pc) const {
    return branch_pcs_.IsBitSet(dex_pc);
  }

  bool CanHandleBackwardsBranch(uint32_t dex_pc) {
    if (!IsBranchTarget(dex_pc)) {
      DCHECK(!branch_pcs_.IsAnyBitSet());
      unimplemented_reason_ = "Loop retry";
      recompile_with_loop_support_ = true;
      return false;
    }

    // Our loop analysis has already computed the object masks at this dex pc.
    DCHECK_NE(object_register_masks_[dex_pc], std::numeric_limits<uint64_t>::max()) << dex_pc;
    DCHECK_EQ(object_register_masks_[dex_pc],
              (object_register_masks_[dex_pc] & object_register_mask_)) << dex_pc;
    DCHECK(object_stack_masks_[dex_pc]->IsSubsetOf(&object_stack_mask_)) << dex_pc;
    return true;
  }

  // Method being compiled.
  ArtMethod* method_;

  // Allocator for any allocation happening in the compiler.
  ArenaAllocator* allocator_;

  VariableSizedHandleScope* handles_;

  // Compilation utilities.
  Arm64Assembler assembler_;
  JitPatchesARM64 jit_patches_;
  const CompilerOptions& compiler_options_;
  const DexCompilationUnit& dex_compilation_unit_;
  std::unique_ptr<CodeGenerationData> code_generation_data_;

  // A bit vector marking which instructions have already been processed.
  BitVectorView<size_t> processed_;

  // Work queue for the compiler, containing dex_pc to start the compilation.
  ArenaPriorityQueue<uint32_t, std::greater<uint32_t>> work_queue_;

  // The current location of each dex register.
  ArenaVector<Location> vreg_locations_;

  // A vector of size code units for dex pcs that are branch targets.
  ArenaVector<vixl::aarch64::Label> branch_targets_;

  // For dex pcs that are branch targets, the register mask and the stack mask that
  // will be used at the point of that pc.
  ArenaVector<uint64_t> object_register_masks_;
  ArenaVector<ArenaBitVector*> object_stack_masks_;

  // For dex pcs that are branch targets, the mask for non-null objects that will
  // be used at the point of that pc.
  ArenaVector<uint64_t> is_non_null_masks_;

  // Dex pcs that are catch targets.
  BitVectorView<size_t> catch_pcs_;

  // If we are compiling this method with loop support, the dex pcs that are branch targets.
  // Empty otherwise.
  BitVectorView<size_t> branch_pcs_;

  // Pair of {dex_pc, native_pc} collected during compilation, used when
  // generating stack map entries for catch instructions at the end of
  // compilation.
  ArenaVector<std::pair<uint32_t, uint32_t>> catch_stack_maps_;

  // Whether we've created a frame for this compiled method.
  bool has_frame_;

  // Whether we need dex register info in stack maps.
  bool needs_vreg_info_;

  // CPU registers that have been spilled in the frame.
  uint32_t core_spill_mask_;

  // FPU registers that have been spilled in the frame.
  uint32_t fpu_spill_mask_;

  // The current mask to know which core register holds an object.
  uint64_t object_register_mask_;

  // The current mask to know which stack entry holds an object.
  ArenaBitVector object_stack_mask_;

  // The current mask to know if a dex register is known non-null.
  uint64_t is_non_null_mask_;

  // At the end of an instruction, the register we need to spill to stack. -1 if
  // there is no register to spill.
  std::pair<uint32_t, DataType::Type> register_to_spill_;

  // Temporary registers for when we need to have dex registers in stack.
  uint32_t temp_core_register_;
  uint32_t temp_fpu_register_;

  // The return type of the compiled method. Saved to avoid re-computing it on
  // the return instruction.
  DataType::Type return_type_;

  // The return type of the last invoke instruction.
  DataType::Type previous_invoke_return_type_;

  // If non-empty, the reason the compilation could not be finished.
  const char* unimplemented_reason_ = nullptr;

  // Set to true when we hit a back edge and we haven't collected loop headers.
  bool recompile_with_loop_support_ = false;
};

bool FastCompilerARM64::InitializeParameters() {
  const char* shorty = dex_compilation_unit_.GetShorty();
  uint16_t number_of_vregs = GetCodeItemAccessor().RegistersSize();
  uint16_t number_of_parameters = GetCodeItemAccessor().InsSize();
  uint16_t vreg_parameter_index = number_of_vregs - number_of_parameters;
  bool needs_spill = false;

  if (number_of_vregs > kMaximumRegisters) {
    // Generate the frame, but don't emit the suspend check yet, as we haven't
    // updated the register and stack masks yet.
    if (!GenerateFrame()) {
      return false;
    }
    needs_spill = true;
  }

  InvokeDexCallingConventionVisitorARM64 convention;
  if (!dex_compilation_unit_.IsStatic()) {
    // Add the implicit 'this' argument, not expressed in the signature.
    vreg_locations_[vreg_parameter_index] = convention.GetNextLocation(DataType::Type::kReference);
    if (needs_spill) {
      Location new_location = CreateNewLocation(vreg_parameter_index, DataType::Type::kReference);
      DCHECK(vreg_locations_[vreg_parameter_index].IsCoreRegister());
      MoveLocation(new_location, vreg_locations_[vreg_parameter_index], DataType::Type::kReference);
      vreg_locations_[vreg_parameter_index] = new_location;
    }
    UpdateLocal(vreg_parameter_index,
                /* is_object= */ true,
                /* is_wide= */ false,
                /* can_be_null= */ false);
    ++vreg_parameter_index;
    --number_of_parameters;
  }

  for (int i = 0, shorty_pos = 1;
       i < number_of_parameters;
       i++, shorty_pos++, vreg_parameter_index++) {
    DataType::Type type = DataType::FromShorty(shorty[shorty_pos]);
    Location location = convention.GetNextLocation(type);
    if (location.IsDoubleStackSlot() || location.IsStackSlot()) {
      // We only handle single stack slots in the fast compiler. The
      // dex instructions know if they need two stack slot entries or one.
      location = Location::StackSlot(location.GetStackIndex() + GetFrameSize());
    }
    if (needs_spill) {
      Location new_location = CreateNewLocation(vreg_parameter_index, type);
      MoveLocation(new_location, location, type);
      vreg_locations_[vreg_parameter_index] = new_location;
    } else {
      vreg_locations_[vreg_parameter_index] = location;
    }
    bool is_wide = DataType::Is64BitType(type);
    UpdateLocal(vreg_parameter_index,
                /* is_object= */ (type == DataType::Type::kReference),
                is_wide,
                /* can_be_null= */ true);
    if (is_wide) {
      ++i;
      ++vreg_parameter_index;
    }
  }
  return_type_ = DataType::FromShorty(shorty[0]);

  if (needs_spill) {
    // We can now generate the suspend check.
    GenerateSuspendCheck(/* dex_pc= */ 0u);
  }

  if (branch_pcs_.IsAnyBitSet()) {
    // Generate the frame now. We don't want to create it lazily as the branch
    // instruction going backwards might branch to a dex pc lower than where the
    // frame was created.
    if (!EnsureHasFrame()) {
      return false;
    }
  }

  if (GetCodeItemAccessor().TriesSize() != 0) {
    if (!EnsureHasFrame()) {
      return false;
    }
    const uint8_t* handlers_ptr = GetCodeItemAccessor().GetCatchHandlerData();
    uint32_t handlers_size = DecodeUnsignedLeb128(&handlers_ptr);
    for (uint32_t idx = 0; idx < handlers_size; ++idx) {
      CatchHandlerIterator iterator(handlers_ptr);
      for (; iterator.HasNext(); iterator.Next()) {
        catch_pcs_.SetBit(iterator.GetHandlerAddress());
      }
      handlers_ptr = iterator.EndDataPointer();
    }
  }

  return true;
}

void FastCompilerARM64::MoveConstantsAndFpusToRegisters() {
  DCHECK(has_frame_);
  for (uint32_t i = 0; i < vreg_locations_.size(); ++i) {
    Location location  = vreg_locations_[i];
    if (location.IsConstant()) {
      DCHECK(location.GetConstant()->IsIntConstant() || location.GetConstant()->IsLongConstant());
      DataType::Type type = location.GetConstant()->IsIntConstant()
          ? DataType::Type::kInt32
          : DataType::Type::kInt64;
      vreg_locations_[i] = CreateNewLocation(i, type);
      MoveLocation(vreg_locations_[i], location, type);
      DCHECK(!HitUnimplemented());
    } else if (location.IsFpuRegister()) {
      vreg_locations_[i] = CreateNewLocation(i, DataType::Type::kInt64);
      MoveLocation(vreg_locations_[i], location, DataType::Type::kInt64);
      DCHECK(!HitUnimplemented());
    }
  }
}

void FastCompilerARM64::ResetLocations() {
  DCHECK(has_frame_);
  for (uint32_t i = 0; i < vreg_locations_.size(); ++i) {
    vreg_locations_[i] = CreateNewLocation(i, DataType::Type::kInt64);
  }
}

void FastCompilerARM64::UpdateMasks(uint32_t dex_pc) {
  object_register_masks_[dex_pc] &= object_register_mask_;
  if (object_stack_masks_[dex_pc] == nullptr) {
    object_stack_masks_[dex_pc] =
        new (allocator_) ArenaBitVector(allocator_, /*start_bits=*/ 0, /*expandable=*/ true);
    object_stack_masks_[dex_pc]->Copy(&object_stack_mask_);
  } else {
    object_stack_masks_[dex_pc]->Intersect(&object_stack_mask_);
  }
  is_non_null_masks_[dex_pc] &= is_non_null_mask_;
}

bool FastCompilerARM64::BranchTargetIsInitialized(uint32_t dex_pc) {
  // We always create a stack mask for branch targets.
  return object_stack_masks_[dex_pc] != nullptr;
}

void FastCompilerARM64::GenerateSuspendCheckAndIncrementHotness(uint32_t dex_pc, Register temp) {
  GenerateSuspendCheck(dex_pc);
  __ Ldr(temp, MemOperand(sp, 0));
  IncrementHotness(temp);
}

void FastCompilerARM64::StartBranchTarget(bool flow_continues, uint32_t dex_pc) {
  if (flow_continues) {
    // Emulate a branch to this pc.
    PrepareToBranch(dex_pc);
  } else {
    DCHECK(BranchTargetIsInitialized(dex_pc)) << "0x" << std::hex << dex_pc;
    // Otherwise reset locations to known locations.
    ResetLocations();
  }

  // Set new masks based on all incoming edges.
  if (IsBranchTarget(dex_pc)) {
    // Disable non-null optimizations at potential loop headers. This can avoid
    // the need to re-iterate for finding a fixed point for non-null masks.
    is_non_null_mask_ = 0u;
  } else {
    is_non_null_mask_ = is_non_null_masks_[dex_pc];
  }
  object_register_mask_ = object_register_masks_[dex_pc];
  DCHECK_NE(object_stack_masks_[dex_pc], nullptr);
  object_stack_mask_.Copy(object_stack_masks_[dex_pc]);
}

bool FastCompilerARM64::ProcessBlock(uint32_t dex_pc) {
  bool flow_continues = false;
  DexInstructionIterator it = GetCodeItemAccessor().InstructionsFrom(dex_pc).begin();
  DexInstructionIterator end = GetCodeItemAccessor().end();
  do {
    DexInstructionPcPair pair = *it;
    if (processed_.IsBitSet(pair.DexPc())) {
      if (flow_continues) {
        // If the previous instruction was flowing into its following
        // instruction, we need to branch where this following instruction
        // has been emitted.
        PrepareToBranch(pair.DexPc());
        __ B(GetLabelOf(pair.DexPc()));
      }
      break;
    }
    processed_.SetBit(pair.DexPc());
    ++it;

    // Fetch the next instruction as a micro-optimization currently only used
    // for optimizing returns.
    const Instruction* next = nullptr;
    if (it != end) {
      const DexInstructionPcPair& next_pair = *it;
      if (GetLabelOf(next_pair.DexPc())->IsLinked() ||
          IsBranchTarget(next_pair.DexPc()) ||
          catch_pcs_.IsBitSet(next_pair.DexPc())) {
        // Disable the micro-optimization, as the next instruction is a branch
        // target.
        next = nullptr;
      } else {
        next = &next_pair.Inst();
      }
    }

    vixl::aarch64::Label* label = GetLabelOf(pair.DexPc());
    bool is_catch = catch_pcs_.IsBitSet(pair.DexPc());
    bool is_branch_target = IsBranchTarget(pair.DexPc());
    bool is_linked = label->IsLinked();
    if (is_catch) {
      if (!BranchTargetIsInitialized(pair.DexPc())) {
        // The catch handler is in the normal flow (for example at dex pc 0), but
        // we need to know the masks at throwing instruction. Recompile with loop
        // support.
        unimplemented_reason_ = "Loop retry";
        recompile_with_loop_support_ = true;
        return false;
      }
      catch_stack_maps_.push_back(std::make_pair(pair.DexPc(), GetAssembler()->CodePosition()));
    }
    if (is_linked || is_branch_target || is_catch) {
      StartBranchTarget(flow_continues, pair.DexPc());
    }
    if (is_linked || is_branch_target) {
      __ Bind(label);
    }

    // If the instruction can throw, emulate a branch to each catch handler by
    // updating dex register masks.
    if (GetCodeItemAccessor().TriesSize() != 0 &&
        (Instruction::FlagsOf(pair.Inst().Opcode()) & Instruction::kThrow) != 0) {
      const dex::TryItem* try_item = GetCodeItemAccessor().FindTryItem(pair.DexPc());
      if (try_item != nullptr) {
        for (CatchHandlerIterator iterator(GetCodeItemAccessor(), *try_item);
             iterator.HasNext();
             iterator.Next()) {
          if (iterator.GetHandlerAddress() <= pair.DexPc() &&
              !CanHandleBackwardsBranch(iterator.GetHandlerAddress())) {
            return false;
          }
          UpdateMasks(iterator.GetHandlerAddress());
          AddToWorkQueue(iterator.GetHandlerAddress());
        }
      }
    }

    if (!ProcessDexInstruction(pair.Inst(), pair.DexPc(), next)) {
      DCHECK(HitUnimplemented());
      return false;
    }
    ResetTempRegisters();
    if (NeedsToSpill()) {
      Location stack_location =
          CreateNewLocation(register_to_spill_.first, register_to_spill_.second);
      Location reg_location = DataType::IsFloatingPointType(register_to_spill_.second)
          ? Location::FpuRegister(kResultRegisterForSpill)
          : Location::CoreRegister(kResultRegisterForSpill);
      MoveLocation(stack_location, reg_location, register_to_spill_.second);
      vreg_locations_[register_to_spill_.first] = stack_location;
      if (DataType::Is64BitType(register_to_spill_.second)) {
        DCHECK(vreg_locations_[register_to_spill_.first + 1].IsInvalid());
      }
      ResetRegisterToSpill();
    }
    // Note: There may be no Thread for gtests.
    DCHECK(Thread::Current() == nullptr || !Thread::Current()->IsExceptionPending())
        << GetDexFile().PrettyMethod(dex_compilation_unit_.GetDexMethodIndex())
        << " " << pair.Inst().Name() << "@" << pair.DexPc();

    DCHECK(!HitUnimplemented()) << GetUnimplementedReason();

    // For the next instruction, let it know if the previous instruction was
    // flowing through.
    flow_continues = pair.Inst().CanFlowThrough();
    if (!flow_continues) {
      break;
    }
  } while (it != end);
  return true;
}

bool FastCompilerARM64::ProcessInstructions() {
  DCHECK(GetCodeItemAccessor().HasCodeItem());

  AddToWorkQueue(0);
  while (!work_queue_.empty()) {
    uint32_t dex_pc = work_queue_.top();
    work_queue_.pop();
    if (!ProcessBlock(dex_pc)) {
      return false;
    }
  }
  return true;
}

bool FastCompilerARM64::MoveLocation(Location destination,
                                     Location source,
                                     DataType::Type hint_type) {
  if (source.Equals(destination)) {
    return true;
  }
  if (destination.IsCoreRegister()) {
    Register dst = RegisterFrom(destination, hint_type);
    if (source.IsCoreRegister()) {
      __ Mov(dst, RegisterFrom(source, hint_type));
      return true;
    }
    if (source.IsConstant()) {
      if (source.GetConstant()->IsIntConstant()) {
        // Note: the destination may be 64bits, but that's ok.
        __ Mov(dst, source.GetConstant()->AsIntConstant()->GetValue());
        return true;
      }
      DCHECK(source.GetConstant()->IsLongConstant());
      DCHECK(dst.Is64Bits());
      __ Mov(dst, source.GetConstant()->AsLongConstant()->GetValue());
      return true;
    }
    if (source.IsStackSlot()) {
      DCHECK_EQ(dst.Is64Bits(), DataType::Is64BitType(hint_type));
      __ Ldr(dst, StackOperandFrom(source));
      return true;
    }
    if (source.IsFpuRegister()) {
      // FPU to core register move (conversion).
      VRegister src = DataType::Is64BitType(hint_type)
          ? DRegisterFrom(source)
          : SRegisterFrom(source);
      __ Fmov(dst, src);
      return true;
    }
    unimplemented_reason_ = "UnimplementedSourceForRegisterDestination";
    return false;
  }

  if (destination.IsFpuRegister()) {
    VRegister dst = DataType::Is64BitType(hint_type)
        ? DRegisterFrom(destination)
        : SRegisterFrom(destination);
    if (source.IsFpuRegister()) {
      VRegister src = DataType::Is64BitType(hint_type)
          ? DRegisterFrom(source)
          : SRegisterFrom(source);
      __ Fmov(dst, src);
      return true;
    }
    if (source.IsStackSlot()) {
      DCHECK_EQ(dst.Is64Bits(), DataType::Is64BitType(hint_type));
      __ Ldr(dst, StackOperandFrom(source));
      return true;
    }
    if (source.IsCoreRegister()) {
      Register src = RegisterFrom(
          source, dst.Is64Bits() ? DataType::Type::kInt64 : DataType::Type::kInt32);
      __ Fmov(dst, src);
      return true;
    }
    if (source.IsConstant()) {
      if (source.GetConstant()->IsIntConstant()) {
        // Note: the destination may be 64bits, but that's ok.
        __ Fmov(dst, bit_cast<float, int32_t>(source.GetConstant()->AsIntConstant()->GetValue()));
        return true;
      }
      DCHECK(source.GetConstant()->IsLongConstant());
      DCHECK(dst.Is64Bits());
      __ Fmov(dst, bit_cast<double, int64_t>(source.GetConstant()->AsLongConstant()->GetValue()));
      return true;
    }
    unimplemented_reason_ = "UnimplementedSourceForFPURegisterDestination";
    return false;
  }

  if (destination.IsStackSlot()) {
    if (source.IsCoreRegister()) {
      DataType::Type src_type = DataType::Is64BitType(hint_type)
          ? DataType::Type::kInt64
          : DataType::Type::kInt32;
      Register src = RegisterFrom(source, src_type);
      __ Str(src, StackOperandFrom(destination));
      return true;
    }
    if (source.IsFpuRegister()) {
      VRegister src = DataType::Is64BitType(hint_type)
          ? DRegisterFrom(source)
          : SRegisterFrom(source);
      __ Str(src, StackOperandFrom(destination));
      return true;
    }
    if (source.IsConstant()) {
      UseScratchRegisterScope temps(GetVIXLAssembler());
      if (source.GetConstant()->IsIntConstant()) {
        DCHECK(destination.IsStackSlot());
        Register reg = temps.AcquireW();
        __ Mov(reg, source.GetConstant()->AsIntConstant()->GetValue());
        __ Str(reg, StackOperandFrom(destination));
        return true;
      }
      DCHECK(source.GetConstant()->IsLongConstant());
      Register reg = temps.AcquireX();
      __ Mov(reg, source.GetConstant()->AsLongConstant()->GetValue());
      __ Str(reg, StackOperandFrom(destination));
      return true;
    }
    DCHECK(source.IsStackSlot()) << source;
    UseScratchRegisterScope temps(GetVIXLAssembler());
    Register reg = DataType::Is64BitType(hint_type)
        ? temps.AcquireX()
        : temps.AcquireW();
    __ Ldr(reg, StackOperandFrom(source));
    __ Str(reg, StackOperandFrom(destination));
    return true;
  }

  unimplemented_reason_ = "UnimplementedDestinationLocation";
  return false;
}

Location FastCompilerARM64::CreateNewLocation(uint32_t reg, DataType::Type type) {
  DCHECK(has_frame_);
  if (reg >= kMaximumRegisters) {
    return Location::StackSlot(GetStackSlot(reg));
  }
  if (DataType::IsFloatingPointType(type)) {
    return Location::FpuRegister(kAvailableCalleeSaveFpuRegisters[reg].GetCode());
  }
  return Location::CoreRegister(kAvailableCalleeSaveRegisters[reg].GetCode());
}

Location FastCompilerARM64::CreateNewRegisterLocation(uint32_t reg,
                                                      DataType::Type type,
                                                      const Instruction* next) {
  if (DataType::Is64BitType(type)) {
    // To prevent branch points from moving dex registers which are used for
    // the other half of 64bit types, we invalidate those registers.
    vreg_locations_[reg + 1] = Location();
  }

  if (next != nullptr &&
      (next->Opcode() == Instruction::RETURN_OBJECT || next->Opcode() == Instruction::RETURN) &&
      (next->VRegA_11x() == reg)) {
    // If the next instruction is a return, use the return register from the calling
    // convention.
    InvokeDexCallingConventionVisitorARM64 convention;
    vreg_locations_[reg] = convention.GetReturnLocation(return_type_);
    return vreg_locations_[reg];
  }

  if (DataType::IsFloatingPointType(type)) {
    if (vreg_locations_[reg].IsFpuRegister()) {
      // Re-use existing register.
      return vreg_locations_[reg];
    }
    if (reg >= kMaximumRegisters) {
      DCHECK(has_frame_);
      DCHECK(!NeedsToSpill());
      register_to_spill_ = std::make_pair(reg, type);
      return Location::FpuRegister(kResultRegisterForSpill);
    }
    uint32_t register_code = has_frame_
        ? kAvailableCalleeSaveFpuRegisters[reg].GetCode()
        : kAvailableTempFpuRegisters[reg].GetCode();
    vreg_locations_[reg] = Location::FpuRegister(register_code);
    return vreg_locations_[reg];
  }

  if (vreg_locations_[reg].IsCoreRegister()) {
    // Re-use existing register.
    return vreg_locations_[reg];
  }

  if (reg >= kMaximumRegisters) {
    DCHECK(has_frame_);
    DCHECK(!NeedsToSpill());
    register_to_spill_ = std::make_pair(reg, type);
    return Location::CoreRegister(kResultRegisterForSpill);
  }

  uint32_t register_code = has_frame_
      ? kAvailableCalleeSaveRegisters[reg].GetCode()
      : kAvailableTempRegisters[reg].GetCode();
  vreg_locations_[reg] = Location::CoreRegister(register_code);
  return vreg_locations_[reg];
}

Location FastCompilerARM64::GetExistingRegisterLocation(uint32_t reg, DataType::Type type) {
  DCHECK(!vreg_locations_[reg].IsInvalid());
  if (DataType::IsFloatingPointType(type)) {
    if (vreg_locations_[reg].IsFpuRegister()) {
      return vreg_locations_[reg];
    }
    Location new_location;
    if (reg >= kMaximumRegisters) {
      DCHECK(has_frame_);
      new_location = Location::FpuRegister(GetTempFpuRegister());
      bool res = MoveLocation(new_location, vreg_locations_[reg], type);
      DCHECK(res);
    } else {
      uint32_t register_code = has_frame_
          ? kAvailableCalleeSaveFpuRegisters[reg].GetCode()
          : kAvailableTempFpuRegisters[reg].GetCode();
      new_location = Location::FpuRegister(register_code);
      bool res = MoveLocation(new_location, vreg_locations_[reg], type);
      DCHECK(res);
      vreg_locations_[reg] = new_location;
    }
    // The floating point value may have come from a zero constant, which we
    // treat as object. Therefore, remove the flag if it is there.
    UpdateLocal(reg, /* is_object= */ false, /* is_wide= */ DataType::Is64BitType(type));
    return new_location;
  }

  if (vreg_locations_[reg].IsCoreRegister()) {
    return vreg_locations_[reg];
  }

  Location new_location;
  if (reg >= kMaximumRegisters) {
    DCHECK(has_frame_);
    new_location = Location::CoreRegister(GetTempCoreRegister());
    bool res = MoveLocation(new_location, vreg_locations_[reg], type);
    DCHECK(res);
  } else {
    uint32_t register_code = has_frame_
        ? kAvailableCalleeSaveRegisters[reg].GetCode()
        : kAvailableTempRegisters[reg].GetCode();
    new_location = Location::CoreRegister(register_code);
    bool res = MoveLocation(new_location, vreg_locations_[reg], type);
    DCHECK(res);
    vreg_locations_[reg] = new_location;
  }
  return new_location;
}

void FastCompilerARM64::RecordPcInfo(uint32_t dex_pc) {
  DCHECK(has_frame_);
  uint32_t native_pc = GetAssembler()->CodePosition();
  StackMapStream* stack_map_stream = code_generation_data_->GetStackMapStream();
  CHECK_EQ(object_register_mask_ & callee_saved_core_registers.GetList(), object_register_mask_);
  ArenaBitVector* stack_mask = nullptr;
  if (object_stack_mask_.IsAnyBitSet()) {
    stack_mask =
        new (allocator_) ArenaBitVector(allocator_, /*start_bits=*/ 0, /*expandable=*/ true);
    stack_mask->Copy(&object_stack_mask_);
  }
  stack_map_stream->BeginStackMapEntry(dex_pc,
                                       native_pc,
                                       object_register_mask_,
                                       stack_mask,
                                       StackMap::Kind::Default,
                                       needs_vreg_info_);
  if (needs_vreg_info_) {
    using Kind = DexRegisterLocation::Kind;
    uint32_t size = vreg_locations_.size();
    for (uint32_t i = 0; i < size; ++i) {
      Location location = vreg_locations_[i];
      switch (location.GetKind()) {
        case Location::kConstant: {
          if (location.GetConstant()->IsLongConstant()) {
            int64_t value = location.GetConstant()->AsLongConstant()->GetValue();
            stack_map_stream->AddDexRegisterEntry(Kind::kConstant, Low32Bits(value));
            stack_map_stream->AddDexRegisterEntry(Kind::kConstant, High32Bits(value));
            ++i;
            DCHECK_LT(i, size);
          } else {
            DCHECK(location.GetConstant()->IsIntConstant());
            int32_t value = location.GetConstant()->AsIntConstant()->GetValue();
            stack_map_stream->AddDexRegisterEntry(Kind::kConstant, value);
          }
          break;
        }

        case Location::kStackSlot: {
          stack_map_stream->AddDexRegisterEntry(Kind::kInStack, location.GetStackIndex());
          // Note: if we were using the fast compiler for debuggable, we would
          // need to emit another `kInStack` here for long/double values. This would
          // require knowing if the current entry is a long/double.
          DCHECK(!compiler_options_.GetDebuggable());
          break;
        }

        case Location::kCoreRegister: {
          stack_map_stream->AddDexRegisterEntry(Kind::kInRegister, location.reg());
          // Note: if we were using the fast compiler for debuggable, we would
          // need to emit a `kInRegisterHi` here for long values. This would
          // require knowing if the current entry is a long.
          DCHECK(!compiler_options_.GetDebuggable());
          break;
        }

        case Location::kFpuRegister: {
          stack_map_stream->AddDexRegisterEntry(Kind::kInFpuRegister, location.reg());
          // Note: same as above and `kInFpuRegisterHi` for double values.
          DCHECK(!compiler_options_.GetDebuggable());
          break;
        }

        case Location::kInvalid: {
          stack_map_stream->AddDexRegisterEntry(Kind::kNone, 0);
          break;
        }

        default:
          LOG(FATAL) << "Unexpected kind " << location.GetKind();
      }
    }
  }
  stack_map_stream->EndStackMapEntry();
}

void FastCompilerARM64::PopFrameAndReturn() {
  if (has_frame_) {
    CodeGeneratorARM64::PopFrameAndReturn(GetAssembler(),
                                          GetFrameSize(),
                                          GetFramePreservedCoreRegisters(),
                                          GetFramePreservedFPRegisters());
  } else {
    DCHECK_EQ(GetFrameSize(), 0);
    __ Ret();
  }
}

bool FastCompilerARM64::EnsureHasFrame() {
  if (has_frame_) {
    // Frame entry has already been generated.
    return true;
  }
  if (!GenerateFrame()) {
    return false;
  }

  GenerateSuspendCheck(/* dex_pc= */ 0u);
  return true;
}

void FastCompilerARM64::IncrementHotness(Register method) {
  UseScratchRegisterScope temps(GetVIXLAssembler());
  Register counter = temps.AcquireW();
  vixl::aarch64::Label increment, done;
  uint32_t entrypoint_offset =
      GetThreadOffset<kArm64PointerSize>(kQuickCompileBaseline).Int32Value();

  __ Ldrh(counter, MemOperand(method, ArtMethod::HotnessCountOffset().Int32Value()));
  __ Cbnz(counter, &increment);
  __ Ldr(lr, MemOperand(tr, entrypoint_offset));
  // Note: we don't record the call here (and therefore don't generate a stack
  // map), as the entrypoint should never be suspended.
  __ Blr(lr);
  __ Bind(&increment);
  __ Add(counter, counter, -1);
  __ Strh(counter, MemOperand(method, ArtMethod::HotnessCountOffset().Int32Value()));
  __ Bind(&done);
}

bool FastCompilerARM64::GenerateFrame() {
  DCHECK(!has_frame_);
  has_frame_ = true;
  uint16_t number_of_vregs = GetCodeItemAccessor().RegistersSize();
  for (int i = 0; i < std::min(number_of_vregs, static_cast<uint16_t>(kMaximumRegisters)); ++i) {
    // Assume any vreg will be held in a callee-save register.
    core_spill_mask_ |= (1 << kAvailableCalleeSaveRegisters[i].GetCode());
    // TODO: do this lazily for floats, and recompile?
    fpu_spill_mask_ |= (1 << kAvailableCalleeSaveFpuRegisters[i].GetCode());
  }
  core_spill_mask_ |= (1 << lr.GetCode());

  code_generation_data_->GetStackMapStream()->BeginMethod(GetFrameSize(),
                                                          core_spill_mask_,
                                                          fpu_spill_mask_,
                                                          GetCodeItemAccessor().RegistersSize(),
                                                          /* is_compiling_baseline= */ false,
                                                          /* is_debuggable= */ false,
                                                          /* has_should_deoptimize_flag= */ false,
                                                          /* is_fast= */ true);
  MacroAssembler* masm = GetVIXLAssembler();
  {
    UseScratchRegisterScope temps(masm);
    Register temp = temps.AcquireX();
    __ Sub(temp, sp, static_cast<int32_t>(GetStackOverflowReservedBytes(InstructionSet::kArm64)));
    // Ensure that between load and RecordPcInfo there are no pools emitted.
    ExactAssemblyScope eas(GetVIXLAssembler(),
                           kInstructionSize,
                           CodeBufferCheckScope::kExactSize);
    __ ldr(wzr, MemOperand(temp, 0));
    RecordPcInfo(0);
  }

  CodeGeneratorARM64::GenerateFrame(GetAssembler(),
                                    GetFrameSize(),
                                    GetFramePreservedCoreRegisters(),
                                    GetFramePreservedFPRegisters(),
                                    /* requires_current_method= */ true);

  if (number_of_vregs <= kMaximumRegisters) {
    // Move registers which are currently allocated from caller-saves to callee-saves,
    // and adjust the offsets of stack locations.
    for (uint32_t i = 0; i < number_of_vregs; ++i) {
      if (vreg_locations_[i].IsCoreRegister()) {
        Location new_location =
            Location::CoreRegister(kAvailableCalleeSaveRegisters[i].GetCode());
        if (!MoveLocation(new_location, vreg_locations_[i], DataType::Type::kInt64)) {
          return false;
        }
        vreg_locations_[i] = new_location;
      } else if (vreg_locations_[i].IsFpuRegister()) {
        Location new_location =
            Location::FpuRegister(kAvailableCalleeSaveFpuRegisters[i].GetCode());
        if (!MoveLocation(new_location, vreg_locations_[i], DataType::Type::kFloat64)) {
          return false;
        }
        vreg_locations_[i] = new_location;
      } else if (vreg_locations_[i].IsStackSlot()) {
        DCHECK(IsParameter(i));
        vreg_locations_[i] =
            Location::StackSlot(vreg_locations_[i].GetStackIndex() + GetFrameSize());
        Location new_location =
            Location::CoreRegister(kAvailableCalleeSaveRegisters[i].GetCode());
        if (!MoveLocation(new_location, vreg_locations_[i], DataType::Type::kInt32)) {
          return false;
        }
        vreg_locations_[i] = new_location;
      } else if (vreg_locations_[i].IsConstant() || vreg_locations_[i].IsInvalid()) {
        // Nothing to do.
      } else {
        unimplemented_reason_ = "UnhandledLocation";
        return false;
      }
    }
  }

  // Increment hotness. We use the ArtMethod's counter as we're not allocating a
  // `ProfilingInfo` object in the fast baseline compiler.
  IncrementHotness(kArtMethodRegister);
  return true;
}

void FastCompilerARM64::GenerateSuspendCheck(uint32_t dex_pc) {
  MacroAssembler* masm = GetVIXLAssembler();
  if (compiler_options_.GetImplicitSuspendChecks()) {
    ExactAssemblyScope eas(masm, kInstructionSize, CodeBufferCheckScope::kExactSize);
    __ ldr(kImplicitSuspendCheckRegister, MemOperand(kImplicitSuspendCheckRegister));
    RecordPcInfo(dex_pc);
  } else {
    UseScratchRegisterScope temps(masm);
    Register temp = temps.AcquireW();
    vixl::aarch64::Label continue_label;
    __ Ldr(temp, MemOperand(tr, Thread::ThreadFlagsOffset<kArm64PointerSize>().SizeValue()));
    __ Tst(temp, Thread::SuspendOrCheckpointRequestFlags());
    __ B(eq, &continue_label);
    uint32_t entrypoint_offset =
        GetThreadOffset<kArm64PointerSize>(kQuickTestSuspend).Int32Value();
    __ Ldr(lr, MemOperand(tr, entrypoint_offset));
    {
      ExactAssemblyScope eas(masm, kInstructionSize, CodeBufferCheckScope::kExactSize);
      __ blr(lr);
      RecordPcInfo(dex_pc);
    }
    __ Bind(&continue_label);
  }
}


bool FastCompilerARM64::SetupArguments(InvokeType invoke_type,
                                       const InstructionOperands& operands,
                                       const char* shorty,
                                       /* out */ uint32_t* obj_reg) {
  const size_t number_of_operands = operands.GetNumberOfOperands();

  size_t start_index = 0u;
  InvokeDexCallingConventionVisitorARM64 convention;

  // Handle 'this' parameter.
  if (invoke_type != kStatic) {
    if (number_of_operands == 0u) {
      unimplemented_reason_ = "BogusSignature";
      return false;
    }
    start_index = 1u;
    *obj_reg = operands.GetOperand(0u);
    if (!MoveLocation(convention.GetNextLocation(DataType::Type::kReference),
                      vreg_locations_[*obj_reg],
                      DataType::Type::kReference)) {
      return false;
    }
  }

  uint32_t shorty_index = 1;  // Skip the return type.
  // Handle all parameters except 'this'.
  for (size_t i = start_index; i < number_of_operands; ++i, ++shorty_index) {
    // Make sure we don't go over the expected arguments or over the number of
    // dex registers given. If the instruction was seen as dead by the verifier,
    // it hasn't been properly checked.
    char c = shorty[shorty_index];
    if (UNLIKELY(c == 0)) {
      unimplemented_reason_ = "BogusSignature";
      return false;
    }
    DataType::Type type = DataType::FromShorty(c);
    bool is_wide = (type == DataType::Type::kInt64) || (type == DataType::Type::kFloat64);
    if (is_wide && ((i + 1 == number_of_operands) ||
                    (operands.GetOperand(i) + 1 != operands.GetOperand(i + 1)))) {
      unimplemented_reason_ = "BogusSignature";
      return false;
    }
    Location loc = convention.GetNextLocation(type);
    if (loc.IsDoubleStackSlot()) {
      // We only handle single stack slots in the fast compiler. The
      // instructions will be the ones knowing if we need two stack slot entries
      // or one.
      loc = Location::StackSlot(loc.GetStackIndex());
    }
    if (!MoveLocation(loc,
                      vreg_locations_[operands.GetOperand(i)],
                      type)) {
      return false;
    }
    if (is_wide) {
      ++i;
    }
  }
  return true;
}

bool FastCompilerARM64::LoadMethod(Register reg, ArtMethod* method) {
  __ Ldr(reg, jit_patches_.DeduplicateUint64Literal(reinterpret_cast<uint64_t>(method)));
  return true;
}

bool FastCompilerARM64::HandleInvoke(const Instruction& instruction,
                                     uint32_t dex_pc,
                                     InvokeType invoke_type) {
  Instruction::Code opcode = instruction.Opcode();
  uint16_t method_index = (opcode >= Instruction::INVOKE_VIRTUAL_RANGE)
      ? instruction.VRegB_3rc()
      : instruction.VRegB_35c();
  ArtMethod* resolved_method = nullptr;
  size_t offset = 0u;
  {
    Thread* self = Thread::Current();
    ScopedObjectAccess soa(self);
    ClassLinker* const class_linker = dex_compilation_unit_.GetClassLinker();
    resolved_method = method_->SkipAccessChecks()
        ? class_linker->ResolveMethodId(method_index, method_)
        : class_linker->ResolveMethodWithChecks(
              method_index, method_, invoke_type);
    if (resolved_method == nullptr) {
      DCHECK(self->IsExceptionPending());
      self->ClearException();
      unimplemented_reason_ = "UnresolvedInvoke";
      return false;
    }

    if (resolved_method->IsConstructor() && resolved_method->GetDeclaringClass()->IsObjectClass()) {
      // Object.<init> is always empty. Return early to not generate a frame.
      if (kIsDebugBuild) {
        CHECK(resolved_method->GetDeclaringClass()->IsVerified());
        CodeItemDataAccessor accessor(*resolved_method->GetDexFile(),
                                      resolved_method->GetCodeItem());
        CHECK_EQ(accessor.InsnsSizeInCodeUnits(), 1u);
        CHECK_EQ(accessor.begin().Inst().Opcode(), Instruction::RETURN_VOID);
      }
      // No need to update `previous_invoke_return_type_`, we know it is not going the
      // be used.
      return true;
    }

    if (invoke_type == kSuper) {
      resolved_method = method_->SkipAccessChecks()
          ? FindSuperMethodToCall</*access_check=*/false>(method_index,
                                                          resolved_method,
                                                          method_,
                                                          self)
          : FindSuperMethodToCall</*access_check=*/true>(method_index,
                                                         resolved_method,
                                                         method_,
                                                         self);
      if (resolved_method == nullptr) {
        DCHECK(self->IsExceptionPending()) << method_->PrettyMethod();
        self->ClearException();
        unimplemented_reason_ = "UnresolvedInvokeSuper";
        return false;
      }
    } else if (invoke_type == kVirtual) {
      offset = resolved_method->GetVtableIndex();
    } else if (invoke_type == kInterface) {
      if (resolved_method->GetDeclaringClass()->IsObjectClass()) {
        // If the resolved method is from j.l.Object, emit a virtual call instead.
        // The IMT conflict stub only handles interface methods.
        offset = resolved_method->GetVtableIndex();
        invoke_type = kVirtual;
      } else {
        offset = resolved_method->GetImtIndex();
      }
    }

    if (resolved_method->IsStringConstructor()) {
      unimplemented_reason_ = "StringConstructor";
      return false;
    }
  }

  // Given we are calling a method, generate a frame.
  if (!EnsureHasFrame()) {
    return false;
  }

  // Setup the arguments for the call.
  uint32_t obj_reg = -1;
  const char* shorty = dex_compilation_unit_.GetDexFile()->GetMethodShorty(method_index);
  if (opcode >= Instruction::INVOKE_VIRTUAL_RANGE) {
    RangeInstructionOperands operands(instruction.VRegC(), instruction.VRegA_3rc());
    if (!SetupArguments(invoke_type, operands, shorty, &obj_reg)) {
      return false;
    }
  } else {
    uint32_t args[5];
    uint32_t number_of_vreg_arguments = instruction.GetVarArgs(args);
    VarArgsInstructionOperands operands(args, number_of_vreg_arguments);
    if (!SetupArguments(invoke_type, operands, shorty, &obj_reg)) {
      return false;
    }
  }
  // Save the invoke return type for the next move-result instruction.
  previous_invoke_return_type_ = DataType::FromShorty(shorty[0]);

  if (invoke_type != kStatic) {
    bool can_be_null = CanBeNull(obj_reg);
    // Load the class of the instance. For kDirect and kSuper, this acts as a
    // null check.
    if (can_be_null || invoke_type == kVirtual || invoke_type == kInterface) {
      InvokeDexCallingConvention calling_convention;
      Register receiver = calling_convention.GetRegisterAt(0);
      Offset class_offset = mirror::Object::ClassOffset();
      EmissionCheckScope guard(GetVIXLAssembler(), kMaxMacroInstructionSizeInBytes);
      __ Ldr(kArtMethodRegister.W(), HeapOperand(receiver.W(), class_offset));
      if (can_be_null) {
        RecordPcInfo(dex_pc);
      }
    }
  }

  if (invoke_type == kVirtual) {
    size_t method_offset =
        mirror::Class::EmbeddedVTableEntryOffset(offset, kArm64PointerSize).SizeValue();
    __ Ldr(kArtMethodRegister, MemOperand(kArtMethodRegister, method_offset));
  } else if (invoke_type == kInterface) {
    __ Ldr(kArtMethodRegister,
           MemOperand(kArtMethodRegister,
                      mirror::Class::ImtPtrOffset(kArm64PointerSize).Uint32Value()));
    uint32_t method_offset =
        static_cast<uint32_t>(ImTable::OffsetOfElement(offset, kArm64PointerSize));
    __ Ldr(kArtMethodRegister, MemOperand(kArtMethodRegister, method_offset));
    if (!LoadMethod(ip1, resolved_method)) {
      return false;
    }
  } else {
    DCHECK(invoke_type == kDirect || invoke_type == kSuper || invoke_type == kStatic);
    if (!LoadMethod(kArtMethodRegister, resolved_method)) {
      return false;
    }
  }

  Offset entry_point = ArtMethod::EntryPointFromQuickCompiledCodeOffset(kArm64PointerSize);
  __ Ldr(lr, MemOperand(kArtMethodRegister, entry_point.SizeValue()));
  {
    // Use a scope to help guarantee that `RecordPcInfo()` records the correct pc.
    ExactAssemblyScope eas(GetVIXLAssembler(), kInstructionSize, CodeBufferCheckScope::kExactSize);
    __ blr(lr);
    RecordPcInfo(dex_pc);
  }
  return true;
}

void FastCompilerARM64::InvokeRuntime(QuickEntrypointEnum entrypoint, uint32_t dex_pc) {
  ThreadOffset64 entrypoint_offset = GetThreadOffset<kArm64PointerSize>(entrypoint);
  __ Ldr(lr, MemOperand(tr, entrypoint_offset.Int32Value()));
  // Ensure the pc position is recorded immediately after the `blr` instruction.
  ExactAssemblyScope eas(GetVIXLAssembler(), kInstructionSize, CodeBufferCheckScope::kExactSize);
  __ blr(lr);
  if (EntrypointRequiresStackMap(entrypoint)) {
    RecordPcInfo(dex_pc);
  }
}

bool FastCompilerARM64::BuildLoadString(uint32_t vreg,
                                        dex::StringIndex string_index,
                                        const Instruction* next) {
  // Generate a frame because of the read barrier.
  if (!EnsureHasFrame()) {
    return false;
  }
  Location loc = CreateNewRegisterLocation(vreg, DataType::Type::kReference, next);
  if (HitUnimplemented()) {
    return false;
  }

  ScopedObjectAccess soa(Thread::Current());
  ClassLinker* const class_linker = dex_compilation_unit_.GetClassLinker();
  ObjPtr<mirror::String> str = class_linker->ResolveString(string_index, method_);
  if (str == nullptr) {
    soa.Self()->ClearException();
    unimplemented_reason_ = "NullString";
    return false;
  }

  Handle<mirror::String> h_str = handles_->NewHandle(str);
  Register dst = RegisterFrom(loc, DataType::Type::kReference);
  __ Ldr(dst.W(), jit_patches_.DeduplicateJitStringLiteral(GetDexFile(),
                                                           string_index,
                                                           h_str,
                                                           code_generation_data_.get()));
  __ Ldr(dst.W(), MemOperand(dst.X()));
  DoReadBarrierOn(dst);
  UpdateLocal(vreg, /* is_object= */ true, /* is_wide= */ false, /* can_be_null= */ false);
  return true;
}

bool FastCompilerARM64::BuildLoadClass(uint32_t vreg,
                                       dex::TypeIndex type_index,
                                       const Instruction* next) {
  // Generate a frame because of the read barrier.
  if (!EnsureHasFrame()) {
    return false;
  }
  Location loc = CreateNewRegisterLocation(vreg, DataType::Type::kReference, next);
  if (HitUnimplemented()) {
    return false;
  }

  ScopedObjectAccess soa(Thread::Current());
  ObjPtr<mirror::Class> klass = dex_compilation_unit_.GetClassLinker()->ResolveType(
      type_index, dex_compilation_unit_.GetDexCache(), dex_compilation_unit_.GetClassLoader());
  if (klass == nullptr || !method_->GetDeclaringClass()->CanAccess(klass)) {
    soa.Self()->ClearException();
    unimplemented_reason_ = "UnsupportedLoadClass";
    return false;
  }

  Handle<mirror::Class> h_klass = handles_->NewHandle(klass);
  Register dst = RegisterFrom(loc, DataType::Type::kReference);
  __ Ldr(dst.W(), jit_patches_.DeduplicateJitClassLiteral(GetDexFile(),
                                                          type_index,
                                                          h_klass,
                                                          code_generation_data_.get()));
  __ Ldr(dst.W(), MemOperand(dst.X()));
  DoReadBarrierOn(dst);
  UpdateLocal(vreg, /* is_object= */ true, /* is_wide= */ false, /* can_be_null= */ false);
  return true;
}

bool FastCompilerARM64::BuildNewInstance(uint32_t vreg,
                                         dex::TypeIndex type_index,
                                         uint32_t dex_pc,
                                         const Instruction* next) {
  if (!EnsureHasFrame()) {
    return false;
  }

  ScopedObjectAccess soa(Thread::Current());
  ObjPtr<mirror::Class> klass = dex_compilation_unit_.GetClassLinker()->ResolveType(
      type_index, dex_compilation_unit_.GetDexCache(), dex_compilation_unit_.GetClassLoader());
  if (klass == nullptr ||
      !method_->GetDeclaringClass()->CanAccess(klass) ||
      klass->IsStringClass()) {
    soa.Self()->ClearException();
    unimplemented_reason_ = "UnsupportedClassForNewInstance";
    return false;
  }

  InvokeRuntimeCallingConvention calling_convention;
  Register cls_reg = calling_convention.GetRegisterAt(0);
  Handle<mirror::Class> h_klass = handles_->NewHandle(klass);
  __ Ldr(cls_reg.W(), jit_patches_.DeduplicateJitClassLiteral(GetDexFile(),
                                                              type_index,
                                                              h_klass,
                                                              code_generation_data_.get()));
  __ Ldr(cls_reg.W(), MemOperand(cls_reg.X()));
  DoReadBarrierOn(cls_reg);

  QuickEntrypointEnum entrypoint = kQuickAllocObjectInitialized;
  if (h_klass->IsFinalizable() ||
      !h_klass->IsVisiblyInitialized() ||
      h_klass->IsClassClass() ||  // Classes cannot be allocated in code
      !klass->IsInstantiable()) {
    entrypoint = kQuickAllocObjectWithChecks;
  }
  InvokeRuntime(entrypoint, dex_pc);
  __ Dmb(InnerShareable, BarrierWrites);
  if (!MoveLocation(CreateNewRegisterLocation(vreg, DataType::Type::kReference, next),
                    calling_convention.GetReturnLocation(DataType::Type::kReference),
                    DataType::Type::kReference)) {
    return false;
  }
  if (HitUnimplemented()) {
    return false;
  }
  UpdateLocal(vreg, /* is_object= */ true, /* is_wide= */ false, /* can_be_null= */ false);
  return true;
}


bool FastCompilerARM64::BuildNewArray(dex::TypeIndex type_index,
                                      Location length_location,
                                      uint32_t dex_pc) {
  const char* descriptor = GetDexFile().GetTypeDescriptor(GetDexFile().GetTypeId(type_index));
  DCHECK_EQ(descriptor[0], '[');
  size_t component_type_shift = Primitive::ComponentSizeShift(Primitive::GetType(descriptor[1]));
  QuickEntrypointEnum entrypoint =
      CodeGenerator::GetArrayAllocationEntrypoint(component_type_shift);
  DCHECK(has_frame_);

  InvokeRuntimeCallingConvention calling_convention;
  Register cls_reg = calling_convention.GetRegisterAt(0);
  {
    ScopedObjectAccess soa(Thread::Current());
    ObjPtr<mirror::Class> klass = dex_compilation_unit_.GetClassLinker()->ResolveType(
        type_index, dex_compilation_unit_.GetDexCache(), dex_compilation_unit_.GetClassLoader());
    if (klass == nullptr || !method_->GetDeclaringClass()->CanAccess(klass)) {
      soa.Self()->ClearException();
      unimplemented_reason_ = "UnsupportedClassForNewArray";
      return false;
    }

    Handle<mirror::Class> h_klass = handles_->NewHandle(klass);
    __ Ldr(cls_reg.W(), jit_patches_.DeduplicateJitClassLiteral(GetDexFile(),
                                                                type_index,
                                                                h_klass,
                                                                code_generation_data_.get()));
  }
  __ Ldr(cls_reg.W(), MemOperand(cls_reg.X()));
  DoReadBarrierOn(cls_reg);
  if (!MoveLocation(LocationFrom(calling_convention.GetRegisterAt(1)),
                    length_location,
                    DataType::Type::kInt32)) {
    return false;
  }
  InvokeRuntime(entrypoint, dex_pc);
  __ Dmb(InnerShareable, BarrierWrites);
  return true;
}

bool FastCompilerARM64::BuildNewArray(const Instruction& instruction,
                                      uint32_t dex_pc,
                                      const Instruction* next) {
  if (!EnsureHasFrame()) {
    return false;
  }
  dex::TypeIndex type_index(instruction.VRegC_22c());
  int32_t length = instruction.VRegB_22c();
  int32_t dst = instruction.VRegA_22c();

  Location length_location = GetExistingRegisterLocation(length, DataType::Type::kInt32);
  if (!BuildNewArray(type_index, length_location, dex_pc)) {
    return false;
  }

  InvokeRuntimeCallingConvention calling_convention;
  if (!MoveLocation(CreateNewRegisterLocation(dst, DataType::Type::kReference, next),
                    calling_convention.GetReturnLocation(DataType::Type::kReference),
                    DataType::Type::kReference)) {
    return false;
  }
  if (HitUnimplemented()) {
    return false;
  }
  UpdateLocal(dst, /* is_object= */ true, /* is_wide= */ false, /* can_be_null= */ false);
  return true;
}


bool FastCompilerARM64::BuildFilledNewArray(uint32_t dex_pc,
                                            dex::TypeIndex type_index,
                                            const InstructionOperands& operands) {
  if (!EnsureHasFrame()) {
    return false;
  }
  int32_t number_of_operands = operands.GetNumberOfOperands();

  if (!BuildNewArray(type_index,
                     Location::ConstantLocation(new (allocator_) HIntConstant(number_of_operands)),
                     dex_pc)) {
    return false;
  }
  const char* descriptor = GetDexFile().GetTypeDescriptor(type_index);
  char primitive = descriptor[1];
  if (primitive != 'I' && primitive != 'L' && primitive != '[') {
    unimplemented_reason_ = "BogusFilledNewArray";
    return false;
  }
  bool is_reference_array = (primitive == 'L') || (primitive == '[');
  DataType::Type type = is_reference_array ? DataType::Type::kReference : DataType::Type::kInt32;

  InvokeRuntimeCallingConvention calling_convention;
  Register array = RegisterFrom(calling_convention.GetReturnLocation(DataType::Type::kReference),
                                DataType::Type::kReference);
  size_t offset = mirror::Array::DataOffset(DataType::Size(type)).Uint32Value();
  {
    UseScratchRegisterScope temps(GetVIXLAssembler());
    Register temp = temps.AcquireW();
    for (int32_t i = 0; i < number_of_operands; ++i) {
      Location loc = vreg_locations_[operands.GetOperand(i)];
      Register value;
      if (loc.IsCoreRegister()) {
        value = RegisterFrom(loc, type);
      } else {
        MoveLocation(Location::CoreRegister(temp.GetCode()), loc, type);
        value = temp;
      }
      MemOperand mem = HeapOperand(array, offset + (i <<  DataType::SizeShift(type)));
      CodeGeneratorARM64::Store(GetVIXLAssembler(), type, value, mem);
    }
  }
  if (HitUnimplemented()) {
    return false;
  }
  if (type == DataType::Type::kReference) {
    UseScratchRegisterScope temps(GetVIXLAssembler());
    DoWriteBarrierOn(array, temps);
  }
  previous_invoke_return_type_ = DataType::Type::kReference;
  return true;
}

bool FastCompilerARM64::BuildCheckCast(uint32_t vreg, dex::TypeIndex type_index, uint32_t dex_pc) {
  if (!EnsureHasFrame()) {
    return false;
  }

  InvokeRuntimeCallingConvention calling_convention;
  Register cls = calling_convention.GetRegisterAt(1);
  Register obj_cls = calling_convention.GetRegisterAt(2);
  Register obj = WRegisterFrom(GetExistingRegisterLocation(vreg, DataType::Type::kReference));
  if (HitUnimplemented()) {
    return false;
  }

  ScopedObjectAccess soa(Thread::Current());
  ObjPtr<mirror::Class> klass = dex_compilation_unit_.GetClassLinker()->ResolveType(
      type_index, dex_compilation_unit_.GetDexCache(), dex_compilation_unit_.GetClassLoader());
  if (klass == nullptr || !method_->GetDeclaringClass()->CanAccess(klass)) {
    soa.Self()->ClearException();
    unimplemented_reason_ = "UnsupportedCheckCast";
    return false;
  }
  Handle<mirror::Class> h_klass = handles_->NewHandle(klass);

  vixl::aarch64::Label exit, read_barrier_exit;
  __ Cbz(obj, &exit);
  __ Ldr(cls.W(), jit_patches_.DeduplicateJitClassLiteral(GetDexFile(),
                                                          type_index,
                                                          h_klass,
                                                          code_generation_data_.get()));
  __ Ldr(cls.W(), MemOperand(cls.X()));
  __ Ldr(obj_cls.W(), MemOperand(obj.X()));
  __ Cmp(cls.W(), obj_cls.W());
  __ B(eq, &exit);

  // Read barrier on the GC Root.
  DoReadBarrierOn(cls, &read_barrier_exit);
  // Read barrier on the object's class.
  DoReadBarrierOn(obj_cls, &read_barrier_exit, /* do_mr_check= */ false);

  __ Bind(&read_barrier_exit);
  __ Cmp(cls.W(), obj_cls.W());
  __ B(eq, &exit);
  if (!MoveLocation(LocationFrom(calling_convention.GetRegisterAt(0)),
                    LocationFrom(obj),
                    DataType::Type::kReference)) {
    return false;
  }
  InvokeRuntime(kQuickCheckInstanceOf, dex_pc);

  __ Bind(&exit);
  return true;
}

bool FastCompilerARM64::BuildInstanceOf(uint32_t vreg,
                                        uint32_t vreg_result,
                                        dex::TypeIndex type_index,
                                        uint32_t dex_pc,
                                        const Instruction* next) {
  if (!EnsureHasFrame()) {
    return false;
  }

  InvokeRuntimeCallingConvention calling_convention;
  Register cls = calling_convention.GetRegisterAt(1);
  // Use a temporary register for `obj_cls`. Cannot be a vixl temp as it needs
  // to survive a read barrier.
  Register obj_cls = calling_convention.GetRegisterAt(0);
  Register obj = WRegisterFrom(GetExistingRegisterLocation(vreg, DataType::Type::kReference));
  Location result = CreateNewRegisterLocation(vreg_result, DataType::Type::kInt32, next);
  if (HitUnimplemented()) {
    return false;
  }

  vixl::aarch64::Label exit, read_barrier_exit, set_zero, set_one;
  {
    ScopedObjectAccess soa(Thread::Current());
    ObjPtr<mirror::Class> klass = dex_compilation_unit_.GetClassLinker()->ResolveType(
        type_index, dex_compilation_unit_.GetDexCache(), dex_compilation_unit_.GetClassLoader());
    if (klass == nullptr || !method_->GetDeclaringClass()->CanAccess(klass)) {
      soa.Self()->ClearException();
      unimplemented_reason_ = "UnsupportedCheckCast";
      return false;
    }
    Handle<mirror::Class> h_klass = handles_->NewHandle(klass);
    __ Cbz(obj, &set_zero);
    __ Ldr(cls.W(), jit_patches_.DeduplicateJitClassLiteral(GetDexFile(),
                                                            type_index,
                                                            h_klass,
                                                            code_generation_data_.get()));
  }
  __ Ldr(cls.W(), MemOperand(cls.X()));
  __ Ldr(obj_cls.W(), MemOperand(obj.X()));
  __ Cmp(cls.W(), obj_cls.W());
  __ B(eq, &set_one);

  // Read barrier on the GC Root.
  DoReadBarrierOn(cls, &read_barrier_exit);
  // Read barrier on the object's class.
  DoReadBarrierOn(obj_cls, &read_barrier_exit, /* do_mr_check= */ false);

  __ Bind(&read_barrier_exit);
  __ Cmp(cls.W(), obj_cls.W());
  __ B(eq, &set_one);
  // We clobber `obj_cls` here, which is fine as we don't need it anymore.
  if (!MoveLocation(LocationFrom(calling_convention.GetRegisterAt(0)),
                    LocationFrom(obj),
                    DataType::Type::kReference)) {
    return false;
  }
  InvokeRuntime(kQuickInstanceofNonTrivial, dex_pc);
  if (!MoveLocation(result,
                    calling_convention.GetReturnLocation(DataType::Type::kInt32),
                    DataType::Type::kInt32)) {
    return false;
  }
  __ B(&exit);
  __ Bind(&set_zero);
  if (!MoveLocation(result,
                    Location::ConstantLocation(new (allocator_) HIntConstant(0)),
                    DataType::Type::kInt32)) {
    return false;
  }
  __ B(&exit);
  __ Bind(&set_one);
  if (!MoveLocation(result,
                    Location::ConstantLocation(new (allocator_) HIntConstant(1)),
                    DataType::Type::kInt32)) {
    return false;
  }
  __ Bind(&exit);
  UpdateLocal(vreg_result, /* is_object= */ false, /* is_wide= */ false);
  return true;
}

void FastCompilerARM64::DoReadBarrierOn(Register reg,
                                        vixl::aarch64::Label* exit,
                                        bool do_mr_check) {
  DCHECK(has_frame_);
  vixl::aarch64::Label local_exit;
  if (do_mr_check) {
    __ Cbz(mr, (exit != nullptr) ? exit : &local_exit);
  }
  int32_t entry_point_offset =
      Thread::ReadBarrierMarkEntryPointsOffset<kArm64PointerSize>(reg.GetCode());
  __ Ldr(lr, MemOperand(tr, entry_point_offset));
  __ Blr(lr);
  if (exit == nullptr && do_mr_check) {
    __ Bind(&local_exit);
  }
}

void FastCompilerARM64::DoWriteBarrierOn(Register holder,
                                         UseScratchRegisterScope& temps,
                                         bool overwrite_holder) {
  Register card = temps.AcquireX();
  Register temp = overwrite_holder ? holder : temps.AcquireW();
  __ Ldr(card, MemOperand(tr, Thread::CardTableOffset<kArm64PointerSize>().Int32Value()));
  __ Lsr(temp, holder, gc::accounting::CardTable::kCardShift);
  __ Strb(card, MemOperand(card, temp.X()));
}

#define DO_CASE(arm_op, op, other) \
    case arm_op: { \
      if (constant op other) { \
        __ B(label); \
      } \
      break; \
    } \

template<vixl::aarch64::Condition kCond, bool kCompareWithZero>
bool FastCompilerARM64::If_21_22t(const Instruction& instruction, uint32_t dex_pc) {
  DCHECK_EQ(kCompareWithZero ? Instruction::Format::k21t : Instruction::Format::k22t,
            Instruction::FormatOf(instruction.Opcode()));
  if (!EnsureHasFrame()) {
    return false;
  }
  int32_t target_offset = kCompareWithZero ? instruction.VRegB_21t() : instruction.VRegC_22t();
  DCHECK_EQ(target_offset, instruction.GetTargetOffset());
  vixl::aarch64::Label* label = GetLabelOf(dex_pc + target_offset);
  bool is_back_edge = (target_offset < 0);
  vixl::aarch64::Label suspend_check;
  if (is_back_edge) {
    if (!CanHandleBackwardsBranch(dex_pc + target_offset)) {
      return false;
    }
    label = &suspend_check;
  }
  int32_t register_index = kCompareWithZero ? instruction.VRegA_21t() : instruction.VRegA_22t();
  Location location = vreg_locations_[register_index];

  if (kCompareWithZero) {
    // We are going to branch, move all constants to registers to make the merge
    // point use the same locations.
    PrepareToBranch(dex_pc + target_offset);
    if (location.IsConstant()) {
      DCHECK(location.GetConstant()->IsIntConstant());
      int32_t constant = location.GetConstant()->AsIntConstant()->GetValue();
      switch (kCond) {
        DO_CASE(vixl::aarch64::eq, ==, 0);
        DO_CASE(vixl::aarch64::ne, !=, 0);
        DO_CASE(vixl::aarch64::lt, <, 0);
        DO_CASE(vixl::aarch64::le, <=, 0);
        DO_CASE(vixl::aarch64::gt, >, 0);
        DO_CASE(vixl::aarch64::ge, >=, 0);
      }
    } else {
      location = GetExistingRegisterLocation(register_index, DataType::Type::kInt32);
      if (HitUnimplemented()) {
        return false;
      }
      CPURegister reg = CPURegisterFrom(location, DataType::Type::kInt32);
      switch (kCond) {
        case vixl::aarch64::eq: {
          __ Cbz(Register(reg), label);
          break;
        }
        case vixl::aarch64::ne: {
          __ Cbnz(Register(reg), label);
          break;
        }
        default: {
          __ Cmp(Register(reg), 0);
          __ B(kCond, label);
          break;
        }
      }
    }
  } else {
    // !kCompareWithZero
    Location other_location = vreg_locations_[instruction.VRegB_22t()];
    // We are going to branch, move all constants to registers to make the merge
    // point use the same locations.
    PrepareToBranch(dex_pc + target_offset);
    if (location.IsConstant() && other_location.IsConstant()) {
      int32_t constant = location.GetConstant()->AsIntConstant()->GetValue();
      int32_t other_constant = other_location.GetConstant()->AsIntConstant()->GetValue();
      switch (kCond) {
        DO_CASE(vixl::aarch64::eq, ==, other_constant);
        DO_CASE(vixl::aarch64::ne, !=, other_constant);
        DO_CASE(vixl::aarch64::lt, <, other_constant);
        DO_CASE(vixl::aarch64::le, <=, other_constant);
        DO_CASE(vixl::aarch64::gt, >, other_constant);
        DO_CASE(vixl::aarch64::ge, >=, other_constant);
      }
    } else {
      // Reload the locations, which can now be registers.
      location = GetExistingRegisterLocation(register_index, DataType::Type::kInt32);
      other_location = GetExistingRegisterLocation(instruction.VRegB_22t(), DataType::Type::kInt32);
      if (HitUnimplemented()) {
        return false;
      }
      CPURegister reg = CPURegisterFrom(location, DataType::Type::kInt32);
      CPURegister other_reg = CPURegisterFrom(other_location, DataType::Type::kInt32);
      __ Cmp(Register(reg), Register(other_reg));
      __ B(kCond, label);
    }
  }

  if (is_back_edge) {
    vixl::aarch64::Label next_instruction;
    __ B(&next_instruction);
    __ Bind(label);
    UseScratchRegisterScope temps(GetVIXLAssembler());
    GenerateSuspendCheckAndIncrementHotness(dex_pc, temps.AcquireX());
    __ B(GetLabelOf(dex_pc + target_offset));
    __ Bind(&next_instruction);
  }
  return true;
}
#undef DO_CASE

bool FastCompilerARM64::DoGet(const MemOperand& base,
                              uint16_t field_index,
                              Instruction::Code opcode,
                              uint32_t dest_reg,
                              bool can_receiver_be_null,
                              bool is_object,
                              bool is_volatile,
                              uint32_t dex_pc,
                              const Instruction* next) {
  UseScratchRegisterScope temps(GetVIXLAssembler());
  MemOperand mem = base;
  Register holder = mem.GetBaseRegister();

  if (can_receiver_be_null && !CanDoImplicitNullCheckOn(mem.GetOffset())) {
    vixl::aarch64::Label is_not_null;
    __ Cbnz(holder, &is_not_null);
    InvokeRuntime(kQuickThrowNullPointer, dex_pc);
    __ Bind(&is_not_null);
  }

  bool record_pc_info = (can_receiver_be_null && CanDoImplicitNullCheckOn(mem.GetOffset()));
  if (is_volatile) {
    Register temp = temps.AcquireX();
    __ Add(temp, holder, helpers::OperandFromMemOperand(mem));
    mem = MemOperand(temp);
  }
  if (is_object) {
    Register dst = WRegisterFrom(
        CreateNewRegisterLocation(dest_reg, DataType::Type::kReference, next));
    if (HitUnimplemented()) {
      return false;
    }
    {
      // Ensure the pc position is recorded immediately after the load instruction.
      EmissionCheckScope guard(GetVIXLAssembler(), kMaxMacroInstructionSizeInBytes);
      if (is_volatile) {
        __ ldar(dst, mem);
      } else {
        __ Ldr(dst, mem);
      }
      if (record_pc_info) {
        RecordPcInfo(dex_pc);
      }
    }
    UpdateLocal(dest_reg, /* is_object= */ true, /* is_wide= */ false);
    DoReadBarrierOn(dst);
    return true;
  }

  // Ensure the pc position is recorded immediately after the load instruction.
  EmissionCheckScope guard(GetVIXLAssembler(), kMaxMacroInstructionSizeInBytes);
  bool is_wide = false;
  switch (opcode) {
    case Instruction::SGET_BOOLEAN:
    case Instruction::IGET_BOOLEAN: {
      Register dst = WRegisterFrom(
          CreateNewRegisterLocation(dest_reg, DataType::Type::kInt32, next));
      if (is_volatile) {
        __ ldarb(Register(dst), mem);
      } else {
        __ Ldrb(Register(dst), mem);
      }
      break;
    }
    case Instruction::SGET_BYTE:
    case Instruction::IGET_BYTE: {
      Register dst = WRegisterFrom(
          CreateNewRegisterLocation(dest_reg, DataType::Type::kInt32, next));
      if (is_volatile) {
        __ ldarb(Register(dst), mem);
        if (can_receiver_be_null) {
          record_pc_info = false;
          RecordPcInfo(dex_pc);
        }
        __ Sbfx(dst, dst, 0, DataType::Size(DataType::Type::kInt8) * kBitsPerByte);
      } else {
        __ Ldrsb(Register(dst), mem);
      }
      break;
    }
    case Instruction::SGET_CHAR:
    case Instruction::IGET_CHAR: {
      Register dst = WRegisterFrom(
          CreateNewRegisterLocation(dest_reg, DataType::Type::kInt32, next));
      if (is_volatile) {
        __ ldarh(Register(dst), mem);
      } else {
        __ Ldrh(Register(dst), mem);
      }
      break;
    }
    case Instruction::SGET_SHORT:
    case Instruction::IGET_SHORT: {
      Register dst = WRegisterFrom(
          CreateNewRegisterLocation(dest_reg, DataType::Type::kInt32, next));
      if (is_volatile) {
        __ ldarh(Register(dst), mem);
        if (can_receiver_be_null) {
          record_pc_info = false;
          RecordPcInfo(dex_pc);
        }
        __ Sbfx(dst, dst, 0, DataType::Size(DataType::Type::kInt16) * kBitsPerByte);
      } else {
        __ Ldrsh(Register(dst), mem);
      }
      break;
    }
    case Instruction::SGET_WIDE:
    case Instruction::IGET_WIDE:
      is_wide = true;
      FALLTHROUGH_INTENDED;
    case Instruction::SGET:
    case Instruction::IGET: {
      const dex::FieldId& field_id = GetDexFile().GetFieldId(field_index);
      const char* type = GetDexFile().GetFieldTypeDescriptor(field_id);
      DataType::Type field_type = DataType::FromShorty(type[0]);
      Location location = CreateNewRegisterLocation(dest_reg, field_type, next);
      if (is_volatile) {
        if (DataType::IsFloatingPointType(field_type)) {
          bool can_overwrite_holder = (Instruction::FormatOf(opcode) == Instruction::k21c);
          Register temp = can_overwrite_holder ? holder.X() : temps.AcquireX();
          temp = is_wide ? temp.X() : temp.W();
          __ ldar(temp, mem);
          if (can_receiver_be_null) {
            record_pc_info = false;
            RecordPcInfo(dex_pc);
          }
          VRegister dst = is_wide ? DRegisterFrom(location) : SRegisterFrom(location);
          __ Fmov(dst, temp);
        } else {
          Register dst = is_wide ? XRegisterFrom(location) : WRegisterFrom(location);
          __ ldar(dst, mem);
        }
      } else {
        if (DataType::IsFloatingPointType(field_type)) {
          VRegister dst = is_wide ? DRegisterFrom(location) : SRegisterFrom(location);
          __ Ldr(dst, mem);
        } else {
          Register dst = is_wide ? XRegisterFrom(location) : WRegisterFrom(location);
          __ Ldr(dst, mem);
        }
      }
      if (HitUnimplemented()) {
        return false;
      }
      break;
    }
    default:
      unimplemented_reason_ = Instruction::Name(opcode);
      return false;
  }
  UpdateLocal(dest_reg, is_object, is_wide);
  if (record_pc_info) {
    RecordPcInfo(dex_pc);
  }
  return true;
}

bool FastCompilerARM64::BuildMove(uint32_t dest_reg,
                                  uint32_t src_reg,
                                  DataType::Type type,
                                  const Instruction* next) {
  UpdateLocal(dest_reg,
              /* is_object= */ type == DataType::Type::kReference,
              /* is_wide= */ DataType::Is64BitType(type),
              CanBeNull(src_reg));

  // Fetch the source before creating a new register for the destination, in
  // case they overlap.
  Location source = vreg_locations_[src_reg];

  // Translate a move into an actual move instruction. We could just update
  // `vreg_locations_`, but that would require tracking aliases, which may be
  // costly in compile time.
  if (!MoveLocation(CreateNewRegisterLocation(dest_reg, type, next), source, type)) {
    return false;
  }
  return true;
}

void FastCompilerARM64::SetIntConstant(uint32_t register_index,
                                       int32_t constant,
                                       const Instruction* next) {
  bool can_be_object = (constant == 0);
  if (GetCodeItemAccessor().TriesSize() == 0 && !can_be_object) {
    vreg_locations_[register_index] =
        Location::ConstantLocation(new (allocator_) HIntConstant(constant));
  } else {
    // In the presence of try/catch, we put the constant in a register directly.
    // This avoids having to dump dex register maps for stack maps, saving
    // compilation time.
    // We also store in a register for the constant zero to simplify object
    // register mask merging in the presence of control flow.
    MoveLocation(CreateNewRegisterLocation(register_index, DataType::Type::kInt32, next),
                 Location::ConstantLocation(new (allocator_) HIntConstant(constant)),
                 DataType::Type::kInt32);
  }
  // In case we branch, we need to make sure a null value can be merged
  // with an object value, so treat the 0 value as an object.
  UpdateLocal(register_index, can_be_object, /* is_wide= */ false);
}

bool FastCompilerARM64::BuildInvokeRuntime11x(QuickEntrypointEnum entrypoint,
                                              const Instruction& instruction,
                                              uint32_t dex_pc) {
    if (!EnsureHasFrame()) {
      return false;
    }
    int32_t reg = instruction.VRegA_11x();
    InvokeRuntimeCallingConvention calling_convention;
    if (!MoveLocation(LocationFrom(calling_convention.GetRegisterAt(0)),
                      vreg_locations_[reg],
                      DataType::Type::kReference)) {
      return false;
    }
    InvokeRuntime(entrypoint, dex_pc);
    return true;
}

void FastCompilerARM64::SetLongConstant(uint32_t register_index,
                                        int64_t constant,
                                        const Instruction* next) {
  if (GetCodeItemAccessor().TriesSize() == 0) {
    vreg_locations_[register_index] =
        Location::ConstantLocation(new (allocator_) HLongConstant(constant));
    vreg_locations_[register_index + 1] = Location();
  } else {
    // In the presence of try/catch, we put the constant in a register directly.
    // This avoids having to dump dex register maps for stack maps, saving
    // compilation time.
    MoveLocation(CreateNewRegisterLocation(register_index, DataType::Type::kInt64, next),
                 Location::ConstantLocation(new (allocator_) HLongConstant(constant)),
                 DataType::Type::kInt64);
  }
  UpdateLocal(register_index, /* is_object= */ false, /* is_wide= */ true);
}

bool FastCompilerARM64::BuildArrayAccess(const Instruction& instruction,
                                         uint32_t dex_pc,
                                         bool is_put,
                                         DataType::Type type,
                                         const Instruction* next) {
  // For bounds check, null check, and read barrier.
  if (!EnsureHasFrame()) {
    return false;
  }
  uint8_t source_or_dest_reg = instruction.VRegA_23x();
  uint8_t array_reg = instruction.VRegB_23x();
  uint8_t index_reg = instruction.VRegC_23x();
  Register array = RegisterFrom(GetExistingRegisterLocation(array_reg, DataType::Type::kReference),
                                DataType::Type::kReference);

  MemOperand mem = HeapOperand(array.W(), mirror::Array::LengthOffset().Uint32Value());
  InvokeRuntimeCallingConvention calling_convention;
  Register temp = calling_convention.GetRegisterAt(1);
  // Fetch the length, and do a null pointer check.
  {
    // Ensure the pc position is recorded immediately after the load instruction.
    EmissionCheckScope guard(GetVIXLAssembler(), kMaxMacroInstructionSizeInBytes);
    __ Ldr(temp, mem);
    if (CanBeNull(array_reg)) {
      RecordPcInfo(dex_pc);
    }
  }

  Register index = RegisterFrom(GetExistingRegisterLocation(index_reg, DataType::Type::kInt32),
                                DataType::Type::kInt32);

  if (HitUnimplemented()) {
    return false;
  }
  // Bounds check.
  __ Cmp(index.W(), temp.W());
  vixl::aarch64::Label cont;
  __ B(vixl::aarch64::lo, &cont);
  __ Mov(calling_convention.GetRegisterAt(0).W(), index.W());
  InvokeRuntime(kQuickThrowArrayBounds, dex_pc);
  __ Bind(&cont);

  bool is_object = (type == DataType::Type::kReference);
  if (is_put && is_object) {
    Register value = RegisterFrom(GetExistingRegisterLocation(source_or_dest_reg, type), type);
    __ Mov(calling_convention.GetRegisterAt(0).W(), array.W());
    __ Mov(calling_convention.GetRegisterAt(1).W(), index.W());
    __ Mov(calling_convention.GetRegisterAt(2).W(), value.W());
    InvokeRuntime(kQuickAputObject, dex_pc);
    return true;
  }

  __ Add(temp.W(), array.W(), mirror::Array::DataOffset(DataType::Size(type)).Uint32Value());
  MemOperand src = HeapOperand(temp.W(), index.X(), LSL, DataType::SizeShift(type));
  Location location = is_put
      ? GetExistingRegisterLocation(source_or_dest_reg, type)
      : CreateNewRegisterLocation(source_or_dest_reg, type, next);
  // Array access operations don't explicitly mention if they operate on
  // floating point values. If we find this out ourselves, adjust the type.
  if (location.IsFpuRegister()) {
    type = (type == DataType::Type::kInt32) ? DataType::Type::kFloat32 : DataType::Type::kFloat64;
  }
  CPURegister value_or_dest = CPURegisterFrom(location, type);
  if (is_put) {
    CodeGeneratorARM64::Store(GetVIXLAssembler(), type, value_or_dest, src);
  } else {
    CodeGeneratorARM64::Load(GetVIXLAssembler(), type, value_or_dest, src);
    UpdateLocal(source_or_dest_reg, is_object, DataType::Is64BitType(type));
    if (is_object) {
      DoReadBarrierOn(Register(value_or_dest));
    }
  }
  if (HitUnimplemented()) {
    return false;
  }
  return true;
}

bool FastCompilerARM64::BuildArrayLength(
    const Instruction& instruction, uint32_t dex_pc, const Instruction* next) {
  int32_t array = instruction.VRegB_12x();
  int32_t dest = instruction.VRegA_12x();
  if (CanBeNull(array)) {
    if (!EnsureHasFrame()) {
      return false;
    }
  }
  Register array_reg = RegisterFrom(
      GetExistingRegisterLocation(array, DataType::Type::kReference),
      DataType::Type::kReference);
  Register dest_reg = RegisterFrom(
      CreateNewRegisterLocation(dest, DataType::Type::kInt32, next), DataType::Type::kInt32);
  if (HitUnimplemented()) {
    return false;
  }
  MemOperand mem = HeapOperand(array_reg.W(), mirror::Array::LengthOffset().Uint32Value());
  {
    // Ensure the pc position is recorded immediately after the store instruction.
    EmissionCheckScope guard(GetVIXLAssembler(), kMaxMacroInstructionSizeInBytes);
    __ Ldr(dest_reg, mem);
    if (CanBeNull(array)) {
      RecordPcInfo(dex_pc);
    }
  }
  UpdateLocal(dest, /* is_object= */ false, /* is_wide= */ false);
  return true;
}

bool FastCompilerARM64::BuildInstanceFieldGet(const Instruction& instruction,
                                              uint32_t dex_pc,
                                              bool is_object,
                                              const Instruction* next) {
  uint32_t source_or_dest_reg = instruction.VRegA_22c();
  uint32_t obj_reg = instruction.VRegB_22c();
  uint16_t field_index = instruction.VRegC_22c();
  bool can_receiver_be_null = CanBeNull(obj_reg);
  ArtField* field = nullptr;
  {
    ScopedObjectAccess soa(Thread::Current());
    field = ResolveFieldWithAccessChecks(soa.Self(),
                                         dex_compilation_unit_.GetClassLinker(),
                                         field_index,
                                         method_,
                                         /* is_static= */ false,
                                         /* is_put= */ false,
                                         /* resolve_field_type= */ 0u);
    if (field == nullptr) {
      // Clear potential resolution exception.
      Thread::Current()->ClearException();
      unimplemented_reason_ = "UnresolvedField";
      return false;
    }
  }

  if (can_receiver_be_null || is_object) {
    // We need a frame in case the null check throws or there is a read
    // barrier.
    if (!EnsureHasFrame()) {
      return false;
    }
  }
  MemOperand mem = HeapOperand(
      RegisterFrom(GetExistingRegisterLocation(obj_reg, DataType::Type::kReference),
                   DataType::Type::kReference),
      field->GetOffset());
  if (HitUnimplemented()) {
    return false;
  }
  if (!DoGet(mem,
             field_index,
             instruction.Opcode(),
             source_or_dest_reg,
             can_receiver_be_null,
             is_object,
             field->IsVolatile(),
             dex_pc,
             next)) {
    return false;
  }
  return true;
}

bool FastCompilerARM64::BuildInstanceFieldSet(const Instruction& instruction,
                                              uint32_t dex_pc,
                                              bool is_object) {
  uint32_t source_reg = instruction.VRegA_22c();
  uint32_t obj_reg = instruction.VRegB_22c();
  uint16_t field_index = instruction.VRegC_22c();
  bool can_receiver_be_null = CanBeNull(obj_reg);
  ArtField* field = nullptr;
  {
    ScopedObjectAccess soa(Thread::Current());
    field = ResolveFieldWithAccessChecks(soa.Self(),
                                         dex_compilation_unit_.GetClassLinker(),
                                         field_index,
                                         method_,
                                         /* is_static= */ false,
                                         /* is_put= */ true,
                                         /* resolve_field_type= */ is_object);
    if (field == nullptr) {
      // Clear potential resolution exception.
      Thread::Current()->ClearException();
      unimplemented_reason_ = "UnresolvedField";
      return false;
    }
  }

  if (can_receiver_be_null) {
    // We need a frame in case the null check throws.
    if (!EnsureHasFrame()) {
      return false;
    }
  }

  Register holder = RegisterFrom(
      GetExistingRegisterLocation(obj_reg, DataType::Type::kReference),
      DataType::Type::kReference);
  if (HitUnimplemented()) {
    return false;
  }
  MemOperand mem = HeapOperand(holder, field->GetOffset());

  return DoPut(mem,
               holder,
               field,
               instruction.Opcode(),
               source_reg,
               can_receiver_be_null,
               is_object,
               field->IsVolatile(),
               dex_pc);
}

bool FastCompilerARM64::DoPut(const MemOperand& base,
                              Register holder,
                              ArtField* field,
                              Instruction::Code opcode,
                              int32_t source_reg,
                              bool can_receiver_be_null,
                              bool is_object,
                              bool is_volatile,
                              uint32_t dex_pc) {
  if (can_receiver_be_null && !CanDoImplicitNullCheckOn(field->GetOffset().Uint32Value())) {
    vixl::aarch64::Label is_not_null;
    __ Cbnz(holder, &is_not_null);
    InvokeRuntime(kQuickThrowNullPointer, dex_pc);
    __ Bind(&is_not_null);
  }
  bool record_pc_info =
      (can_receiver_be_null && CanDoImplicitNullCheckOn(field->GetOffset().Uint32Value()));
  UseScratchRegisterScope temps(GetVIXLAssembler());
  Location src = vreg_locations_[source_reg];
  bool assigning_constant = false;
  Register temp;
  // For static access, the holder is already in a temporary, and we can
  // overwrite it.
  bool overwrite_holder = (Instruction::FormatOf(opcode) == Instruction::k21c);
  if (src.IsConstant()) {
    assigning_constant = true;
  }
  MemOperand mem = base;
  if (is_volatile) {
    temp = temps.AcquireX();
    __ Add(temp, mem.GetBaseRegister(), helpers::OperandFromMemOperand(mem));
    mem = MemOperand(temp);
  }
  if (is_object) {
    src = GetExistingRegisterLocation(source_reg, DataType::Type::kReference);
    if (HitUnimplemented()) {
      return false;
    }
    Register reg = WRegisterFrom(src);
    // Ensure the pc position is recorded immediately after the store instruction.
    EmissionCheckScope guard(GetVIXLAssembler(), kMaxMacroInstructionSizeInBytes);
    if (is_volatile) {
      __ stlr(reg, mem);
    } else {
      __ Str(reg, mem);
    }
    if (record_pc_info) {
      RecordPcInfo(dex_pc);
    }
    // If we assign a constant (only null for iput-object), no need for the write
    // barrier.
    if (!assigning_constant) {
      {
        ScopedObjectAccess soa(Thread::Current());
        if (method_->GetDeclaringClass()->HasTypeChecksFailure() &&
            field->ResolveType() == nullptr) {
          soa.Self()->ClearException();
          unimplemented_reason_ = "UnresolvedFieldType";
          return false;
        }
      }
      vixl::aarch64::Label exit;
      __ Cbz(reg, &exit);
      temps.Release(temp);
      DoWriteBarrierOn(holder, temps, overwrite_holder);
      __ Bind(&exit);
    }
    return true;
  }
  // Ensure the pc position is recorded immediately after the store instruction.
  EmissionCheckScope guard(GetVIXLAssembler(), kMaxMacroInstructionSizeInBytes);
  switch (opcode) {
    case Instruction::IPUT_BOOLEAN:
    case Instruction::IPUT_BYTE:
    case Instruction::SPUT_BOOLEAN:
    case Instruction::SPUT_BYTE: {
      src = GetExistingRegisterLocation(source_reg, DataType::Type::kInt32);
      if (is_volatile) {
        __ stlrb(WRegisterFrom(src), mem);
      } else {
        __ Strb(WRegisterFrom(src), mem);
      }
      break;
    }
    case Instruction::IPUT_CHAR:
    case Instruction::IPUT_SHORT:
    case Instruction::SPUT_CHAR:
    case Instruction::SPUT_SHORT: {
      src = GetExistingRegisterLocation(source_reg, DataType::Type::kInt32);
      if (is_volatile) {
        __ stlrh(WRegisterFrom(src), mem);
      } else {
        __ Strh(WRegisterFrom(src), mem);
      }
      break;
    }
    case Instruction::IPUT:
    case Instruction::SPUT: {
      if (is_volatile) {
        if (src.IsFpuRegister()) {
          temp = overwrite_holder ? holder : temps.AcquireW();
          __ Fmov(temp, SRegisterFrom(src));
          __ stlr(temp, mem);
        } else {
          src = GetExistingRegisterLocation(source_reg, DataType::Type::kInt32);
          __ stlr(WRegisterFrom(src), mem);
        }
      } else {
        if (src.IsFpuRegister()) {
          __ Str(SRegisterFrom(src), mem);
        } else {
          src = GetExistingRegisterLocation(source_reg, DataType::Type::kInt32);
          __ Str(WRegisterFrom(src), mem);
        }
      }
      break;
    }
    case Instruction::IPUT_WIDE:
    case Instruction::SPUT_WIDE: {
      if (is_volatile) {
        if (src.IsFpuRegister()) {
          temp = overwrite_holder ? holder.X() : temps.AcquireX();
          __ Fmov(temp, DRegisterFrom(src));
          __ stlr(temp, mem);
        } else {
          src = GetExistingRegisterLocation(source_reg, DataType::Type::kInt64);
          __ stlr(XRegisterFrom(src), mem);
        }
      } else {
        if (src.IsFpuRegister()) {
          __ Str(DRegisterFrom(src), mem);
        } else {
          src = GetExistingRegisterLocation(source_reg, DataType::Type::kInt64);
          __ Str(XRegisterFrom(src), mem);
        }
      }
      break;
    }
    default:
      unimplemented_reason_ = Instruction::Name(opcode);
      return false;
  }

  if (HitUnimplemented()) {
    return false;
  }
  if (record_pc_info) {
    RecordPcInfo(dex_pc);
  }
  return true;
}

bool FastCompilerARM64::BuildStaticFieldAccess(const Instruction& instruction,
                                               uint32_t dex_pc,
                                               bool is_object,
                                               bool is_put,
                                               const Instruction* next) {
  // We need a frame for the read barrier and the clinit check.
  if (!EnsureHasFrame()) {
    return false;
  }
  ArtField* field = nullptr;
  uint16_t field_index = instruction.VRegB_21c();
  uint32_t source_or_dest_reg = instruction.VRegA_21c();
  Register temp = vixl::aarch64::XRegister(GetTempCoreRegister());
  bool generate_clinit_check = false;
  {
    ScopedObjectAccess soa(Thread::Current());
    field = ResolveFieldWithAccessChecks(soa.Self(),
                                         dex_compilation_unit_.GetClassLinker(),
                                         field_index,
                                         method_,
                                         /* is_static= */ true,
                                         is_put,
                                         /* resolve_field_type= */ 0u);
    if (field == nullptr) {
      // Clear potential resolution exception.
      Thread::Current()->ClearException();
      unimplemented_reason_ = "UnresolvedField";
      return false;
    }
    Handle<mirror::Class> h_klass = handles_->NewHandle(field->GetDeclaringClass());
    generate_clinit_check = !h_klass->IsVisiblyInitialized();
    __ Ldr(temp.W(), jit_patches_.DeduplicateJitClassLiteral(h_klass->GetDexFile(),
                                                             h_klass->GetDexTypeIndex(),
                                                             h_klass,
                                                             code_generation_data_.get()));
    __ Ldr(temp.W(), MemOperand(temp.X()));
    DoReadBarrierOn(temp);
    if (generate_clinit_check) {
      vixl::aarch64::Label cont;
      UseScratchRegisterScope temps2(GetVIXLAssembler());
      InvokeRuntimeCallingConvention calling_convention;
      Register reg = temps2.AcquireW();
      __ Ldrb(reg, HeapOperand(temp.W(), kClassStatusByteOffset));
      __ Cmp(reg, kShiftedVisiblyInitializedValue);
      __ B(hs, &cont);
      __ Mov(calling_convention.GetRegisterAt(0).W(), temp.W());
      InvokeRuntime(kQuickInitializeStaticStorage, dex_pc);
      // Reload the class in the temporary register.
      __ Ldr(temp.W(), jit_patches_.DeduplicateJitClassLiteral(h_klass->GetDexFile(),
                                                               h_klass->GetDexTypeIndex(),
                                                               h_klass,
                                                               code_generation_data_.get()));
      __ Ldr(temp.W(), MemOperand(temp.X()));
      DoReadBarrierOn(temp);
      __ Bind(&cont);
    }
  }
  MemOperand mem = HeapOperand(temp.W(), field->GetOffset());
  if (is_put) {
    return DoPut(mem,
                 temp,
                 field,
                 instruction.Opcode(),
                 source_or_dest_reg,
                 /* can_receiver_be_null= */ false,
                 is_object,
                 field->IsVolatile(),
                 dex_pc);
  }
  return DoGet(mem,
               field_index,
               instruction.Opcode(),
               source_or_dest_reg,
               /* can_receiver_be_null= */ false,
               is_object,
               field->IsVolatile(),
               dex_pc,
               next);
}

void FastCompilerARM64::Div(Register dst, Register first, Register second, uint32_t dex_pc) {
  vixl::aarch64::Label cont;
  __ Cbnz(second, &cont);
  InvokeRuntime(kQuickThrowDivZero, dex_pc);
  __ Bind(&cont);
  __ Sdiv(dst, first, second);
}

void FastCompilerARM64::Rem(Register dst, Register first, Register second, uint32_t dex_pc) {
  vixl::aarch64::Label cont;
  __ Cbnz(second, &cont);
  InvokeRuntime(kQuickThrowDivZero, dex_pc);
  __ Bind(&cont);
  UseScratchRegisterScope temps(GetVIXLAssembler());
  Register temp = temps.AcquireSameSizeAs(dst);
  __ Sdiv(temp, first, second);
  __ Msub(dst, temp, second, first);
}

bool FastCompilerARM64::Frem(CPURegister dst,
                             CPURegister first,
                             CPURegister second,
                             uint32_t dex_pc,
                             DataType::Type type) {
  DCHECK(has_frame_);
  InvokeRuntimeCallingConvention calling_convention;
  if (!MoveLocation(LocationFrom(calling_convention.GetFpuRegisterAt(0)),
                    LocationFrom(VRegister(first)),
                    type)) {
    return false;
  }
  if (!MoveLocation(LocationFrom(calling_convention.GetFpuRegisterAt(1)),
                    LocationFrom(VRegister(second)),
                    type)) {
    return false;
  }
  QuickEntrypointEnum entrypoint =
      (type == DataType::Type::kFloat32) ? kQuickFmodf : kQuickFmod;
  InvokeRuntime(entrypoint, dex_pc);
  if (!MoveLocation(LocationFrom(VRegister(dst)),
                    calling_convention.GetReturnLocation(type),
                    type)) {
    return false;
  }
  return true;
}

#define SETUP_UNOP_12x(input_type, output_type) \
  int32_t vreg_a = instruction.VRegA_12x(); \
  CPURegister input = CPURegisterFrom( \
      GetExistingRegisterLocation(instruction.VRegB_12x(), input_type), input_type); \
  CPURegister dst = CPURegisterFrom( \
      CreateNewRegisterLocation(vreg_a, output_type, next), output_type); \
  if (HitUnimplemented()) { \
    return false; \
  } \
  UpdateLocal(vreg_a, /* is_object= */ false, DataType::Is64BitType(output_type));

#define SETUP_BINOP_12x(type, is_shift) \
  int32_t vreg_a = instruction.VRegA_12x(); \
  CPURegister first = CPURegisterFrom(GetExistingRegisterLocation(vreg_a, type), type); \
  DataType::Type second_type = is_shift ? DataType::Type::kInt32 : type; \
  CPURegister second = CPURegisterFrom( \
      GetExistingRegisterLocation(instruction.VRegB_12x(), second_type), type); \
  CPURegister dst = CPURegisterFrom(CreateNewRegisterLocation(vreg_a, type, next), type); \
  if (HitUnimplemented()) { \
    return false; \
  } \
  UpdateLocal(vreg_a, /* is_object= */ false, DataType::Is64BitType(type));

#define SETUP_BINOP_23x(type, is_shift) \
  int32_t vreg_a = instruction.VRegA_23x(); \
  CPURegister first = CPURegisterFrom( \
      GetExistingRegisterLocation(instruction.VRegB_23x(), type), type); \
  DataType::Type second_type = is_shift ? DataType::Type::kInt32 : type; \
  CPURegister second = CPURegisterFrom( \
      GetExistingRegisterLocation(instruction.VRegC_23x(), second_type), type); \
  CPURegister dst = CPURegisterFrom(CreateNewRegisterLocation(vreg_a, type, next), type); \
  if (HitUnimplemented()) { \
    return false; \
  } \
  UpdateLocal(vreg_a, /* is_object= */ false, DataType::Is64BitType(type));

#define SIMPLE_BINOP_12x(type, instruction, is_shift) \
    SETUP_BINOP_12x(type, is_shift) \
    __ instruction(Register(dst), Register(first), Register(second)); \
    return true;

#define SIMPLE_FPU_BINOP_12x(type, instruction) \
    SETUP_BINOP_12x(type, false) \
    __ instruction(VRegister(dst), VRegister(first), VRegister(second)); \
    return true;

#define SIMPLE_BINOP_23x(type, instruction, is_shift) \
    SETUP_BINOP_23x(type, is_shift) \
    __ instruction(Register(dst), Register(first), Register(second)); \
    return true;

#define SIMPLE_FPU_BINOP_23x(type, instruction) \
    SETUP_BINOP_23x(type, false) \
    __ instruction(VRegister(dst), VRegister(first), VRegister(second)); \
    return true;

#define FRAME_BINOP_12x(type, instruction) \
    if (!EnsureHasFrame()) { \
      return false; \
    } \
    SETUP_BINOP_12x(type, false) \
    instruction(Register(dst), Register(first), Register(second), dex_pc); \
    return true;

#define FRAME_BINOP_23x(type, instruction) \
    if (!EnsureHasFrame()) { \
      return false; \
    } \
    SETUP_BINOP_23x(type, false) \
    instruction(Register(dst), Register(first), Register(second), dex_pc); \
    return true;


bool FastCompilerARM64::BuildCompare(const Instruction& instruction, const Instruction* next) {
  bool is_gt_bias = (instruction.Opcode() == Instruction::CMPG_FLOAT ||
                     instruction.Opcode() == Instruction::CMPG_DOUBLE);
  bool is_double = (instruction.Opcode() == Instruction::CMPL_DOUBLE ||
                    instruction.Opcode() == Instruction::CMPG_DOUBLE);
  bool is_long = instruction.Opcode() == Instruction::CMP_LONG;
  DataType::Type type = is_long
      ? DataType::Type::kInt64
      : is_double ? DataType::Type::kFloat64 : DataType::Type::kFloat32;
  int32_t vreg_a = instruction.VRegA_23x();
  CPURegister first = CPURegisterFrom(
      GetExistingRegisterLocation(instruction.VRegB_23x(), type), type);
  CPURegister second = CPURegisterFrom(
      GetExistingRegisterLocation(instruction.VRegC_23x(), type), type);
  Register dst = RegisterFrom(
      CreateNewRegisterLocation(vreg_a, DataType::Type::kInt32, next), DataType::Type::kInt32);
  if (HitUnimplemented()) {
    return false;
  }
  UpdateLocal(vreg_a, /* is_object= */ false, /* is_wide= */ false);

  if (is_long) {
    __ Cmp(first.X(), second.X());
  } else {
    __ Fcmp(VRegister(first), VRegister(second));
  }
  __ Cset(dst.W(), ne);                             // result == +1 if NE or 0 otherwise
  __ Cneg(dst.W(), dst.W(), is_gt_bias ? cc : lt);  // result == -1 if LT or unchanged otherwise
  return true;
}

bool FastCompilerARM64::BuildReturn(const Instruction& instruction) {
  int32_t register_index = instruction.VRegA_11x();
  InvokeDexCallingConventionVisitorARM64 convention;
  if (!MoveLocation(convention.GetReturnLocation(return_type_),
                    vreg_locations_[register_index],
                    return_type_)) {
    return false;
  }
  PopFrameAndReturn();
  return true;
}

bool FastCompilerARM64::BuildMoveResult(const Instruction& instruction,
                                        bool is_object,
                                        const Instruction* next) {
  int32_t register_index = instruction.VRegA_11x();
  InvokeDexCallingConventionVisitorARM64 convention;
  Location new_location =
      CreateNewRegisterLocation(register_index, previous_invoke_return_type_, next);
  if (HitUnimplemented()) {
    return false;
  }
  if (!MoveLocation(new_location,
                    convention.GetReturnLocation(previous_invoke_return_type_),
                    previous_invoke_return_type_)) {
    return false;
  }
  UpdateLocal(register_index, is_object, DataType::Is64BitType(previous_invoke_return_type_));
  return true;
}

bool FastCompilerARM64::BuildSwitch(const Instruction& instruction, uint32_t dex_pc) {
  if (!EnsureHasFrame()) {
    return false;
  }
  Register reg = RegisterFrom(
      GetExistingRegisterLocation(instruction.VRegA_31t(), DataType::Type::kInt32),
      DataType::Type::kInt32);
  if (HitUnimplemented()) {
    return false;
  }
  DexSwitchTable table(instruction, dex_pc);

  if (table.GetNumEntries() == 0) {
    return true;
  }
  UseScratchRegisterScope temps(GetVIXLAssembler());
  Register temp = temps.AcquireX();
  MoveConstantsAndFpusToRegisters();
  for (DexSwitchTableIterator it(table); !it.Done(); it.Advance()) {
    int32_t target_offset = it.CurrentTargetOffset();
    if (target_offset <= 0 && !CanHandleBackwardsBranch(dex_pc + target_offset)) {
      return false;
    }
    vixl::aarch64::Label* label = GetLabelOf(dex_pc + target_offset);
    UpdateMasks(dex_pc + it.CurrentTargetOffset());
    AddToWorkQueue(dex_pc + it.CurrentTargetOffset());
    __ Mov(temp.W(), it.CurrentKey());
    __ Cmp(reg, temp.W());
    if (target_offset <= 0) {
      vixl::aarch64::Label cont;
      __ B(ne, &cont);
      GenerateSuspendCheckAndIncrementHotness(dex_pc, temp);
      __ B(label);
      __ Bind(&cont);
    } else {
      __ B(eq, label);
    }
  }
  // The default case is a fallthrough to the next opcode..
  return true;
}

bool FastCompilerARM64::BuildFillArrayData(const Instruction& instruction, uint32_t dex_pc) {
  if (!EnsureHasFrame()) {
    return false;
  }

  uint32_t array_reg = instruction.VRegA_31t();
  Register array = RegisterFrom(
      GetExistingRegisterLocation(array_reg, DataType::Type::kReference),
      DataType::Type::kReference);
  InvokeRuntimeCallingConvention calling_convention;
  Register length = calling_convention.GetRegisterAt(1).W();
  MemOperand mem = HeapOperand(array.W(), mirror::Array::LengthOffset().Uint32Value());
  // Fetch the length, and do a null pointer check.
  {
    // Ensure the pc position is recorded immediately after the load instruction.
    EmissionCheckScope guard(GetVIXLAssembler(), kMaxMacroInstructionSizeInBytes);
    __ Ldr(length, mem);
    if (CanBeNull(array_reg)) {
      RecordPcInfo(dex_pc);
    }
  }

  int32_t payload_offset = instruction.VRegB_31t() + dex_pc;
  const Instruction::ArrayDataPayload* payload =
      reinterpret_cast<const Instruction::ArrayDataPayload*>(
          GetCodeItemAccessor().Insns() + payload_offset);
  uint32_t element_count = payload->element_count;
  if (element_count == 0u) {
    return true;
  }
  Register temp = calling_convention.GetRegisterAt(0);

  {
    __ Mov(temp.W(), element_count - 1);
    // Bounds check.
    __ Cmp(temp.W(), length.W());
    vixl::aarch64::Label cont;
    __ B(vixl::aarch64::lo, &cont);
    InvokeRuntime(kQuickThrowArrayBounds, dex_pc);
    __ Bind(&cont);
  }

  const uint8_t* data = payload->data;
  uint16_t element_width = payload->element_width;
#define STORE(type, data_type, instruction, reg) \
  { \
    size_t offset = mirror::Array::DataOffset(DataType::Size(data_type)).Uint32Value(); \
    for (uint32_t i = 0; i < element_count; ++i) { \
      __ Mov(reg, reinterpret_cast<const type*>(data)[i]); \
      __ instruction(reg, HeapOperand(array, offset)); \
      offset += DataType::Size(data_type); \
    } \
    break; \
  }

  switch (element_width) {
    case 1: STORE(int8_t, DataType::Type::kInt8, Strb, temp.W())
    case 2: STORE(int16_t, DataType::Type::kInt16, Strh, temp.W())
    case 4: STORE(int32_t, DataType::Type::kInt32, Str, temp.W())
    case 8: STORE(int64_t, DataType::Type::kInt64, Str, temp.X())
    default:
      LOG(FATAL) << "Unknown element width for " << element_width;
  }
#undef STORE
  return true;
}

// Don't error on the stack size of `ProcessDexInstruction`, we know we are not
// going to stack overflow in the compiler.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-larger-than="

bool FastCompilerARM64::ProcessDexInstruction(const Instruction& instruction,
                                              uint32_t dex_pc,
                                              const Instruction* next) {
  bool is_object = false;
  switch (instruction.Opcode()) {
    case Instruction::CONST_4: {
      int32_t register_index = instruction.VRegA_11n();
      int32_t constant = instruction.VRegB_11n();
      SetIntConstant(register_index, constant, next);
      return true;
    }

    case Instruction::CONST_16: {
      int32_t register_index = instruction.VRegA_21s();
      int32_t constant = instruction.VRegB_21s();
      SetIntConstant(register_index, constant, next);
      return true;
    }

    case Instruction::CONST: {
      int32_t register_index = instruction.VRegA_31i();
      int32_t constant = instruction.VRegB_31i();
      SetIntConstant(register_index, constant, next);
      return true;
    }

    case Instruction::CONST_HIGH16: {
      int32_t register_index = instruction.VRegA_21h();
      int32_t constant = instruction.VRegB_21h() << 16;
      SetIntConstant(register_index, constant, next);
      return true;
    }

    case Instruction::CONST_WIDE_16: {
      int32_t register_index = instruction.VRegA_21s();
      // Get 16 bits of constant value, sign extended to 64 bits.
      int64_t value = instruction.VRegB_21s();
      value <<= 48;
      value >>= 48;
      SetLongConstant(register_index, value, next);
      return true;
    }

    case Instruction::CONST_WIDE_32: {
      int32_t register_index = instruction.VRegA_31i();
      // Get 32 bits of constant value, sign extended to 64 bits.
      int64_t value = instruction.VRegB_31i();
      value <<= 32;
      value >>= 32;
      SetLongConstant(register_index, value, next);
      return true;
    }

    case Instruction::CONST_WIDE: {
      int32_t register_index = instruction.VRegA_51l();
      int64_t value = instruction.VRegB_51l();
      SetLongConstant(register_index, value, next);
      return true;
    }

    case Instruction::CONST_WIDE_HIGH16: {
      int32_t register_index = instruction.VRegA_21h();
      int64_t value = static_cast<int64_t>(instruction.VRegB_21h()) << 48;
      SetLongConstant(register_index, value, next);
      return true;
    }

    case Instruction::MOVE: {
      return BuildMove(
          instruction.VRegA_12x(), instruction.VRegB_12x(), DataType::Type::kInt32, next);
    }
    case Instruction::MOVE_FROM16: {
      return BuildMove(
          instruction.VRegA_22x(), instruction.VRegB_22x(), DataType::Type::kInt32, next);
    }
    case Instruction::MOVE_16: {
      return BuildMove(
          instruction.VRegA_32x(), instruction.VRegB_32x(), DataType::Type::kInt32, next);
    }

    case Instruction::MOVE_WIDE: {
      return BuildMove(
          instruction.VRegA_12x(), instruction.VRegB_12x(), DataType::Type::kInt64, next);
    }

    case Instruction::MOVE_WIDE_FROM16: {
      return BuildMove(
          instruction.VRegA_22x(), instruction.VRegB_22x(), DataType::Type::kInt64, next);
    }

    case Instruction::MOVE_WIDE_16: {
      return BuildMove(
          instruction.VRegA_32x(), instruction.VRegB_32x(), DataType::Type::kInt64, next);
    }

    case Instruction::MOVE_OBJECT: {
      return BuildMove(
          instruction.VRegA_12x(), instruction.VRegB_12x(), DataType::Type::kReference, next);
    }
    case Instruction::MOVE_OBJECT_FROM16: {
      return BuildMove(
          instruction.VRegA_22x(), instruction.VRegB_22x(), DataType::Type::kReference, next);
    }
    case Instruction::MOVE_OBJECT_16: {
      return BuildMove(
          instruction.VRegA_32x(), instruction.VRegB_32x(), DataType::Type::kReference, next);
    }

    case Instruction::RETURN_VOID: {
      if (method_->IsConstructor() &&
          !method_->IsStatic() &&
          dex_compilation_unit_.RequiresConstructorBarrier()) {
        __ Dmb(InnerShareable, BarrierWrites);
      }
      PopFrameAndReturn();
      return true;
    }

#define IF_XX(comparison, cond) \
    case Instruction::IF_##cond: \
      return If_21_22t<comparison, /* kCompareWithZero= */ false>(instruction, dex_pc); \
    case Instruction::IF_##cond##Z: \
      return If_21_22t<comparison, /* kCompareWithZero= */ true>(instruction, dex_pc);

    IF_XX(vixl::aarch64::eq, EQ);
    IF_XX(vixl::aarch64::ne, NE);
    IF_XX(vixl::aarch64::lt, LT);
    IF_XX(vixl::aarch64::le, LE);
    IF_XX(vixl::aarch64::gt, GT);
    IF_XX(vixl::aarch64::ge, GE);
#undef IF_XX

    case Instruction::GOTO:
    case Instruction::GOTO_16:
    case Instruction::GOTO_32: {
      if (!EnsureHasFrame()) {
        return false;
      }
      int32_t target_offset = instruction.GetTargetOffset();
      if (target_offset <= 0) {
        if (!CanHandleBackwardsBranch(dex_pc + target_offset)) {
          return false;
        }
        UseScratchRegisterScope temps(GetVIXLAssembler());
        GenerateSuspendCheckAndIncrementHotness(dex_pc, temps.AcquireX());
      }
      PrepareToBranch(dex_pc + target_offset);
      vixl::aarch64::Label* label = GetLabelOf(dex_pc + target_offset);
      __ B(label);
      return true;
    }

    case Instruction::RETURN:
    case Instruction::RETURN_OBJECT:
    case Instruction::RETURN_WIDE: {
      return BuildReturn(instruction);
    }

    case Instruction::INVOKE_DIRECT:
    case Instruction::INVOKE_DIRECT_RANGE:
      return HandleInvoke(instruction, dex_pc, kDirect);
    case Instruction::INVOKE_INTERFACE:
    case Instruction::INVOKE_INTERFACE_RANGE:
      return HandleInvoke(instruction, dex_pc, kInterface);
    case Instruction::INVOKE_STATIC:
    case Instruction::INVOKE_STATIC_RANGE:
      return HandleInvoke(instruction, dex_pc, kStatic);
    case Instruction::INVOKE_SUPER:
    case Instruction::INVOKE_SUPER_RANGE:
      return HandleInvoke(instruction, dex_pc, kSuper);
    case Instruction::INVOKE_VIRTUAL:
    case Instruction::INVOKE_VIRTUAL_RANGE: {
      return HandleInvoke(instruction, dex_pc, kVirtual);
    }

    case Instruction::INVOKE_POLYMORPHIC: {
      break;
    }

    case Instruction::INVOKE_POLYMORPHIC_RANGE: {
      break;
    }

    case Instruction::INVOKE_CUSTOM: {
      break;
    }

    case Instruction::INVOKE_CUSTOM_RANGE: {
      break;
    }

    case Instruction::NEG_FLOAT: {
      SETUP_UNOP_12x(DataType::Type::kFloat32, DataType::Type::kFloat32)
      __ Fneg(dst.S(), input.S());
      return true;
    }

    case Instruction::NEG_DOUBLE: {
      SETUP_UNOP_12x(DataType::Type::kFloat64, DataType::Type::kFloat64)
      __ Fneg(dst.D(), input.D());
      return true;
    }

    case Instruction::NEG_INT: {
      SETUP_UNOP_12x(DataType::Type::kInt32, DataType::Type::kInt32)
      __ Neg(dst.W(), input.W());
      return true;
    }

    case Instruction::NEG_LONG: {
      SETUP_UNOP_12x(DataType::Type::kInt64, DataType::Type::kInt64)
      __ Neg(dst.X(), input.X());
      return true;
    }


    case Instruction::NOT_INT: {
      SETUP_UNOP_12x(DataType::Type::kInt32, DataType::Type::kInt32)
      __ Mvn(dst.W(), input.W());
      return true;
    }

    case Instruction::NOT_LONG: {
      SETUP_UNOP_12x(DataType::Type::kInt64, DataType::Type::kInt64)
      __ Mvn(dst.X(), input.X());
      return true;
    }

    case Instruction::INT_TO_LONG: {
      SETUP_UNOP_12x(DataType::Type::kInt32, DataType::Type::kInt64)
      __ Sbfx(
          dst.X(), input.X(), /* lsb= */ 0u, DataType::Size(DataType::Type::kInt32) * kBitsPerByte);
      return true;
    }

    case Instruction::LONG_TO_INT: {
      SETUP_UNOP_12x(DataType::Type::kInt64, DataType::Type::kInt32)
      __ Mov(dst.W(), input.W());
      return true;
    }

    case Instruction::INT_TO_BYTE: {
      SETUP_UNOP_12x(DataType::Type::kInt32, DataType::Type::kInt32)
      __ Sbfx(
          dst.W(), input.W(), /* lsb= */ 0u, DataType::Size(DataType::Type::kInt8) * kBitsPerByte);
      return true;
    }

    case Instruction::INT_TO_SHORT: {
      SETUP_UNOP_12x(DataType::Type::kInt32, DataType::Type::kInt32)
      __ Sbfx(
          dst.W(), input.W(), /* lsb= */ 0u, DataType::Size(DataType::Type::kInt16) * kBitsPerByte);
      return true;
    }

    case Instruction::INT_TO_CHAR: {
      SETUP_UNOP_12x(DataType::Type::kInt32, DataType::Type::kInt32)
      __ Ubfx(
          dst.W(), input.W(), /* lsb= */ 0u, DataType::Size(DataType::Type::kInt16) * kBitsPerByte);
      return true;
    }

    case Instruction::INT_TO_FLOAT: {
      SETUP_UNOP_12x(DataType::Type::kInt32, DataType::Type::kFloat32)
      __ Scvtf(dst.S(), input.W());
      return true;
    }

    case Instruction::INT_TO_DOUBLE: {
      SETUP_UNOP_12x(DataType::Type::kInt32, DataType::Type::kFloat64)
      __ Scvtf(dst.D(), input.W());
      return true;
    }

    case Instruction::LONG_TO_FLOAT: {
      SETUP_UNOP_12x(DataType::Type::kInt64, DataType::Type::kFloat32)
      __ Scvtf(dst.S(), input.X());
      return true;
    }

    case Instruction::LONG_TO_DOUBLE: {
      SETUP_UNOP_12x(DataType::Type::kInt64, DataType::Type::kFloat64)
      __ Scvtf(dst.D(), input.X());
      return true;
    }

    case Instruction::FLOAT_TO_INT: {
      SETUP_UNOP_12x(DataType::Type::kFloat32, DataType::Type::kInt32)
      __ Fcvtzs(dst.W(), input.S());
      return true;
    }

    case Instruction::FLOAT_TO_LONG: {
      SETUP_UNOP_12x(DataType::Type::kFloat32, DataType::Type::kInt64)
      __ Fcvtzs(dst.X(), input.S());
      return true;
    }

    case Instruction::FLOAT_TO_DOUBLE: {
      SETUP_UNOP_12x(DataType::Type::kFloat32, DataType::Type::kFloat64)
      __ Fcvt(dst.D(), input.S());
      return true;
    }

    case Instruction::DOUBLE_TO_INT: {
      SETUP_UNOP_12x(DataType::Type::kFloat64, DataType::Type::kInt32)
      __ Fcvtzs(dst.W(), input.D());
      return true;
    }

    case Instruction::DOUBLE_TO_LONG: {
      SETUP_UNOP_12x(DataType::Type::kFloat64, DataType::Type::kInt64)
      __ Fcvtzs(dst.X(), input.D());
      return true;
    }

    case Instruction::DOUBLE_TO_FLOAT: {
      SETUP_UNOP_12x(DataType::Type::kFloat64, DataType::Type::kFloat32)
      __ Fcvt(dst.S(), input.D());
      return true;
    }

#define REM_BINOP(type, format) \
    if (!EnsureHasFrame()) { \
      return false; \
    } \
    SETUP_BINOP_##format(type, false); \
    return Frem(dst, first, second, dex_pc, type);

    case Instruction::REM_FLOAT: {
      REM_BINOP(DataType::Type::kFloat32, 23x);
    }

    case Instruction::REM_DOUBLE: {
      REM_BINOP(DataType::Type::kFloat64, 23x);
    }

    case Instruction::REM_FLOAT_2ADDR: {
      REM_BINOP(DataType::Type::kFloat32, 12x);
    }

    case Instruction::REM_DOUBLE_2ADDR: {
      REM_BINOP(DataType::Type::kFloat64, 12x);
    }
#undef REM_BINOP

#define SIMPLE_FPU_OP_CASE(opcode, instruction) \
    case Instruction::opcode ##_FLOAT_2ADDR: { \
      SIMPLE_FPU_BINOP_12x(DataType::Type::kFloat32, instruction) \
    } \
    case Instruction::opcode ##_DOUBLE_2ADDR: { \
      SIMPLE_FPU_BINOP_12x(DataType::Type::kFloat64, instruction) \
    } \
    case Instruction::opcode ##_FLOAT: { \
      SIMPLE_FPU_BINOP_23x(DataType::Type::kFloat32, instruction) \
    } \
    case Instruction::opcode ##_DOUBLE: { \
      SIMPLE_FPU_BINOP_23x(DataType::Type::kFloat64, instruction) \
    }

    SIMPLE_FPU_OP_CASE(ADD, Fadd)
    SIMPLE_FPU_OP_CASE(SUB, Fsub)
    SIMPLE_FPU_OP_CASE(MUL, Fmul)
    SIMPLE_FPU_OP_CASE(DIV, Fdiv)
#undef SIMPLE_FPU_OP_CASE

#define SIMPLE_OP_CASE(opcode, instruction, is_shift) \
    case Instruction::opcode ##_INT_2ADDR: { \
      SIMPLE_BINOP_12x(DataType::Type::kInt32, instruction, is_shift) \
    } \
    case Instruction::opcode ##_LONG_2ADDR: { \
      SIMPLE_BINOP_12x(DataType::Type::kInt64, instruction, is_shift) \
    } \
    case Instruction::opcode ##_INT: { \
      SIMPLE_BINOP_23x(DataType::Type::kInt32, instruction, is_shift) \
    } \
    case Instruction::opcode ##_LONG: { \
      SIMPLE_BINOP_23x(DataType::Type::kInt64, instruction, is_shift) \
    }

    SIMPLE_OP_CASE(ADD, Add, false)
    SIMPLE_OP_CASE(SUB, Sub, false)
    SIMPLE_OP_CASE(MUL, Mul, false)
    SIMPLE_OP_CASE(AND, And, false)
    SIMPLE_OP_CASE(OR, Orr, false)
    SIMPLE_OP_CASE(XOR, Eor, false)
    SIMPLE_OP_CASE(SHL, Lsl, true)
    SIMPLE_OP_CASE(SHR, Asr, true)
    SIMPLE_OP_CASE(USHR, Lsr, true)
#undef SIMPLE_OP_CASE

#define DIV_REM_OP_CASE(opcode, instruction) \
    case Instruction::opcode ##_INT_2ADDR: { \
      FRAME_BINOP_12x(DataType::Type::kInt32, instruction) \
    } \
    case Instruction::opcode ##_LONG_2ADDR: { \
      FRAME_BINOP_12x(DataType::Type::kInt64, instruction) \
    } \
    case Instruction::opcode ##_INT: { \
      FRAME_BINOP_23x(DataType::Type::kInt32, instruction) \
    } \
    case Instruction::opcode ##_LONG: { \
      FRAME_BINOP_23x(DataType::Type::kInt64, instruction) \
    }

    DIV_REM_OP_CASE(DIV, Div)
    DIV_REM_OP_CASE(REM, Rem)
#undef DIV_OP_CASE

#define SETUP_BINOP_22(suffix) \
  Register source = RegisterFrom( \
      GetExistingRegisterLocation(instruction.VRegB_22 ## suffix(), DataType::Type::kInt32), \
      DataType::Type::kInt32); \
  int32_t register_index = instruction.VRegA_22 ## suffix(); \
  Register result = RegisterFrom( \
      CreateNewRegisterLocation(register_index, DataType::Type::kInt32, next), \
      DataType::Type::kInt32); \
  if (HitUnimplemented()) { \
    return false; \
  } \
  int16_t constant = instruction.VRegC_22 ## suffix(); \
  UpdateLocal(register_index, /* is_object= */ false, /* is_wide= */ false);

    case Instruction::ADD_INT_LIT16: {
      SETUP_BINOP_22(s)
      __ Add(result, source, constant);
      return true;
    }

    case Instruction::AND_INT_LIT16: {
      SETUP_BINOP_22(s)
      __ And(result, source, constant);
      return true;
    }

    case Instruction::OR_INT_LIT16: {
      SETUP_BINOP_22(s)
      __ Orr(result, source, constant);
      return true;
    }

    case Instruction::XOR_INT_LIT16: {
      SETUP_BINOP_22(s)
      __ Eor(result, source, constant);
      return true;
    }

    case Instruction::MUL_INT_LIT16: {
      SETUP_BINOP_22(s)
      UseScratchRegisterScope temps(GetVIXLAssembler());
      Register second = temps.AcquireW();
      __ Mov(second, constant);
      __ Mul(result, source, second);
      return true;
    }

    case Instruction::DIV_INT_LIT16: {
      SETUP_BINOP_22(s)
      if (constant == 0) {
        if (!EnsureHasFrame()) {
          return false;
        }
        InvokeRuntime(kQuickThrowDivZero, dex_pc);
      } else {
        UseScratchRegisterScope temps(GetVIXLAssembler());
        Register second = temps.AcquireW();
        __ Mov(second, constant);
        __ Sdiv(result, source, second);
      }
      return true;
    }

    case Instruction::RSUB_INT: {
      SETUP_BINOP_22(s)
      UseScratchRegisterScope temps(GetVIXLAssembler());
      Register second = temps.AcquireW();
      __ Mov(second, constant);
      __ Sub(result, second, source);
      return true;
    }

    case Instruction::REM_INT_LIT16: {
      SETUP_BINOP_22(s)
      if (constant == 0) {
        if (!EnsureHasFrame()) {
          return false;
        }
        InvokeRuntime(kQuickThrowDivZero, dex_pc);
      } else {
        UseScratchRegisterScope temps(GetVIXLAssembler());
        Register second = temps.AcquireW();
        Register temp = temps.AcquireW();
        __ Mov(second, constant);
        __ Sdiv(temp, source, second);
        __ Msub(result, temp, second, source);
      }
      return true;
    }

    case Instruction::ADD_INT_LIT8: {
      SETUP_BINOP_22(b)
      __ Add(result, source, constant);
      return true;
    }

    case Instruction::AND_INT_LIT8: {
      SETUP_BINOP_22(b)
      __ And(result, source, constant);
      return true;
    }

    case Instruction::OR_INT_LIT8: {
      SETUP_BINOP_22(b)
      __ Orr(result, source, constant);
      return true;
    }

    case Instruction::XOR_INT_LIT8: {
      SETUP_BINOP_22(b)
      __ Eor(result, source, constant);
      return true;
    }

    case Instruction::RSUB_INT_LIT8: {
      SETUP_BINOP_22(b)
      UseScratchRegisterScope temps(GetVIXLAssembler());
      Register second = temps.AcquireW();
      __ Mov(second, constant);
      __ Sub(result, second, source);
      return true;
    }

    case Instruction::MUL_INT_LIT8: {
      SETUP_BINOP_22(b)
      UseScratchRegisterScope temps(GetVIXLAssembler());
      Register second = temps.AcquireW();
      __ Mov(second, constant);
      __ Mul(result, source, second);
      return true;
    }

    case Instruction::DIV_INT_LIT8: {
      SETUP_BINOP_22(b)
      if (constant == 0) {
        if (!EnsureHasFrame()) {
          return false;
        }
        InvokeRuntime(kQuickThrowDivZero, dex_pc);
      } else {
        UseScratchRegisterScope temps(GetVIXLAssembler());
        Register second = temps.AcquireW();
        __ Mov(second, constant);
        __ Sdiv(result, source, second);
      }
      return true;
    }

    case Instruction::REM_INT_LIT8: {
      SETUP_BINOP_22(b)
      if (constant == 0) {
        if (!EnsureHasFrame()) {
          return false;
        }
        InvokeRuntime(kQuickThrowDivZero, dex_pc);
      } else {
        UseScratchRegisterScope temps(GetVIXLAssembler());
        Register second = temps.AcquireW();
        Register temp = temps.AcquireW();
        __ Mov(second, constant);
        __ Sdiv(temp, source, second);
        __ Msub(result, temp, second, source);
      }
      return true;
    }

    case Instruction::SHL_INT_LIT8: {
      SETUP_BINOP_22(b)
      constant &= kMaxIntShiftDistance;
      __ Lsl(result, source, constant);
      return true;
    }

    case Instruction::SHR_INT_LIT8: {
      SETUP_BINOP_22(b)
      constant &= kMaxIntShiftDistance;
      __ Asr(result, source, constant);
      return true;
    }

    case Instruction::USHR_INT_LIT8: {
      SETUP_BINOP_22(b)
      constant &= kMaxIntShiftDistance;
      __ Lsr(result, source, constant);
      return true;
    }

    case Instruction::NEW_INSTANCE: {
      dex::TypeIndex type_index(instruction.VRegB_21c());
      return BuildNewInstance(instruction.VRegA_21c(), type_index, dex_pc, next);
    }

    case Instruction::NEW_ARRAY: {
      return BuildNewArray(instruction, dex_pc, next);
    }

    case Instruction::FILLED_NEW_ARRAY: {
      dex::TypeIndex type_index(instruction.VRegB_35c());
      uint32_t args[5];
      uint32_t number_of_vreg_arguments = instruction.GetVarArgs(args);
      VarArgsInstructionOperands operands(args, number_of_vreg_arguments);
      return BuildFilledNewArray(dex_pc, type_index, operands);
    }

    case Instruction::FILLED_NEW_ARRAY_RANGE: {
      dex::TypeIndex type_index(instruction.VRegB_3rc());
      RangeInstructionOperands operands(instruction.VRegC_3rc(), instruction.VRegA_3rc());
      return BuildFilledNewArray(dex_pc, type_index, operands);
    }

    case Instruction::FILL_ARRAY_DATA: {
      return BuildFillArrayData(instruction, dex_pc);
    }

    case Instruction::MOVE_RESULT_OBJECT:
      is_object = true;
      FALLTHROUGH_INTENDED;
    case Instruction::MOVE_RESULT:
    case Instruction::MOVE_RESULT_WIDE: {
      return BuildMoveResult(instruction, is_object, next);
    }

    case Instruction::CMP_LONG:
    case Instruction::CMPG_DOUBLE:
    case Instruction::CMPL_DOUBLE:
    case Instruction::CMPG_FLOAT:
    case Instruction::CMPL_FLOAT: {
      return BuildCompare(instruction, next);
    }

    case Instruction::NOP:
      return true;

    case Instruction::IGET_OBJECT:
      is_object = true;
      FALLTHROUGH_INTENDED;
    case Instruction::IGET:
    case Instruction::IGET_WIDE:
    case Instruction::IGET_BOOLEAN:
    case Instruction::IGET_BYTE:
    case Instruction::IGET_CHAR:
    case Instruction::IGET_SHORT: {
      return BuildInstanceFieldGet(instruction, dex_pc, is_object, next);
    }

    case Instruction::IPUT_OBJECT:
      is_object = true;
      FALLTHROUGH_INTENDED;
    case Instruction::IPUT:
    case Instruction::IPUT_WIDE:
    case Instruction::IPUT_BOOLEAN:
    case Instruction::IPUT_BYTE:
    case Instruction::IPUT_CHAR:
    case Instruction::IPUT_SHORT: {
      return BuildInstanceFieldSet(instruction, dex_pc, is_object);
    }

    case Instruction::SGET_OBJECT:
      is_object = true;
      FALLTHROUGH_INTENDED;
    case Instruction::SGET:
    case Instruction::SGET_WIDE:
    case Instruction::SGET_BOOLEAN:
    case Instruction::SGET_BYTE:
    case Instruction::SGET_CHAR:
    case Instruction::SGET_SHORT: {
      return BuildStaticFieldAccess(instruction, dex_pc, is_object, /* is_put= */ false, next);
    }

    case Instruction::SPUT_OBJECT:
      is_object = true;
      FALLTHROUGH_INTENDED;
    case Instruction::SPUT:
    case Instruction::SPUT_WIDE:
    case Instruction::SPUT_BOOLEAN:
    case Instruction::SPUT_BYTE:
    case Instruction::SPUT_CHAR:
    case Instruction::SPUT_SHORT: {
      return BuildStaticFieldAccess(instruction, dex_pc, is_object, /* is_put= */ true, next);
    }

#define ARRAY_XX(kind, type)                                                         \
    case Instruction::AGET##kind: {                                                  \
      return BuildArrayAccess(instruction, dex_pc, /* is_put= */ false, type, next); \
    }                                                                                \
    case Instruction::APUT##kind: {                                                  \
      return BuildArrayAccess(instruction, dex_pc, /* is_put= */ true, type, next);  \
    }

    ARRAY_XX(, DataType::Type::kInt32)
    ARRAY_XX(_WIDE, DataType::Type::kInt64)
    ARRAY_XX(_OBJECT, DataType::Type::kReference)
    ARRAY_XX(_BOOLEAN, DataType::Type::kBool)
    ARRAY_XX(_BYTE, DataType::Type::kInt8)
    ARRAY_XX(_CHAR, DataType::Type::kUint16)
    ARRAY_XX(_SHORT, DataType::Type::kInt16)
#undef ARRAY_XX

    case Instruction::ARRAY_LENGTH: {
      return BuildArrayLength(instruction, dex_pc, next);
    }

    case Instruction::CONST_STRING: {
      dex::StringIndex string_index(instruction.VRegB_21c());
      return BuildLoadString(instruction.VRegA_21c(), string_index, next);
    }

    case Instruction::CONST_STRING_JUMBO: {
      dex::StringIndex string_index(instruction.VRegB_31c());
      return BuildLoadString(instruction.VRegA_31c(), string_index, next);
    }

    case Instruction::CONST_CLASS: {
      dex::TypeIndex type_index(instruction.VRegB_21c());
      return BuildLoadClass(instruction.VRegA_21c(), type_index, next);
    }

    case Instruction::CONST_METHOD_HANDLE: {
      break;
    }

    case Instruction::CONST_METHOD_TYPE: {
      break;
    }

    case Instruction::MOVE_EXCEPTION: {
      int32_t register_index = instruction.VRegA_11x();
      Location new_location =
          CreateNewRegisterLocation(register_index, DataType::Type::kReference, next);
      MemOperand exception =
          MemOperand(tr, Thread::ExceptionOffset<kArm64PointerSize>().Int32Value());
      __ Ldr(WRegisterFrom(new_location), exception);
      __ Str(wzr, exception);
      UpdateLocal(
          register_index, /* is_object= */ true, /* is_wide= */ false, /* can_be_null= */ false);
      return true;
    }

    case Instruction::THROW: {
      return BuildInvokeRuntime11x(kQuickDeliverException, instruction, dex_pc);
    }

    case Instruction::INSTANCE_OF: {
      uint8_t destination = instruction.VRegA_22c();
      uint8_t reference = instruction.VRegB_22c();
      dex::TypeIndex type_index(instruction.VRegC_22c());
      return BuildInstanceOf(reference, destination, type_index, dex_pc, next);
    }

    case Instruction::CHECK_CAST: {
      uint8_t reference = instruction.VRegA_21c();
      dex::TypeIndex type_index(instruction.VRegB_21c());
      return BuildCheckCast(reference, type_index, dex_pc);
    }

    case Instruction::MONITOR_ENTER: {
      // We can start collecting vreg info per stack map at this point, as the
      // runtime will only start expecting them after getting a report of an
      // Object in a dex register being locked. For any stack maps before that
      // monitor-enter, we know the runtime won't expect it.
      // Note: once we support backwards branching, we'll need to know
      // beforehand if a method has monitor operations.
      needs_vreg_info_ = true;
      return BuildInvokeRuntime11x(kQuickLockObject, instruction, dex_pc);
    }

    case Instruction::MONITOR_EXIT: {
      // We don't support backwards branch yet, so we must have seen the
      // monitor-enter before this instruction.
      DCHECK(needs_vreg_info_);
      return BuildInvokeRuntime11x(kQuickUnlockObject, instruction, dex_pc);
    }

    case Instruction::SPARSE_SWITCH:
    case Instruction::PACKED_SWITCH: {
      return BuildSwitch(instruction, dex_pc);
    }

    case Instruction::UNUSED_3E ... Instruction::UNUSED_43:
    case Instruction::UNUSED_73:
    case Instruction::UNUSED_79:
    case Instruction::UNUSED_7A:
    case Instruction::UNUSED_E3 ... Instruction::UNUSED_F9: {
      break;
    }
  }
  unimplemented_reason_ = instruction.Name();
  return false;
}  // NOLINT(readability/fn_size)


bool FastCompilerARM64::AnalyzeBranches() {
  for (const DexInstructionPcPair& pair : GetCodeItemAccessor()) {
    const uint32_t dex_pc = pair.DexPc();
    const Instruction& instruction = pair.Inst();

    if (instruction.IsBranch()) {
      int32_t target_offset = instruction.GetTargetOffset();
      branch_pcs_.SetBit(dex_pc + target_offset);
    } else if (instruction.IsSwitch()) {
      DexSwitchTable table(instruction, dex_pc);
      for (DexSwitchTableIterator s_it(table); !s_it.Done(); s_it.Advance()) {
        int32_t target_offset = s_it.CurrentTargetOffset();
        branch_pcs_.SetBit(dex_pc + target_offset);
      }
    }
  }

  if (GetCodeItemAccessor().TriesSize() != 0) {
    const uint8_t* handlers_ptr = GetCodeItemAccessor().GetCatchHandlerData();
    uint32_t handlers_size = DecodeUnsignedLeb128(&handlers_ptr);
    for (uint32_t idx = 0; idx < handlers_size; ++idx) {
      CatchHandlerIterator iterator(handlers_ptr);
      for (; iterator.HasNext(); iterator.Next()) {
        branch_pcs_.SetBit(iterator.GetHandlerAddress());
      }
      handlers_ptr = iterator.EndDataPointer();
    }
  }

  // The existing masks have been updated with parameter information.
  // Save them and restore them after this analysis.
  uint64_t saved_object_register_mask = object_register_mask_;
  ArenaBitVector saved_object_stack_mask(allocator_, /*start_bits=*/ 0, /*expandable=*/ true);
  saved_object_stack_mask.Copy(&object_stack_mask_);

  AddToWorkQueue(/* dex_pc= */ 0u);
  UpdateObjectMasks(/* dex_pc= */ 0u);
  while (!work_queue_.empty()) {
    uint32_t dex_pc = work_queue_.top();
    work_queue_.pop();
    if (!ProcessBlockForMasks(dex_pc)) {
      return false;
    }
  }
  object_register_mask_ = saved_object_register_mask;
  object_stack_mask_.Copy(&saved_object_stack_mask);
  return true;
}

bool FastCompilerARM64::UpdateObjectMasks(uint32_t dex_pc) {
  bool changed = false;
  uint64_t old_mask = object_register_masks_[dex_pc];
  object_register_masks_[dex_pc] &= object_register_mask_;
  changed = (old_mask != object_register_masks_[dex_pc]);

  if (object_stack_masks_[dex_pc] == nullptr) {
    object_stack_masks_[dex_pc] =
        new (allocator_) ArenaBitVector(allocator_, /*start_bits=*/ 0, /*expandable=*/ true);
    object_stack_masks_[dex_pc]->Copy(&object_stack_mask_);
    changed = true;
  } else if (!object_stack_masks_[dex_pc]->IsSubsetOf(&object_stack_mask_)) {
    object_stack_masks_[dex_pc]->Intersect(&object_stack_mask_);
    changed = true;
  }
  return changed;
}

bool FastCompilerARM64::ProcessBlockForMasks(uint32_t dex_pc) {
  DexInstructionIterator it = GetCodeItemAccessor().InstructionsFrom(dex_pc).begin();
  DexInstructionIterator end = GetCodeItemAccessor().end();

  object_register_mask_ = object_register_masks_[dex_pc];
  DCHECK_NE(object_stack_masks_[dex_pc], nullptr);
  object_stack_mask_.Copy(object_stack_masks_[dex_pc]);

  do {
    DexInstructionPcPair pair = *it;
    const Instruction& instruction = pair.Inst();

    // If the instruction can throw, emulate a branch to each catch handler.
    if (GetCodeItemAccessor().TriesSize() != 0 &&
        (Instruction::FlagsOf(instruction.Opcode()) & Instruction::kThrow) != 0) {
      const dex::TryItem* try_item = GetCodeItemAccessor().FindTryItem(pair.DexPc());
      if (try_item != nullptr) {
        for (CatchHandlerIterator iterator(GetCodeItemAccessor(), *try_item);
             iterator.HasNext();
             iterator.Next()) {
          if (UpdateObjectMasks(iterator.GetHandlerAddress())) {
            AddToWorkQueue(iterator.GetHandlerAddress());
          }
        }
      }
    }

    if (instruction.IsBranch()) {
      int32_t target_offset = instruction.GetTargetOffset();
      if (UpdateObjectMasks(dex_pc + target_offset)) {
        AddToWorkQueue(dex_pc + target_offset);
      }
    } else if (instruction.IsSwitch()) {
      DexSwitchTable table(instruction, dex_pc);
      for (DexSwitchTableIterator s_it(table); !s_it.Done(); s_it.Advance()) {
        int32_t target_offset = s_it.CurrentTargetOffset();
        if (UpdateObjectMasks(dex_pc + target_offset)) {
          AddToWorkQueue(dex_pc + target_offset);
        }
      }
    } else if (!ProcessDexInstructionForMasks(instruction)) {
      DCHECK(HitUnimplemented());
      return false;
    }

    ++it;
    if (it == end || !instruction.CanFlowThrough()) {
      break;
    }
    dex_pc = (*it).DexPc();
    if (IsBranchTarget(dex_pc)) {
      if (UpdateObjectMasks(dex_pc)) {
        AddToWorkQueue(dex_pc);
      }
      break;
    }
  } while (true);
  return true;
}


bool FastCompilerARM64::ProcessDexInstructionForMasks(const Instruction& instruction) {
  switch (instruction.Opcode()) {
    case Instruction::CONST_4:
    case Instruction::CONST_16:
    case Instruction::CONST:
    case Instruction::CONST_HIGH16: {
      bool can_be_object = (instruction.VRegB() == 0);
      UpdateLocal(instruction.VRegA(), can_be_object, /* is_wide= */ false);
      return true;
    }

#define IF_XX(cond) \
    case Instruction::IF_##cond: \
    case Instruction::IF_##cond##Z:
    IF_XX(EQ)
    IF_XX(NE)
    IF_XX(LT)
    IF_XX(LE)
    IF_XX(GT)
    IF_XX(GE)
#undef IF_XX
    case Instruction::GOTO:
    case Instruction::GOTO_16:
    case Instruction::GOTO_32:
    case Instruction::RETURN:
    case Instruction::RETURN_OBJECT:
    case Instruction::RETURN_WIDE:
    case Instruction::INVOKE_DIRECT:
    case Instruction::INVOKE_DIRECT_RANGE:
    case Instruction::INVOKE_INTERFACE:
    case Instruction::INVOKE_INTERFACE_RANGE:
    case Instruction::INVOKE_STATIC:
    case Instruction::INVOKE_STATIC_RANGE:
    case Instruction::INVOKE_SUPER:
    case Instruction::INVOKE_SUPER_RANGE:
    case Instruction::INVOKE_VIRTUAL:
    case Instruction::INVOKE_VIRTUAL_RANGE:
    case Instruction::INVOKE_POLYMORPHIC:
    case Instruction::INVOKE_POLYMORPHIC_RANGE:
    case Instruction::INVOKE_CUSTOM:
    case Instruction::INVOKE_CUSTOM_RANGE:
    case Instruction::RETURN_VOID:
    case Instruction::NOP:
    case Instruction::IPUT_OBJECT:
    case Instruction::IPUT:
    case Instruction::IPUT_BOOLEAN:
    case Instruction::IPUT_BYTE:
    case Instruction::IPUT_CHAR:
    case Instruction::IPUT_SHORT:
    case Instruction::IPUT_WIDE:
    case Instruction::SPUT:
    case Instruction::SPUT_BOOLEAN:
    case Instruction::SPUT_BYTE:
    case Instruction::SPUT_CHAR:
    case Instruction::SPUT_SHORT:
    case Instruction::SPUT_WIDE:
    case Instruction::APUT_WIDE:
    case Instruction::APUT_OBJECT:
    case Instruction::SPUT_OBJECT:
    case Instruction::APUT:
    case Instruction::APUT_BOOLEAN:
    case Instruction::APUT_BYTE:
    case Instruction::APUT_CHAR:
    case Instruction::APUT_SHORT:
    case Instruction::THROW:
    case Instruction::CHECK_CAST:
    case Instruction::MONITOR_ENTER:
    case Instruction::MONITOR_EXIT:
    case Instruction::SPARSE_SWITCH:
    case Instruction::PACKED_SWITCH:
    case Instruction::FILL_ARRAY_DATA: {
      return true;
    }

#define OP_CASE(opcode) \
    case Instruction::opcode ##_INT_2ADDR: \
    case Instruction::opcode ##_INT:
    OP_CASE(ADD)
    OP_CASE(SUB)
    OP_CASE(MUL)
    OP_CASE(AND)
    OP_CASE(OR)
    OP_CASE(XOR)
    OP_CASE(DIV)
    OP_CASE(REM)
#undef OP_CASE
#define OP_CASE(opcode) \
    case Instruction::opcode ##_INT_2ADDR: \
    case Instruction::opcode ##_INT: \
    case Instruction::opcode ##_LONG_2ADDR: \
    case Instruction::opcode ##_LONG:
    OP_CASE(SHL)
    OP_CASE(SHR)
    OP_CASE(USHR)
#undef OP_CASE
    case Instruction::CMP_LONG:
    case Instruction::CMPG_DOUBLE:
    case Instruction::CMPL_DOUBLE:
    case Instruction::CMPG_FLOAT:
    case Instruction::CMPL_FLOAT:
    case Instruction::NEG_INT:
    case Instruction::NEG_FLOAT:
    case Instruction::NOT_INT:
    case Instruction::LONG_TO_INT:
    case Instruction::INT_TO_BYTE:
    case Instruction::INT_TO_SHORT:
    case Instruction::INT_TO_CHAR:
    case Instruction::INT_TO_FLOAT:
    case Instruction::LONG_TO_FLOAT:
    case Instruction::FLOAT_TO_INT:
    case Instruction::DOUBLE_TO_INT:
    case Instruction::DOUBLE_TO_FLOAT:
    case Instruction::REM_FLOAT:
    case Instruction::REM_FLOAT_2ADDR:
    case Instruction::ADD_FLOAT_2ADDR:
    case Instruction::ADD_FLOAT:
    case Instruction::SUB_FLOAT_2ADDR:
    case Instruction::SUB_FLOAT:
    case Instruction::MUL_FLOAT_2ADDR:
    case Instruction::MUL_FLOAT:
    case Instruction::DIV_FLOAT_2ADDR:
    case Instruction::DIV_FLOAT:
    case Instruction::ADD_INT_LIT16:
    case Instruction::AND_INT_LIT16:
    case Instruction::OR_INT_LIT16:
    case Instruction::XOR_INT_LIT16:
    case Instruction::MUL_INT_LIT16:
    case Instruction::DIV_INT_LIT16:
    case Instruction::RSUB_INT:
    case Instruction::REM_INT_LIT16:
    case Instruction::ADD_INT_LIT8:
    case Instruction::AND_INT_LIT8:
    case Instruction::OR_INT_LIT8:
    case Instruction::XOR_INT_LIT8:
    case Instruction::RSUB_INT_LIT8:
    case Instruction::MUL_INT_LIT8:
    case Instruction::DIV_INT_LIT8:
    case Instruction::REM_INT_LIT8:
    case Instruction::SHL_INT_LIT8:
    case Instruction::SHR_INT_LIT8:
    case Instruction::USHR_INT_LIT8:
    case Instruction::MOVE_RESULT:
    case Instruction::IGET:
    case Instruction::IGET_BOOLEAN:
    case Instruction::IGET_BYTE:
    case Instruction::IGET_CHAR:
    case Instruction::IGET_SHORT:
    case Instruction::SGET:
    case Instruction::SGET_BOOLEAN:
    case Instruction::SGET_BYTE:
    case Instruction::SGET_CHAR:
    case Instruction::SGET_SHORT:
    case Instruction::ARRAY_LENGTH:
    case Instruction::AGET:
    case Instruction::AGET_BOOLEAN:
    case Instruction::AGET_BYTE:
    case Instruction::AGET_CHAR:
    case Instruction::AGET_SHORT:
    case Instruction::MOVE:
    case Instruction::MOVE_16:
    case Instruction::MOVE_FROM16:
    case Instruction::INSTANCE_OF: {
      int32_t vreg_a = instruction.VRegA();
      UpdateLocal(vreg_a, /* is_object= */ false, /* is_wide= */ false);
      return true;
    }

#define OP_CASE(opcode) \
    case Instruction::opcode ##_LONG_2ADDR: \
    case Instruction::opcode ##_LONG:
    OP_CASE(ADD)
    OP_CASE(SUB)
    OP_CASE(MUL)
    OP_CASE(AND)
    OP_CASE(OR)
    OP_CASE(XOR)
    OP_CASE(DIV)
    OP_CASE(REM)
#undef OP_CASE
    case Instruction::NEG_DOUBLE:
    case Instruction::NEG_LONG:
    case Instruction::NOT_LONG:
    case Instruction::INT_TO_LONG:
    case Instruction::INT_TO_DOUBLE:
    case Instruction::LONG_TO_DOUBLE:
    case Instruction::FLOAT_TO_LONG:
    case Instruction::FLOAT_TO_DOUBLE:
    case Instruction::DOUBLE_TO_LONG:
    case Instruction::REM_DOUBLE:
    case Instruction::REM_DOUBLE_2ADDR:
    case Instruction::ADD_DOUBLE_2ADDR:
    case Instruction::ADD_DOUBLE:
    case Instruction::SUB_DOUBLE_2ADDR:
    case Instruction::SUB_DOUBLE:
    case Instruction::MUL_DOUBLE_2ADDR:
    case Instruction::MUL_DOUBLE:
    case Instruction::DIV_DOUBLE_2ADDR:
    case Instruction::DIV_DOUBLE:
    case Instruction::MOVE_RESULT_WIDE:
    case Instruction::SGET_WIDE:
    case Instruction::IGET_WIDE:
    case Instruction::AGET_WIDE:
    case Instruction::CONST_WIDE_16:
    case Instruction::CONST_WIDE_32:
    case Instruction::CONST_WIDE:
    case Instruction::CONST_WIDE_HIGH16:
    case Instruction::MOVE_WIDE:
    case Instruction::MOVE_WIDE_FROM16:
    case Instruction::MOVE_WIDE_16: {
      int32_t vreg_a = instruction.VRegA();
      UpdateLocal(vreg_a, /* is_object= */ false, /* is_wide= */ true);
      return true;
    }

    case Instruction::NEW_ARRAY:
    case Instruction::NEW_INSTANCE:
    case Instruction::FILLED_NEW_ARRAY:
    case Instruction::FILLED_NEW_ARRAY_RANGE:
    case Instruction::MOVE_RESULT_OBJECT:
    case Instruction::IGET_OBJECT:
    case Instruction::SGET_OBJECT:
    case Instruction::AGET_OBJECT:
    case Instruction::CONST_STRING:
    case Instruction::CONST_STRING_JUMBO:
    case Instruction::CONST_CLASS:
    case Instruction::CONST_METHOD_HANDLE:
    case Instruction::CONST_METHOD_TYPE:
    case Instruction::MOVE_EXCEPTION:
    case Instruction::MOVE_OBJECT:
    case Instruction::MOVE_OBJECT_FROM16:
    case Instruction::MOVE_OBJECT_16: {
      UpdateLocal(instruction.VRegA(), /* is_object= */ true, /* is_wide= */ false);
      return true;
    }

    case Instruction::UNUSED_3E ... Instruction::UNUSED_43:
    case Instruction::UNUSED_73:
    case Instruction::UNUSED_79:
    case Instruction::UNUSED_7A:
    case Instruction::UNUSED_E3 ... Instruction::UNUSED_F9: {
      break;
    }
  }
  unimplemented_reason_ = instruction.Name();
  return false;
}

bool FastCompilerARM64::Compile(bool with_loop_support) {
  if (!InitializeParameters()) {
    DCHECK(HitUnimplemented());
    AbortCompilation();
    return false;
  }
  if (with_loop_support) {
    if (!EnsureHasFrame()) {
      AbortCompilation();
      return false;
    }
    AnalyzeBranches();
  }

  if (!ProcessInstructions()) {
    DCHECK(HitUnimplemented());
    AbortCompilation();
    return false;
  }
  DCHECK(!HitUnimplemented()) << GetUnimplementedReason();

  StackMapStream* stack_map_stream = code_generation_data_->GetStackMapStream();
  if (!has_frame_) {
    stack_map_stream->BeginMethod(/* frame_size= */ 0u,
                                  /* core_spill_mask= */ 0u,
                                  /* fp_spill_mask= */ 0u,
                                  GetCodeItemAccessor().RegistersSize(),
                                  /* is_compiling_baseline= */ false,
                                  /* is_debuggable= */ false,
                                  /* has_should_deoptimize_flag= */ false,
                                  /* is_fast= */ true);
  }

  // Catch stack maps are pushed at the end.
  for (auto pair : catch_stack_maps_) {
    uint32_t dex_pc = pair.first;
    uint32_t native_pc = pair.second;
    std::vector<uint32_t> dex_pc_list_for_verification;
    if (kIsDebugBuild) {
      dex_pc_list_for_verification.push_back(dex_pc);
    }
    stack_map_stream->BeginStackMapEntry(dex_pc,
                                         native_pc,
                                         /* register_mask= */ 0,
                                         /* sp_mask= */ nullptr,
                                         StackMap::Kind::Catch,
                                         /* needs_vreg_info= */ false,
                                         dex_pc_list_for_verification);
    stack_map_stream->EndStackMapEntry();
  }
  stack_map_stream->EndMethod(assembler_.CodeSize());
  assembler_.FinalizeCode();

  if (VLOG_IS_ON(jit)) {
    // Dump the generated code
    {
      ScopedObjectAccess soa(Thread::Current());
      VLOG(jit) << "Dumping generated fast baseline code for " << method_->PrettyMethod();
    }
    FILE* file = tmpfile();
    if (file != nullptr) {
      MacroAssembler* masm = GetVIXLAssembler();
      PrintDisassembler print_disasm(file);
      vixl::aarch64::Instruction* dis_start =
          masm->GetBuffer()->GetStartAddress<vixl::aarch64::Instruction*>();
      vixl::aarch64::Instruction* dis_end =
          masm->GetBuffer()->GetEndAddress<vixl::aarch64::Instruction*>();
      print_disasm.DisassembleBuffer(dis_start, dis_end);
      fseek(file, 0L, SEEK_SET);
      char buffer[1024];
      const char* line;
      while ((line = fgets(buffer, sizeof(buffer), file)) != nullptr) {
        VLOG(jit) << std::string(line);
      }
      fclose(file);
    }
  }
  return true;
}

}  // namespace arm64

std::unique_ptr<arm64::FastCompilerARM64> TryCompile(
    ArtMethod* method,
    ArenaAllocator* allocator,
    ArenaStack* arena_stack,
    VariableSizedHandleScope* handles,
    const CompilerOptions& compiler_options,
    const DexCompilationUnit& dex_compilation_unit,
    bool with_loop_support) {
  std::unique_ptr<arm64::FastCompilerARM64> compiler(new arm64::FastCompilerARM64(
      method,
      allocator,
      arena_stack,
      handles,
      compiler_options,
      dex_compilation_unit));
  if (compiler->Compile(with_loop_support)) {
    return compiler;
  }

  if (!with_loop_support && compiler->ShouldRecompileWithLoopSupport()) {
    compiler.reset();
    VLOG(jit) << "Recompiling with loop support";
    return TryCompile(method,
                      allocator,
                      arena_stack,
                      handles,
                      compiler_options,
                      dex_compilation_unit,
                      /* with_loop_support= */ true);
  }

  VLOG(jit) << "Did not fast compile because of " << compiler->GetUnimplementedReason();
  return nullptr;
}

std::unique_ptr<FastCompiler> FastCompiler::CompileARM64(
    ArtMethod* method,
    ArenaAllocator* allocator,
    ArenaStack* arena_stack,
    VariableSizedHandleScope* handles,
    const CompilerOptions& compiler_options,
    const DexCompilationUnit& dex_compilation_unit) {
  if (!compiler_options.GetImplicitNullChecks() ||
      !compiler_options.GetImplicitStackOverflowChecks() ||
      compiler_options.IsJitCompilerForSharedCode() ||
      kUseTableLookupReadBarrier ||
      !kReserveMarkingRegister ||
      kPoisonHeapReferences) {
    // Configurations we don't support.
    return nullptr;
  }
  return TryCompile(method,
                    allocator,
                    arena_stack,
                    handles,
                    compiler_options,
                    dex_compilation_unit,
                    /* with_loop_support= */ false);
}

}  // namespace art

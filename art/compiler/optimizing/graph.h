/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef ART_COMPILER_OPTIMIZING_GRAPH_H_
#define ART_COMPILER_OPTIMIZING_GRAPH_H_

#include <algorithm>
#include <iosfwd>
#include <functional>
#include <optional>
#include <string>

#include "base/arena_allocator.h"
#include "base/arena_containers.h"
#include "base/arena_object.h"
#include "base/macros.h"
#include "base/stl_util.h"
#include "compilation_kind.h"
#include "data_type.h"
#include "dex/invoke_type.h"
#include "handle_cache.h"
#include "reference_type_info.h"

namespace art HIDDEN {

class BlockNamer;
class CodeGenerator;
class HBasicBlock;
class HConstant;
class HCurrentMethod;
class HDoubleConstant;
class HFloatConstant;
class HInstruction;
class HLongConstant;
class HIntConstant;
class HInvoke;
class HNullConstant;
class VariableSizedHandleScope;

enum GraphAnalysisResult {
  kAnalysisSkipped,
  kAnalysisInvalidBytecode,
  kAnalysisFailThrowCatchLoop,
  kAnalysisFailAmbiguousArrayOp,
  kAnalysisFailIrreducibleLoopAndStringInit,
  kAnalysisFailPhiEquivalentInOsr,
  kAnalysisSuccess,
};

std::ostream& operator<<(std::ostream& os, GraphAnalysisResult ga);

// Control-flow graph of a method. Contains a list of basic blocks.
class HGraph : public ArenaObject<kArenaAllocGraph> {
 public:
  HGraph(ArenaAllocator* allocator,
         ArenaStack* arena_stack,
         VariableSizedHandleScope* handles,
         const DexFile& dex_file,
         uint32_t method_idx,
         InstructionSet instruction_set,
         InvokeType invoke_type,
         bool dead_reference_safe = false,
         bool debuggable = false,
         CompilationKind compilation_kind = CompilationKind::kOptimized,
         int start_instruction_id = 0,
         bool dynamic_instrumentation = false)
      : allocator_(allocator),
        arena_stack_(arena_stack),
        handle_cache_(handles),
        blocks_(allocator->Adapter(kArenaAllocBlockList)),
        reverse_post_order_(allocator->Adapter(kArenaAllocReversePostOrder)),
        linear_order_(allocator->Adapter(kArenaAllocLinearOrder)),
        entry_block_(nullptr),
        exit_block_(nullptr),
        number_of_vregs_(0),
        number_of_in_vregs_(0),
        temporaries_vreg_slots_(0),
        has_bounds_checks_(false),
        has_try_catch_(false),
        has_monitor_operations_(false),
        has_traditional_simd_(false),
        has_predicated_simd_(false),
        has_loops_(false),
        has_irreducible_loops_(false),
        has_direct_critical_native_call_(false),
        has_always_throwing_invokes_(false),
        dead_reference_safe_(dead_reference_safe),
        debuggable_(debuggable),
        dynamic_instrumentation_(dynamic_instrumentation),
        current_instruction_id_(start_instruction_id),
        dex_file_(dex_file),
        method_idx_(method_idx),
        invoke_type_(invoke_type),
        in_ssa_form_(false),
        number_of_cha_guards_(0),
        instruction_set_(instruction_set),
        cached_null_constant_(nullptr),
        cached_int_constants_(std::less<int32_t>(), allocator->Adapter(kArenaAllocConstantsMap)),
        cached_float_constants_(std::less<int32_t>(), allocator->Adapter(kArenaAllocConstantsMap)),
        cached_long_constants_(std::less<int64_t>(), allocator->Adapter(kArenaAllocConstantsMap)),
        cached_double_constants_(std::less<int64_t>(), allocator->Adapter(kArenaAllocConstantsMap)),
        cached_current_method_(nullptr),
        art_method_(nullptr),
        compilation_kind_(compilation_kind),
        useful_optimizing_(false),
        cha_single_implementation_list_(allocator->Adapter(kArenaAllocCHA)) {
    blocks_.reserve(kDefaultNumberOfBlocks);
  }

  std::ostream& Dump(std::ostream& os,
                     CodeGenerator* codegen,
                     std::optional<std::reference_wrapper<const BlockNamer>> namer = std::nullopt);

  ArenaAllocator* GetAllocator() const { return allocator_; }
  ArenaStack* GetArenaStack() const { return arena_stack_; }

  HandleCache* GetHandleCache() { return &handle_cache_; }

  const ArenaVector<HBasicBlock*>& GetBlocks() const { return blocks_; }

  // An iterator to only blocks that are still actually in the graph (when
  // blocks are removed they are replaced with 'nullptr' in GetBlocks to
  // simplify block-id assignment and avoid memmoves in the block-list).
  IterationRange<FilterNull<ArenaVector<HBasicBlock*>::const_iterator>> GetActiveBlocks() const {
    return FilterOutNull(MakeIterationRange(GetBlocks()));
  }

  bool IsInSsaForm() const { return in_ssa_form_; }
  void SetInSsaForm() { in_ssa_form_ = true; }

  HBasicBlock* GetEntryBlock() const { return entry_block_; }
  HBasicBlock* GetExitBlock() const { return exit_block_; }
  bool HasExitBlock() const { return exit_block_ != nullptr; }

  void SetEntryBlock(HBasicBlock* block) { entry_block_ = block; }
  void SetExitBlock(HBasicBlock* block) { exit_block_ = block; }

  bool IsEntryBlock(HBasicBlock* block) { return GetEntryBlock() == block; }
  bool IsExitBlock(HBasicBlock* block) { return GetExitBlock() == block; }

  void AddBlock(HBasicBlock* block);

  void ComputeDominanceInformation();
  void ClearDominanceInformation();
  void ClearLoopInformation();
  void FindBackEdges(/*out*/ BitVectorView<size_t> visited);
  GraphAnalysisResult BuildDominatorTree();
  GraphAnalysisResult RecomputeDominatorTree();
  void SimplifyCFG();
  void SimplifyCatchBlocks();

  // Analyze all natural loops in this graph. Returns a code specifying that it
  // was successful or the reason for failure. The method will fail if a loop
  // is a throw-catch loop, i.e. the header is a catch block.
  GraphAnalysisResult AnalyzeLoops() const;

  // Iterate over blocks to compute try block membership. Needs reverse post
  // order and loop information.
  void ComputeTryBlockInformation();

  // Inline this graph in `outer_graph`, replacing the given `invoke` instruction.
  // Returns the instruction to replace the invoke expression or null if the
  // invoke is for a void method. Note that the caller is responsible for replacing
  // and removing the invoke instruction.
  HInstruction* InlineInto(HGraph* outer_graph, HInvoke* invoke);

  // Update the loop and try membership of `block`, which was spawned from `reference`.
  // In case `reference` is a back edge, `replace_if_back_edge` notifies whether `block`
  // should be the new back edge.
  // `has_more_specific_try_catch_info` will be set to true when inlining a try catch.
  void UpdateLoopAndTryInformationOfNewBlock(HBasicBlock* block,
                                             HBasicBlock* reference,
                                             bool replace_if_back_edge,
                                             bool has_more_specific_try_catch_info = false);

  // Need to add a couple of blocks to test if the loop body is entered and
  // put deoptimization instructions, etc.
  void TransformLoopHeaderForBCE(HBasicBlock* header);

  // Adds a new loop directly after the loop with the given header and exit.
  // Returns the new preheader.
  HBasicBlock* TransformLoopForVectorization(HBasicBlock* header,
                                             HBasicBlock* body,
                                             HBasicBlock* exit);

  // Removes `block` from the graph. Assumes `block` has been disconnected from
  // other blocks and has no instructions or phis.
  void DeleteDeadEmptyBlock(HBasicBlock* block);

  // Splits the edge between `block` and `successor` while preserving the
  // indices in the predecessor/successor lists. If there are multiple edges
  // between the blocks, the lowest indices are used.
  // Returns the new block which is empty and has the same dex pc as `successor`.
  HBasicBlock* SplitEdge(HBasicBlock* block, HBasicBlock* successor);

  void SplitCriticalEdge(HBasicBlock* block, HBasicBlock* successor);

  void OrderLoopHeaderPredecessors(HBasicBlock* header);

  // Transform a loop into a format with a single preheader.
  //
  // Each phi in the header should be split: original one in the header should only hold
  // inputs reachable from the back edges and a single input from the preheader. The newly created
  // phi in the preheader should collate the inputs from the original multiple incoming blocks.
  //
  // Loops in the graph typically have a single preheader, so this method is used to "repair" loops
  // that no longer have this property.
  void TransformLoopToSinglePreheaderFormat(HBasicBlock* header);

  void SimplifyLoop(HBasicBlock* header);

  ALWAYS_INLINE int32_t AllocateInstructionId();

  int32_t GetCurrentInstructionId() const {
    return current_instruction_id_;
  }

  void SetCurrentInstructionId(int32_t id) {
    CHECK_GE(id, current_instruction_id_);
    current_instruction_id_ = id;
  }

  void UpdateTemporariesVRegSlots(size_t slots) {
    temporaries_vreg_slots_ = std::max(slots, temporaries_vreg_slots_);
  }

  size_t GetTemporariesVRegSlots() const {
    DCHECK(!in_ssa_form_);
    return temporaries_vreg_slots_;
  }

  void SetNumberOfVRegs(uint16_t number_of_vregs) {
    number_of_vregs_ = number_of_vregs;
  }

  uint16_t GetNumberOfVRegs() const {
    return number_of_vregs_;
  }

  void SetNumberOfInVRegs(uint16_t value) {
    number_of_in_vregs_ = value;
  }

  uint16_t GetNumberOfInVRegs() const {
    return number_of_in_vregs_;
  }

  uint16_t GetNumberOfLocalVRegs() const {
    DCHECK(!in_ssa_form_);
    return number_of_vregs_ - number_of_in_vregs_;
  }

  const ArenaVector<HBasicBlock*>& GetReversePostOrder() const {
    return reverse_post_order_;
  }

  ArrayRef<HBasicBlock* const> GetReversePostOrderSkipEntryBlock() const {
    DCHECK(GetReversePostOrder()[0] == entry_block_);
    return ArrayRef<HBasicBlock* const>(GetReversePostOrder()).SubArray(1);
  }

  IterationRange<ArenaVector<HBasicBlock*>::const_reverse_iterator> GetPostOrder() const {
    return ReverseRange(GetReversePostOrder());
  }

  const ArenaVector<HBasicBlock*>& GetLinearOrder() const {
    return linear_order_;
  }

  IterationRange<ArenaVector<HBasicBlock*>::const_reverse_iterator> GetLinearPostOrder() const {
    return ReverseRange(GetLinearOrder());
  }

  bool HasBoundsChecks() const {
    return has_bounds_checks_;
  }

  void SetHasBoundsChecks(bool value) {
    has_bounds_checks_ = value;
  }

  // Is the code known to be robust against eliminating dead references
  // and the effects of early finalization?
  bool IsDeadReferenceSafe() const { return dead_reference_safe_; }

  void MarkDeadReferenceUnsafe() { dead_reference_safe_ = false; }

  bool IsDebuggable() const { return debuggable_; }

  bool IsDynamicallyInstrumented() const { return dynamic_instrumentation_; }

  // Returns a constant of the given type and value. If it does not exist
  // already, it is created and inserted into the graph. This method is only for
  // integral types.
  HConstant* GetConstant(DataType::Type type, int64_t value);

  // TODO: This is problematic for the consistency of reference type propagation
  // because it can be created anytime after the pass and thus it will be left
  // with an invalid type.
  HNullConstant* GetNullConstant();

  HIntConstant* GetIntConstant(int32_t value);
  HLongConstant* GetLongConstant(int64_t value);
  HFloatConstant* GetFloatConstant(float value);
  HDoubleConstant* GetDoubleConstant(double value);

  HCurrentMethod* GetCurrentMethod();

  const DexFile& GetDexFile() const {
    return dex_file_;
  }

  uint32_t GetMethodIdx() const {
    return method_idx_;
  }

  // Get the method name (without the signature), e.g. "<init>"
  const char* GetMethodName() const;

  // Get the pretty method name (class + name + optionally signature).
  std::string PrettyMethod(bool with_signature = true) const;

  InvokeType GetInvokeType() const {
    return invoke_type_;
  }

  InstructionSet GetInstructionSet() const {
    return instruction_set_;
  }

  bool IsCompilingOsr() const { return compilation_kind_ == CompilationKind::kOsr; }

  bool IsCompilingBaseline() const { return compilation_kind_ == CompilationKind::kBaseline; }

  CompilationKind GetCompilationKind() const { return compilation_kind_; }

  ArenaSet<ArtMethod*>& GetCHASingleImplementationList() {
    return cha_single_implementation_list_;
  }

  // In case of OSR we intend to use SuspendChecks as an entry point to the
  // function; for debuggable graphs we might deoptimize to interpreter from
  // SuspendChecks. In these cases we should always generate code for them.
  bool SuspendChecksAreAllowedToNoOp() const {
    return !IsDebuggable() && !IsCompilingOsr();
  }

  void AddCHASingleImplementationDependency(ArtMethod* method) {
    cha_single_implementation_list_.insert(method);
  }

  bool HasShouldDeoptimizeFlag() const {
    return number_of_cha_guards_ != 0 || debuggable_;
  }

  bool HasTryCatch() const { return has_try_catch_; }
  void SetHasTryCatch(bool value) { has_try_catch_ = value; }

  bool HasMonitorOperations() const { return has_monitor_operations_; }
  void SetHasMonitorOperations(bool value) { has_monitor_operations_ = value; }

  bool HasTraditionalSIMD() { return has_traditional_simd_; }
  void SetHasTraditionalSIMD(bool value) { has_traditional_simd_ = value; }

  bool HasPredicatedSIMD() { return has_predicated_simd_; }
  void SetHasPredicatedSIMD(bool value) { has_predicated_simd_ = value; }

  bool HasSIMD() const { return has_traditional_simd_ || has_predicated_simd_; }

  bool HasLoops() const { return has_loops_; }
  void SetHasLoops(bool value) { has_loops_ = value; }

  bool HasIrreducibleLoops() const { return has_irreducible_loops_; }
  void SetHasIrreducibleLoops(bool value) { has_irreducible_loops_ = value; }

  bool HasDirectCriticalNativeCall() const { return has_direct_critical_native_call_; }
  void SetHasDirectCriticalNativeCall(bool value) { has_direct_critical_native_call_ = value; }

  bool HasAlwaysThrowingInvokes() const { return has_always_throwing_invokes_; }
  void SetHasAlwaysThrowingInvokes(bool value) { has_always_throwing_invokes_ = value; }

  ArtMethod* GetArtMethod() const { return art_method_; }
  void SetArtMethod(ArtMethod* method) { art_method_ = method; }

  void SetProfilingInfo(ProfilingInfo* info) { profiling_info_ = info; }
  ProfilingInfo* GetProfilingInfo() const { return profiling_info_; }

  ReferenceTypeInfo GetInexactObjectRti() {
    return ReferenceTypeInfo::Create(handle_cache_.GetObjectClassHandle(), /* is_exact= */ false);
  }

  uint32_t GetNumberOfCHAGuards() const { return number_of_cha_guards_; }
  void SetNumberOfCHAGuards(uint32_t num) { number_of_cha_guards_ = num; }
  void IncrementNumberOfCHAGuards() { number_of_cha_guards_++; }

  void SetUsefulOptimizing() { useful_optimizing_ = true; }
  bool IsUsefulOptimizing() const { return useful_optimizing_; }

  size_t CountNumberOfInstructions();
  bool HasMoreInstructionsThan(size_t limit);

 private:
  static const size_t kDefaultNumberOfBlocks = 8u;

  void RemoveDeadBlocksInstructionsAsUsersAndDisconnect(BitVectorView<const size_t> visited) const;
  void RemoveDeadBlocks(BitVectorView<const size_t> visited);

  template <class InstructionType, typename ValueType>
  InstructionType* CreateConstant(ValueType value,
                                  ArenaSafeMap<ValueType, InstructionType*>* cache);

  void InsertConstant(HConstant* instruction);

  // Cache a float constant into the graph. This method should only be
  // called by the SsaBuilder when creating "equivalent" instructions.
  void CacheFloatConstant(HFloatConstant* constant);

  // See CacheFloatConstant comment.
  void CacheDoubleConstant(HDoubleConstant* constant);

  ArenaAllocator* const allocator_;
  ArenaStack* const arena_stack_;

  HandleCache handle_cache_;

  // List of blocks in insertion order.
  ArenaVector<HBasicBlock*> blocks_;

  // List of blocks to perform a reverse post order tree traversal.
  ArenaVector<HBasicBlock*> reverse_post_order_;

  // List of blocks to perform a linear order tree traversal. Unlike the reverse
  // post order, this order is not incrementally kept up-to-date.
  ArenaVector<HBasicBlock*> linear_order_;

  HBasicBlock* entry_block_;
  HBasicBlock* exit_block_;

  // The number of virtual registers in this method. Contains the parameters.
  uint16_t number_of_vregs_;

  // The number of virtual registers used by parameters of this method.
  uint16_t number_of_in_vregs_;

  // Number of vreg size slots that the temporaries use (used in baseline compiler).
  size_t temporaries_vreg_slots_;

  // Flag whether there are bounds checks in the graph. We can skip
  // BCE if it's false.
  bool has_bounds_checks_;

  // Flag whether there are try/catch blocks in the graph. We will skip
  // try/catch-related passes if it's false.
  bool has_try_catch_;

  // Flag whether there are any HMonitorOperation in the graph. If yes this will mandate
  // DexRegisterMap to be present to allow deadlock analysis for non-debuggable code.
  bool has_monitor_operations_;

  // Flags whether SIMD (traditional or predicated) instructions appear in the graph.
  // If either is true, the code generators may have to be more careful spilling the wider
  // contents of SIMD registers.
  bool has_traditional_simd_;
  bool has_predicated_simd_;

  // Flag whether there are any loops in the graph. We can skip loop
  // optimization if it's false.
  bool has_loops_;

  // Flag whether there are any irreducible loops in the graph.
  bool has_irreducible_loops_;

  // Flag whether there are any direct calls to native code registered
  // for @CriticalNative methods.
  bool has_direct_critical_native_call_;

  // Flag whether the graph contains invokes that always throw.
  bool has_always_throwing_invokes_;

  // Is the code known to be robust against eliminating dead references
  // and the effects of early finalization? If false, dead reference variables
  // are kept if they might be visible to the garbage collector.
  // Currently this means that the class was declared to be dead-reference-safe,
  // the method accesses no reachability-sensitive fields or data, and the same
  // is true for any methods that were inlined into the current one.
  bool dead_reference_safe_;

  // Indicates whether the graph should be compiled in a way that
  // ensures full debuggability. If false, we can apply more
  // aggressive optimizations that may limit the level of debugging.
  const bool debuggable_;

  // Whether the graph should be compiled with extra hooks to attach dynamic
  // instrumentation.
  const bool dynamic_instrumentation_;

  // The current id to assign to a newly added instruction. See HInstruction.id_.
  int32_t current_instruction_id_;

  // The dex file from which the method is from.
  const DexFile& dex_file_;

  // The method index in the dex file.
  const uint32_t method_idx_;

  // If inlined, this encodes how the callee is being invoked.
  const InvokeType invoke_type_;

  // Whether the graph has been transformed to SSA form. Only used
  // in debug mode to ensure we are not using properties only valid
  // for non-SSA form (like the number of temporaries).
  bool in_ssa_form_;

  // Number of CHA guards in the graph. Used to short-circuit the
  // CHA guard optimization pass when there is no CHA guard left.
  uint32_t number_of_cha_guards_;

  const InstructionSet instruction_set_;

  // Cached constants.
  HNullConstant* cached_null_constant_;
  ArenaSafeMap<int32_t, HIntConstant*> cached_int_constants_;
  ArenaSafeMap<int32_t, HFloatConstant*> cached_float_constants_;
  ArenaSafeMap<int64_t, HLongConstant*> cached_long_constants_;
  ArenaSafeMap<int64_t, HDoubleConstant*> cached_double_constants_;

  HCurrentMethod* cached_current_method_;

  // The ArtMethod this graph is for. Note that for AOT, it may be null,
  // for example for methods whose declaring class could not be resolved
  // (such as when the superclass could not be found).
  ArtMethod* art_method_;

  // The `ProfilingInfo` associated with the method being compiled.
  ProfilingInfo* profiling_info_;

  // How we are compiling the graph: either optimized, osr, or baseline.
  // For osr, we will make all loops seen as irreducible and emit special
  // stack maps to mark compiled code entries which the interpreter can
  // directly jump to.
  const CompilationKind compilation_kind_;

  // Whether after compiling baseline it is still useful re-optimizing this
  // method.
  bool useful_optimizing_;

  // List of methods that are assumed to have single implementation.
  ArenaSet<ArtMethod*> cha_single_implementation_list_;

  friend class SsaBuilder;           // For caching constants.
  friend class SsaLivenessAnalysis;  // For the linear order.
  friend class HInliner;             // For the reverse post order.
  ART_FRIEND_TEST(GraphTest, IfSuccessorSimpleJoinBlock1);
  DISALLOW_COPY_AND_ASSIGN(HGraph);
};

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_GRAPH_H_


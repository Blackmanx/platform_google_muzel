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

#include "register_allocator.h"

#include "arch/x86/instruction_set_features_x86.h"
#include "base/arena_allocator.h"
#include "base/macros.h"
#include "builder.h"
#include "code_generator.h"
#include "com_android_art_flags.h"
#include "dex/dex_file.h"
#include "dex/dex_file_types.h"
#include "dex/dex_instruction.h"
#include "driver/compiler_options.h"
#include "nodes.h"
#include "optimizing_unit_test.h"
#include "register_allocator_linear_scan.h"
#include "ssa_liveness_analysis.h"
#include "ssa_phi_elimination.h"

namespace art HIDDEN {

// Note: the register allocator tests rely on the fact that constants have live
// intervals and registers get allocated to them.

class RegisterAllocatorTest : public CommonCompilerTest, public OptimizingUnitTestHelper {
 protected:
  // Define a shortcut for the `kLivenessPositionsPerInstruction`.
  static constexpr size_t kLppi = kLivenessPositionsPerInstruction;

  std::unique_ptr<CodeGenerator> CreateCodeGenerator(
      HGraph* graph, InstructionSet instruction_set = InstructionSet::kNone) {
    if (instruction_set == InstructionSet::kNone) {
      instruction_set = kRuntimeISA;
    }
    if (instruction_set == InstructionSet::kArm) {
      instruction_set = InstructionSet::kThumb2;
    }
    compiler_options_ = CommonCompilerTest::CreateCompilerOptions(instruction_set, "default");
    CHECK(compiler_options_ != nullptr);
    CHECK_EQ(instruction_set, compiler_options_->GetInstructionSet());
    std::unique_ptr<CodeGenerator> codegen = CodeGenerator::Create(graph, *compiler_options_);
    CHECK_IMPLIES(codegen != nullptr, codegen->GetInstructionSet() == instruction_set);
    return codegen;
  }

  // Helper functions that make use of the OptimizingUnitTest's members.
  bool Check(const std::vector<uint16_t>& data);
  void BuildIfElseWithPhi(HPhi** phi, HInstruction** input1, HInstruction** input2);
  void BuildFieldReturn(HInstruction** field, HInstruction** ret);
  void BuildTwoSubs(HInstruction** first_sub, HInstruction** second_sub);
  void BuildDiv(HInstruction** div);

  bool ValidateIntervals(const ScopedArenaVector<LiveInterval*>& intervals,
                         const CodeGenerator& codegen) {
    return RegisterAllocator::ValidateIntervals(ArrayRef<LiveInterval* const>(intervals),
                                                /* number_of_spill_slots= */ 0u,
                                                /* number_of_out_slots= */ 0u,
                                                codegen,
                                                /*liveness=*/ nullptr,
                                                PhysicalRegisterType::kCoreRegister,
                                                /* log_fatal_on_failure= */ false);
  }

  void OverrideOutput(LocationSummary* locations,
                      Location out,
                      Location::OutputOverlap output_overlaps = Location::kOutputOverlap) {
    locations->output_ = out;
    locations->output_overlaps_ = output_overlaps;
  }

  template <typename RegType>
  void BlockCoreRegistersExcept(CodeGenerator* codegen, std::initializer_list<RegType> allowed) {
    size_t number_of_core_registers = codegen->GetNumberOfCoreRegisters();
    uint32_t blocked_core_registers = MaxInt<uint32_t>(number_of_core_registers);
    for (RegType reg : allowed) {
      CHECK_LT(reg, number_of_core_registers);
      blocked_core_registers &= ~(1u << reg);
    }
    RegisterSet blocked_registers = RegisterSet::Empty();
    blocked_registers.AddCoreRegisterSet(blocked_core_registers);
    blocked_registers.AddFpuRegisterSet(codegen->blocked_registers_.GetFpuRegisterSet());
    blocked_registers.AddVecRegisterSet(codegen->blocked_registers_.GetVecRegisterSet());
    codegen->blocked_registers_ = blocked_registers;
  }

  void TestFreeUntil(bool special_first);
  void TestSpillInactive();
  void TestNoOutputOverlap();
  void TestNoOutputOverlapAndTemp();

  std::unique_ptr<CompilerOptions> compiler_options_;
};

bool RegisterAllocatorTest::Check(const std::vector<uint16_t>& data) {
  HGraph* graph = CreateCFG(data);
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph);
  CHECK(codegen != nullptr);
  SsaLivenessAnalysis liveness(graph, codegen.get(), GetScopedAllocator());
  liveness.Analyze();
  std::unique_ptr<RegisterAllocator> register_allocator =
      RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
  register_allocator->AllocateRegisters();
  return register_allocator->Validate(false);
}

/**
 * Unit testing of RegisterAllocator::ValidateIntervals. Register allocator
 * tests are based on this validation method.
 */
TEST_F(RegisterAllocatorTest, ValidateIntervals) {
  CreateGraph();
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
  if (codegen == nullptr) {
    GTEST_SKIP() << "X86 codegen is unavailable.";
  }
  ScopedArenaVector<LiveInterval*> intervals(GetScopedAllocator()->Adapter());

  // Test with two intervals of the same range.
  {
    static constexpr size_t ranges[][2] = {{0, 42}};
    intervals.push_back(BuildInterval(ranges, arraysize(ranges), GetScopedAllocator(), 1u << 0));
    intervals.push_back(BuildInterval(ranges, arraysize(ranges), GetScopedAllocator(), 1u << 1));
    ASSERT_TRUE(ValidateIntervals(intervals, *codegen));

    intervals[1]->SetRegisters(1u << 0);
    ASSERT_FALSE(ValidateIntervals(intervals, *codegen));
    intervals.clear();
  }

  // Test with two non-intersecting intervals.
  {
    static constexpr size_t ranges1[][2] = {{0, 42}};
    intervals.push_back(BuildInterval(ranges1, arraysize(ranges1), GetScopedAllocator(), 1u << 0));
    static constexpr size_t ranges2[][2] = {{42, 43}};
    intervals.push_back(BuildInterval(ranges2, arraysize(ranges2), GetScopedAllocator(), 1u << 1));
    ASSERT_TRUE(ValidateIntervals(intervals, *codegen));

    intervals[1]->SetRegisters(1u << 0);
    ASSERT_TRUE(ValidateIntervals(intervals, *codegen));
    intervals.clear();
  }

  // Test with two non-intersecting intervals, with one with a lifetime hole.
  {
    static constexpr size_t ranges1[][2] = {{0, 42}, {45, 48}};
    intervals.push_back(BuildInterval(ranges1, arraysize(ranges1), GetScopedAllocator(), 1u << 0));
    static constexpr size_t ranges2[][2] = {{42, 43}};
    intervals.push_back(BuildInterval(ranges2, arraysize(ranges2), GetScopedAllocator(), 1u << 1));
    ASSERT_TRUE(ValidateIntervals(intervals, *codegen));

    intervals[1]->SetRegisters(1u << 0);
    ASSERT_TRUE(ValidateIntervals(intervals, *codegen));
    intervals.clear();
  }

  // Test with intersecting intervals.
  {
    static constexpr size_t ranges1[][2] = {{0, 42}, {44, 48}};
    intervals.push_back(BuildInterval(ranges1, arraysize(ranges1), GetScopedAllocator(), 1u << 0));
    static constexpr size_t ranges2[][2] = {{42, 47}};
    intervals.push_back(BuildInterval(ranges2, arraysize(ranges2), GetScopedAllocator(), 1u << 1));
    ASSERT_TRUE(ValidateIntervals(intervals, *codegen));

    intervals[1]->SetRegisters(1u << 0);
    ASSERT_FALSE(ValidateIntervals(intervals, *codegen));
    intervals.clear();
  }

  // Test with siblings.
  {
    static constexpr size_t ranges1[][2] = {{0, 42}, {44, 48}};
    intervals.push_back(BuildInterval(ranges1, arraysize(ranges1), GetScopedAllocator(), 1u << 0));
    intervals[0]->SplitAt(43);
    static constexpr size_t ranges2[][2] = {{42, 47}};
    intervals.push_back(BuildInterval(ranges2, arraysize(ranges2), GetScopedAllocator(), 1u << 1));
    ASSERT_TRUE(ValidateIntervals(intervals, *codegen));

    intervals[1]->SetRegisters(1u << 0);
    // Sibling of the first interval has no register allocated to it.
    ASSERT_TRUE(ValidateIntervals(intervals, *codegen));

    intervals[0]->GetNextSibling()->SetRegisters(1u << 0);
    ASSERT_FALSE(ValidateIntervals(intervals, *codegen));
  }
}

TEST_F(RegisterAllocatorTest, CFG1) {
  /*
   * Test the following snippet:
   *  return 0;
   *
   * Which becomes the following graph:
   *       constant0
   *       goto
   *        |
   *       return
   *        |
   *       exit
   */
  const std::vector<uint16_t> data = ONE_REGISTER_CODE_ITEM(
    Instruction::CONST_4 | 0 | 0,
    Instruction::RETURN);

  ASSERT_TRUE(Check(data));
}

TEST_F(RegisterAllocatorTest, Loop1) {
  /*
   * Test the following snippet:
   *  int a = 0;
   *  while (a == a) {
   *    a = 4;
   *  }
   *  return 5;
   *
   * Which becomes the following graph:
   *       constant0
   *       constant4
   *       constant5
   *       goto
   *        |
   *       goto
   *        |
   *       phi
   *       equal
   *       if +++++
   *        |       \ +
   *        |     goto
   *        |
   *       return
   *        |
   *       exit
   */

  const std::vector<uint16_t> data = TWO_REGISTERS_CODE_ITEM(
    Instruction::CONST_4 | 0 | 0,
    Instruction::IF_EQ, 4,
    Instruction::CONST_4 | 4 << 12 | 0,
    Instruction::GOTO | 0xFD00,
    Instruction::CONST_4 | 5 << 12 | 1 << 8,
    Instruction::RETURN | 1 << 8);

  ASSERT_TRUE(Check(data));
}

TEST_F(RegisterAllocatorTest, Loop2) {
  /*
   * Test the following snippet:
   *  int a = 0;
   *  while (a == 8) {
   *    a = 4 + 5;
   *  }
   *  return 6 + 7;
   *
   * Which becomes the following graph:
   *       constant0
   *       constant4
   *       constant5
   *       constant6
   *       constant7
   *       constant8
   *       goto
   *        |
   *       goto
   *        |
   *       phi
   *       equal
   *       if +++++
   *        |       \ +
   *        |      4 + 5
   *        |      goto
   *        |
   *       6 + 7
   *       return
   *        |
   *       exit
   */

  const std::vector<uint16_t> data = TWO_REGISTERS_CODE_ITEM(
    Instruction::CONST_4 | 0 | 0,
    Instruction::CONST_4 | 8 << 12 | 1 << 8,
    Instruction::IF_EQ | 1 << 8, 7,
    Instruction::CONST_4 | 4 << 12 | 0 << 8,
    Instruction::CONST_4 | 5 << 12 | 1 << 8,
    Instruction::ADD_INT, 1 << 8 | 0,
    Instruction::GOTO | 0xFA00,
    Instruction::CONST_4 | 6 << 12 | 1 << 8,
    Instruction::CONST_4 | 7 << 12 | 1 << 8,
    Instruction::ADD_INT, 1 << 8 | 0,
    Instruction::RETURN | 1 << 8);

  ASSERT_TRUE(Check(data));
}

TEST_F(RegisterAllocatorTest, Loop3) {
  /*
   * Test the following snippet:
   *  int a = 0
   *  do {
   *    b = a;
   *    a++;
   *  } while (a != 5)
   *  return b;
   *
   * Which becomes the following graph:
   *       constant0
   *       constant1
   *       constant5
   *       goto
   *        |
   *       goto
   *        |++++++++++++
   *       phi          +
   *       a++          +
   *       equals       +
   *       if           +
   *        |++++++++++++
   *       return
   *        |
   *       exit
   */

  const std::vector<uint16_t> data = THREE_REGISTERS_CODE_ITEM(
    Instruction::CONST_4 | 0 | 0,
    Instruction::ADD_INT_LIT8 | 1 << 8, 1 << 8,
    Instruction::CONST_4 | 5 << 12 | 2 << 8,
    Instruction::IF_NE | 1 << 8 | 2 << 12, 3,
    Instruction::RETURN | 0 << 8,
    Instruction::MOVE | 1 << 12 | 0 << 8,
    Instruction::GOTO | 0xF900);

  HGraph* graph = CreateCFG(data);
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_);
  CHECK(codegen != nullptr);
  SsaLivenessAnalysis liveness(graph, codegen.get(), GetScopedAllocator());
  liveness.Analyze();
  std::unique_ptr<RegisterAllocator> register_allocator =
      RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
  register_allocator->AllocateRegisters();
  ASSERT_TRUE(register_allocator->Validate(false));

  HBasicBlock* loop_header = graph->GetBlocks()[2];
  HPhi* phi = loop_header->GetFirstPhi()->AsPhi();

  LiveInterval* phi_interval = phi->GetLiveInterval();
  LiveInterval* loop_update = phi->InputAt(1)->GetLiveInterval();
  ASSERT_TRUE(phi_interval->HasRegisters());
  ASSERT_TRUE(loop_update->HasRegisters());
  ASSERT_NE(phi_interval->GetRegisters(), loop_update->GetRegisters());

  HBasicBlock* return_block = graph->GetBlocks()[3];
  HReturn* ret = return_block->GetLastInstruction()->AsReturn();
  ASSERT_EQ(phi_interval->GetRegisters(), ret->InputAt(0)->GetLiveInterval()->GetRegisters());
}

TEST_F(RegisterAllocatorTest, FirstRegisterUse) {
  const std::vector<uint16_t> data = THREE_REGISTERS_CODE_ITEM(
    Instruction::CONST_4 | 0 | 0,
    Instruction::XOR_INT_LIT8 | 1 << 8, 1 << 8,
    Instruction::XOR_INT_LIT8 | 0 << 8, 1 << 8,
    Instruction::XOR_INT_LIT8 | 1 << 8, 1 << 8 | 1,
    Instruction::RETURN_VOID);

  HGraph* graph = CreateCFG(data);
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
  if (codegen == nullptr) {
    GTEST_SKIP() << "X86 codegen is unavailable.";
  }
  SsaLivenessAnalysis liveness(graph, codegen.get(), GetScopedAllocator());
  liveness.Analyze();

  HXor* first_xor = graph->GetBlocks()[1]->GetFirstInstruction()->AsXor();
  HXor* last_xor = graph->GetBlocks()[1]->GetLastInstruction()->GetPrevious()->AsXor();
  ASSERT_EQ(last_xor->InputAt(0), first_xor);
  LiveInterval* interval = first_xor->GetLiveInterval();
  ASSERT_EQ(interval->GetEnd(), last_xor->GetLifetimePosition());
  ASSERT_TRUE(interval->GetNextSibling() == nullptr);

  // We need a register for the output of the instruction.
  ASSERT_TRUE(interval->RequiresRegisterForDefinitionAt(interval->GetStart()));
  ASSERT_EQ(interval->FirstRegisterUse(), last_xor->GetLifetimePosition());

  // Split at the next instruction.
  interval = interval->SplitAt(first_xor->GetLifetimePosition() + 2);
  // The user of the split is the last add.
  ASSERT_EQ(interval->FirstRegisterUse(), last_xor->GetLifetimePosition());

  // Split before the last add.
  LiveInterval* new_interval = interval->SplitAt(last_xor->GetLifetimePosition() - 1);
  // Ensure the current interval has no register use...
  ASSERT_EQ(interval->FirstRegisterUse(), kNoLifetime);
  // And the new interval has it for the last add.
  ASSERT_EQ(new_interval->FirstRegisterUse(), last_xor->GetLifetimePosition());
}

TEST_F(RegisterAllocatorTest, FpuSameAsFirstInput) {
  HBasicBlock* block = InitEntryMainExitGraphWithReturnVoid();
  HInstruction* param1 = MakeParam(DataType::Type::kFloat32);
  HNeg* neg = MakeUnOp<HNeg>(block, DataType::Type::kFloat32, param1);
  LocationSummary* locations = LocationSummary::CreateNoCall(GetAllocator(), neg);
  locations->SetInAt(0, Location::FpuRegister(0));
  locations->SetOut(Location::SameAsFirstInput());
  static constexpr size_t kLifetimePosition = 32u;
  neg->SetLifetimePosition(kLifetimePosition);
  LiveInterval* live_interval = LiveInterval::MakeInterval(
      GetScopedAllocator(), DataType::Type::kFloat32, /*is_pair=*/ false, neg);
  live_interval->AddRange(kLifetimePosition, kLifetimePosition + 1);
  // This used to fail a `DCHECK()`.
  bool requires_reg = live_interval->RequiresRegisterForDefinitionAt(kLifetimePosition);
  EXPECT_TRUE(requires_reg);
}

TEST_F(RegisterAllocatorTest, DeadPhi) {
  /* Test for a dead loop phi taking as back-edge input a phi that also has
   * this loop phi as input. Walking backwards in SsaDeadPhiElimination
   * does not solve the problem because the loop phi will be visited last.
   *
   * Test the following snippet:
   *  int a = 0
   *  do {
   *    if (true) {
   *      a = 2;
   *    }
   *  } while (true);
   */

  const std::vector<uint16_t> data = TWO_REGISTERS_CODE_ITEM(
    Instruction::CONST_4 | 0 | 0,
    Instruction::CONST_4 | 1 << 8 | 0,
    Instruction::IF_NE | 1 << 8 | 1 << 12, 3,
    Instruction::CONST_4 | 2 << 12 | 0 << 8,
    Instruction::GOTO | 0xFD00,
    Instruction::RETURN_VOID);

  HGraph* graph = CreateCFG(data);
  SsaDeadPhiElimination(graph).Run();
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph);
  CHECK(codegen != nullptr);
  SsaLivenessAnalysis liveness(graph, codegen.get(), GetScopedAllocator());
  liveness.Analyze();
  std::unique_ptr<RegisterAllocator> register_allocator =
      RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
  register_allocator->AllocateRegisters();
  ASSERT_TRUE(register_allocator->Validate(false));
}

/**
 * Test that the TryAllocateFreeReg method works in the presence of inactive intervals
 * that share the same register. It should split the interval it is currently
 * allocating for at the minimum lifetime position between the two inactive intervals.
 * This test only applies to the linear scan allocator.
 */
void RegisterAllocatorTest::TestFreeUntil(bool special_first) {
  HBasicBlock* block = InitEntryMainExitGraphWithReturnVoid();
  HInstruction* const0 = graph_->GetIntConstant(0);

  HAdd* add = MakeBinOp<HAdd>(block, DataType::Type::kInt32, const0, const0);
  HInstruction* placeholder1 = MakeUnOp<HNeg>(block, DataType::Type::kInt32, const0);
  HInstruction* placeholder2 = MakeUnOp<HNeg>(block, DataType::Type::kInt32, const0);
  MakeReturn(block, add);

  graph_->ComputeDominanceInformation();
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
  if (codegen == nullptr) {
    GTEST_SKIP() << "X86 codegen is unavailable.";
  }
  SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
  liveness.Analyze();

  // Avoid allocating the register for the `const0` when used by the `add`.
  add->GetLocations()->SetInAt(0, Location::ConstantLocation(const0));
  ASSERT_TRUE(add->GetLocations()->InAt(1).IsConstant());

  // Record placeholder positions for blocking intervals and remove placeholders.
  size_t blocking_pos1 = placeholder1->GetLiveInterval()->GetStart();
  size_t blocking_pos2 = placeholder2->GetLiveInterval()->GetStart();
  auto& const0_uses = const0->GetLiveInterval()->uses_;
  ASSERT_EQ(4, std::distance(const0_uses.begin(), const0_uses.end()));
  auto add_it = std::next(const0_uses.begin());
  ASSERT_TRUE(add_it->GetUser() == add);
  for (HInstruction* placeholder : {placeholder1, placeholder2}) {
    block->RemoveInstruction(placeholder);
    ASSERT_TRUE(std::next(add_it)->GetUser() == placeholder);
    const0_uses.erase_after(add_it);
    // Set the block again in the dead placeholders to allow `liveness` to retrieve the block.
    placeholder->SetBlock(block);
  }

  // Set just one register available to make all intervals compete for the same.
  BlockCoreRegistersExcept(codegen.get(), {x86::EAX});

  RegisterAllocatorLinearScan register_allocator(GetScopedAllocator(), codegen.get(), liveness);

  // Test two variants, so that we hit the desired configuration once, no matter the order
  // in which the register allocator inserts the blocking intervals into inactive intervals.
  size_t call_pos = special_first ? blocking_pos2 : blocking_pos1;
  size_t special_pos = special_first ? blocking_pos1 : blocking_pos2;
  register_allocator.block_registers_for_call_interval_->AddRange(call_pos, call_pos + 1);
  register_allocator.block_registers_special_interval_->AddRange(special_pos, special_pos + 1);

  register_allocator.AllocateRegistersInternal();

  std::pair<size_t, int> expected_add_start_and_regs[] = {
    {add->GetLifetimePosition() + kLivenessPositionOfNonOverlappingOutput, 1u << 0},
    {blocking_pos1, kNoRegisters},
  };
  LiveInterval* li = add->GetLiveInterval();
  for (const std::pair<size_t, int>& expected_start_and_regs : expected_add_start_and_regs) {
    ASSERT_TRUE(li != nullptr);
    ASSERT_EQ(expected_start_and_regs.first, li->GetStart());
    ASSERT_EQ(expected_start_and_regs.second, li->GetRegisters());
    li = li->GetNextSibling();
  }
  ASSERT_TRUE(li == nullptr);
}

TEST_F(RegisterAllocatorTest, FreeUntilCallFirst) {
  TestFreeUntil(/*special_first=*/ false);
}

TEST_F(RegisterAllocatorTest, FreeUntilSpecialFirst) {
  TestFreeUntil(/*special_first=*/ true);
}

void RegisterAllocatorTest::BuildIfElseWithPhi(HPhi** phi,
                                               HInstruction** input1,
                                               HInstruction** input2) {
  HBasicBlock* join = InitEntryMainExitGraph();
  auto [if_block, then, else_] = CreateDiamondPattern(join);
  HInstruction* parameter = MakeParam(DataType::Type::kReference);
  HInstruction* test = MakeIFieldGet(if_block, parameter, DataType::Type::kBool, MemberOffset(22));
  MakeIf(if_block, test);

  *input1 = MakeIFieldGet(then, parameter, DataType::Type::kInt32, MemberOffset(42));
  *input2 = MakeIFieldGet(else_, parameter, DataType::Type::kInt32, MemberOffset(42));
  *phi = MakePhi(join, {*input1, *input2});
  MakeReturn(join, *phi);

  graph_->BuildDominatorTree();
  graph_->AnalyzeLoops();
}

TEST_F(RegisterAllocatorTest, PhiHint) {
  HPhi *phi;
  HInstruction *input1, *input2;

  {
    BuildIfElseWithPhi(&phi, &input1, &input2);
    std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
    if (codegen == nullptr) {
      GTEST_SKIP() << "X86 codegen is unavailable.";
    }
    CHECK(codegen != nullptr);
    SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
    liveness.Analyze();

    // Check that the register allocator is deterministic.
    std::unique_ptr<RegisterAllocator> register_allocator =
        RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
    register_allocator->AllocateRegisters();

    ASSERT_EQ(input1->GetLiveInterval()->GetRegisters(), 1u << 0);
    ASSERT_EQ(input2->GetLiveInterval()->GetRegisters(), 1u << 0);
    ASSERT_EQ(phi->GetLiveInterval()->GetRegisters(), 1u << 0);
  }

  {
    BuildIfElseWithPhi(&phi, &input1, &input2);
    std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
    CHECK(codegen != nullptr);
    SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
    liveness.Analyze();

    // Set the phi to a specific register, and check that the inputs get allocated
    // the same register.
    phi->GetLocations()->UpdateOut(Location::CoreRegister(2));
    std::unique_ptr<RegisterAllocator> register_allocator =
        RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
    register_allocator->AllocateRegisters();

    ASSERT_EQ(input1->GetLiveInterval()->GetRegisters(), 1u << 2);
    ASSERT_EQ(input2->GetLiveInterval()->GetRegisters(), 1u << 2);
    ASSERT_EQ(phi->GetLiveInterval()->GetRegisters(), 1u << 2);
  }

  {
    BuildIfElseWithPhi(&phi, &input1, &input2);
    std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
    CHECK(codegen != nullptr);
    SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
    liveness.Analyze();

    // Set input1 to a specific register, and check that the phi and other input get allocated
    // the same register.
    input1->GetLocations()->UpdateOut(Location::CoreRegister(2));
    std::unique_ptr<RegisterAllocator> register_allocator =
        RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
    register_allocator->AllocateRegisters();

    ASSERT_EQ(input1->GetLiveInterval()->GetRegisters(), 1u << 2);
    ASSERT_EQ(input2->GetLiveInterval()->GetRegisters(), 1u << 2);
    ASSERT_EQ(phi->GetLiveInterval()->GetRegisters(), 1u << 2);
  }

  {
    BuildIfElseWithPhi(&phi, &input1, &input2);
    std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
    CHECK(codegen != nullptr);
    SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
    liveness.Analyze();

    // Set input2 to a specific register, and check that the phi and other input get allocated
    // the same register.
    input2->GetLocations()->UpdateOut(Location::CoreRegister(2));
    std::unique_ptr<RegisterAllocator> register_allocator =
        RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
    register_allocator->AllocateRegisters();

    ASSERT_EQ(input1->GetLiveInterval()->GetRegisters(), 1u << 2);
    ASSERT_EQ(input2->GetLiveInterval()->GetRegisters(), 1u << 2);
    ASSERT_EQ(phi->GetLiveInterval()->GetRegisters(), 1u << 2);
  }
}

void RegisterAllocatorTest::BuildFieldReturn(HInstruction** field, HInstruction** ret) {
  HBasicBlock* block = InitEntryMainExitGraph();
  HInstruction* parameter = MakeParam(DataType::Type::kReference);

  *field = MakeIFieldGet(block, parameter, DataType::Type::kInt32, MemberOffset(42));
  *ret = MakeReturn(block, *field);

  graph_->BuildDominatorTree();
}

TEST_F(RegisterAllocatorTest, ExpectedInRegisterHint) {
  HInstruction *field, *ret;

  {
    BuildFieldReturn(&field, &ret);
    std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
    if (codegen == nullptr) {
      GTEST_SKIP() << "X86 codegen is unavailable.";
    }
    SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
    liveness.Analyze();

    std::unique_ptr<RegisterAllocator> register_allocator =
        RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
    register_allocator->AllocateRegisters();

    // Check the validity that in normal conditions, the register should be hinted to 0 (EAX).
    ASSERT_EQ(field->GetLiveInterval()->GetRegisters(), 1u << 0);
  }

  {
    BuildFieldReturn(&field, &ret);
    std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
    CHECK(codegen != nullptr);
    SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
    liveness.Analyze();

    // Check that the field gets put in the register expected by its use.
    // Don't use SetInAt because we are overriding an already allocated location.
    ret->GetLocations()->Inputs()[0] = Location::CoreRegister(2);

    std::unique_ptr<RegisterAllocator> register_allocator =
        RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
    register_allocator->AllocateRegisters();

    ASSERT_EQ(field->GetLiveInterval()->GetRegisters(), 1u << 2);
  }
}

void RegisterAllocatorTest::BuildTwoSubs(HInstruction** first_sub, HInstruction** second_sub) {
  HBasicBlock* block = InitEntryMainExitGraph();
  HInstruction* parameter = MakeParam(DataType::Type::kInt32);
  HInstruction* constant1 = graph_->GetIntConstant(1);
  HInstruction* constant2 = graph_->GetIntConstant(2);

  *first_sub = new (GetAllocator()) HSub(DataType::Type::kInt32, parameter, constant1);
  block->AddInstruction(*first_sub);
  *second_sub = new (GetAllocator()) HSub(DataType::Type::kInt32, *first_sub, constant2);
  block->AddInstruction(*second_sub);
  MakeReturn(block, *second_sub);

  graph_->BuildDominatorTree();
}

TEST_F(RegisterAllocatorTest, SameAsFirstInputHint) {
  HInstruction *first_sub, *second_sub;

  {
    BuildTwoSubs(&first_sub, &second_sub);
    std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
    if (codegen == nullptr) {
      GTEST_SKIP() << "X86 codegen is unavailable.";
    }
    SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
    liveness.Analyze();

    std::unique_ptr<RegisterAllocator> register_allocator =
        RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
    register_allocator->AllocateRegisters();

    // Check the validity that in normal conditions, the registers are the same.
    ASSERT_EQ(first_sub->GetLiveInterval()->GetRegisters(), 1u << 1);
    ASSERT_EQ(second_sub->GetLiveInterval()->GetRegisters(), 1u << 1);
  }

  {
    BuildTwoSubs(&first_sub, &second_sub);
    std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
    CHECK(codegen != nullptr);
    SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
    liveness.Analyze();

    // check that both adds get the same register.
    // Don't use UpdateOutput because output is already allocated.
    OverrideOutput(first_sub->InputAt(0)->GetLocations(), Location::CoreRegister(2));
    ASSERT_EQ(first_sub->GetLocations()->Out().GetPolicy(), Location::kSameAsFirstInput);
    ASSERT_EQ(second_sub->GetLocations()->Out().GetPolicy(), Location::kSameAsFirstInput);

    std::unique_ptr<RegisterAllocator> register_allocator =
        RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
    register_allocator->AllocateRegisters();

    ASSERT_EQ(first_sub->GetLiveInterval()->GetRegisters(), 1u << 2);
    ASSERT_EQ(second_sub->GetLiveInterval()->GetRegisters(), 1u << 2);
  }
}

void RegisterAllocatorTest::BuildDiv(HInstruction** div) {
  HBasicBlock* block = InitEntryMainExitGraph();
  HInstruction* first = MakeParam(DataType::Type::kInt32);
  HInstruction* second = MakeParam(DataType::Type::kInt32);

  *div = new (GetAllocator()) HDiv(
      DataType::Type::kInt32, first, second, 0);  // don't care about dex_pc.
  block->AddInstruction(*div);
  MakeReturn(block, *div);

  graph_->BuildDominatorTree();
}

TEST_F(RegisterAllocatorTest, ExpectedExactInRegisterAndSameOutputHint) {
  HInstruction *div;
  BuildDiv(&div);
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
  if (codegen == nullptr) {
    GTEST_SKIP() << "X86 codegen is unavailable.";
  }
  SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
  liveness.Analyze();

  std::unique_ptr<RegisterAllocator> register_allocator =
      RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
  register_allocator->AllocateRegisters();

  // div on x86 requires its first input in eax and the output be the same as the first input.
  ASSERT_EQ(div->GetLiveInterval()->GetRegisters(), 1u << 0);
}

// Test a bug in the register allocator, where allocating a blocked
// register would lead to spilling an inactive interval at the wrong
// position.
// This test only applies to the linear scan allocator.
void RegisterAllocatorTest::TestSpillInactive() {
  HBasicBlock* block = InitEntryMainExitGraphWithReturnVoid();
  HInstruction* one = MakeParam(DataType::Type::kInt32);
  HInstruction* two = MakeParam(DataType::Type::kInt32);
  HInstruction* three = MakeParam(DataType::Type::kInt32);
  HInstruction* four = MakeParam(DataType::Type::kInt32);

  // We create a synthesized user requesting a register, to avoid just spilling the
  // intervals.
  HPhi* user = new (GetAllocator()) HPhi(GetAllocator(), 0, 1, DataType::Type::kInt32);
  user->SetBlock(block);
  user->AddInput(one);
  LocationSummary* locations = LocationSummary::CreateNoCall(GetAllocator(), user);
  locations->SetInAt(0, Location::RequiresCoreRegister());
  static constexpr size_t phi_ranges[][2] = {{10 * kLppi, 15 * kLppi}};
  BuildInterval(phi_ranges, arraysize(phi_ranges), GetScopedAllocator(), kNoRegisters, user);

  // Create an interval with lifetime holes.
  static constexpr size_t ranges1[][2] =
      {{0u * kLppi, 2u * kLppi}, {4u * kLppi, 5u * kLppi}, {7u * kLppi, 8u * kLppi}};
  LiveInterval* first =
      BuildInterval(ranges1, arraysize(ranges1), GetScopedAllocator(), kNoRegisters, one);
  first->uses_.push_front(*new (GetScopedAllocator()) UsePosition(user, 0u, 7u * kLppi));
  first->uses_.push_front(*new (GetScopedAllocator()) UsePosition(user, 0u, 6u * kLppi));
  first->uses_.push_front(*new (GetScopedAllocator()) UsePosition(user, 0u, 5u * kLppi));

  locations = LocationSummary::CreateNoCall(GetAllocator(), first->GetDefinedBy());
  locations->SetOut(Location::RequiresCoreRegister());
  first = first->SplitAt(1u * kLppi);

  // Create an interval that conflicts with the next interval, to force the next
  // interval to call `AllocateBlockedReg`.
  static constexpr size_t ranges2[][2] = {{2u * kLppi, 4u * kLppi}};
  LiveInterval* second =
      BuildInterval(ranges2, arraysize(ranges2), GetScopedAllocator(), kNoRegisters, two);
  locations = LocationSummary::CreateNoCall(GetAllocator(), second->GetDefinedBy());
  locations->SetOut(Location::RequiresCoreRegister());

  // Create an interval that will lead to splitting the first interval. The bug occured
  // by splitting at a wrong position, in this case at the next intersection between
  // this interval and the first interval. We would have then put the interval with ranges
  // "[0, 2(, [4, 6(" in the list of handled intervals, even though we haven't processed intervals
  // before lifetime position 6 yet.
  static constexpr size_t ranges3[][2] = {{2u * kLppi, 4u * kLppi}, {7u * kLppi, 8u * kLppi}};
  LiveInterval* third =
      BuildInterval(ranges3, arraysize(ranges3), GetScopedAllocator(), kNoRegisters, three);
  third->uses_.push_front(*new (GetScopedAllocator()) UsePosition(user, 0u, 7u * kLppi));
  third->uses_.push_front(*new (GetScopedAllocator()) UsePosition(user, 0u, 4u * kLppi));
  third->uses_.push_front(*new (GetScopedAllocator()) UsePosition(user, 0u, 3u * kLppi));
  locations = LocationSummary::CreateNoCall(GetAllocator(), third->GetDefinedBy());
  locations->SetOut(Location::RequiresCoreRegister());
  third = third->SplitAt(3u * kLppi);

  // Because the first part of the split interval was considered handled, this interval
  // was free to allocate the same register, even though it conflicts with it.
  static constexpr size_t ranges4[][2] = {{4u * kLppi, 5u * kLppi}};
  LiveInterval* fourth =
      BuildInterval(ranges4, arraysize(ranges4), GetScopedAllocator(), kNoRegisters, four);
  locations = LocationSummary::CreateNoCall(GetAllocator(), fourth->GetDefinedBy());
  locations->SetOut(Location::RequiresCoreRegister());

  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
  if (codegen == nullptr) {
    GTEST_SKIP() << "X86 codegen is unavailable.";
  }
  SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
  // Populate the instructions in the liveness object, to please the register allocator.
  liveness.instructions_from_lifetime_position_.assign(16, user);

  // Set just one register available to make all intervals compete for the same.
  BlockCoreRegistersExcept(codegen.get(), {x86::EAX});

  RegisterAllocatorLinearScan register_allocator(GetScopedAllocator(), codegen.get(), liveness);
  register_allocator.unhandled_core_intervals_.assign({fourth, third, second, first});

  // We have set up all intervals manually and we want `AllocateRegistersInternal()` to run
  // the linear scan without processing instructions - check that the linear order is empty.
  ASSERT_TRUE(codegen->GetGraph()->GetLinearOrder().empty());
  register_allocator.AllocateRegistersInternal();

  // Test that there is no conflicts between intervals.
  ScopedArenaVector<LiveInterval*> intervals({first, second, third, fourth},
                                             GetScopedAllocator()->Adapter());
  ASSERT_TRUE(ValidateIntervals(intervals, *codegen));
}

TEST_F(RegisterAllocatorTest, SpillInactive) {
  TestSpillInactive();
}

TEST_F(RegisterAllocatorTest, ReuseSpillSlots) {
  if (!com::android::art::flags::reg_alloc_spill_slot_reuse()) {
    GTEST_SKIP() << "Improved spill slot reuse disabled.";
  }
  HBasicBlock* return_block = InitEntryMainExitGraph();
  auto [start, left, right] = CreateDiamondPattern(return_block);
  HInstruction* obj = MakeParam(DataType::Type::kReference);
  HInstruction* cond = MakeIFieldGet(start, obj, DataType::Type::kBool, MemberOffset(32));
  MakeIf(start, cond);

  // Load two values from fields. Both shall be used as Phi inputs later.
  HInstruction* left_get1 = MakeIFieldGet(left, obj, DataType::Type::kInt32, MemberOffset(36));
  HInstruction* left_get2 = MakeIFieldGet(left, obj, DataType::Type::kInt32, MemberOffset(40));
  // Convert one of the values to `Int64` to spill the loaded values.
  HInstruction* left_conv1 = MakeUnOp<HTypeConversion>(left, DataType::Type::kInt64, left_get1);
  // Convert the `Int64` value back to `Int32`. x86 codegen uses EAX and EDX for conversion
  // which is not a normal pair, so avoid using this odd explicit pair for a Phi.
  HInstruction* left_conv2 = MakeUnOp<HTypeConversion>(left, DataType::Type::kInt32, left_conv1);

  // Repeat the sequence from `left` block in the `right` block (with different offsets). Without
  // spill slot hints, spill slots should be assigned the same way as in the `left` block.
  HInstruction* right_get1 = MakeIFieldGet(right, obj, DataType::Type::kInt32, MemberOffset(44));
  HInstruction* right_get2 = MakeIFieldGet(right, obj, DataType::Type::kInt32, MemberOffset(48));
  HInstruction* right_conv1 = MakeUnOp<HTypeConversion>(right, DataType::Type::kInt64, right_get1);
  HInstruction* right_conv2 = MakeUnOp<HTypeConversion>(right, DataType::Type::kInt32, right_conv1);

  // Add Phis that tie the first field load in `left` to the second field load in `right` and
  // vice versa, to check that the hints can align the spill slots assigned to inputs.
  HPhi* phi1 = MakePhi(return_block, {left_get1, right_get2});
  HPhi* phi2 = MakePhi(return_block, {left_get2, right_get1});

  // Add some instructions that use the `phi1`, `phi2` and even the converted values
  // to derive some return value.
  HPhi* phi_conv = MakePhi(return_block, {left_conv2, right_conv2});
  HMin* min1 = MakeBinOp<HMin>(return_block, DataType::Type::kInt32, phi1, phi2);
  HMin* min2 = MakeBinOp<HMin>(return_block, DataType::Type::kInt32, min1, phi_conv);
  MakeReturn(return_block, min2);

  graph_->ComputeDominanceInformation();
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
  if (codegen == nullptr) {
    GTEST_SKIP() << "X86 codegen is unavailable.";
  }
  SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
  liveness.Analyze();

  // Set just two registers available to make it easy to force spills.
  // Choose EAX and EDX which are used by type conversion from Int32 to Int64, so that
  // we can use the type conversion to spill all live intervals wherever we want.
  BlockCoreRegistersExcept(codegen.get(), {x86::EAX, x86::EDX});

  // Change the `obj` parameter to come in EDX.
  OverrideOutput(obj->GetLocations(), Location::CoreRegister(x86::EDX));

  std::unique_ptr<RegisterAllocator> register_allocator =
      RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
  register_allocator->AllocateRegisters();

  // Field loads would be spilled even without using spill slot hints.
  ASSERT_TRUE(left_get1->GetLiveInterval()->HasSpillSlot());
  ASSERT_TRUE(left_get2->GetLiveInterval()->HasSpillSlot());
  ASSERT_TRUE(right_get1->GetLiveInterval()->HasSpillSlot());
  ASSERT_TRUE(right_get2->GetLiveInterval()->HasSpillSlot());

  // Input spill slots are aligned thanks to the spill slot hints.
  EXPECT_EQ(left_get1->GetLiveInterval()->GetSpillSlot(),
            right_get2->GetLiveInterval()->GetSpillSlot());
  EXPECT_EQ(left_get2->GetLiveInterval()->GetSpillSlot(),
            right_get1->GetLiveInterval()->GetSpillSlot());

  // Check that `phi1` and `phi2` use the spill slots used by their inputs.
  EXPECT_TRUE(phi1->GetLiveInterval()->HasSpillSlot());
  EXPECT_EQ(left_get1->GetLiveInterval()->GetSpillSlot(), phi1->GetLiveInterval()->GetSpillSlot());
  EXPECT_TRUE(phi2->GetLiveInterval()->HasSpillSlot());
  EXPECT_EQ(left_get2->GetLiveInterval()->GetSpillSlot(), phi2->GetLiveInterval()->GetSpillSlot());

  // Check that `phi1` and `phi2` are split and don't have a register in the first sibling.
  EXPECT_TRUE(phi1->GetLiveInterval()->GetNextSibling() != nullptr);
  EXPECT_TRUE(!phi1->GetLiveInterval()->HasRegisters());
  EXPECT_TRUE(phi2->GetLiveInterval()->GetNextSibling() != nullptr);
  EXPECT_TRUE(!phi2->GetLiveInterval()->HasRegisters());
}

TEST_F(RegisterAllocatorTest, ReuseSpillSlotGaps) {
  if (!com::android::art::flags::reg_alloc_spill_slot_reuse()) {
    GTEST_SKIP() << "Improved spill slot reuse disabled.";
  }
  HBasicBlock* return_block = InitEntryMainExitGraph();
  auto [pre_header, header, body] = CreateWhileLoop(return_block);

  HInstruction* const0 = graph_->GetIntConstant(0);
  HInstruction* const10 = graph_->GetIntConstant(10);

  HPhi* phi1 = MakePhi(header, {const0, /* placeholder */ const0});
  HNeg* neg1 = MakeUnOp<HNeg>(body, DataType::Type::kInt32, phi1);
  phi1->ReplaceInput(neg1, 1u);  // Update back-edge input.

  HPhi* phi2 = MakePhi(header, {const0, /* placeholder */ const0});
  HNeg* neg2 = MakeUnOp<HNeg>(body, DataType::Type::kInt32, phi2);
  phi2->ReplaceInput(neg2, 1u);  // Update back-edge input.

  // Loop variable and condition. This is added after `neg1` and `neg2` to spill both.
  HPhi* phi = MakePhi(header, {const0, /* placeholder */ const0});
  HNeg* neg = MakeUnOp<HNeg>(body, DataType::Type::kInt32, phi);
  phi->ReplaceInput(neg, 1u);  // Update back-edge input.
  HCondition* cond = MakeCondition(header, kCondGE, phi, const10);
  MakeIf(header, cond);

  // Add an environment use of `phi1` and a normal use of `phi2`.
  HCondition* deopt_cond = MakeCondition(header, kCondLT, phi, const0);
  HDeoptimize* deopt = new (GetAllocator()) HDeoptimize(
      GetAllocator(), deopt_cond, DeoptimizationKind::kDebugging, /*dex_pc=*/ 0u);
  AddOrInsertInstruction(return_block, deopt);
  ManuallyBuildEnvFor(deopt, {phi1});
  MakeReturn(return_block, phi2);

  graph_->BuildDominatorTree();
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
  if (codegen == nullptr) {
    GTEST_SKIP() << "X86 codegen is unavailable.";
  }
  SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
  liveness.Analyze();

  // Set just one register available to make all intervals compete for the same.
  BlockCoreRegistersExcept(codegen.get(), {x86::EAX});
  // Rewrite condition locations to work with the single register EAX.
  for (HCondition* c : {cond, deopt_cond}) {
    ASSERT_TRUE(c->GetLocations()->Out().Equals(Location::CoreRegister(x86::ECX)));
    OverrideOutput(c->GetLocations(), Location::CoreRegister(x86::EAX));
    c->GetLocations()->SetInAt(0, Location::Any());
    ASSERT_TRUE(c->GetLocations()->InAt(1).Equals(Location::Any()));
  }

  std::unique_ptr<RegisterAllocator> register_allocator =
      RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
  register_allocator->AllocateRegisters();

  ASSERT_TRUE(phi1->GetLiveInterval()->HasSpillSlot());
  ASSERT_TRUE(neg1->GetLiveInterval()->HasSpillSlot());
  EXPECT_EQ(phi1->GetLiveInterval()->GetSpillSlot(), neg1->GetLiveInterval()->GetSpillSlot());
  ASSERT_TRUE(phi2->GetLiveInterval()->HasSpillSlot());
  ASSERT_TRUE(neg2->GetLiveInterval()->HasSpillSlot());
  EXPECT_EQ(phi2->GetLiveInterval()->GetSpillSlot(), neg2->GetLiveInterval()->GetSpillSlot());
}

// Regression test for wrongly assuming that a Phi interval with a spill slot hint
// is not split when checking if the spill slot can be used. Indeed, it can be split
// and we must use the sibling to determine the lifetime end.
TEST_F(RegisterAllocatorTest, ReuseSpillSlotsUnavailableWithSplitPhiInterval) {
  if (!com::android::art::flags::reg_alloc_spill_slot_reuse()) {
    GTEST_SKIP() << "Improved spill slot reuse disabled.";
  }
  if (!com::android::art::flags::reg_alloc_no_output_overlap()) {
    GTEST_SKIP() << "Improved `Location::kNoOutputOverlap` handling disabled.";
  }
  HBasicBlock* return_block = InitEntryMainExitGraph();
  auto [start, left, right] = CreateDiamondPattern(return_block);
  HInstruction* const0 = graph_->GetIntConstant(0);
  HInstruction* obj = MakeParam(DataType::Type::kReference);
  HInstruction* cond = MakeIFieldGet(start, obj, DataType::Type::kBool, MemberOffset(32));
  MakeIf(start, cond);

  // Add a load followed by `HNeg`, so that the loaded value is spilled.
  HInstruction* left_get = MakeIFieldGet(left, obj, DataType::Type::kInt32, MemberOffset(36));
  HNeg* left_neg = MakeUnOp<HNeg>(left, DataType::Type::kInt32, left_get);

  // Repeat the sequence from `left` block in the `right` block (with a different offset).
  HInstruction* right_get = MakeIFieldGet(right, obj, DataType::Type::kInt32, MemberOffset(40));
  HNeg* right_neg = MakeUnOp<HNeg>(right, DataType::Type::kInt32, right_get);

  // Phis shall be processed in the order in which they are inserted.
  // The first Phi shall initially be allocated the only available register.
  HPhi* phi1 = MakePhi(return_block, {const0, right_neg});
  // The second Phi has no register use, so it shall be spilled.
  // The spill slot used by `left_get` and `right_get` shall be reused for this unrelated Phi.
  HPhi* phi2 = MakePhi(return_block, {left_neg, const0});
  // The third Phi has a hint that would put it to the same spill slot as both of its inputs
  // if it was not already taken by `phi2`. But the spill slot is no longer available, so we
  // try to allocate a register. Since `get_phi`'s first register use is before the `phi1`'s
  // first register use, we allocate the register for `get_phi` and re-insert `phi1` to the
  // unhandled intervals. However, since the last use of `get_phi` is after the `invoke`
  // which blocks the register, `get_phi`'s interval shall be split in the process.
  HPhi* get_phi = MakePhi(return_block, {left_get, right_get});
  // The fourth Phi has an earlier register use than `get_phi`, so it shall be allocated
  // the register and `get_phi`'s interval shall be re-inserted to unhandled intervals.
  HPhi* neg_phi = MakePhi(return_block, {left_neg, right_neg});
  // Then we shall re-process `phi1`, assigning the next spill slot.
  // Then we shall re-process `get_phi` and try to assign the hint slot where we previously
  // wrongly assumed that it has no sibling and triggered a `DCHECK()`.

  // Add a register use for the `neg_phi`.
  HNeg* neg_neg = MakeUnOp<HNeg>(return_block, DataType::Type::kInt32, neg_phi);
  // Add a register use for the `get_phi`.
  // Use `HSub` which can have the second operand on the stack for x86.
  HSub* sub1 = MakeBinOp<HSub>(return_block, DataType::Type::kInt32, get_phi, neg_neg);
  // Add an invoke that forces the `get_phi` interval to be split when initially allocated.
  MakeInvokeStatic(return_block, DataType::Type::kVoid, {}, {});
  // Add another register use for the `get_phi` after the `invoke`.
  HSub* sub2 = MakeBinOp<HSub>(return_block, DataType::Type::kInt32, get_phi, sub1);

  // Add some instructions that use all the values to derive some return value.
  HSub* sub3 = MakeBinOp<HSub>(return_block, DataType::Type::kInt32, sub2, phi1);
  HSub* sub4 = MakeBinOp<HSub>(return_block, DataType::Type::kInt32, sub3, phi2);
  MakeReturn(return_block, sub4);

  graph_->ComputeDominanceInformation();
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
  if (codegen == nullptr) {
    GTEST_SKIP() << "X86 codegen is unavailable.";
  }
  SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
  liveness.Analyze();

  // Set just one register available to make all intervals compete for the same.
  BlockCoreRegistersExcept(codegen.get(), {x86::EAX});
  // Change the `obj` parameter to come in EAX.
  OverrideOutput(obj->GetLocations(), Location::CoreRegister(x86::EAX));

  std::unique_ptr<RegisterAllocator> register_allocator =
      RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
  register_allocator->AllocateRegisters();

  ASSERT_TRUE(left_get->GetLiveInterval()->HasSpillSlot());
  ASSERT_TRUE(right_get->GetLiveInterval()->HasSpillSlot());
  ASSERT_EQ(left_get->GetLiveInterval()->GetSpillSlot(),
            right_get->GetLiveInterval()->GetSpillSlot());

  ASSERT_TRUE(phi2->GetLiveInterval()->HasSpillSlot());
  ASSERT_EQ(left_get->GetLiveInterval()->GetSpillSlot(), phi2->GetLiveInterval()->GetSpillSlot());

  ASSERT_TRUE(get_phi->GetLiveInterval()->HasSpillSlot());
  ASSERT_NE(left_get->GetLiveInterval()->GetSpillSlot(),
            get_phi->GetLiveInterval()->GetSpillSlot());
}

// Regression test for splitting a live range that's recorded as a search hint in the
// `SpillSlotData`. Previously, a new range was created for the first part of the live
// range and the original `LiveRange` object had its start adjusted but that means the
// search hint was pointing to the second part of the split live range and we missed
// overlaps when trying to reuse a spill slot. Bug: 426785078
TEST_F(RegisterAllocatorTest, SplitSpillSlotLiveRangeHint) {
  if (!com::android::art::flags::reg_alloc_spill_slot_reuse()) {
    GTEST_SKIP() << "Improved spill slot reuse disabled.";
  }
  // Create a graph with a three-way switch to blocks `left`, `mid` and `right`,
  // and a diamond pattern in the `left` block with branch blocks `left_left`
  // and `left_right` and merging to `left_end`. The linear order shall have
  // the block `right` followed by `mid` and `mid` followed by `left`.
  HBasicBlock* return_block = InitEntryMainExitGraph();
  auto [start, left_end, mid] = CreateDiamondPattern(return_block);
  HBasicBlock* right = AddNewBlock();
  start->AddSuccessor(right);
  right->AddSuccessor(return_block);
  auto [left, left_left, left_right] = CreateDiamondPattern(left_end);

  // Create `iget_*` instructions in `start`, `right`, `mid` and `left_right`
  // (listed in linear order). Merge `iget_start` and `iget_left_right` with
  // a `phi_left_end` in `left_end` and merge `phi_left_end`, `iget_mid` and
  // `iget_right` with the final `phi` in `return_block`. Use invokes to force
  // spilling `iget_*`s; in `left_right` use `HMin` instead to avoid blocking
  // registers too early. When the `iget_start` is spilled, the spill slot is
  // propagated to the Phis as a hint that's seen by all the other `iget_*`s.
  //
  // For `iget_start`, add a register use in `left`, an `HDeoptimize` in
  // `left_end` to extend the `iget_start` lifetime across `left_right` and
  // most of `left_end` and an invoke in `left_end` before the `HDeoptimize`
  // which blocks registers and forces the splitting of the `iget_start`
  // interval when allocating a register for the use in `left`.

  HInstruction* obj = MakeParam(DataType::Type::kReference);
  HInstruction* const1 = graph_->GetIntConstant(1);

  HInstruction* iget_start = MakeIFieldGet(start, obj, DataType::Type::kInt32, MemberOffset(40));
  HInstruction* switch_input = MakeIFieldGet(start, obj, DataType::Type::kInt32, MemberOffset(32));
  MakeInvokeStatic(start, DataType::Type::kVoid, {});  // Spill all.
  start->AddInstruction(
      new (GetAllocator()) HPackedSwitch(/*start_value=*/ 0, /*num_entries=*/ 2, switch_input));

  HInstruction* iget_right = MakeIFieldGet(right, obj, DataType::Type::kInt32, MemberOffset(44));
  MakeInvokeStatic(right, DataType::Type::kVoid, {});  // Spill all.
  MakeGoto(right);  // This block was constructed with `AddNewBlock()` without `HGoto`.

  HInstruction* iget_mid = MakeIFieldGet(mid, obj, DataType::Type::kInt32, MemberOffset(48));
  MakeInvokeStatic(mid, DataType::Type::kVoid, {});  // Spill all.

  HSub* sub = MakeBinOp<HSub>(left, DataType::Type::kInt32, iget_start, const1);
  HInstruction* left_cond = MakeIFieldGet(left, obj, DataType::Type::kBool, MemberOffset(36));
  MakeIf(left, left_cond);

  HInstruction* iget_left_right =
      MakeIFieldGet(left_right, obj, DataType::Type::kInt32, MemberOffset(40));
  // Force spilling `iget_left_right` without using an invoke.
  HMin* min = MakeBinOp<HMin>(left_right, DataType::Type::kInt32, sub, const1);

  HPhi* phi_left_end = MakePhi(left_end, {iget_start, iget_left_right});
  HPhi* phi_min = MakePhi(left_end, {sub, min});
  // Block registers to cause splitting of the `iget_start` interval when allocating
  // a register at the start of the `left` block.
  MakeInvokeStatic(left_end, DataType::Type::kVoid, {phi_min});  // Use `phi_min`, spill all.
  HInstruction* deopt_cond = MakeIFieldGet(left_end, obj, DataType::Type::kBool, MemberOffset(37));
  HDeoptimize* deopt = new (GetAllocator()) HDeoptimize(
      GetAllocator(), deopt_cond, DeoptimizationKind::kFullFrame, kNoDexPc);
  AddOrInsertInstruction(left_end, deopt);
  ManuallyBuildEnvFor(deopt, {iget_start});  // Keep `iget_start` alive up to this point.

  HPhi* phi = MakePhi(return_block, {phi_left_end, iget_mid, iget_right});
  MakeReturn(return_block, phi);

  graph_->ComputeDominanceInformation();
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
  if (codegen == nullptr) {
    GTEST_SKIP() << "X86 codegen is unavailable.";
  }
  SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
  liveness.Analyze();

  ASSERT_LT(right->GetLifetimeStart(), mid->GetLifetimeStart());
  ASSERT_LT(mid->GetLifetimeStart(), left->GetLifetimeStart());

  // Set just two register available, including the `obj` input register ECX.
  BlockCoreRegistersExcept(codegen.get(), {x86::EAX, x86::ECX});

  // Before the bug was fixed, the interval validation at the end of `AllocateRegisters()`
  // would report a spill slot conflict for the `iget_left_right` because it was allocated
  // the same spill slot as `iget_start` despite overlapping lifetime. This would cause
  // clobbering of `iget_start` for its use in the `HDeoptimize` environment.
  std::unique_ptr<RegisterAllocator> register_allocator =
      RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
  register_allocator->AllocateRegisters();

  // While `iget_right` and `iget_mid` use the same spill slot as `iget_start` thanks to the
  // hint, the `iget_left_right` must use a different spill slot due to lifetime overlap.
  for (HInstruction* iget : {iget_start, iget_right, iget_mid, iget_left_right}) {
    ASSERT_TRUE(iget->GetLiveInterval()->HasSpillSlot());
  }
  int iget_start_spill_slot = iget_start->GetLiveInterval()->GetSpillSlot();
  EXPECT_EQ(iget_start_spill_slot, iget_right->GetLiveInterval()->GetSpillSlot());
  EXPECT_EQ(iget_start_spill_slot, iget_mid->GetLiveInterval()->GetSpillSlot());
  EXPECT_NE(iget_start_spill_slot, iget_left_right->GetLiveInterval()->GetSpillSlot());
}

void RegisterAllocatorTest::TestNoOutputOverlap() {
  if (!com::android::art::flags::reg_alloc_no_output_overlap()) {
    GTEST_SKIP() << "Improved `Location::kNoOutputOverlap` handling disabled.";
  }
  HBasicBlock* block = InitEntryMainExitGraph();
  HInstruction* const1 = graph_->GetIntConstant(1);
  HNeg* neg = MakeUnOp<HNeg>(block, DataType::Type::kInt32, const1);
  HAdd* add = MakeBinOp<HAdd>(block, DataType::Type::kInt32, neg, const1);
  // Add another use of `neg` after `add`. Previously this would have prevented
  // using the register allocated for `neg` for the `add` output.
  HAdd* add2 = MakeBinOp<HAdd>(block, DataType::Type::kInt32, neg, const1);
  // Insert `HNop` to test the end of the range of the unused instruction `add2`.
  AddOrInsertInstruction(
      block, new (GetAllocator()) HNop(/*dex_pc=*/ 0u, /*needs_environment=*/ false));
  HReturn* ret = MakeReturn(block, add);

  graph_->ComputeDominanceInformation();
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
  if (codegen == nullptr) {
    GTEST_SKIP() << "X86 codegen is unavailable.";
  }
  SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
  liveness.Analyze();

  ASSERT_TRUE(neg->GetLocations()->OutputCanOverlapWithInputs());
  ASSERT_FALSE(add->GetLocations()->OutputCanOverlapWithInputs());
  ASSERT_FALSE(add2->GetLocations()->OutputCanOverlapWithInputs());

  // Set just one register available to make all intervals compete for the same.
  BlockCoreRegistersExcept(codegen.get(), {x86::EAX});

  RegisterAllocatorLinearScan register_allocator(GetScopedAllocator(), codegen.get(), liveness);

  register_allocator.AllocateRegistersInternal();

  // Check that the register 0 was allocated at the right positions for `neg`, `add` and `add2`.
  static constexpr int kExpectedRegister = 0;
  LiveInterval* neg_li = neg->GetLiveInterval();
  ASSERT_EQ(neg->GetLifetimePosition(), neg_li->GetStart());
  ASSERT_EQ(1u << kExpectedRegister, neg_li->GetRegisters());
  LiveInterval* add_li = add->GetLiveInterval();
  ASSERT_EQ(add->GetLifetimePosition() + kLivenessPositionOfNonOverlappingOutput,
            add_li->GetStart());
  ASSERT_EQ(1u << kExpectedRegister, add_li->GetRegisters());
  LiveInterval* add2_li = add2->GetLiveInterval();
  ASSERT_EQ(add2->GetLifetimePosition() + kLivenessPositionOfNonOverlappingOutput,
            add2_li->GetStart());
  ASSERT_EQ(1u << kExpectedRegister, add2_li->GetRegisters());

  // Check the splits of the `neg` interval.
  ASSERT_EQ(add_li->GetStart(), neg_li->GetEnd());
  LiveInterval* neg_li2 = neg_li->GetNextSibling();
  ASSERT_TRUE(neg_li2 != nullptr);
  ASSERT_EQ(add_li->GetStart(), neg_li2->GetStart());
  ASSERT_EQ(kNoRegisters, neg_li2->GetRegisters());
  ASSERT_EQ(add2->GetLifetimePosition(), neg_li2->GetEnd());
  LiveInterval* neg_li3 = neg_li2->GetNextSibling();
  ASSERT_TRUE(neg_li3 != nullptr);
  ASSERT_EQ(add2->GetLifetimePosition(), neg_li3->GetStart());
  ASSERT_EQ(1u << kExpectedRegister, neg_li3->GetRegisters());
  ASSERT_EQ(add2->GetLifetimePosition() + kLivenessPositionOfNormalUse, neg_li3->GetEnd());
  ASSERT_TRUE(neg_li3->GetNextSibling() == nullptr);

  // Check the splits of the `add` interval.
  ASSERT_EQ(add2->GetLifetimePosition(), add_li->GetEnd());
  LiveInterval* add_li2 = add_li->GetNextSibling();
  ASSERT_TRUE(add_li2 != nullptr);
  ASSERT_EQ(add2->GetLifetimePosition(), add_li2->GetStart());
  ASSERT_EQ(kNoRegisters, add_li2->GetRegisters());
  ASSERT_EQ(ret->GetLifetimePosition(), add_li2->GetEnd());
  ASSERT_TRUE(add_li2->GetNextSibling() == nullptr);

  // Check that the interval for the unused `add2` was not split and ends after the `add2`.
  ASSERT_TRUE(add2_li->GetNextSibling() == nullptr);
  ASSERT_EQ(add2->GetLifetimePosition() + kLppi, add2_li->GetEnd());
}

TEST_F(RegisterAllocatorTest, NoOutputOverlap) {
  TestNoOutputOverlap();
}

void RegisterAllocatorTest::TestNoOutputOverlapAndTemp() {
  if (!com::android::art::flags::reg_alloc_no_output_overlap()) {
    GTEST_SKIP() << "Improved `Location::kNoOutputOverlap` handling disabled.";
  }
  HBasicBlock* block = InitEntryMainExitGraph();
  HInstruction* const1 = graph_->GetIntConstant(1);
  HNeg* neg = MakeUnOp<HNeg>(block, DataType::Type::kInt32, const1);
  HAdd* add = MakeBinOp<HAdd>(block, DataType::Type::kInt32, neg, const1);
  HSub* sub = MakeBinOp<HSub>(block, DataType::Type::kInt32, add, neg);

  graph_->ComputeDominanceInformation();
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
  if (codegen == nullptr) {
    GTEST_SKIP() << "X86 codegen is unavailable.";
  }
  SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
  liveness.Analyze();

  ASSERT_TRUE(neg->GetLocations()->OutputCanOverlapWithInputs());
  ASSERT_FALSE(add->GetLocations()->OutputCanOverlapWithInputs());
  ASSERT_TRUE(sub->GetLocations()->OutputCanOverlapWithInputs());
  ASSERT_TRUE(sub->GetLocations()->InAt(1).Equals(Location::Any()));

  // Add a temp for `add`. We want to test that the temp interval shall not be split.
  // Trying to split it would trigger a `DCHECK(!IsTemp())`.
  ASSERT_EQ(0, add->GetLocations()->GetTempCount());
  add->GetLocations()->AddTemp(Location::RequiresCoreRegister());

  // Set just two registers available to avoid adding more instructions
  // to reproduce the situation where we could try to split the temp.
  BlockCoreRegistersExcept(codegen.get(), {x86::EAX, x86::ECX});

  RegisterAllocatorLinearScan register_allocator(GetScopedAllocator(), codegen.get(), liveness);

  register_allocator.AllocateRegistersInternal();

  // Check the splits of the `neg` interval.
  LiveInterval* neg_li = neg->GetLiveInterval();
  ASSERT_EQ(neg->GetLifetimePosition(), neg_li->GetStart());
  ASSERT_TRUE(neg_li->HasRegisters());
  ASSERT_EQ(add->GetLifetimePosition() + kLivenessPositionOfNonOverlappingOutput, neg_li->GetEnd());
  LiveInterval* neg_li2 = neg_li->GetNextSibling();
  ASSERT_TRUE(neg_li2 != nullptr);
  ASSERT_EQ(neg_li->GetEnd(), neg_li2->GetStart());
  ASSERT_EQ(kNoRegisters, neg_li2->GetRegisters());
  ASSERT_EQ(sub->GetLifetimePosition() + kLivenessPositionOfNormalUse, neg_li2->GetEnd());
  ASSERT_TRUE(neg_li2->GetNextSibling() == nullptr);
}

TEST_F(RegisterAllocatorTest, NoOutputOverlapAndTemp) {
  TestNoOutputOverlapAndTemp();
}

TEST_F(RegisterAllocatorTest, NoOutputOverlapImmediateSpill) {
  if (!com::android::art::flags::reg_alloc_no_output_overlap()) {
    GTEST_SKIP() << "Improved `Location::kNoOutputOverlap` handling disabled.";
  }
  HBasicBlock* block = InitEntryMainExitGraph();
  HInstruction* param = MakeParam(DataType::Type::kReference);
  HInvoke* invoke = MakeInvokeStatic(block, DataType::Type::kVoid, {});  // Spill `param`.
  // After the `invoke`, all registers are free and the `param` input to `get1` is allocated
  // register 0. With only registers 0-2 available, the only available pair is 0-1, so the
  // `get1` with no-output-overlap is allocated that pair. Now we need to do something about
  // the `param` but trying to allocate another register now is not easy. We cannot put moves
  // in the middle of an instruction, so we'd have to split the interval at the start of the
  // instruction and move back to that position, potentially bringing back already handled
  // intervals for inputs that die at this instruction and somehow handle the output as live
  // at the start of the instructions even though if its lifetime has not yet started. It's
  // much easier to just force spilling active intervals when we need to take their register
  // for the no-output-overlap output.
  //
  // This test case exposes a situation that led to a compiler crash during development when
  // we didn't force the spill and ended up with two parallel moves for `param` at the start
  // of `get1`, hitting a debug-mode check that prevents such move collisions.
  HInstruction* get1 = MakeIFieldGet(block, param, DataType::Type::kInt64, MemberOffset(32));
  HInstruction* get2 = MakeIFieldGet(block, param, DataType::Type::kInt32, MemberOffset(40));
  HInstruction* conv = MakeUnOp<HTypeConversion>(block, DataType::Type::kInt32, get1);
  HAdd* add = MakeBinOp<HAdd>(block, DataType::Type::kInt32, conv, get2);
  MakeReturn(block, add);

  graph_->BuildDominatorTree();
  std::unique_ptr<CodeGenerator> codegen = CreateCodeGenerator(graph_, InstructionSet::kX86);
  if (codegen == nullptr) {
    GTEST_SKIP() << "X86 codegen is unavailable.";
  }
  SsaLivenessAnalysis liveness(graph_, codegen.get(), GetScopedAllocator());
  liveness.Analyze();

  // There is no instruction that would get the locations required for this test on x86.
  // We need no output overlap for an instruction that takes a single-register input and
  // produces two-register output. This happens for 64-bit `HInstanceFieldGet` on arm32.
  // Note: The processing of "no output overlap" happens at the start of the register
  // allocation. It could conceivably be moved to the liveness analysis which would make
  // this adjustment too late and we'd need to intercept the location creation which is
  // doable as long as the `CodeGeneratorX86` is not `final`.
  LocationSummary* get1_locs = get1->GetLocations();
  ASSERT_TRUE(get1_locs->OutputCanOverlapWithInputs());
  ASSERT_TRUE(get1_locs->Out().Equals(Location::RequiresCoreRegister()));
  get1_locs->UpdateOut(Location());  // Invalidate output to work around `DCHECK()` in `SetOut()`.
  get1_locs->SetOut(Location::RequiresCoreRegister(), Location::kNoOutputOverlap);

  // Make three registers available.
  BlockCoreRegistersExcept(codegen.get(), {x86::EAX, x86::ECX, x86::EDX});

  std::unique_ptr<RegisterAllocator> register_allocator =
      RegisterAllocator::Create(GetScopedAllocator(), codegen.get(), liveness);
  register_allocator->AllocateRegisters();

  // Check the splits of the `param`.
  LiveInterval* param_li = param->GetLiveInterval();
  ASSERT_TRUE(param_li != nullptr);
  ASSERT_TRUE(param_li->HasSpillSlot());
  EXPECT_GT(block->GetLifetimeStart(), param_li->GetStart());
  EXPECT_EQ(block->GetLifetimeStart(), param_li->GetEnd());
  EXPECT_EQ(1u << 1, param_li->GetRegisters());  // Initial argument register.
  LiveInterval* param_li2 = param_li->GetNextSibling();
  ASSERT_TRUE(param_li2 != nullptr);
  EXPECT_EQ(block->GetLifetimeStart(), param_li2->GetStart());
  EXPECT_EQ(invoke->GetLifetimePosition(), param_li2->GetEnd());
  EXPECT_EQ(1u << 0, param_li2->GetRegisters());  // Moved to register 0.
  LiveInterval* param_li3 = param_li2->GetNextSibling();
  ASSERT_TRUE(param_li3 != nullptr);
  EXPECT_EQ(invoke->GetLifetimePosition(), param_li3->GetStart());
  EXPECT_EQ(get1->GetLifetimePosition(), param_li3->GetEnd());
  EXPECT_FALSE(param_li3->HasRegisters());  // No register.
  LiveInterval* param_li4 = param_li3->GetNextSibling();
  ASSERT_TRUE(param_li4 != nullptr);
  EXPECT_EQ(get1->GetLifetimePosition(), param_li4->GetStart());
  EXPECT_EQ(get1->GetLifetimePosition() + kLivenessPositionOfNonOverlappingOutput,
            param_li4->GetEnd());
  EXPECT_EQ(1u << 0, param_li4->GetRegisters());  // Allocated register 0.
  LiveInterval* param_li5 = param_li4->GetNextSibling();
  ASSERT_TRUE(param_li5 != nullptr);
  EXPECT_EQ(get1->GetLifetimePosition() + kLivenessPositionOfNonOverlappingOutput,
            param_li5->GetStart());
  EXPECT_EQ(get2->GetLifetimePosition(), param_li5->GetEnd());
  EXPECT_FALSE(param_li5->HasRegisters());  // No register.
  LiveInterval* param_li6 = param_li5->GetNextSibling();
  ASSERT_TRUE(param_li6 != nullptr);
  EXPECT_EQ(get2->GetLifetimePosition(), param_li6->GetStart());
  EXPECT_EQ(get2->GetLifetimePosition() + kLivenessPositionOfNormalUse, param_li6->GetEnd());
  EXPECT_EQ(1u << 2, param_li6->GetRegisters());  // Allocated reg 2, avoiding `get1` result in 0-1.
  ASSERT_TRUE(param_li6->GetNextSibling() == nullptr);
}

}  // namespace art

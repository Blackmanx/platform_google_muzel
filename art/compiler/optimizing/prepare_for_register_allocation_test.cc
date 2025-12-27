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

#include "prepare_for_register_allocation.h"

#include <gtest/gtest.h>

#include "base/macros.h"
#include "optimizing_unit_test.h"

namespace art HIDDEN {

class PrepareForRegisterAllocationTest
    : public CommonCompilerTest, public OptimizingUnitTestHelper {
 protected:
  void RunPass() {
    graph_->BuildDominatorTree();
    PrepareForRegisterAllocation(graph_, *compiler_options_).Run();
  }
};

TEST_F(PrepareForRegisterAllocationTest, MergeConditionToSelect) {
  HBasicBlock* ret = InitEntryMainExitGraphWithReturnVoid();

  HInstruction* param = MakeParam(DataType::Type::kInt32);
  HInstruction* zero_const = graph_->GetIntConstant(0);
  HCondition* condition = MakeCondition(ret, kCondLT, param, zero_const);
  HSelect* select = MakeSelect(ret, condition, zero_const, param);

  RunPass();

  ASSERT_TRUE(condition->IsEmittedAtUseSite());
  ASSERT_EQ(condition->GetNext(), select);
}

TEST_F(PrepareForRegisterAllocationTest, MergeConditionToDeoptimize) {
  HBasicBlock* ret = InitEntryMainExitGraphWithReturnVoid();

  HInstruction* param = MakeParam(DataType::Type::kInt32);
  HInstruction* zero_const = graph_->GetIntConstant(0);
  HCondition* condition = MakeCondition(ret, kCondLT, param, zero_const);
  HDeoptimize* deopt = new (GetAllocator()) HDeoptimize(
      GetAllocator(), condition, DeoptimizationKind::kAotInlineCache, /*dex_pc=*/ 0u);
  AddOrInsertInstruction(ret, deopt);

  RunPass();

  ASSERT_TRUE(condition->IsEmittedAtUseSite());
  ASSERT_EQ(condition->GetNext(), deopt);
}

TEST_F(PrepareForRegisterAllocationTest, MergeConditionToIf) {
  HBasicBlock* ret = InitEntryMainExitGraphWithReturnVoid();
  auto [start, left, right] = CreateDiamondPattern(ret);

  HInstruction* param = MakeParam(DataType::Type::kInt32);
  HInstruction* zero_const = graph_->GetIntConstant(0);
  HCondition* condition = MakeCondition(start, kCondLT, param, zero_const);
  HIf* start_if = MakeIf(start, condition);

  RunPass();

  ASSERT_TRUE(condition->IsEmittedAtUseSite());
  ASSERT_EQ(condition->GetNext(), start_if);
}

TEST_F(PrepareForRegisterAllocationTest, MergeConditionToIfWithMove) {
  HBasicBlock* ret = InitEntryMainExitGraphWithReturnVoid();
  auto [start, left, right] = CreateDiamondPattern(ret);

  HInstruction* param = MakeParam(DataType::Type::kInt32);
  HInstruction* zero_const = graph_->GetIntConstant(0);
  HCondition* condition = MakeCondition(start, kCondLT, param, zero_const);
  HInstruction* add = MakeBinOp<HAdd>(start, DataType::Type::kInt32, param, param);
  HIf* start_if = MakeIf(start, condition);

  ASSERT_EQ(condition->GetNext(), add);
  ASSERT_EQ(add->GetNext(), start_if);

  RunPass();

  ASSERT_TRUE(condition->IsEmittedAtUseSite());
  ASSERT_EQ(add->GetNext(), condition);
  ASSERT_EQ(condition->GetNext(), start_if);
}

TEST_F(PrepareForRegisterAllocationTest, MergeConditionToIfWithMoveFromPredecessor) {
  HBasicBlock* ret = InitEntryMainExitGraphWithReturnVoid();
  auto [start, left, right_end] = CreateDiamondPattern(ret);
  auto [right_start, right_left, right_right] = CreateDiamondPattern(right_end);

  HInstruction* cond_param = MakeParam(DataType::Type::kBool);
  HInstruction* param = MakeParam(DataType::Type::kInt32);
  HInstruction* zero_const = graph_->GetIntConstant(0);
  HCondition* condition = MakeCondition(start, kCondLT, param, zero_const);
  MakeIf(start, cond_param);
  // Note: The condition for this `HIf` is in the predecessor block.
  HIf* right_start_if = MakeIf(right_start, condition);

  ASSERT_NE(condition->GetBlock(), right_start_if->GetBlock());

  RunPass();

  ASSERT_TRUE(condition->IsEmittedAtUseSite());
  ASSERT_EQ(condition->GetBlock(), right_start_if->GetBlock());
  ASSERT_EQ(condition->GetNext(), right_start_if);
}

TEST_F(PrepareForRegisterAllocationTest, MergeConditionPreventedByOtherUse) {
  HBasicBlock* ret = InitEntryMainExitGraphWithReturnVoid();
  auto [start, left, right] = CreateDiamondPattern(ret);

  HInstruction* param = MakeParam(DataType::Type::kInt32);
  HInstruction* zero_const = graph_->GetIntConstant(0);
  HCondition* condition = MakeCondition(start, kCondLT, param, zero_const);
  HIf* start_if = MakeIf(start, condition);

  // Other use.
  MakeBinOp<HAdd>(ret, DataType::Type::kInt32, param, condition);

  RunPass();

  ASSERT_TRUE(!condition->IsEmittedAtUseSite());
  ASSERT_EQ(condition->GetNext(), start_if);
}

TEST_F(PrepareForRegisterAllocationTest, MergeConditionPreventedByEnvUse) {
  HBasicBlock* ret = InitEntryMainExitGraphWithReturnVoid();
  auto [start, left, right] = CreateDiamondPattern(ret);

  HInstruction* param = MakeParam(DataType::Type::kInt32);
  HInstruction* zero_const = graph_->GetIntConstant(0);
  HCondition* condition = MakeCondition(start, kCondLT, param, zero_const);
  HIf* start_if = MakeIf(start, condition);

  // Environment use.
  MakeInvokeStatic(ret, DataType::Type::kVoid, /*args=*/ {}, /*env=*/ {condition});

  RunPass();

  ASSERT_TRUE(!condition->IsEmittedAtUseSite());
  ASSERT_EQ(condition->GetNext(), start_if);
}

TEST_F(PrepareForRegisterAllocationTest, MergeConditionPrevented_RefNoEnvInBlock) {
  ScopedObjectAccess soa(Thread::Current());
  VariableSizedHandleScope vshs(soa.Self());
  HBasicBlock* ret = InitEntryMainExitGraphWithReturnVoid(&vshs);
  auto [start, left, right_end] = CreateDiamondPattern(ret);
  auto [right_start, right_left, right_right] = CreateDiamondPattern(right_end);

  HInstruction* cond_param = MakeParam(DataType::Type::kBool);
  HInstruction* param = MakeParam(DataType::Type::kReference);
  HInstruction* null_const = graph_->GetNullConstant();
  HCondition* condition = MakeCondition(start, kCondEQ, param, null_const);
  MakeIf(start, cond_param);
  // Note: The condition for this `HIf` is in the predecessor block.
  HIf* right_start_if = MakeIf(right_start, condition);

  RunPass();

  ASSERT_TRUE(!condition->IsEmittedAtUseSite());
  ASSERT_NE(condition->GetBlock(), right_start_if->GetBlock());  // Not moved to the `HIf`.
}

TEST_F(PrepareForRegisterAllocationTest, MergeCondition_RefsInEnv) {
  ScopedObjectAccess soa(Thread::Current());
  VariableSizedHandleScope vshs(soa.Self());
  HBasicBlock* ret = InitEntryMainExitGraphWithReturnVoid(&vshs);
  auto [start, left, right_end] = CreateDiamondPattern(ret);

  HInstruction* param1 = MakeParam(DataType::Type::kReference);
  HInstruction* param2 = MakeParam(DataType::Type::kReference);
  HCondition* condition = MakeCondition(start, kCondEQ, param1, param2);

  // This invoke's environment already contains `param1` and `param2`, so reordering
  // the `condition` after the invoke would not extend their lifetime for the purpose of GC.
  HInvoke* invoke =
      MakeInvokeStatic(start, DataType::Type::kVoid, /*args=*/ {}, /*env=*/ {param1, param2});

  HIf* start_if = MakeIf(start, condition);

  ASSERT_EQ(condition->GetNext(), invoke);
  ASSERT_EQ(invoke->GetNext(), start_if);

  RunPass();

  ASSERT_TRUE(condition->IsEmittedAtUseSite());
  ASSERT_EQ(invoke->GetNext(), condition);
  ASSERT_EQ(condition->GetNext(), start_if);
}

TEST_F(PrepareForRegisterAllocationTest, MergeCondition_RefLhsInEnv) {
  ScopedObjectAccess soa(Thread::Current());
  VariableSizedHandleScope vshs(soa.Self());
  HBasicBlock* ret = InitEntryMainExitGraphWithReturnVoid(&vshs);
  auto [start, left, right_end] = CreateDiamondPattern(ret);

  HInstruction* param = MakeParam(DataType::Type::kReference);
  HInstruction* null_const = graph_->GetNullConstant();
  HCondition* condition = MakeCondition(start, kCondEQ, param, null_const);

  // This invoke's environment already contains `param`, so reordering the `condition`
  // after the invoke would not extend its lifetime for the purpose of GC.
  HInvoke* invoke = MakeInvokeStatic(start, DataType::Type::kVoid, /*args=*/ {}, /*env=*/ {param});

  HIf* start_if = MakeIf(start, condition);

  ASSERT_EQ(condition->GetNext(), invoke);
  ASSERT_EQ(invoke->GetNext(), start_if);

  RunPass();

  ASSERT_TRUE(condition->IsEmittedAtUseSite());
  ASSERT_EQ(invoke->GetNext(), condition);
  ASSERT_EQ(condition->GetNext(), start_if);
}

TEST_F(PrepareForRegisterAllocationTest, MergeCondition_RefRhsInEnv) {
  ScopedObjectAccess soa(Thread::Current());
  VariableSizedHandleScope vshs(soa.Self());
  HBasicBlock* ret = InitEntryMainExitGraphWithReturnVoid(&vshs);
  auto [start, left, right_end] = CreateDiamondPattern(ret);

  HInstruction* param = MakeParam(DataType::Type::kReference);
  HInstruction* null_const = graph_->GetNullConstant();
  HCondition* condition = MakeCondition(start, kCondEQ, null_const, param);

  // This invoke's environment already contains `param`, so reordering the `condition`
  // after the invoke would not extend its lifetime for the purpose of GC.
  HInvoke* invoke = MakeInvokeStatic(start, DataType::Type::kVoid, /*args=*/ {}, /*env=*/ {param});

  HIf* start_if = MakeIf(start, condition);

  ASSERT_EQ(condition->GetNext(), invoke);
  ASSERT_EQ(invoke->GetNext(), start_if);

  RunPass();

  ASSERT_TRUE(condition->IsEmittedAtUseSite());
  ASSERT_EQ(invoke->GetNext(), condition);
  ASSERT_EQ(condition->GetNext(), start_if);
}

TEST_F(PrepareForRegisterAllocationTest, MergeConditionPrevented_RefLhsNotInEnv) {
  ScopedObjectAccess soa(Thread::Current());
  VariableSizedHandleScope vshs(soa.Self());
  HBasicBlock* ret = InitEntryMainExitGraphWithReturnVoid(&vshs);
  auto [start, left, right_end] = CreateDiamondPattern(ret);

  HInstruction* param1 = MakeParam(DataType::Type::kReference);
  HInstruction* param2 = MakeParam(DataType::Type::kReference);
  HCondition* condition = MakeCondition(start, kCondEQ, param1, param2);

  // This invoke's environment does not contain `param1`, so reordering the `condition`
  // after the invoke would need to extend the lifetime of `param1` for the purpose of GC.
  // We do not want to extend lifetime of references, therefore the optimization is skipped.
  HInvoke* invoke = MakeInvokeStatic(start, DataType::Type::kVoid, /*args=*/ {}, /*env=*/ {param2});

  HIf* start_if = MakeIf(start, condition);

  ASSERT_EQ(condition->GetNext(), invoke);
  ASSERT_EQ(invoke->GetNext(), start_if);

  RunPass();

  ASSERT_TRUE(!condition->IsEmittedAtUseSite());
  ASSERT_EQ(condition->GetNext(), invoke);
  ASSERT_EQ(invoke->GetNext(), start_if);
}

TEST_F(PrepareForRegisterAllocationTest, MergeConditionPrevented_RefRhsNotInEnv) {
  ScopedObjectAccess soa(Thread::Current());
  VariableSizedHandleScope vshs(soa.Self());
  HBasicBlock* ret = InitEntryMainExitGraphWithReturnVoid(&vshs);
  auto [start, left, right_end] = CreateDiamondPattern(ret);

  HInstruction* param1 = MakeParam(DataType::Type::kReference);
  HInstruction* param2 = MakeParam(DataType::Type::kReference);
  HCondition* condition = MakeCondition(start, kCondEQ, param1, param2);

  // This invoke's environment does not contain `param2`, so reordering the `condition`
  // after the invoke would need to extend the lifetime of `param2` for the purpose of GC.
  // We do not want to extend lifetime of references, therefore the optimization is skipped.
  HInvoke* invoke = MakeInvokeStatic(start, DataType::Type::kVoid, /*args=*/ {}, /*env=*/ {param1});

  HIf* start_if = MakeIf(start, condition);

  ASSERT_EQ(condition->GetNext(), invoke);
  ASSERT_EQ(invoke->GetNext(), start_if);

  RunPass();

  ASSERT_TRUE(!condition->IsEmittedAtUseSite());
  ASSERT_EQ(condition->GetNext(), invoke);
  ASSERT_EQ(invoke->GetNext(), start_if);
}

}  // namespace art

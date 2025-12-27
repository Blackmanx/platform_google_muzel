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

#include "instruction_simplifier_arm64.h"

#include <gtest/gtest.h>

#include "base/globals.h"
#include "optimizing/nodes.h"
#include "optimizing_unit_test.h"

namespace art HIDDEN {
namespace arm64 {

class InstructionSimplifierArm64Test : public OptimizingUnitTest {};

// Regression test for a bug in simplification of `Sub(., Sub(Shl(...), .))` to enable shifter
// operand merging. The code wrongly assumed that the `Shl` can have only one non-environment
// use. It is the inner `Sub` that can have only one non-environment use.
TEST_F(InstructionSimplifierArm64Test, SubRightSubLeftShlRegressionTest) {
  HBasicBlock* return_block = InitEntryMainExitGraph();

  HInstruction* param0 = MakeParam(DataType::Type::kInt32);
  HInstruction* param1 = MakeParam(DataType::Type::kInt32);
  HInstruction* param2 = MakeParam(DataType::Type::kInt32);
  HInstruction* param3 = MakeParam(DataType::Type::kInt32);
  HInstruction* param4 = MakeParam(DataType::Type::kInt32);
  HInstruction* param5 = MakeParam(DataType::Type::kInt32);
  HInstruction* param6 = MakeParam(DataType::Type::kInt32);
  HInstruction* bool_param = MakeParam(DataType::Type::kBool);
  HInstruction* const1 = graph_->GetIntConstant(1);

  auto [start, left, right] = CreateDiamondPattern(return_block, bool_param);

  // When visiting the `sub2`, we exchange the `sub1` operands and replace `sub2` with an `Add`.
  // We used to crash if `shl` had more than one use (the `Sub` with exchanged operands) but we
  // fixed this and we test with additional uses both before and after the replaced `sub2`,
  // inside and outside the block. The `shl` is then merged as a shifter operand to all its uses.

  HShl* shl = MakeBinOp<HShl>(start, DataType::Type::kInt32, param0, const1);
  HAdd* add1 = MakeBinOp<HAdd>(start, DataType::Type::kInt32, param1, shl);

  HAdd* add2 = MakeBinOp<HAdd>(right, DataType::Type::kInt32, param2, shl);
  HSub* sub1 = MakeBinOp<HSub>(right, DataType::Type::kInt32, shl, param3);
  HSub* sub2 = MakeBinOp<HSub>(right, DataType::Type::kInt32, param4, sub1);
  HAdd* add3 = MakeBinOp<HAdd>(right, DataType::Type::kInt32, param5, shl);
  HOr* or1 = MakeBinOp<HOr>(right, DataType::Type::kInt32, add2, sub2);
  HOr* or2 = MakeBinOp<HOr>(right, DataType::Type::kInt32, or1, add3);

  HPhi* phi = MakePhi(return_block, {add1, or2});
  HAdd* add4 = MakeBinOp<HAdd>(return_block, DataType::Type::kInt32, param6, shl);
  HOr* or3 = MakeBinOp<HOr>(return_block, DataType::Type::kInt32, phi, add4);
  MakeReturn(return_block, or3);

  graph_->BuildDominatorTree();
  InstructionSimplifierArm64 simplifier(graph_, /*codegen=*/ nullptr, /*stats=*/ nullptr);
  simplifier.Run();

  EXPECT_FALSE(shl->IsInBlock());
  EXPECT_FALSE(add1->IsInBlock());
  EXPECT_FALSE(add2->IsInBlock());
  EXPECT_FALSE(sub1->IsInBlock());
  EXPECT_FALSE(sub2->IsInBlock());
  EXPECT_FALSE(add3->IsInBlock());
  EXPECT_TRUE(or1->IsInBlock());
  EXPECT_TRUE(or2->IsInBlock());
  EXPECT_TRUE(phi->IsInBlock());
  EXPECT_FALSE(add4->IsInBlock());
  EXPECT_TRUE(or3->IsInBlock());
}

}  // namespace arm64
}  // namespace art

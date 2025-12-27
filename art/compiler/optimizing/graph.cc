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

#include "graph.h"

#include "base/arena_bit_vector.h"
#include "base/logging.h"
#include "base/scoped_arena_containers.h"
#include "common_dominator.h"
#include "graph_visualizer.h"
#include "loop_information-inl.h"
#include "nodes.h"
#include "scoped_thread_state_change-inl.h"

namespace art HIDDEN {

void HGraph::AddBlock(HBasicBlock* block) {
  block->SetBlockId(blocks_.size());
  blocks_.push_back(block);
}

int32_t HGraph::AllocateInstructionId() {
  CHECK_NE(current_instruction_id_, INT32_MAX);
  return current_instruction_id_++;
}

size_t HGraph::CountNumberOfInstructions() {
  size_t number_of_instructions = 0;
  for (HBasicBlock* block : GetReversePostOrderSkipEntryBlock()) {
    for (HInstructionIteratorPrefetchNext instr_it(block->GetInstructions()); !instr_it.Done();
         instr_it.Advance()) {
      ++number_of_instructions;
    }
  }
  return number_of_instructions;
}

bool HGraph::HasMoreInstructionsThan(size_t limit) {
  size_t number_of_instructions = 0;
  for (HBasicBlock* block : GetReversePostOrderSkipEntryBlock()) {
    for (HInstructionIteratorPrefetchNext instr_it(block->GetInstructions()); !instr_it.Done();
         instr_it.Advance()) {
      ++number_of_instructions;
      if (number_of_instructions > limit) {
        return true;
      }
    }
  }
  return false;
}

// Register a back edge; if the block was not a loop header before the call,
// associate a newly created loop info with it.
void AddBackEdge(HBasicBlock* block, HBasicBlock* back_edge) {
  if (block->GetLoopInformation() == nullptr) {
    HGraph* graph = block->GetGraph();
    block->SetLoopInformation(new (graph->GetAllocator()) HLoopInformation(block, graph));
  }
  DCHECK_EQ(block->GetLoopInformation()->GetHeader(), block);
  block->GetLoopInformation()->AddBackEdge(back_edge);
}

void HGraph::FindBackEdges(/*out*/ BitVectorView<size_t> visited) {
  // "visited" must be empty on entry, it's an output argument for all visited (i.e. live) blocks.
  DCHECK(!visited.IsAnyBitSet());

  // Allocate memory from local ScopedArenaAllocator.
  ScopedArenaAllocator allocator(GetArenaStack());
  // Nodes that we're currently visiting, indexed by block id.
  BitVectorView<size_t> visiting =
      ArenaBitVector::CreateFixedSize(&allocator, blocks_.size(), kArenaAllocGraphBuilder);
  // Number of successors visited from a given node, indexed by block id.
  ScopedArenaVector<size_t> successors_visited(blocks_.size(),
                                               0u,
                                               allocator.Adapter(kArenaAllocGraphBuilder));
  // Stack of nodes that we're currently visiting (same as marked in "visiting" above).
  ScopedArenaVector<HBasicBlock*> worklist(allocator.Adapter(kArenaAllocGraphBuilder));
  constexpr size_t kDefaultWorklistSize = 8;
  worklist.reserve(kDefaultWorklistSize);
  visited.SetBit(entry_block_->GetBlockId());
  visiting.SetBit(entry_block_->GetBlockId());
  worklist.push_back(entry_block_);

  while (!worklist.empty()) {
    HBasicBlock* current = worklist.back();
    uint32_t current_id = current->GetBlockId();
    if (successors_visited[current_id] == current->GetSuccessors().size()) {
      visiting.ClearBit(current_id);
      worklist.pop_back();
    } else {
      HBasicBlock* successor = current->GetSuccessors()[successors_visited[current_id]++];
      uint32_t successor_id = successor->GetBlockId();
      if (visiting.IsBitSet(successor_id)) {
        DCHECK(ContainsElement(worklist, successor));
        AddBackEdge(successor, current);
      } else if (!visited.IsBitSet(successor_id)) {
        visited.SetBit(successor_id);
        visiting.SetBit(successor_id);
        worklist.push_back(successor);
      }
    }
  }
}

void HGraph::RemoveDeadBlocksInstructionsAsUsersAndDisconnect(
    BitVectorView<const size_t> visited) const {
  for (size_t i = 0; i < blocks_.size(); ++i) {
    if (!visited.IsBitSet(i)) {
      HBasicBlock* block = blocks_[i];
      if (block == nullptr) continue;

      // Remove as user.
      for (HInstructionIteratorPrefetchNext it(block->GetPhis()); !it.Done(); it.Advance()) {
        it.Current()->RemoveAsUser();
      }
      for (HInstructionIteratorPrefetchNext it(block->GetInstructions()); !it.Done();
           it.Advance()) {
        it.Current()->RemoveAsUser();
      }

      // Remove non-catch phi uses, and disconnect the block.
      block->DisconnectFromSuccessors(visited);
    }
  }
}

void HGraph::RemoveDeadBlocks(BitVectorView<const size_t> visited) {
  DCHECK(reverse_post_order_.empty()) << "We shouldn't have dominance information.";
  for (size_t i = 0; i < blocks_.size(); ++i) {
    if (!visited.IsBitSet(i)) {
      HBasicBlock* block = blocks_[i];
      if (block == nullptr) continue;

      // Remove all remaining uses (which should be only catch phi uses), and the instructions.
      block->RemoveCatchPhiUsesAndInstruction(/* building_dominator_tree = */ true);

      // Remove the block from the list of blocks, so that further analyses
      // never see it.
      blocks_[i] = nullptr;
      if (IsExitBlock(block)) {
        SetExitBlock(nullptr);
      }
      // Mark the block as removed. This is used by the HGraphBuilder to discard
      // the block as a branch target.
      block->SetGraph(nullptr);
    }
  }
}

GraphAnalysisResult HGraph::BuildDominatorTree() {
  // Allocate memory from local ScopedArenaAllocator.
  ScopedArenaAllocator allocator(GetArenaStack());

  BitVectorView<size_t> visited =
      ArenaBitVector::CreateFixedSize(&allocator, blocks_.size(), kArenaAllocGraphBuilder);

  // (1) Find the back edges in the graph doing a DFS traversal.
  FindBackEdges(visited);

  // (2) Remove instructions and phis from blocks not visited during
  //     the initial DFS as users from other instructions, so that
  //     users can be safely removed before uses later.
  //     Also disconnect the block from its successors, updating the successor's phis if needed.
  RemoveDeadBlocksInstructionsAsUsersAndDisconnect(visited);

  // (3) Remove blocks not visited during the initial DFS.
  //     Step (5) requires dead blocks to be removed from the
  //     predecessors list of live blocks.
  RemoveDeadBlocks(visited);

  // (4) Simplify the CFG now, so that we don't need to recompute
  //     dominators and the reverse post order.
  SimplifyCFG();

  // (5) Compute the dominance information and the reverse post order.
  ComputeDominanceInformation();

  // (6) Analyze loops discovered through back edge analysis, and
  //     set the loop information on each block.
  GraphAnalysisResult result = AnalyzeLoops();
  if (result != kAnalysisSuccess) {
    return result;
  }

  // (7) Precompute per-block try membership before entering the SSA builder,
  //     which needs the information to build catch block phis from values of
  //     locals at throwing instructions inside try blocks.
  ComputeTryBlockInformation();

  return kAnalysisSuccess;
}

GraphAnalysisResult HGraph::RecomputeDominatorTree() {
  DCHECK(!HasIrreducibleLoops()) << "Recomputing loop information in graphs with irreducible loops "
                                 << "is unsupported, as it could lead to loop header changes";
  ClearLoopInformation();
  ClearDominanceInformation();
  return BuildDominatorTree();
}

void HGraph::ClearDominanceInformation() {
  for (HBasicBlock* block : GetActiveBlocks()) {
    block->ClearDominanceInformation();
  }
  reverse_post_order_.clear();
}

void HGraph::ClearLoopInformation() {
  SetHasLoops(false);
  SetHasIrreducibleLoops(false);
  for (HBasicBlock* block : GetActiveBlocks()) {
    block->SetLoopInformation(nullptr);
  }
}

static bool UpdateDominatorOfSuccessor(HBasicBlock* block, HBasicBlock* successor) {
  DCHECK(ContainsElement(block->GetSuccessors(), successor));

  HBasicBlock* old_dominator = successor->GetDominator();
  HBasicBlock* new_dominator =
      (old_dominator == nullptr) ? block
                                 : CommonDominator::ForPair(old_dominator, block);

  if (old_dominator == new_dominator) {
    return false;
  } else {
    successor->SetDominator(new_dominator);
    return true;
  }
}

void HGraph::ComputeDominanceInformation() {
  DCHECK(reverse_post_order_.empty());
  const size_t size = blocks_.size();
  reverse_post_order_.reserve(size);
  reverse_post_order_.push_back(entry_block_);

  {
    // Allocate memory from local ScopedArenaAllocator.
    ScopedArenaAllocator allocator(GetArenaStack());
    // Number of visits of a given node, indexed by block id.
    ScopedArenaVector<size_t> visits(size, 0u, allocator.Adapter(kArenaAllocGraphBuilder));
    // Number of successors visited from a given node, indexed by block id.
    ScopedArenaVector<size_t> successors_visited(
        size, 0u, allocator.Adapter(kArenaAllocGraphBuilder));
    // Nodes for which we need to visit successors.
    ScopedArenaVector<HBasicBlock*> worklist(allocator.Adapter(kArenaAllocGraphBuilder));
    worklist.reserve(size);
    worklist.push_back(entry_block_);

    // Cached for the check below.
    HBasicBlock* exit = GetExitBlock();

    while (!worklist.empty()) {
      HBasicBlock* current = worklist.back();
      uint32_t current_id = current->GetBlockId();
      DCHECK_LT(successors_visited[current_id], current->GetSuccessors().size());
      HBasicBlock* successor = current->GetSuccessors()[successors_visited[current_id]++];
      if (successors_visited[current_id] == current->GetSuccessors().size()) {
        worklist.pop_back();
      }
      UpdateDominatorOfSuccessor(current, successor);

      // Once all the forward edges have been visited, we know the immediate
      // dominator of the block. We can then start visiting its successors.
      size_t successor_visits_needed =
          successor->GetPredecessors().size() -
          (successor->IsLoopHeader() ? successor->GetLoopInformation()->NumberOfBackEdges() : 0u);
      if (++visits[successor->GetBlockId()] == successor_visits_needed) {
        reverse_post_order_.push_back(successor);
        // The exit block is the only one with no successors. Will be encountered only one time per
        // graph, at the end.
        if (LIKELY(successor != exit)) {
          worklist.push_back(successor);
        }
      }
    }
  }

  // Check if the graph has back edges not dominated by their respective headers.
  // If so, we need to update the dominators of those headers and recursively of
  // their successors. We do that with a fix-point iteration over all blocks.
  // The algorithm is guaranteed to terminate because it loops only if the sum
  // of all dominator chains has decreased in the current iteration.
  bool must_run_fix_point = false;
  for (HBasicBlock* block : blocks_) {
    if (block != nullptr &&
        block->IsLoopHeader() &&
        block->GetLoopInformation()->HasBackEdgeNotDominatedByHeader()) {
      must_run_fix_point = true;
      break;
    }
  }
  if (must_run_fix_point) {
    bool update_occurred = true;
    while (update_occurred) {
      update_occurred = false;
      for (HBasicBlock* block : GetReversePostOrder()) {
        for (HBasicBlock* successor : block->GetSuccessors()) {
          update_occurred |= UpdateDominatorOfSuccessor(block, successor);
        }
      }
    }
  }

  // Make sure that there are no remaining blocks whose dominator information
  // needs to be updated.
  if (kIsDebugBuild) {
    for (HBasicBlock* block : GetReversePostOrder()) {
      for (HBasicBlock* successor : block->GetSuccessors()) {
        DCHECK(!UpdateDominatorOfSuccessor(block, successor));
      }
    }
  }

  // Populate `dominated_blocks_` information after computing all dominators.
  // The potential presence of irreducible loops requires to do it after.
  for (HBasicBlock* block : GetReversePostOrder()) {
    if (!IsEntryBlock(block)) {
      block->GetDominator()->AddDominatedBlock(block);
    }
  }
}

HBasicBlock* HGraph::SplitEdge(HBasicBlock* block, HBasicBlock* successor) {
  HBasicBlock* new_block = HBasicBlock::Create(allocator_, this, successor->GetDexPc());
  AddBlock(new_block);
  // Use `InsertBetween` to ensure the predecessor index and successor index of
  // `block` and `successor` are preserved.
  new_block->InsertBetween(block, successor);
  return new_block;
}

void HGraph::SplitCriticalEdge(HBasicBlock* block, HBasicBlock* successor) {
  // Insert a new node between `block` and `successor` to split the
  // critical edge.
  HBasicBlock* new_block = SplitEdge(block, successor);
  new_block->AddInstruction(new (allocator_) HGoto(successor->GetDexPc()));
  if (successor->IsLoopHeader()) {
    // If we split at a back edge boundary, make the new block the back edge.
    HLoopInformation* info = successor->GetLoopInformation();
    if (info->IsBackEdge(*block)) {
      info->RemoveBackEdge(block);
      info->AddBackEdge(new_block);
    }
  }
}

// Reorder phi inputs to match reordering of the block's predecessors.
static void FixPhisAfterPredecessorsReodering(HBasicBlock* block, size_t first, size_t second) {
  for (HInstructionIteratorPrefetchNext it(block->GetPhis()); !it.Done(); it.Advance()) {
    HPhi* phi = it.Current()->AsPhi();
    HInstruction* first_instr = phi->InputAt(first);
    HInstruction* second_instr = phi->InputAt(second);
    phi->ReplaceInput(first_instr, second);
    phi->ReplaceInput(second_instr, first);
  }
}

// Make sure that the first predecessor of a loop header is the incoming block.
void HGraph::OrderLoopHeaderPredecessors(HBasicBlock* header) {
  DCHECK(header->IsLoopHeader());
  HLoopInformation* info = header->GetLoopInformation();
  if (info->IsBackEdge(*header->GetPredecessors()[0])) {
    HBasicBlock* to_swap = header->GetPredecessors()[0];
    for (size_t pred = 1, e = header->GetPredecessors().size(); pred < e; ++pred) {
      HBasicBlock* predecessor = header->GetPredecessors()[pred];
      if (!info->IsBackEdge(*predecessor)) {
        header->predecessors_[pred] = to_swap;
        header->predecessors_[0] = predecessor;
        FixPhisAfterPredecessorsReodering(header, 0, pred);
        break;
      }
    }
  }
}

// Transform control flow of the loop to a single preheader format (don't touch the data flow).
// New_preheader can be already among the header predecessors - this situation will be correctly
// processed.
static void FixControlForNewSinglePreheader(HBasicBlock* header, HBasicBlock* new_preheader) {
  HLoopInformation* loop_info = header->GetLoopInformation();
  for (size_t pred = 0; pred < header->GetPredecessors().size(); ++pred) {
    HBasicBlock* predecessor = header->GetPredecessors()[pred];
    if (!loop_info->IsBackEdge(*predecessor) && predecessor != new_preheader) {
      predecessor->ReplaceSuccessor(header, new_preheader);
      pred--;
    }
  }
}

//             == Before ==                                               == After ==
//      _________         _________                               _________         _________
//     | B0      |       | B1      |      (old preheaders)       | B0      |       | B1      |
//     |=========|       |=========|                             |=========|       |=========|
//     | i0 = .. |       | i1 = .. |                             | i0 = .. |       | i1 = .. |
//     |_________|       |_________|                             |_________|       |_________|
//           \               /                                         \              /
//            \             /                                        ___v____________v___
//             \           /               (new preheader)          | B20 <- B0, B1      |
//              |         |                                         |====================|
//              |         |                                         | i20 = phi(i0, i1)  |
//              |         |                                         |____________________|
//              |         |                                                   |
//    /\        |         |        /\                           /\            |              /\
//   /  v_______v_________v_______v  \                         /  v___________v_____________v  \
//  |  | B10 <- B0, B1, B2, B3     |  |                       |  | B10 <- B20, B2, B3        |  |
//  |  |===========================|  |       (header)        |  |===========================|  |
//  |  | i10 = phi(i0, i1, i2, i3) |  |                       |  | i10 = phi(i20, i2, i3)    |  |
//  |  |___________________________|  |                       |  |___________________________|  |
//  |        /               \        |                       |        /               \        |
//  |      ...              ...       |                       |      ...              ...       |
//  |   _________         _________   |                       |   _________         _________   |
//  |  | B2      |       | B3      |  |                       |  | B2      |       | B3      |  |
//  |  |=========|       |=========|  |     (back edges)      |  |=========|       |=========|  |
//  |  | i2 = .. |       | i3 = .. |  |                       |  | i2 = .. |       | i3 = .. |  |
//  |  |_________|       |_________|  |                       |  |_________|       |_________|  |
//   \     /                   \     /                         \     /                   \     /
//    \___/                     \___/                           \___/                     \___/
//
void HGraph::TransformLoopToSinglePreheaderFormat(HBasicBlock* header) {
  HLoopInformation* loop_info = header->GetLoopInformation();

  HBasicBlock* preheader = HBasicBlock::Create(allocator_, this, header->GetDexPc());
  AddBlock(preheader);
  preheader->AddInstruction(new (allocator_) HGoto(header->GetDexPc()));

  // If the old header has no Phis then we only need to fix the control flow.
  if (header->GetPhis().IsEmpty()) {
    FixControlForNewSinglePreheader(header, preheader);
    preheader->AddSuccessor(header);
    return;
  }

  // Find the first non-back edge block in the header's predecessors list.
  size_t first_nonbackedge_pred_pos = 0;
  bool found = false;
  for (size_t pred = 0; pred < header->GetPredecessors().size(); ++pred) {
    HBasicBlock* predecessor = header->GetPredecessors()[pred];
    if (!loop_info->IsBackEdge(*predecessor)) {
      first_nonbackedge_pred_pos = pred;
      found = true;
      break;
    }
  }

  DCHECK(found);

  // Fix the data-flow.
  for (HInstructionIteratorPrefetchNext it(header->GetPhis()); !it.Done(); it.Advance()) {
    HPhi* header_phi = it.Current()->AsPhi();

    HPhi* preheader_phi = new (GetAllocator()) HPhi(GetAllocator(),
                                                    header_phi->GetRegNumber(),
                                                    0,
                                                    header_phi->GetType());
    if (header_phi->GetType() == DataType::Type::kReference) {
      preheader_phi->SetReferenceTypeInfoIfValid(header_phi->GetReferenceTypeInfo());
    }
    preheader->AddPhi(preheader_phi);

    HInstruction* orig_input = header_phi->InputAt(first_nonbackedge_pred_pos);
    header_phi->ReplaceInput(preheader_phi, first_nonbackedge_pred_pos);
    preheader_phi->AddInput(orig_input);

    for (size_t input_pos = first_nonbackedge_pred_pos + 1;
         input_pos < header_phi->InputCount();
         input_pos++) {
      HInstruction* input = header_phi->InputAt(input_pos);
      HBasicBlock* pred_block = header->GetPredecessors()[input_pos];

      if (loop_info->Contains(*pred_block)) {
        DCHECK(loop_info->IsBackEdge(*pred_block));
      } else {
        preheader_phi->AddInput(input);
        header_phi->RemoveInputAt(input_pos);
        input_pos--;
      }
    }
  }

  // Fix the control-flow.
  HBasicBlock* first_pred = header->GetPredecessors()[first_nonbackedge_pred_pos];
  preheader->InsertBetween(first_pred, header);

  FixControlForNewSinglePreheader(header, preheader);
}

void HGraph::SimplifyLoop(HBasicBlock* header) {
  HLoopInformation* info = header->GetLoopInformation();

  // Make sure the loop has only one pre header. This simplifies SSA building by having
  // to just look at the pre header to know which locals are initialized at entry of the
  // loop. Also, don't allow the entry block to be a pre header: this simplifies inlining
  // this graph.
  size_t number_of_incomings = header->GetPredecessors().size() - info->NumberOfBackEdges();
  if (number_of_incomings != 1 || (GetEntryBlock()->GetSingleSuccessor() == header)) {
    TransformLoopToSinglePreheaderFormat(header);
  }

  OrderLoopHeaderPredecessors(header);

  HInstruction* first_instruction = header->GetFirstInstruction();
  if (first_instruction != nullptr && first_instruction->IsSuspendCheck()) {
    // Called from DeadBlockElimination. Update SuspendCheck pointer.
    info->SetSuspendCheck(first_instruction->AsSuspendCheck());
  }
}

void HGraph::ComputeTryBlockInformation() {
  // Iterate in reverse post order to propagate try membership information from
  // predecessors to their successors.
  bool graph_has_try_catch = false;

  for (HBasicBlock* block : GetReversePostOrder()) {
    if (IsEntryBlock(block) || block->IsCatchBlock()) {
      // Catch blocks after simplification have only exceptional predecessors
      // and hence are never in tries.
      continue;
    }

    // Infer try membership from the first predecessor. Having simplified loops,
    // the first predecessor can never be a back edge and therefore it must have
    // been visited already and had its try membership set.
    HBasicBlock* first_predecessor = block->GetPredecessors()[0];
    DCHECK_IMPLIES(block->IsLoopHeader(),
                   !block->GetLoopInformation()->IsBackEdge(*first_predecessor));
    const HTryBoundary* try_entry = first_predecessor->ComputeTryEntryOfSuccessors();
    graph_has_try_catch |= try_entry != nullptr;
    if (try_entry != nullptr &&
        (block->GetTryCatchInformation() == nullptr ||
         try_entry != &block->GetTryCatchInformation()->GetTryEntry())) {
      // We are either setting try block membership for the first time or it
      // has changed.
      block->SetTryCatchInformation(new (allocator_) TryCatchInformation(*try_entry));
    }
  }

  SetHasTryCatch(graph_has_try_catch);
}

void HGraph::SimplifyCFG() {
// Simplify the CFG for future analysis, and code generation:
  // (1): Split critical edges.
  // (2): Simplify loops by having only one preheader.
  // NOTE: We're appending new blocks inside the loop, so we need to use index because iterators
  // can be invalidated. We remember the initial size to avoid iterating over the new blocks.
  for (size_t block_id = 0u, end = blocks_.size(); block_id != end; ++block_id) {
    HBasicBlock* block = blocks_[block_id];
    if (block == nullptr) continue;
    if (block->GetSuccessors().size() > 1) {
      // Only split normal-flow edges. We cannot split exceptional edges as they
      // are synthesized (approximate real control flow), and we do not need to
      // anyway. Moves that would be inserted there are performed by the runtime.
      ArrayRef<HBasicBlock* const> normal_successors = block->GetNormalSuccessors();
      for (size_t j = 0, e = normal_successors.size(); j < e; ++j) {
        HBasicBlock* successor = normal_successors[j];
        DCHECK(!successor->IsCatchBlock());
        if (successor == exit_block_) {
          // (Throw/Return/ReturnVoid)->TryBoundary->Exit. Special case which we
          // do not want to split because Goto->Exit is not allowed.
          DCHECK(block->IsSingleTryBoundary());
        } else if (successor->GetPredecessors().size() > 1) {
          SplitCriticalEdge(block, successor);
          // SplitCriticalEdge could have invalidated the `normal_successors`
          // ArrayRef. We must re-acquire it.
          normal_successors = block->GetNormalSuccessors();
          DCHECK_EQ(normal_successors[j]->GetSingleSuccessor(), successor);
          DCHECK_EQ(e, normal_successors.size());
        }
      }
    }
    if (block->IsLoopHeader()) {
      SimplifyLoop(block);
    } else if (!IsEntryBlock(block) &&
               block->GetFirstInstruction() != nullptr &&
               block->GetFirstInstruction()->IsSuspendCheck()) {
      // We are being called by the dead code elimiation pass, and what used to be
      // a loop got dismantled. Just remove the suspend check.
      block->RemoveInstruction(block->GetFirstInstruction());
    }
  }
}

GraphAnalysisResult HGraph::AnalyzeLoops() const {
  // We iterate post order to ensure we visit inner loops before outer loops.
  // `PopulateRecursive` needs this guarantee to know whether a natural loop
  // contains an irreducible loop.
  for (HBasicBlock* block : GetPostOrder()) {
    if (block->IsLoopHeader()) {
      if (block->IsCatchBlock()) {
        // TODO: Dealing with exceptional back edges could be tricky because
        //       they only approximate the real control flow. Bail out for now.
        VLOG(compiler) << "Not compiled: Exceptional back edges";
        return kAnalysisFailThrowCatchLoop;
      }
      block->GetLoopInformation()->Populate();
    }
  }
  return kAnalysisSuccess;
}

template <class InstructionType, typename ValueType>
InstructionType* HGraph::CreateConstant(ValueType value,
                                        ArenaSafeMap<ValueType, InstructionType*>* cache) {
  // Try to find an existing constant of the given value.
  InstructionType* constant = nullptr;
  auto cached_constant = cache->find(value);
  if (cached_constant != cache->end()) {
    constant = cached_constant->second;
  }

  // If not found or previously deleted, create and cache a new instruction.
  // Don't bother reviving a previously deleted instruction, for simplicity.
  if (constant == nullptr || constant->GetBlock() == nullptr) {
    constant = new (allocator_) InstructionType(value);
    cache->Overwrite(value, constant);
    InsertConstant(constant);
  }
  return constant;
}

void HGraph::InsertConstant(HConstant* constant) {
  // New constants are inserted before the SuspendCheck at the bottom of the
  // entry block. Note that this method can be called from the graph builder and
  // the entry block therefore may not end with SuspendCheck->Goto yet.
  HInstruction* insert_before = nullptr;

  HInstruction* gota = entry_block_->GetLastInstruction();
  if (gota != nullptr && gota->IsGoto()) {
    HInstruction* suspend_check = gota->GetPrevious();
    if (suspend_check != nullptr && suspend_check->IsSuspendCheck()) {
      insert_before = suspend_check;
    } else {
      insert_before = gota;
    }
  }

  if (insert_before == nullptr) {
    entry_block_->AddInstruction(constant);
  } else {
    entry_block_->InsertInstructionBefore(constant, insert_before);
  }
}

HNullConstant* HGraph::GetNullConstant() {
  // For simplicity, don't bother reviving the cached null constant if it is
  // not null and not in a block. Otherwise, we need to clear the instruction
  // id and/or any invariants the graph is assuming when adding new instructions.
  if ((cached_null_constant_ == nullptr) || (cached_null_constant_->GetBlock() == nullptr)) {
    cached_null_constant_ = new (allocator_) HNullConstant();
    cached_null_constant_->SetReferenceTypeInfo(GetInexactObjectRti());
    InsertConstant(cached_null_constant_);
  }
  if (kIsDebugBuild) {
    ScopedObjectAccess soa(Thread::Current());
    DCHECK(cached_null_constant_->GetReferenceTypeInfo().IsValid());
  }
  return cached_null_constant_;
}

HIntConstant* HGraph::GetIntConstant(int32_t value) {
  return CreateConstant(value, &cached_int_constants_);
}

HLongConstant* HGraph::GetLongConstant(int64_t value) {
  return CreateConstant(value, &cached_long_constants_);
}

HFloatConstant* HGraph::GetFloatConstant(float value) {
  return CreateConstant(bit_cast<int32_t, float>(value), &cached_float_constants_);
}

HDoubleConstant* HGraph::GetDoubleConstant(double value) {
  return CreateConstant(bit_cast<int64_t, double>(value), &cached_double_constants_);
}

HCurrentMethod* HGraph::GetCurrentMethod() {
  // For simplicity, don't bother reviving the cached current method if it is
  // not null and not in a block. Otherwise, we need to clear the instruction
  // id and/or any invariants the graph is assuming when adding new instructions.
  if ((cached_current_method_ == nullptr) || (cached_current_method_->GetBlock() == nullptr)) {
    cached_current_method_ = new (allocator_) HCurrentMethod(
        Is64BitInstructionSet(instruction_set_) ? DataType::Type::kInt64 : DataType::Type::kInt32,
        entry_block_->GetDexPc());
    if (entry_block_->GetFirstInstruction() == nullptr) {
      entry_block_->AddInstruction(cached_current_method_);
    } else {
      entry_block_->InsertInstructionBefore(
          cached_current_method_, entry_block_->GetFirstInstruction());
    }
  }
  return cached_current_method_;
}

const char* HGraph::GetMethodName() const {
  const dex::MethodId& method_id = dex_file_.GetMethodId(method_idx_);
  return dex_file_.GetMethodName(method_id);
}

std::string HGraph::PrettyMethod(bool with_signature) const {
  return dex_file_.PrettyMethod(method_idx_, with_signature);
}

HConstant* HGraph::GetConstant(DataType::Type type, int64_t value) {
  switch (type) {
    case DataType::Type::kBool:
      DCHECK(IsUint<1>(value));
      FALLTHROUGH_INTENDED;
    case DataType::Type::kUint8:
    case DataType::Type::kInt8:
    case DataType::Type::kUint16:
    case DataType::Type::kInt16:
    case DataType::Type::kInt32:
      DCHECK(IsInt(DataType::Size(type) * kBitsPerByte, value));
      return GetIntConstant(static_cast<int32_t>(value));

    case DataType::Type::kInt64:
      return GetLongConstant(value);

    default:
      LOG(FATAL) << "Unsupported constant type";
      UNREACHABLE();
  }
}

void HGraph::CacheFloatConstant(HFloatConstant* constant) {
  int32_t value = bit_cast<int32_t, float>(constant->GetValue());
  DCHECK(cached_float_constants_.find(value) == cached_float_constants_.end());
  cached_float_constants_.Overwrite(value, constant);
}

void HGraph::CacheDoubleConstant(HDoubleConstant* constant) {
  int64_t value = bit_cast<int64_t, double>(constant->GetValue());
  DCHECK(cached_double_constants_.find(value) == cached_double_constants_.end());
  cached_double_constants_.Overwrite(value, constant);
}

std::ostream& HGraph::Dump(std::ostream& os,
                           CodeGenerator* codegen,
                           std::optional<std::reference_wrapper<const BlockNamer>> namer) {
  HGraphVisualizer vis(&os, this, codegen, namer);
  vis.DumpGraphDebug();
  return os;
}

void HGraph::DeleteDeadEmptyBlock(HBasicBlock* block) {
  DCHECK_EQ(block->GetGraph(), this);
  DCHECK(block->GetSuccessors().empty());
  DCHECK(block->GetPredecessors().empty());
  DCHECK(block->GetDominatedBlocks().empty());
  DCHECK(block->GetDominator() == nullptr);
  DCHECK(block->GetInstructions().IsEmpty());
  DCHECK(block->GetPhis().IsEmpty());

  if (IsExitBlock(block)) {
    SetExitBlock(nullptr);
  }

  RemoveElement(reverse_post_order_, block);
  blocks_[block->GetBlockId()] = nullptr;
  block->SetGraph(nullptr);
}

void HGraph::UpdateLoopAndTryInformationOfNewBlock(HBasicBlock* block,
                                                   HBasicBlock* reference,
                                                   bool replace_if_back_edge,
                                                   bool has_more_specific_try_catch_info) {
  if (block->IsLoopHeader()) {
    // Clear the information of which blocks are contained in that loop. Since the
    // information is stored as a bit vector based on block ids, we have to update
    // it, as those block ids were specific to the callee graph and we are now adding
    // these blocks to the caller graph.
    block->GetLoopInformation()->ClearAllBlocks();
  }

  // If not already in a loop, update the loop information.
  if (!block->IsInLoop()) {
    block->SetLoopInformation(reference->GetLoopInformation());
  }

  // If the block is in a loop, update all its outward loops.
  HLoopInformation* loop_info = block->GetLoopInformation();
  if (loop_info != nullptr) {
    for (HLoopInformationOutwardIterator loop_it(*block);
         !loop_it.Done();
         loop_it.Advance()) {
      loop_it.Current()->Add(block);
    }
    if (replace_if_back_edge && loop_info->IsBackEdge(*reference)) {
      loop_info->ReplaceBackEdge(reference, block);
    }
  }

  DCHECK_IMPLIES(has_more_specific_try_catch_info, !reference->IsTryBlock())
      << "We don't allow to inline try catches inside of other try blocks.";

  // Update the TryCatchInformation, if we are not inlining a try catch.
  if (!has_more_specific_try_catch_info) {
    // Copy TryCatchInformation if `reference` is a try block, not if it is a catch block.
    TryCatchInformation* try_catch_info =
        reference->IsTryBlock() ? reference->GetTryCatchInformation() : nullptr;
    block->SetTryCatchInformation(try_catch_info);
  }
}

HInstruction* HGraph::InlineInto(HGraph* outer_graph, HInvoke* invoke) {
  DCHECK(HasExitBlock()) << "Unimplemented scenario";
  // Update the environments in this graph to have the invoke's environment
  // as parent.
  {
    // Skip the entry block, we do not need to update the entry's suspend check.
    for (HBasicBlock* block : GetReversePostOrderSkipEntryBlock()) {
      for (HInstructionIteratorPrefetchNext instr_it(block->GetInstructions());
           !instr_it.Done();
           instr_it.Advance()) {
        HInstruction* current = instr_it.Current();
        if (current->NeedsEnvironment()) {
          DCHECK(current->HasEnvironment());
          current->GetEnvironment()->SetAndCopyParentChain(
              outer_graph->GetAllocator(), invoke->GetEnvironment());
        }
      }
    }
  }

  if (HasBoundsChecks()) {
    outer_graph->SetHasBoundsChecks(true);
  }
  if (HasLoops()) {
    outer_graph->SetHasLoops(true);
  }
  if (HasIrreducibleLoops()) {
    outer_graph->SetHasIrreducibleLoops(true);
  }
  if (HasDirectCriticalNativeCall()) {
    outer_graph->SetHasDirectCriticalNativeCall(true);
  }
  if (HasTryCatch()) {
    outer_graph->SetHasTryCatch(true);
  }
  if (HasMonitorOperations()) {
    outer_graph->SetHasMonitorOperations(true);
  }
  if (HasTraditionalSIMD()) {
    outer_graph->SetHasTraditionalSIMD(true);
  }
  if (HasPredicatedSIMD()) {
    outer_graph->SetHasPredicatedSIMD(true);
  }
  if (HasAlwaysThrowingInvokes()) {
    outer_graph->SetHasAlwaysThrowingInvokes(true);
  }

  HInstruction* return_value = nullptr;
  if (GetBlocks().size() == 3) {
    // Simple case of an entry block, a body block, and an exit block.
    // Put the body block's instruction into `invoke`'s block.
    HBasicBlock* body = GetBlocks()[1];
    DCHECK(IsEntryBlock(GetBlocks()[0]));
    DCHECK(IsExitBlock(GetBlocks()[2]));
    DCHECK(!IsExitBlock(body));
    DCHECK(!body->IsInLoop());
    HInstruction* last = body->GetLastInstruction();

    // Note that we add instructions before the invoke only to simplify polymorphic inlining.
    invoke->GetBlock()->instructions_.AddBefore(invoke, body->GetInstructions());
    body->GetInstructions().SetBlockOfInstructions(invoke->GetBlock());

    // Replace the invoke with the return value of the inlined graph.
    if (last->IsReturn()) {
      return_value = last->InputAt(0);
    } else {
      // Inliner already made sure we don't inline methods that always throw.
      DCHECK(last->IsReturnVoid()) << *last;
    }

    invoke->GetBlock()->RemoveInstruction(last);
  } else {
    // Need to inline multiple blocks. We split `invoke`'s block
    // into two blocks, merge the first block of the inlined graph into
    // the first half, and replace the exit block of the inlined graph
    // with the second half.
    ArenaAllocator* allocator = outer_graph->GetAllocator();
    HBasicBlock* at = invoke->GetBlock();
    // Note that we split before the invoke only to simplify polymorphic inlining.
    HBasicBlock* to = at->SplitBeforeForInlining(invoke);

    HBasicBlock* first = entry_block_->GetSuccessors()[0];
    DCHECK(!first->IsInLoop());
    DCHECK(first->GetTryCatchInformation() == nullptr);
    at->MergeWithInlined(first);
    exit_block_->ReplaceWith(to);

    // Update the meta information surrounding blocks:
    // (1) the graph they are now in,
    // (2) the reverse post order of that graph,
    // (3) their potential loop information, inner and outer,
    // (4) try block membership.
    // Note that we do not need to update catch phi inputs because they
    // correspond to the register file of the outer method which the inlinee
    // cannot modify.

    // We don't add the entry block, the exit block, and the first block, which
    // has been merged with `at`.
    static constexpr int kNumberOfSkippedBlocksInCallee = 3;

    // We add the `to` block.
    static constexpr int kNumberOfNewBlocksInCaller = 1;
    size_t blocks_added = (reverse_post_order_.size() - kNumberOfSkippedBlocksInCallee)
        + kNumberOfNewBlocksInCaller;

    // Find the location of `at` in the outer graph's reverse post order. The new
    // blocks will be added after it.
    size_t index_of_at = IndexOfElement(outer_graph->reverse_post_order_, at);
    MakeRoomFor(&outer_graph->reverse_post_order_, blocks_added, index_of_at);

    // Do a reverse post order of the blocks in the callee and do (1), (2), (3)
    // and (4) to the blocks that apply.
    for (HBasicBlock* current : GetReversePostOrder()) {
      if (current != exit_block_ && current != entry_block_ && current != first) {
        DCHECK(current->GetGraph() == this);
        current->SetGraph(outer_graph);
        outer_graph->AddBlock(current);
        outer_graph->reverse_post_order_[++index_of_at] = current;
        UpdateLoopAndTryInformationOfNewBlock(current,
                                              at,
                                              /* replace_if_back_edge= */ false,
                                              current->GetTryCatchInformation() != nullptr);
      }
    }

    // Do (1), (2), (3) and (4) to `to`.
    to->SetGraph(outer_graph);
    outer_graph->AddBlock(to);
    outer_graph->reverse_post_order_[++index_of_at] = to;
    // Only `to` can become a back edge, as the inlined blocks
    // are predecessors of `to`.
    UpdateLoopAndTryInformationOfNewBlock(to, at, /* replace_if_back_edge= */ true);

    // Update all predecessors of the exit block (now the `to` block)
    // to not `HReturn` but `HGoto` instead. Special case throwing blocks
    // to now get the outer graph exit block as successor.
    HPhi* return_value_phi = nullptr;
    bool rerun_dominance = false;
    bool rerun_loop_analysis = false;
    for (size_t pred = 0; pred < to->GetPredecessors().size(); ++pred) {
      HBasicBlock* predecessor = to->GetPredecessors()[pred];
      HInstruction* last = predecessor->GetLastInstruction();

      // The whole method might end in a TryBoundary.
      const bool saw_try_boundary = last->IsTryBoundary();
      if (saw_try_boundary) {
        DCHECK(predecessor->IsSingleTryBoundary());
        DCHECK(!last->AsTryBoundary()->IsEntry());
        predecessor = predecessor->GetSinglePredecessor();
        last = predecessor->GetLastInstruction();
      }

      // At this point we might either have:
      // A) Return/ReturnVoid/Throw as the last instruction, or
      // B) AlwaysThrowingInstruction + Goto
      if (last->IsGoto() && last->GetPrevious() != nullptr) {
        last = last->GetPrevious();
        DCHECK(!last->IsThrow());
        DCHECK(last->AlwaysThrows());
      }

      if (last->AlwaysThrows()) {
        if (at->IsTryBlock()) {
          DCHECK(!saw_try_boundary) << "We don't support inlining of try blocks into try blocks.";
          // Create a TryBoundary of kind:exit and point it to the Exit block.
          HBasicBlock* new_block = outer_graph->SplitEdge(predecessor, to);
          new_block->AddInstruction(
              new (allocator) HTryBoundary(HTryBoundary::BoundaryKind::kExit, last->GetDexPc()));
          new_block->ReplaceSuccessor(to, outer_graph->GetExitBlock());

          // Copy information from the predecessor.
          new_block->SetLoopInformation(predecessor->GetLoopInformation());
          TryCatchInformation* try_catch_info = predecessor->GetTryCatchInformation();
          new_block->SetTryCatchInformation(try_catch_info);
          for (HBasicBlock* xhandler :
               try_catch_info->GetTryEntry().GetBlock()->GetExceptionalSuccessors()) {
            new_block->AddSuccessor(xhandler);
          }
          DCHECK(try_catch_info->GetTryEntry().HasSameExceptionHandlersAs(
              *new_block->GetLastInstruction()->AsTryBoundary()));
        } else {
          // We either have `Throw->TryBoundary` or `Throw`. We want to point the whole chain to the
          // exit, so we recompute `predecessor`
          predecessor = to->GetPredecessors()[pred];
          predecessor->ReplaceSuccessor(to, outer_graph->GetExitBlock());
        }

        --pred;
        // We need to re-run dominance information, as the exit block now has
        // a new predecessor and potential new dominator.
        // TODO(solanes): See if it's worth it to hand-modify the domination chain instead of
        // rerunning the dominance for the whole graph.
        rerun_dominance = true;
        if (predecessor->GetLoopInformation() != nullptr) {
          // The loop information might have changed e.g. `predecessor` might not be in a loop
          // anymore. We only do this if `predecessor` has loop information as it is impossible for
          // predecessor to end up in a loop if it wasn't in one before.
          rerun_loop_analysis = true;
        }
      } else {
        if (last->IsReturnVoid()) {
          DCHECK(return_value == nullptr);
          DCHECK(return_value_phi == nullptr);
        } else {
          DCHECK(last->IsReturn()) << *last;
          if (return_value_phi != nullptr) {
            return_value_phi->AddInput(last->InputAt(0));
          } else if (return_value == nullptr) {
            return_value = last->InputAt(0);
          } else {
            // There will be multiple returns.
            return_value_phi = new (allocator) HPhi(
                allocator, kNoRegNumber, 0, HPhi::ToPhiType(invoke->GetType()), to->GetDexPc());
            to->AddPhi(return_value_phi);
            return_value_phi->AddInput(return_value);
            return_value_phi->AddInput(last->InputAt(0));
            return_value = return_value_phi;
          }
        }
        predecessor->AddInstruction(new (allocator) HGoto(last->GetDexPc()));
        predecessor->RemoveInstruction(last);

        if (saw_try_boundary) {
          predecessor = to->GetPredecessors()[pred];
          DCHECK(predecessor->EndsWithTryBoundary());
          DCHECK_EQ(predecessor->GetNormalSuccessors().size(), 1u);
          if (predecessor->GetSuccessors()[0]->GetPredecessors().size() > 1) {
            outer_graph->SplitCriticalEdge(predecessor, to);
            rerun_dominance = true;
            if (predecessor->GetLoopInformation() != nullptr) {
              rerun_loop_analysis = true;
            }
          }
        }
      }
    }
    if (rerun_loop_analysis) {
      outer_graph->RecomputeDominatorTree();
    } else if (rerun_dominance) {
      outer_graph->ClearDominanceInformation();
      outer_graph->ComputeDominanceInformation();
    }
  }

  // Walk over the entry block and:
  // - Move constants from the entry block to the outer_graph's entry block,
  // - Replace HParameterValue instructions with their real value.
  // - Remove suspend checks, that hold an environment.
  // We must do this after the other blocks have been inlined, otherwise ids of
  // constants could overlap with the inner graph.
  size_t parameter_index = 0;
  for (HInstructionIteratorPrefetchNext it(entry_block_->GetInstructions()); !it.Done();
       it.Advance()) {
    HInstruction* current = it.Current();
    HInstruction* replacement = nullptr;
    if (current->IsNullConstant()) {
      replacement = outer_graph->GetNullConstant();
    } else if (current->IsIntConstant()) {
      replacement = outer_graph->GetIntConstant(current->AsIntConstant()->GetValue());
    } else if (current->IsLongConstant()) {
      replacement = outer_graph->GetLongConstant(current->AsLongConstant()->GetValue());
    } else if (current->IsFloatConstant()) {
      replacement = outer_graph->GetFloatConstant(current->AsFloatConstant()->GetValue());
    } else if (current->IsDoubleConstant()) {
      replacement = outer_graph->GetDoubleConstant(current->AsDoubleConstant()->GetValue());
    } else if (current->IsParameterValue()) {
      if (kIsDebugBuild &&
          invoke->IsInvokeStaticOrDirect() &&
          invoke->AsInvokeStaticOrDirect()->IsStaticWithExplicitClinitCheck()) {
        // Ensure we do not use the last input of `invoke`, as it
        // contains a clinit check which is not an actual argument.
        size_t last_input_index = invoke->InputCount() - 1;
        DCHECK(parameter_index != last_input_index);
      }
      replacement = invoke->InputAt(parameter_index++);
    } else if (current->IsCurrentMethod()) {
      replacement = outer_graph->GetCurrentMethod();
    } else {
      // It is OK to ignore MethodEntryHook for inlined functions.
      // In debug mode we don't inline and in release mode method
      // tracing is best effort so OK to ignore them.
      DCHECK(current->IsGoto() || current->IsSuspendCheck() || current->IsMethodEntryHook());
      entry_block_->RemoveInstruction(current);
    }
    if (replacement != nullptr) {
      current->ReplaceWith(replacement);
      // If the current is the return value then we need to update the latter.
      if (current == return_value) {
        DCHECK_EQ(entry_block_, return_value->GetBlock());
        return_value = replacement;
      }
    }
  }

  return return_value;
}

/*
 * Loop will be transformed to:
 *       old_pre_header
 *             |
 *          if_block
 *           /    \
 *  true_block   false_block
 *           \    /
 *       new_pre_header
 *             |
 *           header
 */
void HGraph::TransformLoopHeaderForBCE(HBasicBlock* header) {
  DCHECK(header->IsLoopHeader());
  HBasicBlock* old_pre_header = header->GetDominator();

  // Need extra block to avoid critical edge.
  HBasicBlock* if_block = HBasicBlock::Create(allocator_, this, header->GetDexPc());
  HBasicBlock* true_block = HBasicBlock::Create(allocator_, this, header->GetDexPc());
  HBasicBlock* false_block = HBasicBlock::Create(allocator_, this, header->GetDexPc());
  HBasicBlock* new_pre_header = HBasicBlock::Create(allocator_, this, header->GetDexPc());
  AddBlock(if_block);
  AddBlock(true_block);
  AddBlock(false_block);
  AddBlock(new_pre_header);

  header->ReplacePredecessor(old_pre_header, new_pre_header);
  old_pre_header->successors_.clear();
  old_pre_header->dominated_blocks_.clear();

  old_pre_header->AddSuccessor(if_block);
  if_block->AddSuccessor(true_block);  // True successor
  if_block->AddSuccessor(false_block);  // False successor
  true_block->AddSuccessor(new_pre_header);
  false_block->AddSuccessor(new_pre_header);

  old_pre_header->dominated_blocks_.push_back(if_block);
  if_block->SetDominator(old_pre_header);
  if_block->dominated_blocks_.push_back(true_block);
  true_block->SetDominator(if_block);
  if_block->dominated_blocks_.push_back(false_block);
  false_block->SetDominator(if_block);
  if_block->dominated_blocks_.push_back(new_pre_header);
  new_pre_header->SetDominator(if_block);
  new_pre_header->dominated_blocks_.push_back(header);
  header->SetDominator(new_pre_header);

  // Fix reverse post order.
  size_t index_of_header = IndexOfElement(reverse_post_order_, header);
  MakeRoomFor(&reverse_post_order_, 4, index_of_header - 1);
  reverse_post_order_[index_of_header++] = if_block;
  reverse_post_order_[index_of_header++] = true_block;
  reverse_post_order_[index_of_header++] = false_block;
  reverse_post_order_[index_of_header++] = new_pre_header;

  // The pre_header can never be a back edge of a loop.
  DCHECK((old_pre_header->GetLoopInformation() == nullptr) ||
         !old_pre_header->GetLoopInformation()->IsBackEdge(*old_pre_header));
  UpdateLoopAndTryInformationOfNewBlock(
      if_block, old_pre_header, /* replace_if_back_edge= */ false);
  UpdateLoopAndTryInformationOfNewBlock(
      true_block, old_pre_header, /* replace_if_back_edge= */ false);
  UpdateLoopAndTryInformationOfNewBlock(
      false_block, old_pre_header, /* replace_if_back_edge= */ false);
  UpdateLoopAndTryInformationOfNewBlock(
      new_pre_header, old_pre_header, /* replace_if_back_edge= */ false);
}

// Creates a new two-basic-block loop and inserts it between original loop header and
// original loop exit; also adjusts dominators, post order and new LoopInformation.
HBasicBlock* HGraph::TransformLoopForVectorization(HBasicBlock* header,
                                                   HBasicBlock* body,
                                                   HBasicBlock* exit) {
  DCHECK(header->IsLoopHeader());
  HLoopInformation* loop = header->GetLoopInformation();

  // Add new loop blocks.
  HBasicBlock* new_pre_header = HBasicBlock::Create(allocator_, this, header->GetDexPc());
  HBasicBlock* new_header = HBasicBlock::Create(allocator_, this, header->GetDexPc());
  HBasicBlock* new_body = HBasicBlock::Create(allocator_, this, header->GetDexPc());
  AddBlock(new_pre_header);
  AddBlock(new_header);
  AddBlock(new_body);

  // Set up control flow.
  header->ReplaceSuccessor(exit, new_pre_header);
  new_pre_header->AddSuccessor(new_header);
  new_header->AddSuccessor(exit);
  new_header->AddSuccessor(new_body);
  new_body->AddSuccessor(new_header);

  // Set up dominators.
  header->ReplaceDominatedBlock(exit, new_pre_header);
  new_pre_header->SetDominator(header);
  new_pre_header->dominated_blocks_.push_back(new_header);
  new_header->SetDominator(new_pre_header);
  new_header->dominated_blocks_.push_back(new_body);
  new_body->SetDominator(new_header);
  new_header->dominated_blocks_.push_back(exit);
  exit->SetDominator(new_header);

  // Fix reverse post order.
  size_t index_of_header = IndexOfElement(reverse_post_order_, header);
  MakeRoomFor(&reverse_post_order_, 2, index_of_header);
  reverse_post_order_[++index_of_header] = new_pre_header;
  reverse_post_order_[++index_of_header] = new_header;
  size_t index_of_body = IndexOfElement(reverse_post_order_, body);
  MakeRoomFor(&reverse_post_order_, 1, index_of_body - 1);
  reverse_post_order_[index_of_body] = new_body;

  // Add gotos and suspend check (client must add conditional in header).
  new_pre_header->AddInstruction(new (allocator_) HGoto());
  HSuspendCheck* suspend_check = new (allocator_) HSuspendCheck(header->GetDexPc());
  new_header->AddInstruction(suspend_check);
  new_body->AddInstruction(new (allocator_) HGoto());
  DCHECK(loop->GetSuspendCheck() != nullptr);
  suspend_check->CopyEnvironmentFromWithLoopPhiAdjustment(
      loop->GetSuspendCheck()->GetEnvironment(), header);

  // Update loop information.
  AddBackEdge(new_header, new_body);
  new_header->GetLoopInformation()->SetSuspendCheck(suspend_check);
  new_header->GetLoopInformation()->Populate();
  new_pre_header->SetLoopInformation(loop->GetPreHeader()->GetLoopInformation());  // outward
  HLoopInformationOutwardIterator it(*new_header);
  for (it.Advance(); !it.Done(); it.Advance()) {
    it.Current()->Add(new_pre_header);
    it.Current()->Add(new_header);
    it.Current()->Add(new_body);
  }
  return new_pre_header;
}

}  // namespace art

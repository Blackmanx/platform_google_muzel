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

#ifndef ART_COMPILER_OPTIMIZING_PROFILING_INFO_BUILDER_H_
#define ART_COMPILER_OPTIMIZING_PROFILING_INFO_BUILDER_H_

#include "base/macros.h"
#include "nodes.h"

namespace art HIDDEN {

class CodeGenerator;
class CompilerOptions;
class HInliner;
class InlineCache;
class ProfilingInfo;

class ProfilingInfoBuilder final : public CRTPGraphVisitor<ProfilingInfoBuilder> {
 public:
  ProfilingInfoBuilder(HGraph* graph,
                       const CompilerOptions& compiler_options,
                       CodeGenerator* codegen)
      : CRTPGraphVisitor(graph),
        codegen_(codegen),
        compiler_options_(compiler_options) {}

  void Run();

  static InlineCache* GetInlineCache(ProfilingInfo* info,
                                     const CompilerOptions& compiler_options,
                                     HInvoke* invoke);
  static bool IsInlineCacheUseful(HInvoke* invoke, CodeGenerator* codegen);
  static uint32_t EncodeInlinedDexPc(
      const HInliner* inliner, const CompilerOptions& compiler_options, HInvoke* invoke)
      REQUIRES_SHARED(Locks::mutator_lock_);

 private:
  // Keep `ForwardVisit()` functions from base class visible except for those we replace below.
  using CRTPGraphVisitor::ForwardVisit;

  // Forward `InvokeVirtual` and `InvokeInterface` to `HandleInvoke()`.
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HInvokeVirtual*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitInvokeVirtual);
    return &ProfilingInfoBuilder::HandleInvoke;
  }
  static constexpr auto ForwardVisit(void (CRTPGraphVisitor::*visit)(HInvokeInterface*)) {
    DCHECK(visit == &CRTPGraphVisitor::VisitInvokeInterface);
    return &ProfilingInfoBuilder::HandleInvoke;
  }

  void HandleInvoke(HInvoke* invoke);

  CodeGenerator* codegen_;
  const CompilerOptions& compiler_options_;
  std::vector<uint32_t> inline_caches_;

  template <typename T> friend class CRTPGraphVisitor;

  DISALLOW_COPY_AND_ASSIGN(ProfilingInfoBuilder);
};

}  // namespace art


#endif  // ART_COMPILER_OPTIMIZING_PROFILING_INFO_BUILDER_H_

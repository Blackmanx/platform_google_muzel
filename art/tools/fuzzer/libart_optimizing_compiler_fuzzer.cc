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

#include <cstdint>
#include <vector>

#include "compiler.h"
#include "dex/standard_dex_file.h"
#include "driver/compiler_options.h"
#include "fuzzer_common.h"
#include "gc/heap.h"
#include "runtime.h"

namespace art {
namespace fuzzer {

// Global variable to count how many DEX files passed DEX file verification and they were
// registered, since these are the cases for which we would be running the GC. In case of
// scheduling multiple fuzzer jobs, using the ‘-jobs’ flag, this is not shared among the threads.
int skipped_gc_iterations = 0;
// Global variable to call the GC once every maximum number of iterations.
// TODO: These values were obtained from local experimenting. They can be changed after
// further investigation.
static constexpr int kMaxSkipGCIterations = 3000;

// Extra debugging information.
static constexpr bool kDebugPrints = false;

std::vector<std::unique_ptr<uint8_t[]>> data_to_delete;
std::vector<std::unique_ptr<StandardDexFile>> dex_files_to_delete;

std::unique_ptr<Compiler> compiler;
std::unique_ptr<CompilerOptions> compiler_options;
std::unique_ptr<FuzzerCompilerCallbacks> callbacks;
// No need for unique_ptr as it is just a fake storage.
FuzzerCompiledMethodStorage storage;

extern "C" int LLVMFuzzerInitialize([[maybe_unused]] int* argc, [[maybe_unused]] char*** argv) {
  callbacks.reset(new FuzzerCompilerCallbacks());
  FuzzerInitialize(callbacks.get(), kRuntimeISA);
  compiler_options = CreateCompilerOptions(/*is_baseline=*/ false, kRuntimeISA);
  compiler.reset(CreateCompiler(*compiler_options, &storage));
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  return CompilerFuzzerTestOneInput(data,
                                    size,
                                    compiler.get(),
                                    callbacks.get(),
                                    &skipped_gc_iterations,
                                    kMaxSkipGCIterations,
                                    kDebugPrints,
                                    data_to_delete,
                                    dex_files_to_delete);
}

}  // namespace fuzzer
}  // namespace art

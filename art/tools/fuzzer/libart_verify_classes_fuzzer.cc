/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "dex/standard_dex_file.h"
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

std::vector<std::unique_ptr<uint8_t[]>> data_to_delete;
std::vector<std::unique_ptr<StandardDexFile>> dex_files_to_delete;

extern "C" int LLVMFuzzerInitialize([[maybe_unused]] int* argc, [[maybe_unused]] char*** argv) {
  static NoopCompilerCallbacks callbacks;
  FuzzerInitialize(&callbacks, kRuntimeISA);
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  // Note that we ignore the resulting DEX file from `VerifyDexFile` since we want to copy `data` to
  // manage it ourselves.
  if (VerifyDexFile(data, size, "fuzz.dex") == nullptr) {
    // DEX file couldn't be verified, don't save it in the corpus.
    return -1;
  }

  // Copy data to keep it alive. Use unique_ptr so that we don't leak.
  data_to_delete.emplace_back(new uint8_t[size]);
  uint8_t* new_data = data_to_delete.back().get();
  memcpy(new_data, data, size);

  dex_files_to_delete.emplace_back(
      new StandardDexFile(new_data,
                          /*location=*/"fuzz.dex",
                          /*location_checksum=*/0,
                          /*oat_dex_file=*/nullptr,
                          std::make_shared<MemoryDexFileContainer>(new_data, size)));
  StandardDexFile* dex_file = dex_files_to_delete.back().get();

  Runtime* runtime = Runtime::Current();
  CHECK(runtime != nullptr);

  jobject class_loader = RegisterDexFileAndGetClassLoader(runtime, dex_file);

  VerifyClasses(class_loader, dex_file);
  IterationCleanup(class_loader, dex_file);

  if (skipped_gc_iterations == kMaxSkipGCIterations) {
    runtime->GetHeap()->CollectGarbage(/* clear_soft_references */ true);
    skipped_gc_iterations = 0;
    data_to_delete.clear();
    dex_files_to_delete.clear();
  } else {
    skipped_gc_iterations++;
  }

  return 0;
}

}  // namespace fuzzer
}  // namespace art

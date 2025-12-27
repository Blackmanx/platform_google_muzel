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

#include "android-base/logging.h"
#include "fuzzer_common.h"

namespace art {
namespace fuzzer {

extern "C" int LLVMFuzzerInitialize([[maybe_unused]] int* argc, [[maybe_unused]] char*** argv) {
  // Set logging to error and above to avoid warnings about unexpected checksums.
  android::base::SetMinimumLogSeverity(android::base::ERROR);
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  VerifyDexFile(data, size, "fuzz.dex");
  return 0;
}

}  // namespace fuzzer
}  // namespace art

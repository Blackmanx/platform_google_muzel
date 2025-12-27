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

#include <cstdint>
#include <filesystem>
#include <unordered_set>

#include "android-base/file.h"
#include "android-base/macros.h"
#include "common_runtime_test.h"
#include "fuzzer_common.h"
#include "gtest/gtest.h"
#include "handle_scope-inl.h"
#include "ziparchive/zip_archive.h"

namespace art {
// Manages the ZipArchiveHandle liveness.
class ZipArchiveHandleScope {
 public:
  explicit ZipArchiveHandleScope(ZipArchiveHandle* handle) : handle_(handle) {}
  ~ZipArchiveHandleScope() { CloseArchive(*(handle_.release())); }

 private:
  std::unique_ptr<ZipArchiveHandle> handle_;
};

class FuzzerCorpusTest : public CommonRuntimeTest {
 public:
  static void DexFileVerification(const uint8_t* data,
                                  size_t size,
                                  const std::string& name,
                                  bool expected_success) {
    const bool is_valid_dex_file = fuzzer::VerifyDexFile(data, size, name) != nullptr;
    ASSERT_EQ(is_valid_dex_file, expected_success) << " Failed for " << name;
  }

  static void ClassVerification(const uint8_t* data,
                                size_t size,
                                const std::string& name,
                                bool expected_success) {
    std::unique_ptr<StandardDexFile> dex_file = fuzzer::VerifyDexFile(data, size, name);
    ASSERT_EQ(dex_file != nullptr, true) << " Failed for " << name;

    Runtime* runtime = Runtime::Current();
    CHECK(runtime != nullptr);

    jobject class_loader = fuzzer::RegisterDexFileAndGetClassLoader(runtime, dex_file.get());
    const bool passed_class_verification = fuzzer::VerifyClasses(class_loader, dex_file.get());
    fuzzer::IterationCleanup(class_loader, dex_file.get());
    ASSERT_EQ(passed_class_verification, expected_success) << " Failed for " << name;
  }

  static void CommonCompilation(const uint8_t* data,
                                size_t size,
                                const std::string& name,
                                bool expected_success,
                                bool is_baseline) {
    std::unique_ptr<StandardDexFile> dex_file = fuzzer::VerifyDexFile(data, size, name);
    ASSERT_EQ(dex_file != nullptr, true) << " Failed for " << name;

    Runtime* runtime = Runtime::Current();
    CHECK(runtime != nullptr);

    fuzzer::FuzzerCompiledMethodStorage storage;
    std::unique_ptr<fuzzer::FuzzerCompilerCallbacks> callbacks(
        new fuzzer::FuzzerCompilerCallbacks());
    std::unique_ptr<CompilerOptions> compiler_options =
        fuzzer::CreateCompilerOptions(is_baseline, kRuntimeISA);
    std::unique_ptr<Compiler> compiler(fuzzer::CreateCompiler(*compiler_options, &storage));

    jobject class_loader = fuzzer::RegisterDexFileAndGetClassLoader(runtime, dex_file.get());
    fuzzer::VerifyClasses(class_loader, dex_file.get());
    const bool at_least_one_method_called_the_compiler = fuzzer::CompileClasses(
        class_loader, dex_file.get(), compiler.get(), callbacks.get(), /*kDebugPrints=*/false);
    // Note: no need to reset callbacks as they will get destroyed
    fuzzer::IterationCleanup(class_loader, dex_file.get());
    ASSERT_EQ(at_least_one_method_called_the_compiler, expected_success) << " Failed for " << name;
  }

  static void OptimizedCompilation(const uint8_t* data,
                                   size_t size,
                                   const std::string& name,
                                   bool expected_success) {
    CommonCompilation(data, size, name, expected_success, /*is_baseline=*/false);
  }

  static void BaselineCompilation(const uint8_t* data,
                                  size_t size,
                                  const std::string& name,
                                  bool expected_success) {
    CommonCompilation(data, size, name, expected_success, /*is_baseline=*/true);
  }

  static void FastCompilation(const uint8_t* data,
                              size_t size,
                              const std::string& name,
                              bool expected_success) {
    std::unique_ptr<StandardDexFile> dex_file = fuzzer::VerifyDexFile(data, size, name);
    ASSERT_EQ(dex_file != nullptr, true) << " Failed for " << name;

    Runtime* runtime = Runtime::Current();
    CHECK(runtime != nullptr);

    std::unique_ptr<fuzzer::FuzzerCompilerCallbacks> callbacks(
        new fuzzer::FuzzerCompilerCallbacks());
    std::unique_ptr<CompilerOptions> compiler_options =
        fuzzer::CreateCompilerOptions(/*is_baseline=*/false, InstructionSet::kArm64);

    jobject class_loader = fuzzer::RegisterDexFileAndGetClassLoader(runtime, dex_file.get());
    fuzzer::VerifyClasses(class_loader, dex_file.get());
    const bool at_least_one_method_called_the_compiler =
        fuzzer::CompileClassesFast(class_loader,
                                   dex_file.get(),
                                   compiler_options.get(),
                                   callbacks.get(),
                                   /*kDebugPrints=*/false);
    // Note: no need to reset callbacks as they will get destroyed
    fuzzer::IterationCleanup(class_loader, dex_file.get());
    ASSERT_EQ(at_least_one_method_called_the_compiler, expected_success) << " Failed for " << name;
  }

  void TestFuzzerHelper(
      const std::string& archive_filename,
      std::function<bool(std::string&)> should_expect_success,
      std::function<void(const uint8_t*, size_t, const std::string&, bool)> verify_file) {
    // Consistency checks.
    const std::string folder = android::base::GetExecutableDirectory();
    ASSERT_TRUE(std::filesystem::is_directory(folder)) << folder << " is not a folder";
    ASSERT_FALSE(std::filesystem::is_empty(folder)) << " No files found for directory " << folder;
    const std::string filename = folder + "/" + archive_filename;

    // Iterate using ZipArchiveHandle. We have to be careful about managing the pointers with
    // CloseArchive, StartIteration, and EndIteration.
    std::string error_msg;
    ZipArchiveHandle handle;
    ZipArchiveHandleScope scope(&handle);
    int32_t error = OpenArchive(filename.c_str(), &handle);
    ASSERT_TRUE(error == 0) << "Error: " << error;

    void* cookie;
    error = StartIteration(handle, &cookie);
    ASSERT_TRUE(error == 0) << "couldn't iterate " << filename << " : " << ErrorCodeString(error);

    ZipEntry64 entry;
    std::string name;
    std::vector<char> data;
    while ((error = Next(cookie, &entry, &name)) >= 0) {
      if (!name.ends_with(".dex")) {
        // Skip non-DEX files.
        LOG(WARNING) << "Found a non-dex file: " << name;
        continue;
      }
      data.resize(entry.uncompressed_length);
      error = ExtractToMemory(handle, &entry, reinterpret_cast<uint8_t*>(data.data()), data.size());
      ASSERT_TRUE(error == 0) << "failed to extract entry: " << name << " from " << filename << ""
                              << ErrorCodeString(error);

      const uint8_t* file_data = reinterpret_cast<const uint8_t*>(data.data());
      // Special case for empty dex file. Set a fake data since the size is 0 anyway.
      if (file_data == nullptr) {
        ASSERT_EQ(data.size(), 0);
        file_data = reinterpret_cast<const uint8_t*>(&name);
      }

      verify_file(file_data, data.size(), name, should_expect_success(name));
    }

    ASSERT_TRUE(error >= -1) << "failed iterating " << filename << " : " << ErrorCodeString(error);
    EndIteration(cookie);
  }
};

// Tests that we can verify dex files without crashing.
TEST_F(FuzzerCorpusTest, VerifyCorpusDexFiles) {
  // These dex files are expected to pass verification. The others are regressions tests.
  const std::unordered_set<std::string> valid_dex_files = {"Main.dex", "hello_world.dex"};
  const std::string archive_filename = "dex_verification_fuzzer_corpus.zip";

  TestFuzzerHelper(
      archive_filename,
      [&valid_dex_files](std::string& str) {
        return valid_dex_files.find(str) != valid_dex_files.end();
      },
      DexFileVerification);
}

// Tests that we can verify classes from dex files without crashing.
TEST_F(FuzzerCorpusTest, VerifyCorpusClassDexFiles) {
  // These dex files are expected to pass verification. The others are regressions tests.
  const std::unordered_set<std::string> valid_dex_files = {"Main.dex", "hello_world.dex"};
  const std::string archive_filename = "class_verification_fuzzer_corpus.zip";

  TestFuzzerHelper(
      archive_filename,
      [&valid_dex_files](std::string& str) {
        return valid_dex_files.find(str) != valid_dex_files.end();
      },
      ClassVerification);
}

// Tests that we can compile classes with kOptimizing from dex files without crashing.
TEST_F(FuzzerCorpusTest, OptimizeCompileDexFiles) {
  const std::string archive_filename = "optimized_compiler_fuzzer_corpus.zip";

  // All added dex files should try to compile at least one method.
  constexpr auto should_expect_success = [](std::string&) { return true; };
  TestFuzzerHelper(archive_filename, should_expect_success, OptimizedCompilation);
}

// Tests that we can compile classes with kBaseline from dex files without crashing.
TEST_F(FuzzerCorpusTest, BaselineCompileDexFiles) {
  const std::string archive_filename = "baseline_compiler_fuzzer_corpus.zip";

  // All added dex files should try to compile at least one method.
  constexpr auto should_expect_success = [](std::string&) { return true; };
  TestFuzzerHelper(archive_filename, should_expect_success, BaselineCompilation);
}

// Tests that we can compile classes with FastCompiler from dex files without crashing.
TEST_F(FuzzerCorpusTest, FastCompileDexFiles) {
  const std::string archive_filename = "fast_compiler_fuzzer_corpus.zip";

  // All added dex files should try to compile at least one method.
  constexpr auto should_expect_success = [](std::string&) { return true; };
  TestFuzzerHelper(archive_filename, should_expect_success, FastCompilation);
}

}  // namespace art

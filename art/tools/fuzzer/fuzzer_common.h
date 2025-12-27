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

#ifndef ART_TOOLS_FUZZER_FUZZER_COMMON_H_
#define ART_TOOLS_FUZZER_FUZZER_COMMON_H_

#include <cstdint>
#include <string>

#include "android-base/file.h"
#include "android-base/strings.h"
#include "base/file_utils.h"
#include "base/mem_map.h"
#include "class_linker.h"
#include "compiler.h"
#include "dex/class_accessor-inl.h"
#include "dex/class_reference.h"
#include "dex/dex_file_verifier.h"
#include "dex/method_reference.h"
#include "dex/standard_dex_file.h"
#include "driver/compiled_code_storage.h"
#include "driver/compiler_options.h"
#include "interpreter/unstarted_runtime.h"
#include "jit/debugger_interface.h"
#include "jni/java_vm_ext.h"
#include "noop_compiler_callbacks.h"
#include "runtime.h"
#include "verifier/class_verifier.h"
#include "well_known_classes.h"

namespace art {
namespace fuzzer {

// Dex file verification

// Returns the DEX file created using the parameters, if it could be verified. Returns nullptr if
// the DEX file couldn't be verified.
ALWAYS_INLINE std::unique_ptr<StandardDexFile> VerifyDexFile(const uint8_t* data,
                                                             size_t size,
                                                             const std::string& location);

// Class verification

// A class to be friends with ClassLinker and access the internal FindDexCacheDataLocked method.
class FuzzerCommonHelper {
 public:
  static const ClassLinker::DexCacheData* GetDexCacheData(Runtime* runtime,
                                                          const StandardDexFile* dex_file)
      REQUIRES_SHARED(Locks::mutator_lock_);
};

// Verifies all classes within a DEX file. Returns true iff all classes could be verified.
ALWAYS_INLINE bool VerifyClasses(jobject class_loader, const StandardDexFile* dex_file);

// Registers a DEX file with the given runtime.
jobject RegisterDexFileAndGetClassLoader(Runtime* runtime, const StandardDexFile* dex_file);

// Initialization for all fuzzers that need a Runtime.
void FuzzerInitialize(CompilerCallbacks* callbacks, InstructionSet isa);

// Cleans up memory after the iteration ran.
ALWAYS_INLINE void IterationCleanup(jobject class_loader, const StandardDexFile* dex_file);

// Compilation

// Storage that will fake setting the thunk code so that the fuzzer can call Emit and test that code
// path.
class FuzzerCompiledMethodStorage final : public CompiledCodeStorage {
 public:
  FuzzerCompiledMethodStorage() {}
  ~FuzzerCompiledMethodStorage() {}

  CompiledMethod* CreateCompiledMethod(InstructionSet instruction_set,
                                       ArrayRef<const uint8_t> code,
                                       ArrayRef<const uint8_t> stack_map,
                                       [[maybe_unused]] ArrayRef<const uint8_t> cfi,
                                       [[maybe_unused]] ArrayRef<const linker::LinkerPatch> patches,
                                       [[maybe_unused]] bool is_intrinsic) override {
    DCHECK_NE(instruction_set, InstructionSet::kNone);
    DCHECK(!code.empty());
    DCHECK(!stack_map.empty());
    return reinterpret_cast<CompiledMethod*>(this);
  }

  ArrayRef<const uint8_t> GetThunkCode([[maybe_unused]] const linker::LinkerPatch& patch,
                                       [[maybe_unused]] /*out*/ std::string* debug_name) override {
    return ArrayRef<const uint8_t>();
  }

  void SetThunkCode([[maybe_unused]] const linker::LinkerPatch& patch,
                    [[maybe_unused]] ArrayRef<const uint8_t> code,
                    [[maybe_unused]] const std::string& debug_name) override {}
};

class FuzzerCompilerCallbacks final : public NoopCompilerCallbacks {
 public:
  FuzzerCompilerCallbacks() {}

  void AddUncompilableMethod(MethodReference ref) override { uncompilable_methods_.insert(ref); }

  void AddUncompilableClass(ClassReference ref) override {
    const DexFile& dex_file = *ref.dex_file;
    const dex::ClassDef& class_def = dex_file.GetClassDef(ref.ClassDefIdx());
    ClassAccessor accessor(dex_file, class_def);
    for (const ClassAccessor::Method& method : accessor.GetMethods()) {
      MethodReference method_ref(&dex_file, method.GetIndex());
      AddUncompilableMethod(method_ref);
    }
  }

  void ClassRejected(ClassReference ref) override { rejected_classes_.insert(ref); }

  bool IsUncompilableMethod(MethodReference ref) override {
    return uncompilable_methods_.find(ref) != uncompilable_methods_.end();
  }

  bool IsClassRejected(ClassReference ref) {
    return rejected_classes_.find(ref) != rejected_classes_.end();
  }

  void Reset() {
    uncompilable_methods_.clear();
    rejected_classes_.clear();
  }

 private:
  std::set<MethodReference> uncompilable_methods_;
  std::set<ClassReference> rejected_classes_;
};

// Common code for several compiler's CompileClasses.
// The compile_method_fn is called for each method that passes the filters.
// Returns true if the compiler was called for this method; false otherwise.
template <typename CompileMethodFn>
ALWAYS_INLINE bool CompileClassesCommon(jobject class_loader,
                                        const StandardDexFile* dex_file,
                                        FuzzerCompilerCallbacks* callbacks,
                                        CompileMethodFn compile_method_fn);

// Compiles all classes within a DEX file. Returns true iff at least one method called the compiler.
ALWAYS_INLINE bool CompileClasses(jobject class_loader,
                                  const StandardDexFile* dex_file,
                                  Compiler* compiler,
                                  FuzzerCompilerCallbacks* callbacks,
                                  bool debug_prints);

// Compiles all classes within a DEX file using the fast compiler. Returns true iff at least one
// method called the compiler.
ALWAYS_INLINE bool CompileClassesFast(jobject class_loader,
                                      const StandardDexFile* dex_file,
                                      CompilerOptions* compiler_options,
                                      FuzzerCompilerCallbacks* callbacks,
                                      bool debug_prints);

// Creates a compiler. As part of this, creates and sets `compiler_options`.
Compiler* CreateCompiler(const CompilerOptions& compiler_options,
                         CompiledCodeStorage* storage);
std::unique_ptr<CompilerOptions> CreateCompilerOptions(bool is_baseline, InstructionSet isa);

// Common compiler fuzzer main logic.
template <typename CompilerOrCompilerOptions, typename CompileFn>
ALWAYS_INLINE int FuzzerTestOneInputCommon(
    const uint8_t* data,
    size_t size,
    CompilerOrCompilerOptions* compiler_or_compiler_options,
    FuzzerCompilerCallbacks* callbacks,
    int* skipped_gc_iterations,
    int max_skip_gc_iterations,
    bool debug_prints,
    std::vector<std::unique_ptr<uint8_t[]>>& data_to_delete,
    std::vector<std::unique_ptr<StandardDexFile>>& dex_files_to_delete,
    CompileFn compile_fn);

// Baseline and Optimizing compilers
ALWAYS_INLINE int CompilerFuzzerTestOneInput(
    const uint8_t* data,
    size_t size,
    Compiler* compiler,
    FuzzerCompilerCallbacks* callbacks,
    int* skipped_gc_iterations,
    int max_skip_gc_iterations,
    bool debug_prints,
    std::vector<std::unique_ptr<uint8_t[]>>& data_to_delete,
    std::vector<std::unique_ptr<StandardDexFile>>& dex_files_to_delete);

// Fast compiler
ALWAYS_INLINE int FastCompilerFuzzerTestOneInput(
    const uint8_t* data,
    size_t size,
    CompilerOptions* compiler_options,
    FuzzerCompilerCallbacks* callbacks,
    int* skipped_gc_iterations,
    int max_skip_gc_iterations,
    bool debug_prints,
    std::vector<std::unique_ptr<uint8_t[]>>& data_to_delete,
    std::vector<std::unique_ptr<StandardDexFile>>& dex_files_to_delete);

}  // namespace fuzzer
}  // namespace art

#endif  // ART_TOOLS_FUZZER_FUZZER_COMMON_H_

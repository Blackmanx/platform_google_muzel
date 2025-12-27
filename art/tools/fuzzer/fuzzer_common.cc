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

#include "fuzzer_common.h"

#include <signal.h>

#include "art_method-inl.h"
#include "dex/class_accessor-inl.h"
#include "driver/dex_compilation_unit.h"
#include "handle_scope-inl.h"
#include "interpreter/unstarted_runtime.h"
#include "optimizing/fast_compiler.h"
#include "runtime_intrinsics.h"
#include "scoped_thread_state_change-inl.h"

namespace art {
namespace fuzzer {

// Global variable to signal LSAN that we are not leaking memory.
uint8_t* allocated_signal_stack = nullptr;

static std::string GetDexFileName(const std::string& jar_name) {
  // The jar files are located in the data directory within the directory of the fuzzer's binary.
  std::string executable_dir = android::base::GetExecutableDirectory();

  std::string result =
      android::base::StringPrintf("%s/data/%s.jar", executable_dir.c_str(), jar_name.c_str());

  return result;
}

static std::vector<std::string> GetLibCoreDexFileNames() {
  std::vector<std::string> result;
  const std::vector<std::string> modules = {
      "core-oj",
      "core-libart",
      "okhttp",
      "bouncycastle",
      "apache-xml",
      "core-icu4j",
      "conscrypt",
  };
  result.reserve(modules.size());
  for (const std::string& module : modules) {
    result.push_back(GetDexFileName(module));
  }
  return result;
}

static std::string GetClassPathOption(const char* option,
                                      const std::vector<std::string>& class_path) {
  return option + android::base::Join(class_path, ':');
}

void FuzzerInitialize(CompilerCallbacks* callbacks, InstructionSet isa) {
  // Set logging to error and above to avoid warnings about unexpected checksums.
  android::base::SetMinimumLogSeverity(android::base::ERROR);

  // Create runtime.
  RuntimeOptions options;
  options.push_back(std::make_pair("compilercallbacks", callbacks));

  std::string boot_class_path_string =
      GetClassPathOption("-Xbootclasspath:", GetLibCoreDexFileNames());
  options.push_back(std::make_pair(boot_class_path_string, nullptr));

  // Instruction set.
  options.push_back(std::make_pair("imageinstructionset",
                                   reinterpret_cast<const void*>(GetInstructionSetString(isa))));

  if (!Runtime::Create(options, false)) {
    LOG(FATAL) << "We should always be able to create the runtime";
    UNREACHABLE();
  }

  interpreter::UnstartedRuntime::Initialize();
  Runtime::Current()->GetClassLinker()->RunEarlyRootClinits(Thread::Current());
  InitializeIntrinsics();
  Runtime::Current()->RunRootClinits(Thread::Current());

  // Check for heap corruption before running the fuzzer.
  Runtime::Current()->GetHeap()->VerifyHeap();

  // Runtime::Create acquired the mutator_lock_ that is normally given away when we
  // Runtime::Start, give it away now with `TransitionFromSuspendedToRunnable` until we figure out
  // how to start a Runtime.
  Thread::Current()->TransitionFromRunnableToSuspended(ThreadState::kNative);

  // Query the current stack and add it to the global variable. Otherwise LSAN complains about a
  // non-existing leak.
  stack_t ss;
  if (sigaltstack(nullptr, &ss) == -1) {
    PLOG(FATAL) << "sigaltstack failed";
  }
  allocated_signal_stack = reinterpret_cast<uint8_t*>(ss.ss_sp);
}

ALWAYS_INLINE std::unique_ptr<StandardDexFile> VerifyDexFile(const uint8_t* data,
                                                             size_t size,
                                                             const std::string& location) {
  // Do not verify the checksum as we only care about the DEX file contents,
  // and know that the checksum would probably be erroneous (i.e. random).
  constexpr bool kVerifyChecksum = false;

  std::unique_ptr<StandardDexFile> dex_file(
      new StandardDexFile(data,
                          location,
                          /*location_checksum=*/0,
                          /*oat_dex_file=*/nullptr,
                          std::make_shared<MemoryDexFileContainer>(data, size)));

  std::string error_msg;
  if (!dex::Verify(dex_file.get(), dex_file->GetLocation().c_str(), kVerifyChecksum, &error_msg)) {
    dex_file.reset();
  }
  return dex_file;
}

const ClassLinker::DexCacheData* FuzzerCommonHelper::GetDexCacheData(
    Runtime* runtime, const StandardDexFile* dex_file) {
  Thread* self = Thread::Current();
  ReaderMutexLock mu(self, *Locks::dex_lock_);
  ClassLinker* class_linker = runtime->GetClassLinker();
  const ClassLinker::DexCacheData* cached_data = class_linker->FindDexCacheDataLocked(*dex_file);
  return cached_data;
}

ALWAYS_INLINE bool VerifyClasses(jobject class_loader, const StandardDexFile* dex_file) {
  bool passed_class_verification = true;
  Runtime* runtime = Runtime::Current();
  CHECK(runtime != nullptr);
  ClassLinker* class_linker = runtime->GetClassLinker();
  ScopedObjectAccess soa(Thread::Current());

  StackHandleScope<4> scope(soa.Self());
  Handle<mirror::ClassLoader> h_loader =
      scope.NewHandle(soa.Decode<mirror::ClassLoader>(class_loader));
  MutableHandle<mirror::Class> h_klass(scope.NewHandle<mirror::Class>(nullptr));
  MutableHandle<mirror::DexCache> h_dex_cache(scope.NewHandle<mirror::DexCache>(nullptr));
  MutableHandle<mirror::ClassLoader> h_dex_cache_class_loader = scope.NewHandle(h_loader.Get());

  for (ClassAccessor accessor : dex_file->GetClasses()) {
    h_klass.Assign(
        class_linker->FindClass(soa.Self(), *dex_file, accessor.GetClassIdx(), h_loader));
    // Ignore classes that couldn't be loaded since we are looking for crashes during
    // class/method verification.
    if (h_klass == nullptr || h_klass->IsErroneous()) {
      // Treat as failure to pass verification
      passed_class_verification = false;
      soa.Self()->ClearException();
      continue;
    }
    if (&h_klass->GetDexFile() != dex_file) {
      // Skip a duplicate class (as the resolved class is from another dex file). This can happen
      // e.g. if we have a class named "sun.misc.Unsafe" in fuzz.dex.
      continue;
    }

    h_dex_cache.Assign(h_klass->GetDexCache());

    // The class loader from the class's dex cache is different from the dex file's class loader
    // for boot image classes e.g. java.util.AbstractCollection.
    h_dex_cache_class_loader.Assign(h_klass->GetDexCache()->GetClassLoader());

    CHECK(h_klass->IsResolved()) << h_klass->PrettyClass();
    verifier::FailureKind failure_kind =
        class_linker->VerifyClass(soa.Self(),
                                  /* verifier_deps= */ nullptr,
                                  h_klass,
                                  // Don't abort on verification errors.
                                  verifier::HardFailLogMode::kLogWarning);

    DCHECK_EQ(h_klass->IsErroneous(), failure_kind == verifier::FailureKind::kHardFailure);
    if (failure_kind == verifier::FailureKind::kHardFailure) {
      passed_class_verification = false;
      // ClassLinker::VerifyClass throws, so we clear it before we continue.
      CHECK(soa.Self()->IsExceptionPending());
      soa.Self()->ClearException();
    }

    CHECK(h_klass->ShouldVerifyAtRuntime() || h_klass->IsVerifiedNeedsAccessChecks() ||
          h_klass->IsVerified() || h_klass->IsErroneous())
        << h_klass->PrettyDescriptor() << ": state=" << h_klass->GetStatus();

    soa.Self()->AssertNoPendingException();
  }

  return passed_class_verification;
}

ALWAYS_INLINE void IterationCleanup(jobject class_loader, const StandardDexFile* dex_file) {
  ScopedObjectAccess soa(Thread::Current());
  Runtime* runtime = Runtime::Current();
  // Clear the arena pool to free RAM. The next iteration won't be referencing the pool we just
  // used.
  runtime->ReclaimArenaPoolMemory();

  // Delete weak root to the DexCache before removing a DEX file from the cache. This is usually
  // handled by the GC, but since we are not calling it every iteration, we need to delete them
  // manually.
  const ClassLinker::DexCacheData* dex_cache_data =
      FuzzerCommonHelper::GetDexCacheData(runtime, dex_file);
  if (dex_cache_data != nullptr && dex_cache_data->weak_root != nullptr) {
    soa.Env()->GetVm()->DeleteWeakGlobalRef(soa.Self(), dex_cache_data->weak_root);
  }

  // Mimic DexFile_closeDexFile.
  RemoveNativeDebugInfoForDex(soa.Self(), dex_file);
  runtime->GetClassLinker()->RemoveDexFromCaches(*dex_file);

  // Delete global ref and unload class loader to free RAM.
  soa.Env()->GetVm()->DeleteGlobalRef(soa.Self(), class_loader);
}

jobject RegisterDexFileAndGetClassLoader(Runtime* runtime, const StandardDexFile* dex_file) {
  Thread* self = Thread::Current();
  ScopedObjectAccess soa(self);
  ClassLinker* class_linker = runtime->GetClassLinker();
  const std::vector<const DexFile*> dex_files = {dex_file};
  jobject class_loader = class_linker->CreatePathClassLoader(self, dex_files);
  ObjPtr<mirror::ClassLoader> cl = self->DecodeJObject(class_loader)->AsClassLoader();
  class_linker->RegisterDexFile(*dex_file, cl);
  return class_loader;
}

std::unique_ptr<CompilerOptions> CreateCompilerOptions(bool is_baseline, InstructionSet isa) {
  std::unique_ptr<CompilerOptions> opt = std::make_unique<CompilerOptions>();
  opt->emit_read_barrier_ = gUseReadBarrier;
  opt->instruction_set_ = (isa == InstructionSet::kArm) ? InstructionSet::kThumb2 : isa;
  std::string error_msg;
  opt->instruction_set_features_ =
      InstructionSetFeatures::FromVariant(opt->instruction_set_, "default", &error_msg);
  CHECK(opt->instruction_set_features_ != nullptr) << error_msg;
  opt->implicit_null_checks_ = true;
  opt->implicit_so_checks_ = true;
  opt->implicit_suspend_checks_ = (opt->instruction_set_ == InstructionSet::kArm64);
  opt->baseline_ = is_baseline;
  opt->inline_max_code_units_ = CompilerOptions::kDefaultInlineMaxCodeUnits;
  return opt;
}

Compiler* CreateCompiler(const CompilerOptions& compiler_options,
                         CompiledCodeStorage* storage) {
  // Consistency checks
  CHECK(!compiler_options.GetDebuggable());

  // Testing AOT compiler.
  CHECK_EQ(Runtime::Current()->GetJit(), nullptr);

  return Compiler::Create(compiler_options, storage);
}

template <typename CompileMethodFn>
ALWAYS_INLINE bool CompileClassesCommon(jobject class_loader,
                                        const StandardDexFile* dex_file,
                                        FuzzerCompilerCallbacks* callbacks,
                                        CompileMethodFn compile_method_fn) {
  bool at_least_one_method_called_the_compiler = false;
  Runtime* runtime = Runtime::Current();
  ClassLinker* class_linker = runtime->GetClassLinker();

  ScopedObjectAccess soa(Thread::Current());
  StackHandleScope<4> scope(soa.Self());
  Handle<mirror::ClassLoader> h_loader =
      scope.NewHandle(soa.Decode<mirror::ClassLoader>(class_loader));
  MutableHandle<mirror::Class> h_klass(scope.NewHandle<mirror::Class>(nullptr));
  MutableHandle<mirror::DexCache> h_dex_cache(scope.NewHandle<mirror::DexCache>(nullptr));
  MutableHandle<mirror::ClassLoader> h_dex_cache_class_loader = scope.NewHandle(h_loader.Get());

  // We finished verification and we can move to compilation, using the verification results
  for (ClassAccessor accessor : dex_file->GetClasses()) {
    h_klass.Assign(
        class_linker->FindClass(soa.Self(), *dex_file, accessor.GetClassIdx(), h_loader));
    // Ignore classes that couldn't be loaded since we are looking for crashes during
    // compilation.
    if (h_klass == nullptr || h_klass->IsErroneous()) {
      soa.Self()->ClearException();
      continue;
    }
    if (&h_klass->GetDexFile() != dex_file) {
      // Skip a duplicate class (as the resolved class is from another dex file). This can happen
      // e.g. if we have a class named "sun.misc.Unsafe" in fuzz.dex.
      continue;
    }

    ClassReference ref(dex_file, h_klass->GetDexClassDefIndex());
    if (callbacks->IsClassRejected(ref)) {
      continue;
    }

    h_dex_cache.Assign(h_klass->GetDexCache());

    // The class loader from the class's dex cache is different from the dex file's class loader
    // for boot image classes e.g. java.util.AbstractCollection.
    h_dex_cache_class_loader.Assign(h_klass->GetDexCache()->GetClassLoader());

    int64_t previous_method_idx = -1;
    for (const ClassAccessor::Method& method : accessor.GetMethods()) {
      const uint32_t method_idx = method.GetIndex();
      if (method_idx == previous_method_idx) {
        // Duplicate method
        continue;
      }
      previous_method_idx = method_idx;

      if (compile_method_fn(dex_file,
                            callbacks,
                            method,
                            h_klass,
                            h_dex_cache,
                            h_dex_cache_class_loader,
                            accessor.GetClassDefIndex())) {
        at_least_one_method_called_the_compiler = true;
      }
    }
  }
  return at_least_one_method_called_the_compiler;
}

ALWAYS_INLINE bool CompileClasses(jobject class_loader,
                                  const StandardDexFile* dex_file,
                                  Compiler* compiler,
                                  FuzzerCompilerCallbacks* callbacks,
                                  bool debug_prints) {
  return CompileClassesCommon(
      class_loader,
      dex_file,
      callbacks,
      [compiler, debug_prints](const StandardDexFile* dex_file,
                               FuzzerCompilerCallbacks* callbacks,
                               const ClassAccessor::Method& method,
                               Handle<mirror::Class>& h_klass,
                               MutableHandle<mirror::DexCache>& h_dex_cache,
                               MutableHandle<mirror::ClassLoader>& h_dex_cache_class_loader,
                               uint16_t class_def_idx) REQUIRES_SHARED(art::Locks::mutator_lock_) {
        const uint32_t access_flags = method.GetAccessFlags();

        if (ArtMethod::IsNative(access_flags)) {
          // TODO(solanes): Support JNI?
          return false;
        }

        if (ArtMethod::IsAbstract(access_flags)) {
          // Abstract methods don't have code.
          return false;
        }

        if (!ArtMethod::IsCompilable(access_flags) || ArtMethod::IsIntrinsic(access_flags)) {
          // This method will never be compiled.
          return false;
        }

        MethodReference method_ref(dex_file, method.GetIndex());
        if (debug_prints) {
          LOG(ERROR) << "Going to compile: " << dex_file->PrettyMethod(method.GetIndex())
                     << ". IsUncompilableMethod: " << std::boolalpha
                     << callbacks->IsUncompilableMethod(method_ref) << std::noboolalpha
                     << " using klass " << h_klass->PrettyClass();
        }

        if (callbacks->IsUncompilableMethod(method_ref)) {
          return false;
        }
        compiler->Compile(method.GetCodeItem(),
                          access_flags,
                          class_def_idx,
                          method.GetIndex(),
                          h_dex_cache_class_loader,
                          *dex_file,
                          h_dex_cache);
        return true;
      });
}

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
    CompileFn compile_fn) {
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
  const bool at_least_one_method_called_the_compiler =
      compile_fn(class_loader, dex_file, compiler_or_compiler_options, callbacks, debug_prints);
  callbacks->Reset();
  IterationCleanup(class_loader, dex_file);

  if (*skipped_gc_iterations == max_skip_gc_iterations) {
    runtime->GetHeap()->CollectGarbage(/* clear_soft_references */ true);
    *skipped_gc_iterations = 0;
    data_to_delete.clear();
    dex_files_to_delete.clear();
  } else {
    *skipped_gc_iterations += 1;
  }

  // Save only if at least one method compiled
  return at_least_one_method_called_the_compiler ? 0 : -1;
}

ALWAYS_INLINE int CompilerFuzzerTestOneInput(
    const uint8_t* data,
    size_t size,
    Compiler* compiler,
    FuzzerCompilerCallbacks* callbacks,
    int* skipped_gc_iterations,
    int max_skip_gc_iterations,
    bool debug_prints,
    std::vector<std::unique_ptr<uint8_t[]>>& data_to_delete,
    std::vector<std::unique_ptr<StandardDexFile>>& dex_files_to_delete) {
  return FuzzerTestOneInputCommon(data,
                                  size,
                                  compiler,
                                  callbacks,
                                  skipped_gc_iterations,
                                  max_skip_gc_iterations,
                                  debug_prints,
                                  data_to_delete,
                                  dex_files_to_delete,
                                  [](jobject class_loader,
                                     const StandardDexFile* dex_file,
                                     Compiler* compiler,
                                     FuzzerCompilerCallbacks* callbacks,
                                     bool debug_prints) {
                                    return CompileClasses(
                                        class_loader, dex_file, compiler, callbacks, debug_prints);
                                  });
}

ALWAYS_INLINE bool CompileClassesFast(jobject class_loader,
                                      const StandardDexFile* dex_file,
                                      CompilerOptions* compiler_options,
                                      FuzzerCompilerCallbacks* callbacks,
                                      bool debug_prints) {
  art::ArenaPool* pool = art::Runtime::Current()->GetArenaPool();
  return CompileClassesCommon(
      class_loader,
      dex_file,
      callbacks,
      [compiler_options, debug_prints, pool](
          const StandardDexFile* dex_file,
          FuzzerCompilerCallbacks* callbacks,
          const ClassAccessor::Method& method,
          Handle<mirror::Class>& h_klass,
          MutableHandle<mirror::DexCache>& h_dex_cache,
          MutableHandle<mirror::ClassLoader>& h_dex_cache_class_loader,
          uint16_t class_def_idx) REQUIRES_SHARED(art::Locks::mutator_lock_) {
        Thread* self = Thread::Current();
        ClassLinker* class_linker = Runtime::Current()->GetClassLinker();

        // TODO(solanes): Is this the best way to get an art method?
        ArtMethod* art_method =
            class_linker->ResolveMethodId(method.GetIndex(), h_dex_cache, h_dex_cache_class_loader);

        if (art_method == nullptr) {
          // TODO(solanes): Can we guarantee that we will resolve to an ArtMethod?
          return false;
        }

        // Use the flags from the ArtMethod, which may be different from the ClassAccessor::Method
        const uint32_t access_flags = art_method->GetAccessFlags();

        if (ArtMethod::IsNative(access_flags)) {
          // TODO(solanes): Support JNI?
          return false;
        }

        if (ArtMethod::IsAbstract(access_flags)) {
          // Abstract methods don't have code.
          return false;
        }

        if (!ArtMethod::IsCompilable(access_flags) ||
            ArtMethod::IsIntrinsic(access_flags) ||
            !ArtMethod::IsInvokable(access_flags)) {
          // This method will never be compiled.
          return false;
        }

        MethodReference method_ref(dex_file, method.GetIndex());
        if (callbacks->IsUncompilableMethod(method_ref)) {
          return false;
        }

        if (debug_prints) {
          LOG(ERROR) << "Going to compile: " << dex_file->PrettyMethod(method.GetIndex())
                     << ". IsUncompilableMethod: " << std::boolalpha
                     << callbacks->IsUncompilableMethod(method_ref) << std::noboolalpha
                     << " using klass " << h_klass->PrettyClass();
        }

        art::ArenaAllocator allocator(pool);
        art::ArenaStack arena_stack(pool);
        art::VariableSizedHandleScope handles(self);
        art::DexCompilationUnit dex_compilation_unit(h_dex_cache_class_loader,
                                                     class_linker,
                                                     *dex_file,
                                                     art_method->GetCodeItem(),
                                                     class_def_idx,
                                                     art_method->GetDexMethodIndex(),
                                                     access_flags,
                                                     nullptr,
                                                     h_dex_cache,
                                                     h_klass);

        art::FastCompiler::Compile(art_method,
                                   &allocator,
                                   &arena_stack,
                                   &handles,
                                   *compiler_options,
                                   dex_compilation_unit);
        return true;
      });
}

ALWAYS_INLINE int FastCompilerFuzzerTestOneInput(
    const uint8_t* data,
    size_t size,
    CompilerOptions* compiler_options,
    FuzzerCompilerCallbacks* callbacks,
    int* skipped_gc_iterations,
    int max_skip_gc_iterations,
    bool debug_prints,
    std::vector<std::unique_ptr<uint8_t[]>>& data_to_delete,
    std::vector<std::unique_ptr<StandardDexFile>>& dex_files_to_delete) {
  return FuzzerTestOneInputCommon(
      data,
      size,
      compiler_options,
      callbacks,
      skipped_gc_iterations,
      max_skip_gc_iterations,
      debug_prints,
      data_to_delete,
      dex_files_to_delete,
      [](jobject class_loader,
         const StandardDexFile* dex_file,
         CompilerOptions* compiler_options,
         FuzzerCompilerCallbacks* callbacks,
         bool debug_prints) {
        return CompileClassesFast(
            class_loader, dex_file, compiler_options, callbacks, debug_prints);
      });
}

}  // namespace fuzzer
}  // namespace art

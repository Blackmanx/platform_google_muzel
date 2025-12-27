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

#include "asm_support_x86_64.S"
#include "interpreter/cfi_asm_support.h"

/*
 * This file contains all native entrypoints that are called using the native ABI and do not
 * transition to the quick ABI. For example: the switch interpreter (using the native ABI) directly
 * calls ExecuteSwitchImplAsm and this code will always return back to the switch interpreter,
 * again using the native ABI. Because of this behaviour ExecuteSwitchImplAsm should be included in
 * this file. This is done so these native entrypoints can be compiled independently to quick
 * entrypoints for cases when the kRuntimeISA and kRuntimeQuickCodeISA do not match.
 *
 * See comment on StackType (thread.h) for definitions and examples of quick ABI/code and
 * native ABI/code.
 */

// Wrap ExecuteSwitchImpl in assembly method which specifies DEX PC for unwinding.
//  Argument 0: RDI: The context pointer for ExecuteSwitchImpl.
//  Argument 1: RSI: Pointer to the templated ExecuteSwitchImpl to call.
//  Argument 2: RDX: The value of DEX PC (memory address of the methods bytecode).
DEFINE_FUNCTION ExecuteSwitchImplAsm
    PUSH rbx                 // Spill RBX
    movq %rdx, %rbx          // RBX = DEX PC (callee save register)
    CFI_DEFINE_DEX_PC_WITH_OFFSET(0 /* RAX */, 3 /* RBX */, 0)

    call *%rsi               // Call the wrapped function

    POP rbx                  // Restore RBX
    ret
END_FUNCTION ExecuteSwitchImplAsm

    /*
     * Jni dlsym lookup stub.
     */
DEFINE_FUNCTION art_jni_dlsym_lookup_stub
    // Save callee and GPR args.
    PUSH_ARG r9   // Arg.
    PUSH_ARG r8   // Arg.
    PUSH_ARG rdi  // Arg. (JniEnv for normal and @FastNative)
    PUSH_ARG rsi  // Arg.
    PUSH_ARG rdx  // Arg.
    PUSH_ARG rcx  // Arg.
    // Create space for FPR args, plus padding for alignment
    INCREASE_FRAME 72
    // Save FPRs.
    movq %xmm0, 0(%rsp)
    movq %xmm1, 8(%rsp)
    movq %xmm2, 16(%rsp)
    movq %xmm3, 24(%rsp)
    movq %xmm4, 32(%rsp)
    movq %xmm5, 40(%rsp)
    movq %xmm6, 48(%rsp)
    movq %xmm7, 56(%rsp)
    // prepare call
    movq %gs:THREAD_SELF_OFFSET, %rdi      // RDI := Thread::Current()
    // Call artFindNativeMethod() for normal native and artFindNativeMethodRunnable()
    // for @FastNative or @CriticalNative.
    movq THREAD_TOP_QUICK_FRAME_OFFSET(%rdi), %rax   // uintptr_t tagged_quick_frame
    andq LITERAL(TAGGED_JNI_SP_MASK_TOGGLED64), %rax // ArtMethod** sp
    movq (%rax), %rax                                // ArtMethod* method
#ifdef ART_USE_RESTRICTED_MODE
    // Critical native methods are disabled and treated as normal native methods instead.
    testl LITERAL(ACCESS_FLAGS_METHOD_IS_FAST_NATIVE), \
          ART_METHOD_ACCESS_FLAGS_OFFSET(%rax)
#else
    testl LITERAL(ACCESS_FLAGS_METHOD_IS_FAST_NATIVE | ACCESS_FLAGS_METHOD_IS_CRITICAL_NATIVE), \
          ART_METHOD_ACCESS_FLAGS_OFFSET(%rax)
#endif
    jne .Llookup_stub_fast_or_critical_native
    call SYMBOL(artFindNativeMethod)  // (Thread*)
    jmp .Llookup_stub_continue
.Llookup_stub_fast_or_critical_native:
    call SYMBOL(artFindNativeMethodRunnable)  // (Thread*)
.Llookup_stub_continue:
    // restore arguments
    movq 0(%rsp), %xmm0
    movq 8(%rsp), %xmm1
    movq 16(%rsp), %xmm2
    movq 24(%rsp), %xmm3
    movq 32(%rsp), %xmm4
    movq 40(%rsp), %xmm5
    movq 48(%rsp), %xmm6
    movq 56(%rsp), %xmm7
    DECREASE_FRAME 72
    POP_ARG rcx  // Arg.
    POP_ARG rdx  // Arg.
    POP_ARG rsi  // Arg.
    POP_ARG rdi  // Arg. (JniEnv for normal and @FastNative)
    POP_ARG r8   // Arg.
    POP_ARG r9   // Arg.
    testq %rax, %rax              // check if returned method code is null
    jz .Lno_native_code_found     // if null, jump to return to handle
    jmp *%rax                     // otherwise, tail call to intended method
.Lno_native_code_found:
    ret
END_FUNCTION art_jni_dlsym_lookup_stub

    /*
     * Simulator's Generic JNI stub - native part.
     *
     * When this is called, artQuickGenericJniTrampoline has already setup the new native frame on
     * the simulated (quick) stack. This stub will reserve an equal space on the hardware (native)
     * stack and copy across the native frame from the simulated stack to the newly reserved area
     * on the hardware stack before doing the native call.
     *
     * See comments for StackType in thread.h for more information on the types of stack being
     * used.
     *
     * On entry:
     *   rdi = native function pointer
     *   rsi = simulated_reserved_area pointer
     *   rdx = out_fp_result pointer
     */
DEFINE_FUNCTION art_quick_generic_jni_trampoline_simulator
    PUSH_ARG rdi    // native function pointer
    PUSH_ARG rsi    // simulated_reserved_area pointer
    PUSH_ARG rdx    // out_fp_result pointer
    PUSH r12        // callee saved
    PUSH rbp        // callee saved

    movq %rsp, %rbp
    CFI_DEF_CFA_REGISTER(rbp)
    subq LITERAL(GENERIC_JNI_TRAMPOLINE_RESERVED_AREA), %rsp

    // prepare for memcpy call
    // (native stack ptr, simulator reserved area ptr, length of reserved area)
    movq %rsp, %rdi           // pass SP to param 0
    // The simulated reserved area is still in rsi which is passed as param 1
    movq LITERAL(GENERIC_JNI_TRAMPOLINE_RESERVED_AREA), %rdx  // pass the length of memory to copy

    call SYMBOL(memcpy)

    // 120 bytes is the constant offset from the top of the reserved area to the pointer which
    // points to the JNI stack args on the simulated stack. Space is always reserved for the
    // maximum number of arguments so we can always load them all, even if some are unused.
    //
    // See a diagram near art_quick_generic_jni_trampoline and the number of native args from
    // BuildNativeCallFrameStateMachine:
    // 8 FP regs, 6 GP regs, Hidden arg = 120 bytes
    movq (120)(%rsp), %r12
    // def r12: delta between JNI stack args on the simulated stack and the simulated reserved area
    // ptr.
    subq 24(%rbp), %r12

    // Load parameters from frame into registers.
    movq 0(%rsp), %rdi
    movq 8(%rsp), %rsi
    movq 16(%rsp), %rdx
    movq 24(%rsp), %rcx
    movq 32(%rsp), %r8
    movq 40(%rsp), %r9

    movq 48(%rsp), %xmm0
    movq 56(%rsp), %xmm1
    movq 64(%rsp), %xmm2
    movq 72(%rsp), %xmm3
    movq 80(%rsp), %xmm4
    movq 88(%rsp), %xmm5
    movq 96(%rsp), %xmm6
    movq 104(%rsp), %xmm7

    // Save call target in scratch register.
    movq (32)(%rbp), %r11

    // Load hidden arg (rax) for @CriticalNative.
    movq 112(%rsp), %rax
    // Increase rsp from the reserved area pointer on the hardware stack, to point to the JNI stack
    // args on the hardware stack.
    addq %r12, %rsp

    // native call
    call *%r11

    // Tear down the reserved area on the hardware stack. The reserved area on the simulated stack
    // will be torn down after we return to art_quick_generic_jni_trampoline.
    movq %rbp, %rsp

    // Load out_fp_result pointer
    movq 16(%rbp), %rsi
    movq %xmm0, (%rsi)

    POP rbp
    POP r12
    POP_ARG rdx
    POP_ARG rsi
    POP_ARG rdi

    ret
END_FUNCTION art_quick_generic_jni_trampoline_simulator

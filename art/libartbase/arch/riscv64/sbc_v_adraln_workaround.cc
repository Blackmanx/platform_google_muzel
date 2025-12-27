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

#include <signal.h>
#include <stdlib.h>

#include <android-base/stringprintf.h>

#include "base/logging.h"
#include "base/macros.h"

namespace art HIDDEN {
namespace riscv64 {

#if defined(ART_TEST_ON_SBC_RISCV64_V_ADRALN_WORKAROUND)
NO_RETURN static void fatal(const char* file, unsigned line, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::string error;
  android::base::StringAppendV(&error, fmt, ap);
  va_end(ap);
  LogHelper::LogLineLowStack(file, line, android::base::FATAL, error.c_str());
  abort();
}

#define FATAL(...) fatal(__FILE__, __LINE__, __VA_ARGS__)

// Some SBCs such as Orange Pi RV2 are almost RVA22+V compliant, except that
// the V extension does not comply with the Zicclsm and unaligned memory access
// triggers SIGBUS/ADRALN for vector instructions. It is possible to test ART
// on these SBCs with a workaround to emulate these instructions.
EXPORT void SigBusAdrAlnWorkaround(int signo, siginfo_t* siginfo, void* ucontext_raw) {
  if (signo != SIGBUS) {
    FATAL("Unexpected signal %d in SigBusAdrAlnWorkaround()", signo);
  }
  if (siginfo->si_code != BUS_ADRALN) {
    FATAL("Unexpected SIGBUS code %d in SigBusAdrAlnWorkaround()", siginfo->si_code);
  }
  ucontext_t* uc = reinterpret_cast<ucontext_t*>(ucontext_raw);
  mcontext_t* mc = reinterpret_cast<mcontext_t*>(&uc->uc_mcontext);
  uint32_t* pc = reinterpret_cast<uint32_t*>(mc->__gregs[REG_PC]);  // `pc` can be unaligned.
  // Currently, the only instruction we need to emulate is `vse64.v vs3, (rs1)`.
  static constexpr uint32_t kFDVStoreOpcode = 0x27u;  // Stores in F, D, V extensions.
  static constexpr uint32_t kVWidth64 = 0x7u;  // With `mew=0` encoded separately in bit 28.
  static constexpr uint32_t kVWidthBitPos = 12u;  // `width[2:0]` is encoded in bits 12-14.
  static constexpr uint32_t kVMDisabled = 1u;  // Unmasked.
  static constexpr uint32_t kVMBitPos = 25u;
  // Other fields are zero: `mop = 0` specifies unit-stride, `sumop = 0` for normal unit-stride
  // store, `nf = 0` for single value moved at each position and `mew = 0` (1 is reserved).
  static constexpr uint32_t kVse64VEncoding =
      kFDVStoreOpcode | (kVWidth64 << kVWidthBitPos) | (kVMDisabled << kVMBitPos);
  // Test all bits except register numbers.
  static constexpr uint32_t kRegMask = 0x1fu;
  static constexpr uint32_t kVs3BitPos = 7u;
  static constexpr uint32_t kRs1BitPos = 15u;
  static constexpr uint32_t kVse64VMask = ~((kRegMask << kVs3BitPos) | (kRegMask << kRs1BitPos));
  if ((*pc & kVse64VMask) == kVse64VEncoding) {
    uint32_t rs1 = (*pc >> kRs1BitPos) & kRegMask;
    uint32_t vs3 = (*pc >> kVs3BitPos) & kRegMask;
    __riscv_v_ext_state* v_state = reinterpret_cast<__riscv_v_ext_state*>(uc + 1);
    uint64_t* src = reinterpret_cast<uint64_t*>(
        reinterpret_cast<uint8_t*>(v_state->datap) + vs3 * v_state->vlenb);
    uint64_t* dest = reinterpret_cast<uint64_t*>(mc->__gregs[rs1]);
    for (size_t i = v_state->vstart, end = v_state->vl; i != end; ++i) {
      uint64_t value = src[i];
      asm volatile ("sd %1, (%0)" : : "r"(&dest[i]), "r"(value) : "memory");
    }
    v_state->vstart = 0u;
    mc->__gregs[REG_PC] = reinterpret_cast<uintptr_t>(pc + 1);
  } else {
    FATAL("SIGBUS/BUS_ADRALN without workaround: 0x%08x", *pc);
  }
}

// Install the workaround before we reach `main()` to support gtests that do not
// create a `Runtime` or install another SIGBUS handler. If the `Runtime` installs
// another SIGBUS handler, it must defer to the workaround handler for BUS_ADRALN.
__attribute__((constructor)) static void InitializeSigBusAdrAlnWorkaround() {
  // Install workaround signal handler for SIGBUS.
  struct sigaction old_action = {};
  struct sigaction action = {};
  action.sa_sigaction = SigBusAdrAlnWorkaround;
  action.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
  int result = sigaction(SIGBUS, &action, &old_action);
  if (result != 0) {
    FATAL("Failed to install SIGBUS/BUS_ADRALN workaround: %d", result);
  }
}
#endif  // defined(ART_TEST_ON_SBC_RISCV64_V_ADRALN_WORKAROUND)

}  // namespace riscv64
}   // namespace art

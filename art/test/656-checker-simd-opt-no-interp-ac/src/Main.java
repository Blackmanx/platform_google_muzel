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

/**
 * Tests for SIMD related optimizations.
 */
public class Main {
  /// CHECK-START: long Main.$noinline$longInductionReduction_ManyTrips(long[]) loop_optimization (before)
  /// CHECK-DAG: <<L0:j\d+>>    LongConstant 0             loop:none
  /// CHECK-DAG: <<L1:j\d+>>    LongConstant 1             loop:none
  /// CHECK-DAG: <<I0:i\d+>>    IntConstant 0              loop:none
  /// CHECK-DAG: <<Get:j\d+>>   ArrayGet [{{l\d+}},<<I0>>] loop:none
  /// CHECK-DAG: <<Phi1:j\d+>>  Phi [<<L0>>,<<Add1:j\d+>>] loop:<<Loop:B\d+>> outer_loop:none
  /// CHECK-DAG: <<Phi2:j\d+>>  Phi [<<L1>>,<<Add2:j\d+>>] loop:<<Loop>>      outer_loop:none
  /// CHECK-DAG: <<Add2>>       Add [<<Phi2>>,<<Get>>]     loop:<<Loop>>      outer_loop:none
  /// CHECK-DAG: <<Add1>>       Add [<<Phi1>>,<<L1>>]      loop:<<Loop>>      outer_loop:none
  //
  /// CHECK-START-{X86_64,ARM64}: long Main.$noinline$longInductionReduction_ManyTrips(long[]) loop_optimization (after)
  /// CHECK-DAG: <<L0:j\d+>>    LongConstant 0               loop:none
  /// CHECK-DAG: <<L1:j\d+>>    LongConstant 1               loop:none
  /// CHECK-DAG: <<I0:i\d+>>    IntConstant 0                loop:none
  /// CHECK-DAG: <<Get:j\d+>>   ArrayGet [{{l\d+}},<<I0>>]   loop:none
  /// CHECK-IF:     hasIsaFeature("sve") and os.environ.get('ART_FORCE_TRY_PREDICATED_SIMD') == 'true'
  //
  ///     CHECK-DAG: <<Rep:d\d+>>   VecReplicateScalar [<<Get>>,{{j\d+}}]  loop:none
  ///     CHECK-DAG: <<Set:d\d+>>   VecSetScalars [<<L1>>,{{j\d+}}]        loop:none
  ///     CHECK-DAG: <<Phi1:j\d+>>  Phi [<<L0>>,{{j\d+}}]                  loop:<<Loop:B\d+>> outer_loop:none
  ///     CHECK-DAG: <<Phi2:d\d+>>  Phi [<<Set>>,{{d\d+}}]                 loop:<<Loop>>      outer_loop:none
  ///     CHECK-DAG: <<LoopP:j\d+>> VecPredWhile [<<Phi1>>,{{j\d+}}]       loop:<<Loop>>      outer_loop:none
  ///     CHECK-DAG:                VecAdd [<<Phi2>>,<<Rep>>,<<LoopP>>]    loop:<<Loop>>      outer_loop:none
  ///     CHECK-DAG:                Add [<<Phi1>>,{{j\d+}}]                loop:<<Loop>>      outer_loop:none
  //
  /// CHECK-ELSE:
  //
  ///     CHECK-DAG: <<L2:j\d+>>    LongConstant 2               loop:none
  ///     CHECK-DAG: <<Rep:d\d+>>   VecReplicateScalar [<<Get>>] loop:none
  ///     CHECK-DAG: <<Set:d\d+>>   VecSetScalars [<<L1>>]       loop:none
  ///     CHECK-DAG: <<Phi1:j\d+>>  Phi [<<L0>>,{{j\d+}}]        loop:<<Loop:B\d+>> outer_loop:none
  ///     CHECK-DAG: <<Phi2:d\d+>>  Phi [<<Set>>,{{d\d+}}]       loop:<<Loop>>      outer_loop:none
  ///     CHECK-DAG:                VecAdd [<<Phi2>>,<<Rep>>]    loop:<<Loop>>      outer_loop:none
  ///     CHECK-DAG:                Add [<<Phi1>>,<<L2>>]        loop:<<Loop>>      outer_loop:none
  //
  /// CHECK-FI:
  // Similar as `longInductionReduction` from `656-checker-simd-opt`, but with a trip count bigger than uint32_t
  static long $noinline$longInductionReduction_ManyTrips(long[] y) {
    long x = 1;
    long trips = (1L << 33);
    for (long i = 0; i < trips; i++) {
      x += y[0];
    }
    return x;
  }

  static void testTypes() {
    long[] l = { 3 };
    expectEquals(25769803777L, $noinline$longInductionReduction_ManyTrips(l));
  }

  public static void main(String[] args) {
    testTypes();
    System.out.println("passed");
  }

  private static void expectEquals(long expected, long result) {
    if (expected != result) {
      throw new Error("Expected: " + expected + ", found: " + result);
    }
  }
}

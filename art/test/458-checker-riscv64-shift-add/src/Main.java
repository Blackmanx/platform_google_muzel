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

public class Main {
  public static void assertIntEquals(int expected, int result) {
    if (expected != result) {
      throw new Error("Expected: " + expected + ", found: " + result);
    }
  }

  public static void assertLongEquals(long expected, long result) {
    if (expected != result) {
      throw new Error("Expected: " + expected + ", found: " + result);
    }
  }

  /// CHECK-START-RISCV64: int Main.$noinline$intRiscvShift1Add(int, int) instruction_simplifier_riscv64 (before)
  /// CHECK:          <<A:i\d+>>        ParameterValue
  /// CHECK:          <<B:i\d+>>        ParameterValue
  /// CHECK:          <<One:i\d+>>      IntConstant 1
  /// CHECK:          <<Shift:i\d+>>    Shl [<<A>>,<<One>>]
  /// CHECK:          <<Add:i\d+>>      Add [<<B>>,<<Shift>>]
  /// CHECK:          <<Return:v\d+>>   Return [<<Add>>]

  /// CHECK-START-RISCV64: int Main.$noinline$intRiscvShift1Add(int, int) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Riscv64ShiftAdd

  public static int $noinline$intRiscvShift1Add(int a, int b) {
    return (a << 1) + b;
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longIntRiscvShift1Add(long, int) instruction_simplifier_riscv64 (before)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:i\d+>>        ParameterValue
  /// CHECK:          <<One:i\d+>>      IntConstant 1
  /// CHECK:          <<Shift:j\d+>>    Shl [<<A>>,<<One>>]
  /// CHECK:          <<Convert:j\d+>>  TypeConversion [<<B>>]
  /// CHECK:          <<Add:j\d+>>      Add [<<Shift>>,<<Convert>>]
  /// CHECK:          <<Return:v\d+>>   Return [<<Add>>]

  /// CHECK-START-RISCV64: long Main.$noinline$longIntRiscvShift1Add(long, int) instruction_simplifier_riscv64 (after)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:i\d+>>        ParameterValue
  /// CHECK:          <<Convert:j\d+>>  TypeConversion [<<B>>]
  /// CHECK-DAG:      <<ShiftAdd:j\d+>> Riscv64ShiftAdd [<<A>>,<<Convert>>] distance:1

  /// CHECK-START-RISCV64: long Main.$noinline$longIntRiscvShift1Add(long, int) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Shl

  /// CHECK-START-RISCV64: long Main.$noinline$longIntRiscvShift1Add(long, int) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Add

  public static long $noinline$longIntRiscvShift1Add(long a, int b) {
    return (a << 1) + b;
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longRiscvShift1Add(long, long) instruction_simplifier_riscv64 (before)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:j\d+>>        ParameterValue
  /// CHECK:          <<One:i\d+>>      IntConstant 1
  /// CHECK:          <<Shift:j\d+>>    Shl [<<A>>,<<One>>]
  /// CHECK:          <<Add:j\d+>>      Add [<<B>>,<<Shift>>]
  /// CHECK:          <<Return:v\d+>>   Return [<<Add>>]

  /// CHECK-START-RISCV64: long Main.$noinline$longRiscvShift1Add(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:j\d+>>        ParameterValue
  /// CHECK-DAG:      <<ShiftAdd:j\d+>> Riscv64ShiftAdd [<<A>>,<<B>>] distance:1

  /// CHECK-START-RISCV64: long Main.$noinline$longRiscvShift1Add(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Shl

  /// CHECK-START-RISCV64: long Main.$noinline$longRiscvShift1Add(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Add

  public static long $noinline$longRiscvShift1Add(long a, long b) {
    return (a << 1) + b;
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longRiscvShift2Add(long, long) instruction_simplifier_riscv64 (before)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:j\d+>>        ParameterValue
  /// CHECK:          <<Two:i\d+>>      IntConstant 2
  /// CHECK:          <<Shift:j\d+>>    Shl [<<A>>,<<Two>>]
  /// CHECK:          <<Add:j\d+>>      Add [<<B>>,<<Shift>>]
  /// CHECK:          <<Return:v\d+>>   Return [<<Add>>]

  /// CHECK-START-RISCV64: long Main.$noinline$longRiscvShift2Add(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:j\d+>>        ParameterValue
  /// CHECK-DAG:      <<ShiftAdd:j\d+>> Riscv64ShiftAdd [<<A>>,<<B>>] distance:2

  /// CHECK-START-RISCV64: long Main.$noinline$longRiscvShift2Add(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Shl

  /// CHECK-START-RISCV64: long Main.$noinline$longRiscvShift2Add(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Add

  public static long $noinline$longRiscvShift2Add(long a, long b) {
    return (a << 2) + b;
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longRiscvShift3Add(long, long) instruction_simplifier_riscv64 (before)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:j\d+>>        ParameterValue
  /// CHECK:          <<Three:i\d+>>    IntConstant 3
  /// CHECK:          <<Shift:j\d+>>    Shl [<<A>>,<<Three>>]
  /// CHECK:          <<Add:j\d+>>      Add [<<B>>,<<Shift>>]
  /// CHECK:          <<Return:v\d+>>   Return [<<Add>>]

  /// CHECK-START-RISCV64: long Main.$noinline$longRiscvShift3Add(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:j\d+>>        ParameterValue
  /// CHECK-DAG:      <<ShiftAdd:j\d+>> Riscv64ShiftAdd [<<A>>,<<B>>] distance:3

  /// CHECK-START-RISCV64: long Main.$noinline$longRiscvShift3Add(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Shl

  /// CHECK-START-RISCV64: long Main.$noinline$longRiscvShift3Add(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Add

  public static long $noinline$longRiscvShift3Add(long a, long b) {
    return (a << 3) + b;
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longReverseRiscvShift3Add(long, long) instruction_simplifier_riscv64 (before)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:j\d+>>        ParameterValue
  /// CHECK:          <<Three:i\d+>>    IntConstant 3
  /// CHECK:          <<Shift:j\d+>>    Shl [<<A>>,<<Three>>]
  /// CHECK:          <<Add:j\d+>>      Add [<<B>>,<<Shift>>]
  /// CHECK:          <<Return:v\d+>>   Return [<<Add>>]

  /// CHECK-START-RISCV64: long Main.$noinline$longReverseRiscvShift3Add(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:j\d+>>        ParameterValue
  /// CHECK-DAG:      <<ShiftAdd:j\d+>> Riscv64ShiftAdd [<<A>>,<<B>>] distance:3

  /// CHECK-START-RISCV64: long Main.$noinline$longReverseRiscvShift3Add(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Shl

  /// CHECK-START-RISCV64: long Main.$noinline$longReverseRiscvShift3Add(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Add

  public static long $noinline$longReverseRiscvShift3Add(long a, long b) {
    return b + (a << 3);
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longRightShift(long, long) instruction_simplifier_riscv64 (before)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:j\d+>>        ParameterValue
  /// CHECK:          <<Four:i\d+>>     IntConstant 4
  /// CHECK:          <<One:i\d+>>      IntConstant 1
  /// CHECK:          <<Shift1:j\d+>>   Shl [<<A>>,<<Four>>]
  /// CHECK:          <<Shift2:j\d+>>   Shl [<<B>>,<<One>>]
  /// CHECK:          <<Add:j\d+>>      Add [<<Shift1>>,<<Shift2>>]
  /// CHECK:          <<Return:v\d+>>   Return [<<Add>>]

  /// CHECK-START-RISCV64: long Main.$noinline$longRightShift(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:j\d+>>        ParameterValue
  /// CHECK:          <<Four:i\d+>>     IntConstant 4
  /// CHECK:          <<Shift1:j\d+>>   Shl [<<A>>,<<Four>>]
  /// CHECK-DAG:      <<ShiftAdd:j\d+>> Riscv64ShiftAdd [<<B>>,<<Shift1>>] distance:1

  /// CHECK-START-RISCV64: long Main.$noinline$longRightShift(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Add

  public static long $noinline$longRightShift(long a, long b) {
    return (a << 4) + (b << 1);
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longLeftShift(long, long) instruction_simplifier_riscv64 (before)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:j\d+>>        ParameterValue
  /// CHECK:          <<One:i\d+>>      IntConstant 1
  /// CHECK:          <<Four:i\d+>>     IntConstant 4
  /// CHECK:          <<Shift1:j\d+>>   Shl [<<A>>,<<One>>]
  /// CHECK:          <<Shift2:j\d+>>   Shl [<<B>>,<<Four>>]
  /// CHECK:          <<Add:j\d+>>      Add [<<Shift1>>,<<Shift2>>]
  /// CHECK:          <<Return:v\d+>>   Return [<<Add>>]

  /// CHECK-START-RISCV64: long Main.$noinline$longLeftShift(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK:          <<A:j\d+>>        ParameterValue
  /// CHECK:          <<B:j\d+>>        ParameterValue
  /// CHECK:          <<Four:i\d+>>     IntConstant 4
  /// CHECK:          <<Shift2:j\d+>>   Shl [<<B>>,<<Four>>]
  /// CHECK-DAG:      <<ShiftAdd:j\d+>> Riscv64ShiftAdd [<<A>>,<<Shift2>>] distance:1

  /// CHECK-START-RISCV64: long Main.$noinline$longLeftShift(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Add

  public static long $noinline$longLeftShift(long a, long b) {
    return (a << 1) + (b << 4);
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longShiftTooLittleDistance(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Riscv64ShiftAdd

  public static long $noinline$longShiftTooLittleDistance(long a, long b) {
    return (a << 0) + b;
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longShiftTooGreatDistance(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Riscv64ShiftAdd

  public static long $noinline$longShiftTooGreatDistance(long a, long b) {
    return (a << 4) + b;
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longTwoSimplifications(long, long) instruction_simplifier_riscv64 (before)
  /// CHECK:          <<A:j\d+>>         ParameterValue
  /// CHECK:          <<B:j\d+>>         ParameterValue
  /// CHECK:          <<One:i\d+>>       IntConstant 1
  /// CHECK:          <<Two:i\d+>>       IntConstant 2
  /// CHECK:          <<Shift1:j\d+>>    Shl [<<A>>,<<One>>]
  /// CHECK:          <<Add1:j\d+>>      Add [<<B>>,<<Shift1>>]
  /// CHECK:          <<Shift2:j\d+>>    Shl [<<B>>,<<Two>>]
  /// CHECK:          <<Add2:j\d+>>      Add [<<A>>,<<Shift2>>]
  /// CHECK:          <<Xor:j\d+>>       Xor [<<Add1>>,<<Add2>>]

  /// CHECK-START-RISCV64: long Main.$noinline$longTwoSimplifications(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-DAG:      <<ShiftAdd1:j\d+>> Riscv64ShiftAdd distance:1
  /// CHECK-DAG:      <<ShiftAdd2:j\d+>> Riscv64ShiftAdd distance:2

  /// CHECK-START-RISCV64: long Main.$noinline$longTwoSimplifications(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                         Shl

  /// CHECK-START-RISCV64: long Main.$noinline$longTwoSimplifications(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                         Add

  public static long $noinline$longTwoSimplifications(long a, long b) {
    long x = b + (a << 1);
    long y = a + (b << 2);
    return x ^ y;
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longTwoAddsUseShl(long, long, long) instruction_simplifier_riscv64 (before)
  /// CHECK:          <<A:j\d+>>         ParameterValue
  /// CHECK:          <<B:j\d+>>         ParameterValue
  /// CHECK:          <<C:j\d+>>         ParameterValue
  /// CHECK:          <<One:i\d+>>       IntConstant 1
  /// CHECK:          <<Shl:j\d+>>       Shl [<<A>>,<<One>>]
  /// CHECK:          <<X:j\d+>>         Add [<<B>>,<<Shl>>]
  /// CHECK:          <<Y:j\d+>>         Add [<<C>>,<<Shl>>]

  /// CHECK-START-RISCV64: long Main.$noinline$longTwoAddsUseShl(long, long, long) instruction_simplifier_riscv64 (after)
  /// CHECK:          <<A:j\d+>>         ParameterValue
  /// CHECK:          <<B:j\d+>>         ParameterValue
  /// CHECK:          <<C:j\d+>>         ParameterValue
  /// CHECK-DAG:      <<ShAdd1:j\d+>>    Riscv64ShiftAdd [<<A>>,<<B>>] distance:1
  /// CHECK-DAG:      <<ShAdd2:j\d+>>    Riscv64ShiftAdd [<<A>>,<<C>>] distance:1

  /// CHECK-START-RISCV64: long Main.$noinline$longTwoAddsUseShl(long, long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                         Shl

  /// CHECK-START-RISCV64: long Main.$noinline$longTwoAddsUseShl(long, long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                         Add

  public static long $noinline$longTwoAddsUseShl(long a, long b, long c) {
    long shl = a << 1;
    long x = shl + b;
    long y = shl + c;
    return x ^ y;
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longTwoAddsMixedOrderUseShl(long, long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-DAG:                         Riscv64ShiftAdd
  /// CHECK-DAG:                         Riscv64ShiftAdd

  /// CHECK-START-RISCV64: long Main.$noinline$longTwoAddsMixedOrderUseShl(long, long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                         Shl

  /// CHECK-START-RISCV64: long Main.$noinline$longTwoAddsMixedOrderUseShl(long, long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                         Add

  public static long $noinline$longTwoAddsMixedOrderUseShl(long a, long b, long c) {
    long x = (a << 1) + b;
    long y = c + (a << 1);
    return x ^ y;
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longOneAddSharesShlUse(long, long, long) instruction_simplifier_riscv64 (before)
  /// CHECK:          <<A:j\d+>>         ParameterValue
  /// CHECK:          <<B:j\d+>>         ParameterValue
  /// CHECK:          <<C:j\d+>>         ParameterValue
  /// CHECK:          <<One:i\d+>>       IntConstant 1
  /// CHECK:          <<Shl:j\d+>>       Shl [<<A>>,<<One>>]
  /// CHECK:          <<X:j\d+>>         Add [<<B>>,<<Shl>>]
  /// CHECK:          <<Y:j\d+>>         Sub [<<Shl>>,<<C>>]

  /// CHECK-START-RISCV64: long Main.$noinline$longOneAddSharesShlUse(long, long, long) instruction_simplifier_riscv64 (after)
  /// CHECK:          <<A:j\d+>>         ParameterValue
  /// CHECK:          <<B:j\d+>>         ParameterValue
  /// CHECK:          <<C:j\d+>>         ParameterValue
  /// CHECK:          <<One:i\d+>>       IntConstant 1
  /// CHECK:          <<Shl:j\d+>>       Shl [<<A>>,<<One>>]
  /// CHECK-DAG:      <<ShAdd:j\d+>>     Riscv64ShiftAdd [<<A>>,<<B>>] distance:1
  /// CHECK:          <<Y:j\d+>>         Sub [<<Shl>>,<<C>>]

  /// CHECK-START-RISCV64: long Main.$noinline$longOneAddSharesShlUse(long, long, long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                         Add

  public static long $noinline$longOneAddSharesShlUse(long a, long b, long c) {
    long shl = a << 1;
    long x = shl + b;
    long y = shl - c;
    return x ^ y;
  }

  public static void $noinline$returnVoid(long a) {}

  /// CHECK-START-RISCV64: long Main.$noinline$longOneAddSharesShlEnvironmentUse(long, long) instruction_simplifier_riscv64 (after)
  /// CHECK:                             Shl
  /// CHECK-DAG:                         Riscv64ShiftAdd

  public static long $noinline$longOneAddSharesShlEnvironmentUse(long a, long b) {
    long shl = a << 1;
    long x = shl + b;
    $noinline$returnVoid(shl);
    return x;
  }

  /// CHECK-START-RISCV64: long Main.$noinline$longTwoTheSameShl(long) instruction_simplifier_riscv64 (after)
  /// CHECK-NOT:                        Riscv64ShiftAdd

  public static long $noinline$longTwoTheSameShl(long a) {
    return (a << 1) + (a << 1);
  }

  public static void main(String[] args) {
    assertIntEquals(0, $noinline$intRiscvShift1Add(0, 0));

    assertLongEquals(-3L, $noinline$longIntRiscvShift1Add(-1L, -1));
    assertLongEquals(3L, $noinline$longIntRiscvShift1Add(1L, 1));

    assertLongEquals(-3L, $noinline$longRiscvShift1Add(-1L, -1L));
    assertLongEquals(-1L, $noinline$longRiscvShift1Add(0L, -1L));
    assertLongEquals(-2L, $noinline$longRiscvShift1Add(-1L, 0L));
    assertLongEquals(0L, $noinline$longRiscvShift1Add(0L, 0L));
    assertLongEquals(1L, $noinline$longRiscvShift1Add(0L, 1L));
    assertLongEquals(2L, $noinline$longRiscvShift1Add(1L, 0L));
    assertLongEquals(3L, $noinline$longRiscvShift1Add(1L, 1L));

    assertLongEquals(-1L, $noinline$longRiscvShift1Add(Long.MAX_VALUE, 1L));

    assertLongEquals(0L, $noinline$longRiscvShift2Add(0L, 0L));
    assertLongEquals(1L, $noinline$longRiscvShift2Add(0L, 1L));
    assertLongEquals(4L, $noinline$longRiscvShift2Add(1L, 0L));
    assertLongEquals(5L, $noinline$longRiscvShift2Add(1L, 1L));

    assertLongEquals(0L, $noinline$longRiscvShift3Add(0L, 0L));
    assertLongEquals(0L, $noinline$longReverseRiscvShift3Add(0L, 0L));
    assertLongEquals(1L, $noinline$longRiscvShift3Add(0L, 1L));
    assertLongEquals(1L, $noinline$longReverseRiscvShift3Add(0L, 1L));
    assertLongEquals(8L, $noinline$longRiscvShift3Add(1L, 0L));
    assertLongEquals(8L, $noinline$longReverseRiscvShift3Add(1L, 0L));
    assertLongEquals(9L, $noinline$longRiscvShift3Add(1L, 1L));
    assertLongEquals(9L, $noinline$longReverseRiscvShift3Add(1L, 1L));

    assertLongEquals(2L, $noinline$longRightShift(0L, 1L));
    assertLongEquals(4L, $noinline$longRightShift(0L, 2L));

    assertLongEquals(2L, $noinline$longLeftShift(1L, 0L));
    assertLongEquals(4L, $noinline$longLeftShift(2L, 0L));

    assertLongEquals(1L, $noinline$longShiftTooLittleDistance(1L, 0L));
    assertLongEquals(2L, $noinline$longShiftTooLittleDistance(2L, 0L));

    assertLongEquals(17L, $noinline$longShiftTooGreatDistance(1L, 1L));
    assertLongEquals(32L, $noinline$longShiftTooGreatDistance(2L, 0L));

    assertLongEquals(6L, $noinline$longTwoSimplifications(1L, 1L));

    assertLongEquals(0L, $noinline$longTwoAddsUseShl(1L, 1L, 1L));
    assertLongEquals(1L, $noinline$longTwoAddsUseShl(1L, 0L, 1L));
    assertLongEquals(3L, $noinline$longTwoAddsUseShl(0L, 1L, 2L));
    assertLongEquals(7L, $noinline$longTwoAddsUseShl(1L, 2L, 1L));

    assertLongEquals(0L, $noinline$longTwoAddsMixedOrderUseShl(1L, 1L, 1L));
    assertLongEquals(1L, $noinline$longTwoAddsMixedOrderUseShl(1L, 0L, 1L));
    assertLongEquals(3L, $noinline$longTwoAddsMixedOrderUseShl(0L, 1L, 2L));

    assertLongEquals(2L, $noinline$longOneAddSharesShlUse(1L, 1L, 1L));
    assertLongEquals(3L, $noinline$longOneAddSharesShlUse(1L, 0L, 1L));
    assertLongEquals(-1L, $noinline$longOneAddSharesShlUse(0L, 1L, 2L));
    assertLongEquals(5L, $noinline$longOneAddSharesShlUse(1L, 2L, 1L));

    assertLongEquals(3L, $noinline$longOneAddSharesShlEnvironmentUse(1L, 1L));

    assertLongEquals(4L, $noinline$longTwoTheSameShl(1L));
  }
}

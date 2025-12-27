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

import java.lang.reflect.Method;

public class Main {
    public static void main(String[] args) throws Throwable {
        assertLongEquals(11L, $noinline$testIntToLong(0, 1));
        assertLongEquals(12L, $noinline$testIntToLong(1, 0));

        assertFloatEquals(11f, $noinline$testIntToFloat(0, 1));
        assertFloatEquals(12f, $noinline$testIntToFloat(1, 0));

        assertIntEquals(11, $noinline$testIntToByte(0, 1));
        assertIntEquals(12, $noinline$testIntToByte(1, 0));

        assertIntEquals(11, $noinline$testLongToInt(0, 1));
        assertIntEquals(12, $noinline$testLongToInt(1, 0));

        assertIntEquals(-2, $noinline$testTwoSelects_SameSelect(false));
        assertIntEquals(2, $noinline$testTwoSelects_SameSelect(true));

        assertIntEquals(8, $noinline$testTwoSelects_DifferentSelect_UpdateLeft(false));
        assertIntEquals(3, $noinline$testTwoSelects_DifferentSelect_UpdateLeft(true));

        assertIntEquals(8, $noinline$testTwoSelects_DifferentSelect_UpdateRight(false));
        assertIntEquals(3, $noinline$testTwoSelects_DifferentSelect_UpdateRight(true));

        assertIntEquals(
                24, $noinline$testTwoSelects_DifferentSelect_UpdateLeft_MultipleRightUses(false));
        assertIntEquals(
                12, $noinline$testTwoSelects_DifferentSelect_UpdateLeft_MultipleRightUses(true));

        assertIntEquals(
                12, $noinline$testTwoSelects_DifferentSelect_UpdateRight_MultipleLeftUses(false));
        assertIntEquals(
                4, $noinline$testTwoSelects_DifferentSelect_UpdateRight_MultipleLeftUses(true));
    }

    /// CHECK-START: long Main.$noinline$testIntToLong(int, int) control_flow_simplifier (after)
    /// CHECK:     <<Const10:j\d+>> LongConstant 10
    /// CHECK:     <<Const1:i\d+>>  IntConstant 1
    /// CHECK:     <<Const2:i\d+>>  IntConstant 2
    /// CHECK:     <<Sel:i\d+>>     Select [<<Const1>>,<<Const2>>,<<Condition:z\d+>>]
    /// CHECK:     <<Type:j\d+>>    TypeConversion [<<Sel>>]
    /// CHECK:                      Add [<<Type>>,<<Const10>>]

    /// CHECK-START: long Main.$noinline$testIntToLong(int, int) constant_folding$after_gvn (after)
    /// CHECK:     <<Const11:j\d+>> LongConstant 11
    /// CHECK:     <<Const12:j\d+>> LongConstant 12
    /// CHECK:                      Select [<<Const11>>,<<Const12>>,<<Condition:z\d+>>]

    /// CHECK-START: long Main.$noinline$testIntToLong(int, int) constant_folding$after_gvn (after)
    /// CHECK-NOT:                 TypeConversion
    /// CHECK-NOT:                 Add
    private static long $noinline$testIntToLong(int a, int b) {
        long result = 10;
        int c = 1;
        int d = 2;
        return result + (a < b ? c : d);
    }

    /// CHECK-START: float Main.$noinline$testIntToFloat(int, int) control_flow_simplifier (after)
    /// CHECK:     <<Const10:f\d+>> FloatConstant 10
    /// CHECK:     <<Const1:i\d+>>  IntConstant 1
    /// CHECK:     <<Const2:i\d+>>  IntConstant 2
    /// CHECK:     <<Sel:i\d+>>     Select [<<Const1>>,<<Const2>>,<<Condition:z\d+>>]
    /// CHECK:     <<Type:f\d+>>    TypeConversion [<<Sel>>]
    /// CHECK:                      Add [<<Type>>,<<Const10>>]

    /// CHECK-START: float Main.$noinline$testIntToFloat(int, int) constant_folding$after_gvn (after)
    /// CHECK:     <<Const11:f\d+>> FloatConstant 11
    /// CHECK:     <<Const12:f\d+>> FloatConstant 12
    /// CHECK:                      Select [<<Const11>>,<<Const12>>,<<Condition:z\d+>>]

    /// CHECK-START: float Main.$noinline$testIntToFloat(int, int) constant_folding$after_gvn (after)
    /// CHECK-NOT:                 TypeConversion
    /// CHECK-NOT:                 Add
    private static float $noinline$testIntToFloat(int a, int b) {
        float result = 10;
        int c = 1;
        int d = 2;
        return result + (a < b ? c : d);
    }

    /// CHECK-START: byte Main.$noinline$testIntToByte(int, int) control_flow_simplifier (after)
    /// CHECK:     <<Const10:i\d+>>  IntConstant 10
    /// CHECK:     <<Const257:i\d+>> IntConstant 257
    /// CHECK:     <<Const258:i\d+>> IntConstant 258
    /// CHECK:     <<Sel:i\d+>>      Select [<<Const257>>,<<Const258>>,<<Condition:z\d+>>]
    /// CHECK:     <<Type:b\d+>>     TypeConversion [<<Sel>>]
    /// CHECK:                       Add [<<Type>>,<<Const10>>]

    /// CHECK-START: byte Main.$noinline$testIntToByte(int, int) constant_folding$after_gvn (after)
    /// CHECK:     <<Const11:i\d+>>   IntConstant 11
    /// CHECK:     <<Const12:i\d+>>   IntConstant 12
    /// CHECK:                        Select [<<Const11>>,<<Const12>>,<<Condition:z\d+>>]

    /// CHECK-START: byte Main.$noinline$testIntToByte(int, int) constant_folding$after_gvn (after)
    /// CHECK-NOT:                   TypeConversion
    /// CHECK-NOT:                   Add
    private static byte $noinline$testIntToByte(int a, int b) {
        byte result = 10;
        int c = 257; // equal to 1 in byte
        int d = 258; // equal to 2 in byte
        // Due to `+` operating in ints, we need to do an extra cast. We can optimize away both type
        // conversions.
        return (byte) (result + (byte) (a < b ? c : d));
    }

    /// CHECK-START: int Main.$noinline$testLongToInt(int, int) control_flow_simplifier (after)
    /// CHECK:     <<Const10:i\d+>> IntConstant 10
    /// CHECK:     <<Const1:j\d+>>  LongConstant 4294967297
    /// CHECK:     <<Const2:j\d+>>  LongConstant 4294967298
    /// CHECK:     <<Sel:j\d+>>     Select [<<Const1>>,<<Const2>>,<<Condition:z\d+>>]
    /// CHECK:     <<Type:i\d+>>    TypeConversion [<<Sel>>]
    /// CHECK:                      Add [<<Type>>,<<Const10>>]

    /// CHECK-START: int Main.$noinline$testLongToInt(int, int) constant_folding$after_gvn (after)
    /// CHECK:     <<Const11:i\d+>> IntConstant 11
    /// CHECK:     <<Const12:i\d+>> IntConstant 12
    /// CHECK:                      Select [<<Const11>>,<<Const12>>,<<Condition:z\d+>>]

    /// CHECK-START: int Main.$noinline$testLongToInt(int, int) constant_folding$after_gvn (after)
    /// CHECK-NOT:                 TypeConversion
    /// CHECK-NOT:                 Add
    private static int $noinline$testLongToInt(int a, int b) {
        int result = 10;
        long c = (1L << 32) + 1L; // Will be 1, when cast to int
        long d = (1L << 32) + 2L; // Will be 2, when cast to int
        return result + (int) (a < b ? c : d);
    }

    /// CHECK-START: int Main.$noinline$testTwoSelects_SameSelect(boolean) control_flow_simplifier (after)
    /// CHECK:     <<ConstMinus1:i\d+>> IntConstant -1
    /// CHECK:     <<Const1:i\d+>>      IntConstant 1
    /// CHECK:     <<Sel:i\d+>>         Select [<<ConstMinus1>>,<<Const1>>,<<Condition:z\d+>>]
    /// CHECK:                          Add [<<Sel>>,<<Sel>>]

    /// CHECK-START: int Main.$noinline$testTwoSelects_SameSelect(boolean) constant_folding$after_gvn (after)
    /// CHECK:     <<ConstMinus2:i\d+>> IntConstant -2
    /// CHECK:     <<Const2:i\d+>>      IntConstant 2
    /// CHECK:     <<Sel:i\d+>>         Select [<<ConstMinus2>>,<<Const2>>,<<Condition:z\d+>>]
    /// CHECK:                          Return [<<Sel>>]

    /// CHECK-START: int Main.$noinline$testTwoSelects_SameSelect(boolean) constant_folding$after_gvn (after)
    /// CHECK-NOT:                      Add
    private static int $noinline$testTwoSelects_SameSelect(boolean b) {
        int x = b ? 1 : -1;
        return x + x;
    }

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateLeft(boolean) constant_folding$after_gvn (before)
    /// CHECK-DAG: <<Const1:i\d+>>      IntConstant 1
    /// CHECK-DAG: <<Const2:i\d+>>      IntConstant 2
    /// CHECK-DAG: <<Const3:i\d+>>      IntConstant 3
    /// CHECK-DAG: <<Const4:i\d+>>      IntConstant 4
    /// CHECK-DAG: <<Sel1:i\d+>>        Select [<<Const2>>,<<Const1>>,<<Condition:z\d+>>]
    /// CHECK-DAG: <<Sel2:i\d+>>        Select [<<Const4>>,<<Const3>>,<<Condition>>]
    /// CHECK-DAG:                      Mul [<<Sel1>>,<<Sel2>>]

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateLeft(boolean) constant_folding$after_gvn (after)
    /// CHECK-DAG: <<Const3:i\d+>>      IntConstant 3
    /// CHECK-DAG: <<Const4:i\d+>>      IntConstant 4
    /// CHECK-DAG: <<Const8:i\d+>>      IntConstant 8
    /// CHECK-DAG: <<Sel:i\d+>>         Select [<<Const8>>,<<Const3>>,<<Condition:z\d+>>]
    /// CHECK-DAG:                      Select [<<Const4>>,<<Const3>>,<<Condition>>]
    /// CHECK-DAG:                      Return [<<Sel>>]

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateLeft(boolean) constant_folding$after_gvn (after)
    /// CHECK-NOT:                      Mul

    // We can eliminate the unused Select in DCE.
    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateLeft(boolean) dead_code_elimination$after_gvn (after)
    /// CHECK:                          Select
    /// CHECK-NOT:                      Select
    private static int $noinline$testTwoSelects_DifferentSelect_UpdateLeft(boolean b) {
        int x = b ? 1 : 2;
        int y = b ? 3 : 4;
        return x * y;
    }

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateRight(boolean) constant_folding$after_gvn (before)
    /// CHECK-DAG: <<Const1:i\d+>>      IntConstant 1
    /// CHECK-DAG: <<Const2:i\d+>>      IntConstant 2
    /// CHECK-DAG: <<Const3:i\d+>>      IntConstant 3
    /// CHECK-DAG: <<Const4:i\d+>>      IntConstant 4
    /// CHECK-DAG: <<Sel1:i\d+>>        Select [<<Const2>>,<<Const1>>,<<Condition:z\d+>>]
    /// CHECK-DAG: <<Sel2:i\d+>>        Select [<<Const4>>,<<Const3>>,<<Condition>>]
    /// CHECK-DAG:                      Mul [<<Sel1>>,<<Sel2>>]

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateRight(boolean) constant_folding$after_gvn (after)
    /// CHECK-DAG: <<Const1:i\d+>>      IntConstant 1
    /// CHECK-DAG: <<Const2:i\d+>>      IntConstant 2
    /// CHECK-DAG: <<Const3:i\d+>>      IntConstant 3
    /// CHECK-DAG: <<Const4:i\d+>>      IntConstant 4
    /// CHECK-DAG: <<Const8:i\d+>>      IntConstant 8
    /// CHECK-DAG: <<Sel:i\d+>>         Select [<<Const8>>,<<Const3>>,<<Condition:z\d+>>]
    /// CHECK-DAG:                      Select [<<Const2>>,<<Const1>>,<<Condition>>]
    /// CHECK-DAG:                      Return [<<Sel>>]

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateRight(boolean) constant_folding$after_gvn (after)
    /// CHECK-NOT:                      Mul

    // Can't eliminate the unused Select in DCE since it is used in the $noinline$nop environment.
    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateRight(boolean) dead_code_elimination$after_gvn (after)
    /// CHECK:                          Select
    /// CHECK:                          Select
    private static int $noinline$testTwoSelects_DifferentSelect_UpdateRight(boolean b) {
        int x = b ? 1 : 2;
        $noinline$nop();
        int y = b ? 3 : 4;
        return x * y;
    }

    private static void $noinline$nop() {}

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateLeft_MultipleRightUses(boolean) constant_folding$after_gvn (before)
    /// CHECK-DAG: <<Const1:i\d+>>      IntConstant 1
    /// CHECK-DAG: <<Const2:i\d+>>      IntConstant 2
    /// CHECK-DAG: <<Const3:i\d+>>      IntConstant 3
    /// CHECK-DAG: <<Const4:i\d+>>      IntConstant 4
    /// CHECK-DAG: <<Sel1:i\d+>>        Select [<<Const2>>,<<Const1>>,<<Condition:z\d+>>]
    /// CHECK-DAG: <<Sel2:i\d+>>        Select [<<Const4>>,<<Const3>>,<<Condition>>]
    /// CHECK-DAG: <<Mul1:i\d+>>        Mul [<<Sel1>>,<<Sel2>>]
    /// CHECK-DAG: <<Mul2:i\d+>>        Mul [<<Sel2>>,<<Sel2>>]
    /// CHECK-DAG:                      Add [<<Mul1>>,<<Mul2>>]

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateLeft_MultipleRightUses(boolean) constant_folding$after_gvn (after)
    /// CHECK-DAG: <<Const9:i\d+>>      IntConstant 9
    /// CHECK-DAG: <<Const12:i\d+>>     IntConstant 12
    /// CHECK-DAG: <<Const16:i\d+>>     IntConstant 16
    /// CHECK-DAG: <<Const24:i\d+>>     IntConstant 24
    /// CHECK-DAG: <<Sel1:i\d+>>        Select [<<Const24>>,<<Const12>>,<<Condition:z\d+>>]
    /// CHECK-DAG:                      Select [<<Const16>>,<<Const9>>,<<Condition>>]
    /// CHECK-DAG:                      Return [<<Sel1>>]

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateLeft_MultipleRightUses(boolean) constant_folding$after_gvn (after)
    /// CHECK-NOT:                      Mul

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateLeft_MultipleRightUses(boolean) constant_folding$after_gvn (after)
    /// CHECK-NOT:                      Add

    // We can eliminate the unused Select in DCE.
    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateLeft_MultipleRightUses(boolean) dead_code_elimination$after_gvn (after)
    /// CHECK:                          Select
    /// CHECK-NOT:                      Select
    private static int $noinline$testTwoSelects_DifferentSelect_UpdateLeft_MultipleRightUses(
            boolean b) {
        int x = b ? 1 : 2;
        int y = b ? 3 : 4;
        return y * x + y * y;
    }

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateRight_MultipleLeftUses(boolean) constant_folding$after_gvn (before)
    /// CHECK-DAG: <<Const1:i\d+>>      IntConstant 1
    /// CHECK-DAG: <<Const2:i\d+>>      IntConstant 2
    /// CHECK-DAG: <<Const3:i\d+>>      IntConstant 3
    /// CHECK-DAG: <<Const4:i\d+>>      IntConstant 4
    /// CHECK-DAG: <<Sel1:i\d+>>        Select [<<Const2>>,<<Const1>>,<<Condition:z\d+>>]
    /// CHECK-DAG: <<Sel2:i\d+>>        Select [<<Const4>>,<<Const3>>,<<Condition>>]
    /// CHECK-DAG: <<Mul1:i\d+>>        Mul [<<Sel1>>,<<Sel2>>]
    /// CHECK-DAG: <<Mul2:i\d+>>        Mul [<<Sel1>>,<<Sel1>>]
    /// CHECK-DAG:                      Add [<<Mul1>>,<<Mul2>>]

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateRight_MultipleLeftUses(boolean) constant_folding$after_gvn (after)
    /// CHECK-DAG: <<Const1:i\d+>>      IntConstant 1
    /// CHECK-DAG: <<Const4:i\d+>>      IntConstant 4
    /// CHECK-DAG: <<Const12:i\d+>>     IntConstant 12
    /// CHECK-DAG: <<Sel1:i\d+>>        Select [<<Const4>>,<<Const1>>,<<Condition:z\d+>>]
    /// CHECK-DAG: <<Sel2:i\d+>>        Select [<<Const12>>,<<Const4>>,<<Condition>>]
    /// CHECK-DAG:                      Return [<<Sel2>>]

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateRight_MultipleLeftUses(boolean) constant_folding$after_gvn (after)
    /// CHECK-NOT:                      Mul

    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateRight_MultipleLeftUses(boolean) constant_folding$after_gvn (after)
    /// CHECK-NOT:                      Add

    // We can eliminate the unused Select in DCE.
    /// CHECK-START: int Main.$noinline$testTwoSelects_DifferentSelect_UpdateRight_MultipleLeftUses(boolean) dead_code_elimination$after_gvn (after)
    /// CHECK:                          Select
    /// CHECK-NOT:                      Select
    private static int $noinline$testTwoSelects_DifferentSelect_UpdateRight_MultipleLeftUses(
            boolean b) {
        int x = b ? 1 : 2;
        int y = b ? 3 : 4;
        return x * y + x * x;
    }

    private static void assertIntEquals(int expected, int actual) {
        if (expected != actual) {
            throw new AssertionError("Expected " + expected + " got " + actual);
        }
    }

    private static void assertLongEquals(long expected, long actual) {
        if (expected != actual) {
            throw new AssertionError("Expected " + expected + " got " + actual);
        }
    }

    private static void assertFloatEquals(float expected, float actual) {
        if (expected != actual) {
            throw new AssertionError("Expected " + expected + " got " + actual);
        }
    }
}

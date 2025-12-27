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
    // Numbers have to be inlined for the bug to trigger: we cannot define this as a generic method
    // that takes e.g. 12 as a parameter. We can define some methods to test different cases. Each
    // method is the same as doing:
    //   result = initial_value;
    //   for (int i = start; i < end; ++i) { result += i * 10; }
    //   return result;
    // As long as `end` is 5 or less

    private static int $noinline$nestedLoopCalculation_0_2_12() {
        int result = 12;
        for (int outer = 0; outer < 2; ++outer) {
            int first_inner;
            for (first_inner = 5; first_inner > outer; first_inner--) {}
            for (int second_inner = 0; second_inner < 10; second_inner++) {
                result += first_inner;
            }
        }
        return result;
    }

    private static int $noinline$nestedLoopCalculation_1_3_12() {
        int result = 12;
        for (int outer = 1; outer < 3; ++outer) {
            int first_inner;
            for (first_inner = 5; first_inner > outer; first_inner--) {}
            for (int second_inner = 0; second_inner < 10; second_inner++) {
                result += first_inner;
            }
        }
        return result;
    }

    private static int $noinline$nestedLoopCalculation_minus2_2_12() {
        int result = 12;
        for (int outer = -2; outer < 2; ++outer) {
            int first_inner;
            for (first_inner = 5; first_inner > outer; first_inner--) {}
            for (int second_inner = 0; second_inner < 10; second_inner++) {
                result += first_inner;
            }
        }
        return result;
    }

    private static int $noinline$nestedLoopCalculation_0_5_12() {
        int result = 12;
        for (int outer = 0; outer < 5; ++outer) {
            int first_inner;
            for (first_inner = 5; first_inner > outer; first_inner--) {}
            for (int second_inner = 0; second_inner < 10; second_inner++) {
                result += first_inner;
            }
        }
        return result;
    }

    public static void main(String[] f) {
        // 12 + 0 + 10 = 22
        assertIntEquals(22, $noinline$nestedLoopCalculation_0_2_12());
        // 12 + 10 + 20 = 42
        assertIntEquals(42, $noinline$nestedLoopCalculation_1_3_12());
        // 12 + (-20) + (-10) + 0 + 10 = -8
        assertIntEquals(-8, $noinline$nestedLoopCalculation_minus2_2_12());
        // 12 + 0 + 10 + 20 + 30 + 40 = 112
        assertIntEquals(112, $noinline$nestedLoopCalculation_0_5_12());
    }

    public static void assertIntEquals(int expected, int result) {
        if (expected != result) {
            throw new Error("Expected: " + expected + ", found: " + result);
        }
    }
}

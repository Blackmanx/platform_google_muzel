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

class Main {
    public static void main(String[] h) {
        assertIntEquals(50, $noinline$SecondInnerLoopReferencesFirst());
    }

    static int $noinline$SecondInnerLoopReferencesFirst() {
        int f = 0;
        for (int outer = 0; outer < 5; outer++) {
            // This will create a Phi[const_3, const_1] that we eliminate in loop optimization
            byte ab = 3;
            // This loop will be eliminated but the next loop will be referencing this one as the
            // `ab` is used in the addition below, which used to lead to a crash. The reason for the
            // crash is that we are not updating the induction variables correctly and they are
            // pointing to a deleted instruction.
            for (int first_inner = 0; first_inner < 5; first_inner++) {
                ab = 1;
            }
            byte i = 0;
            for (int second_inner = 0; second_inner < 10; second_inner++) {
                i += ab;
            }
            f += (int) i;
        }
        return f;
    }

    public static void assertIntEquals(int expected, int result) {
        if (expected != result) {
            throw new Error("Expected: " + expected + ", found: " + result);
        }
    }
}

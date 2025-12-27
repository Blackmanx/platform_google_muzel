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
    static int field;
    static long checksum;

    static void $noinline$testDoNotPeelLoop() {
        int to_add_to_checksum = 112;
        int[] iArr1 = new int[256];
        try {
            for (int i = 10; i < 187; i++) {
                try {
                    int i20 = field % iArr1[i];
                } catch (ArithmeticException ae) {
                }
                // This loop won't be fully unrolled, since `to_add_to_checksum` is part of the
                // outer catch phi.
                // This part of the code can't throw and therefore we eliminate the TryBoundary
                // instructions related to it. However, we still have the value used in the outer
                // catch phi as we don't remove it from there. SuperblockCloner doesn't deal with
                // blocks in a try/catch but this is not considered to be part of a try/catch and
                // therefore it used to try to peel this loop, resulting in errors.
                for (int j = 1; j < 2; j++) {
                    to_add_to_checksum += 209;
                    field -= field;
                }
            }
        } catch (Throwable e) {
        }
        checksum += to_add_to_checksum;
    }

    public static void main(String[] strArr) {
        for (int i = 0; i < 10; ++i) {
            $noinline$testDoNotPeelLoop();
            System.out.println(checksum);
        }
    }
}

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
    public static void main(String[] args) {
        $noinline$testAlwaysThrows();
    }

    // This never visibily throws as all throws are caught within the same method.
    private static void alwaysThrowsInfiniteLoop() {
        while (true) {
            try {
                throw new Error();
            } catch (Error expected) {
            }
        }
    }

    private static void alwaysThrows() throws Exception {
        try {
            throw new Error();
        } catch (Error expected) {
            throw new Exception();
        }
    }

    /// CHECK-START: void Main.$noinline$testAlwaysThrows() inliner (after)
    /// CHECK: InvokeStaticOrDirect method_name:Main.alwaysThrows always_throws:false
    private static void $noinline$testAlwaysThrows() {
        try {
            alwaysThrows();
            System.out.println("alwaysThrows didn't throw");
        } catch (Exception expected) {
        }
    }

    // Don't call this method as it has an infinite loop

    /// CHECK-START: void Main.$noinline$doNotCallInfiniteLoop() inliner (after)
    /// CHECK: InvokeStaticOrDirect method_name:Main.alwaysThrowsInfiniteLoop always_throws:false
    private static void $noinline$doNotCallInfiniteLoop() {
        alwaysThrowsInfiniteLoop();
    }
}

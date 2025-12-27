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

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.lang.invoke.WrongMethodTypeException;
import java.util.Objects;
import java.util.Optional;

public class Multi {
    public static MethodHandle $noinline$getMethodHandle() {
        return OPTIONAL_GET;
    }

    // MethodHandle comes from a different dex file.
    public static void $noinline$testMHFromMain(MethodHandle mh) throws Throwable {
        Optional<Integer> nonEmpty = Optional.<Integer>of(1001);
        Object result = (Object) mh.invokeExact(nonEmpty);
        assertEquals("Expected 1001, but got " + result, 1001, result);

        try {
            mh.invokeExact(nonEmpty);
            fail("mh.type() is (Optional)Object, but callsite is (Optional)V");
        } catch (WrongMethodTypeException expected) {}
    }

    private static void assertEquals(String msg, Object expected, Object actual) {
        if (!Objects.equals(expected, actual)) {
            fail(msg);
        }
    }

    private static void fail(String msg) {
      throw new AssertionError(msg);
    }

    private static final MethodHandle OPTIONAL_GET;

    static {
        try {
            OPTIONAL_GET = MethodHandles.lookup()
                .findVirtual(Optional.class, "get", MethodType.methodType(Object.class));
        } catch (Throwable t) {
            throw new RuntimeException(t);
        }
    }
}

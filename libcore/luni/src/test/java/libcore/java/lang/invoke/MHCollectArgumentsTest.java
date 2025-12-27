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

package libcore.java.lang.invoke;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThrows;

import static java.lang.invoke.MethodType.methodType;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;

@RunWith(JUnit4.class)
@SuppressWarnings("UnusedMethod") // Methods are used by MethodHandles
public class MHCollectArgumentsTest {

    private static int INT_FIELD = 0;

    @Test
    public void noArgs_voidFilter() throws Throwable {
        MethodHandle sum = MethodHandles.lookup()
            .findStatic(
                    MHCollectArgumentsTest.class,
                    "sum",
                    methodType(int.class, int.class, int.class));

        MethodHandle filter = MethodHandles.lookup()
            .findStatic(
                    MHCollectArgumentsTest.class,
                    "sideEffect",
                    methodType(void.class));

        int result = (int) MethodHandles.collectArguments(sum, 0, filter).invokeExact(1, 2);
        assertEquals(3, result);
        assertEquals(42, INT_FIELD);

        result = (int) MethodHandles.collectArguments(sum, 1, filter).invokeExact(1, 2);
        assertEquals(3, result);
        assertEquals(42 * 2, INT_FIELD);

        result = (int) MethodHandles.collectArguments(sum, 2, filter).invokeExact(1, 2);
        assertEquals(3, result);
        assertEquals(42 * 3, INT_FIELD);
    }

    @Test
    public void voidFilter_invalidPos() throws Throwable {
        MethodHandle sum = MethodHandles.lookup()
                .findStatic(
                        MHCollectArgumentsTest.class,
                        "sum",
                        methodType(int.class, int.class, int.class));

        MethodHandle filter = MethodHandles.lookup()
                .findStatic(
                        MHCollectArgumentsTest.class,
                        "sideEffect",
                        methodType(void.class));

        assertThrows(
                IllegalArgumentException.class,
                () -> MethodHandles.collectArguments(sum, -1, filter));

        assertThrows(
                IllegalArgumentException.class,
                () -> MethodHandles.collectArguments(sum, 3, filter));
    }

    private static int sum(int a, int b) {
        return a + b;
    }

    private static void sideEffect() {
        INT_FIELD += 42;
    }

}

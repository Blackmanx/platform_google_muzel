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

import static java.lang.invoke.MethodType.methodType;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.VarHandle;

public class Main {

    public static void main(String[] args) throws Throwable {
        testMethodHandleStaticFieldGet();
        testMethodHandleStaticFieldPut();
        testMethodHandleStaticMethod();
        testVarHandleStaticFieldGet();
        testVarHandleStaticFieldPut();
    }

    private static void testMethodHandleStaticFieldGet() throws Throwable {
        MethodHandle mh = MethodHandles.lookup()
            .findStaticGetter(MHStaticGetter.class, "a", int.class);

        int actual = (int) mh.invokeExact();

        assertEquals(actual, 10);
        assertEquals(MHStaticGetter.b, 12);
    }


    private static void testMethodHandleStaticFieldPut() throws Throwable {
        MethodHandle mh = MethodHandles.lookup()
            .findStaticSetter(MHStaticSetter.class, "a", int.class);

        mh.invokeExact(100);

        assertEquals(MHStaticSetter.a, 100);
        assertEquals(MHStaticGetter.b, 12);
    }

    private static void testMethodHandleStaticMethod() throws Throwable {
        MethodHandle mh = MethodHandles.lookup()
            .findStatic(MHStaticMethod.class, "getB", methodType(int.class));

        int actualB = (int) mh.invokeExact();

        assertEquals(actualB, 12);
        assertEquals(MHStaticMethod.a, 10);
    }

    private static void testVarHandleStaticFieldGet() throws Throwable {
        VarHandle vh = MethodHandles.lookup()
            .findStaticVarHandle(VHStaticGet.class, "a", int.class);

        int actual = (int) vh.get();

        assertEquals(actual, 10);
        assertEquals(VHStaticGet.b, 12);
    }

    private static void testVarHandleStaticFieldPut() throws Throwable {
        VarHandle vh = MethodHandles.lookup()
            .findStaticVarHandle(VHStaticGet.class, "a", int.class);

        vh.set(100);

        assertEquals(VHStaticGet.a, 100);
        assertEquals(VHStaticGet.b, 12);
    }

    private static void assertEquals(int actual, int expected) {
        if (actual != expected) {
            throw new AssertionError("Expected: " + expected + ", but got: " + actual);
        }
    }
}

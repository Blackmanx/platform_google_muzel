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

import static java.lang.invoke.MethodType.methodType;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.lang.reflect.Method;

public class Main {
    public static void main(String[] args) throws Throwable {
        MethodHandle mh = MethodHandles.lookup()
            .findStatic(Main.class, "toString", methodType(Object.class, Object[].class));

        Object[] objects = new Object[2];
        objects[0] = "111";
        objects[1] = 10;

        Class testClass = Class.forName("Test");

        Method m = testClass.getDeclaredMethod("test", MethodHandle.class, Object[].class);
        try {
            Object o = m.invoke(null, mh, objects);
            throw new AssertionError("unreachable");
        } catch (Exception expected) {}
    }

    static Object toString(Object[] objects) {
        return "objects";
    }
}

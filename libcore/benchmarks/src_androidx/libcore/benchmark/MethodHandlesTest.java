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

package libcore.benchmark;

import androidx.benchmark.BenchmarkState;
import androidx.benchmark.junit4.BenchmarkRule;
import androidx.test.runner.AndroidJUnit4;

import dalvik.annotation.optimization.NeverInline;

import org.junit.Rule;
import org.junit.runner.RunWith;
import org.junit.Test;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;

@RunWith(AndroidJUnit4.class)
public class MethodHandlesTest {

    @Rule
    public BenchmarkRule benchmarkRule = new BenchmarkRule();

    private static final MethodHandle MH_1;
    private static final MethodHandle MH_0;
    private static final MethodHandle FIELD_GETTER;
    private static final MethodHandle FIELD_SETTER;
    private static final MethodHandle STATIC_METHOD;

    static {
        try {
            MH_1 = MethodHandles.lookup()
                    .findVirtual(A.class, "identity", MethodType.methodType(int.class, int.class));
            MH_0 = MethodHandles.lookup()
                    .findVirtual(A.class, "constant", MethodType.methodType(int.class));
            FIELD_GETTER = MethodHandles.lookup()
                    .findGetter(A.class, "fField", long.class);
            FIELD_SETTER = MethodHandles.lookup()
                    .findSetter(A.class, "mField", long.class);
            STATIC_METHOD = MethodHandles.lookup()
                    .findStatic(A.class, "staticMethod", MethodType.methodType(int.class));
        } catch (ReflectiveOperationException ex) {
            throw new RuntimeException(ex);
        }
    }

    private final A a = new A();
    private int x1 = 10;

    @Test
    public void virtualCall_noArguments() {
        final BenchmarkState state = benchmarkRule.getState();
        while (state.keepRunning()) {
            a.constant();
        }
    }

    @Test
    public void virtualCall_singleArgument() {
        final BenchmarkState state = benchmarkRule.getState();
        while (state.keepRunning()) {
            a.identity(x1);
        }
    }

    @Test
    public void staticCall() {
        final BenchmarkState state = benchmarkRule.getState();
        while (state.keepRunning()) {
            A.staticMethod();
        }
    }

    @Test
    public void methodHandles_staticMethodCall() throws Throwable {
        final BenchmarkState state = benchmarkRule.getState();
        while (state.keepRunning()) {
            int ignored = (int) STATIC_METHOD.invokeExact();
        }
    }

    @Test
    public void methodHandles_virtualMethod_noArguments() throws Throwable {
        final BenchmarkState state = benchmarkRule.getState();
        while (state.keepRunning()) {
            int ignored = (int) MH_0.invokeExact(a);
        }
    }

    @Test
    public void methodHandles_virtualMethod_singleArgument() throws Throwable {
        final BenchmarkState state = benchmarkRule.getState();
        while (state.keepRunning()) {
            int ignored = (int) MH_1.invokeExact(a, x1);
        }
    }

    @Test
    public void methodHandles_finalFieldGetter() throws Throwable {
        final BenchmarkState state = benchmarkRule.getState();
        while (state.keepRunning()) {
            long ignored = (long) FIELD_GETTER.invokeExact(a);
        }
    }

    @NeverInline
    private long getter() {
        return a.fField;
    }

    @Test
    public void finalFieldGetter() throws Throwable {
        final BenchmarkState state = benchmarkRule.getState();
        while (state.keepRunning()) {
            getter();
        }
    }

    @NeverInline
    private void setterMh() throws Throwable {
        FIELD_SETTER.invokeExact(a, 10L);
    }

    @Test
    public void methodHandles_fieldSetter() throws Throwable {
        final BenchmarkState state = benchmarkRule.getState();
        while (state.keepRunning()) {
            setterMh();
        }
    }

    @NeverInline
    private void setter() {
        a.mField = 10;
    }

    @Test
    public void fieldSetter() throws Throwable {
        final BenchmarkState state = benchmarkRule.getState();
        while (state.keepRunning()) {
            setter();
        }
    }

    @NeverInline
    private void noop() {}

    @Test
    public void testNoop() {
        final BenchmarkState state = benchmarkRule.getState();
        while (state.keepRunning()) {
            noop();
        }
    }

    static class A {

        final long fField = 42;
        long mField = 0;

        @NeverInline
        public int constant() {
            return 42;
        }

        @NeverInline
        public int identity(int a) {
            return a;
        }

        @NeverInline
        public static int staticMethod() {
            return 1001;
        }

    }

}

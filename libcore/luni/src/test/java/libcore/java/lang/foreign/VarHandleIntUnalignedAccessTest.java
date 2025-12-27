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

package libcore.java.lang.foreign;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.invoke.VarHandle;
import java.util.Arrays;
import java.util.Collection;
import java.util.function.Consumer;

import static java.lang.foreign.ValueLayout.JAVA_INT_UNALIGNED;
import static org.junit.Assert.assertThrows;

@RunWith(Parameterized.class)
public class VarHandleIntUnalignedAccessTest {
    private static final long BYTE_SIZE = 100;
    private static final long BYTE_ALIGNMENT = 4;
    private static final Arena GLOBAL_ARENA = Arena.global();
    private static final MemorySegment SEGMENT = GLOBAL_ARENA.allocate(BYTE_SIZE, BYTE_ALIGNMENT);
    private static final long OFFSET = 1;
    private static final int VALUE = 42;
    private static final int EXPECTED_VALUE = 0;
    private static final int NEW_VALUE = 1;

    private final String operationName;
    private final Consumer<VarHandle> operation;

    public VarHandleIntUnalignedAccessTest(String operationName, Consumer<VarHandle> operation) {
        this.operationName = operationName;
        this.operation = operation;
    }

    @Parameters(name = "{0}")
    public static Collection<Object[]> unalignedOperations() {
        return Arrays.asList(new Object[][]{
                // Opaque and Volatile Access
                {"getVolatile", (Consumer<VarHandle>)
                        vh -> vh.getVolatile(SEGMENT, OFFSET)},
                {"setVolatile", (Consumer<VarHandle>)
                        vh -> vh.setVolatile(SEGMENT, OFFSET, VALUE)},
                {"getAcquire", (Consumer<VarHandle>)
                        vh -> vh.getAcquire(SEGMENT, OFFSET)},
                {"setRelease", (Consumer<VarHandle>)
                        vh -> vh.setRelease(SEGMENT, OFFSET, VALUE)},
                {"getOpaque", (Consumer<VarHandle>)
                        vh -> vh.getOpaque(SEGMENT, OFFSET)},
                {"setOpaque", (Consumer<VarHandle>)
                        vh -> vh.setOpaque(SEGMENT, OFFSET, VALUE)},

                // Atomic Compare-and-Set Operations
                {"compareAndSet", (Consumer<VarHandle>)
                        vh -> vh.compareAndSet(SEGMENT, OFFSET, EXPECTED_VALUE, NEW_VALUE)},
                {"compareAndExchange", (Consumer<VarHandle>)
                        vh -> vh.compareAndExchange(SEGMENT, OFFSET, EXPECTED_VALUE, NEW_VALUE)},
                {"compareAndExchangeAcquire", (Consumer<VarHandle>)
                        vh -> vh.compareAndExchangeAcquire(SEGMENT, OFFSET, EXPECTED_VALUE, NEW_VALUE)},
                {"compareAndExchangeRelease", (Consumer<VarHandle>)
                        vh -> vh.compareAndExchangeRelease(SEGMENT, OFFSET, EXPECTED_VALUE, NEW_VALUE)},
                {"weakCompareAndSetPlain", (Consumer<VarHandle>)
                        vh -> vh.weakCompareAndSetPlain(SEGMENT, OFFSET, EXPECTED_VALUE, NEW_VALUE)},
                {"weakCompareAndSet", (Consumer<VarHandle>)
                        vh -> vh.weakCompareAndSet(SEGMENT, OFFSET, EXPECTED_VALUE, NEW_VALUE)},
                {"weakCompareAndSetAcquire", (Consumer<VarHandle>)
                        vh -> vh.weakCompareAndSetAcquire(SEGMENT, OFFSET, EXPECTED_VALUE, NEW_VALUE)},
                {"weakCompareAndSetRelease", (Consumer<VarHandle>)
                        vh -> vh.weakCompareAndSetRelease(SEGMENT, OFFSET, EXPECTED_VALUE, NEW_VALUE)},

                // Atomic Read-Modify-Write Operations
                {"getAndSet", (Consumer<VarHandle>)
                        vh -> vh.getAndSet(SEGMENT, OFFSET, VALUE)},
                {"getAndSetAcquire", (Consumer<VarHandle>)
                        vh -> vh.getAndSetAcquire(SEGMENT, OFFSET, VALUE)},
                {"getAndSetRelease", (Consumer<VarHandle>)
                        vh -> vh.getAndSetRelease(SEGMENT, OFFSET, VALUE)},
                {"getAndAdd", (Consumer<VarHandle>)
                        vh -> vh.getAndAdd(SEGMENT, OFFSET, VALUE)},
                {"getAndAddAcquire", (Consumer<VarHandle>)
                        vh -> vh.getAndAddAcquire(SEGMENT, OFFSET, VALUE)},
                {"getAndAddRelease", (Consumer<VarHandle>)
                        vh -> vh.getAndAddRelease(SEGMENT, OFFSET, VALUE)},
                {"getAndBitwiseOr", (Consumer<VarHandle>)
                        vh -> vh.getAndBitwiseOr(SEGMENT, OFFSET, VALUE)},
                {"getAndBitwiseOrRelease", (Consumer<VarHandle>)
                        vh -> vh.getAndBitwiseOrRelease(SEGMENT, OFFSET, VALUE)},
                {"getAndBitwiseOrAcquire", (Consumer<VarHandle>)
                        vh -> vh.getAndBitwiseOrAcquire(SEGMENT, OFFSET, VALUE)},
                {"getAndBitwiseAnd", (Consumer<VarHandle>)
                        vh -> vh.getAndBitwiseAnd(SEGMENT, OFFSET, VALUE)},
                {"getAndBitwiseAndRelease", (Consumer<VarHandle>)
                        vh -> vh.getAndBitwiseAndRelease(SEGMENT, OFFSET, VALUE)},
                {"getAndBitwiseAndAcquire", (Consumer<VarHandle>)
                        vh -> vh.getAndBitwiseAndAcquire(SEGMENT, OFFSET, VALUE)},
                {"getAndBitwiseXor", (Consumer<VarHandle>)
                        vh -> vh.getAndBitwiseXor(SEGMENT, OFFSET, VALUE)},
                {"getAndBitwiseXorRelease", (Consumer<VarHandle>)
                        vh -> vh.getAndBitwiseXorRelease(SEGMENT, OFFSET, VALUE)},
                {"getAndBitwiseXorAcquire", (Consumer<VarHandle>)
                        vh -> vh.getAndBitwiseXorAcquire(SEGMENT, OFFSET, VALUE)},
        });
    }

    @Test
    public void atomicOperation_throwsUnsupportedOperation_forUnalignedAccess() {
        VarHandle unalignedVarHandle = JAVA_INT_UNALIGNED.varHandle();
        assertThrows(UnsupportedOperationException.class,
                () -> operation.accept(unalignedVarHandle));
    }
}




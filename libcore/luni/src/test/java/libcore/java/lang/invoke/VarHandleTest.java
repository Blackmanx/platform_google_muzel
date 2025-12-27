/*
 * Copyright (C) 2021 The Android Open Source Project
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

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertEquals;

import static java.util.Objects.requireNonNull;
import static java.util.stream.Collectors.toMap;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.lang.invoke.MethodHandles;
import java.lang.invoke.VarHandle;
import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.Set;
import java.util.function.Function;

@RunWith(JUnit4.class)
public class VarHandleTest {

    private int field = 0;

    private static int getField(String name) {
        try {
            Field f = VarHandle.class.getDeclaredField(name);
            if (f.getType() != int.class) {
                throw new AssertionError(name +
                    " expected to be int, but was: " + f.getType());
            }
            f.setAccessible(true);
            return (int) f.get(null);
        } catch (NoSuchFieldException | IllegalAccessException e) {
            throw new RuntimeException(e);
        }
    }

    // Reflectively fetched values of otherwise inaccessible VarHandle.AccessType enum.
    static class AccessType {
        static final Object GET;
        static final Object SET;
        static final Object COMPARE_AND_SET;
        static final Object COMPARE_AND_EXCHANGE;
        static final Object GET_AND_UPDATE;
        static final Object GET_AND_UPDATE_BITWISE;
        static final Object GET_AND_UPDATE_NUMERIC;

        static {
            try {
                Class accessTypeClass = Class.forName("java.lang.invoke.VarHandle$AccessType");
                var accessTypes = Arrays.stream(accessTypeClass.getEnumConstants())
                        .collect(toMap(Object::toString, Function.identity()));
                GET = requireNonNull(accessTypes.get("GET"));
                SET = requireNonNull(accessTypes.get("SET"));
                COMPARE_AND_SET = requireNonNull(accessTypes.get("COMPARE_AND_SET"));
                COMPARE_AND_EXCHANGE = requireNonNull(accessTypes.get("COMPARE_AND_EXCHANGE"));
                GET_AND_UPDATE = requireNonNull(accessTypes.get("GET_AND_UPDATE"));
                GET_AND_UPDATE_BITWISE = requireNonNull(accessTypes.get("GET_AND_UPDATE_BITWISE"));
                GET_AND_UPDATE_NUMERIC = requireNonNull(accessTypes.get("GET_AND_UPDATE_NUMERIC"));
            } catch (ClassNotFoundException e) {
                throw new AssertionError(e);
            }
        }
    }

    @Test
    public void constantsAreConsistent() {
        {
            int READ_ACCESS_MODES_BIT_MASK = getField("READ_ACCESS_MODES_BIT_MASK");
            int expected = accessTypesToBitMask(Set.of(AccessType.GET));
            String msg = "READ_ACCESS_MODES_BIT_MASK";
            assertEquals(msg, expected, READ_ACCESS_MODES_BIT_MASK);
        }
        {
            int WRITE_ACCESS_MODES_BIT_MASK = getField("WRITE_ACCESS_MODES_BIT_MASK");
            int expected = accessTypesToBitMask(Set.of(AccessType.SET));
            String msg = "WRITE_ACCESS_MODES_BIT_MASK";
            assertEquals(msg, expected, WRITE_ACCESS_MODES_BIT_MASK);
        }
        {
            int ATOMIC_UPDATE_ACCESS_MODES_BIT_MASK =
                getField("ATOMIC_UPDATE_ACCESS_MODES_BIT_MASK");
            int expected = accessTypesToBitMask(
                    Set.of(
                            AccessType.COMPARE_AND_EXCHANGE,
                            AccessType.COMPARE_AND_SET,
                            AccessType.GET_AND_UPDATE));
            String msg = "ATOMIC_UPDATE_ACCESS_MODES_BIT_MASK";
            assertEquals(msg, expected, ATOMIC_UPDATE_ACCESS_MODES_BIT_MASK);
        }
        {
            int NUMERIC_ATOMIC_UPDATE_ACCESS_MODES_BIT_MASK =
                getField("NUMERIC_ATOMIC_UPDATE_ACCESS_MODES_BIT_MASK");
            int expected = accessTypesToBitMask(Set.of(AccessType.GET_AND_UPDATE_NUMERIC));
            String msg = "NUMERIC_ATOMIC_UPDATE_ACCESS_MODES_BIT_MASK";
            assertEquals(msg, expected, NUMERIC_ATOMIC_UPDATE_ACCESS_MODES_BIT_MASK);
        }
        {
            int BITWISE_ATOMIC_UPDATE_ACCESS_MODES_BIT_MASK =
                getField("BITWISE_ATOMIC_UPDATE_ACCESS_MODES_BIT_MASK");
            int expected = accessTypesToBitMask(
                    Set.of(AccessType.GET_AND_UPDATE_BITWISE));
            String msg = "BITWISE_ATOMIC_UPDATE_ACCESS_MODES_BIT_MASK";
            assertEquals(msg, expected, BITWISE_ATOMIC_UPDATE_ACCESS_MODES_BIT_MASK);
        }
    }

    private static Object getAtField(VarHandle.AccessMode accessMode) {
        try {
            Field atField = VarHandle.AccessMode.class.getDeclaredField("at");
            atField.setAccessible(true);
            return atField.get(accessMode);
        } catch (Throwable t) {
            throw new RuntimeException(t);
        }
    }

    static int accessTypesToBitMask(final Set<?> accessTypes) {
        int m = 0;
        for (VarHandle.AccessMode accessMode : VarHandle.AccessMode.values()) {
            if (accessTypes.contains(getAtField(accessMode))) {
                m |= 1 << accessMode.ordinal();
            }
        }
        return m;
    }

    @Test
    public void accessMode_shouldNotOverflow() {
        // Check we're not about to overflow the storage of the
        // bitmasks here and in the accessModesBitMask field.
        assertTrue(VarHandle.AccessMode.values().length <= Integer.SIZE);
    }

    @Test
    public void fences() {
        // In theory, these should log coverage for these fences, but they are implemented
        // as intrinsics in the runtime and the compiler.
        VarHandle.acquireFence();
        VarHandle.releaseFence();
        VarHandle.fullFence();
        VarHandle.loadLoadFence();
        VarHandle.storeStoreFence();
    }

    @Test
    public void toString_describes_variable_and_its_coordinates_plain_field() throws Throwable {
        VarHandle vh = MethodHandles.lookup().findVarHandle(
                VarHandleTest.class, "field", int.class);

        String str = vh.toString();

        // Type of field is int.
        assertTrue(str + " does not mention int", str.contains("int"));
        assertTrue(str + " does not mention VarHandleTest", str.contains("VarHandleTest"));
        // Just to make errorprone happy.
        assertEquals(0, field);
    }
}

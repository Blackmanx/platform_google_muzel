/*
 * Copyright (C) 2014 The Android Open Source Project
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

package libcore.dalvik.system;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import android.platform.test.annotations.RequiresFlagsEnabled;

import java.lang.reflect.Array;
import java.util.concurrent.atomic.AtomicInteger;

import dalvik.system.VMRuntime;

import org.junit.Assume;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/**
 * Test VMRuntime behavior.
 */
@RunWith(JUnit4.class)
public final class VMRuntimeTest {

    private void doTestNewNonMovableArray(Class<?> componentType, int step, int maxLength) {
        // Can't create negative sized arrays.
        try {
            Object array = VMRuntime.getRuntime().newNonMovableArray(componentType, -1);
            assertTrue(false);
        } catch (NegativeArraySizeException expected) {
        }

        try {
            Object array = VMRuntime.getRuntime().newNonMovableArray(componentType, Integer.MIN_VALUE);
            assertTrue(false);
        } catch (NegativeArraySizeException expected) {
        }

        // Allocate arrays in a loop and check their properties.
        for (int i = 0; i <= maxLength; i += step) {
            Object array = VMRuntime.getRuntime().newNonMovableArray(componentType, i);
            assertTrue(array.getClass().isArray());
            assertEquals(array.getClass().getComponentType(), componentType);
            assertEquals(Array.getLength(array), i);
        }
    }

    @Test
    public void testNewNonMovableArray() {
        // Can't create arrays with no component type.
        try {
            Object array = VMRuntime.getRuntime().newNonMovableArray(null, 0);
            assertTrue(false);
        } catch (NullPointerException expected) {
        }

        // Can't create arrays of void.
        try {
            Object array = VMRuntime.getRuntime().newNonMovableArray(void.class, 0);
            assertTrue(false);
        } catch (NoClassDefFoundError expected) {
        }

        int maxLengthForLoop = 16 * 1024;
        int step = 67;
        doTestNewNonMovableArray(boolean.class, step, maxLengthForLoop);
        doTestNewNonMovableArray(byte.class, step, maxLengthForLoop);
        doTestNewNonMovableArray(char.class, step, maxLengthForLoop);
        doTestNewNonMovableArray(short.class, step, maxLengthForLoop);
        doTestNewNonMovableArray(int.class, step, maxLengthForLoop);
        doTestNewNonMovableArray(long.class, step, maxLengthForLoop);
        doTestNewNonMovableArray(float.class, step, maxLengthForLoop);
        doTestNewNonMovableArray(double.class, step, maxLengthForLoop);
        doTestNewNonMovableArray(Object.class, step, maxLengthForLoop);
        doTestNewNonMovableArray(Number.class, step, maxLengthForLoop);
        doTestNewNonMovableArray(String.class, step, maxLengthForLoop);
        doTestNewNonMovableArray(Runnable.class, step, maxLengthForLoop);
    }

    private void doTestNewUnpaddedArray(Class<?> componentType, int step, int maxLength) {
         // Can't create negative sized arrays.
        try {
            Object array = VMRuntime.getRuntime().newUnpaddedArray(componentType, -1);
            assertTrue(false);
        } catch (NegativeArraySizeException expected) {
        }

        try {
            Object array = VMRuntime.getRuntime().newUnpaddedArray(componentType, Integer.MIN_VALUE);
            assertTrue(false);
        } catch (NegativeArraySizeException expected) {
        }

        // Allocate arrays in a loop and check their properties.
        for (int i = 0; i <= maxLength; i += step) {
            Object array = VMRuntime.getRuntime().newUnpaddedArray(componentType, i);
            assertTrue(array.getClass().isArray());
            assertEquals(array.getClass().getComponentType(), componentType);
            assertTrue(Array.getLength(array) >= i);
        }
    }

    @Test
    public void testNewUnpaddedArray() {
        // Can't create arrays with no component type.
        try {
            Object array = VMRuntime.getRuntime().newUnpaddedArray(null, 0);
            assertTrue(false);
        } catch (NullPointerException expected) {
        }

        // Can't create arrays of void.
        try {
            Object array = VMRuntime.getRuntime().newUnpaddedArray(void.class, 0);
            assertTrue(false);
        } catch (NoClassDefFoundError expected) {
        }

        int maxLengthForLoop = 16 * 1024;
        int step = 67;
        doTestNewUnpaddedArray(boolean.class, step, maxLengthForLoop);
        doTestNewUnpaddedArray(byte.class, step, maxLengthForLoop);
        doTestNewUnpaddedArray(char.class, step, maxLengthForLoop);
        doTestNewUnpaddedArray(short.class, step, maxLengthForLoop);
        doTestNewUnpaddedArray(int.class, step, maxLengthForLoop);
        doTestNewUnpaddedArray(long.class, step, maxLengthForLoop);
        doTestNewUnpaddedArray(float.class, step, maxLengthForLoop);
        doTestNewUnpaddedArray(double.class, step, maxLengthForLoop);
        doTestNewUnpaddedArray(Object.class, step, maxLengthForLoop);
        doTestNewUnpaddedArray(Number.class, step, maxLengthForLoop);
        doTestNewUnpaddedArray(String.class, step, maxLengthForLoop);
        doTestNewUnpaddedArray(Runnable.class, step, maxLengthForLoop);
    }

    @Test
    public void testIsVTrunkStableFlagEnabled() {
        // The flag should be on all stages, including trunk_staging and next.
        assertTrue(VMRuntime.isVTrunkStableFlagEnabled());
    }

    @Test
    public void testIsArtTestFlagEnabled() {
        assertTrue(VMRuntime.isArtTestFlagEnabled());
    }

    @Test
    public void testIsArtTestRwFlagEnabled() {
        assertTrue(VMRuntime.isArtTestRwFlagEnabled());
    }


    @Test
    public void testGetFullGcCount() {
        long gcCount = VMRuntime.getFullGcCount();
        // full GC count needs to be larger or equal to 0
        assertTrue("Full GC count needs to be non-negative", gcCount >= 0);
    }

    @Test
    @RequiresFlagsEnabled(com.android.libcore.Flags.FLAG_POST_CLEANUP_APIS)
    public void testPostCleanup() {
        AtomicInteger cleanupCounter = new AtomicInteger(0);

        Runnable r1 = new Runnable() {
            @Override
            public void run() {
                cleanupCounter.addAndGet(1);
            }
        };
        Runnable r2 = new Runnable() {
            @Override
            public void run() {
                cleanupCounter.addAndGet(2);
            }
        };

        // test callbacks are called when explicitly trigger onPostCleanup()
        VMRuntime.addPostCleanupCallback(r1);
        VMRuntime.addPostCleanupCallback(r2);
        VMRuntime.onPostCleanup();
        assertTrue(cleanupCounter.get() == 3);

        // test callbacks are called when GC is triggered and finalization is done
        System.gc();
        System.runFinalization();
        try {
            int nsleep = 5;
            while (cleanupCounter.get() < 6 && (nsleep-- > 0)) {
                Thread.sleep(1000);
            }
        } catch (InterruptedException e) {
            assertTrue("Sleep in test interrupted", false);
        }

        // NOTE: post cleanup could happen more than 1 time, hence the counter could be >= 6
        int counter = cleanupCounter.get();
        assertTrue("cleanupCounter should be >=6, got " + counter + " instead", counter >= 6);

        VMRuntime.removePostCleanupCallback(r1);
        VMRuntime.removePostCleanupCallback(r2);
    }
}


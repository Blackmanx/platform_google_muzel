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
package libcore.jdk.internal.misc;

import static org.junit.Assert.assertEquals;

import java.lang.reflect.Field;

import jdk.internal.misc.Unsafe;

import libcore.test.annotation.NonCts;
import libcore.test.reasons.NonCtsReasons;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@NonCts(bug = 287231726, reason = NonCtsReasons.INTERNAL_APIS)
@RunWith(JUnit4.class)
public class UnsafeTest {

    @SuppressWarnings("unused")
    private static class TestFixture {
        public static boolean staticBooleanVar = false;
        public static byte staticByteVar = 40;
        public static int staticIntVar = 2056;
        public static long staticLongVar = 1234567890;
        public static float staticFloatVar = 2.618f;
        public static double staticDoubleVar = 3.1415;

        public boolean booleanVar = true;
        public byte byteVar = 42;
        public int intVar = 2046;
        public long longVar = 123456789;
        public float floatVar = 1.618f;
        public double doubleVar = 3.141;
        public Object objectVar = new Object();
    }


    @Test
    public void testCompareAndExchangeLong() throws Exception {
        Unsafe unsafe = getUnsafe();
        TestFixture tf = new TestFixture();
        long offset = unsafe.objectFieldOffset(TestFixture.class, "longVar");
        assertEquals(123456789, unsafe.compareAndExchangeLong(tf, offset, 0, -1));
        assertEquals(123456789, tf.longVar);
        assertEquals(123456789, unsafe.compareAndExchangeLong(tf, offset, 123456789, -1));
        assertEquals(-1, tf.longVar);
        assertEquals(-1, unsafe.compareAndExchangeLong(tf, offset, 0, 1));
        assertEquals(-1, tf.longVar);
        assertEquals(-1, unsafe.compareAndExchangeLong(tf, offset, -1, 1));
        assertEquals(1, tf.longVar);
    }

    @Test
    public void testStaticOffset() throws Exception {
        Unsafe unsafe = getUnsafe();
        Class c = Class.forName("libcore.jdk.internal.misc.UnsafeTest$TestFixture");
        Field f = c.getDeclaredField("staticIntVar");
        Object obj = unsafe.staticFieldBase(f);
        long offset = unsafe.staticFieldOffset(f);

        assertEquals(2056, unsafe.getInt(obj, offset));
        assertEquals(2056, unsafe.getAndSetInt(obj, offset, 0));
        assertEquals(0, TestFixture.staticIntVar);

        assertEquals(0, unsafe.getAndSetInt(obj, offset, 1));
        assertEquals(1, TestFixture.staticIntVar);

        assertEquals(1, unsafe.getAndSetInt(obj, offset, 2056));
        assertEquals(2056, TestFixture.staticIntVar);
    }

    private static Unsafe getUnsafe() throws Exception {
        Class<?> unsafeClass = Class.forName("jdk.internal.misc.Unsafe");
        Field f = unsafeClass.getDeclaredField("theUnsafe");
        f.setAccessible(true);
        return (Unsafe) f.get(null);
    }

}

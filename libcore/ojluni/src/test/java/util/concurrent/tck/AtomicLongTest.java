/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

/*
 * This file is available under and governed by the GNU General Public
 * License version 2 only, as published by the Free Software Foundation.
 * However, the following notice accompanied the original version of this
 * file:
 *
 * Written by Doug Lea with assistance from members of JCP JSR-166
 * Expert Group and released to the public domain, as explained at
 * http://creativecommons.org/publicdomain/zero/1.0/
 * Other contributors include Andrew Wright, Jeffrey Hayes,
 * Pat Fisher, Mike Judd.
 */

package test.java.util.concurrent.tck;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertNotSame;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.concurrent.atomic.AtomicLong;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

// Android-changed: Use JUnit4.
@RunWith(JUnit4.class)
public class AtomicLongTest extends JSR166TestCase {
    // Android-changed: Use JUnitCore.main.
    public static void main(String[] args) {
        // main(suite(), args);
        org.junit.runner.JUnitCore.main("test.java.util.concurrent.tck.AtomicLongTest");
    }
    // public static Test suite() {
    //     return new TestSuite(AtomicLongTest.class);
    // }

    final long[] VALUES = {
        Long.MIN_VALUE,
        Integer.MIN_VALUE, -1, 0, 1, 42, Integer.MAX_VALUE,
        Long.MAX_VALUE,
    };

    /**
     * constructor initializes to given value
     */
    @Test
    public void testConstructor() {
        AtomicLong ai = new AtomicLong(1);
        assertEquals(1, ai.get());
    }

    /**
     * default constructed initializes to zero
     */
    @Test
    public void testConstructor2() {
        AtomicLong ai = new AtomicLong();
        assertEquals(0, ai.get());
    }

    /**
     * get returns the last value set
     */
    @Test
    public void testGetSet() {
        AtomicLong ai = new AtomicLong(1);
        assertEquals(1, ai.get());
        ai.set(2);
        assertEquals(2, ai.get());
        ai.set(-3);
        assertEquals(-3, ai.get());
    }

    /**
     * get returns the last value lazySet in same thread
     */
    @Test
    public void testGetLazySet() {
        AtomicLong ai = new AtomicLong(1);
        assertEquals(1, ai.get());
        ai.lazySet(2);
        assertEquals(2, ai.get());
        ai.lazySet(-3);
        assertEquals(-3, ai.get());
    }

    /**
     * compareAndSet succeeds in changing value if equal to expected else fails
     */
    @Test
    public void testCompareAndSet() {
        AtomicLong ai = new AtomicLong(1);
        assertTrue(ai.compareAndSet(1, 2));
        assertTrue(ai.compareAndSet(2, -4));
        assertEquals(-4, ai.get());
        assertFalse(ai.compareAndSet(-5, 7));
        assertEquals(-4, ai.get());
        assertTrue(ai.compareAndSet(-4, 7));
        assertEquals(7, ai.get());
    }

    /**
     * compareAndSet in one thread enables another waiting for value
     * to succeed
     */
    @Test
    public void testCompareAndSetInMultipleThreads() throws Exception {
        final AtomicLong ai = new AtomicLong(1);
        Thread t = new Thread(new CheckedRunnable() {
            public void realRun() {
                while (!ai.compareAndSet(2, 3))
                    Thread.yield();
            }});

        t.start();
        assertTrue(ai.compareAndSet(1, 2));
        t.join(LONG_DELAY_MS);
        assertFalse(t.isAlive());
        assertEquals(3, ai.get());
    }

    /**
     * repeated weakCompareAndSet succeeds in changing value when equal
     * to expected
     */
    @Test
    public void testWeakCompareAndSet() {
        AtomicLong ai = new AtomicLong(1);
        do {} while (!ai.weakCompareAndSet(1, 2));
        do {} while (!ai.weakCompareAndSet(2, -4));
        assertEquals(-4, ai.get());
        do {} while (!ai.weakCompareAndSet(-4, 7));
        assertEquals(7, ai.get());
    }

    /**
     * getAndSet returns previous value and sets to given value
     */
    @Test
    public void testGetAndSet() {
        AtomicLong ai = new AtomicLong(1);
        assertEquals(1, ai.getAndSet(0));
        assertEquals(0, ai.getAndSet(-10));
        assertEquals(-10, ai.getAndSet(1));
    }

    /**
     * getAndAdd returns previous value and adds given value
     */
    @Test
    public void testGetAndAdd() {
        AtomicLong ai = new AtomicLong(1);
        assertEquals(1, ai.getAndAdd(2));
        assertEquals(3, ai.get());
        assertEquals(3, ai.getAndAdd(-4));
        assertEquals(-1, ai.get());
    }

    /**
     * getAndDecrement returns previous value and decrements
     */
    @Test
    public void testGetAndDecrement() {
        AtomicLong ai = new AtomicLong(1);
        assertEquals(1, ai.getAndDecrement());
        assertEquals(0, ai.getAndDecrement());
        assertEquals(-1, ai.getAndDecrement());
    }

    /**
     * getAndIncrement returns previous value and increments
     */
    @Test
    public void testGetAndIncrement() {
        AtomicLong ai = new AtomicLong(1);
        assertEquals(1, ai.getAndIncrement());
        assertEquals(2, ai.get());
        ai.set(-2);
        assertEquals(-2, ai.getAndIncrement());
        assertEquals(-1, ai.getAndIncrement());
        assertEquals(0, ai.getAndIncrement());
        assertEquals(1, ai.get());
    }

    /**
     * addAndGet adds given value to current, and returns current value
     */
    @Test
    public void testAddAndGet() {
        AtomicLong ai = new AtomicLong(1);
        assertEquals(3, ai.addAndGet(2));
        assertEquals(3, ai.get());
        assertEquals(-1, ai.addAndGet(-4));
        assertEquals(-1, ai.get());
    }

    /**
     * decrementAndGet decrements and returns current value
     */
    @Test
    public void testDecrementAndGet() {
        AtomicLong ai = new AtomicLong(1);
        assertEquals(0, ai.decrementAndGet());
        assertEquals(-1, ai.decrementAndGet());
        assertEquals(-2, ai.decrementAndGet());
        assertEquals(-2, ai.get());
    }

    /**
     * incrementAndGet increments and returns current value
     */
    @Test
    public void testIncrementAndGet() {
        AtomicLong ai = new AtomicLong(1);
        assertEquals(2, ai.incrementAndGet());
        assertEquals(2, ai.get());
        ai.set(-2);
        assertEquals(-1, ai.incrementAndGet());
        assertEquals(0, ai.incrementAndGet());
        assertEquals(1, ai.incrementAndGet());
        assertEquals(1, ai.get());
    }

    /**
     * a deserialized serialized atomic holds same value
     */
    @Test
    public void testSerialization() throws Exception {
        AtomicLong x = new AtomicLong();
        AtomicLong y = serialClone(x);
        assertNotSame(x, y);
        x.set(-22);
        AtomicLong z = serialClone(x);
        assertNotSame(y, z);
        assertEquals(-22, x.get());
        assertEquals(0, y.get());
        assertEquals(-22, z.get());
    }

    /**
     * toString returns current value.
     */
    @Test
    public void testToString() {
        AtomicLong ai = new AtomicLong();
        assertEquals("0", ai.toString());
        for (long x : VALUES) {
            ai.set(x);
            assertEquals(Long.toString(x), ai.toString());
        }
    }

    /**
     * intValue returns current value.
     */
    @Test
    public void testIntValue() {
        AtomicLong ai = new AtomicLong();
        assertEquals(0, ai.intValue());
        for (long x : VALUES) {
            ai.set(x);
            assertEquals((int)x, ai.intValue());
        }
    }

    /**
     * longValue returns current value.
     */
    @Test
    public void testLongValue() {
        AtomicLong ai = new AtomicLong();
        assertEquals(0L, ai.longValue());
        for (long x : VALUES) {
            ai.set(x);
            assertEquals(x, ai.longValue());
        }
    }

    /**
     * floatValue returns current value.
     */
    @Test
    public void testFloatValue() {
        AtomicLong ai = new AtomicLong();
        assertEquals(0.0f, ai.floatValue(), 0.0f);
        for (long x : VALUES) {
            ai.set(x);
            assertEquals((float)x, ai.floatValue(), 0.0f);
        }
    }

    /**
     * doubleValue returns current value.
     */
    @Test
    public void testDoubleValue() {
        AtomicLong ai = new AtomicLong();
        assertEquals(0.0d, ai.doubleValue(), 0.0d);
        for (long x : VALUES) {
            ai.set(x);
            assertEquals((double)x, ai.doubleValue(), 0.0d);
        }
    }

}

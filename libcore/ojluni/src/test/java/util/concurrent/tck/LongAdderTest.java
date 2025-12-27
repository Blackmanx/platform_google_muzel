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

import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.atomic.LongAdder;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

// Android-changed: Use JUnit4.
@RunWith(JUnit4.class)
public class LongAdderTest extends JSR166TestCase {
    // Android-changed: Use JUnitCore.main.
    public static void main(String[] args) {
        // main(suite(), args);
        org.junit.runner.JUnitCore.main("test.java.util.concurrent.tck.LongAdderTest");
    }
    // public static Test suite() {
    //     return new TestSuite(LongAdderTest.class);
    // }

    /**
     * default constructed initializes to zero
     */
    @Test
    public void testConstructor() {
        LongAdder ai = new LongAdder();
        assertEquals(0, ai.sum());
    }

    /**
     * add adds given value to current, and sum returns current value
     */
    @Test
    public void testAddAndSum() {
        LongAdder ai = new LongAdder();
        ai.add(2);
        assertEquals(2, ai.sum());
        ai.add(-4);
        assertEquals(-2, ai.sum());
    }

    /**
     * decrement decrements and sum returns current value
     */
    @Test
    public void testDecrementAndsum() {
        LongAdder ai = new LongAdder();
        ai.decrement();
        assertEquals(-1, ai.sum());
        ai.decrement();
        assertEquals(-2, ai.sum());
    }

    /**
     * incrementAndGet increments and returns current value
     */
    @Test
    public void testIncrementAndsum() {
        LongAdder ai = new LongAdder();
        ai.increment();
        assertEquals(1, ai.sum());
        ai.increment();
        assertEquals(2, ai.sum());
    }

    /**
     * reset() causes subsequent sum() to return zero
     */
    @Test
    public void testReset() {
        LongAdder ai = new LongAdder();
        ai.add(2);
        assertEquals(2, ai.sum());
        ai.reset();
        assertEquals(0, ai.sum());
    }

    /**
     * sumThenReset() returns sum; subsequent sum() returns zero
     */
    @Test
    public void testSumThenReset() {
        LongAdder ai = new LongAdder();
        ai.add(2);
        assertEquals(2, ai.sum());
        assertEquals(2, ai.sumThenReset());
        assertEquals(0, ai.sum());
    }

    /**
     * a deserialized serialized adder holds same value
     */
    @Test
    public void testSerialization() throws Exception {
        LongAdder x = new LongAdder();
        LongAdder y = serialClone(x);
        assertNotSame(x, y);
        x.add(-22);
        LongAdder z = serialClone(x);
        assertNotSame(y, z);
        assertEquals(-22, x.sum());
        assertEquals(0, y.sum());
        assertEquals(-22, z.sum());
    }

    /**
     * toString returns current value.
     */
    @Test
    public void testToString() {
        LongAdder ai = new LongAdder();
        assertEquals("0", ai.toString());
        ai.increment();
        assertEquals(Long.toString(1), ai.toString());
    }

    /**
     * intValue returns current value.
     */
    @Test
    public void testIntValue() {
        LongAdder ai = new LongAdder();
        assertEquals(0, ai.intValue());
        ai.increment();
        assertEquals(1, ai.intValue());
    }

    /**
     * longValue returns current value.
     */
    @Test
    public void testLongValue() {
        LongAdder ai = new LongAdder();
        assertEquals(0, ai.longValue());
        ai.increment();
        assertEquals(1, ai.longValue());
    }

    /**
     * floatValue returns current value.
     */
    @Test
    public void testFloatValue() {
        LongAdder ai = new LongAdder();
        assertEquals(0.0f, ai.floatValue(), 0.0f);
        ai.increment();
        assertEquals(1.0f, ai.floatValue(), 0.0f);
    }

    /**
     * doubleValue returns current value.
     */
    @Test
    public void testDoubleValue() {
        LongAdder ai = new LongAdder();
        assertEquals(0.0, ai.doubleValue(), 0.0d);
        ai.increment();
        assertEquals(1.0, ai.doubleValue(), 0.0d);
    }

    /**
     * adds by multiple threads produce correct sum
     */
    @Test
    public void testAddAndSumMT() throws Throwable {
        final int incs = 1000000;
        final int nthreads = 4;
        final ExecutorService pool = Executors.newCachedThreadPool();
        LongAdder a = new LongAdder();
        CyclicBarrier barrier = new CyclicBarrier(nthreads + 1);
        for (int i = 0; i < nthreads; ++i)
            pool.execute(new AdderTask(a, barrier, incs));
        barrier.await();
        barrier.await();
        long total = (long)nthreads * incs;
        long sum = a.sum();
        assertEquals(sum, total);
        pool.shutdown();
    }

    static final class AdderTask implements Runnable {
        final LongAdder adder;
        final CyclicBarrier barrier;
        final int incs;
        volatile long result;
        AdderTask(LongAdder adder, CyclicBarrier barrier, int incs) {
            this.adder = adder;
            this.barrier = barrier;
            this.incs = incs;
        }

        public void run() {
            try {
                barrier.await();
                LongAdder a = adder;
                for (int i = 0; i < incs; ++i)
                    a.add(1L);
                result = a.sum();
                barrier.await();
            } catch (Throwable t) { throw new Error(t); }
        }
    }

}

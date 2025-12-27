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
import java.util.concurrent.atomic.DoubleAdder;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

// Android-changed: Use JUnit4.
@RunWith(JUnit4.class)
public class DoubleAdderTest extends JSR166TestCase {
    // Android-changed: Use JUnitCore.main.
    public static void main(String[] args) {
        // main(suite(), args);
        org.junit.runner.JUnitCore.main("test.java.util.concurrent.tck.DoubleAdderTest");
    }
    // public static Test suite() {
    //     return new TestSuite(DoubleAdderTest.class);
    // }

    /**
     * default constructed initializes to zero
     */
    @Test
    public void testConstructor() {
        DoubleAdder ai = new DoubleAdder();
        assertEquals(0.0, ai.sum(), 0.0d);
    }

    /**
     * add adds given value to current, and sum returns current value
     */
    @Test
    public void testAddAndSum() {
        DoubleAdder ai = new DoubleAdder();
        ai.add(2.0);
        assertEquals(2.0, ai.sum(), 0.0d);
        ai.add(-4.0);
        assertEquals(-2.0, ai.sum(), 0.0d);
    }

    /**
     * reset() causes subsequent sum() to return zero
     */
    @Test
    public void testReset() {
        DoubleAdder ai = new DoubleAdder();
        ai.add(2.0);
        assertEquals(2.0, ai.sum(), 0.0d);
        ai.reset();
        assertEquals(0.0, ai.sum(), 0.0d);
    }

    /**
     * sumThenReset() returns sum; subsequent sum() returns zero
     */
    @Test
    public void testSumThenReset() {
        DoubleAdder ai = new DoubleAdder();
        ai.add(2.0);
        assertEquals(2.0, ai.sum(), 0.0d);
        assertEquals(2.0, ai.sumThenReset(), 0.0d);
        assertEquals(0.0, ai.sum(), 0.0d);
    }

    /**
     * a deserialized serialized adder holds same value
     */
    @Test
    public void testSerialization() throws Exception {
        DoubleAdder x = new DoubleAdder();
        DoubleAdder y = serialClone(x);
        assertNotSame(x, y);
        x.add(-22.0);
        DoubleAdder z = serialClone(x);
        assertEquals(-22.0, x.sum(), 0.0d);
        assertEquals(0.0, y.sum(), 0.0d);
        assertEquals(-22.0, z.sum(), 0.0d);
    }

    /**
     * toString returns current value.
     */
    @Test
    public void testToString() {
        DoubleAdder ai = new DoubleAdder();
        assertEquals(Double.toString(0.0), ai.toString());
        ai.add(1.0);
        assertEquals(Double.toString(1.0), ai.toString());
    }

    /**
     * intValue returns current value.
     */
    @Test
    public void testIntValue() {
        DoubleAdder ai = new DoubleAdder();
        assertEquals(0, ai.intValue());
        ai.add(1.0);
        assertEquals(1, ai.intValue());
    }

    /**
     * longValue returns current value.
     */
    @Test
    public void testLongValue() {
        DoubleAdder ai = new DoubleAdder();
        assertEquals(0, ai.longValue());
        ai.add(1.0);
        assertEquals(1, ai.longValue());
    }

    /**
     * floatValue returns current value.
     */
    @Test
    public void testFloatValue() {
        DoubleAdder ai = new DoubleAdder();
        assertEquals(0.0f, ai.floatValue(), 0.0f);
        ai.add(1.0);
        assertEquals(1.0f, ai.floatValue(), 0.0f);
    }

    /**
     * doubleValue returns current value.
     */
    @Test
    public void testDoubleValue() {
        DoubleAdder ai = new DoubleAdder();
        assertEquals(0.0, ai.doubleValue(), 0.0d);
        ai.add(1.0);
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
        DoubleAdder a = new DoubleAdder();
        CyclicBarrier barrier = new CyclicBarrier(nthreads + 1);
        for (int i = 0; i < nthreads; ++i)
            pool.execute(new AdderTask(a, barrier, incs));
        barrier.await();
        barrier.await();
        double total = (long)nthreads * incs;
        double sum = a.sum();
        assertEquals(sum, total, 0.0d);
        pool.shutdown();
    }

    static final class AdderTask implements Runnable {
        final DoubleAdder adder;
        final CyclicBarrier barrier;
        final int incs;
        volatile double result;
        AdderTask(DoubleAdder adder, CyclicBarrier barrier, int incs) {
            this.adder = adder;
            this.barrier = barrier;
            this.incs = incs;
        }

        public void run() {
            try {
                barrier.await();
                DoubleAdder a = adder;
                for (int i = 0; i < incs; ++i)
                    a.add(1.0);
                result = a.sum();
                barrier.await();
            } catch (Throwable t) { throw new Error(t); }
        }
    }

}

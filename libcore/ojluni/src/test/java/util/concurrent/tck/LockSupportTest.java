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
 * Written by Doug Lea and Martin Buchholz with assistance from
 * members of JCP JSR-166 Expert Group and released to the public
 * domain, as explained at
 * http://creativecommons.org/publicdomain/zero/1.0/
 * Other contributors include Andrew Wright, Jeffrey Hayes,
 * Pat Fisher, Mike Judd.
 */

package test.java.util.concurrent.tck;
import static java.util.concurrent.TimeUnit.MILLISECONDS;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertNotSame;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.locks.LockSupport;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

// Android-changed: Use JUnit4.
@RunWith(JUnit4.class)
public class LockSupportTest extends JSR166TestCase {
    // Android-changed: Use JUnitCore.main.
    public static void main(String[] args) {
        // main(suite(), args);
        org.junit.runner.JUnitCore.main("test.java.util.concurrent.tck.LockSupportTest");
    }

    // public static Test suite() {
    //     return new TestSuite(LockSupportTest.class);
    // }

    static {
        // Reduce the risk of rare disastrous classloading in first call to
        // LockSupport.park: https://bugs.openjdk.java.net/browse/JDK-8074773
        Class<?> ensureLoaded = LockSupport.class;
    }

    /**
     * Returns the blocker object used by tests in this file.
     * Any old object will do; we'll return a convenient one.
     */
    static Object theBlocker() {
        return LockSupportTest.class;
    }

    enum ParkMethod {
        park() {
            void park() {
                LockSupport.park();
            }
            void park(long millis) {
                throw new UnsupportedOperationException();
            }
        },
        parkUntil() {
            void park(long millis) {
                LockSupport.parkUntil(deadline(millis));
            }
        },
        parkNanos() {
            void park(long millis) {
                LockSupport.parkNanos(MILLISECONDS.toNanos(millis));
            }
        },
        parkBlocker() {
            void park() {
                LockSupport.park(theBlocker());
            }
            void park(long millis) {
                throw new UnsupportedOperationException();
            }
        },
        parkUntilBlocker() {
            void park(long millis) {
                LockSupport.parkUntil(theBlocker(), deadline(millis));
            }
        },
        parkNanosBlocker() {
            void park(long millis) {
                LockSupport.parkNanos(theBlocker(),
                                      MILLISECONDS.toNanos(millis));
            }
        };

        void park() { park(2 * LONG_DELAY_MS); }
        abstract void park(long millis);

        /** Returns a deadline to use with parkUntil. */
        long deadline(long millis) {
            // beware of rounding
            return System.currentTimeMillis() + millis + 1;
        }
    }

    /**
     * park is released by subsequent unpark
     */
    @Test
    public void testParkBeforeUnpark_park() {
        testParkBeforeUnpark(ParkMethod.park);
    }
    @Test
    public void testParkBeforeUnpark_parkNanos() {
        testParkBeforeUnpark(ParkMethod.parkNanos);
    }
    @Test
    public void testParkBeforeUnpark_parkUntil() {
        testParkBeforeUnpark(ParkMethod.parkUntil);
    }
    @Test
    public void testParkBeforeUnpark_parkBlocker() {
        testParkBeforeUnpark(ParkMethod.parkBlocker);
    }
    @Test
    public void testParkBeforeUnpark_parkNanosBlocker() {
        testParkBeforeUnpark(ParkMethod.parkNanosBlocker);
    }
    @Test
    public void testParkBeforeUnpark_parkUntilBlocker() {
        testParkBeforeUnpark(ParkMethod.parkUntilBlocker);
    }
    private void testParkBeforeUnpark(final ParkMethod parkMethod) {
        final CountDownLatch pleaseUnpark = new CountDownLatch(1);
        Thread t = newStartedThread(new CheckedRunnable() {
            public void realRun() {
                pleaseUnpark.countDown();
                parkMethod.park();
            }});

        await(pleaseUnpark);
        LockSupport.unpark(t);
        awaitTermination(t);
    }

    /**
     * park is released by preceding unpark
     */
    @Test
    public void testParkAfterUnpark_park() {
        testParkAfterUnpark(ParkMethod.park);
    }
    @Test
    public void testParkAfterUnpark_parkNanos() {
        testParkAfterUnpark(ParkMethod.parkNanos);
    }
    @Test
    public void testParkAfterUnpark_parkUntil() {
        testParkAfterUnpark(ParkMethod.parkUntil);
    }
    @Test
    public void testParkAfterUnpark_parkBlocker() {
        testParkAfterUnpark(ParkMethod.parkBlocker);
    }
    @Test
    public void testParkAfterUnpark_parkNanosBlocker() {
        testParkAfterUnpark(ParkMethod.parkNanosBlocker);
    }
    @Test
    public void testParkAfterUnpark_parkUntilBlocker() {
        testParkAfterUnpark(ParkMethod.parkUntilBlocker);
    }
    private void testParkAfterUnpark(final ParkMethod parkMethod) {
        final CountDownLatch pleaseUnpark = new CountDownLatch(1);
        final AtomicBoolean pleasePark = new AtomicBoolean(false);
        Thread t = newStartedThread(new CheckedRunnable() {
            public void realRun() {
                pleaseUnpark.countDown();
                while (!pleasePark.get())
                    Thread.yield();
                parkMethod.park();
            }});

        await(pleaseUnpark);
        LockSupport.unpark(t);
        pleasePark.set(true);
        awaitTermination(t);
    }

    /**
     * park is released by subsequent interrupt
     */
    @Test
    public void testParkBeforeInterrupt_park() {
        testParkBeforeInterrupt(ParkMethod.park);
    }
    @Test
    public void testParkBeforeInterrupt_parkNanos() {
        testParkBeforeInterrupt(ParkMethod.parkNanos);
    }
    @Test
    public void testParkBeforeInterrupt_parkUntil() {
        testParkBeforeInterrupt(ParkMethod.parkUntil);
    }
    @Test
    public void testParkBeforeInterrupt_parkBlocker() {
        testParkBeforeInterrupt(ParkMethod.parkBlocker);
    }
    @Test
    public void testParkBeforeInterrupt_parkNanosBlocker() {
        testParkBeforeInterrupt(ParkMethod.parkNanosBlocker);
    }
    @Test
    public void testParkBeforeInterrupt_parkUntilBlocker() {
        testParkBeforeInterrupt(ParkMethod.parkUntilBlocker);
    }
    private void testParkBeforeInterrupt(final ParkMethod parkMethod) {
        final CountDownLatch pleaseInterrupt = new CountDownLatch(1);
        Thread t = newStartedThread(new CheckedRunnable() {
            public void realRun() {
                pleaseInterrupt.countDown();
                do {
                    parkMethod.park();
                    // park may return spuriously
                } while (! Thread.currentThread().isInterrupted());
            }});

        await(pleaseInterrupt);
        assertThreadStaysAlive(t);
        t.interrupt();
        awaitTermination(t);
    }

    /**
     * park is released by preceding interrupt
     */
    @Test
    public void testParkAfterInterrupt_park() {
        testParkAfterInterrupt(ParkMethod.park);
    }
    @Test
    public void testParkAfterInterrupt_parkNanos() {
        testParkAfterInterrupt(ParkMethod.parkNanos);
    }
    @Test
    public void testParkAfterInterrupt_parkUntil() {
        testParkAfterInterrupt(ParkMethod.parkUntil);
    }
    @Test
    public void testParkAfterInterrupt_parkBlocker() {
        testParkAfterInterrupt(ParkMethod.parkBlocker);
    }
    @Test
    public void testParkAfterInterrupt_parkNanosBlocker() {
        testParkAfterInterrupt(ParkMethod.parkNanosBlocker);
    }
    @Test
    public void testParkAfterInterrupt_parkUntilBlocker() {
        testParkAfterInterrupt(ParkMethod.parkUntilBlocker);
    }
    private void testParkAfterInterrupt(final ParkMethod parkMethod) {
        final CountDownLatch pleaseInterrupt = new CountDownLatch(1);
        final AtomicBoolean pleasePark = new AtomicBoolean(false);
        Thread t = newStartedThread(new CheckedRunnable() {
            public void realRun() throws Exception {
                pleaseInterrupt.countDown();
                while (!pleasePark.get())
                    Thread.yield();
                assertTrue(Thread.currentThread().isInterrupted());
                parkMethod.park();
                assertTrue(Thread.currentThread().isInterrupted());
            }});

        await(pleaseInterrupt);
        t.interrupt();
        pleasePark.set(true);
        awaitTermination(t);
    }

    /**
     * timed park times out if not unparked
     */
    @Test
    public void testParkTimesOut_parkNanos() {
        testParkTimesOut(ParkMethod.parkNanos);
    }
    @Test
    public void testParkTimesOut_parkUntil() {
        testParkTimesOut(ParkMethod.parkUntil);
    }
    @Test
    public void testParkTimesOut_parkNanosBlocker() {
        testParkTimesOut(ParkMethod.parkNanosBlocker);
    }
    @Test
    public void testParkTimesOut_parkUntilBlocker() {
        testParkTimesOut(ParkMethod.parkUntilBlocker);
    }
    private void testParkTimesOut(final ParkMethod parkMethod) {
        Thread t = newStartedThread(new CheckedRunnable() {
            public void realRun() {
                for (;;) {
                    long startTime = System.nanoTime();
                    parkMethod.park(timeoutMillis());
                    // park may return spuriously
                    if (millisElapsedSince(startTime) >= timeoutMillis())
                        return;
                }
            }});

        awaitTermination(t);
    }

    /**
     * getBlocker(null) throws NullPointerException
     */
    @Test
    public void testGetBlockerNull() {
        try {
            LockSupport.getBlocker(null);
            shouldThrow();
        } catch (NullPointerException success) {}
    }

    /**
     * getBlocker returns the blocker object passed to park
     */
    @Test
    public void testGetBlocker_parkBlocker() {
        testGetBlocker(ParkMethod.parkBlocker);
    }
    @Test
    public void testGetBlocker_parkNanosBlocker() {
        testGetBlocker(ParkMethod.parkNanosBlocker);
    }
    @Test
    public void testGetBlocker_parkUntilBlocker() {
        testGetBlocker(ParkMethod.parkUntilBlocker);
    }
    private void testGetBlocker(final ParkMethod parkMethod) {
        final CountDownLatch started = new CountDownLatch(1);
        Thread t = newStartedThread(new CheckedRunnable() {
            public void realRun() {
                Thread t = Thread.currentThread();
                started.countDown();
                do {
                    assertNull(LockSupport.getBlocker(t));
                    parkMethod.park();
                    assertNull(LockSupport.getBlocker(t));
                    // park may return spuriously
                } while (! Thread.currentThread().isInterrupted());
            }});

        long startTime = System.nanoTime();
        await(started);
        for (;;) {
            Object x = LockSupport.getBlocker(t);
            if (x == theBlocker()) { // success
                t.interrupt();
                awaitTermination(t);
                assertNull(LockSupport.getBlocker(t));
                return;
            } else {
                assertNull(x);  // ok
                if (millisElapsedSince(startTime) > LONG_DELAY_MS)
                    fail("timed out");
                Thread.yield();
            }
        }
    }

    /**
     * timed park(0) returns immediately.
     *
     * Requires hotspot fix for:
     * 6763959 java.util.concurrent.locks.LockSupport.parkUntil(0) blocks forever
     * which is in jdk7-b118 and 6u25.
     */
    @Test
    public void testPark0_parkNanos() {
        testPark0(ParkMethod.parkNanos);
    }
    @Test
    public void testPark0_parkUntil() {
        testPark0(ParkMethod.parkUntil);
    }
    @Test
    public void testPark0_parkNanosBlocker() {
        testPark0(ParkMethod.parkNanosBlocker);
    }
    @Test
    public void testPark0_parkUntilBlocker() {
        testPark0(ParkMethod.parkUntilBlocker);
    }
    private void testPark0(final ParkMethod parkMethod) {
        Thread t = newStartedThread(new CheckedRunnable() {
            public void realRun() {
                parkMethod.park(0L);
            }});

        awaitTermination(t);
    }

    /**
     * timed park(Long.MIN_VALUE) returns immediately.
     */
    @Test
    public void testParkNeg_parkNanos() {
        testParkNeg(ParkMethod.parkNanos);
    }
    @Test
    public void testParkNeg_parkUntil() {
        testParkNeg(ParkMethod.parkUntil);
    }
    @Test
    public void testParkNeg_parkNanosBlocker() {
        testParkNeg(ParkMethod.parkNanosBlocker);
    }
    @Test
    public void testParkNeg_parkUntilBlocker() {
        testParkNeg(ParkMethod.parkUntilBlocker);
    }
    private void testParkNeg(final ParkMethod parkMethod) {
        Thread t = newStartedThread(new CheckedRunnable() {
            public void realRun() {
                parkMethod.park(Long.MIN_VALUE);
            }});

        awaitTermination(t);
    }
}

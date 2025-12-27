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

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

// Android-changed: Use JUnit4.
@RunWith(JUnit4.class)
public class CountDownLatchTest extends JSR166TestCase {
    // Android-changed: Use JUnitCore.main.
    public static void main(String[] args) {
        // main(suite(), args);
        org.junit.runner.JUnitCore.main("test.java.util.concurrent.tck.CountDownLatchTest");
    }
    // public static Test suite() {
    //     return new TestSuite(CountDownLatchTest.class);
    // }

    /**
     * negative constructor argument throws IAE
     */
    @Test
    public void testConstructor() {
        try {
            new CountDownLatch(-1);
            shouldThrow();
        } catch (IllegalArgumentException success) {}
    }

    /**
     * getCount returns initial count and decreases after countDown
     */
    @Test
    public void testGetCount() {
        final CountDownLatch l = new CountDownLatch(2);
        assertEquals(2, l.getCount());
        l.countDown();
        assertEquals(1, l.getCount());
    }

    /**
     * countDown decrements count when positive and has no effect when zero
     */
    @Test
    public void testCountDown() {
        final CountDownLatch l = new CountDownLatch(1);
        assertEquals(1, l.getCount());
        l.countDown();
        assertEquals(0, l.getCount());
        l.countDown();
        assertEquals(0, l.getCount());
    }

    /**
     * await returns after countDown to zero, but not before
     */
    @Test
    public void testAwait() {
        final CountDownLatch l = new CountDownLatch(2);
        final CountDownLatch pleaseCountDown = new CountDownLatch(1);

        Thread t = newStartedThread(new CheckedRunnable() {
            public void realRun() throws InterruptedException {
                assertEquals(2, l.getCount());
                pleaseCountDown.countDown();
                l.await();
                assertEquals(0, l.getCount());
            }});

        await(pleaseCountDown);
        assertEquals(2, l.getCount());
        l.countDown();
        assertEquals(1, l.getCount());
        assertThreadStaysAlive(t);
        l.countDown();
        assertEquals(0, l.getCount());
        awaitTermination(t);
    }

    /**
     * timed await returns after countDown to zero
     */
    @Test
    public void testTimedAwait() {
        final CountDownLatch l = new CountDownLatch(2);
        final CountDownLatch pleaseCountDown = new CountDownLatch(1);

        Thread t = newStartedThread(new CheckedRunnable() {
            public void realRun() throws InterruptedException {
                assertEquals(2, l.getCount());
                pleaseCountDown.countDown();
                assertTrue(l.await(LONG_DELAY_MS, MILLISECONDS));
                assertEquals(0, l.getCount());
            }});

        await(pleaseCountDown);
        assertEquals(2, l.getCount());
        l.countDown();
        assertEquals(1, l.getCount());
        assertThreadStaysAlive(t);
        l.countDown();
        assertEquals(0, l.getCount());
        awaitTermination(t);
    }

    /**
     * await throws IE if interrupted before counted down
     */
    @Test
    public void testAwait_Interruptible() {
        final CountDownLatch l = new CountDownLatch(1);
        final CountDownLatch pleaseInterrupt = new CountDownLatch(1);
        Thread t = newStartedThread(new CheckedRunnable() {
            public void realRun() throws InterruptedException {
                Thread.currentThread().interrupt();
                try {
                    l.await();
                    shouldThrow();
                } catch (InterruptedException success) {}
                assertFalse(Thread.interrupted());

                pleaseInterrupt.countDown();
                try {
                    l.await();
                    shouldThrow();
                } catch (InterruptedException success) {}
                assertFalse(Thread.interrupted());

                assertEquals(1, l.getCount());
            }});

        await(pleaseInterrupt);
        assertThreadStaysAlive(t);
        t.interrupt();
        awaitTermination(t);
    }

    /**
     * timed await throws IE if interrupted before counted down
     */
    @Test
    public void testTimedAwait_Interruptible() {
        final CountDownLatch l = new CountDownLatch(1);
        final CountDownLatch pleaseInterrupt = new CountDownLatch(1);
        Thread t = newStartedThread(new CheckedRunnable() {
            public void realRun() throws InterruptedException {
                Thread.currentThread().interrupt();
                try {
                    l.await(LONG_DELAY_MS, MILLISECONDS);
                    shouldThrow();
                } catch (InterruptedException success) {}
                assertFalse(Thread.interrupted());

                pleaseInterrupt.countDown();
                try {
                    l.await(LONG_DELAY_MS, MILLISECONDS);
                    shouldThrow();
                } catch (InterruptedException success) {}
                assertFalse(Thread.interrupted());

                assertEquals(1, l.getCount());
            }});

        await(pleaseInterrupt);
        assertThreadStaysAlive(t);
        t.interrupt();
        awaitTermination(t);
    }

    /**
     * timed await times out if not counted down before timeout
     */
    @Test
    public void testAwaitTimeout() throws InterruptedException {
        final CountDownLatch l = new CountDownLatch(1);
        Thread t = newStartedThread(new CheckedRunnable() {
            public void realRun() throws InterruptedException {
                assertEquals(1, l.getCount());
                assertFalse(l.await(timeoutMillis(), MILLISECONDS));
                assertEquals(1, l.getCount());
            }});

        awaitTermination(t);
        assertEquals(1, l.getCount());
    }

    /**
     * toString indicates current count
     */
    @Test
    public void testToString() {
        CountDownLatch s = new CountDownLatch(2);
        assertTrue(s.toString().contains("Count = 2"));
        s.countDown();
        assertTrue(s.toString().contains("Count = 1"));
        s.countDown();
        assertTrue(s.toString().contains("Count = 0"));
    }

}

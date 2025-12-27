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

package libcore.java.util.concurrent;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import libcore.test.annotation.NonCts;
import libcore.test.reasons.NonCtsReasons;

import java.util.List;
import java.util.Optional;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.ForkJoinTask;
import java.util.concurrent.ForkJoinWorkerThread;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicReference;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import jdk.internal.vm.ThreadContainer;
import jdk.internal.vm.ThreadContainers;

@RunWith(JUnit4.class)
public class ForkJoinPoolTest {

    @Test
    public void testSubmit() {
        final ForkJoinPool pool = new ForkJoinPool();
        try (ExecutorServiceAutoCloseable cleaner = new ExecutorServiceAutoCloseable(pool)) {
            final AtomicInteger result = new AtomicInteger(0);
            ForkJoinTask task = pool.submit(() -> result.addAndGet(42), result);
            assertSame(result, task.get());
            assertEquals(42, result.get());
        } catch(Throwable t) {
            fail("Unexpected exception: " + t.getMessage());
        }
    }

    @Test
    public void testGetRunningThreadCount() {
        final ForkJoinPool pool = new ForkJoinPool();
        try (ExecutorServiceAutoCloseable cleaner = new ExecutorServiceAutoCloseable(pool)) {
            assertEquals(0, pool.getRunningThreadCount());

            final AtomicInteger value = new AtomicInteger(0);
            final AtomicBoolean stop = new AtomicBoolean(false);
            final CountDownLatch startPending = new CountDownLatch(1);
            ForkJoinTask task = pool.submit(new Runnable() {
                    public void run() {
                        startPending.countDown();
                        while(!stop.get()) {
                            value.incrementAndGet();
                        }
                        stop.set(false);
                    }
                });
            while (startPending.getCount() > 0) {
                Thread.yield();
            }
            assertEquals(1, pool.getRunningThreadCount());
            stop.set(true);
            task.join();
            pool.awaitTermination(2000, TimeUnit.MILLISECONDS);
            assertEquals(0, pool.getRunningThreadCount());
        } catch(Throwable t) {
            fail("Unexpected exception: " + t.getMessage());
        }
    }

    @NonCts(reason = NonCtsReasons.INTERNAL_APIS)
    @Test
    public void testThreadContainerTracking() throws Exception {
        final ForkJoinPool pool = new ForkJoinPool();
        try (ExecutorServiceAutoCloseable cleaner = new ExecutorServiceAutoCloseable(pool)) {
            final AtomicReference<Thread> result = new AtomicReference<>(null);
            ForkJoinTask<AtomicReference<Thread>> task = pool.submit(() -> {
                Thread cur = Thread.currentThread();
                // Avoid using lambda. http://b/417565895
                List<ThreadContainer> list = ThreadContainers.root().children().toList();
                for (ThreadContainer c : list) {
                    if (!c.name().startsWith("ForkJoinPool-")) {
                        continue;
                    }
                    List<Thread> threads = c.threads().toList();
                    for (Thread th : threads) {
                        if (cur.equals(th)) {
                            result.set(cur);
                            break;
                        }
                    }
                    if (result.get() != null) {
                        break;
                    }
                }
            }, result);
            assertSame(result, task.get());
            Thread th = result.get();
            assertNotNull("ForkJoinWorkerThread wasn't found in the shared container", th);
            assertTrue(th instanceof ForkJoinWorkerThread);
        }
    }

    @Test
    public void testConstructor_withKeepAliveTime() {

        try {
            ForkJoinPool pool = new ForkJoinPool(0,
                    ForkJoinPool.defaultForkJoinWorkerThreadFactory,
                    /*handler*/ null,
                    /*asyncMode*/ false,
                    /*corePoolSize*/ 10,
                    /*maximumPoolSize*/ 10,
                    /*minimumRunnable*/ 1,
                    /*saturate*/ null,
                    /*keepAliveTime*/ 60,
                    /*unit*/ TimeUnit.SECONDS);
            fail("Expected IllegalArgumentException when parallelism is 0");
        } catch (IllegalArgumentException e) {
        }

        try {
            ForkJoinPool pool = new ForkJoinPool(-1,
                    ForkJoinPool.defaultForkJoinWorkerThreadFactory,
                    /*handler*/ null,
                    /*asyncMode*/ false,
                    /*corePoolSize*/ 10,
                    /*maximumPoolSize*/ 10,
                    /*minimumRunnable*/ 1,
                    /*saturate*/ null,
                    /*keepAliveTime*/ 60,
                    /*unit*/ TimeUnit.SECONDS);
            fail("Expected IllegalArgumentException when parallelism is less than 0");
        } catch (IllegalArgumentException e) {
        }

        try {
            ForkJoinPool pool = new ForkJoinPool(8,
                    ForkJoinPool.defaultForkJoinWorkerThreadFactory,
                    /*handler*/ null,
                    /*asyncMode*/ false,
                    /*corePoolSize*/ 10,
                    /*maximumPoolSize*/ 5,
                    /*minimumRunnable*/ 1,
                    /*saturate*/ null,
                    /*keepAliveTime*/ 60,
                    /*unit*/ TimeUnit.SECONDS);
            fail("Expected IllegalArgumentException when maximumPoolSize is less than parallelism");
        } catch (IllegalArgumentException e) {
        }

        try {
            ForkJoinPool pool = new ForkJoinPool(8,
                    ForkJoinPool.defaultForkJoinWorkerThreadFactory,
                    /*handler*/ null,
                    /*asyncMode*/ false,
                    /*corePoolSize*/ 10,
                    /*maximumPoolSize*/ 10,
                    /*minimumRunnable*/ 1,
                    /*saturate*/ null,
                    /*keepAliveTime*/ 0,
                    /*unit*/ TimeUnit.SECONDS);
            fail("Expected IllegalArgumentException when keepAlivetime is 0");
        } catch (IllegalArgumentException e) {
        }

        try {
            ForkJoinPool pool = new ForkJoinPool(8,
                    null,
                    /*handler*/ null,
                    /*asyncMode*/ false,
                    /*corePoolSize*/ 10,
                    /*maximumPoolSize*/ 10,
                    /*minimumRunnable*/ 1,
                    /*saturate*/ null,
                    /*keepAliveTime*/ 60,
                    /*unit*/ TimeUnit.SECONDS);
            fail("Expected NullPointerException when factory is null");
        } catch (NullPointerException e) {
        }

        ForkJoinPool pool = new ForkJoinPool(8,
                ForkJoinPool.defaultForkJoinWorkerThreadFactory,
                /*handler*/ null,
                /*asyncMode*/ false,
                /*corePoolSize*/ 10,
                /*maximumPoolSize*/ 10,
                /*minimumRunnable*/ 1,
                /*saturate*/ null,
                /*keepAliveTime*/ 60,
                /*unit*/ TimeUnit.SECONDS);
    }
}

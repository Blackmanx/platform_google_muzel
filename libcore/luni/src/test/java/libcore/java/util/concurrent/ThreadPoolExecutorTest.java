/*
 * Copyright (C) 2016 The Android Open Source Project
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

import static org.junit.Assert.assertNotNull;

import libcore.test.annotation.NonCts;
import libcore.test.reasons.NonCtsReasons;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.util.List;
import java.util.Optional;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import jdk.internal.vm.ThreadContainer;
import jdk.internal.vm.ThreadContainers;

@RunWith(JUnit4.class)
public class ThreadPoolExecutorTest {

    // http://b/27702221
    @Test
    public void testCorePoolSizeGreaterThanMax() {
        ThreadPoolExecutor tp = new ThreadPoolExecutor(
                1 /* core pool size */, 1 /* max pool size */,
                1000, TimeUnit.MILLISECONDS, new ArrayBlockingQueue<>(10));

        // It should be illegal to set a core pool size that's larger than the max
        // pool size but apps have been allowed to get away with it so far. The pattern
        // below occurs in a commonly used library. Note that the executor is in a
        // consistent state at the end of both method calls.
        tp.setCorePoolSize(5);
        tp.setMaximumPoolSize(5);
    }

    @NonCts(reason = NonCtsReasons.INTERNAL_APIS)
    @Test
    public void testThreadContainerTracking() throws InterruptedException {
        try (ThreadPoolExecutor tp = new ThreadPoolExecutor(
                1 /* core pool size */, 1 /* max pool size */,
                1000, TimeUnit.MILLISECONDS, new ArrayBlockingQueue<>(10))) {

            CountDownLatch latch = new CountDownLatch(1);
            final AtomicReference<Thread> result = new AtomicReference<>(null);
            tp.execute(() -> {
                Thread cur = Thread.currentThread();
                // Avoid using lambda. http://b/417565895
                List<ThreadContainer> list = ThreadContainers.root().children().toList();
                for (ThreadContainer c : list) {
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
                latch.countDown();
            });
            latch.await();
            Thread th = result.get();
            assertNotNull("The pooled thread isn't found in the thread container", th);
        }
    }
}

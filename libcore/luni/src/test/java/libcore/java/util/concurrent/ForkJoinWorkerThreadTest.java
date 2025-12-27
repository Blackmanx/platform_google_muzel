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

package libcore.java.util.concurrent;

import static org.junit.Assert.assertSame;

import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.ForkJoinWorkerThread;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class ForkJoinWorkerThreadTest {

    @Test
    public void testCtorWithThreadGroup() {
        final ThreadGroup group = new ThreadGroup("test-thread-group");
        final ForkJoinPool pool = new ForkJoinPool();
        ForkJoinWorkerThread worker = new WorkerThreadImpl(group, pool, true);
        assertSame(group, worker.getThreadGroup());
        assertSame(pool, worker.getPool());
    }

    private static class WorkerThreadImpl extends ForkJoinWorkerThread {
        public WorkerThreadImpl(ThreadGroup group, ForkJoinPool pool,
                                    boolean preserveThreadLocals) {
            super(group, pool, preserveThreadLocals);
        }
    }
}

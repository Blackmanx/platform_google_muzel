/*
 * Copyright (C) 2022 The Android Open Source Project
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

import static org.hamcrest.CoreMatchers.instanceOf;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.concurrent.Callable;
import java.util.concurrent.Future;
import java.util.concurrent.FutureTask;
import java.util.concurrent.atomic.AtomicReference;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class FutureTaskTest {

    /*
     * See b/241297967.
     */
    @Test
    public void testRecursiveToString() {
        TestCallable callable = new TestCallable();
        FutureTask task = new FutureTask(callable);
        callable.set(task);
        String placeholder = task.toString();
        assertFalse(callable.hasDetectedRecursiveToString());
    }

    @Test
    public void testExceptionNow_OnSuccess() {
        TestCallable callable = new TestCallable();
        FutureTask task = new FutureTask(callable);
        assertThrows(IllegalStateException.class, () -> { task.exceptionNow(); });
        task.run();
        assertTrue(task.isDone());
        assertThrows(IllegalStateException.class, () -> { task.exceptionNow(); });
    }

    @Test
    public void testExceptionNow_OnCancel() {
        TestCallable callable = new TestCallable();
        FutureTask task = new FutureTask(callable);
        assertThrows(IllegalStateException.class, () -> { task.exceptionNow(); });
        task.cancel(false);
        assertTrue(task.isCancelled());
        assertThrows(IllegalStateException.class, () -> { task.exceptionNow(); });
    }

    @Test
    public void testExceptionNow_OnFail() {
        TestCallable callable = new TestCallable();
        FutureTask task = new FutureTask(callable);
        assertThrows(IllegalStateException.class, () -> { task.exceptionNow(); });
        callable.failNext();
        task.run();
        assertTrue(task.isDone());
        assertThat(task.exceptionNow(), instanceOf(TestException.class));
    }

    @Test
    public void testResultNow_OnSuccess() {
        TestCallable callable = new TestCallable();
        FutureTask task = new FutureTask(callable);
        assertThrows(IllegalStateException.class, () -> { task.resultNow(); });
        task.run();
        assertTrue(task.isDone());
        assertEquals(Integer.valueOf(42), task.resultNow());
    }

    @Test
    public void testResultNow_OnCancel() {
        TestCallable callable = new TestCallable();
        FutureTask task = new FutureTask(callable);
        assertThrows(IllegalStateException.class, () -> { task.resultNow(); });
        task.cancel(false);
        assertTrue(task.isCancelled());
        assertThrows(IllegalStateException.class, () -> { task.resultNow(); });
    }

    @Test
    public void testResultNow_OnFail() {
        TestCallable callable = new TestCallable();
        FutureTask task = new FutureTask(callable);
        assertThrows(IllegalStateException.class, () -> { task.resultNow(); });
        callable.failNext();
        task.run();
        assertTrue(task.isDone());
        assertThrows(IllegalStateException.class, () -> { task.resultNow(); });
    }

    @Test
    public void testState_OnSuccess() {
        TestCallable callable = new TestCallable();
        FutureTask task = new FutureTask(callable);
        assertEquals(Future.State.RUNNING, task.state());
        task.run();
        assertEquals(Future.State.SUCCESS, task.state());
    }

    @Test
    public void testState_OnCancel() {
        TestCallable callable = new TestCallable();
        FutureTask task = new FutureTask(callable);
        assertEquals(Future.State.RUNNING, task.state());
        task.cancel(false);
        assertEquals(Future.State.CANCELLED, task.state());
    }

    @Test
    public void testState_OnFail() {
        TestCallable callable = new TestCallable();
        FutureTask task = new FutureTask(callable);
        assertEquals(Future.State.RUNNING, task.state());
        callable.failNext();
        task.run();
        assertEquals(Future.State.FAILED, task.state());
    }

    static class TestCallable extends AtomicReference<FutureTask> implements Callable<Integer> {

        private int toStringCalls = 0;
        private boolean fail = false;

        @Override
        public Integer call() throws Exception {
            if (!fail) {
                return Integer.valueOf(42);
            }
            throw new TestException();
        }

        public boolean hasDetectedRecursiveToString() {
            return toStringCalls >= 3;
        }

        @Override
        public String toString() {
            if (toStringCalls < 3) {
                ++toStringCalls;
                return super.toString();
            } else {
                return "";
            }
        }

        public void failNext() {
            fail = true;
        }
    }

    static class TestException extends Exception { }

}

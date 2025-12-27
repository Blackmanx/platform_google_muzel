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

import static org.hamcrest.CoreMatchers.instanceOf;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;

import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.TimeUnit;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class FutureTest {

    @Test
    public void testExceptionNow_OnSuccess() {
        TestFuture future = new TestFuture();
        assertThrows(IllegalStateException.class, () -> { future.exceptionNow(); });
        future.setResult(Integer.valueOf(2));
        assertTrue(future.isDone());
        assertThrows(IllegalStateException.class, () -> { future.exceptionNow(); });
    }

    @Test
    public void testExceptionNow_OnCancel() {
        TestFuture future = new TestFuture();
        assertThrows(IllegalStateException.class, () -> { future.exceptionNow(); });
        future.cancel(false);
        assertTrue(future.isCancelled());
        assertThrows(IllegalStateException.class, () -> { future.exceptionNow(); });
    }

    @Test
    public void testExceptionNow_OnFail() {
        TestFuture future = new TestFuture();
        assertThrows(IllegalStateException.class, () -> { future.exceptionNow(); });
        future.failNext();
        assertTrue(future.isDone());
        assertThat(future.exceptionNow(), instanceOf(TestException.class));
    }

    @Test
    public void testResultNow_OnSuccess() {
        TestFuture future = new TestFuture();
        assertThrows(IllegalStateException.class, () -> { future.resultNow(); });
        future.setResult(Integer.valueOf(2));
        assertTrue(future.isDone());
        assertEquals(Integer.valueOf(2), future.resultNow());
    }

    @Test
    public void testResultNow_OnCancel() {
        TestFuture future = new TestFuture();
        assertThrows(IllegalStateException.class, () -> { future.resultNow(); });
        future.cancel(false);
        assertTrue(future.isCancelled());
        assertThrows(IllegalStateException.class, () -> { future.resultNow(); });
    }

    @Test
    public void testResultNow_OnFail() {
        TestFuture future = new TestFuture();
        assertThrows(IllegalStateException.class, () -> { future.resultNow(); });
        future.failNext();
        assertTrue(future.isDone());
        assertThrows(IllegalStateException.class, () -> { future.resultNow(); });
    }

    @Test
    public void testState_OnSuccess() {
        TestFuture future = new TestFuture();
        assertEquals(Future.State.RUNNING, future.state());
        future.setResult(Integer.valueOf(2));
        assertEquals(Future.State.SUCCESS, future.state());
    }

    @Test
    public void testState_OnCancel() {
        TestFuture future = new TestFuture();
        assertEquals(Future.State.RUNNING, future.state());
        future.cancel(false);
        assertEquals(Future.State.CANCELLED, future.state());
    }

    @Test
    public void testState_OnFail() {
        TestFuture future = new TestFuture();
        assertEquals(Future.State.RUNNING, future.state());
        future.failNext();
        assertEquals(Future.State.FAILED, future.state());
    }

    static class TestFuture implements Future<Integer> {

        private Integer result = null;
        private boolean cancelled = false;
        private boolean fail = false;

        @Override
        public boolean cancel(boolean mayInterruptIfRunning){
            cancelled = true;
            return true;
        }

        @Override
        public boolean isCancelled() {
            return cancelled;
        }

        @Override
        public boolean isDone() {
            return (result != null || cancelled || fail);
        }

        @Override
        public Integer get() throws InterruptedException, ExecutionException {
            if (fail) {
                throw new ExecutionException(new TestException());
            } else if (cancelled) {
                throw new CancellationException();
            }
            return result;
        }

        @Override
        public Integer get(long timeout, TimeUnit unit)
                throws InterruptedException, ExecutionException, TimeoutException {
            return get();
        }

        public void failNext() {
            fail = true;
        }

        public void setResult(Integer val) {
            result = val;
        }
    }

    static class TestException extends Exception { }

}

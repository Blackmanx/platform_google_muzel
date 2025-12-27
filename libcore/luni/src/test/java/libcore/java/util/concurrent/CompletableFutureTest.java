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

import java.util.concurrent.Future;
import java.util.concurrent.CompletableFuture;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class CompletableFutureTest {

    @Test
    public void testExceptionNow_OnSuccess() {
        CompletableFuture<Integer> future = new CompletableFuture<>();
        assertThrows(IllegalStateException.class, () -> { future.exceptionNow(); });
        future.complete(Integer.valueOf(2));
        assertTrue(future.isDone());
        assertThrows(IllegalStateException.class, () -> { future.exceptionNow(); });
    }

    @Test
    public void testExceptionNow_OnCancel() {
        CompletableFuture<Integer> future = new CompletableFuture<>();
        assertThrows(IllegalStateException.class, () -> { future.exceptionNow(); });
        future.cancel(false);
        assertTrue(future.isCancelled());
        assertThrows(IllegalStateException.class, () -> { future.exceptionNow(); });
    }

    @Test
    public void testExceptionNow_OnFail() {
        CompletableFuture<Integer> future = new CompletableFuture<>();
        assertThrows(IllegalStateException.class, () -> { future.exceptionNow(); });
        future.completeExceptionally(new TestException());
        assertTrue(future.isDone());
        assertThat(future.exceptionNow(), instanceOf(TestException.class));
    }

    @Test
    public void testResultNow_OnSuccess() {
        CompletableFuture<Integer> future = new CompletableFuture<>();
        assertThrows(IllegalStateException.class, () -> { future.resultNow(); });
        future.complete(Integer.valueOf(2));
        assertTrue(future.isDone());
        assertEquals(Integer.valueOf(2), future.resultNow());
    }

    @Test
    public void testResultNow_OnCancel() {
        CompletableFuture<Integer> future = new CompletableFuture<>();
        assertThrows(IllegalStateException.class, () -> { future.resultNow(); });
        future.cancel(false);
        assertTrue(future.isCancelled());
        assertThrows(IllegalStateException.class, () -> { future.resultNow(); });
    }

    @Test
    public void testResultNow_OnFail() {
        CompletableFuture<Integer> future = new CompletableFuture<>();
        assertThrows(IllegalStateException.class, () -> { future.resultNow(); });
        future.completeExceptionally(new TestException());
        assertTrue(future.isDone());
        assertThrows(IllegalStateException.class, () -> { future.resultNow(); });
    }

    @Test
    public void testState_OnSuccess() {
        CompletableFuture<Integer> future = new CompletableFuture<>();
        assertEquals(Future.State.RUNNING, future.state());
        future.complete(Integer.valueOf(2));
        assertEquals(Future.State.SUCCESS, future.state());
    }

    @Test
    public void testState_OnCancel() {
        CompletableFuture<Integer> future = new CompletableFuture<>();
        assertEquals(Future.State.RUNNING, future.state());
        future.cancel(false);
        assertEquals(Future.State.CANCELLED, future.state());
    }

    @Test
    public void testState_OnFail() {
        CompletableFuture<Integer> future = new CompletableFuture<>();
        assertEquals(Future.State.RUNNING, future.state());
        future.completeExceptionally(new TestException());
        assertEquals(Future.State.FAILED, future.state());
    }

    static class TestException extends Exception { }

}

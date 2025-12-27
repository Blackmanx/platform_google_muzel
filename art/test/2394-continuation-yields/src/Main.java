/*
 * Copyright (C) 2025 The Android Open Source Project
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

import dalvik.system.VirtualThreadContext;

import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

import jdk.internal.access.SharedSecrets;
import jdk.internal.vm.Continuation;

/**
 * Continuation is an internal API in OpenJDK. It verifies that the Continuation run
 * on different carrier threads before and after yielding.
 */
public class Main {

    public static void main(String[] args) throws InterruptedException {
        if (!com.android.art.flags.Flags.virtualThreadImplV1()) {
            return;
        }
        // Exit if the thread throws any exception.
        Thread.setDefaultUncaughtExceptionHandler((t, e) -> {
            System.err.println("thread: " + t.getName());
            e.printStackTrace(System.err);
            System.exit(1);
        });

        testContinuationSleep();
    }

    private static void testContinuationSleep() throws InterruptedException {
        Runnable continuationTask = Main::continuationTask;
        long threadId = Long.MAX_VALUE - 1;
        VirtualThreadContext virtualThreadContext = new VirtualThreadContext(continuationTask,
                threadId);
        Continuation cont = new Continuation(virtualThreadContext);

        Timer timer = new Timer();
        LinkedBlockingQueue<Object> blockingQueue = new LinkedBlockingQueue<>(1);
        Runnable carrier2Task = () -> {
            cont.run();
            try {
                blockingQueue.put(new Object());
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        };
        Thread carrier2 = new Thread(carrier2Task);
        TimerTask sleepTask = new TimerTask() {
            @Override
            public void run() {
                carrier2.start();
            }
        };

        Runnable carrier1Task = () -> {
            cont.run();

            timer.schedule(sleepTask, 100L);
        };
        Thread carrier1 = new Thread(carrier1Task);
        carrier1.start();
        carrier1.join(1000L);

        blockingQueue.poll(1L, TimeUnit.SECONDS);
        carrier2.join();

        timer.cancel();
    }

    private static void continuationTask() {
        long tid1 = getCarrierThreadId();
        Continuation.yield(SharedSecrets.getJavaLangAccess().virtualThreadContinuationScope());
        long tid2 = getCarrierThreadId();
        if (tid1 == tid2) {
            throw new RuntimeException("tid shouldn't be the same: "
                    + tid1 + " != " + tid2);
        }
    }

    private static long getCarrierThreadId() {
        return SharedSecrets.getJavaLangAccess().currentCarrierThread().threadId();
    }
}

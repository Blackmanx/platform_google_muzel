/*
 * Copyright (C) 2006 The Android Open Source Project
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

import dalvik.system.VMRuntime;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

/**
 * Test some basic thread stuff.
 */
public class Main {
    public static void main(String[] args) throws Exception {
        System.loadLibrary(args[0]);
        System.out.println("thread test starting");
        testThreadCapacity();
        testThreadDaemons();
        testSleepZero();
        testSetName();
        testThreadPriorities();
        testMainThreadGroup();
        testMainThreadAllStackTraces();
        System.out.println("thread test done");
    }

    /*
     * Simple thread capacity test.
     */
    private static void testThreadCapacity() throws Exception {
        TestCapacityThread[] threads = new TestCapacityThread[128];
        for (int i = 0; i < threads.length; i++) {
            threads[i] = new TestCapacityThread();
        }

        for (TestCapacityThread thread : threads) {
            thread.start();
        }
        for (TestCapacityThread thread : threads) {
            thread.join();
        }

        System.out.println("testThreadCapacity thread count: " + TestCapacityThread.mCount);
    }

    private static class TestCapacityThread extends Thread {
        static int mCount = 0;
        public void run() {
            synchronized (TestCapacityThread.class) {
                ++mCount;
            }
            try {
                sleep(1000);
            } catch (Exception ex) {
            }
        }
    }

    private static void testThreadDaemons() {
        Thread t = new Thread(null, new TestDaemonThread(), "TestDaemonThread", 7168);

        t.setDaemon(false);

        System.out.print("testThreadDaemons starting thread '" + t.getName() + "'\n");
        t.start();

        try {
            t.join();
        } catch (InterruptedException ex) {
            ex.printStackTrace(System.out);
        }

        System.out.print("testThreadDaemons finished\n");
    }

    private static class TestDaemonThread implements Runnable {
        public void run() {
            System.out.print("testThreadDaemons @ Thread running\n");

            try {
                Thread.currentThread().setDaemon(true);
                System.out.print("testThreadDaemons @ FAILED: setDaemon() succeeded\n");
            } catch (IllegalThreadStateException itse) {
                System.out.print("testThreadDaemons @ Got expected setDaemon exception\n");
            }

            try {
                Thread.sleep(2000);
            }
            catch (InterruptedException ie) {
                System.out.print("testThreadDaemons @ Interrupted!\n");
            }
            finally {
                System.out.print("testThreadDaemons @ Thread bailing\n");
            }
        }
    }

    private static void testSleepZero() throws Exception {
        Thread.currentThread().interrupt();
        try {
            Thread.sleep(0);
            throw new AssertionError("unreachable");
        } catch (InterruptedException e) {
            if (Thread.currentThread().isInterrupted()) {
                throw new AssertionError("thread is interrupted");
            }
        }
        System.out.print("testSleepZero finished\n");
    }

    private static void testSetName() throws Exception {
        System.out.print("testSetName starting\n");
        Thread thread = new Thread() {
            @Override
            public void run() {
                System.out.print("testSetName running\n");
            }
        };
        thread.start();
        thread.setName("HelloWorld");  // b/17302037 hang if setName called after start
        if (!thread.getName().equals("HelloWorld")) {
            throw new AssertionError("Unexpected thread name: " + thread.getName());
        }
        thread.join();
        if (!thread.getName().equals("HelloWorld")) {
            throw new AssertionError("Unexpected thread name after join: " + thread.getName());
        }
        System.out.print("testSetName finished\n");
    }

    private static final int NICENESS_UNKNOWN = -1000;

    private static void testThreadPriorities() throws Exception {
        System.out.print("testThreadPriorities starting\n");

        Thread.currentThread().setPriority(Thread.NORM_PRIORITY);
        if (supportsThreadPriorities()) {
            if (haveNicenessApis()) {
                // Should have no impact, because it's not inherited.
                VMRuntime.getRuntime().setThreadNiceness(Thread.currentThread(), 10);
            }
            PriorityStoringThread t1 = new PriorityStoringThread(false);
            t1.setPriority(Thread.MAX_PRIORITY);
            t1.start();
            if (t1.getPriority() != Thread.MAX_PRIORITY) {
                System.out.print("thread priority for t1 while running was " + t1.getPriority() +
                    " [expected Thread.MAX_PRIORITY]\n");
                System.out.println(getPriorityInfo());
            }
            t1.join();
            if (t1.getPriority() != Thread.MAX_PRIORITY) {
                System.out.print("thread priority for t1 while running was " + t1.getPriority() +
                    " [expected Thread.MAX_PRIORITY]\n");
                System.out.println(getPriorityInfo());
            }
            if (t1.nativePriority != Thread.MAX_PRIORITY) {
                System.out.print("thread priority for t1 was " + t1.nativePriority +
                    " [expected Thread.MAX_PRIORITY]\n");
                System.out.println(getPriorityInfo());
            }
            if (t1.reportedPriority != Thread.MAX_PRIORITY) {
                System.out.println("Reported Java priority is wrong");
            }
            if (t1.reportedNiceness != NICENESS_UNKNOWN && t1.reportedNiceness >= 0) {
                // Niceness should correspond to MAX_PRIORITY.
                System.out.println("Reported Java niceness is wrong");
            }

            // Repeat, raising parent priority instead of child priority.
            // Results should be the same.
            Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
            PriorityStoringThread t2 = new PriorityStoringThread(false);
            t2.start();
            if (t2.getPriority() != Thread.MAX_PRIORITY) {
                System.out.print("thread priority for t2 while running was " + t2.getPriority() +
                    " [expected Thread.MAX_PRIORITY]\n");
                System.out.println(getPriorityInfo());
            }
            t2.join();
            if (t2.getPriority() != Thread.MAX_PRIORITY) {
                System.out.print("thread priority for t2 while running was " + t2.getPriority() +
                    " [expected Thread.MAX_PRIORITY]\n");
                System.out.println(getPriorityInfo());
            }
            if (t2.nativePriority != Thread.MAX_PRIORITY) {
                System.out.print("thread priority for t2 was " + t2.nativePriority +
                    " [expected Thread.MAX_PRIORITY]\n");
                System.out.println(getPriorityInfo());
            }
            if (t2.reportedPriority != Thread.MAX_PRIORITY) {
                System.out.println("Reported Java priority is wrong");
            }
            if (t2.reportedNiceness != NICENESS_UNKNOWN && t2.reportedNiceness >= 0) {
                // Niceness should correspond to MAX_PRIORITY.
                System.out.println("Reported Java niceness is wrong");
            }
            Thread.currentThread().setPriority(Thread.NORM_PRIORITY);

            PriorityStoringThread t3 = new PriorityStoringThread(true);
            t3.start();
            t3.join();
            if (t3.nativePriority != Thread.MAX_PRIORITY) {
                System.out.print("thread priority for t3 was " + t3.nativePriority +
                    " [expected Thread.MAX_PRIORITY]\n");
                System.out.println(getPriorityInfo());
            }
        }
        if (Thread.currentThread().getPriority() != Thread.NORM_PRIORITY) {
            System.out.println("Parent priority unexpectedly altered.");
        }
        if (haveNicenessApis()) {
            PriorityStoringThread t4 = new PriorityStoringThread(false);
            VMRuntime.getRuntime().setThreadNiceness(t4, 19);
            // Since priority is inherited on creation, this should affect child thread.
            t4.start();
            t4.join();
            if (supportsThreadPriorities() && t4.nativePriority != Thread.MIN_PRIORITY) {
                System.out.print("Native thread priority for t4 was " + t4.nativePriority +
                                 " [expected Thread.MIN_PRIORITY]\n");
                System.out.println(getPriorityInfo());
            }
            // t4's children should have MAX_PRIORITY, since only niceness was changed.
            if (t4.reportedPriority != Thread.NORM_PRIORITY) {
                System.out.println("Reported t4 Java priority (" + t4.reportedPriority +
                                   ") should be " + Thread.NORM_PRIORITY);
            }
            if (t4.reportedNiceness != 19) {
                // Niceness should correspond to 19.
                System.out.println("Reported Java niceness is wrong");
            }
        }

        System.out.println("testThreadPriorities finished");
    }

    private static void testMainThreadGroup() {
      Thread threads[] = new Thread[10];
      Thread current = Thread.currentThread();
      current.getThreadGroup().enumerate(threads);

      for (Thread t : threads) {
        if (t == current) {
          System.out.println("Found current Thread in ThreadGroup");
          return;
        }
      }
      throw new RuntimeException("Did not find main thread: " + Arrays.toString(threads));
    }

    private static void testMainThreadAllStackTraces() {
      StackTraceElement[] trace = Thread.getAllStackTraces().get(Thread.currentThread());
      if (trace == null) {
        throw new RuntimeException("Did not find main thread: " + Thread.getAllStackTraces());
      }
      List<StackTraceElement> list = Arrays.asList(trace);
      Iterator<StackTraceElement> it = list.iterator();
      while (it.hasNext()) {
        StackTraceElement ste = it.next();
        if (ste.getClassName().equals("Main")) {
          if (!ste.getMethodName().equals("testMainThreadAllStackTraces")) {
            throw new RuntimeException(list.toString());
          }

          StackTraceElement ste2 = it.next();
          if (!ste2.getClassName().equals("Main")) {
            throw new RuntimeException(list.toString());
          }
          if (!ste2.getMethodName().equals("main")) {
            throw new RuntimeException(list.toString());
          }

          System.out.println("Found expected stack in getAllStackTraces()");
          return;
        }
      }
      throw new RuntimeException(list.toString());
    }

    private static native int getNativePriority();
    private static native boolean supportsThreadPriorities();
    private static native String getPriorityInfo();
    private static native boolean haveNicenessApis();

    static class PriorityStoringThread extends Thread {
        private final boolean setPriority;
        volatile int nativePriority;
        volatile int reportedPriority;
        volatile int reportedNiceness;

        public PriorityStoringThread(boolean setPriority) {
            this.setPriority = setPriority;
            this.nativePriority = -1;
            this.reportedPriority = -1;
            this.reportedNiceness = NICENESS_UNKNOWN;
        }

        @Override
        public void run() {
            if (setPriority) {
                setPriority(Thread.MAX_PRIORITY);
            }

            nativePriority = Main.getNativePriority();
            reportedPriority = getPriority();
            if (haveNicenessApis()) {
                this.reportedNiceness =
                    VMRuntime.getRuntime().getThreadNiceness(Thread.currentThread());
            }

        }
    }
}

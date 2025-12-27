/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
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
 */


package java.util.concurrent;

@SuppressWarnings({"unchecked", "deprecation", "all"})
public class Executors {

Executors() { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.Callable<java.lang.Object> callable(java.lang.Runnable task) { throw new RuntimeException("Stub!"); }

public static <T> java.util.concurrent.Callable<T> callable(java.lang.Runnable task, T result) { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.Callable<java.lang.Object> callable(java.security.PrivilegedAction<?> action) { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.Callable<java.lang.Object> callable(java.security.PrivilegedExceptionAction<?> action) { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ThreadFactory defaultThreadFactory() { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ExecutorService newCachedThreadPool() { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ExecutorService newCachedThreadPool(java.util.concurrent.ThreadFactory threadFactory) { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ExecutorService newFixedThreadPool(int nThreads) { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ExecutorService newFixedThreadPool(int nThreads, java.util.concurrent.ThreadFactory threadFactory) { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ScheduledExecutorService newScheduledThreadPool(int corePoolSize) { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ScheduledExecutorService newScheduledThreadPool(int corePoolSize, java.util.concurrent.ThreadFactory threadFactory) { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ExecutorService newSingleThreadExecutor() { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ExecutorService newSingleThreadExecutor(java.util.concurrent.ThreadFactory threadFactory) { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ScheduledExecutorService newSingleThreadScheduledExecutor() { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ScheduledExecutorService newSingleThreadScheduledExecutor(java.util.concurrent.ThreadFactory threadFactory) { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_VIRTUAL_THREAD_API_V1)
public static java.util.concurrent.ExecutorService newThreadPerTaskExecutor(java.util.concurrent.ThreadFactory threadFactory) { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_VIRTUAL_THREAD_API_V1)
public static java.util.concurrent.ExecutorService newVirtualThreadPerTaskExecutor() { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ExecutorService newWorkStealingPool() { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ExecutorService newWorkStealingPool(int parallelism) { throw new RuntimeException("Stub!"); }

@Deprecated
public static <T> java.util.concurrent.Callable<T> privilegedCallable(java.util.concurrent.Callable<T> callable) { throw new RuntimeException("Stub!"); }

@Deprecated
public static <T> java.util.concurrent.Callable<T> privilegedCallableUsingCurrentClassLoader(java.util.concurrent.Callable<T> callable) { throw new RuntimeException("Stub!"); }

@Deprecated
public static java.util.concurrent.ThreadFactory privilegedThreadFactory() { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ExecutorService unconfigurableExecutorService(java.util.concurrent.ExecutorService executor) { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ScheduledExecutorService unconfigurableScheduledExecutorService(java.util.concurrent.ScheduledExecutorService executor) { throw new RuntimeException("Stub!"); }
}


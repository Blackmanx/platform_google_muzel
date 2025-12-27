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
public abstract class ForkJoinTask<V> implements java.util.concurrent.Future<V>, java.io.Serializable {

public ForkJoinTask() { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ForkJoinTask<?> adapt(java.lang.Runnable runnable) { throw new RuntimeException("Stub!"); }

public static <T> java.util.concurrent.ForkJoinTask<T> adapt(java.lang.Runnable runnable, T result) { throw new RuntimeException("Stub!"); }

public static <T> java.util.concurrent.ForkJoinTask<T> adapt(java.util.concurrent.Callable<? extends T> callable) { throw new RuntimeException("Stub!"); }

public static <T> java.util.concurrent.ForkJoinTask<T> adaptInterruptible(java.util.concurrent.Callable<? extends T> callable) { throw new RuntimeException("Stub!"); }

public boolean cancel(boolean mayInterruptIfRunning) { throw new RuntimeException("Stub!"); }

public final boolean compareAndSetForkJoinTaskTag(short expect, short update) { throw new RuntimeException("Stub!"); }

public void complete(V value) { throw new RuntimeException("Stub!"); }

public void completeExceptionally(java.lang.Throwable ex) { throw new RuntimeException("Stub!"); }

public java.lang.Throwable exceptionNow() { throw new RuntimeException("Stub!"); }

protected abstract boolean exec();

public final java.util.concurrent.ForkJoinTask<V> fork() { throw new RuntimeException("Stub!"); }

public final V get() throws java.util.concurrent.ExecutionException, java.lang.InterruptedException { throw new RuntimeException("Stub!"); }

public final V get(long timeout, java.util.concurrent.TimeUnit unit) throws java.util.concurrent.ExecutionException, java.lang.InterruptedException, java.util.concurrent.TimeoutException { throw new RuntimeException("Stub!"); }

public final java.lang.Throwable getException() { throw new RuntimeException("Stub!"); }

public final short getForkJoinTaskTag() { throw new RuntimeException("Stub!"); }

public static java.util.concurrent.ForkJoinPool getPool() { throw new RuntimeException("Stub!"); }

public static int getQueuedTaskCount() { throw new RuntimeException("Stub!"); }

public abstract V getRawResult();

public static int getSurplusQueuedTaskCount() { throw new RuntimeException("Stub!"); }

public static void helpQuiesce() { throw new RuntimeException("Stub!"); }

public static boolean inForkJoinPool() { throw new RuntimeException("Stub!"); }

public final V invoke() { throw new RuntimeException("Stub!"); }

public static <T extends java.util.concurrent.ForkJoinTask<?>> java.util.Collection<T> invokeAll(java.util.Collection<T> tasks) { throw new RuntimeException("Stub!"); }

public static void invokeAll(java.util.concurrent.ForkJoinTask<?> t1, java.util.concurrent.ForkJoinTask<?> t2) { throw new RuntimeException("Stub!"); }

public static void invokeAll(java.util.concurrent.ForkJoinTask<?>... tasks) { throw new RuntimeException("Stub!"); }

public final boolean isCancelled() { throw new RuntimeException("Stub!"); }

public final boolean isCompletedAbnormally() { throw new RuntimeException("Stub!"); }

public final boolean isCompletedNormally() { throw new RuntimeException("Stub!"); }

public final boolean isDone() { throw new RuntimeException("Stub!"); }

public final V join() { throw new RuntimeException("Stub!"); }

protected static java.util.concurrent.ForkJoinTask<?> peekNextLocalTask() { throw new RuntimeException("Stub!"); }

protected static java.util.concurrent.ForkJoinTask<?> pollNextLocalTask() { throw new RuntimeException("Stub!"); }

protected static java.util.concurrent.ForkJoinTask<?> pollTask() { throw new RuntimeException("Stub!"); }

public final void quietlyComplete() { throw new RuntimeException("Stub!"); }

public final void quietlyInvoke() { throw new RuntimeException("Stub!"); }

public final void quietlyJoin() { throw new RuntimeException("Stub!"); }

public final boolean quietlyJoin(long timeout, java.util.concurrent.TimeUnit unit) throws java.lang.InterruptedException { throw new RuntimeException("Stub!"); }

public final boolean quietlyJoinUninterruptibly(long timeout, java.util.concurrent.TimeUnit unit) { throw new RuntimeException("Stub!"); }

public void reinitialize() { throw new RuntimeException("Stub!"); }

public V resultNow() { throw new RuntimeException("Stub!"); }

public final short setForkJoinTaskTag(short newValue) { throw new RuntimeException("Stub!"); }

protected abstract void setRawResult(V value);

public java.util.concurrent.Future.State state() { throw new RuntimeException("Stub!"); }

public boolean tryUnfork() { throw new RuntimeException("Stub!"); }
}


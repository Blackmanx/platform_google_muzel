/*
 * Copyright (C) 2014 The Android Open Source Project
 * Copyright (c) 1995, 2021, Oracle and/or its affiliates. All rights reserved.
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


package java.lang;

@SuppressWarnings({"unchecked", "deprecation", "all"})
public class ThreadGroup implements java.lang.Thread.UncaughtExceptionHandler {

public ThreadGroup(java.lang.String name) { throw new RuntimeException("Stub!"); }

public ThreadGroup(java.lang.ThreadGroup parent, java.lang.String name) { throw new RuntimeException("Stub!"); }

public int activeCount() { throw new RuntimeException("Stub!"); }

public int activeGroupCount() { throw new RuntimeException("Stub!"); }

@Deprecated
public boolean allowThreadSuspension(boolean b) { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_25_V1_APIS)
@Deprecated
public final void checkAccess() { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_25_V1_APIS)
@Deprecated
public final void destroy() { throw new RuntimeException("Stub!"); }

public int enumerate(java.lang.Thread[] list) { throw new RuntimeException("Stub!"); }

public int enumerate(java.lang.Thread[] list, boolean recurse) { throw new RuntimeException("Stub!"); }

public int enumerate(java.lang.ThreadGroup[] list) { throw new RuntimeException("Stub!"); }

public int enumerate(java.lang.ThreadGroup[] list, boolean recurse) { throw new RuntimeException("Stub!"); }

public final int getMaxPriority() { throw new RuntimeException("Stub!"); }

public final java.lang.String getName() { throw new RuntimeException("Stub!"); }

public final java.lang.ThreadGroup getParent() { throw new RuntimeException("Stub!"); }

public final void interrupt() { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_25_V1_APIS)
@Deprecated
public final boolean isDaemon() { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_25_V1_APIS)
@Deprecated
public synchronized boolean isDestroyed() { throw new RuntimeException("Stub!"); }

public void list() { throw new RuntimeException("Stub!"); }

public final boolean parentOf(java.lang.ThreadGroup g) { throw new RuntimeException("Stub!"); }

@Deprecated
public final void resume() { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_25_V1_APIS)
@Deprecated
public final void setDaemon(boolean daemon) { throw new RuntimeException("Stub!"); }

public final void setMaxPriority(int pri) { throw new RuntimeException("Stub!"); }

@Deprecated
public final void stop() { throw new RuntimeException("Stub!"); }

@Deprecated
public final void suspend() { throw new RuntimeException("Stub!"); }

public java.lang.String toString() { throw new RuntimeException("Stub!"); }

public void uncaughtException(java.lang.Thread t, java.lang.Throwable e) { throw new RuntimeException("Stub!"); }
}


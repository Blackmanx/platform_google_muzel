/*
 * Copyright (c) 2023, 2025, Oracle and/or its affiliates. All rights reserved.
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

package java.util.stream;

@SuppressWarnings({"unchecked", "deprecation", "all"})
@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_21_V2_APIS)
public final class Gatherers {

Gatherers() { throw new RuntimeException("Stub!"); }

public static <T, R> java.util.stream.Gatherer<T,?,R> fold(java.util.function.Supplier<R> initial, java.util.function.BiFunction<? super R,? super T,? extends R> folder) { throw new RuntimeException("Stub!"); }

public static <T, R> java.util.stream.Gatherer<T,?,R> scan(java.util.function.Supplier<R> initial, java.util.function.BiFunction<? super R,? super T,? extends R> scanner) { throw new RuntimeException("Stub!"); }

public static <TR> java.util.stream.Gatherer<TR,?,java.util.List<TR>> windowFixed(int windowSize) { throw new RuntimeException("Stub!"); }

public static <TR> java.util.stream.Gatherer<TR,?,java.util.List<TR>> windowSliding(int windowSize) { throw new RuntimeException("Stub!"); }
}


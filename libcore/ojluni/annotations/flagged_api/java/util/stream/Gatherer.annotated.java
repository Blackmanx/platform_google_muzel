/*
 * Copyright (c) 2023, 2024, Oracle and/or its affiliates. All rights reserved.
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
public interface Gatherer<T, A, R> {

public default <RR> java.util.stream.Gatherer<T,?,RR> andThen(java.util.stream.Gatherer<? super R,?,? extends RR> that) { throw new RuntimeException("Stub!"); }

public default java.util.function.BinaryOperator<A> combiner() { throw new RuntimeException("Stub!"); }

public static <A> java.util.function.BinaryOperator<A> defaultCombiner() { throw new RuntimeException("Stub!"); }

public static <A, R> java.util.function.BiConsumer<A,java.util.stream.Gatherer.Downstream<? super R>> defaultFinisher() { throw new RuntimeException("Stub!"); }

public static <A> java.util.function.Supplier<A> defaultInitializer() { throw new RuntimeException("Stub!"); }

public default java.util.function.BiConsumer<A,java.util.stream.Gatherer.Downstream<? super R>> finisher() { throw new RuntimeException("Stub!"); }

public default java.util.function.Supplier<A> initializer() { throw new RuntimeException("Stub!"); }

public java.util.stream.Gatherer.Integrator<A,T,R> integrator();

public static <T, A, R> java.util.stream.Gatherer<T,A,R> of(java.util.function.Supplier<A> initializer, java.util.stream.Gatherer.Integrator<A,T,R> integrator, java.util.function.BinaryOperator<A> combiner, java.util.function.BiConsumer<A,java.util.stream.Gatherer.Downstream<? super R>> finisher) { throw new RuntimeException("Stub!"); }

public static <T, R> java.util.stream.Gatherer<T,java.lang.Void,R> of(java.util.stream.Gatherer.Integrator<java.lang.Void,T,R> integrator) { throw new RuntimeException("Stub!"); }

public static <T, R> java.util.stream.Gatherer<T,java.lang.Void,R> of(java.util.stream.Gatherer.Integrator<java.lang.Void,T,R> integrator, java.util.function.BiConsumer<java.lang.Void,java.util.stream.Gatherer.Downstream<? super R>> finisher) { throw new RuntimeException("Stub!"); }

public static <T, A, R> java.util.stream.Gatherer<T,A,R> ofSequential(java.util.function.Supplier<A> initializer, java.util.stream.Gatherer.Integrator<A,T,R> integrator) { throw new RuntimeException("Stub!"); }

public static <T, A, R> java.util.stream.Gatherer<T,A,R> ofSequential(java.util.function.Supplier<A> initializer, java.util.stream.Gatherer.Integrator<A,T,R> integrator, java.util.function.BiConsumer<A,java.util.stream.Gatherer.Downstream<? super R>> finisher) { throw new RuntimeException("Stub!"); }

public static <T, R> java.util.stream.Gatherer<T,java.lang.Void,R> ofSequential(java.util.stream.Gatherer.Integrator<java.lang.Void,T,R> integrator) { throw new RuntimeException("Stub!"); }

public static <T, R> java.util.stream.Gatherer<T,java.lang.Void,R> ofSequential(java.util.stream.Gatherer.Integrator<java.lang.Void,T,R> integrator, java.util.function.BiConsumer<java.lang.Void,java.util.stream.Gatherer.Downstream<? super R>> finisher) { throw new RuntimeException("Stub!"); }
@SuppressWarnings({"unchecked", "deprecation", "all"})
@java.lang.FunctionalInterface
@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_21_V2_APIS)
public static interface Downstream<T> {

public default boolean isRejecting() { throw new RuntimeException("Stub!"); }

public boolean push(T element);
}

@SuppressWarnings({"unchecked", "deprecation", "all"})
@java.lang.FunctionalInterface
@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_21_V2_APIS)
public static interface Integrator<A, T, R> {

public boolean integrate(A state, T element, java.util.stream.Gatherer.Downstream<? super R> downstream);

public static <A, T, R> java.util.stream.Gatherer.Integrator<A,T,R> of(java.util.stream.Gatherer.Integrator<A,T,R> integrator) { throw new RuntimeException("Stub!"); }

public static <A, T, R> java.util.stream.Gatherer.Integrator.Greedy<A,T,R> ofGreedy(java.util.stream.Gatherer.Integrator.Greedy<A,T,R> greedy) { throw new RuntimeException("Stub!"); }
@SuppressWarnings({"unchecked", "deprecation", "all"})
@java.lang.FunctionalInterface
@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_21_V2_APIS)
public static interface Greedy<A, T, R> extends java.util.stream.Gatherer.Integrator<A,T,R> {
}

}

}


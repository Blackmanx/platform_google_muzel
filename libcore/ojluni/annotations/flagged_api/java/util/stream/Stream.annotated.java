/*
 * Copyright (c) 2012, 2024, Oracle and/or its affiliates. All rights reserved.
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
public interface Stream<T> extends java.util.stream.BaseStream<T,java.util.stream.Stream<T>> {

public boolean allMatch(java.util.function.Predicate<? super T> predicate);

public boolean anyMatch(java.util.function.Predicate<? super T> predicate);

public static <T> java.util.stream.Stream.Builder<T> builder() { throw new RuntimeException("Stub!"); }

public <R> R collect(java.util.function.Supplier<R> supplier, java.util.function.BiConsumer<R,? super T> accumulator, java.util.function.BiConsumer<R,R> combiner);

public <R, A> R collect(java.util.stream.Collector<? super T,A,R> collector);

public static <T> java.util.stream.Stream<T> concat(java.util.stream.Stream<? extends T> a, java.util.stream.Stream<? extends T> b) { throw new RuntimeException("Stub!"); }

public long count();

public java.util.stream.Stream<T> distinct();

public default java.util.stream.Stream<T> dropWhile(java.util.function.Predicate<? super T> predicate) { throw new RuntimeException("Stub!"); }

public static <T> java.util.stream.Stream<T> empty() { throw new RuntimeException("Stub!"); }

public java.util.stream.Stream<T> filter(java.util.function.Predicate<? super T> predicate);

public java.util.Optional<T> findAny();

public java.util.Optional<T> findFirst();

public <R> java.util.stream.Stream<R> flatMap(java.util.function.Function<? super T,? extends java.util.stream.Stream<? extends R>> mapper);

public java.util.stream.DoubleStream flatMapToDouble(java.util.function.Function<? super T,? extends java.util.stream.DoubleStream> mapper);

public java.util.stream.IntStream flatMapToInt(java.util.function.Function<? super T,? extends java.util.stream.IntStream> mapper);

public java.util.stream.LongStream flatMapToLong(java.util.function.Function<? super T,? extends java.util.stream.LongStream> mapper);

public void forEach(java.util.function.Consumer<? super T> action);

public void forEachOrdered(java.util.function.Consumer<? super T> action);

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_21_V2_APIS)
public default <R> java.util.stream.Stream<R> gather(java.util.stream.Gatherer<? super T,?,R> gatherer) { throw new RuntimeException("Stub!"); }

public static <T> java.util.stream.Stream<T> generate(java.util.function.Supplier<? extends T> s) { throw new RuntimeException("Stub!"); }

public static <T> java.util.stream.Stream<T> iterate(T seed, java.util.function.Predicate<? super T> hasNext, java.util.function.UnaryOperator<T> next) { throw new RuntimeException("Stub!"); }

public static <T> java.util.stream.Stream<T> iterate(T seed, java.util.function.UnaryOperator<T> f) { throw new RuntimeException("Stub!"); }

public java.util.stream.Stream<T> limit(long maxSize);

public <R> java.util.stream.Stream<R> map(java.util.function.Function<? super T,? extends R> mapper);

public default <R> java.util.stream.Stream<R> mapMulti(java.util.function.BiConsumer<? super T,? super java.util.function.Consumer<R>> mapper) { throw new RuntimeException("Stub!"); }

public default java.util.stream.DoubleStream mapMultiToDouble(java.util.function.BiConsumer<? super T,? super java.util.function.DoubleConsumer> mapper) { throw new RuntimeException("Stub!"); }

public default java.util.stream.IntStream mapMultiToInt(java.util.function.BiConsumer<? super T,? super java.util.function.IntConsumer> mapper) { throw new RuntimeException("Stub!"); }

public default java.util.stream.LongStream mapMultiToLong(java.util.function.BiConsumer<? super T,? super java.util.function.LongConsumer> mapper) { throw new RuntimeException("Stub!"); }

public java.util.stream.DoubleStream mapToDouble(java.util.function.ToDoubleFunction<? super T> mapper);

public java.util.stream.IntStream mapToInt(java.util.function.ToIntFunction<? super T> mapper);

public java.util.stream.LongStream mapToLong(java.util.function.ToLongFunction<? super T> mapper);

public java.util.Optional<T> max(java.util.Comparator<? super T> comparator);

public java.util.Optional<T> min(java.util.Comparator<? super T> comparator);

public boolean noneMatch(java.util.function.Predicate<? super T> predicate);

public static <T> java.util.stream.Stream<T> of(T t) { throw new RuntimeException("Stub!"); }

@java.lang.SafeVarargs
public static <T> java.util.stream.Stream<T> of(T... values) { throw new RuntimeException("Stub!"); }

public static <T> java.util.stream.Stream<T> ofNullable(T t) { throw new RuntimeException("Stub!"); }

public java.util.stream.Stream<T> peek(java.util.function.Consumer<? super T> action);

public java.util.Optional<T> reduce(java.util.function.BinaryOperator<T> accumulator);

public T reduce(T identity, java.util.function.BinaryOperator<T> accumulator);

public <U> U reduce(U identity, java.util.function.BiFunction<U,? super T,U> accumulator, java.util.function.BinaryOperator<U> combiner);

public java.util.stream.Stream<T> skip(long n);

public java.util.stream.Stream<T> sorted();

public java.util.stream.Stream<T> sorted(java.util.Comparator<? super T> comparator);

public default java.util.stream.Stream<T> takeWhile(java.util.function.Predicate<? super T> predicate) { throw new RuntimeException("Stub!"); }

public java.lang.Object[] toArray();

public <A> A[] toArray(java.util.function.IntFunction<A[]> generator);

public default java.util.List<T> toList() { throw new RuntimeException("Stub!"); }
@SuppressWarnings({"unchecked", "deprecation", "all"})
public static interface Builder<T> extends java.util.function.Consumer<T> {

public void accept(T t);

public default java.util.stream.Stream.Builder<T> add(T t) { throw new RuntimeException("Stub!"); }

public java.util.stream.Stream<T> build();
}

}


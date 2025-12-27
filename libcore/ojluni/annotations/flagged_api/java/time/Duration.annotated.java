/*
 * Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.
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
 * Copyright (c) 2007-2012, Stephen Colebourne & Michael Nascimento Santos
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither the name of JSR-310 nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package java.time;

@SuppressWarnings({"unchecked", "deprecation", "all"})
public final class Duration implements java.lang.Comparable<java.time.Duration>, java.io.Serializable, java.time.temporal.TemporalAmount {

Duration() { throw new RuntimeException("Stub!"); }

public java.time.Duration abs() { throw new RuntimeException("Stub!"); }

public java.time.temporal.Temporal addTo(java.time.temporal.Temporal temporal) { throw new RuntimeException("Stub!"); }

public static java.time.Duration between(java.time.temporal.Temporal startInclusive, java.time.temporal.Temporal endExclusive) { throw new RuntimeException("Stub!"); }

public int compareTo(java.time.Duration otherDuration) { throw new RuntimeException("Stub!"); }

public long dividedBy(java.time.Duration divisor) { throw new RuntimeException("Stub!"); }

public java.time.Duration dividedBy(long divisor) { throw new RuntimeException("Stub!"); }

public boolean equals(java.lang.Object other) { throw new RuntimeException("Stub!"); }

public static java.time.Duration from(java.time.temporal.TemporalAmount amount) { throw new RuntimeException("Stub!"); }

public long get(java.time.temporal.TemporalUnit unit) { throw new RuntimeException("Stub!"); }

public int getNano() { throw new RuntimeException("Stub!"); }

public long getSeconds() { throw new RuntimeException("Stub!"); }

public java.util.List<java.time.temporal.TemporalUnit> getUnits() { throw new RuntimeException("Stub!"); }

public int hashCode() { throw new RuntimeException("Stub!"); }

public boolean isNegative() { throw new RuntimeException("Stub!"); }


@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_OPENJDK_21_V2_APIS)
public boolean isPositive() { throw new RuntimeException("Stub!"); }

public boolean isZero() { throw new RuntimeException("Stub!"); }

public java.time.Duration minus(java.time.Duration duration) { throw new RuntimeException("Stub!"); }

public java.time.Duration minus(long amountToSubtract, java.time.temporal.TemporalUnit unit) { throw new RuntimeException("Stub!"); }

public java.time.Duration minusDays(long daysToSubtract) { throw new RuntimeException("Stub!"); }

public java.time.Duration minusHours(long hoursToSubtract) { throw new RuntimeException("Stub!"); }

public java.time.Duration minusMillis(long millisToSubtract) { throw new RuntimeException("Stub!"); }

public java.time.Duration minusMinutes(long minutesToSubtract) { throw new RuntimeException("Stub!"); }

public java.time.Duration minusNanos(long nanosToSubtract) { throw new RuntimeException("Stub!"); }

public java.time.Duration minusSeconds(long secondsToSubtract) { throw new RuntimeException("Stub!"); }

public java.time.Duration multipliedBy(long multiplicand) { throw new RuntimeException("Stub!"); }

public java.time.Duration negated() { throw new RuntimeException("Stub!"); }

public static java.time.Duration of(long amount, java.time.temporal.TemporalUnit unit) { throw new RuntimeException("Stub!"); }

public static java.time.Duration ofDays(long days) { throw new RuntimeException("Stub!"); }

public static java.time.Duration ofHours(long hours) { throw new RuntimeException("Stub!"); }

public static java.time.Duration ofMillis(long millis) { throw new RuntimeException("Stub!"); }

public static java.time.Duration ofMinutes(long minutes) { throw new RuntimeException("Stub!"); }

public static java.time.Duration ofNanos(long nanos) { throw new RuntimeException("Stub!"); }

public static java.time.Duration ofSeconds(long seconds) { throw new RuntimeException("Stub!"); }

public static java.time.Duration ofSeconds(long seconds, long nanoAdjustment) { throw new RuntimeException("Stub!"); }

public static java.time.Duration parse(java.lang.CharSequence text) { throw new RuntimeException("Stub!"); }

public java.time.Duration plus(java.time.Duration duration) { throw new RuntimeException("Stub!"); }

public java.time.Duration plus(long amountToAdd, java.time.temporal.TemporalUnit unit) { throw new RuntimeException("Stub!"); }

public java.time.Duration plusDays(long daysToAdd) { throw new RuntimeException("Stub!"); }

public java.time.Duration plusHours(long hoursToAdd) { throw new RuntimeException("Stub!"); }

public java.time.Duration plusMillis(long millisToAdd) { throw new RuntimeException("Stub!"); }

public java.time.Duration plusMinutes(long minutesToAdd) { throw new RuntimeException("Stub!"); }

public java.time.Duration plusNanos(long nanosToAdd) { throw new RuntimeException("Stub!"); }

public java.time.Duration plusSeconds(long secondsToAdd) { throw new RuntimeException("Stub!"); }

public java.time.temporal.Temporal subtractFrom(java.time.temporal.Temporal temporal) { throw new RuntimeException("Stub!"); }

public long toDays() { throw new RuntimeException("Stub!"); }

public long toDaysPart() { throw new RuntimeException("Stub!"); }

public long toHours() { throw new RuntimeException("Stub!"); }

public int toHoursPart() { throw new RuntimeException("Stub!"); }

public long toMillis() { throw new RuntimeException("Stub!"); }

public int toMillisPart() { throw new RuntimeException("Stub!"); }

public long toMinutes() { throw new RuntimeException("Stub!"); }

public int toMinutesPart() { throw new RuntimeException("Stub!"); }

public long toNanos() { throw new RuntimeException("Stub!"); }

public int toNanosPart() { throw new RuntimeException("Stub!"); }

public long toSeconds() { throw new RuntimeException("Stub!"); }

public int toSecondsPart() { throw new RuntimeException("Stub!"); }

public java.lang.String toString() { throw new RuntimeException("Stub!"); }

public java.time.Duration truncatedTo(java.time.temporal.TemporalUnit unit) { throw new RuntimeException("Stub!"); }

public java.time.Duration withNanos(int nanoOfSecond) { throw new RuntimeException("Stub!"); }

public java.time.Duration withSeconds(long seconds) { throw new RuntimeException("Stub!"); }

public static final java.time.Duration ZERO;
static { ZERO = null; }
}


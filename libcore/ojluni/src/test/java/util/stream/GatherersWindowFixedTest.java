/*
 * Copyright (c) 2023, 2024, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
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

package test.java.util.stream;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Gatherers;
import java.util.stream.Stream;

import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;
import static org.testng.Assert.*;

/**
 * @test
 * @summary Tests the API and contract of Gatherers.windowFixed
 * @run junit GatherersWindowFixedTest
 */

public class GatherersWindowFixedTest {

    record Config(int streamSize, boolean parallel) {
        Stream<Integer> stream() {
            var stream = Stream.iterate(1, i -> i + 1).limit(streamSize);
            stream = parallel ? stream.parallel() : stream.sequential();
            return stream;
        }
    }

    static final Stream<Integer> sizes(){
        return Stream.of(0,1,10,33,99,9999);
    }

    static final Stream<Config> sequentialAndParallel(int size) {
        return Stream.of(false, true)
                .map(parallel ->
                        new Config(size, parallel));
    }

    static final Stream<Config> configurations() {
        return sizes().flatMap(i -> sequentialAndParallel(i));
    }

    // Android-added: Avoid using JUnit jupiter API.
    @DataProvider(name="configTestData")
    static Object[][] configurationsTestData() {
        return configurations()
                .map(c -> new Object[] {c})
                .toArray(Object[][]::new);
    }

    static final Stream<Config> nonempty_configurations() {
        return sizes().filter(i -> i > 0).flatMap(i -> sequentialAndParallel(i));
    }

    // Android-added: Avoid using JUnit jupiter API.
    @DataProvider(name="nonEmptyConfigTestData")
    static Object[][] nonEmptyConfigurationsTestData() {
        return nonempty_configurations()
                .map(c -> new Object[] {c})
                .toArray(Object[][]::new);
    }

    // Android-changed: Avoid using JUnit jupiter API.
    // @ParameterizedTest
    // @ValueSource(ints = { Integer.MIN_VALUE, -999, -1, 0})
    @Test
    public void throwsIAEWhenWindowSizeIsSmallerThanOne() {
        throwsIAEWhenWindowSizeIsSmallerThanOne(Integer.MIN_VALUE);
        throwsIAEWhenWindowSizeIsSmallerThanOne(-999);
        throwsIAEWhenWindowSizeIsSmallerThanOne(-1);
        throwsIAEWhenWindowSizeIsSmallerThanOne(0);
    }

    public void throwsIAEWhenWindowSizeIsSmallerThanOne(int windowSize) {
        assertThrows(IllegalArgumentException.class,
                () -> Gatherers.windowFixed(windowSize));
    }

    // Android-changed: Avoid using JUnit jupiter API.
    // @ParameterizedTest
    // @MethodSource("nonempty_configurations")
    @Test(dataProvider = "nonEmptyConfigTestData")
    public void behavesAsExpectedWhenWindowSizeIsSizeOfStream(Config config) {
        final var streamSize = config.streamSize();
        final var result = config.stream()
                .gather(Gatherers.windowFixed(streamSize))
                .toList();
        assertEquals(1, result.size());
        assertEquals(config.stream().toList(), result.get(0));
    }

    @Test
    public void toleratesNullElements() {
        assertEquals(
                List.of(Arrays.asList(null, null)),
                Stream.of(null, null)
                    .gather(Gatherers.windowFixed(2))
                    .toList());
    }

    @Test
    public void throwsUOEWhenWindowsAreAttemptedToBeModified() {
        var window = Stream.of(1)
                .gather(Gatherers.windowFixed(1))
                .findFirst()
                .get();
        assertThrows(UnsupportedOperationException.class,
                () -> window.add(2));
    }

    // Android-changed: Avoid using JUnit jupiter API.
    // @ParameterizedTest
    // @MethodSource("configurations")
    @Test(dataProvider = "configTestData")
    public void behavesAsExpected(Config config) {
        final var streamSize = config.streamSize();
        // Tests that the layout of the returned data is as expected
        for (var windowSize : List.of(1, 2, 3, 10)) {
            final var expectLastWindowSize = streamSize % windowSize == 0 ? windowSize : streamSize % windowSize;
            final var expectedSize = (streamSize / windowSize) + ((streamSize % windowSize == 0) ? 0 : 1);

            final var expected = config.stream().toList().iterator();

            final var result = config.stream()
                    .gather(Gatherers.windowFixed(windowSize))
                    .toList();

            int currentWindow = 0;
            for (var window : result) {
                ++currentWindow;
                assertEquals(currentWindow < expectedSize ? windowSize : expectLastWindowSize, window.size());
                for (var element : window)
                    assertEquals(expected.next(), element);
            }

            assertEquals(expectedSize, currentWindow);
        }
    }
}

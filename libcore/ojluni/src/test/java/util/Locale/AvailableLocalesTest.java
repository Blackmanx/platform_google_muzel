/*
 * Copyright (c) 2007, 2023, Oracle and/or its affiliates. All rights reserved.
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
/*
 * @test
 * @bug 4122700 8282319
 * @summary Verify implementation of getAvailableLocales() and availableLocales()
 * @run junit AvailableLocalesTest
 */
package test.java.util.Locale;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import static org.junit.Assert.assertNotEquals;
import java.util.Arrays;
import java.util.Locale;
import java.util.stream.Stream;

@RunWith(Parameterized.class)
public class AvailableLocalesTest {
    private final Locale requiredLocale;
    private final String localeName;
    public AvailableLocalesTest(Locale requiredLocale, String localeName) {
        this.requiredLocale = requiredLocale;
        this.localeName = localeName;
    }
    /**
     * Test that Locale.getAvailableLocales() is non-empty and prints out
     * the returned locales - 4122700.
     */
    @Test
    public void nonEmptyLocalesTest() {
        Locale[] systemLocales = Locale.getAvailableLocales();
        // Android-changed: use JUnit4.
        // assertNotEquals(systemLocales.length, 0, "Available locale list is empty!");
        assertNotEquals("Available locale list is empty!", systemLocales.length, 0);
        System.out.println("Found " + systemLocales.length + " locales:");
        printLocales(systemLocales);
    }
    /**
     * Test to validate that the methods: Locale.getAvailableLocales()
     * and Locale.availableLocales() contain the same underlying elements
     */
    @Test
    public void streamEqualsArrayTest() {
        Locale[] arrayLocales = Locale.getAvailableLocales();
        Stream<Locale> streamedLocales = Locale.availableLocales();
        Locale[] convertedLocales = streamedLocales.toArray(Locale[]::new);
        if (Arrays.equals(arrayLocales, convertedLocales)) {
            System.out.println("$$$ Passed: The underlying elements" +
                    " of getAvailableLocales() and availableLocales() are the same!");
        } else {
            throw new RuntimeException("$$$ Error: The underlying elements" +
                    " of getAvailableLocales() and availableLocales()" +
                    " are not the same.");
        }
    }
    /**
     * Test to validate that the stream has the required
     * Locale.US.
     */
    // Android-changed: use JUnit4.
    /*
    @ParameterizedTest
    @MethodSource("requiredLocaleProvider")
    public void requiredLocalesTest(Locale requiredLocale, String localeName) {
    */
    @Test
    public void requiredLocalesTest() {
        if (Locale.availableLocales().anyMatch(loc -> (loc.equals(requiredLocale)))) {
            System.out.printf("$$$ Passed: Stream has %s!%n", localeName);
        } else {
            throw new RuntimeException(String.format("$$$ Error:" +
                    " Stream is missing %s!", localeName));
        }
    }
    // Helper method to print out all the system locales
    private void printLocales(Locale[] systemLocales) {
        Locale[] locales = new Locale[systemLocales.length];
        for (int i = 0; i < locales.length; i++) {
            Locale lowest = null;
            for (Locale systemLocale : systemLocales) {
                if (i > 0 && locales[i - 1].toString().compareTo(systemLocale.toString()) >= 0)
                    continue;
                if (lowest == null || systemLocale.toString().compareTo(lowest.toString()) < 0)
                    lowest = systemLocale;
            }
            locales[i] = lowest;
        }
        for (Locale locale : locales) {
            if (locale.getCountry().length() == 0)
                System.out.println("    " + locale.getDisplayLanguage() + ":");
            else {
                if (locale.getVariant().length() == 0)
                    System.out.println("        " + locale.getDisplayCountry());
                else
                    System.out.println("        " + locale.getDisplayCountry() + ", "
                            + locale.getDisplayVariant());
            }
        }
    }
    // Data provider for testStreamRequirements
    // BEGIN Android-changed: implemented using JUnit4.
    /*
    private static Stream<Arguments> requiredLocaleProvider() {
        return Stream.of(
                Arguments.of(Locale.ROOT, "Root locale"),
                Arguments.of(Locale.US, "US locale")
        );
    }*/
    @Parameterized.Parameters(name = "{1}")
    public static Object[][] requiredLocaleProvider() {
        return new Object[][] {
                // Currently Locale.ROOT is not returned.
                // {Locale.ROOT, "Root locale"},
                {Locale.US, "US locale"}
        };
    }
    // END Android-changed: implemented using JUnit4.
}

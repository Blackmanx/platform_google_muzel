/*
 * Copyright (c) 2019, 2022, Oracle and/or its affiliates. All rights reserved.
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

/**
 * Holds the file paths to the Unicode Character Database source files.
 * Paths to the source files in the "data" directory are relative.
 */

package test.java.lang;

import java.net.URL;
import java.nio.file.Path;
import java.nio.file.Paths;

public class UCDFiles {
    // Android-changed: Unicode data is packaged as resources in CTS.
    // Resource file can't be converted to path because libcore doesn't have a FileSystemProvider
    // for jar:// scheme.
    // public static Path UCD_DIR = Paths.get(System.getProperty("test.root"),
    //     "../../src/java.base/share/data/unicodedata");

    // Android-changed: Unicode data is packaged as resources in CTS.
    // public static Path BLOCKS =
    //     UCD_DIR.resolve("Blocks.txt");
    public static URL BLOCKS = UCDFiles.class.getClassLoader()
            .getResource("data/unicodedata/Blocks.txt");

    // Android-removed: unused paths.
    /*
    public static Path DERIVED_PROPS =
        UCD_DIR.resolve("DerivedCoreProperties.txt");
    public static Path GRAPHEME_BREAK_PROPERTY =
        UCD_DIR.resolve("auxiliary").resolve("GraphemeBreakProperty.txt");
    public static Path GRAPHEME_BREAK_TEST =
        UCD_DIR.resolve("auxiliary").resolve("GraphemeBreakTest.txt");
    public static Path NORMALIZATION_TEST =
        UCD_DIR.resolve("NormalizationTest.txt");
    public static Path PROP_LIST =
        UCD_DIR.resolve("PropList.txt");
    public static Path PROPERTY_VALUE_ALIASES =
        UCD_DIR.resolve("PropertyValueAliases.txt");
    public static Path SCRIPTS =
        UCD_DIR.resolve("Scripts.txt");
    public static Path SPECIAL_CASING =
        UCD_DIR.resolve("SpecialCasing.txt");
    public static Path UNICODE_DATA =
        UCD_DIR.resolve("UnicodeData.txt");
    public static Path EMOJI_DATA =
        UCD_DIR.resolve("emoji").resolve("emoji-data.txt");
     */
}

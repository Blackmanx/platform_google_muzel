/*
 * Copyright (C) 2025 The Android Open Source Project
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
package test.java.lang.Thread.virtual;

import org.junit.Assume;

public class Utils {

    public static void assumeVirtualThreadFlagTrue() {
        try {
            Assume.assumeTrue(com.android.art.flags.Flags.virtualThreadImplV1());
        } catch (NoSuchMethodError e) {
            System.logE("flag isn't found.", e);
            // Continue running tests as if the flag value was true, because in this case
            // it's likely that the APIs have been fully published and the flag has been removed.
            // Ideally, we should use the exported / test version of java_aconfig_library to read
            // the flag from the aconfig flag storage via frameworks, but ART test infra can't have
            // direct dependency on frameworks. We will need to add an abstraction or indirect
            // dependency to support both CTS infra and ART test infra.
        }
    }
}

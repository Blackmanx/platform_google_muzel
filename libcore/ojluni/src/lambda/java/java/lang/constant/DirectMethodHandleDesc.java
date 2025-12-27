/*
 * Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
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
package java.lang.constant;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandleInfo;
import java.util.OptionalInt;
import java.util.stream.Stream;

import static java.lang.invoke.MethodHandleInfo.REF_getField;
import static java.lang.invoke.MethodHandleInfo.REF_getStatic;
import static java.lang.invoke.MethodHandleInfo.REF_invokeInterface;
import static java.lang.invoke.MethodHandleInfo.REF_invokeSpecial;
import static java.lang.invoke.MethodHandleInfo.REF_invokeStatic;
import static java.lang.invoke.MethodHandleInfo.REF_invokeVirtual;
import static java.lang.invoke.MethodHandleInfo.REF_newInvokeSpecial;
import static java.lang.invoke.MethodHandleInfo.REF_putField;
import static java.lang.invoke.MethodHandleInfo.REF_putStatic;

/**
 * @hide
 */
public interface DirectMethodHandleDesc
        extends MethodHandleDesc {
    enum Kind {
        STATIC(REF_invokeStatic),
        INTERFACE_STATIC(REF_invokeStatic, true),
        VIRTUAL(REF_invokeVirtual),
        INTERFACE_VIRTUAL(REF_invokeInterface, true),
        SPECIAL(REF_invokeSpecial),
        INTERFACE_SPECIAL(REF_invokeSpecial, true),
        CONSTRUCTOR(REF_newInvokeSpecial),
        GETTER(REF_getField),
        SETTER(REF_putField),
        STATIC_GETTER(REF_getStatic),
        STATIC_SETTER(REF_putStatic);

        public final int refKind;

        public final boolean isInterface;
        Kind(int refKind) {
            this(refKind, false);
        }

        Kind(int refKind, boolean isInterface) { this.refKind = refKind; this.isInterface = isInterface; }

        public static Kind valueOf(int refKind) {
            return valueOf(refKind, refKind == REF_invokeInterface);
        }

        public static Kind valueOf(int refKind, boolean isInterface) {
            int i = tableIndex(refKind, isInterface);
            if (i >= 2 && i < TABLE.length) {
                return TABLE[i];
            }
            throw new IllegalArgumentException(String.format("refKind=%d isInterface=%s", refKind, isInterface));
        }

        private static int tableIndex(int refKind, boolean isInterface) {
            if (refKind < 0)  return refKind;
            return (refKind * 2) + (isInterface ? 1 : 0);
        }

        private static final Kind[] TABLE;

        static {
            // Pack the static table.
            int max = 0;
            for (Kind k : values())
                max = Math.max(max, tableIndex(k.refKind, true));

            TABLE = new Kind[max+1];
            for (Kind kind : values()) {
                int i = tableIndex(kind.refKind, kind.isInterface);
                if (i >= TABLE.length || TABLE[i] != null)
                    throw new AssertionError("TABLE entry for " + kind);
                TABLE[i] = kind;
            }

            // Pack in some aliases also.
            int ii = tableIndex(REF_invokeInterface, false);
            if (TABLE[ii] != null)
                throw new AssertionError("TABLE entry for (invokeInterface, false) used by " + TABLE[ii]);
            TABLE[ii] = INTERFACE_VIRTUAL;

            for (Kind kind : values()) {
                if (!kind.isInterface) {
                    // Add extra cache entry to alias the isInterface case.
                    // For example, (REF_getStatic, X) will produce STATIC_GETTER
                    // for either truth value of X.
                    int i = tableIndex(kind.refKind, true);
                    if (TABLE[i] == null) {
                        TABLE[i] = kind;
                    }
                }
            }
        }

        boolean isVirtualMethod() {
            switch (this) {
                case VIRTUAL:
                case SPECIAL:
                case INTERFACE_VIRTUAL:
                case INTERFACE_SPECIAL:
                    return true;
                default:
                    return false;
            }
        }
    }

    Kind kind();

    int refKind();

    boolean isOwnerInterface();

    ClassDesc owner();

    String methodName();

    String lookupDescriptor();
}

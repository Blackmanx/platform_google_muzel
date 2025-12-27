/*
 * Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.
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
import java.lang.invoke.MethodHandles;

/**
 * @hide
 */
final class DirectMethodHandleDescImpl implements DirectMethodHandleDesc {

    private final Kind kind;

    DirectMethodHandleDescImpl(Kind kind, ClassDesc owner, String name, MethodTypeDesc type) {
        this.kind = kind;
    }

    @Override
    public Kind kind() { return kind; }

    @Override
    public int refKind() { return kind.refKind; }

    @Override
    public boolean isOwnerInterface() { return kind.isInterface; }

    @Override
    public ClassDesc owner() { return null; }

    @Override
    public String methodName() { return null; }

    @Override
    public MethodTypeDesc invocationType() { return null; }

    @Override
    public String lookupDescriptor() { return null; }

    public MethodHandle resolveConstantDesc(MethodHandles.Lookup lookup)
            throws ReflectiveOperationException { return null; }

    @Override
    public boolean equals(Object o) { return false; }

    @Override
    public int hashCode() { return -1; }

    @Override
    public String toString() { return null; }
}

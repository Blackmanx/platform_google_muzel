/*
 * Copyright (c) 2018, 2021, Oracle and/or its affiliates. All rights reserved.
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

import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.util.List;

import static java.util.Objects.requireNonNull;

/**
 * @hide
 */
final class MethodTypeDescImpl implements MethodTypeDesc {

    MethodTypeDescImpl(ClassDesc returnType, ClassDesc[] argTypes) { }

    static MethodTypeDescImpl ofDescriptor(String descriptor) { return null; }

    @Override
    public ClassDesc returnType() { return null; }

    @Override
    public int parameterCount() { return -1; }

    @Override
    public ClassDesc parameterType(int index) { return null; }

    @Override
    public List<ClassDesc> parameterList() { return null; }

    @Override
    public ClassDesc[] parameterArray() { return null; }

    @Override
    public MethodTypeDesc changeReturnType(ClassDesc returnType) { return null; }

    @Override
    public MethodTypeDesc changeParameterType(int index, ClassDesc paramType) { return null; }

    @Override
    public MethodTypeDesc dropParameterTypes(int start, int end) { return null; }

    @Override
    public MethodTypeDesc insertParameterTypes(int pos, ClassDesc... paramTypes) { return null; }

    @Override
    public MethodType resolveConstantDesc(MethodHandles.Lookup lookup)
                                            throws ReflectiveOperationException { return null; }

    @Override
    public boolean equals(Object o) { return false; }

    @Override
    public int hashCode() { return -1; }

    @Override
    public String toString() { return null; }
}

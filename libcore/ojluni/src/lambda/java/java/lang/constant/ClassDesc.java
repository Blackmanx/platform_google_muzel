/*
 * Copyright (c) 2018, 2023, Oracle and/or its affiliates. All rights reserved.
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
import java.lang.invoke.TypeDescriptor;

/**
 * @hide
 */
public interface ClassDesc
        extends ConstantDesc,
                TypeDescriptor.OfField<ClassDesc> {

    static ClassDesc of(String name) { return null; }

    static ClassDesc of(String packageName, String className) { return null; }

    static ClassDesc ofDescriptor(String descriptor) { return null; }

    default ClassDesc arrayType() { return null; }

    default ClassDesc arrayType(int rank) { return null; }

    default ClassDesc nested(String nestedName) { return null; }

    default ClassDesc nested(String firstNestedName, String... moreNestedNames) { return null; }

    default boolean isArray() { return false; }

    default boolean isPrimitive() { return false; }

    default boolean isClassOrInterface() { return false; }

    default ClassDesc componentType() { return null; }

    default String packageName() { return null; }

    default String displayName() { return null; }

    String descriptorString();

    @Override
    Class<?> resolveConstantDesc(MethodHandles.Lookup lookup) throws ReflectiveOperationException;

    boolean equals(Object o);
}

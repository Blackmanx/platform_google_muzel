/*
 * Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.
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
package java.lang.invoke;

/**
 * @hide
 */
public final class ConstantBootstraps {
    private ConstantBootstraps() {throw new AssertionError();}

    static Object makeConstant(MethodHandle bootstrapMethod,
                               // Callee information:
                               String name, Class<?> type,
                               // Extra arguments for BSM, if any:
                               Object info,
                               // Caller information:
                               Class<?> callerClass) { return null; }

    public static Object nullConstant(MethodHandles.Lookup lookup, String name, Class<?> type) {
        return null;
    }

    public static Class<?> primitiveClass(MethodHandles.Lookup lookup, String name, Class<?> type) {
        return null;
    }

    public static <E extends Enum<E>> E enumConstant(MethodHandles.Lookup lookup, String name, Class<E> type) {
        return Enum.valueOf(type, name);
    }

    public static Object getStaticFinal(MethodHandles.Lookup lookup, String name, Class<?> type,
                                        Class<?> declaringClass) { return null; }

    public static Object getStaticFinal(MethodHandles.Lookup lookup, String name, Class<?> type) {
        return null;
    }


    public static Object invoke(MethodHandles.Lookup lookup, String name, Class<?> type,
                                MethodHandle handle, Object... args) throws Throwable {
        return null;
    }

    public static VarHandle fieldVarHandle(MethodHandles.Lookup lookup, String name, Class<VarHandle> type,
                                           Class<?> declaringClass, Class<?> fieldType) {
        return null;
    }

    public static VarHandle staticFieldVarHandle(MethodHandles.Lookup lookup, String name, Class<VarHandle> type,
                                                 Class<?> declaringClass, Class<?> fieldType) {
        return null;
    }

    public static VarHandle arrayVarHandle(MethodHandles.Lookup lookup, String name, Class<VarHandle> type,
                                           Class<?> arrayClass) {
        return null;
    }

    public static Object explicitCast(MethodHandles.Lookup lookup, String name, Class<?> dstType, Object value)
            throws ClassCastException { return null; }

    private static <T> Class<T> validateClassAccess(MethodHandles.Lookup lookup, Class<T> type) {
        return null;
    }
}

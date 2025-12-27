/*
 * Copyright (c) 2012, 2013, Oracle and/or its affiliates. All rights reserved.
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

@SuppressWarnings({"unchecked", "deprecation", "all"})
public final class SerializedLambda implements java.io.Serializable {

    public SerializedLambda(java.lang.Class<?> capturingClass,
            java.lang.String functionalInterfaceClass,
            java.lang.String functionalInterfaceMethodName,
            java.lang.String functionalInterfaceMethodSignature,
            int implMethodKind,
            java.lang.String implClass,
            java.lang.String implMethodName,
            java.lang.String implMethodSignature,
            java.lang.String instantiatedMethodType,
            java.lang.Object[] capturedArgs) { }

    public java.lang.String getCapturingClass() { return null; }

    public java.lang.String getFunctionalInterfaceClass() {
        return null;
    }

    public java.lang.String getFunctionalInterfaceMethodName() {
        return null;
    }

    public java.lang.String getFunctionalInterfaceMethodSignature() { return null; }

    public java.lang.String getImplClass() {
        return null;
    }

    public java.lang.String getImplMethodName() {
        return null;
    }

    public java.lang.String getImplMethodSignature() {
        return null;
    }

    public int getImplMethodKind() {
        return 0;
    }

    public final java.lang.String getInstantiatedMethodType() {
        return null;
    }

    public int getCapturedArgCount() {
        return 0;
    }

    public java.lang.Object getCapturedArg(int i) {
        return null;
    }

}

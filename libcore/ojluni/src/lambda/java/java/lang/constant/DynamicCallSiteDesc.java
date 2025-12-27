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

import java.lang.invoke.CallSite;
import java.lang.invoke.MethodHandles;

/**
 * @hide
 */
public class DynamicCallSiteDesc {

    private DynamicCallSiteDesc(DirectMethodHandleDesc bootstrapMethod,
                                String invocationName,
                                MethodTypeDesc invocationType,
                                ConstantDesc[] bootstrapArgs) { }

    public static DynamicCallSiteDesc of(DirectMethodHandleDesc bootstrapMethod,
                                         String invocationName,
                                         MethodTypeDesc invocationType,
                                         ConstantDesc... bootstrapArgs) { return null; }

    public static DynamicCallSiteDesc of(DirectMethodHandleDesc bootstrapMethod,
                                         String invocationName,
                                         MethodTypeDesc invocationType) { return null; }

    public static DynamicCallSiteDesc of(DirectMethodHandleDesc bootstrapMethod,
                                         MethodTypeDesc invocationType) { return null; }

    public DynamicCallSiteDesc withArgs(ConstantDesc... bootstrapArgs) { return null; }

    public DynamicCallSiteDesc withNameAndType(String invocationName,
                                               MethodTypeDesc invocationType) { return null; }

    public String invocationName() { return null; }

    public MethodTypeDesc invocationType() { return null; }

    public MethodHandleDesc bootstrapMethod() { return null; }

    public ConstantDesc[] bootstrapArgs() { return null; }

    public CallSite resolveCallSiteDesc(MethodHandles.Lookup lookup) throws Throwable {
        return null;
    }

    @Override
    public final boolean equals(Object o) { return false; }

    @Override
    public final int hashCode() { return -1; }

    @Override
    public String toString() { return null; }
}

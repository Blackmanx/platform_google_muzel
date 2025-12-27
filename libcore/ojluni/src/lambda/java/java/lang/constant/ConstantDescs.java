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

import static java.lang.constant.DirectMethodHandleDesc.*;

/**
 * @hide
 */
public final class ConstantDescs {
    // No instances
    private ConstantDescs() { }

    public static final String DEFAULT_NAME = "_";

    // Don't change the order of these declarations!

    public static final ClassDesc CD_Object = ClassDesc.of("java.lang.Object");

    public static final ClassDesc CD_String = ClassDesc.of("java.lang.String");

    public static final ClassDesc CD_Class = ClassDesc.of("java.lang.Class");

    public static final ClassDesc CD_Number = ClassDesc.of("java.lang.Number");

    public static final ClassDesc CD_Integer = ClassDesc.of("java.lang.Integer");

    public static final ClassDesc CD_Long = ClassDesc.of("java.lang.Long");

    public static final ClassDesc CD_Float = ClassDesc.of("java.lang.Float");

    public static final ClassDesc CD_Double = ClassDesc.of("java.lang.Double");

    public static final ClassDesc CD_Short = ClassDesc.of("java.lang.Short");

    public static final ClassDesc CD_Byte = ClassDesc.of("java.lang.Byte");

    public static final ClassDesc CD_Character = ClassDesc.of("java.lang.Character");

    public static final ClassDesc CD_Boolean = ClassDesc.of("java.lang.Boolean");

    public static final ClassDesc CD_Void = ClassDesc.of("java.lang.Void");

    public static final ClassDesc CD_Throwable = ClassDesc.of("java.lang.Throwable");

    public static final ClassDesc CD_Exception = ClassDesc.of("java.lang.Exception");

    public static final ClassDesc CD_Enum = ClassDesc.of("java.lang.Enum");

    public static final ClassDesc CD_VarHandle = ClassDesc.of("java.lang.invoke.VarHandle");

    public static final ClassDesc CD_MethodHandles = ClassDesc.of("java.lang.invoke.MethodHandles");

    public static final ClassDesc CD_MethodHandles_Lookup = CD_MethodHandles.nested("Lookup");

    public static final ClassDesc CD_MethodHandle = ClassDesc.of("java.lang.invoke.MethodHandle");

    public static final ClassDesc CD_MethodType = ClassDesc.of("java.lang.invoke.MethodType");

    public static final ClassDesc CD_CallSite = ClassDesc.of("java.lang.invoke.CallSite");

    public static final ClassDesc CD_Collection = ClassDesc.of("java.util.Collection");

    public static final ClassDesc CD_List = ClassDesc.of("java.util.List");

    public static final ClassDesc CD_Set = ClassDesc.of("java.util.Set");

    public static final ClassDesc CD_Map = ClassDesc.of("java.util.Map");

    public static final ClassDesc CD_ConstantDesc = ClassDesc.of("java.lang.constant.ConstantDesc");

    public static final ClassDesc CD_ClassDesc = ClassDesc.of("java.lang.constant.ClassDesc");

    public static final ClassDesc CD_EnumDesc = CD_Enum.nested("EnumDesc");

    public static final ClassDesc CD_MethodTypeDesc = ClassDesc.of("java.lang.constant.MethodTypeDesc");

    public static final ClassDesc CD_MethodHandleDesc = ClassDesc.of("java.lang.constant.MethodHandleDesc");

    public static final ClassDesc CD_DirectMethodHandleDesc = ClassDesc.of("java.lang.constant.DirectMethodHandleDesc");

    public static final ClassDesc CD_VarHandleDesc = CD_VarHandle.nested("VarHandleDesc");

    public static final ClassDesc CD_MethodHandleDesc_Kind = CD_DirectMethodHandleDesc.nested("Kind");

    public static final ClassDesc CD_DynamicConstantDesc = ClassDesc.of("java.lang.constant.DynamicConstantDesc");

    public static final ClassDesc CD_DynamicCallSiteDesc = ClassDesc.of("java.lang.constant.DynamicCallSiteDesc");

    public static final ClassDesc CD_ConstantBootstraps = ClassDesc.of("java.lang.invoke.ConstantBootstraps");

    public static final DirectMethodHandleDesc BSM_PRIMITIVE_CLASS
            = ofConstantBootstrap(CD_ConstantBootstraps, "primitiveClass",
            CD_Class);

    public static final DirectMethodHandleDesc BSM_ENUM_CONSTANT
            = ofConstantBootstrap(CD_ConstantBootstraps, "enumConstant",
            CD_Enum);

    public static final DirectMethodHandleDesc BSM_GET_STATIC_FINAL
            = ofConstantBootstrap(CD_ConstantBootstraps, "getStaticFinal",
            CD_Object, CD_Class);

    public static final DirectMethodHandleDesc BSM_NULL_CONSTANT
            = ofConstantBootstrap(CD_ConstantBootstraps, "nullConstant",
            CD_Object);

    public static final DirectMethodHandleDesc BSM_VARHANDLE_FIELD
            = ofConstantBootstrap(CD_ConstantBootstraps, "fieldVarHandle",
            CD_VarHandle, CD_Class, CD_Class);

    public static final DirectMethodHandleDesc BSM_VARHANDLE_STATIC_FIELD
            = ofConstantBootstrap(CD_ConstantBootstraps, "staticFieldVarHandle",
            CD_VarHandle, CD_Class, CD_Class);

    public static final DirectMethodHandleDesc BSM_VARHANDLE_ARRAY
            = ofConstantBootstrap(CD_ConstantBootstraps, "arrayVarHandle",
            CD_VarHandle, CD_Class);

    public static final DirectMethodHandleDesc BSM_INVOKE
            = ofConstantBootstrap(CD_ConstantBootstraps, "invoke",
            CD_Object, CD_MethodHandle, CD_Object.arrayType());

    public static final DirectMethodHandleDesc BSM_EXPLICIT_CAST
            = ofConstantBootstrap(CD_ConstantBootstraps, "explicitCast",
            CD_Object, CD_Object);

    public static final ClassDesc CD_int = ClassDesc.ofDescriptor("I");

    public static final ClassDesc CD_long = ClassDesc.ofDescriptor("J");

    public static final ClassDesc CD_float = ClassDesc.ofDescriptor("F");

    public static final ClassDesc CD_double = ClassDesc.ofDescriptor("D");

    public static final ClassDesc CD_short = ClassDesc.ofDescriptor("S");

    public static final ClassDesc CD_byte = ClassDesc.ofDescriptor("B");

    public static final ClassDesc CD_char = ClassDesc.ofDescriptor("C");

    public static final ClassDesc CD_boolean = ClassDesc.ofDescriptor("Z");

    public static final ClassDesc CD_void = ClassDesc.ofDescriptor("V");

    public static final ConstantDesc NULL
            = DynamicConstantDesc.ofNamed(ConstantDescs.BSM_NULL_CONSTANT,
                                          DEFAULT_NAME, ConstantDescs.CD_Object);

    public static final DynamicConstantDesc<Boolean> TRUE
            = DynamicConstantDesc.ofNamed(BSM_GET_STATIC_FINAL,
                                          "TRUE", CD_Boolean, CD_Boolean);

    public static final DynamicConstantDesc<Boolean> FALSE
            = DynamicConstantDesc.ofNamed(BSM_GET_STATIC_FINAL,
                                          "FALSE", CD_Boolean, CD_Boolean);

    static final DirectMethodHandleDesc MHD_METHODHANDLE_ASTYPE
            = MethodHandleDesc.ofMethod(Kind.VIRTUAL, CD_MethodHandle, "asType",
                                        MethodTypeDesc.of(CD_MethodHandle, CD_MethodType));
    public static DirectMethodHandleDesc ofCallsiteBootstrap(ClassDesc owner,
                                                             String name,
                                                             ClassDesc returnType,
                                                             ClassDesc... paramTypes) { return null; }

    public static DirectMethodHandleDesc ofConstantBootstrap(ClassDesc owner,
                                                             String name,
                                                             ClassDesc returnType,
                                                             ClassDesc... paramTypes) { return null; }
}

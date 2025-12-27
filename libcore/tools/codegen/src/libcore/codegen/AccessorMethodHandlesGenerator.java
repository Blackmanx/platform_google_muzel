/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package libcore.codegen;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.StringJoiner;

/**
 * Generates methods of {@link DirectMethodHandle.Holder} class.
 */
public class AccessorMethodHandlesGenerator {

    static String capitalize(String str) {
        if (str.isEmpty()) {
            throw new IllegalArgumentException("Can't capitalize empty string");
        }
        return Character.toUpperCase(str.charAt(0)) + str.substring(1);
    }

    enum HandleKind {
        IPUT,
        IGET,
        SPUT,
        SGET;

        public boolean isStatic() {
            return this == SGET || this == SPUT;
        }
        public boolean isPut() {
            return this == IPUT || this == SPUT;
        }
        public boolean isGet() {
            return !isPut();
        }

        /* Prefix of a method in Holder class implementing this HandleKind. */
        public String prefix() {
            if (isPut()) {
                return "put";
            } else {
                return "get";
            }
        }
    }

    enum BasicType {
        BOOLEAN,
        BYTE,
        CHAR,
        SHORT,
        INT,
        LONG,
        DOUBLE,
        FLOAT,
        REFERENCE;

        public String typeName() {
            return this == REFERENCE ? "Object" : name().toLowerCase(Locale.ROOT);
        }

        /* Returns a string corresponding to a method parameter of this type named as paramName,
         * for example "int paramName".
         */
        public String param(String paramName) {
            return typeName() + " " + paramName;
        }
    }

    static String parameters(HandleKind kind, BasicType actingUpon) {
        var params = new ArrayList<String>();
        if (!kind.isStatic()) {
            params.add(BasicType.REFERENCE.param("base"));
        }

        if (kind.isPut()) {
            params.add(actingUpon.param("value"));
        }

        // There is always MethodHandle object.
        params.add("MethodHandleImpl mh");

        return String.join(", ", params);
    }

    static String function(HandleKind kind, BasicType actingUpon, boolean isVolatile) {
        var sb = new StringBuilder();
        var modifiersAndReturnType = new StringJoiner(" ");
        modifiersAndReturnType.add("static");
        if (kind.isPut()) {
            modifiersAndReturnType.add("void");
        } else {
            modifiersAndReturnType.add(actingUpon.typeName());
        }
        sb.append(modifiersAndReturnType).append(" ");
        sb.append(kind.prefix()).append(capitalize(actingUpon.name().toLowerCase(Locale.ROOT)));
        if (isVolatile) {
            sb.append("Volatile");
        }
        sb.append("(").append(parameters(kind, actingUpon)).append(") {\n");

        if (kind.isStatic()) {
            sb.append("  ").append("Object base = staticBase(mh);\n");
            sb.append("  ").append("long offset = staticOffset(mh);\n");
        } else {
            sb.append("  ").append("checkBase(base);\n");
            sb.append("  ").append("long offset = fieldOffset(mh);\n");
        }
        var accessMode = isVolatile ? "Volatile" : "";
        sb.append("  ");
        if (kind.isGet()) {
            sb.append("return UNSAFE.")
                .append(kind.prefix())
                .append(capitalize(actingUpon.name().toLowerCase(Locale.ROOT)))
                .append(accessMode)
                .append("(base, offset);");
        } else {
            sb.append("UNSAFE.")
                .append(kind.prefix())
                .append(capitalize(actingUpon.name().toLowerCase(Locale.ROOT)))
                .append(accessMode)
                .append("(base, offset, value);");
        }

        sb.append("\n}\n");
        return sb.toString();
    }

    public static void main(String[] args) {
        for (HandleKind kind : HandleKind.values()) {
            for (BasicType name : BasicType.values()) {
                for (boolean isVolatile : List.of(false, true)) {
                    System.out.println(function(kind, name, isVolatile));
                }
            }
        }
    }
}
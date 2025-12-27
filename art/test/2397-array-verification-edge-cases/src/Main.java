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

import java.lang.reflect.InvocationTargetException;

public class Main {
    public static void main(String[] args) throws Exception {
        try {
            Class.forName("b_448576150");
            throw new Error("UNREACHABLE");
        } catch (java.lang.VerifyError expected) {
        }
        try {
            Class.forName("NullArrayGetIPutS").getDeclaredMethod("test").invoke(null);
        } catch (InvocationTargetException ite) {
            // We expect NPE, so rethrow if the cause is different.
            if (!(ite.getCause() instanceof NullPointerException)) {
                throw ite;
            }
        }
        try {
            Class.forName("NullArrayPutIntAsShort").getDeclaredMethod("test").invoke(null);
            throw new Error("UNREACHABLE");
        } catch (InvocationTargetException ite) {
            // We expect NPE, so rethrow if the cause is different.
            if (!(ite.getCause() instanceof NullPointerException)) {
                throw ite;
            }
        }
        try {
            Class.forName("UnresolvedMergedArrayPutIntAsObject");
            throw new Error("UNREACHABLE");
        } catch (VerifyError expected) {
        }
    }
}

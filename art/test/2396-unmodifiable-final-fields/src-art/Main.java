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

import static java.lang.invoke.MethodType.methodType;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.VarHandle;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.util.concurrent.atomic.AtomicIntegerFieldUpdater;
import java.util.concurrent.atomic.AtomicLongFieldUpdater;
import java.util.concurrent.atomic.AtomicReferenceFieldUpdater;

public class Main {

    private static final String DEX_FILE =
        System.getenv("DEX_LOCATION") + "/2396-unmodifiable-final-fields.jar";
    private static final String LIBRARY_SEARCH_PATH = System.getProperty("java.library.path");


    private static final Object OBJECT = new Object();
    private static final MethodHandle FIELD_SETTER;
    private static final VarHandle FIELD;
    private static final AtomicIntegerFieldUpdater INTEGER_FIELD_UPDATER;
    private static final AtomicReferenceFieldUpdater REFERENCE_FIELD_UPDATER;
    private static final AtomicLongFieldUpdater LONG_FIELD_UPDATER;

    static {
        try {
            FIELD_SETTER = MethodHandles.lookup()
                .findSetter(Main.class, "field", int.class);
            FIELD = MethodHandles.lookup()
                .findVarHandle(Main.class, "field", int.class);
            INTEGER_FIELD_UPDATER = AtomicIntegerFieldUpdater.newUpdater(Main.class, "field");
            REFERENCE_FIELD_UPDATER =
                AtomicReferenceFieldUpdater.newUpdater(Main.class, Object.class, "referenceField");
            LONG_FIELD_UPDATER = AtomicLongFieldUpdater.newUpdater(Main.class, "longField");
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    // volatile to make AtomicIntegerFieldUpdater happy.
    private volatile int field;
    private volatile long longField;
    private volatile Object referenceField;

    public record MyRecord(int x) {
        public static Object NON_FINAL = new Object();
    }

    public static void main(String[] args) throws Throwable {
        System.loadLibrary(args[0]);

        boolean specialCasedStaticFinalAreFinal = getTargetSdkVersion() > 36;
        boolean allStaticFinalAreFinal =
            getTargetSdkVersion() > 36 && (getSdkVersion() == 0 || getSdkVersion() > 36);

        testFields(specialCasedStaticFinalAreFinal, allStaticFinalAreFinal);
        runTestInASeparateClassLoader(specialCasedStaticFinalAreFinal, allStaticFinalAreFinal);
    }

    public static void testFields(boolean specialCasedStaticFinalAreFinal,
                                  boolean allStaticFinalAreFinal) throws Exception {
        // Following fields are unmodifiable:
        // * Final fields in record classes on all releases.
        // * `value` instance field of box classes on all releases.
        // * `static final` holding VarHandle, MethodHandle or Atomic*FieldUpdater instances in apps
        //   targeting Android C or later.
        // * Other `static final` fields in apps running on and targeting Android C or later.
        Field recordX = getAccessible(MyRecord.class, "x");
        test(/*shouldFail=*/ true, () -> recordX.setInt(new MyRecord(0), 10));
        Field recordNON_FINAL = getAccessible(MyRecord.class, "NON_FINAL");
        test(/*shouldFail=*/ false, () -> recordNON_FINAL.set(null, new Object()));

        Field mhTypeField = getAccessible(MethodHandle.class, "type");
        test(/*shouldFail=*/ true, () -> mhTypeField.set(FIELD_SETTER, null));
        Field vhVarTypeField = getAccessible(VarHandle.class, "varType");
        test(/*shouldFail=*/ true, () -> vhVarTypeField.set(FIELD, null));
        Field intUpdaterCclassField = getAccessible(INTEGER_FIELD_UPDATER.getClass(), "cclass");
        test(/*shouldFail=*/ true, () -> intUpdaterCclassField.set(INTEGER_FIELD_UPDATER, null));
        Field refUpdaterCclassField = getAccessible(REFERENCE_FIELD_UPDATER.getClass(), "cclass");
        test(/*shouldFail=*/ true, () -> refUpdaterCclassField.set(REFERENCE_FIELD_UPDATER, null));
        Field longUpdaterCclassField = getAccessible(LONG_FIELD_UPDATER.getClass(), "cclass");
        test(/*shouldFail=*/ true, () -> longUpdaterCclassField.set(LONG_FIELD_UPDATER, null));

        Field booleanValue = getAccessible(Boolean.class, "value");
        test(/*shouldFail=*/ true, () -> booleanValue.set(Boolean.TRUE, false));
        Field byteValue = getAccessible(Byte.class, "value");
        test(/*shouldFail=*/ true, () -> byteValue.set(Byte.valueOf((byte) 42), (byte) 10));
        Field characterField = getAccessible(Character.class, "value");
        test(/*shouldFail=*/ true, () -> characterField.set(Character.valueOf('a'), 'b'));
        Field shortField = getAccessible(Short.class, "value");
        test(/*shouldFail=*/ true, () -> shortField.set(Short.valueOf((short) 42), (short) 10));
        Field integerField = getAccessible(Integer.class, "value");
        test(/*shouldFail=*/ true, () -> integerField.set(Integer.valueOf(42), 10));
        Field floatField = getAccessible(Float.class, "value");
        test(/*shouldFail=*/ true, () -> floatField.set(Float.valueOf(42.0f), 10.0f));
        Field longField = getAccessible(Long.class, "value");
        test(/*shouldFail=*/ true, () -> longField.set(Long.valueOf(42), 10));
        Field doubleField = getAccessible(Double.class, "value");
        test(/*shouldFail=*/ true, () -> doubleField.set(Double.valueOf(42.0d), 10.0d));

        Field fieldField = getAccessible(Main.class, "field");
        test(/*shouldFail=*/ false, () -> fieldField.set(new Main(), 10));

        Field fieldSetterField = getAccessible(Main.class, "FIELD_SETTER");
        test(/*shouldFail=*/ specialCasedStaticFinalAreFinal,
             () -> fieldSetterField.set(null, null));
        Field fieldVhField = getAccessible(Main.class, "FIELD");
        test(/*shouldFail=*/ specialCasedStaticFinalAreFinal,
             () -> fieldVhField.set(null, null));
        Field integerFieldUpdaterField = getAccessible(Main.class, "INTEGER_FIELD_UPDATER");
        test(/*shouldFail=*/ specialCasedStaticFinalAreFinal,
             () -> integerFieldUpdaterField.set(null, null));
        Field longFieldUpdaterField = getAccessible(Main.class, "LONG_FIELD_UPDATER");
        test(/*shouldFail=*/ specialCasedStaticFinalAreFinal,
             () -> longFieldUpdaterField.set(null, null));
        Field refFieldUpdaterField = getAccessible(Main.class, "REFERENCE_FIELD_UPDATER");
        test(/*shouldFail=*/ specialCasedStaticFinalAreFinal,
             () -> refFieldUpdaterField.set(null, null));

        Field objectField = getAccessible(Main.class, "OBJECT");
        test(/*shouldFail=*/ allStaticFinalAreFinal,
             () -> objectField.set(null, new Object()));
    }

    private static void runTestInASeparateClassLoader(boolean specialCasedStaticFinalAreFinal,
                                                      boolean allStaticFinalAreFinal)
            throws Throwable {
        Class<?> pathClassLoader = Class.forName("dalvik.system.PathClassLoader");
        Constructor<?> constructor =
            pathClassLoader.getDeclaredConstructor(String.class, String.class, ClassLoader.class);
        ClassLoader loader = (ClassLoader) constructor.newInstance(
                DEX_FILE, LIBRARY_SEARCH_PATH, /*parent=*/ Main.class.getClassLoader().getParent());

        Class<?> mainFromDifferentClassLoader = loader.loadClass("Main");
        if (mainFromDifferentClassLoader == Main.class) {
            throw new AssertionError("Expected to obtain a different instance of Main.class");
        }

        MethodHandle runHandle = MethodHandles.lookup()
            .findStatic(mainFromDifferentClassLoader,
                        "testFields",
                        methodType(void.class, boolean.class, boolean.class));
        runHandle.invokeExact(specialCasedStaticFinalAreFinal, allStaticFinalAreFinal);
    }

    private interface ThrowingRunnable {
        void run() throws Exception;
    }

    private static void test(boolean shouldFail, ThrowingRunnable subject) {
        try {
            subject.run();
        } catch (Exception e) {
            if (!shouldFail) {
                throw new AssertionError("Did not expect subject to fail", e);
            }
            return;
        }

        if (shouldFail) {
            throw new AssertionError("Expected subject to fail, but it didn't");
        }
    }

    private static Field getAccessible(Class<?> clazz, String name) throws Exception {
        Field field = clazz.getDeclaredField(name);
        field.setAccessible(true);
        return field;
    }

    public static native int getTargetSdkVersion();
    public static native int getSdkVersion();
}

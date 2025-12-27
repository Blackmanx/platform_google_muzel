/*
 * Copyright (C) 2008 The Android Open Source Project
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

package libcore.java.lang.reflect;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import static org.junit.Assume.assumeTrue;

import dalvik.annotation.compat.VersionCodes;
import dalvik.system.VMRuntime;

import libcore.junit.util.SwitchTargetSdkVersionRule;
import libcore.junit.util.SwitchTargetSdkVersionRule.TargetSdkVersion;

import java.lang.reflect.Field;
import java.lang.reflect.Modifier;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestRule;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public final class FieldTest {
    private static final long MY_LONG = 5073258162644648461L;
    private static final Object MY_REF = new Object();

    @Rule
    public TestRule switchTargetSdkVersionRule = SwitchTargetSdkVersionRule.getInstance();

    // Reflection for static long fields was broken http://b/1120750
    @Test
    public void testLongFieldReflection() throws Exception {
        Field field = getClass().getDeclaredField("MY_LONG");
        assertEquals(5073258162644648461L, field.getLong(null));
    }

    @Test
    @TargetSdkVersion(VersionCodes.C)
    public void testSetStaticFinalField_shouldThrow_afterBaklava() throws Exception {
        // Prior to Android C it was possible to override static final fields.
        assumeTrue(VMRuntime.getSdkVersion() >= VersionCodes.C);
        Field longField = getClass().getDeclaredField("MY_LONG");
        longField.setAccessible(true);

        try {
            longField.set(null, 1001L);
            fail("Should fail to override static final field");
        } catch (IllegalAccessException expected) {}

        try {
            longField.setLong(null, 1001L);
            fail("Should fail to override static final field");
        } catch (IllegalAccessException expected) {}

        Field refField = getClass().getDeclaredField("MY_REF");
        refField.setAccessible(true);

        try {
            refField.set(null, new Object());
            fail("Should fail to override static final field");
        } catch (IllegalAccessException expected) {}
    }

    @Test
    @TargetSdkVersion(VersionCodes.C)
    public void setWriteProtectedField_shouldThrow_afterBaklava() throws Exception {
        // Prior to Android C it was possible to override static final fields.
        assumeTrue(VMRuntime.getSdkVersion() >= VersionCodes.C);

        Field systemIn = System.class.getDeclaredField("in");
        systemIn.setAccessible(true);

        try {
            systemIn.set(null, null);
            fail("Should fail to set System.in");
        } catch (IllegalAccessException expected) {}

        Field systemOut = System.class.getDeclaredField("out");
        systemOut.setAccessible(true);

        try {
            systemOut.set(null, null);
            fail("Should fail to set System.out");
        } catch (IllegalAccessException expected) {}

        Field systemErr = System.class.getDeclaredField("err");
        systemErr.setAccessible(true);

        try {
            systemErr.set(null, null);
            fail("Should fail to set System.err");
        } catch (IllegalAccessException expected) {}
    }

    @Test
    public void testEqualConstructorEqualsAndHashCode() throws Exception {
        Field f1 = FieldTestHelper.class.getField("a");
        Field f2 = FieldTestHelper.class.getField("a");
        assertEquals(f1, f2);
        assertEquals(f1.hashCode(), f2.hashCode());
    }

    @Test
    public void testHashCodeSpec() throws Exception {
        Field f1 = FieldTestHelper.class.getField("a");
        assertEquals(FieldTestHelper.class.getName().hashCode() ^ "a".hashCode(), f1.hashCode());
    }

    @Test
    public void testDifferentConstructorEqualsAndHashCode() throws Exception {
        Field f1 = FieldTestHelper.class.getField("a");
        Field f2 = FieldTestHelper.class.getField("b");
        assertFalse(f1.equals(f2));
    }

    // Tests that the "synthetic" modifier is handled correctly.
    // It's supposed to be present but not shown in toString.
    @Test
    public void testSyntheticModifier() throws NoSuchFieldException {
        Field valuesField = Thread.State.class.getDeclaredField("$VALUES");
        // Check that this test makes sense.
        assertTrue(valuesField.isSynthetic());
        assertEquals(Modifier.SYNTHETIC, valuesField.getModifiers() & Modifier.SYNTHETIC);
        assertEquals("private static final java.lang.Thread$State[] java.lang.Thread$State.$VALUES",
                valuesField.toString());
    }

    // Ensure that the "enum constant" bit is not returned in toString.
    @Test
    public void testEnumValueField() throws NoSuchFieldException {
        Field blockedField = Thread.State.class.getDeclaredField("BLOCKED");
        assertTrue(Thread.State.class.getDeclaredField("BLOCKED").isEnumConstant());
        assertEquals("public static final", Modifier.toString(blockedField.getModifiers()));
        assertEquals(
                "public static final java.lang.Thread$State java.lang.Thread$State.BLOCKED",
                blockedField.toString());
    }

    class ClassWithATransientField {
        private transient Class<String> transientField = String.class;
    }

    // Tests that the "transient" modifier is handled correctly.
    // The underlying constant value for it is the same as for the "varargs" method modifier.
    // http://b/18488857
    @Test
    public void testTransientModifier() throws NoSuchFieldException {
        Field transientField = ClassWithATransientField.class.getDeclaredField("transientField");
        // Check that this test makes sense.
        assertEquals(Modifier.TRANSIENT, transientField.getModifiers() & Modifier.TRANSIENT);
        assertEquals(
                "private transient java.lang.Class "
                        + "libcore.java.lang.reflect.FieldTest$ClassWithATransientField"
                        + ".transientField",
                transientField.toString());
    }

    @Test
    public void testToGenericString() throws NoSuchFieldException {
        Field transientField = ClassWithATransientField.class.getDeclaredField("transientField");
        // Check that this test makes sense.
        assertEquals(Modifier.TRANSIENT, transientField.getModifiers() & Modifier.TRANSIENT);
        assertEquals(
                "private transient java.lang.Class<java.lang.String> "
                        + "libcore.java.lang.reflect.FieldTest$ClassWithATransientField"
                        + ".transientField",
                transientField.toGenericString());
    }

    static class FieldTestHelper {
        public String a;
        public Object b;
    }
}

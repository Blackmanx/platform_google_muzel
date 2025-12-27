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

package libcore.java.lang;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public final class WrongThreadExceptionTest {

    @Test
    public void test_constructor() {
        WrongThreadException e = new WrongThreadException();
        assertNull(e.getMessage());
        assertNull(e.getCause());
    }

    @Test
    public void test_constructor_String() {
        String message = "error message";
        WrongThreadException e = new WrongThreadException(message);
        assertEquals(message, e.getMessage());
        assertNull(e.getCause());
    }

    @Test
    public void test_constructor_String_Throwable() {
        String message = "error message";
        Throwable cause = new Throwable("cause");
        WrongThreadException e = new WrongThreadException(message, cause);
        assertEquals(message, e.getMessage());
        assertEquals(cause, e.getCause());
    }

    @Test
    public void test_constructor_Throwable() {
        Throwable cause = new Throwable("cause");
        WrongThreadException e = new WrongThreadException(cause);
        assertEquals("java.lang.Throwable: cause", e.getMessage());
        assertEquals(cause, e.getCause());
    }

    @Test
    public void test_constructor_Throwable_null() {
        WrongThreadException e = new WrongThreadException((Throwable) null);
        assertNull(e.getMessage());
        assertNull(e.getCause());
    }
}

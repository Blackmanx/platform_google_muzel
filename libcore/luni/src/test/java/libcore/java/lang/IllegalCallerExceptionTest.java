/*
 * Copyright (C) 2021 The Android Open Source Project
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

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

@RunWith(JUnit4.class)
public class IllegalCallerExceptionTest {

    @Test
    public void constructor_noArg() {
        IllegalCallerException exception = new IllegalCallerException();

        assertNull(exception.getMessage());
        assertNull(exception.getCause());
    }

    @Test
    public void constructor_String() {
        String message = "message";

        IllegalCallerException exception = new IllegalCallerException(message);

        assertEquals(message, exception.getMessage());
        assertNull(exception.getCause());
    }

    @Test
    public void constructor_String_Throwable() {
        String message = "message";
        Exception cause = new Exception();

        IllegalCallerException exception = new IllegalCallerException(message, cause);

        assertEquals(message, exception.getMessage());
        assertEquals(cause, exception.getCause());
    }

    @Test
    public void constructor_Throwable() {
        Exception cause = new Exception();

        IllegalCallerException exception = new IllegalCallerException(cause);

        assertEquals(cause, exception.getCause());
    }

    @Test
    public void constructor_String_null() {
        // Scenario: Call the constructor with a null message.
        // The Javadoc specifies that the message can be null.
        IllegalCallerException exception = new IllegalCallerException((String) null);

        // Verification: The message and cause should be null.
        assertNull(exception.getMessage());
        assertNull(exception.getCause());
    }

    @Test
    public void constructor_String_Throwable_nullMessage() {
        // Scenario: Call the constructor with a null message and a valid cause.
        // The Javadoc specifies that the message can be null.
        Exception cause = new Exception();
        IllegalCallerException exception = new IllegalCallerException(null, cause);

        // Verification: The message should be null and the cause should be correctly set.
        assertNull(exception.getMessage());
        assertEquals(cause, exception.getCause());
    }

    @Test
    public void constructor_String_Throwable_nullCause() {
        // Scenario: Call the constructor with a valid message and a null cause.
        // The Javadoc specifies that the cause can be null.
        String message = "message";
        IllegalCallerException exception = new IllegalCallerException(message, null);

        // Verification: The message should be set and the cause should be null.
        assertEquals(message, exception.getMessage());
        assertNull(exception.getCause());
    }

    @Test
    public void constructor_String_Throwable_nullMessageAndCause() {
        // Scenario: Call the constructor with both a null message and a null cause.
        IllegalCallerException exception = new IllegalCallerException(null, null);

        // Verification: Both the message and the cause should be null.
        assertNull(exception.getMessage());
        assertNull(exception.getCause());
    }

    @Test
    public void constructor_Throwable_null() {
        // Scenario: Call the constructor with a null cause.
        // The Javadoc specifies that the cause can be null.
        IllegalCallerException exception = new IllegalCallerException((Throwable) null);

        // Verification: The cause should be null. The message will also be null,
        // consistent with the behavior of new RuntimeException((Throwable) null).
        assertNull(exception.getCause());
        assertNull(exception.getMessage());
    }

}

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

package libcore.java.lang.invoke;

import static org.junit.Assert.assertEquals;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.lang.invoke.StringConcatException;

@RunWith(JUnit4.class)
public class StringConcatExceptionTest {

    @Test
    public void constructor_LString() {
        String msg = "message";
        StringConcatException exception = new StringConcatException(msg);

        assertEquals(msg, exception.getMessage());
    }

    @Test
    public void constructor_LStringLThrowable() {
        String msg = "message";
        Throwable cause = new Exception();
        StringConcatException exception = new StringConcatException(msg, cause);

        assertEquals(msg, exception.getMessage());
        assertEquals(cause, exception.getCause());
    }

}

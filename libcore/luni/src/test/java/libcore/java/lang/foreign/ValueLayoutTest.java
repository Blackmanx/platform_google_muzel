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

package libcore.java.lang.foreign;

import static org.junit.Assert.assertEquals;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.lang.foreign.ValueLayout;
import java.nio.ByteOrder;
import java.util.Optional;

@RunWith(JUnit4.class)
public class ValueLayoutTest {

    @Test
    public void valueLayout_int_order() {
        // Default byte order should be native
        assertEquals(ByteOrder.nativeOrder(), ValueLayout.JAVA_INT.order());
    }

    @Test
    public void valueLayout_int_byteSize() {
        assertEquals(4, ValueLayout.JAVA_INT.byteSize());
    }

    @Test
    public void valueLayout_int_byteAlignment() {
        assertEquals(4, ValueLayout.JAVA_INT.byteAlignment());
    }

    @Test
    public void valueLayout_int_withByteAlignemnt() {
        ValueLayout valueLayout = ValueLayout.JAVA_INT.withByteAlignment(2);
        assertEquals(2, valueLayout.byteAlignment());
    }

    @Test
    public void valueLayout_int_withOrder() {
        ValueLayout valueLayout = ValueLayout.JAVA_INT.withOrder(ByteOrder.LITTLE_ENDIAN);
        assertEquals(ByteOrder.LITTLE_ENDIAN, valueLayout.order());
    }

    @Test
    public void valueLayout_int_withoutName() {
        ValueLayout valueLayout = ValueLayout.JAVA_INT.withoutName();
        assertEquals(Optional.empty(), valueLayout.name());
    }

    @Test
    public void valueLayout_int_withName() {
        ValueLayout valueLayout = ValueLayout.JAVA_INT.withName("test");
        assertEquals(Optional.of("test"), valueLayout.name());
    }

    @Test
    public void valueLayout_correct_carrier_for_int() {
        assertEquals(ValueLayout.JAVA_INT.carrier(), int.class);
        assertEquals(ValueLayout.JAVA_INT_UNALIGNED.carrier(), int.class);
    }

}

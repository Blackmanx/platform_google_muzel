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

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.ValueLayout;
import java.util.List;
import java.util.stream.Stream;

import libcore.test.annotation.NonCts;
import libcore.test.reasons.NonCtsReasons;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assume.assumeTrue;
import static java.lang.foreign.ValueLayout.JAVA_INT;

@RunWith(JUnit4.class)
public class MemorySegmentTest {
    private static final long BYTE_SIZE = 100;
    private static final long BYTE_ALIGNMENT = 4;
    private final Arena globalArena = Arena.global();
    private final MemorySegment segment = globalArena.allocate(BYTE_SIZE, BYTE_ALIGNMENT);
    private final MemorySegment segment2 = globalArena.allocate(BYTE_SIZE, BYTE_ALIGNMENT);

    @Test
    public void nullSegment_equals_ofAddressZero() {
        MemorySegment nullSegment = MemorySegment.NULL;

        assertNotNull(nullSegment);
        assertEquals(0, nullSegment.byteSize());
        assertTrue(nullSegment.scope().isAlive());
        assertEquals(nullSegment.address(), MemorySegment.ofAddress(0L).address());
        assertEquals(nullSegment, MemorySegment.ofAddress(0L));
    }

    @Test
    public void zeroLength_native_segment_byteSize_equals_zero() {
        MemorySegment nativeSegment = MemorySegment.ofAddress(0L);
        assertEquals(nativeSegment.byteSize(), 0);
    }

    @Test
    public void memorySegments_allocated_areUnique() {
        assertNotEquals(segment, segment2);
        assertNotEquals(segment.address(), segment2.address());
    }

    // Access Tests
    @Test
    public void set_get_int() {
        int value = 42;
        long offset = 0;
        segment.set(JAVA_INT, offset, value);
        int result = segment.get(JAVA_INT, offset);
        assertEquals(value, result);
    }

    @Test
    public void set_get_int_withOffset() {
        int value = 123;
        long offset = 16;
        segment.set(JAVA_INT, offset, value);
        int result = segment.get(JAVA_INT, offset);
        assertEquals(value, result);
    }

    @Test
    public void set_get_multipleInts() {
        int value1 = 99;
        long offset1 = 0;
        segment.set(JAVA_INT, offset1, value1);

        int value2 = -55;
        long offset2 = 4;
        segment.set(JAVA_INT, offset2, value2);

        assertEquals(value1, segment.get(JAVA_INT, offset1));
        assertEquals(value2, segment.get(JAVA_INT, offset2));
    }

    @Test
    public void set_get_int_minMaxValues() {
        long offset1 = 8;
        segment.set(JAVA_INT, offset1, Integer.MAX_VALUE);
        assertEquals(Integer.MAX_VALUE, segment.get(JAVA_INT, offset1));

        long offset2 = 12;
        segment.set(JAVA_INT, offset2, Integer.MIN_VALUE);
        assertEquals(Integer.MIN_VALUE, segment.get(JAVA_INT, offset2));
    }

    @Test
    public void get_throwsIndexOutOfBounds_forNegativeOffset() {
        assertThrows(IndexOutOfBoundsException.class,
                () -> segment.get(JAVA_INT, -1));
    }

    @Test
    public void set_throwsIndexOutOfBounds_forNegativeOffset() {
        assertThrows(IndexOutOfBoundsException.class,
                () -> segment.set(JAVA_INT, -1, 42));
    }

    @Test
    public void get_throwsIndexOutOfBounds_forOffsetEqualToSize() {
        assertThrows(IndexOutOfBoundsException.class,
                () -> segment.get(JAVA_INT, BYTE_SIZE));
    }

    @Test
    public void set_throwsIndexOutOfBounds_forOffsetEqualToSize() {
        assertThrows(IndexOutOfBoundsException.class,
                () -> segment.set(JAVA_INT, BYTE_SIZE, 42));
    }

    @Test
    public void get_throwsIndexOutOfBounds_forOffsetExceedingSize() {
        assertThrows(IndexOutOfBoundsException.class,
                () -> segment.get(JAVA_INT, BYTE_SIZE + 4));
    }

    @Test
    public void set_throwsIndexOutOfBounds_forOffsetExceedingSize() {
        assertThrows(IndexOutOfBoundsException.class,
                () -> segment.set(JAVA_INT, BYTE_SIZE + 4, 42));
    }

    @Test
    public void set_throwsIndexOutOfBounds_whenWriteExceedsBounds() {
        assertThrows(IndexOutOfBoundsException.class,
                () -> segment.set(JAVA_INT, BYTE_SIZE - 2, 42));
    }

    @Test
    public void setGetInt_atBounds() {
        long offset = BYTE_SIZE - Integer.BYTES;
        int value = 255;
        segment.set(JAVA_INT, offset, value);
        assertEquals(value, segment.get(JAVA_INT, offset));
    }

    @Test
    public void set_throwsIllegalArgument_forMisalignedOffset() {
        assertThrows(IllegalArgumentException.class,
                () -> segment.set(JAVA_INT, 1, 42));
    }

    @Test
    public void get_throwsIllegalArgument_forMisalignedOffset() {
        assertThrows(IllegalArgumentException.class,
                () -> segment.get(JAVA_INT, 1));
    }

    @Test
    public void set_throwsIllegalArgument_forReadOnlySegment() {
        // TODO b/447128212 - UnsupportedOperationException in JDK21
        assumeTrue("Dalvik".equals(System.getProperty("java.vm.name")));

        MemorySegment readOnlySegment = segment.asReadOnly();

        assertThrows(IllegalArgumentException.class,
                () -> readOnlySegment.set(JAVA_INT, 0, 42));
    }

    @Test
    public void get_forReadOnlySegment() {
        int value = 42;
        long offset = 0;
        segment.set(JAVA_INT, offset, value);

        MemorySegment readOnlySegment = segment.asReadOnly();

        // Getting from a read-only segment should work
        assertEquals(value, readOnlySegment.get(JAVA_INT, offset));
    }

    @Test
    public void get_set_withByteAlignment() {
        List<Integer> alignments = Stream.iterate(4, x -> x * 2).limit(10).toList();
        int value = 42;
        int byteSize = Integer.BYTES;;

        for (int alignment : alignments) {
            ValueLayout.OfInt alignedLayout = JAVA_INT.withByteAlignment(alignment);
            MemorySegment alignedSegment = globalArena.allocate(byteSize, alignment);

            // Set and get a value to ensure it passes
            alignedSegment.set(alignedLayout, 0, value);

            assertEquals(value, alignedSegment.get(alignedLayout, 0));
        }
    }

    @Test
    public void set_throwsIllegalArgument_forMisalignedAllocation_withByteAlignment() {
        // TODO b/447128212 - maxByteAlignment not supported in JDK 21
        assumeTrue("Dalvik".equals(System.getProperty("java.vm.name")));

        ValueLayout.OfInt layout = JAVA_INT.withByteAlignment(segment.maxByteAlignment() * 2);

        assertThrows(IllegalArgumentException.class, () -> segment.set(layout, 0, 42));
    }

    @Test
    public void set_get_forUnalignedAccess() {
        JAVA_INT.withByteAlignment(1).varHandle().set(segment, 0, 42);
        int result = (int) JAVA_INT.withByteAlignment(1).varHandle().get(segment, 0);

        assertEquals(42, result);
    }

    // End of Access Tests

    @Test
    public void native_segment_is_zeroed() {
        for (int i = 0; i < BYTE_SIZE - Integer.BYTES; i += 4) {
            assertEquals(0, segment.get(JAVA_INT, i));
        }
    }

    @Test
    public void native_allocation_too_big() {
        assertThrows(OutOfMemoryError.class,
                () -> globalArena.allocate(Long.MAX_VALUE, 1)
        );
    }

    @Test
    public void native_segment_isNative(){
        assertTrue(segment.isNative());
    }

    @Test
    public void null_segment_maxByteAlignment_correct() {
        // TODO b/447128212 - maxByteAlignment not supported in JDK 21
        assumeTrue("Dalvik".equals(System.getProperty("java.vm.name")));

        assertEquals(1L << 62, MemorySegment.NULL.maxByteAlignment());
    }

    @Test
    public void native_segment_maxByteAlignment_correct() {
        // TODO b/447128212 - maxByteAlignment not supported in JDK 21
        assumeTrue("Dalvik".equals(System.getProperty("java.vm.name")));
        long alignment = 16;
        // byte size is just an arbitrary power of two
        MemorySegment alignedSegment = globalArena.allocate(16384, alignment);

        // The address should not be 0
        assertTrue(alignedSegment.address() != 0);
        // maxByteAlignment should be the lowest set bit of the address
        assertEquals(Long.lowestOneBit(
                alignedSegment.address()),
                alignedSegment.maxByteAlignment());
        // The alignment of the segment should be at least what was requested.
        assertTrue(alignedSegment.maxByteAlignment() >= alignment);
    }
}

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

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.util.List;
import java.util.stream.Stream;

import libcore.test.annotation.NonCts;
import libcore.test.reasons.NonCtsReasons;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;

@RunWith(JUnit4.class)
@NonCts(bug = 443192271, reason = NonCtsReasons.INTERNAL_APIS)
public class GlobalArenaTest {
    private static final long BYTE_SIZE = 100;
    private static final long BYTE_ALIGNMENT = 1;
    private final Arena globalArena = Arena.global();
    private final MemorySegment segment = globalArena.allocate(BYTE_SIZE, BYTE_ALIGNMENT);

    @Test
    public void arena_segment_notNull() {
        assertNotNull(segment);
    }

    @Test
    public void arena_scope_isAlive() {
        assertTrue(segment.scope().isAlive());
        assertTrue(globalArena.scope().isAlive());
    }

    @Test
    public void arena_close_throws_UnsupportedOperationException() {
        assertThrows(UnsupportedOperationException.class, globalArena::close);
    }

    @Test
    public void arena_scope_matches_segment_scope(){
        assertSame(globalArena.scope(), segment.scope());
    }

    @Test
    public void arena_byteSizeEquals() {
        assertEquals(BYTE_SIZE, segment.byteSize());
    }

    @Test
    public void heapBase_IsEmpty() {
        assertTrue(segment.heapBase().isEmpty());
    }

    @Test
    public void allocate_invalidArguments_throws() {
        assertThrows(IllegalArgumentException.class,
                () -> globalArena.allocate(-1, BYTE_ALIGNMENT));
        assertThrows(IllegalArgumentException.class,
                () -> globalArena.allocate(100, 0));
        assertThrows(IllegalArgumentException.class,
                () -> globalArena.allocate(100, -1));
        assertThrows(IllegalArgumentException.class,
                () -> globalArena.allocate(100, 7));
    }

    @Test
    public void allocate_address_isAligned() {
        List<Integer> alignments = Stream.iterate(1, x -> x * 2).limit(10).toList();

        for (int alignment : alignments) {
            long address = globalArena.allocate(BYTE_SIZE, alignment).address();
            String errorMessage = String.format(
                    "Address 0x%x is not a multiple of alignment %d",
                    address,
                    alignment);
            assertTrue(errorMessage, (address % alignment) == 0);
        }
    }

}
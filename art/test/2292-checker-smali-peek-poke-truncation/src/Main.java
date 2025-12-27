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

import java.lang.reflect.Field;

import sun.misc.Unsafe;

public class Main {
    public static void main(String[] args) throws Exception {
        $noinline$testPokeByte();
        $noinline$testPokeShort(false);
        $noinline$testPokeShort(true);
    }

    public static void $noinline$testPokeByte() {
        Unsafe unsafe = getUnsafe();
        long addr = unsafe.allocateMemory(1);
        try {
            Class.forName("B446169228")
                    .getDeclaredMethod("pokeByte12345678", long.class)
                    .invoke(null, addr);
            if (unsafe.getByte(addr) != 0x78) {
                throw new RuntimeException("Expected 0x78, got " + unsafe.getByte(addr));
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        } finally {
            unsafe.freeMemory(addr);
        }
    }

    public static void $noinline$testPokeShort(boolean swap) {
        Unsafe unsafe = getUnsafe();
        long addr = unsafe.allocateMemory(2);
        try {
            Class.forName("B446169228")
                    .getDeclaredMethod("pokeShort12345678", long.class, boolean.class)
                    .invoke(null, new Object[] {addr, swap});
            short expected_value = swap ? (short) 0x7856 : (short) 0x5678;
            if (unsafe.getShort(addr) != expected_value) {
                throw new RuntimeException("Expected 0x" + Integer.toHexString(expected_value)
                        + ", got 0x" + Integer.toHexString(unsafe.getShort(addr)));
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        } finally {
            unsafe.freeMemory(addr);
        }
    }

    private static Unsafe getUnsafe() {
        try {
            Class<?> unsafeClass = Unsafe.class;
            Field f = unsafeClass.getDeclaredField("theUnsafe");
            f.setAccessible(true);
            return (Unsafe) f.get(null);
        } catch (Exception e) {
            throw new Error("Cannot get Unsafe instance");
        }
    }
}

/*
 * Copyright (C) 2024 The Android Open Source Project
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

package libcore.android34.java.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Optional;

@RunWith(JUnit4.class)
public class ListTest {

    public static class MyList implements List<String> {

        private final ArrayList<String> l = new ArrayList<>();
        @Override
        public int size() {
            return l.size();
        }

        @Override
        public boolean isEmpty() {
            return l.isEmpty();
        }

        @Override
        public boolean contains(Object o) {
            return l.contains(o);
        }

        @Override
        public Iterator<String> iterator() {
            return l.iterator();
        }

        @Override
        public Object[] toArray() {
            return l.toArray();
        }

        @Override
        public <T> T[] toArray(T[] a) {
            return l.toArray(a);
        }

        @Override
        public boolean add(String s) {
            return l.add(s);
        }

        @Override
        public boolean remove(Object o) {
            return l.remove(o);
        }

        @Override
        public boolean containsAll(Collection<?> c) {
            return l.containsAll(c);
        }

        @Override
        public boolean addAll(Collection<? extends String> c) {
            return l.addAll(c);
        }

        @Override
        public boolean addAll(int index, Collection<? extends String> c) {
            return l.addAll(index, c);
        }

        @Override
        public boolean removeAll(Collection<?> c) {
            return l.removeAll(c);
        }

        @Override
        public boolean retainAll(Collection<?> c) {
            return l.retainAll(c);
        }

        @Override
        public void clear() {
            l.clear();
        }

        @Override
        public String get(int index) {
            return l.get(index);
        }

        @Override
        public String set(int index, String s) {
            return l.set(index, s);
        }

        @Override
        public void add(int index, String s) {
            l.add(index, s);
        }

        @Override
        public String remove(int index) {
            return l.remove(index);
        }

        @Override
        public int indexOf(Object o) {
            return l.indexOf(o);
        }

        @Override
        public int lastIndexOf(Object o) {
            return l.lastIndexOf(o);
        }

        @Override
        public ListIterator<String> listIterator() {
            return l.listIterator();
        }

        @Override
        public ListIterator<String> listIterator(int index) {
            return l.listIterator(index);
        }

        @Override
        public List<String> subList(int fromIndex, int toIndex) {
            return l.subList(fromIndex, toIndex);
        }

        /* New methods in OpenJDK 21 begins */

        /** vs void addFirst(E). A build should fail if compiling the latest SDK. */
        public MyList addFirst(String s) {
            l.add(0, s);
            return this;
        }
        /** vs void addLast(E). A build should fail if compiling the latest SDK. */
        public MyList addLast(String s) {
            l.add(s);
            return this;
        }
        /** vs E getFirst(). A build should fail if compiling the latest SDK. */
        public Optional<String> getFirst() {
            return l.isEmpty() ? Optional.empty() : Optional.of(l.get(0));
        }
        /** vs E getLast(). A build should fail if compiling the latest SDK. */
        public Optional<String> getLast() {
            return l.isEmpty() ? Optional.empty() : Optional.of(l.get(size() - 1));
        }

        /** vs String removeFirst(E). A build should fail if compiling the latest SDK. */
        public boolean removeFirst() {
            if (l.isEmpty()) {
                return false;
            } else {
                l.remove(0);
                return true;
            }
        }
        /** vs String removeLast(E). A build should fail if compiling the latest SDK. */
        public boolean removeLast() {
            if (l.isEmpty()) {
                return false;
            } else {
                l.remove(size() - 1);
                return true;
            }
        }
    }

    private MyList myList;

    @Before
    public void setUp() {
        myList = new MyList();
    }

    @Test
    public void testAddFirst() {
        myList.addFirst("c");
        myList.addFirst("b");
        assertEquals(myList, myList.addFirst("a"));
        assertEquals("a", myList.getFirst().get());
        assertEquals("c", myList.getLast().get());
        assertTrue(myList.removeFirst());
        assertEquals("b", myList.getFirst().get());
        assertTrue(myList.removeFirst());
        assertTrue(myList.removeFirst());
        assertFalse(myList.removeFirst()); // False because the list is empty.
    }

    @Test
    public void testAddLast() {
        myList.addLast("a");
        myList.addLast("b");
        assertEquals(myList, myList.addLast("c"));
        assertEquals("a", myList.getFirst().get());
        assertEquals("c", myList.getLast().get());
        assertTrue(myList.removeLast());
        assertEquals("b", myList.getLast().get());
        assertTrue(myList.removeLast());
        assertTrue(myList.removeLast());
        assertFalse(myList.removeFirst()); // False because the list is empty.
    }

    @Test
    public void testPackageProtectedOverriddenMethods() throws ReflectiveOperationException {
        PackageProtectedList p = new PackageProtectedList();
        p.addFirst("a");
        // addFirst() method adds twice.
        assertEquals(2, p.size());
        assertEquals("a", p.get(0));
        assertEquals("a", p.get(1));
        assertSame(p, p.getFirst());

        List<Object> list = new PackageProtectedList();
        // Uses reflection API to simulate invoke-interface addFirst from the ART / frameworks.
        // However, I don't see any usage of List.addFirst in ART, and we can't directly call the
        // method here because the subclass can only be compiled when compiling SDK 34 or below.
        Method addFirst = List.class.getMethod("addFirst", Object.class);
        addFirst.invoke(list, "a");
        // addFirst() method adds twice.
        assertEquals(2, p.size());
    }

    @Test
    public void testPrivateMethods() throws ReflectiveOperationException {
        ListWithPrivateMethods p = new ListWithPrivateMethods();
        p.externalAddFirst("a");
        // addFirst() method adds twice.
        assertEquals(2, p.size());
        assertEquals("a", p.get(0));
        assertEquals("a", p.get(1));
        assertSame(p, p.externalGetFirst());

        List<Object> list = new ListWithPrivateMethods();
        // Uses reflection API to simulate invoke-interface addFirst from the ART / frameworks.
        // However, I don't see any usage of List.addFirst in ART, and we can't directly call the
        // method here because the subclass can only be compiled when compiling SDK 34 or below.
        Method addFirst = List.class.getMethod("addFirst", Object.class);
        addFirst.invoke(list, "a");
        // addFirst() method adds twice.
        assertEquals(2, p.size());
    }

    public interface ClashList<E> {
        default void addFirst(E e) {}
    }

    /** This class doesn't override addFirst(E) */
    public static class ClashClass implements List<Object>, ClashList<Object> {
        @Override
        public int size() {
            return 0;
        }

        @Override
        public boolean isEmpty() {
            return false;
        }

        @Override
        public boolean contains(Object o) {
            return false;
        }

        @Override
        public Iterator<Object> iterator() {
            return null;
        }

        @Override
        public Object[] toArray() {
            return new Object[0];
        }

        @Override
        public <T> T[] toArray(T[] a) {
            return null;
        }

        @Override
        public boolean add(Object o) {
            return false;
        }

        @Override
        public boolean remove(Object o) {
            return false;
        }

        @Override
        public boolean containsAll(Collection<?> c) {
            return false;
        }

        @Override
        public boolean addAll(Collection<?> c) {
            return false;
        }

        @Override
        public boolean addAll(int index, Collection<?> c) {
            return false;
        }

        @Override
        public boolean removeAll(Collection<?> c) {
            return false;
        }

        @Override
        public boolean retainAll(Collection<?> c) {
            return false;
        }

        @Override
        public void clear() {

        }

        @Override
        public Object get(int index) {
            return null;
        }

        @Override
        public Object set(int index, Object element) {
            return null;
        }

        @Override
        public void add(int index, Object element) {

        }

        @Override
        public Object remove(int index) {
            return null;
        }

        @Override
        public int indexOf(Object o) {
            return 0;
        }

        @Override
        public int lastIndexOf(Object o) {
            return 0;
        }

        @Override
        public ListIterator<Object> listIterator() {
            return null;
        }

        @Override
        public ListIterator<Object> listIterator(int index) {
            return null;
        }

        @Override
        public List<Object> subList(int fromIndex, int toIndex) {
            return null;
        }
    }

    /**
     * Test for 13.5.7. Interface Method Declarations. It's a known risk of adding
     * a new default method.
     */
    @Test(expected=IncompatibleClassChangeError.class)
    public void testDefaultMethodClash() {
        ClashClass l = new ClashClass();
        l.addFirst("a");
    }
}

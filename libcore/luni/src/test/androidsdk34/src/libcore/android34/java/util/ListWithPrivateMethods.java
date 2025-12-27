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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

class ListWithPrivateMethods implements List<Object> {
    private final ArrayList<Object> l = new ArrayList<>();
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
    public Iterator<Object> iterator() {
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
    public boolean add(Object s) {
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
    public boolean addAll(Collection<? extends Object> c) {
        return l.addAll(c);
    }

    @Override
    public boolean addAll(int index, Collection<? extends Object> c) {
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
    public Object get(int index) {
        return l.get(index);
    }

    @Override
    public Object set(int index, Object s) {
        return l.set(index, s);
    }

    @Override
    public void add(int index, Object s) {
        l.add(index, s);
    }

    @Override
    public Object remove(int index) {
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
    public ListIterator<Object> listIterator() {
        return l.listIterator();
    }

    @Override
    public ListIterator<Object> listIterator(int index) {
        return l.listIterator(index);
    }

    @Override
    public List<Object> subList(int fromIndex, int toIndex) {
        return l.subList(fromIndex, toIndex);
    }

    /* New methods in OpenJDK 21 begins */

    /** vs void addFirst(E). It adds twice. */
    private void addFirst(Object s) {
        l.add(0, s);
        l.add(0, s);
    }

    public void externalAddFirst(Object s) {
        addFirst(s);
    }

    /** vs E getFirst(). It returns list, not an element. */
    private Object getFirst() {
        return this;
    }

    public Object externalGetFirst() {
        return getFirst();
    }
}

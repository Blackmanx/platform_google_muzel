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

/**
 * Coordinates execution of two threads racing to load the same class, resulting
 * in one of the threads observing a temporary class in CreateArrayClass
 */
public class RaceCoordinator {
    /**
     * Identifier of a racing thread that successfully loads Component.class and its base class,
     * Base.class. It inserts temporary and then final Component.class object into the class table
     * of the defining class loader.
     */
    private static final int THREAD_LOADING_COMPONENT = 0;

    /**
     * Identifier of a racing thread that tries to create an array class (Component[].class) but
     * fails to load the component class. In the test the failure results from a bad behaviour of
     * a custom ClassLoader, but similar failures can be observed when JIT attempts to load classes
     * in a presence of a ClassLoader requiring Java method calls.
     */
    private static final int THREAD_CREATING_ARRAY = 1;

    private static final ThreadLocal<Integer> sThreadRole = new ThreadLocal<>();

    /**
     * Initial state:
     * - THREAD_LOADING_COMPONENT is suspended, has made no progress
     * - THREAD_CREATING_ARRAY is running, but has not attempted to load component of the array
     * class yet
     */
    private final static int BEGIN = 0;

    /**
     * State reached after THREAD_CREATING_ARRAY calls findClass method of the custom ClassLoader
     * to load the class of the component type (Component.class).
     * - THREAD_LOADING_COMPONENT is resumed, but has not attempted to load the base class of the
     *   Component class yet
     * - THREAD_CREATING_ARRAY is suspended
     */
    private final static int ARRAY_COMPONENT_REQUESTED = 1;

    /**
     * State reached after THREAD_LOADING_COMPONENT calls findClass method of the custom ClassLoader
     * to load the base class of the component type (Base.class). At this point the class table of
     * the defining ClassLoader of Component.class contains a temporary Component.class object.
     * - THREAD_LOADING_COMPONENT is suspended
     * - THREAD_CREATING_ARRAY is resumed. The attempt to load Component class (the previous
     *   suspension point) fails because the custom ClassLoader returns null
     */
    private final static int TEMPORARY_CLASS_IN_CLASS_TABLE = 2;

    /**
     * State reached when THREAD_LOADING_COMPONENT fails to create the array class despite
     * temporary class being available in the class table.
     * - THREAD_LOADING_COMPONENT is resumed and allowed to complete loading of the Component.class
     *   and its base class
     * - THREAD_CREATING_ARRAY has completed its work
     */
    private static final int ARRAY_CLASS_CREATION_FAILED = 3;

    private int mState = BEGIN;

    private static String COMPONENT_CLASS_NAME = "Component";
    private static String BASE_CLASS_NAME = "Base";

    private synchronized void waitForState(int state) {
        try {
            while (mState != state) {
                wait();
            }
        } catch (InterruptedException ie) {
            throw new RuntimeException(ie);
        }
    }

    private synchronized void stateTransition(int from, int to) {
        assertStateIs(from);
        mState = to;
        notifyAll();
    }

    private synchronized void assertStateIs(int state) {
        if (mState != state) {
            throw new RuntimeException();
        }
    }

    public Class<?> onSharedLibraryFindClass(String name) throws ClassNotFoundException {
        Integer role = sThreadRole.get();
        if (role == null) {
            throw new ClassNotFoundException();
        }

        return onSharedLibraryFindClass(name, role);
    }

    public Class<?> onSharedLibraryFindClass(String name, int role) throws ClassNotFoundException {
        switch (role) {
            case THREAD_CREATING_ARRAY:
                if (COMPONENT_CLASS_NAME.equals(name)) {
                    stateTransition(BEGIN, ARRAY_COMPONENT_REQUESTED);
                    waitForState(TEMPORARY_CLASS_IN_CLASS_TABLE);
                    // Returning null breaks ClassLoader contract and fails FindClass call looking
                    // up the component type of the array
                    return null;
                }

                break;

            case THREAD_LOADING_COMPONENT:
                if (COMPONENT_CLASS_NAME.equals(name)) {
                    assertStateIs(ARRAY_COMPONENT_REQUESTED);
                    throw new ClassNotFoundException();
                } else if (BASE_CLASS_NAME.equals(name)) {
                    stateTransition(ARRAY_COMPONENT_REQUESTED, TEMPORARY_CLASS_IN_CLASS_TABLE);
                    waitForState(ARRAY_CLASS_CREATION_FAILED);
                    throw new ClassNotFoundException();
                }

                break;

            default:
                break;
        }

        throw new IllegalArgumentException();
    }

    public void loadComponentClass(ClassLoader cl) {
        sThreadRole.set(THREAD_LOADING_COMPONENT);
        try {
            waitForState(ARRAY_COMPONENT_REQUESTED);
            Class.forName("Component", false, cl);
        } catch (ClassNotFoundException cnfe) {
            throw new RuntimeException(cnfe);
        } finally {
            sThreadRole.remove();
        }
    }

    public void createArrayClass(ClassLoader cl) {
        sThreadRole.set(THREAD_CREATING_ARRAY);
        try {
            Class.forName("[LComponent;", false, cl);
        } catch (ClassNotFoundException cnfe) {
            stateTransition(TEMPORARY_CLASS_IN_CLASS_TABLE, ARRAY_CLASS_CREATION_FAILED);
        } finally {
            sThreadRole.remove();
        }
    }
}

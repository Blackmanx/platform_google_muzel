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

import dalvik.system.PathClassLoader;

import java.io.File;

public class Main {
    private static final String DEX_LOCATION = System.getenv("DEX_LOCATION");
    private static final String DEX_FILE =
            new File(DEX_LOCATION, "2287-no-temp-component-class-ex.jar").getAbsolutePath();

    public static void main(String[] args) throws Exception {
        final RaceCoordinator coordinator = new RaceCoordinator();
        final ClassLoader cl = createClassLoader(coordinator);

        final Thread threadLoadingComponentClass = new Thread(new Runnable() {
            @Override
            public void run() {
                coordinator.loadComponentClass(cl);
            }
        });
        threadLoadingComponentClass.start();

        final Thread threadCreatingArrayClass = new Thread(new Runnable() {
            @Override
            public void run() {
                coordinator.createArrayClass(cl);
            }
        });
        threadCreatingArrayClass.start();
        threadCreatingArrayClass.join();
        threadLoadingComponentClass.join();

        Class<?> component = Class.forName("Component", false, cl);
        Class<?> array = Class.forName("[LComponent;", false, cl);
        if (component != array.getComponentType()) {
            throw new Exception("Component.class != Component[].getComponentType()");
        }
    }

    private static ClassLoader createClassLoader(RaceCoordinator coordinator) {
        // Custom parent class loader disables background verification which might influence
        // this test.
        ClassLoader parent = new ClassLoader(Main.class.getClassLoader()) {};
        ClassLoader sharedLibrary = new ClassLoader(Main.class.getClassLoader()) {
            @Override
            public Class findClass(String name) throws ClassNotFoundException {
                return coordinator.onSharedLibraryFindClass(name);
            }
        };
        return new PathClassLoader(DEX_FILE, null, parent, new ClassLoader[] {sharedLibrary});
    }
}

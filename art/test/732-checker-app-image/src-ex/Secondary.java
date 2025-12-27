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

public class Secondary {
    public static void main() {
        System.out.println($noinline$getSecondaryAppImageClass().getName());
        System.out.println($noinline$getSecondaryNonAppImageClass().getName());
        System.out.println($noinline$getSecondaryClassWithUnmetDependency().getName());
    }

    /// CHECK-START: java.lang.Class Secondary.$noinline$getSecondaryAppImageClass() builder (after)
    /// CHECK:            LoadClass load_kind:AppImageRelRo in_image:true
    public static Class<?> $noinline$getSecondaryAppImageClass() {
        return SecondaryAppImageClass.class;
    }

    /// CHECK-START: java.lang.Class Secondary.$noinline$getSecondaryNonAppImageClass() builder (after)
    /// CHECK:            LoadClass load_kind:BssEntry in_image:false
    public static Class<?> $noinline$getSecondaryNonAppImageClass() {
        return SecondaryNonAppImageClass.class;
    }

    /// CHECK-START: java.lang.Class Secondary.$noinline$getSecondaryClassWithUnmetDependency() builder (after)
    /// CHECK:            LoadClass load_kind:BssEntry in_image:false
    public static Class<?> $noinline$getSecondaryClassWithUnmetDependency() {
        return SecondaryClassWithUnmetDependency.class;
    }
}

class SecondaryAppImageClass {  // Included in the profile.
}

class SecondaryNonAppImageClass {  // Not included in the profile.
}

// Included in the profile but with interface defined in parent class loader
// and therefore unsuitable for inclusion in the app image.
class SecondaryClassWithUnmetDependency implements PrimaryInterface {}

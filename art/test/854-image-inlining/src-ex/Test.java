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

class WithStatic {
  public static int field = 42;
}

// `CallStatic` being part of the profile, the compiler used to think it will be in the app image.
// However, given it implements `Itf` which is from a different dex file from a different class
// loader, we cannot encode `CallStatic` in the image. We used to find that information too late in
// the compilation stage and were therefore wrongly assuming we could inline methods from
// `CallStatic` without the need for resolving it.
//
// Note that to trigger the crash, we needed an interface from a different dex file. We were
// correctly pruning the class if the superclass was from a different dex file.
class CallStatic implements Itf {
  public static int $inline$foo() {
    // Access a static field to make sure we invoke the runtime, which will then walk the call
    // stack.
    return WithStatic.field;
  }
}

public class Test {
  public static int callInstance() {
    // Call a method from `CallStatic` which will be inlined. If the compiler thinks `CallStatic`
    // will be in the image, it will skip the slow path of resolving it.
    return CallStatic.$inline$foo();
  }
}

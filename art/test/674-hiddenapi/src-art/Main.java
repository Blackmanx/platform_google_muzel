/*
 * Copyright (C) 2017 The Android Open Source Project
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
import dalvik.system.VMRuntime;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.nio.file.Files;
import java.util.Arrays;

public class Main {
  // This needs to be kept in sync with DexDomain in ChildClass.
  enum DexDomain {
    CorePlatform,
    Platform,
    Application
  }

  public static void main(String[] args) throws Exception {
    System.loadLibrary(args[0]);
    prepareNativeLibFileName(args[1]);

    // Cache flag value before `init()` makes the flag method inaccessible.
    hiddenapiJniApiCallers = com.android.art.flags.Flags.hiddenapiJniApiCallers();

    // Enable hidden API checks in case they are disabled by default. Sets
    // targetSdkVersion to the version for O (i.e. 27).
    init();

    // TODO there are sequential depencies between these test cases, and bugs
    // in the production code may lead to subsequent tests to erroneously pass,
    // or test the wrong thing. We rely on not deduping hidden API warnings
    // here for the same reasons), meaning the code under test and production
    // code are running in different configurations. Each test should be run in
    // a fresh process to ensure that they are working correctly and not
    // accidentally interfering with each other.
    // As a side effect, we also cannot test Platform->Platform and later
    // Platform->CorePlatform as the former succeeds in verifying linkage usages
    // that should fail in the latter.
    // We also cannot use InMemoryDexClassLoader because it runs verification in
    // a background thread and being able to dynamically change the configuration
    // (like list of exemptions) would require proper thread synchronization.

    // Run test with both parent and child dex files loaded with class loaders.
    // The expectation is that hidden members in parent should be visible to
    // the child.
    doTest(DexDomain.Application, DexDomain.Application, false, false);
    doUnloading();

    // Now append parent dex file to boot class path and run again. This time
    // the child dex file should not be able to access private APIs of the
    // parent.
    int parentIdx = appendToBootClassLoader(DEX_PARENT_BOOT, /* isCorePlatform */ false);
    doTest(DexDomain.Platform, DexDomain.Application, false, false);
    doUnloading();

    // Now run the same test again, but with the blocklist exemptions list set
    // to "L" which matches everything.
    doTest(DexDomain.Platform, DexDomain.Application, true, false);
    doUnloading();

    // Repeat the two tests above, only with parent being a core-platform dex file.
    setDexDomain(parentIdx, /* isCorePlatform */ true);
    doTest(DexDomain.CorePlatform, DexDomain.Application, false, false);
    doUnloading();
    doTest(DexDomain.CorePlatform, DexDomain.Application, true, false);
    doUnloading();

    // The following tests use the boot class loader to load ChildClass, and
    // that class loader uses the "system" namespace in the native linker
    // namespace config rather than the usual "clns-XXX" namespaces created for
    // class loaders by libnativeloader. Hence we need to add links to the libs
    // in NATIVELOADER_DEFAULT_NAMESPACE_LIBS (in particular libarttest(d).so)
    // to the "system" namespace, so that the tests below can load the copy of
    // libarttest(d)_external.so (which depends on libarttest(d).so). Note that
    // this cannot be undone.
    addDefaultNamespaceLibsLinkToSystemLinkerNamespace();

    // Append child to boot class path, first as a platform dex file.
    // It should not be allowed to access non-public, non-core platform API members.
    int childIdx = appendToBootClassLoader(DEX_CHILD, /* isCorePlatform */ false);
    doTest(DexDomain.CorePlatform, DexDomain.Platform, false, false);
    doUnloading();

    // And finally change child to core-platform dex. With both in the boot classpath
    // and both core-platform, access should be granted.
    setDexDomain(childIdx, /* isCorePlatform */ true);
    doTest(DexDomain.CorePlatform, DexDomain.CorePlatform, false, false);

    // With the child class in the core-platform domain, run tests again to test
    // native callers in the app domain which are invoked directly from a
    // core-platform Java caller. This reduces the false negatives in verifying
    // that hiddenapi checks the closest native caller frame over the closest
    // Java caller. Disabled on host because hiddenapi doesn't support
    // identifying domains for native callers there.
    if (!runningOnHost() && hiddenapiJniApiCallers) {
      VMRuntime.getRuntime().setTargetSdkVersion(10000); // Enable native frame check in app domain.

      doTest(DexDomain.CorePlatform, DexDomain.Application, false, true);

      // This combination currently doesn't do anything because we don't install
      // a DSO in the platform domain, but it's here for the sake of completeness.
      doTest(DexDomain.CorePlatform, DexDomain.Platform, false, true);

      // Run this combination as well for the sake of completeness.
      doTest(DexDomain.CorePlatform, DexDomain.CorePlatform, false, true);
    }

    doUnloading();
  }

  private static void doTest(DexDomain parentDomain, DexDomain childDomain, boolean addAllApisToSdk,
      boolean checkNativeOnly) throws Exception {
    // Load parent dex if it is not in boot class path.
    ClassLoader parentLoader = null;
    if (parentDomain == DexDomain.Application) {
      parentLoader = new PathClassLoader(DEX_PARENT, ClassLoader.getSystemClassLoader());
    } else {
      parentLoader = BOOT_CLASS_LOADER;
    }

    // Load child dex if it is not in boot class path.
    ClassLoader childLoader = null;
    if (!checkNativeOnly && childDomain == DexDomain.Application) {
      childLoader = new PathClassLoader(DEX_CHILD, parentLoader);
    } else {
      if (parentLoader != BOOT_CLASS_LOADER) {
        throw new IllegalStateException(
            "DeclaringClass must be in parent class loader of CallingClass");
      }
      childLoader = BOOT_CLASS_LOADER;
    }

    // Create a unique copy of the native library. Each shared library can only
    // be loaded once, but for some reason even classes from a class loader
    // cannot register their native methods against symbols in a shared library
    // loaded by their parent class loader.
    String nativeLibCopy =
        createNativeLibCopy(parentDomain, childDomain, addAllApisToSdk, checkNativeOnly);

    // Set exemptions to "L" (matches all classes) if we are testing sdk APIs.
    setSdkAll(addAllApisToSdk);

    // Invoke ChildClass.runTest
    Class<?> childClass = Class.forName("ChildClass", true, childLoader);
    Method runTestMethod = childClass.getDeclaredMethod(
        "runTest", String.class, Integer.TYPE, Integer.TYPE, Boolean.TYPE, Boolean.TYPE);
    runTestMethod.invoke(null, nativeLibCopy, parentDomain.ordinal(), childDomain.ordinal(),
        addAllApisToSdk, checkNativeOnly);

    if (parentDomain == DexDomain.CorePlatform && childDomain == DexDomain.Platform
        && !checkNativeOnly) {
      // Ad-hoc test for platform -> core-platform. The parent domain doesn't actually matter here.
      childClass.getDeclaredMethod("checkNonCorePlatformApis").invoke(null);
    }
  }

  // Routine which tries to figure out the absolute path of our native libarttest(d)_external.so.
  private static void prepareNativeLibFileName(String arg) throws Exception {
    String libName = System.mapLibraryName(arg);
    Method libPathsMethod = Runtime.class.getDeclaredMethod("getLibPaths");
    libPathsMethod.setAccessible(true);
    String[] libPaths = (String[]) libPathsMethod.invoke(Runtime.getRuntime());
    nativeLibFileName = null;
    for (String p : libPaths) {
      String candidate = p + libName;
      if (new File(candidate).exists()) {
        nativeLibFileName = candidate;
        break;
      }
    }
    if (nativeLibFileName == null) {
      throw new IllegalStateException("Didn't find " + libName + " in " +
          Arrays.toString(libPaths));
    }
  }

  // Copy native library to a new file with a unique name so it does not
  // conflict with other loaded instance of the same binary file.
  private static String createNativeLibCopy(DexDomain parentDomain, DexDomain childDomain,
      boolean addAllApisToSdk, boolean checkNativeOnly) throws Exception {
    String tempFileName = System.mapLibraryName("hiddenapitest_" + (parentDomain.ordinal())
        + (childDomain.ordinal()) + (addAllApisToSdk ? "1" : "0") + (checkNativeOnly ? "1" : "0"));
    File tempFile = new File(System.getenv("DEX_LOCATION"), tempFileName);
    Files.copy(new File(nativeLibFileName).toPath(), tempFile.toPath());
    tempFile.setWritable(false);
    return tempFile.getAbsolutePath();
  }

  private static void doUnloading() {
    // Do multiple GCs to prevent rare flakiness if some other thread is
    // keeping the classloader live.
    for (int i = 0; i < 5; ++i) {
       Runtime.getRuntime().gc();
    }
  }

  private static String nativeLibFileName;

  private static final String DEX_PARENT =
      new File(System.getenv("DEX_LOCATION"), "674-hiddenapi.jar").getAbsolutePath();
  private static final String DEX_PARENT_BOOT =
      new File(new File(System.getenv("DEX_LOCATION"), "res"), "boot.jar").getAbsolutePath();
  private static final String DEX_CHILD =
      new File(System.getenv("DEX_LOCATION"), "674-hiddenapi-ex.jar").getAbsolutePath();

  private static ClassLoader BOOT_CLASS_LOADER = Object.class.getClassLoader();

  private static boolean hiddenapiJniApiCallers;

  private static native void addDefaultNamespaceLibsLinkToSystemLinkerNamespace();
  private static native int appendToBootClassLoader(String dexPath, boolean isCorePlatform);
  private static native void setDexDomain(int index, boolean isCorePlatform);
  private static native void init();
  private static native boolean runningOnHost();
  private static native void setSdkAll(boolean value);
}

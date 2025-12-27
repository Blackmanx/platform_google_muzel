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

import dalvik.system.VMRuntime;

import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.function.Consumer;

import sun.misc.Unsafe;

public class ChildClass {
  enum PrimitiveType {
    TInteger('I', Integer.TYPE, Integer.valueOf(0)),
    TLong('J', Long.TYPE, Long.valueOf(0)),
    TFloat('F', Float.TYPE, Float.valueOf(0)),
    TDouble('D', Double.TYPE, Double.valueOf(0)),
    TBoolean('Z', Boolean.TYPE, Boolean.valueOf(false)),
    TByte('B', Byte.TYPE, Byte.valueOf((byte) 0)),
    TShort('S', Short.TYPE, Short.valueOf((short) 0)),
    TCharacter('C', Character.TYPE, Character.valueOf('0'));

    PrimitiveType(char shorty, Class klass, Object value) {
      mShorty = shorty;
      mClass = klass;
      mDefaultValue = value;
    }

    public char mShorty;
    public Class mClass;
    public Object mDefaultValue;
  }

  enum Hiddenness {
    Sdk(PrimitiveType.TShort),
    Unsupported(PrimitiveType.TBoolean),
    ConditionallyBlocked(PrimitiveType.TByte),
    Blocklist(PrimitiveType.TCharacter),
    BlocklistAndCorePlatformApi(PrimitiveType.TInteger);

    Hiddenness(PrimitiveType type) { mAssociatedType = type; }
    public PrimitiveType mAssociatedType;
  }

  enum Visibility {
    Public(PrimitiveType.TInteger),
    Package(PrimitiveType.TFloat),
    Protected(PrimitiveType.TLong),
    Private(PrimitiveType.TDouble);

    Visibility(PrimitiveType type) { mAssociatedType = type; }
    public PrimitiveType mAssociatedType;
  }

  enum Behaviour {
    Granted,
    Warning,
    Denied,
  }

  // This needs to be kept in sync with DexDomain in Main.
  enum DexDomain {
    CorePlatform,
    Platform,
    Application
  }

  private static final boolean booleanValues[] = new boolean[] { false, true };

  // checkNativeOnly: Only run tests where the caller is in native code - this ChildClass object
  // is in the core-platform domain when it is true.
  public static void runTest(String libFileName, int parentDomainOrdinal, int childDomainOrdinal,
      boolean everythingSdked, boolean checkNativeOnly) throws Exception {
    System.load(libFileName);

    DexDomain parentDomain = DexDomain.values()[parentDomainOrdinal];
    DexDomain childDomain = DexDomain.values()[childDomainOrdinal];

    boolean skipJniTests = false;
    if (childDomain == DexDomain.Application) {
      registerAppJniApiCallers(JNI.class);
    } else if (childDomain == DexDomain.CorePlatform) {
      registerCorePlatformJniApiCallers(JNI.class);
    } else {
      // It's difficult to put a .so in a location that identifies it as the platform domain, so
      // skip those JNI tests.
      skipJniTests = true;
    }

    int targetSdkVersion = VMRuntime.getRuntime().getTargetSdkVersion();
    if (checkNativeOnly) {
      if (targetSdkVersion != 10000) {
        throw new RuntimeException(
            "Expected future targetSdkVersion for native caller tests, got " + targetSdkVersion);
      }
    }

    // Check expectations about loading into boot class path.
    boolean isParentInBoot = (ParentClass.class.getClassLoader().getParent() == null);
    boolean expectedParentInBoot = (parentDomain != DexDomain.Application);
    if (isParentInBoot != expectedParentInBoot) {
      throw new RuntimeException("Expected ParentClass " +
                                 (expectedParentInBoot ? "" : "not ") + "in boot class path");
    }
    boolean isChildInBoot = (ChildClass.class.getClassLoader().getParent() == null);
    boolean expectedChildInBoot = (childDomain != DexDomain.Application);
    if (isChildInBoot != expectedChildInBoot && !checkNativeOnly) {
      throw new RuntimeException("Expected ChildClass " + (expectedChildInBoot ? "" : "not ") +
                                 "in boot class path");
    }

    boolean isSameBoot = (expectedParentInBoot == expectedChildInBoot);

    // For compat reasons, meta-reflection should still be usable by apps if hidden api check
    // hardening is disabled (i.e. target SDK is Q or earlier). The only configuration where this
    // workaround used to work is for ChildClass in the Application domain and ParentClass in the
    // Platform domain, so only test that configuration with hidden api check hardening disabled.
    boolean testHiddenApiCheckHardeningDisabled =
        (childDomain == DexDomain.Application) && (parentDomain == DexDomain.Platform);

    // Run meaningful combinations of access flags.
    for (Hiddenness hiddenness : Hiddenness.values()) {
      final Behaviour expected;
      final boolean invokesMemberCallback;
      // Warnings are now disabled whenever access is granted, even for
      // greylisted APIs. This is the behaviour for release builds.
      if (everythingSdked || hiddenness == Hiddenness.Sdk) {
        expected = Behaviour.Granted;
        invokesMemberCallback = false;
      } else if (parentDomain == DexDomain.CorePlatform && childDomain == DexDomain.Platform) {
        expected = (hiddenness == Hiddenness.Unsupported
                           || hiddenness == Hiddenness.BlocklistAndCorePlatformApi)
                ? Behaviour.Granted
                : Behaviour.Denied;
        invokesMemberCallback = false;
      } else if (isSameBoot) {
        expected = Behaviour.Granted;
        invokesMemberCallback = false;
      } else if (hiddenness == Hiddenness.Blocklist ||
                 hiddenness == Hiddenness.BlocklistAndCorePlatformApi) {
        expected = Behaviour.Denied;
        invokesMemberCallback = true;
      } else if (hiddenness == Hiddenness.ConditionallyBlocked && targetSdkVersion == 10000) {
        expected = Behaviour.Denied;
        invokesMemberCallback = true; // Not applicable when checkNativeOnly is true.
      } else {
        expected = Behaviour.Warning;
        invokesMemberCallback = true;
      }

      // Saved variables from the loops below to be shown in the test context if an exception is
      // thrown.
      boolean saveIsStatic = false;
      Visibility saveVisibility = Visibility.Public;
      Class saveParentClass = null;

      try {
        for (boolean isStatic : booleanValues) {
          saveIsStatic = isStatic;
          String suffix = (isStatic ? "Static" : "") + hiddenness.name();

          for (Visibility visibility : Visibility.values()) {
            saveVisibility = visibility;
            // Test reflection and JNI on methods and fields
            for (Class parentClass : new Class<?>[] {ParentClass.class, ParentInterface.class}) {
              saveParentClass = parentClass;
              String baseName = visibility.name() + suffix;
              checkField(parentClass, "field" + baseName, isStatic, visibility, expected,
                  invokesMemberCallback, testHiddenApiCheckHardeningDisabled, skipJniTests,
                  checkNativeOnly);
              checkMethod(parentClass, "method" + baseName, isStatic, visibility, expected,
                  invokesMemberCallback, testHiddenApiCheckHardeningDisabled, skipJniTests,
                  checkNativeOnly);
            }
            saveParentClass = null;

            // Check whether one can use a class constructor.
            checkConstructor(ParentClass.class, visibility, hiddenness, expected,
                testHiddenApiCheckHardeningDisabled, skipJniTests, checkNativeOnly);

            // Check whether one can use an interface default method.
            String name = "method" + visibility.name() + "Default" + hiddenness.name();
            checkMethod(ParentInterface.class, name, /*isStatic*/ false, visibility, expected,
                invokesMemberCallback, testHiddenApiCheckHardeningDisabled, skipJniTests,
                checkNativeOnly);
          }

          if (!checkNativeOnly) {
            // Test whether static linking succeeds.
            checkLinking("LinkFieldGet" + suffix, /*takesParameter*/ false, expected);
            checkLinking("LinkFieldSet" + suffix, /*takesParameter*/ true, expected);
            checkLinking("LinkMethod" + suffix, /*takesParameter*/ false, expected);
            checkLinking("LinkMethodInterface" + suffix, /*takesParameter*/ false, expected);
          }
        }

        if (!checkNativeOnly) {
          // Check whether Class.newInstance succeeds.
          checkNullaryConstructor(
              Class.forName("NullaryConstructor" + hiddenness.name()), expected);
        }
      } catch (Exception e) {
        throw new RuntimeException("Exception in test context {parentDomain=" + parentDomain.name()
                + ", childDomain=" + childDomain.name() + ", everythingSdked=" + everythingSdked
                + ", checkNativeOnly=" + checkNativeOnly
                + ", targetSdkVersion=" + targetSdkVersion + ", hiddenness=" + hiddenness
                + ", expected=" + expected + ", invokesMemberCallback=" + invokesMemberCallback
                + ", isStatic=" + saveIsStatic + ", visibility=" + saveVisibility
                + ", parentClass=" + saveParentClass + "}",
            e);
      }
    }
  }

  static final class RecordingConsumer implements Consumer<String> {
      public String recordedValue = null;

      @Override
      public void accept(String value) {
          recordedValue = value;
      }
  }

  private static void checkMemberCallback(Class<?> klass, String name,
          boolean isPublic, boolean isField, boolean expectedCallback) {
      try {
          RecordingConsumer consumer = new RecordingConsumer();
          VMRuntime.setNonSdkApiUsageConsumer(consumer);
          try {
              if (isPublic) {
                  if (isField) {
                      klass.getField(name);
                  } else {
                      klass.getMethod(name);
                  }
              } else {
                  if (isField) {
                      klass.getDeclaredField(name);
                  } else {
                      klass.getDeclaredMethod(name);
                  }
              }
          } catch (NoSuchFieldException|NoSuchMethodException ignored) {
              // We're not concerned whether an exception is thrown or not - we're
              // only interested in whether the callback is invoked.
          }

          boolean actualCallback = consumer.recordedValue != null &&
                          consumer.recordedValue.contains(name);
          if (expectedCallback != actualCallback) {
              if (expectedCallback) {
                throw new RuntimeException("Expected callback for member: " + name);
              } else {
                throw new RuntimeException("Did not expect callback for member: " + name);
              }
          }
      } finally {
          VMRuntime.setNonSdkApiUsageConsumer(null);
      }
  }

  private static void checkField(Class<?> klass, String name, boolean isStatic,
      Visibility visibility, Behaviour behaviour, boolean invokesMemberCallback,
      boolean testHiddenApiCheckHardeningDisabled, boolean skipJniTests, boolean checkNativeOnly)
      throws Exception {
    boolean isPublic = (visibility == Visibility.Public);
    boolean canDiscover = (behaviour != Behaviour.Denied);

    if (klass.isInterface() && (!isStatic || !isPublic)) {
      // Interfaces only have public static fields.
      return;
    }

    // Test discovery with JNI.

    if (!skipJniTests && JNI.canDiscoverField(klass, name, isStatic) != canDiscover) {
      throwDiscoveryException(klass, name, true, "JNI", canDiscover);
    }

    if (!checkNativeOnly) {
      // Test discovery with reflection.

      if (Reflection.canDiscoverWithGetDeclaredField(klass, name) != canDiscover) {
        throwDiscoveryException(klass, name, true, "getDeclaredField()", canDiscover);
      }

      if (Reflection.canDiscoverWithGetDeclaredFields(klass, name) != canDiscover) {
        throwDiscoveryException(klass, name, true, "getDeclaredFields()", canDiscover);
      }

      if (Reflection.canDiscoverWithGetField(klass, name) != (canDiscover && isPublic)) {
        throwDiscoveryException(klass, name, true, "getField()", (canDiscover && isPublic));
      }

      if (Reflection.canDiscoverWithGetFields(klass, name) != (canDiscover && isPublic)) {
        throwDiscoveryException(klass, name, true, "getFields()", (canDiscover && isPublic));
      }

      // Test discovery with MethodHandles.lookup() which is caller
      // context sensitive.

      final MethodHandles.Lookup lookup = MethodHandles.lookup();
      if (JLI.canDiscoverWithLookupFindGetter(lookup, klass, name, int.class) != canDiscover) {
        throwDiscoveryException(
            klass, name, true, "MethodHandles.lookup().findGetter()", canDiscover);
      }
      if (JLI.canDiscoverWithLookupFindStaticGetter(lookup, klass, name, int.class)
          != canDiscover) {
        throwDiscoveryException(
            klass, name, true, "MethodHandles.lookup().findStaticGetter()", canDiscover);
      }

      // Test discovery with MethodHandles.publicLookup() which can only
      // see public fields. Looking up setters here and fields in
      // interfaces are implicitly final.

      final MethodHandles.Lookup publicLookup = MethodHandles.publicLookup();
      if (JLI.canDiscoverWithLookupFindSetter(publicLookup, klass, name, int.class)
          != canDiscover) {
        throwDiscoveryException(
            klass, name, true, "MethodHandles.publicLookup().findSetter()", canDiscover);
      }
      if (JLI.canDiscoverWithLookupFindStaticSetter(publicLookup, klass, name, int.class)
          != canDiscover) {
        throwDiscoveryException(
            klass, name, true, "MethodHandles.publicLookup().findStaticSetter()", canDiscover);
      }

      // Check for meta reflection.

      // With hidden api check hardening enabled, only white and light greylisted fields should be
      // discoverable.
      if (Reflection.canDiscoverFieldWithMetaReflection(klass, name, true) != canDiscover) {
        throwDiscoveryException(
            klass, name, false, "Meta reflection with hidden api hardening enabled", canDiscover);
      }

      if (testHiddenApiCheckHardeningDisabled) {
        // With hidden api check hardening disabled, all fields should be discoverable.
        if (Reflection.canDiscoverFieldWithMetaReflection(klass, name, false) != true) {
          throwDiscoveryException(
              klass, name, false, "Meta reflection with hidden api hardening enabled", canDiscover);
        }
      }
    }

    if (canDiscover) {
      if (!checkNativeOnly) {
        // Test that modifiers are unaffected.

        if (Reflection.canObserveFieldHiddenAccessFlags(klass, name)) {
          throwModifiersException(klass, name, true);
        }

        // Test getters and setters when meaningful.

        if (!Reflection.canGetField(klass, name)) {
          throwAccessException(klass, name, true, "Field.getInt()");
        }
        if (!isUnmodifiable(klass, name) && !Reflection.canSetField(klass, name)) {
          throwAccessException(klass, name, true, "Field.setInt()");
        }
      }

      if (!skipJniTests) {
        if (!JNI.canGetField(klass, name, isStatic)) {
          throwAccessException(klass, name, true, "getIntField");
        }
        if (!isUnmodifiable(klass, name) && !JNI.canSetField(klass, name, isStatic)) {
          throwAccessException(klass, name, true, "setIntField");
        }
      }
    }

    // Test that callbacks are invoked correctly.
    if (!checkNativeOnly) {
      checkMemberCallback(klass, name, isPublic, true /* isField */, invokesMemberCallback);
    }
  }

  private static final boolean isUnmodifiable(Class<?> klass, String fieldName) throws Exception {
    Field field = klass.getDeclaredField(fieldName);

    return Modifier.isFinal(field.getModifiers()) && Modifier.isStatic(field.getModifiers());
  }

  private static void checkMethod(Class<?> klass, String name, boolean isStatic,
      Visibility visibility, Behaviour behaviour, boolean invokesMemberCallback,
      boolean testHiddenApiCheckHardeningDisabled, boolean skipJniTests, boolean checkNativeOnly)
      throws Exception {
    boolean isPublic = (visibility == Visibility.Public);
    if (klass.isInterface() && !isPublic) {
      // All interface members are public.
      return;
    }

    boolean canDiscover = (behaviour != Behaviour.Denied);

    // Test discovery with JNI.

    if (!skipJniTests && JNI.canDiscoverMethod(klass, name, isStatic) != canDiscover) {
      throwDiscoveryException(klass, name, false, "JNI", canDiscover);
    }

    if (!checkNativeOnly) {
      // Test discovery with reflection.

      if (Reflection.canDiscoverWithGetDeclaredMethod(klass, name) != canDiscover) {
        throwDiscoveryException(klass, name, false, "getDeclaredMethod()", canDiscover);
      }

      if (Reflection.canDiscoverWithGetDeclaredMethods(klass, name) != canDiscover) {
        throwDiscoveryException(klass, name, false, "getDeclaredMethods()", canDiscover);
      }

      if (Reflection.canDiscoverWithGetMethod(klass, name) != (canDiscover && isPublic)) {
        throwDiscoveryException(klass, name, false, "getMethod()", (canDiscover && isPublic));
      }

      if (Reflection.canDiscoverWithGetMethods(klass, name) != (canDiscover && isPublic)) {
        throwDiscoveryException(klass, name, false, "getMethods()", (canDiscover && isPublic));
      }

      // Test discovery with MethodHandles.lookup().

      final MethodHandles.Lookup lookup = MethodHandles.lookup();
      final MethodType methodType = MethodType.methodType(int.class);
      if (JLI.canDiscoverWithLookupFindVirtual(lookup, klass, name, methodType) != canDiscover) {
        throwDiscoveryException(
            klass, name, false, "MethodHandles.lookup().findVirtual()", canDiscover);
      }

      if (JLI.canDiscoverWithLookupFindStatic(lookup, klass, name, methodType) != canDiscover) {
        throwDiscoveryException(
            klass, name, false, "MethodHandles.lookup().findStatic()", canDiscover);
      }

      // Check for meta reflection.

      // With hidden api check hardening enabled, only white and light greylisted methods should be
      // discoverable.
      if (Reflection.canDiscoverMethodWithMetaReflection(klass, name, true) != canDiscover) {
        throwDiscoveryException(
            klass, name, false, "Meta reflection with hidden api hardening enabled", canDiscover);
      }

      if (testHiddenApiCheckHardeningDisabled) {
        // With hidden api check hardening disabled, all methods should be discoverable.
        if (Reflection.canDiscoverMethodWithMetaReflection(klass, name, false) != true) {
          throwDiscoveryException(
              klass, name, false, "Meta reflection with hidden api hardening enabled", canDiscover);
        }
      }
    }

    // Finish here if we could not discover the method.

    if (canDiscover) {
      // Test that modifiers are unaffected.

      if (!checkNativeOnly && Reflection.canObserveMethodHiddenAccessFlags(klass, name)) {
        throwModifiersException(klass, name, false);
      }

      // Test whether we can invoke the method. This skips non-static interface methods.
      if (!klass.isInterface() || isStatic) {
        if (!checkNativeOnly && !Reflection.canInvokeMethod(klass, name)) {
          throwAccessException(klass, name, false, "invoke()");
        }
        if (!skipJniTests) {
          if (!JNI.canInvokeMethodA(klass, name, isStatic)) {
            throwAccessException(klass, name, false, "CallMethodA");
          }
          if (!JNI.canInvokeMethodV(klass, name, isStatic)) {
            throwAccessException(klass, name, false, "CallMethodV");
          }
        }
      }
    }

    // Test that callbacks are invoked correctly.
    if (!checkNativeOnly) {
      checkMemberCallback(klass, name, isPublic, false /* isField */, invokesMemberCallback);
    }
  }

  private static void checkConstructor(Class<?> klass, Visibility visibility, Hiddenness hiddenness,
      Behaviour behaviour, boolean testHiddenApiCheckHardeningDisabled, boolean skipJniTests,
      boolean checkNativeOnly) throws Exception {
    boolean isPublic = (visibility == Visibility.Public);
    String signature = "(" + visibility.mAssociatedType.mShorty +
                             hiddenness.mAssociatedType.mShorty + ")V";
    String fullName = "<init>" + signature;
    Class<?> args[] = new Class[] { visibility.mAssociatedType.mClass,
                                    hiddenness.mAssociatedType.mClass };
    Object initargs[] = new Object[] { visibility.mAssociatedType.mDefaultValue,
                                       hiddenness.mAssociatedType.mDefaultValue };
    MethodType methodType = MethodType.methodType(void.class, args);

    boolean canDiscover = (behaviour != Behaviour.Denied);

    // Test discovery with JNI.

    if (!skipJniTests && JNI.canDiscoverConstructor(klass, signature) != canDiscover) {
      throwDiscoveryException(klass, fullName, false, "JNI", canDiscover);
    }

    if (!checkNativeOnly) {
      // Test discovery with reflection.

      if (Reflection.canDiscoverWithGetDeclaredConstructor(klass, args) != canDiscover) {
        throwDiscoveryException(klass, fullName, false, "getDeclaredConstructor()", canDiscover);
      }

      if (Reflection.canDiscoverWithGetDeclaredConstructors(klass, args) != canDiscover) {
        throwDiscoveryException(klass, fullName, false, "getDeclaredConstructors()", canDiscover);
      }

      if (Reflection.canDiscoverWithGetConstructor(klass, args) != (canDiscover && isPublic)) {
        throwDiscoveryException(
            klass, fullName, false, "getConstructor()", (canDiscover && isPublic));
      }

      if (Reflection.canDiscoverWithGetConstructors(klass, args) != (canDiscover && isPublic)) {
        throwDiscoveryException(
            klass, fullName, false, "getConstructors()", (canDiscover && isPublic));
      }

      // Test discovery with MethodHandles.lookup()

      final MethodHandles.Lookup lookup = MethodHandles.lookup();
      if (JLI.canDiscoverWithLookupFindConstructor(lookup, klass, methodType) != canDiscover) {
        throwDiscoveryException(
            klass, fullName, false, "MethodHandles.lookup().findConstructor", canDiscover);
      }

      final MethodHandles.Lookup publicLookup = MethodHandles.publicLookup();
      if (JLI.canDiscoverWithLookupFindConstructor(publicLookup, klass, methodType)
          != canDiscover) {
        throwDiscoveryException(
            klass, fullName, false, "MethodHandles.publicLookup().findConstructor", canDiscover);
      }

      // Check for meta reflection.

      // With hidden api check hardening enabled, only white and light greylisted constructors
      // should be discoverable.
      if (Reflection.canDiscoverConstructorWithMetaReflection(klass, args, true) != canDiscover) {
        throwDiscoveryException(klass, fullName, false,
            "Meta reflection with hidden api hardening enabled", canDiscover);
      }

      if (testHiddenApiCheckHardeningDisabled) {
        // With hidden api check hardening disabled, all constructors should be discoverable.
        if (Reflection.canDiscoverConstructorWithMetaReflection(klass, args, false) != true) {
          throwDiscoveryException(klass, fullName, false,
              "Meta reflection with hidden api hardening enabled", canDiscover);
        }
      }
    }

    if (canDiscover) {
      // Test whether we can invoke the constructor.

      if (!checkNativeOnly && !Reflection.canInvokeConstructor(klass, args, initargs)) {
        throwAccessException(klass, fullName, false, "invoke()");
      }
      if (!skipJniTests) {
        if (!JNI.canInvokeConstructorA(klass, signature)) {
          throwAccessException(klass, fullName, false, "NewObjectA");
        }
        if (!JNI.canInvokeConstructorV(klass, signature)) {
          throwAccessException(klass, fullName, false, "NewObjectV");
        }
      }
    }
  }

  private static void checkNullaryConstructor(Class<?> klass, Behaviour behaviour)
      throws Exception {
    boolean canAccess = (behaviour != Behaviour.Denied);

    if (Reflection.canUseNewInstance(klass) != canAccess) {
      throw new RuntimeException(
          "Expected to " + (canAccess ? "" : "not ") + "be able to construct " + klass.getName());
    }
  }

  private static void checkLinking(String className, boolean takesParameter, Behaviour behaviour)
      throws Exception {
    boolean canAccess = (behaviour != Behaviour.Denied);

    if (Linking.canAccess(className, takesParameter) != canAccess) {
      throw new RuntimeException(
          "Expected to " + (canAccess ? "" : "not ") + "be able to verify " + className);
    }
  }

  private static void throwDiscoveryException(Class<?> klass, String name, boolean isField,
      String fn, boolean canAccess) {
    throw new RuntimeException("Expected " + (isField ? "field " : "method ") + klass.getName()
        + "." + name + " to " + (canAccess ? "" : "not ") + "be discoverable with " + fn);
  }

  private static void throwAccessException(Class<?> klass, String name, boolean isField,
      String fn) {
    throw new RuntimeException("Expected to be able to access " + (isField ? "field " : "method ")
        + klass.getName() + "." + name + " using " + fn);
  }

  private static void throwModifiersException(Class<?> klass, String name, boolean isField) {
    throw new RuntimeException("Expected " + (isField ? "field " : "method ") + klass.getName() +
        "." + name + " to not expose hidden modifiers");
  }

  private static native void registerCorePlatformJniApiCallers(Class targetClass);
  private static native void registerAppJniApiCallers(Class targetClass);

  // Check a few real symbols that HiddenApiSdk28AppTest depends on being inaccessible from platform
  // to core-platform, to avoid false negatives. Only called when ChildClass is in the platform
  // domain.
  public static void checkNonCorePlatformApis() {
    try {
      Field f = Byte.class.getDeclaredField("value");
      throw new RuntimeException("Expected java.lang.Byte.value to be inaccessible from platform");
    } catch (NoSuchFieldException expected) {
    }
    try {
      Method m =
          Unsafe.class.getDeclaredMethod("getAndAddInt", Object.class, Long.class, Integer.class);
      throw new RuntimeException(
          "Expected sun.misc.Unsafe.getAndAddInt to be inaccessible from platform");
    } catch (NoSuchMethodException expected) {
    }
  }
}

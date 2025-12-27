/*
 * Copyright (C) 2007 The Android Open Source Project
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


package dalvik.system;
import java.io.FileDescriptor;

@SuppressWarnings({"unchecked", "deprecation", "all"})
public final class VMDebug {

VMDebug() { throw new RuntimeException("Stub!"); }

public static native void suspendAllAndSendVmStart();

public static native long lastDebuggerActivity();

public static native boolean isDebuggingEnabled();

public static native boolean isDebuggerConnected();

public static native String[] getVmFeatureList();

public static void startMethodTracing(String traceFileName, int bufferSize, int flags, boolean samplingEnabled, int intervalUs) { throw new RuntimeException("Stub!"); }

public static void startMethodTracing(String traceFileName, FileDescriptor fd, int bufferSize, int flags, boolean samplingEnabled, int intervalUs, boolean streamingOutput) { throw new RuntimeException("Stub!"); }

public static void startMethodTracingDdms(int bufferSize, int flags, boolean samplingEnabled, int intervalUs) { throw new RuntimeException("Stub!"); }

public static native int getMethodTracingMode();

public static native void stopMethodTracing();

public static native long threadCpuTimeNanos();

public static native void startAllocCounting();

public static native void stopAllocCounting();

public static native int getAllocCount(int kind);

public static native void resetAllocCount(int kinds);

public static native void printLoadedClasses(int flags);

public static native int getLoadedClassCount();

public static void dumpHprofData(String filename) { throw new RuntimeException("Stub!"); }

public static native void dumpHprofDataDdms();

public static void dumpHprofData(String fileName, FileDescriptor fd) { throw new RuntimeException("Stub!"); }

public static native void dumpReferenceTables();

public static native long countInstancesOfClass(Class klass, boolean assignable);

public static native long[] countInstancesOfClasses(Class[] classes, boolean assignable);

public static String getRuntimeStat(String statName) { throw new RuntimeException("Stub!"); }

public static Map<String,String> getRuntimeStats() { throw new RuntimeException("Stub!"); }

public static void attachAgent(String agent, ClassLoader classLoader) { throw new RuntimeException("Stub!"); }

public static native void setAllocTrackerStackDepth(int stackDepth);

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_APPINFO)
public static native void setCurrentProcessName(String processName);

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_APPINFO)
public static native void addApplication(String packageName);

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_APPINFO)
public static native void removeApplication(String packageName);

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_APPINFO)
public static native void setUserId(int userId);

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_APPINFO)
public static native void setWaitingForDebugger(boolean waiting);

@android.annotation.FlaggedApi(com.android.art.flags.Flags.FLAG_ALWAYS_ENABLE_PROFILE_CODE)
public static class TraceDestination {

  @android.annotation.FlaggedApi(com.android.art.flags.Flags.FLAG_ALWAYS_ENABLE_PROFILE_CODE)
  public static @NonNull TraceDestination fromFileName(@NonNull String traceFileName);

  @android.annotation.FlaggedApi(com.android.art.flags.Flags.FLAG_ALWAYS_ENABLE_PROFILE_CODE)
  public static @NonNull TraceDestination fromFileDescriptor(@NonNull FileDescriptor fd);
}

@android.annotation.FlaggedApi(com.android.art.flags.Flags.FLAG_ALWAYS_ENABLE_PROFILE_CODE)
public static void startLowOverheadTraceForAllMethods();

@android.annotation.FlaggedApi(com.android.art.flags.Flags.FLAG_ALWAYS_ENABLE_PROFILE_CODE)
public static void stopLowOverheadTrace();

@android.annotation.FlaggedApi(com.android.art.flags.Flags.FLAG_ALWAYS_ENABLE_PROFILE_CODE)
public static void dumpLowOverheadTrace(@NonNull VMDebug.TraceDestination traceFileName);

@android.annotation.FlaggedApi(com.android.art.flags.Flags.FLAG_ALWAYS_ENABLE_PROFILE_CODE)
public static void startLowOverheadTraceForLongRunningMethods(long traceDuration);

public static class ExecutableMethodFileOffsets {
  public @NonNull String getContainerPath();
  public long getContainerOffset();
  public long getMethodOffset();
}

@android.annotation.FlaggedApi(com.android.art.flags.Flags.FLAG_EXECUTABLE_METHOD_FILE_OFFSETS_DEPRECATION)
public static VMDebug.ExecutableMethodFileOffsets getExecutableMethodFileOffsets(
        @NonNull java.lang.reflect.Method javaMethod);

@android.annotation.FlaggedApi(com.android.art.flags.Flags.FLAG_EXECUTABLE_METHOD_FILE_OFFSETS_V2)
public static VMDebug.ExecutableMethodFileOffsets getExecutableMethodFileOffsets(
        @NonNull java.lang.reflect.Executable javaExecutable);

public static final int KIND_ALL_COUNTS = -1; // 0xffffffff

public static final int KIND_GLOBAL_ALLOCATED_BYTES = 2; // 0x2

public static final int KIND_GLOBAL_ALLOCATED_OBJECTS = 1; // 0x1

public static final int KIND_GLOBAL_CLASS_INIT_COUNT = 32; // 0x20

public static final int KIND_GLOBAL_CLASS_INIT_TIME = 64; // 0x40

public static final int KIND_GLOBAL_FREED_BYTES = 8; // 0x8

public static final int KIND_GLOBAL_FREED_OBJECTS = 4; // 0x4

public static final int KIND_GLOBAL_GC_INVOCATIONS = 16; // 0x10

public static final int KIND_THREAD_ALLOCATED_BYTES = 131072; // 0x20000

public static final int KIND_THREAD_ALLOCATED_OBJECTS = 65536; // 0x10000

public static final int KIND_THREAD_GC_INVOCATIONS = 1048576; // 0x100000

public static final int TRACE_COUNT_ALLOCS = 1; // 0x1
}


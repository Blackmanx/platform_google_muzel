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

package com.android.server.art.testing;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyBoolean;
import static org.mockito.Mockito.lenient;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.same;

import android.os.UserHandle;

import com.android.modules.utils.pm.PackageStateModulesUtils;
import com.android.server.pm.pkg.AndroidPackage;
import com.android.server.pm.pkg.AndroidPackageSplit;
import com.android.server.pm.pkg.PackageState;
import com.android.server.pm.pkg.PackageUserState;
import com.android.server.pm.pkg.SharedLibrary;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;

public final class TestDataHelper {
    private TestDataHelper() {}

    public static PackageStateBuilder newPackageState(String packageName) {
        return new PackageStateBuilder()
                .setPackageName(packageName)
                .setApex(false)
                .setVmSafeMode(false)
                .setDebuggable(false)
                .setSignedWithPlatformKey(false)
                .setNonSdkApiRequested(false);
    }

    public static class PackageStateBuilder {
        private final PackageState mPkgState = mock(PackageState.class);
        private final AndroidPackage mPkg = mock(AndroidPackage.class);
        private final List<AndroidPackageSplit> mSplits = new ArrayList<>();
        private final List<SharedLibrary> mDeps = new ArrayList<>();

        private PackageStateBuilder() {
            lenient().when(mPkgState.getAndroidPackage()).thenReturn(mPkg);
            lenient().when(mPkg.getSplits()).thenReturn(mSplits);
            lenient().when(mPkgState.getSharedLibraryDependencies()).thenReturn(mDeps);
        }

        public PackageStateBuilder setPackageName(String packageName) {
            lenient().when(mPkgState.getPackageName()).thenReturn(packageName);
            return this;
        }

        public PackageStateBuilder setAbis(String primaryAbi, String secondaryAbi) {
            lenient().when(mPkgState.getPrimaryCpuAbi()).thenReturn(primaryAbi);
            lenient().when(mPkgState.getSecondaryCpuAbi()).thenReturn(secondaryAbi);
            return this;
        }

        public PackageStateBuilder setAbi(String primaryAbi) {
            return setAbis(primaryAbi, null /* secondaryAbi */);
        }

        public PackageStateBuilder clearAbis() {
            return setAbis(null /* primaryAbi */, null /* secondaryAbi */);
        }

        public PackageStateBuilder setAppId(int appId) {
            lenient().when(mPkgState.getAppId()).thenReturn(appId);
            return this;
        }

        public PackageStateBuilder setSeInfo(String seInfo) {
            lenient().when(mPkgState.getSeInfo()).thenReturn(seInfo);
            return this;
        }

        public PackageStateBuilder setApex(boolean isApex) {
            lenient().when(mPkgState.isApex()).thenReturn(isApex);
            return this;
        }

        public PackageStateBuilder clearAndroidPackage() {
            lenient().when(mPkgState.getAndroidPackage()).thenReturn(null);
            return this;
        }

        public PackageStateBuilder setUserState(int userId, PackageUserState userState) {
            lenient().when(mPkgState.getStateForUser(UserHandle.of(userId))).thenReturn(userState);
            return this;
        }

        public PackageStateBuilder setUserStateForAny(PackageUserState userState) {
            lenient().when(mPkgState.getStateForUser(any())).thenReturn(userState);
            return this;
        }

        public PackageStateBuilder addSplit(AndroidPackageSplit split) {
            mSplits.add(split);
            return this;
        }

        public PackageStateBuilder addSharedLibraryDeps(SharedLibrary... deps) {
            mDeps.addAll(Arrays.asList(deps));
            return this;
        }

        public PackageStateBuilder setTargetSdkVersion(int sdkVersion) {
            lenient().when(mPkg.getTargetSdkVersion()).thenReturn(sdkVersion);
            return this;
        }

        public PackageStateBuilder setVersionName(String name) {
            lenient().when(mPkg.getVersionName()).thenReturn(name);
            return this;
        }

        public PackageStateBuilder setLongVersionCode(long versionCode) {
            lenient().when(mPkg.getLongVersionCode()).thenReturn(versionCode);
            return this;
        }

        public PackageStateBuilder setStorageUuid(UUID uuid) {
            lenient().when(mPkg.getStorageUuid()).thenReturn(uuid);
            return this;
        }

        public PackageStateBuilder setVmSafeMode(boolean isVmSafeMode) {
            lenient().when(mPkg.isVmSafeMode()).thenReturn(isVmSafeMode);
            return this;
        }

        public PackageStateBuilder setDebuggable(boolean isDebuggable) {
            lenient().when(mPkg.isDebuggable()).thenReturn(isDebuggable);
            return this;
        }

        public PackageStateBuilder setSignedWithPlatformKey(boolean isSignedWithPlatformKey) {
            lenient().when(mPkg.isSignedWithPlatformKey()).thenReturn(isSignedWithPlatformKey);
            return this;
        }

        public PackageStateBuilder setNonSdkApiRequested(boolean isNonSdkApiRequested) {
            lenient().when(mPkg.isNonSdkApiRequested()).thenReturn(isNonSdkApiRequested);
            return this;
        }

        public PackageStateBuilder setIsolatedSplitLoading(boolean isIsolatedSplitLoading) {
            lenient().when(mPkg.isIsolatedSplitLoading()).thenReturn(isIsolatedSplitLoading);
            return this;
        }

        public PackageStateBuilder setDexoptable(boolean isDexoptable) {
            lenient()
                    .when(PackageStateModulesUtils.isDexoptable(mPkgState))
                    .thenReturn(isDexoptable);
            return this;
        }

        public PackageStateBuilder setLoadableInOtherProcesses(boolean isLoadable) {
            lenient()
                    .when(PackageStateModulesUtils.isLoadableInOtherProcesses(
                            same(mPkgState), anyBoolean()))
                    .thenReturn(isLoadable);
            return this;
        }

        public PackageState build() {
            return mPkgState;
        }
    }

    public static PackageUserStateBuilder newUserState() {
        // All packages are by default pre-installed.
        return new PackageUserStateBuilder().setInstalled(true).setFirstInstallTimeMillis(0L);
    }

    public static class PackageUserStateBuilder {
        private final PackageUserState mUserState = mock(PackageUserState.class);

        public PackageUserStateBuilder setInstalled(boolean isInstalled) {
            lenient().when(mUserState.isInstalled()).thenReturn(isInstalled);
            return this;
        }

        public PackageUserStateBuilder setFirstInstallTimeMillis(long timeMillis) {
            lenient().when(mUserState.getFirstInstallTimeMillis()).thenReturn(timeMillis);
            return this;
        }

        public PackageUserState build() {
            return mUserState;
        }
    }

    public static AndroidPackageSplitBuilder newSplit() {
        // The default is base split and has code.
        return new AndroidPackageSplitBuilder().setName(null).setHasCode(true);
    }

    public static class AndroidPackageSplitBuilder {
        private final AndroidPackageSplit mSplit = mock(AndroidPackageSplit.class);
        private final List<AndroidPackageSplit> mDeps = new ArrayList<>();

        private AndroidPackageSplitBuilder() {
            lenient().when(mSplit.getDependencies()).thenReturn(mDeps);
        }

        public AndroidPackageSplitBuilder setName(String name) {
            lenient().when(mSplit.getName()).thenReturn(name);
            return this;
        }

        public AndroidPackageSplitBuilder setPath(String path) {
            lenient().when(mSplit.getPath()).thenReturn(path);
            return this;
        }

        public AndroidPackageSplitBuilder setHasCode(boolean isHasCode) {
            lenient().when(mSplit.isHasCode()).thenReturn(isHasCode);
            return this;
        }

        public AndroidPackageSplitBuilder setClassLoaderName(String classLoaderName) {
            lenient().when(mSplit.getClassLoaderName()).thenReturn(classLoaderName);
            return this;
        }

        public AndroidPackageSplitBuilder addDeps(AndroidPackageSplit... deps) {
            mDeps.addAll(Arrays.asList(deps));
            return this;
        }

        public AndroidPackageSplit build() {
            return mSplit;
        }
    }

    public static SharedLibraryBuilder newLibrary() {
        return new SharedLibraryBuilder().setNative(false);
    }

    public static class SharedLibraryBuilder {
        private final SharedLibrary mLibrary = mock(SharedLibrary.class);
        private final List<SharedLibrary> mDeps = new ArrayList<>();
        private final List<String> mCodePaths = new ArrayList<>();

        private SharedLibraryBuilder() {
            lenient().when(mLibrary.getDependencies()).thenReturn(mDeps);
            lenient().when(mLibrary.getAllCodePaths()).thenReturn(mCodePaths);
        }

        public SharedLibraryBuilder setName(String name) {
            lenient().when(mLibrary.getName()).thenReturn(name);
            return this;
        }

        public SharedLibraryBuilder setPackageName(String packageName) {
            lenient().when(mLibrary.getPackageName()).thenReturn(packageName);
            return this;
        }

        public SharedLibraryBuilder setNative(boolean isNative) {
            lenient().when(mLibrary.isNative()).thenReturn(isNative);
            return this;
        }

        public SharedLibraryBuilder addCodePaths(String... paths) {
            mCodePaths.addAll(Arrays.asList(paths));
            return this;
        }

        public SharedLibraryBuilder addDeps(SharedLibrary... deps) {
            mDeps.addAll(Arrays.asList(deps));
            return this;
        }

        public SharedLibrary build() {
            return mLibrary;
        }
    }
}

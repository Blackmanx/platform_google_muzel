/*
 * Copyright (C) 2022 The Android Open Source Project
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

package com.android.server.art;

import static com.android.server.art.testing.TestDataHelper.newPackageState;
import static com.android.server.art.testing.TestDataHelper.newSplit;
import static com.android.server.art.testing.TestDataHelper.newUserState;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyBoolean;
import static org.mockito.Mockito.argThat;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.lenient;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.same;

import android.os.CancellationSignal;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.UserManager;
import android.os.storage.StorageManager;

import com.android.modules.utils.pm.PackageStateModulesUtils;
import com.android.server.art.DexUseManagerLocal.DexLoader;
import com.android.server.art.model.Config;
import com.android.server.art.testing.StaticMockitoRule;
import com.android.server.pm.PackageManagerLocal.FilteredSnapshot;
import com.android.server.pm.pkg.AndroidPackage;
import com.android.server.pm.pkg.AndroidPackageSplit;
import com.android.server.pm.pkg.PackageState;
import com.android.server.pm.pkg.PackageUserState;

import dalvik.system.PathClassLoader;

import org.junit.Before;
import org.junit.Rule;
import org.mockito.Mock;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ThreadPoolExecutor;

public class PrimaryDexopterTestBase {
    protected static final String PKG_NAME = "com.example.foo";
    protected static final int UID = 12345;
    protected static final int SHARED_GID = UserHandle.getSharedAppGid(UID);
    protected static final long ART_VERSION = 331413030l;
    protected static final String APP_VERSION_NAME = "12.34.56";
    protected static final long APP_VERSION_CODE = 1536036288l;

    @Rule
    public StaticMockitoRule mockitoRule = new StaticMockitoRule(SystemProperties.class,
            Constants.class, PackageStateModulesUtils.class, ArtStatsLog.class);

    @Mock protected PrimaryDexopter.Injector mInjector;
    @Mock protected FilteredSnapshot mSnapshot;
    @Mock protected IArtd mArtd;
    @Mock protected UserManager mUserManager;
    @Mock protected DexUseManagerLocal mDexUseManager;
    @Mock protected StorageManager mStorageManager;
    @Mock protected DexMetadataHelper.Injector mDexMetadataHelperInjector;
    @Mock protected ThreadPoolExecutor mReporterExecutor;
    protected PackageState mPkgState;
    protected AndroidPackage mPkg;
    protected PackageUserState mPkgUserStateNotInstalled;
    protected PackageUserState mPkgUserStateInstalled;
    protected CancellationSignal mCancellationSignal;
    protected Config mConfig;
    protected DexMetadataHelper mDexMetadataHelper;

    @Before
    public void setUp() throws Exception {
        mPkgUserStateNotInstalled = newUserState().setInstalled(false).build();
        mPkgUserStateInstalled = newUserState().setInstalled(true).build();
        mPkgState = createPackageState();
        mPkg = mPkgState.getAndroidPackage();
        mCancellationSignal = new CancellationSignal();
        mConfig = new Config();
        mDexMetadataHelper = new DexMetadataHelper(mDexMetadataHelperInjector);

        lenient().when(mInjector.getArtd()).thenReturn(mArtd);
        lenient().when(mInjector.isSystemUiPackage(any())).thenReturn(false);
        lenient().when(mInjector.isLauncherPackage(any())).thenReturn(false);
        lenient().when(mInjector.getUserManager()).thenReturn(mUserManager);
        lenient().when(mInjector.getDexUseManager()).thenReturn(mDexUseManager);
        lenient().when(mInjector.getStorageManager()).thenReturn(mStorageManager);
        lenient().when(mInjector.getArtVersion()).thenReturn(ART_VERSION);
        lenient().when(mInjector.getConfig()).thenReturn(mConfig);
        lenient().when(mInjector.getReporterExecutor()).thenReturn(mReporterExecutor);
        lenient().when(mInjector.getDexMetadataHelper()).thenReturn(mDexMetadataHelper);
        lenient().when(mInjector.isPreReboot()).thenReturn(false);

        lenient()
                .when(SystemProperties.get("dalvik.vm.systemuicompilerfilter"))
                .thenReturn("speed");
        lenient()
                .when(SystemProperties.getBoolean(eq("dalvik.vm.always_debuggable"), anyBoolean()))
                .thenReturn(false);
        lenient().when(SystemProperties.get("dalvik.vm.appimageformat")).thenReturn("lz4");
        lenient().when(SystemProperties.get("pm.dexopt.shared")).thenReturn("speed");

        // No ISA translation.
        lenient()
                .when(SystemProperties.get(argThat(arg -> arg.startsWith("ro.dalvik.vm.isa."))))
                .thenReturn("");

        lenient().when(Constants.getPreferredAbi()).thenReturn("arm64-v8a");
        lenient().when(Constants.getNative64BitAbi()).thenReturn("arm64-v8a");
        lenient().when(Constants.getNative32BitAbi()).thenReturn("armeabi-v7a");

        lenient()
                .when(mUserManager.getUserHandles(anyBoolean()))
                .thenReturn(List.of(UserHandle.of(0), UserHandle.of(1), UserHandle.of(2)));

        lenient().when(mDexUseManager.isPrimaryDexUsedByOtherApps(any(), any())).thenReturn(false);

        lenient().when(mStorageManager.getAllocatableBytes(any())).thenReturn(1l);

        // Set up the primary dex loaders to make sure that the secondary ISA is
        // used and dexopted when calling {@link Utils#getUsedPrimaryDexAbis()}.
        String loadingPkgName = "com.example.foo.1";
        Set<DexLoader> loaders =
                Set.of(DexLoader.create(loadingPkgName, true /* isolatedProcess */));
        lenient()
                .when(mDexUseManager.getPrimaryDexLoaders(eq(PKG_NAME), any() /* dexPath */))
                .thenReturn(loaders);
        PackageState state = newPackageState(loadingPkgName).setAbi("armeabi-v7a").build();
        lenient().when(mSnapshot.getPackageState(eq(loadingPkgName))).thenReturn(state);
    }

    private PackageState createPackageState() {
        return newPackageState(PKG_NAME)
                .setAbis("arm64-v8a", "armeabi-v7a")
                .setAppId(UID)
                .setUserStateForAny(mPkgUserStateNotInstalled)
                .setLoadableInOtherProcesses(false)
                .setTargetSdkVersion(123)
                .setVersionName(APP_VERSION_NAME)
                .setLongVersionCode(APP_VERSION_CODE)
                .addSplit(newSplit()
                                .setPath("/somewhere/app/foo/base.apk")
                                .setHasCode(true)
                                .setClassLoaderName(PathClassLoader.class.getName())
                                .build())
                .addSplit(newSplit()
                                .setName("split_0")
                                .setPath("/somewhere/app/foo/split_0.apk")
                                .setHasCode(true)
                                .build())
                .addSplit(newSplit()
                                .setName("split_1")
                                .setPath("/somewhere/app/foo/split_1.apk")
                                .setHasCode(false)
                                .build())
                .build();
    }

    protected GetDexoptNeededResult dexoptIsNotNeeded() {
        return dexoptIsNotNeeded(true /* hasDexCode */);
    }

    protected GetDexoptNeededResult dexoptIsNotNeeded(boolean hasDexCode) {
        var result = new GetDexoptNeededResult();
        result.isDexoptNeeded = false;
        result.hasDexCode = hasDexCode;
        return result;
    }

    protected GetDexoptNeededResult dexoptIsNeeded() {
        return dexoptIsNeeded(ArtifactsLocation.NONE_OR_ERROR);
    }

    protected GetDexoptNeededResult dexoptIsNeeded(@ArtifactsLocation int location) {
        var result = new GetDexoptNeededResult();
        result.isDexoptNeeded = true;
        result.artifactsLocation = location;
        if (location != ArtifactsLocation.NONE_OR_ERROR) {
            result.isVdexUsable = true;
        }
        result.hasDexCode = true;
        return result;
    }

    protected ArtdDexoptResult createArtdDexoptResult(boolean cancelled, long wallTimeMs,
            long cpuTimeMs, long sizeBytes, long sizeBeforeBytes) {
        var result = new ArtdDexoptResult();
        result.cancelled = cancelled;
        result.wallTimeMs = wallTimeMs;
        result.cpuTimeMs = cpuTimeMs;
        result.sizeBytes = sizeBytes;
        result.sizeBeforeBytes = sizeBeforeBytes;
        return result;
    }

    protected ArtdDexoptResult createArtdDexoptResult(boolean cancelled) {
        return createArtdDexoptResult(cancelled, 0 /* wallTimeMs */, 0 /* cpuTimeMs */,
                0 /* sizeBytes */, 0 /* sizeBeforeBytes */);
    }
}

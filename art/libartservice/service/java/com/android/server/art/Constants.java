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

import android.annotation.NonNull;
import android.annotation.Nullable;
import android.content.res.Resources;
import android.content.res.XmlResourceParser;
import android.os.Build;
import android.os.SystemProperties;
import android.system.Os;

import androidx.annotation.RequiresApi;

import com.android.internal.annotations.GuardedBy;
import com.android.internal.annotations.VisibleForTesting;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

/**
 * A mockable wrapper class for device-specific constants.
 *
 * @hide
 */
@RequiresApi(Build.VERSION_CODES.UPSIDE_DOWN_CAKE)
public class Constants {
    private Constants() {}

    @GuardedBy("Constants.class") private static Set<String> sWebviewPackageNamesCache;

    /** Returns the ABI that the device prefers. */
    @NonNull
    public static String getPreferredAbi() {
        return Build.SUPPORTED_ABIS[0];
    }

    /** Returns the 64 bit ABI that is native to the device. */
    @Nullable
    public static String getNative64BitAbi() {
        // The value comes from "ro.product.cpu.abilist64" and we assume that the first element is
        // the native one.
        return Build.SUPPORTED_64_BIT_ABIS.length > 0 ? Build.SUPPORTED_64_BIT_ABIS[0] : null;
    }

    /** Returns the 32 bit ABI that is native to the device. */
    @Nullable
    public static String getNative32BitAbi() {
        // The value comes from "ro.product.cpu.abilist32" and we assume that the first element is
        // the native one.
        return Build.SUPPORTED_32_BIT_ABIS.length > 0 ? Build.SUPPORTED_32_BIT_ABIS[0] : null;
    }

    @Nullable
    public static String getenv(@NonNull String name) {
        return Os.getenv(name);
    }

    public static boolean isBootImageProfilingEnabled() {
        boolean profileBootClassPath = SystemProperties.getBoolean(
                "persist.device_config.runtime_native_boot.profilebootclasspath",
                SystemProperties.getBoolean("dalvik.vm.profilebootclasspath", false /* def */));
        return Build.isDebuggable() && profileBootClassPath;
    }

    /**
     * Returns the set of webview package names from WebViewProviders config file.
     */
    public static Set<String> getWebviewPackageNames() {
        if (sWebviewPackageNamesCache == null) {
            synchronized (Constants.class) {
                if (sWebviewPackageNamesCache == null) {
                    Resources resources = Resources.getSystem();
                    try (XmlResourceParser parser = resources.getXml(resources.getIdentifier(
                                 "config_webview_packages", "xml", "android"))) {
                        sWebviewPackageNamesCache = getWebviewPackageNames(parser);
                    } catch (IOException e) {
                        AsLog.e("Failed to get webview package names", e);
                    } catch (XmlPullParserException e) {
                        AsLog.wtf("Failed to parse webview packages config", e);
                    }
                }
            }
        }
        return sWebviewPackageNamesCache;
    }

    @VisibleForTesting
    public static Set<String> getWebviewPackageNames(XmlPullParser parser)
            throws XmlPullParserException, IOException {
        Set<String> packageNames = new HashSet<>();
        int type;
        while ((type = parser.next()) != XmlPullParser.END_DOCUMENT) {
            if (type == XmlPullParser.START_TAG) {
                if ("webviewprovider".equals(parser.getName())) {
                    String packageName =
                            parser.getAttributeValue(null /* namespace */, "packageName");
                    if (packageName != null) {
                        packageNames.add(packageName);
                    }
                }
            }
        }
        return Collections.unmodifiableSet(packageNames);
    }
}

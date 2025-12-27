/*
 * Copyright (C) 2015 The Android Open Source Project
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


package libcore.net;

@SuppressWarnings({"unchecked", "deprecation", "all"})
public abstract class NetworkSecurityPolicy {

public NetworkSecurityPolicy() { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_NETWORK_SECURITY_POLICY_REASON_CT_ENABLED_API)
public int getCertificateTransparencyVerificationReason(String hostname) { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_NETWORK_SECURITY_POLICY_ECH_API)
public int getDomainEncryptionMode(String hostname) { throw new RuntimeException("Stub!"); }

public static libcore.net.NetworkSecurityPolicy getInstance() { throw new RuntimeException("Stub!"); }

public abstract boolean isCertificateTransparencyVerificationRequired(String hostname);

public abstract boolean isCleartextTrafficPermitted();

public abstract boolean isCleartextTrafficPermitted(String hostname);

public static void setInstance(libcore.net.NetworkSecurityPolicy policy) { throw new RuntimeException("Stub!"); }

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_NETWORK_SECURITY_POLICY_REASON_CT_ENABLED_API)
public static final int CERTIFICATE_TRANSPARENCY_REASON_APP_OPT_IN = 2;

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_NETWORK_SECURITY_POLICY_REASON_CT_ENABLED_API)
public static final int CERTIFICATE_TRANSPARENCY_REASON_DOMAIN_OPT_IN = 3;

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_NETWORK_SECURITY_POLICY_REASON_CT_ENABLED_API)
public static final int CERTIFICATE_TRANSPARENCY_REASON_SDK_TARGET_DEFAULT_ENABLED = 1;

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_NETWORK_SECURITY_POLICY_REASON_CT_ENABLED_API)
public static final int CERTIFICATE_TRANSPARENCY_REASON_UNKNOWN = 0;

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_NETWORK_SECURITY_POLICY_ECH_API)
public static final int DOMAIN_ENCRYPTION_MODE_DISABLED = 1;

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_NETWORK_SECURITY_POLICY_ECH_API)
public static final int DOMAIN_ENCRYPTION_MODE_REQUIRED = 4;

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_NETWORK_SECURITY_POLICY_ECH_API)
public static final int DOMAIN_ENCRYPTION_MODE_OPPORTUNISTIC = 2;

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_NETWORK_SECURITY_POLICY_ECH_API)
public static final int DOMAIN_ENCRYPTION_MODE_ENABLED = 3;

@android.annotation.FlaggedApi(com.android.libcore.Flags.FLAG_NETWORK_SECURITY_POLICY_ECH_API)
public static final int DOMAIN_ENCRYPTION_MODE_UNKNOWN = 0;
}
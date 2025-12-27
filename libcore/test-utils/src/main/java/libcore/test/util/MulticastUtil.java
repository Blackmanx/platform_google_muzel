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

package libcore.test.util;

import java.io.IOException;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Enumeration;

public final class MulticastUtil {

  private MulticastUtil() {}

  public static Capabilities getMulticastCapabilities() throws IOException {

        // Determine if the device is marked to support multicast or not. If this property is not
        // set we assume the device has an interface capable of supporting multicast.
        boolean supportsMulticast = Boolean.parseBoolean(
                System.getProperty("android.cts.device.multicast", "true"));
        if (!supportsMulticast) {
            return new Capabilities(false, null, null);
        }

        Enumeration<NetworkInterface> interfaces = NetworkInterface.getNetworkInterfaces();

        NetworkInterface ipv4NetworkInterface = null;
        NetworkInterface ipv6NetworkInterface = null;

        while (interfaces.hasMoreElements()
                && (ipv4NetworkInterface == null || ipv6NetworkInterface == null)) {
            NetworkInterface nextInterface = interfaces.nextElement();
            if (willWorkForMulticast(nextInterface)) {
                Enumeration<InetAddress> addresses = nextInterface.getInetAddresses();
                while (addresses.hasMoreElements()) {
                    final InetAddress nextAddress = addresses.nextElement();
                    if (nextAddress instanceof Inet6Address && ipv6NetworkInterface == null) {
                        ipv6NetworkInterface = nextInterface;
                    } else if (nextAddress instanceof Inet4Address
                            && ipv4NetworkInterface == null) {
                        ipv4NetworkInterface = nextInterface;
                    }
                }
            }
        }

        return new Capabilities(true, ipv4NetworkInterface, ipv6NetworkInterface);
  }

  private static boolean willWorkForMulticast(NetworkInterface iface) throws IOException {
      return iface.isUp()
          // Typically loopback interfaces do not support multicast, but they are ruled out
          // explicitly here anyway.
          && !iface.isLoopback() && iface.supportsMulticast()
          && iface.getInetAddresses().hasMoreElements();
  }

  public record Capabilities(boolean supportsMulticast,
          NetworkInterface ipv4NetworkInterface,
          NetworkInterface ipv6NetworkInterface) { }
}

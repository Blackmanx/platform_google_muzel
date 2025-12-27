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
package libcore.android.crypto.hpke;

import static android.crypto.hpke.AeadParameterSpec.AES_128_GCM;
import static android.crypto.hpke.AeadParameterSpec.AES_256_GCM;
import static android.crypto.hpke.AeadParameterSpec.CHACHA20POLY1305;
import static android.crypto.hpke.KdfParameterSpec.HKDF_SHA256;
import static android.crypto.hpke.KemParameterSpec.DHKEM_X25519_HKDF_SHA256;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertThrows;

import android.crypto.hpke.AeadParameterSpec;
import android.crypto.hpke.Hpke;
import android.crypto.hpke.Message;
import android.crypto.hpke.Recipient;
import android.crypto.hpke.Sender;

import libcore.util.NonNull;

import org.junit.Before;
import org.junit.Test;
import org.junit.experimental.runners.Enclosed;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameter;
import org.junit.runners.Parameterized.Parameters;

import java.nio.charset.StandardCharsets;
import java.security.InvalidKeyException;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.PrivateKey;
import java.security.Provider;
import java.security.PublicKey;
import java.security.Security;
import java.util.List;

@RunWith(Enclosed.class)
public class HpkeTest {

    @RunWith(Parameterized.class)
    public static class SendReceiveTests {
        private PublicKey publicKey;
        private PrivateKey privateKey;

        private static final byte[] EMPTY = new byte[0];
        private static final byte[] MESSAGE = "This is only a test".getBytes(StandardCharsets.US_ASCII);
        private static final byte[] INFO = "App info".getBytes(StandardCharsets.US_ASCII);
        private static final byte[] AAD = "Additional data".getBytes(StandardCharsets.US_ASCII);

        @Parameters
        public static Object[][] data() {
            Object[] aads = new Object[]{null, EMPTY, AAD};
            Object[] infos = new Object[]{null, EMPTY, INFO};
            Object[] messages = new Object[]{EMPTY, MESSAGE};
            Object[][] ciphers = new Object[][]{
                    {AES_128_GCM},
                    {AES_256_GCM},
                    {CHACHA20POLY1305}
            };
            return permute(aads, permute(infos, permute(messages, ciphers)));
        }

        @Parameter()
        public byte[] aad;

        @Parameter(1)
        public byte[] info;

        @Parameter(2)
        public byte[] plaintext;

        @Parameter(3)
        public AeadParameterSpec aead;

        private String suiteName;

        @Before
        public void before() throws Exception {
            KeyPairGenerator generator = KeyPairGenerator.getInstance("XDH");
            KeyPair pair = generator.generateKeyPair();
            publicKey = pair.getPublic();
            privateKey = pair.getPrivate();
            suiteName = Hpke.getSuiteName(DHKEM_X25519_HKDF_SHA256, HKDF_SHA256, aead);
            assertNotNull(suiteName);
        }

        @Test
        public void sendMessage() throws Exception {
            Hpke hpke = Hpke.getInstance(suiteName);
            assertNotNull(hpke);
            Sender.Builder senderBuilder = new Sender.Builder(hpke, publicKey);
            if (info != null) {
                senderBuilder.setApplicationInfo(info);
            }
            Sender sender = senderBuilder.build();
            byte[] ciphertext = sender.seal(plaintext, aad);
            byte[] encapsulated = sender.getEncapsulated();
            assertNotNull(encapsulated);

            Recipient.Builder recipientBuilder =
                    new Recipient.Builder(hpke, encapsulated, privateKey);
            if (info != null) {
                recipientBuilder.setApplicationInfo(info);
            }
            Recipient recipient = recipientBuilder.build();
            byte[] decoded = recipient.open(ciphertext, aad);
            assertNotNull(decoded);

            assertArrayEquals(plaintext, decoded);
        }

        @Test
        public void oneshot() throws Exception {
            Hpke hpke = Hpke.getInstance(suiteName);
            Message message = hpke.seal(publicKey, info, plaintext, aad);
            byte[] decoded = hpke.open(privateKey, info, message, aad);
            assertArrayEquals(plaintext, decoded);
        }

        // Permute a new set of values into an existing Parameters array, i.e. one new row
        // is created for every combination of each new value and existing row.
        private static Object[][] permute(Object[] newValues, Object[][] existing) {
            int newSize = newValues.length * existing.length;
            int rowSize = existing[0].length + 1;
            Object[][] result = new Object[newSize][];
            for (int i = 0; i < newSize; i++) {
                Object[] row = new Object[rowSize];
                result[i] = row;
                row[0] = newValues[i % newValues.length];
                System.arraycopy(existing[i / newValues.length], 0, row, 1, rowSize - 1);
            }
            return result;
        }
    }

    @RunWith(JUnit4.class)
    public static class OtherTests {
        private static final String SUITE_NAME = "DHKEM_X25519_HKDF_SHA256/HKDF_SHA256/AES_128_GCM";
        private static final int EXPORT_LENGTH = 16;
        private static final String CONSCRYPT_NAME = "AndroidOpenSSL";
        private final Provider conscrypt = Security.getProvider(CONSCRYPT_NAME);

        private PublicKey publicKey;
        private PrivateKey privateKey;

        @Before
        public void before() throws Exception {
            KeyPairGenerator generator = KeyPairGenerator.getInstance("XDH");
            KeyPair pair = generator.generateKeyPair();
            publicKey = pair.getPublic();
            privateKey = pair.getPrivate();
        }

        @Test
        public void init_Errors() {
            assertThrows(NoSuchAlgorithmException.class,
                    () ->Hpke.getInstance("No such"));
            assertThrows(NoSuchAlgorithmException.class,
                    () ->Hpke.getInstance(""));
            assertThrows(NoSuchAlgorithmException.class,
                    () ->Hpke.getInstance(null));

            assertThrows(IllegalArgumentException.class,
                    () ->Hpke.getInstance(SUITE_NAME, (String) null));
            assertThrows(IllegalArgumentException.class,
                    () ->Hpke.getInstance(SUITE_NAME, ""));
            assertThrows(NoSuchProviderException.class,
                    () ->Hpke.getInstance(SUITE_NAME, "No such"));
            assertThrows(NoSuchAlgorithmException.class,
                    () ->Hpke.getInstance("No such", CONSCRYPT_NAME));
            assertThrows(NoSuchAlgorithmException.class,
                    () ->Hpke.getInstance("", CONSCRYPT_NAME));
            assertThrows(NoSuchAlgorithmException.class,
                    () ->Hpke.getInstance(null, CONSCRYPT_NAME));

            assertThrows(IllegalArgumentException.class,
                    () ->Hpke.getInstance(SUITE_NAME, (Provider) null));
            assertThrows(NoSuchAlgorithmException.class,
                    () ->Hpke.getInstance("No such", conscrypt));
            assertThrows(NoSuchAlgorithmException.class,
                    () ->Hpke.getInstance("", conscrypt));
            assertThrows(NoSuchAlgorithmException.class,
                    () ->Hpke.getInstance(null, conscrypt));
        }

        @Test
        public void keyType() throws Exception {
            KeyPairGenerator generator = KeyPairGenerator.getInstance("RSA");
            KeyPair pair = generator.generateKeyPair();
            PublicKey publicRsa = pair.getPublic();
            PrivateKey privateRsa = pair.getPrivate();
            Hpke hpke = Hpke.getInstance(SUITE_NAME);

            assertThrows(InvalidKeyException.class,
                    () -> new Sender.Builder(hpke, publicRsa).build());
            assertThrows(InvalidKeyException.class,
                    () -> new Recipient.Builder(hpke, new byte[16], privateRsa).build());
        }

        @Test
        public void suiteNames() throws Exception {
            List<AeadParameterSpec> aeads = List.of(AES_128_GCM, AES_256_GCM, CHACHA20POLY1305);
            for (AeadParameterSpec aead : aeads) {
                String suiteName
                        = Hpke.getSuiteName(DHKEM_X25519_HKDF_SHA256, HKDF_SHA256, aead);
                assertNotNull(suiteName);
                assertNotNull(Hpke.getInstance(suiteName));
                // Also check Tink-compatible names
                // TODO(prb) enable after https://github.com/google/conscrypt/pull/1258 lands
                // String altName = suiteName.replaceAll("/", "_");
                // assertNotNull(Hpke.getInstance(altName));
            }
        }

        // Note API test only, implementation tests are in Conscrypt.
        @Test
        public void export() throws Exception {
            byte[] context = "Hello".getBytes(StandardCharsets.UTF_8);
            Hpke hpke = Hpke.getInstance(SUITE_NAME);
            assertNotNull(hpke);
            Sender sender = new Sender.Builder(hpke, publicKey).build();
            Recipient recipient =
                    new Recipient.Builder(hpke, sender.getEncapsulated(), privateKey).build();
            assertNotNull(recipient);

            byte[] senderData = sender.export(EXPORT_LENGTH, context);
            assertNotNull(senderData);
            assertEquals(EXPORT_LENGTH, senderData.length);
            int sum = 0;
            for (byte b : senderData) {
                sum += b;
            }
            // Check data isn't all zeroes.
            assertNotEquals(0, sum);

            byte[] recipientData = recipient.export(EXPORT_LENGTH, context);
            assertArrayEquals(senderData, recipientData);
        }

        @Test
        public void spiAndProvider() throws Exception{
            Hpke hpke = Hpke.getInstance(SUITE_NAME);
            assertNotNull(hpke);
            assertNotNull(hpke.getProvider());

            Sender sender = new Sender.Builder(hpke, publicKey).build();
            Recipient recipient =
                    new Recipient.Builder(hpke, sender.getEncapsulated(), privateKey).build();
            assertNotNull(recipient);

            assertNotNull(sender.getProvider());
            assertNotNull(sender.getSpi());
            assertNotNull(recipient.getProvider());
            assertNotNull(recipient.getSpi());
        }

        // Note API test only.  Implementation not yet present.
        @Test
        public void futureBuilderMethods() throws Exception {
            byte[] appInfo = "App Info".getBytes(StandardCharsets.UTF_8);
            byte[] psk = "Very Secret Key".getBytes(StandardCharsets.UTF_8);
            byte[] pskId = "ID".getBytes(StandardCharsets.UTF_8);

            Hpke hpke = Hpke.getInstance(SUITE_NAME);
            assertNotNull(hpke);

            Sender.Builder senderBuilder = new Sender.Builder(hpke, publicKey)
                    .setApplicationInfo(appInfo)
                    .setSenderKey(privateKey)
                    .setPsk(psk, pskId);
            assertThrows(UnsupportedOperationException.class, senderBuilder::build);

            Sender sender = new Sender.Builder(hpke, publicKey).build();
            assertNotNull(sender);

            Recipient.Builder recipientBuilder =
                    new Recipient.Builder(hpke, sender.getEncapsulated(), privateKey)
                            .setApplicationInfo(appInfo)
                            .setSenderKey(publicKey)
                            .setPsk(psk, pskId);
            assertThrows(UnsupportedOperationException.class, recipientBuilder::build);
        }
    }
}
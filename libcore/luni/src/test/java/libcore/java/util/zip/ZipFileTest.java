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

package libcore.java.util.zip;

import libcore.io.ClassPathURLStreamHandler;
import libcore.libcore.io.ClassPathURLStreamHandlerTest;
import libcore.test.annotation.NonCts;
import libcore.test.reasons.NonCtsReasons;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.Charset;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;
import java.util.zip.ZipOutputStream;

public final class ZipFileTest extends AbstractZipFileTest {

    @Override
    protected ZipOutputStream createZipOutputStream(OutputStream wrapped) {
        return new ZipOutputStream(wrapped);
    }

    // b/31077136
    @NonCts(bug = 338503591, reason = NonCtsReasons.NON_BREAKING_BEHAVIOR_FIX)
    public void test_FileNotFound() throws Exception {
        File nonExistentFile = new File("fileThatDefinitelyDoesntExist.zip");
        assertFalse(nonExistentFile.exists());

        try (ZipFile zipFile = new ZipFile(nonExistentFile, ZipFile.OPEN_READ)) {
            fail();
        } catch(IOException expected) {}
    }

    /**
     * cp1251.zip archive has single empty file with cp1251 encoding name.
     * Its name is 'имя файла'('file name' in Russian), but in cp1251.
     * It was created using "convmv -f utf-8 -t cp1251 &lt;file&gt; --notest".
     */
    public void test_zipFileWith_cp1251_fileNames() throws Exception {
        String resourceName = "/libcore/java/util/zip/cp1251.zip";

        File zipWithCp1251 = readResource(resourceName);

        Charset cp1251 = Charset.forName("cp1251");
        try (ZipFile zipFile = new ZipFile(zipWithCp1251, cp1251)) {
            ZipEntry zipEntry = zipFile.entries().nextElement();

            assertEquals("имя файла", zipEntry.getName());
        }

        try (ZipFile zipFile = new ZipFile(zipWithCp1251.getAbsolutePath(), cp1251)) {
            ZipEntry zipEntry = zipFile.entries().nextElement();

            assertEquals("имя файла", zipEntry.getName());
        }
    }

    // Regression test for http://b/425897917 and http://b/418765270.
    public void test_zipFile_notAffectedByInterrupt() throws Exception {
        String jarName = "/tests/resources/ClassPathURLStreamHandlerTest.jar";
        File file = readResource(jarName);
        String expected_file = ClassPathURLStreamHandlerTest.ENTRY_STORED;
        Thread thread = Thread.currentThread();

        new Thread(thread::interrupt).start();
        while (!thread.isInterrupted()) {}
        try (ZipFile zipFile = new ZipFile(file)) {
            assertNotNull("Can't open " + expected_file,  zipFile.getEntry(expected_file));
            assertTrue(thread.isInterrupted());
        } finally {
            // Clear interrupted status
            Thread.interrupted();
        }

        new Thread(thread::interrupt).start();
        while (!thread.isInterrupted()) {}
        try (HandlerWrapper wrapper = new HandlerWrapper(
                new ClassPathURLStreamHandler(file.getPath(), false))) {
            assertTrue(wrapper.handler().isEntryStored(expected_file));
            assertNotNull("Can't open " + expected_file,
                    wrapper.handler().getEntryUrlOrNull(expected_file));
            assertTrue(thread.isInterrupted());
        } finally {
            // Clear interrupted status
            Thread.interrupted();
        }
}

    record HandlerWrapper(ClassPathURLStreamHandler handler) implements AutoCloseable {
        @Override
        public void close() throws Exception {
            handler.close();
        }
    }


    public void test_throwsWhenTriesToOpenCorruptedFile() throws Exception {
        // This file was by running following commands:
        // echo foo > a.txt
        // zip a.zip a.txt
        // vim --noplugin a.zip
        // :%!xxd in vim to enter into hex mode
        // Changed 3rd and 4th bytes into FFFF
        // :%!xxd -r to write back
        String resourceName = "/libcore/java/util/zip/Corrupted.zip";

        File corruptedZip = readResource(resourceName);
        try {
            new ZipFile(corruptedZip);
            fail("ZipException was expected");
        } catch (ZipException expected) {
            String msg = "Expected exception to contain \"invalid LFH\". Exception was: " +
                expected;
            assertTrue(msg, expected.getMessage().contains("invalid LFH"));
        }
    }

    public void test_throwsWhenTriesToOpen_nonEmptyFileWhichStartsWithEndHeader() throws Exception {
        // This file was by running following commands:
        // echo foo > a.txt
        // zip a.zip a.txt
        // vim --noplugin a.zip
        // :%!xxd in vim to enter into hex mode
        // Changed 3rd and 4th bytes into 0506
        // :%!xxd -r to write back
        String resourceName = "/libcore/java/util/zip/Corrupted2.zip";

        File corruptedZip = readResource(resourceName);
        try {
            new ZipFile(corruptedZip);
            fail("ZipException was expected");
        } catch (ZipException expected) {}
    }

    /* Reads resource and stores its content to a temp file. */
    private static File readResource(String resourceName) throws Exception {
        File tempFile = createTemporaryZipFile();
        try (InputStream is = ZipFileTest.class.getResourceAsStream(resourceName);
             FileOutputStream fos = new FileOutputStream(tempFile)) {

            int read;
            byte[] arr = new byte[1024];

            while ((read = is.read(arr)) >= 0) {
                fos.write(arr, 0, read);
            }
            fos.flush();

            return tempFile;
        }
    }
}

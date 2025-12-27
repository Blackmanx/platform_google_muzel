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
 * limitations under the License
 */

package com.android.server.art;

import static com.google.common.truth.Truth.assertThat;

import android.content.res.XmlResourceParser;
import android.util.Xml;

import androidx.test.filters.SmallTest;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.junit.MockitoJUnitRunner;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.StringReader;
import java.util.Set;

@SmallTest
@RunWith(MockitoJUnitRunner.StrictStubs.class)
public final class ConstantsTest {

    private XmlPullParser createMockParser(String xmlContent)
            throws XmlPullParserException, IOException {
        XmlPullParser parser = Xml.newPullParser();
        parser.setInput(new StringReader(xmlContent));
        return parser;
    }

    @Test
    public void testGetWebviewPackageNames_singleProvider() throws Exception {
        String xml = "<webviewproviders>\n"
                + "    <webviewprovider description=\"Android WebView\" "
                + "packageName=\"com.android.webview\" availableByDefault=\"true\">\n"
                + "    </webviewprovider>\n"
                + "</webviewproviders>";
        XmlPullParser parser = createMockParser(xml);

        Set<String> packageNames = Constants.getWebviewPackageNames(parser);
        assertThat(packageNames).containsExactly("com.android.webview");
    }

    @Test
    public void testGetWebviewPackageNames_multipleProviders() throws Exception {
        String xml = "<webviewproviders>\n"
                + "    <webviewprovider description=\"Android WebView\" "
                + "packageName=\"com.android.webview\" availableByDefault=\"true\">\n"
                + "    </webviewprovider>\n"
                + "    <webviewprovider description=\"Google Chrome\" "
                + "packageName=\"com.chrome.webview\" availableByDefault=\"false\">\n"
                + "    </webviewprovider>\n"
                + "</webviewproviders>";
        XmlPullParser parser = createMockParser(xml);

        Set<String> packageNames = Constants.getWebviewPackageNames(parser);
        assertThat(packageNames).containsExactly("com.android.webview", "com.chrome.webview");
    }

    @Test
    public void testGetWebviewPackageNames_noProviders() throws Exception {
        String xml = "<webviewproviders>\n"
                + "</webviewproviders>";
        XmlPullParser parser = createMockParser(xml);

        Set<String> packageNames = Constants.getWebviewPackageNames(parser);
        assertThat(packageNames).isEmpty();
    }

    @Test
    public void testGetWebviewPackageNames_providerWithoutPackageName() throws Exception {
        String xml = "<webviewproviders>\n"
                + "    <webviewprovider description=\"Android WebView\" "
                + "availableByDefault=\"true\">\n"
                + "    </webviewprovider>\n"
                + "</webviewproviders>";
        XmlPullParser parser = createMockParser(xml);

        Set<String> packageNames = Constants.getWebviewPackageNames(parser);
        assertThat(packageNames).isEmpty();
    }

    @Test
    public void testGetWebviewPackageNames_emptyXml() throws Exception {
        XmlPullParser parser = createMockParser("");

        Set<String> packageNames = Constants.getWebviewPackageNames(parser);
        assertThat(packageNames).isEmpty();
    }
}
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

package libcore.sun.util.locale;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.lang.reflect.Field;
import java.util.concurrent.ConcurrentMap;

import sun.util.locale.BaseLocale;
import sun.util.locale.LocaleObjectCache;

@RunWith(JUnit4.class)
public class BaseLocaleTest {
    @Test
    public void testCache_holdingNoStrongReferences() throws ReflectiveOperationException {
        // We use reflection to avoid making the fields public solely for testing purposes.
        // If the names of fields are changed, please update the test accordingly.
        Field globalCacheField = BaseLocale.Cache.class.getDeclaredField("CACHE");
        globalCacheField.setAccessible(true);

        BaseLocale.Cache cache = (BaseLocale.Cache) globalCacheField.get(null);
        Field mapField = LocaleObjectCache.class.getDeclaredField("map");
        mapField.setAccessible(true);
        ConcurrentMap<BaseLocale.Key, BaseLocale> map =
                (ConcurrentMap<BaseLocale.Key, BaseLocale>) mapField.get(cache);


        Field holderField = BaseLocale.Key.class.getDeclaredField("holder");
        holderField.setAccessible(true);

        for (BaseLocale.Key key : map.keySet()) {
            // The strong references by the holder field should always be null.
            Object baseLocale = holderField.get(key);
            assertNull(baseLocale);
        }
    }

    static class TestCache extends BaseLocale.Cache {
        @Override
        protected BaseLocale createObject(BaseLocale.Key key) {
            // TODO(http://b/348646292): Replace gc() with a reliable way collecting SoftReference.
            Runtime.getRuntime().gc();
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
            return super.createObject(key);
        }
    }

    /**
     * Regression test for http://b/348646292.
     */
    @Test
    public void testCacheCreateObject_notNull() {
        BaseLocale.Cache cache = new TestCache();
        for (int i = 0; i < 26 * 26 - 1; i++) {
            String lang = createLanguageCode(i);
            String region = createRegion(i);
            BaseLocale.Key key = new BaseLocale.Key(lang, "", region, "", false);
            BaseLocale b = cache.get(key);
            assertNotNull(b);
            assertEquals(lang, b.getLanguage());
            assertEquals(region, b.getRegion());
        }
    }

    private static String createLanguageCode(int i) {
        int q = i / 26;
        int r = i % 26;
        return new String(new char[] {(char) ('a' + q), (char) ('a' +  r)});
    }

    private static String createRegion(int i) {
        int q = i / 26;
        int r = i % 26;
        return new String(new char[] {(char) ('A' + q), (char) ('A' +  r)});
    }
}

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

package libcore.libcore.internal;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import java.util.Arrays;
import libcore.internal.Java21LanguageFeatures;

import org.junit.Test;
import org.junit.Ignore;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class Java21LanguageFeaturesTest {

    @Test
    public void testPatternMatchingSwitch() {
        Java21LanguageFeatures.Shape s = new Java21LanguageFeatures.Triangle(6, 10);
        assertEquals(30, Java21LanguageFeatures.calculateApproximateArea(s));

        s = new Java21LanguageFeatures.Rectangle(4, 5);
        assertEquals(20, Java21LanguageFeatures.calculateApproximateArea(s));

        s = new Java21LanguageFeatures.Circle(5);
        assertEquals(75, Java21LanguageFeatures.calculateApproximateArea(s));

        s = new Blob();
        assertEquals(0, Java21LanguageFeatures.calculateApproximateArea(s));
    }

    private static class Blob extends Java21LanguageFeatures.Shape {
    }

    @Test
    public void testPatternMatchingSwitchWithNull() {
        assertEquals("404", Java21LanguageFeatures.isDroid(null));
        assertEquals("Yes", Java21LanguageFeatures.isDroid("Android"));
        assertEquals("Yes", Java21LanguageFeatures.isDroid("Marvin"));
        assertEquals("No", Java21LanguageFeatures.isDroid("Snoopy"));
    }

    @Test
    public void testPatternMatchingSwitchCaseRefinement() {
        assertEquals("404", Java21LanguageFeatures.isDroidIgnoreCase(null));
        assertEquals("Yes", Java21LanguageFeatures.isDroidIgnoreCase("Android"));
        assertEquals("Yes", Java21LanguageFeatures.isDroidIgnoreCase("MARVIN"));
        assertEquals("No", Java21LanguageFeatures.isDroidIgnoreCase("Snoopy"));
    }

    @Test
    public void testPatternMatchingSwitchWithEnum() {
        assertFalse(Java21LanguageFeatures.hasManySides(
                Java21LanguageFeatures.PointyShapeType.TRIANGLE));
        assertTrue(Java21LanguageFeatures.hasManySides(
                Java21LanguageFeatures.PointyShapeType.RECTANGLE));
        assertFalse(Java21LanguageFeatures.hasManySides(
                Java21LanguageFeatures.RoundedShapeType.CIRCLE));
    }

    @Test
    public void testRecordPatterns() {
        assertEquals(5, Java21LanguageFeatures.getX(new Java21LanguageFeatures.Point(5, 10)));
        assertEquals(0, Java21LanguageFeatures.getX(Integer.valueOf(5)));
    }

    @Test
    public void testRecordPatternsWithTypeInference() {
        assertEquals(10, Java21LanguageFeatures.getY(new Java21LanguageFeatures.Point(5, 10)));
        assertEquals(0, Java21LanguageFeatures.getY(Integer.valueOf(10)));
    }

    @Test
    public void testNestedRecordPatterns() {
        final var pointOne = new Java21LanguageFeatures.Point(5, 10);
        final var pointTwo = new Java21LanguageFeatures.Point(5, 20);
        final var pointThree = new Java21LanguageFeatures.Point(10, 20);

        var line = new Java21LanguageFeatures.Line(pointOne, pointTwo);
        assertTrue(Java21LanguageFeatures.isLineVertical(line));

        line = new Java21LanguageFeatures.Line(pointOne, pointThree);
        assertFalse(Java21LanguageFeatures.isLineVertical(line));

        assertFalse(Java21LanguageFeatures.isLineVertical(Integer.valueOf(5)));
    }

    @Test
    public void testSwitchRecordPatterns() {
        final var five = new Java21LanguageFeatures.PairableInt(5);
        final var ten = new Java21LanguageFeatures.PairableInt(10);
        final var str = new Java21LanguageFeatures.PairableString("5");

        var pair = new Java21LanguageFeatures.Pair(five, ten);
        assertFalse(Java21LanguageFeatures.isFirstItemLarger(pair));

        pair = new Java21LanguageFeatures.Pair(ten, five);
        assertTrue(Java21LanguageFeatures.isFirstItemLarger(pair));

        pair = new Java21LanguageFeatures.Pair(str, five);
        assertFalse(Java21LanguageFeatures.isFirstItemLarger(pair));

        pair = new Java21LanguageFeatures.Pair(five, str);
        assertFalse(Java21LanguageFeatures.isFirstItemLarger(pair));
    }

    @Test
    public void testSwitchRecordPatternsTypeInference() {
        var pair = new Java21LanguageFeatures.AnyPair<Integer, Integer>(
                Integer.valueOf(2), Integer.valueOf(3));
        assertEquals(5, Java21LanguageFeatures.sumOfMembers(pair));
    }

    @Test
    public void testComplexSwitchPatterns() {
        Java21LanguageFeatures.Blob blob;
        Object result;

        blob = Java21LanguageFeatures.SquishyBlob.SMALL;
        result = Java21LanguageFeatures.getMainCharacteristic(blob);
        assertEquals(Java21LanguageFeatures.SquishyBlob.SMALL, result);

        blob = Java21LanguageFeatures.SquishyBlob.MEDIUM;
        result = Java21LanguageFeatures.getMainCharacteristic(blob);
        assertEquals(Java21LanguageFeatures.SquishyBlob.MEDIUM, result);

        blob = Java21LanguageFeatures.SquishyBlob.LARGE;
        result = Java21LanguageFeatures.getMainCharacteristic(blob);
        assertEquals(Java21LanguageFeatures.SquishyBlob.LARGE, result);

        blob = new Java21LanguageFeatures.MultiBlob(Java21LanguageFeatures.RoundedShapeType.CIRCLE,
                Java21LanguageFeatures.Color.RED, Integer.valueOf(5));
        result = Java21LanguageFeatures.getMainCharacteristic(blob);
        assertEquals(Java21LanguageFeatures.RoundedShapeType.CIRCLE, result);

        blob = new Java21LanguageFeatures.MultiBlob(Java21LanguageFeatures.PointyShapeType.TRIANGLE,
                Java21LanguageFeatures.Color.RED, Integer.valueOf(5));
        result = Java21LanguageFeatures.getMainCharacteristic(blob);
        assertEquals(Java21LanguageFeatures.Color.RED, result);

        blob = new Java21LanguageFeatures.MultiBlob(Java21LanguageFeatures.PointyShapeType.TRIANGLE,
                Java21LanguageFeatures.Color.BLUE, Integer.valueOf(5));
        result = Java21LanguageFeatures.getMainCharacteristic(blob);
        assertEquals(Integer.valueOf(5), result);

        blob = new Java21LanguageFeatures.MultiBlob(Java21LanguageFeatures.PointyShapeType.TRIANGLE,
                Java21LanguageFeatures.Color.BLUE, Integer.valueOf(10));
        result = Java21LanguageFeatures.getMainCharacteristic(blob);
        assertEquals(Integer.valueOf(10), result);

        blob = new Java21LanguageFeatures.GeometricBlob(20);
        result = Java21LanguageFeatures.getMainCharacteristic(blob);
        assertEquals(Integer.valueOf(20), result);

        blob = new Java21LanguageFeatures.GeometricBlob(2000);
        result = Java21LanguageFeatures.getMainCharacteristic(blob);
        assertEquals(Integer.valueOf(1000), result);
    }
}

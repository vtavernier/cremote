package com.vtavernier.cremote.models

import org.junit.Test

import org.junit.Assert.*

/**
 * Example local unit test, which will execute on the development machine (host).
 *
 * See [testing documentation](http://d.android.com/tools/testing).
 */
class StepDurationTest {
    val tests = arrayOf("1/500\"",
            "1/250\"",
            "1/125\"",
            "1/60\"",
            "1/30\"",
            "1/15\"",
            "1/8\"",
            "1/4\"",
            "1/2\"",
            "1\"",
            "2\"",
            "3\"",
            "4\"",
            "5\"",
            "6\"",
            "7\"",
            "8\"",
            "9\"",
            "10\"",
            "15\"",
            "20\"",
            "25\"",
            "30\"",
            "35\"",
            "40\"",
            "45\"",
            "50\"",
            "55\"",
            "60\"")

    @Test
    fun testStandard() {
        tests.forEach {
            assertNotNull(StepDuration.parse(it))
        }

        assertEquals(StepDuration.parse(tests[0])?.amount, 2.toShort())
    }

    @Test
    fun testRoundTrip() {
        tests.forEach {
            val parsed = StepDuration.parse(it)
            assertNotNull(it, parsed?.toShutterSpeed())
        }
    }
}

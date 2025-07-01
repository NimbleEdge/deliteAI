/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.common.utils

import dev.deliteai.impl.DependencyContainer
import dev.deliteai.impl.loggers.LocalLogger
import io.mockk.MockKAnnotations
import io.mockk.every
import io.mockk.mockk
import io.mockk.mockkObject
import io.mockk.verify
import junit.framework.Assert.assertEquals
import junit.framework.Assert.assertTrue
import org.json.JSONObject
import org.junit.Before
import org.junit.Test

class KotlinUtilsTest {
    private val localLoggerMocked: LocalLogger = mockk(relaxed = true)
    private val dependencyContainerMocked: DependencyContainer = mockk(relaxed = true)

    @Before
    fun setup() {
        MockKAnnotations.init(this)
        mockkObject(DependencyContainer)
        every { DependencyContainer.getInstance(any(), any()) } returns dependencyContainerMocked
        every { dependencyContainerMocked.getLocalLogger() } returns localLoggerMocked
    }

    @Test
    fun `jsonStringToHeaders should convert valid JSON array to Headers`() {
        val jsonString =
            """[{"Content-Type": "application/json"}, {"Authorization": "Bearer token"}]"""

        val result = jsonStringToHeaders(jsonString)

        assertEquals("application/json", result.get("Content-Type"))
        assertEquals("Bearer token", result.get("Authorization"))
    }

    @Test
    fun `jsonStringToHeaders should handle empty JSON array`() {
        val jsonString = "[]"

        val result = jsonStringToHeaders(jsonString)

        assertEquals(0, result.size)
    }

    @Test
    fun `jsonStringToHeaders should handle single header in JSON array`() {
        val jsonString = """[{"User-Agent": "NimbleNet/1.0"}]"""

        val result = jsonStringToHeaders(jsonString)

        assertEquals(1, result.size)
        assertEquals("NimbleNet/1.0", result.get("User-Agent"))
    }

    @Test
    fun `jsonStringToHeaders should handle multiple headers in single JSON object`() {
        val jsonString = """[{"Content-Type": "application/json", "Accept": "application/json"}]"""

        val result = jsonStringToHeaders(jsonString)

        assertEquals(2, result.size)
        assertEquals("application/json", result.get("Content-Type"))
        assertEquals("application/json", result.get("Accept"))
    }

    @Test
    fun `jsonStringToHeaders should handle duplicate headers by using lookupMap`() {
        val jsonString =
            """[{"Content-Type": "application/json"}, {"Content-Type": "application/json"}]"""

        val result = jsonStringToHeaders(jsonString)

        assertEquals(1, result.size)
        assertEquals("application/json", result.get("Content-Type"))
    }

    @Test
    fun `jsonStringToHeaders should handle different value types`() {
        val jsonString = """[{"X-Request-ID": 12345}, {"X-Enabled": true}]"""

        val result = jsonStringToHeaders(jsonString)

        assertEquals("12345", result.get("X-Request-ID"))
        assertEquals("true", result.get("X-Enabled"))
    }

    @Test
    fun `jsonStringToHeaders should handle invalid JSON gracefully and log error`() {
        val invalidJsonString = "invalid json"

        val result = jsonStringToHeaders(invalidJsonString)

        assertEquals(0, result.size)
        verify { localLoggerMocked.e(any<Exception>()) }
    }

    @Test
    fun `jsonStringToHeaders should handle null values in JSON`() {
        val jsonString = """[{"X-Test": null}]"""

        val result = jsonStringToHeaders(jsonString)

        assertEquals("null", result.get("X-Test"))
    }

    @Test
    fun `buildLibDownloadLogBody should create JSONObject with all parameters`() {
        val module = "test-module"
        val status = true
        val timeElapsed = 1500L

        val result = buildLibDownloadLogBody(module, status, timeElapsed)

        assertTrue(result is JSONObject)
        assertEquals(module, result.getString("module"))
        assertEquals(status, result.getBoolean("status"))
        assertEquals(timeElapsed, result.getLong("timeElapsed"))
    }

    @Test
    fun `buildLibDownloadLogBody should create JSONObject with module only`() {
        val module = "test-module"

        val result = buildLibDownloadLogBody(module)

        assertTrue(result is JSONObject)
        assertEquals(module, result.getString("module"))
        assertTrue(result.isNull("status"))
        assertTrue(result.isNull("timeElapsed"))
    }

    @Test
    fun `buildLibDownloadLogBody should create JSONObject with module and status only`() {
        val module = "test-module"
        val status = false

        val result = buildLibDownloadLogBody(module, status)

        assertTrue(result is JSONObject)
        assertEquals(module, result.getString("module"))
        assertEquals(status, result.getBoolean("status"))
        assertTrue(result.isNull("timeElapsed"))
    }

    @Test
    fun `buildLibDownloadLogBody should create JSONObject with module and timeElapsed only`() {
        val module = "test-module"
        val timeElapsed = 2000L

        val result = buildLibDownloadLogBody(module, timeElapsed = timeElapsed)

        assertTrue(result is JSONObject)
        assertEquals(module, result.getString("module"))
        assertTrue(result.isNull("status"))
        assertEquals(timeElapsed, result.getLong("timeElapsed"))
    }

    @Test
    fun `buildLibDownloadLogBody should handle empty module string`() {
        val module = ""
        val status = true
        val timeElapsed = 500L

        val result = buildLibDownloadLogBody(module, status, timeElapsed)

        assertTrue(result is JSONObject)
        assertEquals("", result.getString("module"))
        assertEquals(status, result.getBoolean("status"))
        assertEquals(timeElapsed, result.getLong("timeElapsed"))
    }
}

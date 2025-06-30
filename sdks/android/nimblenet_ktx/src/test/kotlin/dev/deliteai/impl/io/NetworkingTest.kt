/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.io

import dev.deliteai.impl.common.utils.jsonStringToHeaders
import dev.deliteai.impl.loggers.LocalLogger
import io.mockk.every
import io.mockk.mockk
import io.mockk.mockkConstructor
import io.mockk.mockkStatic
import io.mockk.slot
import junit.framework.Assert.assertEquals
import junit.framework.Assert.assertTrue
import kotlin.random.Random
import kotlin.text.Charsets.UTF_8
import okhttp3.Call
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.OkHttpClient
import okhttp3.Protocol
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import okhttp3.Response
import okhttp3.ResponseBody.Companion.toResponseBody
import okio.Buffer
import okio.GzipSource
import okio.buffer
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.JUnit4

@RunWith(JUnit4::class)
class NetworkingTest {
    private var okHttpClientMocked: OkHttpClient = mockk(relaxed = true)
    private var callMocked: Call = mockk(relaxed = true)
    private var localLoggerMocked: LocalLogger = mockk(relaxed = true)
    private var chunkDownloadManagerMocked: ChunkDownloadManager = mockk(relaxed = true)
    private lateinit var networkingMocked: Networking

    @Before
    fun `setup mocks`() {
        mockkConstructor(OkHttpClient.Builder::class)
        mockkConstructor(OkHttpClient::class)
        mockkConstructor(Call.Factory::class)

        every { anyConstructed<OkHttpClient.Builder>().build() } returns okHttpClientMocked

        val fakeBuilder = mockk<OkHttpClient.Builder>(relaxed = true)
        every { fakeBuilder.readTimeout(any(), any()) } returns fakeBuilder
        every { fakeBuilder.writeTimeout(any(), any()) } returns fakeBuilder
        every { fakeBuilder.callTimeout(any(), any()) } returns fakeBuilder
        every { fakeBuilder.build() } returns okHttpClientMocked

        val fakeHeaders = okhttp3.Headers.Builder().build()

        mockkStatic("dev.deliteai.impl.common.utils.KotlinUtilsKt")
        every { jsonStringToHeaders(any()) } returns fakeHeaders

        every { okHttpClientMocked.newBuilder() } returns fakeBuilder

        networkingMocked =
            Networking(okHttpClientMocked, localLoggerMocked, chunkDownloadManagerMocked)
    }

    @Test
    fun `sendRequest should return successful response for post request`() {
        val responseBody = "Success".toResponseBody("application/json".toMediaType())
        val mockResponse =
            Response.Builder()
                .request(Request.Builder().url("http://test.url").build())
                .protocol(Protocol.HTTP_1_1)
                .message("OK")
                .code(200)
                .body(responseBody)
                .build()

        every { okHttpClientMocked.newCall(any()) } returns callMocked
        every { callMocked.execute() } returns mockResponse

        val networkResponse =
            networkingMocked.sendRequest(
                url = "http://test.url",
                requestHeaders = """[{"Content-Type":"application/json"}]""",
                requestBody = """{"key":"value"}""",
                requestBodyByte = null,
                method = "POST",
                totalCallTimeoutInSecs = 10,
            )

        assertEquals(200, networkResponse.statusCode)
        assertTrue(String(networkResponse.body, UTF_8).contains("Success"))
    }

    @Test
    fun `on non-null requestBodyByte sendRequest should return successful response for post request`() {
        val largeByteArray: ByteArray = ByteArray(1024) { Random.nextInt(0, 256).toByte() }
        val responseBody = "Success".toResponseBody("application/json".toMediaType())
        val mockResponse =
            Response.Builder()
                .request(Request.Builder().url("http://test.url").build())
                .protocol(Protocol.HTTP_1_1)
                .message("OK")
                .code(200)
                .body(responseBody)
                .build()

        every { okHttpClientMocked.newCall(any()) } returns callMocked
        every { callMocked.execute() } returns mockResponse

        val networkResponse =
            networkingMocked.sendRequest(
                url = "http://test.url",
                requestHeaders = """[{"Content-Type":"application/json"}]""",
                requestBody = """{"key":"value"}""",
                requestBodyByte = largeByteArray,
                method = "POST",
                totalCallTimeoutInSecs = 10,
            )

        assertEquals(200, networkResponse.statusCode)
        assertTrue(String(networkResponse.body, UTF_8).contains("Success"))
    }

    @Test
    fun `sendRequest should handle 4xx error response`() {
        val responseBody = "Error".toResponseBody("application/json".toMediaType())
        val mockResponse =
            Response.Builder()
                .request(Request.Builder().url("http://test.url").build())
                .protocol(Protocol.HTTP_1_1)
                .message("Bad Request")
                .code(400)
                .body(responseBody)
                .build()

        every { okHttpClientMocked.newCall(any()) } returns callMocked
        every { callMocked.execute() } returns mockResponse

        val networkResponse =
            networkingMocked.sendRequest(
                url = "http://test.url",
                requestHeaders = """[{"Content-Type":"application/json"}]""",
                requestBody = """{"key":"value"}""",
                requestBodyByte = null,
                method = "POST",
                totalCallTimeoutInSecs = 10,
            )

        assertEquals(400, networkResponse.statusCode)
        assertTrue(String(networkResponse.body, UTF_8).contains("Error"))
    }

    @Test
    fun `sendRequest should handle 5xx error response`() {
        val responseBody = "Error".toResponseBody("application/json".toMediaType())
        val mockResponse =
            Response.Builder()
                .request(Request.Builder().url("http://test.url").build())
                .protocol(Protocol.HTTP_1_1)
                .message("Bad Request")
                .code(500)
                .body(responseBody)
                .build()

        every { okHttpClientMocked.newCall(any()) } returns callMocked
        every { callMocked.execute() } returns mockResponse

        val networkResponse =
            networkingMocked.sendRequest(
                url = "http://test.url",
                requestHeaders = """[{"Content-Type":"application/json"}]""",
                requestBody = """{"key":"value"}""",
                requestBodyByte = null,
                method = "POST",
                totalCallTimeoutInSecs = 10,
            )

        assertEquals(500, networkResponse.statusCode)
        assertTrue(String(networkResponse.body, UTF_8).contains("Error"))
    }

    @Test
    fun `sendRequest should handle exception`() {
        every { okHttpClientMocked.newCall(any()) } returns callMocked
        every { callMocked.execute() } throws RuntimeException("Test Exception")

        val networkResponse =
            networkingMocked.sendRequest(
                url = "http://test.url",
                requestHeaders = """[{"Content-Type":"application/json"}]""",
                requestBody = """{"key":"value"}""",
                requestBodyByte = null,
                method = "GET",
                totalCallTimeoutInSecs = 10,
            )

        assertEquals(900, networkResponse.statusCode)
        assertTrue(String(networkResponse.body, UTF_8).contains("Test Exception"))
    }

    @Test
    fun `request created in sendRequest must contain Content-Encoding header`() {
        val capturedRequest = slot<Request>()
        every { okHttpClientMocked.newCall(capture(capturedRequest)) } returns callMocked
        every { callMocked.execute() } returns
            Response.Builder()
                .request(Request.Builder().url("http://test.url").build())
                .protocol(Protocol.HTTP_1_1)
                .message("OK")
                .code(200)
                .body("Success".toResponseBody("application/json".toMediaType()))
                .build()

        networkingMocked.sendRequest(
            url = "http://test.url",
            requestHeaders = "[{\"Content-Encoding\":\"gzip\"}]",
            requestBody = "Test body",
            requestBodyByte = null,
            method = "POST",
            totalCallTimeoutInSecs = 10,
        )

        assertTrue(capturedRequest.isCaptured)
        assertEquals("gzip", capturedRequest.captured.header("Content-Encoding"))
    }

    @Test
    fun `gzip should return a compressed request body`() {
        val originalContent = "This is a test string for gzip compression."
        val requestBody = originalContent.toByteArray().toRequestBody("text/plain".toMediaType())

        val gzippedBody = networkingMocked.gzip(requestBody)

        val buffer = Buffer()
        gzippedBody.writeTo(buffer)

        val compressedBytes = buffer.readByteArray()
        val decompressedBytes = GzipSource(Buffer().write(compressedBytes)).buffer().readByteArray()

        assertEquals(originalContent, String(decompressedBytes, UTF_8))
    }

    @Test
    fun `gzip should preserve content type`() {
        val requestBody =
            "Sample content".toByteArray().toRequestBody("application/json".toMediaType())
        val gzippedBody = networkingMocked.gzip(requestBody)

        assertEquals("application/json".toMediaType(), gzippedBody.contentType())
    }

    @Test
    fun `gzip should return unknown content length`() {
        val requestBody = "Sample content".toByteArray().toRequestBody("text/plain".toMediaType())
        val gzippedBody = networkingMocked.gzip(requestBody)

        assertEquals(-1, gzippedBody.contentLength())
    }
}

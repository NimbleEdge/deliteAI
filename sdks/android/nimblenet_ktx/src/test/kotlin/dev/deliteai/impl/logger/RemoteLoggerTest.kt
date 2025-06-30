/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.logger

import dev.deliteai.impl.common.HardwareInfo
import dev.deliteai.impl.io.Networking
import dev.deliteai.impl.loggers.LocalLogger
import dev.deliteai.impl.loggers.RemoteLogger
import dev.deliteai.impl.nativeBridge.CoreRuntime
import dev.deliteai.testUtils.nnConfig
import io.mockk.MockKAnnotations
import io.mockk.every
import io.mockk.mockk
import io.mockk.verify
import org.json.JSONObject
import org.junit.Before
import org.junit.Test

class RemoteLoggerTest {
    private lateinit var remoteLogger: RemoteLogger
    private val networkingMocked: Networking = mockk(relaxed = true)
    private val hardwareInfoMocked: HardwareInfo = mockk(relaxed = true)
    private val localLoggerMocked: LocalLogger = mockk(relaxed = true)
    private val coreRuntimeMocked: CoreRuntime = mockk(relaxed = true)

    @Before
    fun setup() {
        MockKAnnotations.init(this)

        every { hardwareInfoMocked.getDeviceArch() } returns "arm64"

        remoteLogger =
            RemoteLogger(
                networking = networkingMocked,
                hardwareInfo = hardwareInfoMocked,
                nimbleNetConfig = nnConfig,
                localLogger = localLoggerMocked,
                coreRuntime = coreRuntimeMocked,
            )
    }

    @Test
    fun `writeMetric delegates to coreRuntime`() {
        val metricType = "TEST_METRIC"
        val metric = "test data"

        remoteLogger.writeMetric(metricType, metric)

        verify { coreRuntimeMocked.writeMetric(metricType, metric) }
    }

    @Test
    fun `bufferMetric stores metrics for later transmission`() {
        val metricType = "PERFORMANCE"
        val jsonObject =
            JSONObject().apply {
                put("duration", 123)
                put("status", "success")
            }

        remoteLogger.bufferMetric(metricType, jsonObject)

        every { networkingMocked.sendRequest(any(), any(), any(), any(), any(), any()) } returns
            mockk { every { statusCode } returns 200 }

        remoteLogger.flush()

        verify(timeout = 1000) {
            networkingMocked.sendRequest(any(), any(), any(), any(), "POST", 0)
        }
    }

    @Test
    fun `flush handles empty buffer gracefully`() {
        every { networkingMocked.sendRequest(any(), any(), any(), any(), any(), any()) } returns
            mockk { every { statusCode } returns 200 }

        remoteLogger.flush()
        Thread.sleep(100)

        verify(exactly = 0) {
            networkingMocked.sendRequest(any(), any(), any(), any(), any(), any())
        }
    }

    @Test
    fun `flush sends buffered logs and clears buffer on success`() {
        val jsonObject = JSONObject().apply { put("test", "data") }
        remoteLogger.bufferMetric("TEST", jsonObject)

        every { networkingMocked.sendRequest(any(), any(), any(), any(), any(), any()) } returns
            mockk { every { statusCode } returns 200 }

        remoteLogger.flush()
        Thread.sleep(200)

        verify(timeout = 1000) {
            networkingMocked.sendRequest(any(), any(), any(), any(), "POST", 0)
        }

        // Buffer should be cleared - subsequent flush should not call networking again
        remoteLogger.flush()
        Thread.sleep(100)

        verify(exactly = 1) {
            networkingMocked.sendRequest(any(), any(), any(), any(), any(), any())
        }
    }

    @Test
    fun `flush retains logs in buffer on network failure`() {
        val jsonObject = JSONObject().apply { put("test", "data") }
        remoteLogger.bufferMetric("TEST", jsonObject)

        every { networkingMocked.sendRequest(any(), any(), any(), any(), any(), any()) } returns
            mockk {
                every { statusCode } returns 500
                every { body } returns "Server Error".toByteArray()
            }

        remoteLogger.flush()
        Thread.sleep(200)

        verify(timeout = 1000) { localLoggerMocked.e(any<String>()) }

        // Logs should be retained - second flush should retry with same logs
        every { networkingMocked.sendRequest(any(), any(), any(), any(), any(), any()) } returns
            mockk { every { statusCode } returns 200 }

        remoteLogger.flush()
        Thread.sleep(200)

        // Should have called networking twice total (first failed, second succeeded)
        verify(exactly = 2) {
            networkingMocked.sendRequest(any(), any(), any(), any(), any(), any())
        }
    }

    @Test
    fun `flush calls networking with proper parameters`() {
        val jsonObject = JSONObject().apply { put("test", "data") }
        remoteLogger.bufferMetric("TEST", jsonObject)

        every { networkingMocked.sendRequest(any(), any(), any(), any(), any(), any()) } returns
            mockk { every { statusCode } returns 200 }

        remoteLogger.flush()

        verify(timeout = 1000) {
            networkingMocked.sendRequest(any(), any(), any(), any(), "POST", 0)
        }
    }

    @Test
    fun `flush logs error for failed status codes`() {
        val jsonObject = JSONObject().apply { put("test", "data") }
        remoteLogger.bufferMetric("TEST", jsonObject)

        every { networkingMocked.sendRequest(any(), any(), any(), any(), any(), any()) } returns
            mockk {
                every { statusCode } returns 500
                every { body } returns "Error occurred".toByteArray()
            }

        remoteLogger.flush()

        verify(timeout = 1000) { localLoggerMocked.e(any<String>()) }
    }

    @Test
    fun `multiple metrics can be buffered and flushed`() {
        val json1 = JSONObject().apply { put("metric1", "value1") }
        val json2 = JSONObject().apply { put("metric2", "value2") }

        remoteLogger.bufferMetric("TYPE1", json1)
        remoteLogger.bufferMetric("TYPE2", json2)

        every { networkingMocked.sendRequest(any(), any(), any(), any(), any(), any()) } returns
            mockk { every { statusCode } returns 200 }

        remoteLogger.flush()
        Thread.sleep(200)

        // Should call networking exactly once with both metrics combined
        verify(exactly = 1) {
            networkingMocked.sendRequest(any(), any(), any(), any(), any(), any())
        }

        // Buffer should be cleared after successful flush
        remoteLogger.flush()
        Thread.sleep(100)

        // Should still only have been called once (no additional calls for empty buffer)
        verify(exactly = 1) {
            networkingMocked.sendRequest(any(), any(), any(), any(), any(), any())
        }
    }
}

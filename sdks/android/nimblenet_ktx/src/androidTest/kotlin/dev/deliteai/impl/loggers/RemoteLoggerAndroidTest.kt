/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.loggers

import android.app.Application
import androidx.test.core.app.ApplicationProvider
import dev.deliteai.impl.DependencyContainer
import dev.deliteai.testUtils.nnConfig
import org.json.JSONObject
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

class RemoteLoggerAndroidTest {
    private lateinit var remoteLogger: RemoteLogger
    private lateinit var application: Application

    @Before
    fun setup() {
        application = ApplicationProvider.getApplicationContext<Application>()
        val dependencyContainer = DependencyContainer.getInstance(application, nnConfig)
        remoteLogger = dependencyContainer.getRemoteLogger()
    }

    @Test
    fun bufferMetricShouldNotThrowException() {
        val metricType = "performance"
        val jsonObject =
            JSONObject().apply {
                put("cpu_usage", 45.6)
                put("memory_usage", 1024)
                put("timestamp", System.currentTimeMillis())
            }

        try {
            remoteLogger.bufferMetric(metricType, jsonObject)
            assertTrue("bufferMetric should complete without exception", true)
        } catch (e: Exception) {
            assertTrue("bufferMetric should not throw exception: ${e.message}", false)
        }
    }

    @Test
    fun flushShouldNotThrowException() {
        val jsonObject =
            JSONObject().apply {
                put("test_metric", "test_value")
                put("device_info", "android_test")
                put("timestamp", System.currentTimeMillis())
            }

        try {
            remoteLogger.bufferMetric("test_type", jsonObject)
            remoteLogger.flush()
            assertTrue("flush should complete without exception", true)
        } catch (e: Exception) {
            assertTrue("flush should not throw exception: ${e.message}", false)
        }
    }

    @Test
    fun multipleBufferAndFlushShouldWork() {
        try {
            for (i in 1..3) {
                val jsonObject =
                    JSONObject().apply {
                        put("metric_$i", "value_$i")
                        put("iteration", i)
                        put("timestamp", System.currentTimeMillis())
                    }
                remoteLogger.bufferMetric("batch_test", jsonObject)
            }

            remoteLogger.flush()
            assertTrue("Multiple buffer and flush operations should work", true)
        } catch (e: Exception) {
            assertTrue("Multiple buffer and flush should not throw exception: ${e.message}", false)
        }
    }

    @Test
    fun bufferDifferentMetricTypesShouldWork() {
        try {
            val performanceMetric =
                JSONObject().apply {
                    put("cpu_usage", 65.4)
                    put("memory_mb", 512)
                }

            val networkMetric =
                JSONObject().apply {
                    put("request_count", 10)
                    put("response_time_ms", 250)
                }

            val errorMetric =
                JSONObject().apply {
                    put("error_code", "404")
                    put("error_message", "Not Found")
                }

            remoteLogger.bufferMetric("performance", performanceMetric)
            remoteLogger.bufferMetric("network", networkMetric)
            remoteLogger.bufferMetric("error", errorMetric)

            remoteLogger.flush()
            assertTrue("Different metric types should be buffered and flushed successfully", true)
        } catch (e: Exception) {
            assertTrue("Different metric types should not cause exception: ${e.message}", false)
        }
    }

    @Test
    fun emptyFlushShouldNotThrowException() {
        try {
            remoteLogger.flush()
            assertTrue("Empty flush should complete without exception", true)
        } catch (e: Exception) {
            assertTrue("Empty flush should not throw exception: ${e.message}", false)
        }
    }

    @Test
    fun complexJSONObjectShouldBeBuffered() {
        val complexObject =
            JSONObject().apply {
                put("string_field", "test_value")
                put("numeric_field", 42.5)
                put("boolean_field", true)
                put("timestamp", System.currentTimeMillis())

                val nestedObject =
                    JSONObject().apply {
                        put("nested_string", "nested_value")
                        put("nested_number", 100)
                    }
                put("nested_object", nestedObject)
            }

        try {
            remoteLogger.bufferMetric("complex_test", complexObject)
            remoteLogger.flush()
            assertTrue("Complex JSON object should be handled correctly", true)
        } catch (e: Exception) {
            assertTrue("Complex JSON should not cause exception: ${e.message}", false)
        }
    }
}

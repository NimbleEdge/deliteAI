/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.loggers

import android.annotation.SuppressLint
import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.impl.common.HardwareInfo
import dev.deliteai.impl.io.Networking
import dev.deliteai.impl.nativeBridge.CoreRuntime
import dev.deliteai.nimblenet_ktx.BuildConfig
import java.text.SimpleDateFormat
import java.util.Collections
import java.util.Date
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import org.json.JSONArray
import org.json.JSONObject

internal class RemoteLogger(
    private val networking: Networking,
    private val hardwareInfo: HardwareInfo,
    private val nimbleNetConfig: NimbleNetConfig,
    private val localLogger: LocalLogger,
    private val coreRuntime: CoreRuntime,
) {
    private var logBuffer = Collections.synchronizedList(mutableListOf<String>())
    private val ioScope = CoroutineScope(Dispatchers.IO + SupervisorJob())

    fun writeMetric(metricType: String, metric: String) {
        coreRuntime.writeMetric(metricType, metric)
    }

    fun bufferMetric(metricType: String, jsonObject: JSONObject) {
        logBuffer.add(
            "METRICS::: ${formattedTimestamp()} ::: $metricType ::: " + jsonObject.toString()
        )
    }

    fun flush() {
        ioScope.launch {
            val oldQueue = logBuffer
            logBuffer = mutableListOf()

            val success = sendBuffer(oldQueue)

            if (!success) {
                logBuffer.addAll(0, oldQueue)
            }
        }
    }

    private fun sendBuffer(logs: List<String>): Boolean {
        if (logs.isEmpty()) {
            return true
        }

        val payload = logs.joinToString(separator = "\n")

        val headerString =
            JSONArray(
                    listOf(
                        mapOf(
                            "Content-Type" to "application/octet-stream",
                            "Secret-Key" to BuildConfig.REMOTE_LOGGER_KEY,
                            "Accept" to "application/json",
                            "ddsource" to "android_${hardwareInfo.getDeviceArch()}",
                            "clientId" to nimbleNetConfig.clientId,
                            "deviceID" to nimbleNetConfig.deviceId,
                            "compatibilityTag" to nimbleNetConfig.compatibilityTag,
                            "internalDeviceID" to
                                (nimbleNetConfig.getInternalDeviceId() ?: JSONObject.NULL),
                        )
                    )
                )
                .toString()

        val res =
            networking.sendRequest(
                BuildConfig.REMOTE_LOGGER_URL,
                headerString,
                payload,
                null,
                "POST",
                totalCallTimeoutInSecs = 0,
            )

        return if (res.statusCode in 200..300) {
            true
        } else {
            localLogger.e(res.body.toString())
            false
        }
    }

    @SuppressLint("SimpleDateFormat")
    private fun formattedTimestamp(): String {
        val dateFormat = SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS")
        val currentDateTime = Date()
        return dateFormat.format(currentDateTime)
    }
}

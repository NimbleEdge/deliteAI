/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.datamodels

import ai.nimbleedge.impl.common.NIMBLENET_VARIANTS
import ai.nimbleedge.impl.common.SDK_CONSTANTS
import org.json.JSONObject

class NimbleNetConfig(
    val clientId: String,
    val clientSecret: String,
    val host: String,
    val deviceId: String,
    val debug: Boolean = false,
    val initTimeOutInMs: Long = SDK_CONSTANTS.INIT_DEFAULT_TIMEOUT,
    val compatibilityTag: String,
    val sessionId: String = "",
    val maxDBSizeKBs: Float? = null,
    val maxEventsSizeKBs: Float? = null,
    val cohortIds: Array<String> = arrayOf(),
    val libraryVariant: NIMBLENET_VARIANTS,
    val showDownloadProgress: Boolean = false,
) {
    private var internalDeviceId: String? = null

    fun setInternalDeviceId(id: String) {
        internalDeviceId = id
    }

    fun getInternalDeviceId(): String? = internalDeviceId

    fun copy(
        clientId: String = this.clientId,
        clientSecret: String = this.clientSecret,
        host: String = this.host,
        deviceId: String = this.deviceId,
        debug: Boolean = this.debug,
        initTimeOutInMs: Long = this.initTimeOutInMs,
        compatibilityTag: String = this.compatibilityTag,
        sessionId: String = this.sessionId,
        maxDBSizeKBs: Float? = this.maxDBSizeKBs,
        maxEventsSizeKBs: Float? = this.maxEventsSizeKBs,
        cohortIds: Array<String> = this.cohortIds,
        libraryVariant: NIMBLENET_VARIANTS = this.libraryVariant,
        showDownloadProgress: Boolean = this.showDownloadProgress,
    ): NimbleNetConfig {
        return NimbleNetConfig(
                clientId = clientId,
                clientSecret = clientSecret,
                host = host,
                deviceId = deviceId,
                debug = debug,
                initTimeOutInMs = initTimeOutInMs,
                compatibilityTag = compatibilityTag,
                sessionId = sessionId,
                maxDBSizeKBs = maxDBSizeKBs,
                maxEventsSizeKBs = maxEventsSizeKBs,
                cohortIds = cohortIds,
                libraryVariant = libraryVariant,
                showDownloadProgress = showDownloadProgress,
            )
            .apply { this.internalDeviceId = this@NimbleNetConfig.internalDeviceId }
    }

    override fun toString(): String {
        val map =
            mutableMapOf(
                "clientId" to this.clientId,
                "clientSecret" to this.clientSecret,
                "host" to this.host,
                "deviceId" to this.deviceId,
                "debug" to this.debug,
                "initTimeOutInMs" to this.initTimeOutInMs,
                "compatibilityTag" to this.compatibilityTag,
                "sessionId" to this.sessionId,
                "cohortIds" to this.cohortIds,
                "libraryVariant" to this.libraryVariant.name,
                "internalDeviceId" to this.internalDeviceId,
                "showDownloadProgress" to this.showDownloadProgress,
            )

        // conditional entries
        this.maxDBSizeKBs?.let { map["maxDBSizeKBs"] = it }
        this.maxEventsSizeKBs?.let { map["maxEventsSizeKBs"] = it }

        return JSONObject(map.toMap()).toString()
    }

    companion object {
        fun fromString(jsonString: String): NimbleNetConfig {
            val jsonObject = JSONObject(jsonString)

            return NimbleNetConfig(
                    clientId = jsonObject.getString("clientId"),
                    clientSecret = jsonObject.getString("clientSecret"),
                    host = jsonObject.getString("host"),
                    deviceId = jsonObject.getString("deviceId"),
                    debug = jsonObject.optBoolean("debug", false),
                    initTimeOutInMs =
                        jsonObject.optLong("initTimeOutInMs", SDK_CONSTANTS.INIT_DEFAULT_TIMEOUT),
                    compatibilityTag = jsonObject.getString("compatibilityTag"),
                    sessionId = jsonObject.optString("sessionId", ""),
                    maxDBSizeKBs =
                        jsonObject.optDouble("maxDBSizeKBs").takeIf { !it.isNaN() }?.toFloat(),
                    maxEventsSizeKBs =
                        jsonObject.optDouble("maxEventsSizeKBs").takeIf { !it.isNaN() }?.toFloat(),
                    libraryVariant =
                        NIMBLENET_VARIANTS.valueOf(jsonObject.getString("libraryVariant")),
                    cohortIds =
                        jsonObject.optJSONArray("cohortIds")?.let { jsonArray ->
                            Array(jsonArray.length()) { index -> jsonArray.getString(index) }
                        } ?: arrayOf(),
                    showDownloadProgress = jsonObject.optBoolean("showDownloadProgress", false),
                )
                .apply {
                    if (jsonObject.has("internalDeviceId")) {
                        this.internalDeviceId = jsonObject.optString("internalDeviceId", "")
                    }
                }
        }
    }
}

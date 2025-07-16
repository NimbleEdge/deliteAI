/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.datamodels

import dev.deliteai.impl.common.NIMBLENET_VARIANTS
import dev.deliteai.impl.common.SDK_CONSTANTS
import org.json.JSONObject

/**
 * Configuration class for initializing the NimbleNet SDK.
 *
 * This class contains all the necessary parameters and settings required to initialize and
 * configure the NimbleNet SDK. It includes authentication credentials, server endpoints, resource
 * limits, and deployment options.
 *
 * ## Parameters
 * - [clientId]: Your unique client identifier from NimbleNet platform
 * - [clientSecret]: Authentication secret key
 * - [host]: NimbleNet platform API endpoint URL
 * - [deviceId]: Unique identifier for this device
 * - [compatibilityTag]: Version compatibility identifier to identify an online deployment
 * - [libraryVariant]: Library deployment variant (static or dynamic)
 * - [debug]: Enable debug logging and diagnostics for the delitePy script
 * - [sessionId]: Custom session identifier for session management of events
 * - [maxDBSizeKBs]: Database storage limit in kilobytes
 * - [maxEventsSizeKBs]: Event data storage limit in kilobytes
 * - [cohortIds]: Array of cohort identifiers for A/B testing
 * - [showDownloadProgress]: Display download progress indicators for models in the notification bar
 * - [online]: Flag to control whether the SDK should connect to cloud to download assets or if the
 *   assets already bundled with the app
 *
 * ## Usage Examples
 *
 * ### Basic Configuration
 *
 * ```kotlin
 * val config = NimbleNetConfig(
 *     clientId = "your-client-id",
 *     clientSecret = "your-secret-key",
 *     host = "https://api.nimblenet.ai",
 *     deviceId = "device-id",
 *     compatibilityTag = "v1.2.0",
 *     libraryVariant = NIMBLENET_VARIANTS.STATIC,
 *     online = true
 * )
 * ```
 *
 * ## Library Variants
 * - **STATIC**: Traditional static library embedding, suitable for most applications
 * - **GOOGLE_PLAY_FEATURE_DYNAMIC**: Google Play Dynamic Feature delivery for reduced APK size
 *
 * ## Security Considerations
 * - Store [clientSecret] securely, never in plain text or version control
 * - Use different credentials for development/staging/production environments
 *
 * ## Performance Tuning
 * - Configure [maxDBSizeKBs] and [maxEventsSizeKBs] based on device storage constraints
 * - Use [debug] = false in production for optimal performance
 *
 * @param clientId Unique client identifier provided by NimbleNet platform. Must be a non-empty
 *   string obtained from your NimbleNet account. Default: "" (empty string)
 * @param clientSecret Authentication secret key for API access. Keep this value secure and never
 *   expose it in logs or client-side code. Default: "" (empty string)
 * @param host The base URL of the NimbleNet platform API endpoint. Must be a valid HTTPS URL (e.g.,
 *   "https://api.nimblenet.ai"). Default: "" (empty string)
 * @param deviceId Unique identifier for this device or application installation. Can be a UUID,
 *   device fingerprint, or custom identifier. Should remain consistent across app sessions for the
 *   same device. Default: "" (empty string)
 * @param debug Enable debug mode for detailed logging and diagnostics. Set to false in production
 *   builds for optimal performance. Default: false
 * @param initTimeOutInMs Maximum time in milliseconds to wait for SDK initialization. Default:
 *   [SDK_CONSTANTS.INIT_DEFAULT_TIMEOUT]
 * @param compatibilityTag Version identifier for API compatibility checking. Should match your
 *   application version or SDK compatibility version. Used to ensure client-server API
 *   compatibility. Default: "" (empty string)
 * @param sessionId Optional custom session identifier for session management. If empty, the SDK
 *   will generate a default session ID. Useful for multi-user applications or session persistence.
 *   Default: "" (empty string)
 * @param maxDBSizeKBs Optional maximum database size limit in kilobytes. When reached, older data
 *   may be purged to stay within limits. Set based on your device storage constraints. Default:
 *   null (no specific limit)
 * @param maxEventsSizeKBs Optional maximum event data storage limit in kilobytes. Controls how much
 *   event data can be cached before upload. Default: null (no specific limit)
 * @param cohortIds Array of cohort identifiers for A/B testing and experimentation. Used to assign
 *   users to specific test groups or feature variants. Default: empty array
 * @param libraryVariant Deployment variant determining how the SDK is packaged and loaded. Choose
 *   based on your app distribution and size requirements. Default: NIMBLENET_VARIANTS.STATIC
 * @param showDownloadProgress Whether to display progress indicators during model downloads.
 *   Default: false
 *     @param online Whether the assets will be downloaded from cloud or they are already bundled
 *       with the app Default: false
 * @see NIMBLENET_VARIANTS
 * @see SDK_CONSTANTS
 * @since 1.0.0
 */
class NimbleNetConfig(
    val clientId: String = "",
    val clientSecret: String = "",
    val host: String = "",
    val deviceId: String = "",
    val debug: Boolean = false,
    val initTimeOutInMs: Long = SDK_CONSTANTS.INIT_DEFAULT_TIMEOUT,
    val compatibilityTag: String = "",
    val sessionId: String = "",
    val maxDBSizeKBs: Float? = null,
    val maxEventsSizeKBs: Float? = null,
    val cohortIds: Array<String> = arrayOf(),
    val libraryVariant: NIMBLENET_VARIANTS = NIMBLENET_VARIANTS.STATIC,
    val showDownloadProgress: Boolean = false,
    val online: Boolean = false,
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
        online: Boolean = this.online,
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
                online = online,
            )
            .apply { this.internalDeviceId = this@NimbleNetConfig.internalDeviceId }
    }

    /**
     * Returns a JSON string representation of this configuration.
     *
     * This method serializes the configuration to JSON format, which can be useful for logging,
     * debugging, persistence, or network transmission.
     *
     * @return JSON string representation of the configuration
     * @since 1.0.0
     */
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
                "online" to this.online,
            )

        // conditional entries
        this.maxDBSizeKBs?.let { map["maxDBSizeKBs"] = it }
        this.maxEventsSizeKBs?.let { map["maxEventsSizeKBs"] = it }

        return JSONObject(map.toMap()).toString()
    }

    companion object {
        /**
         * Creates a [NimbleNetConfig] instance from a JSON string representation.
         *
         * This factory method deserializes a JSON string back into a configuration object.
         *
         * ## Example
         *
         * ```kotlin
         * val config = NimbleNetConfig.fromString(jsonConfig)
         * ```
         *
         * @param jsonString A valid JSON string containing configuration parameters
         * @return A new [NimbleNetConfig] instance parsed from the JSON
         * @throws org.json.JSONException if the JSON string is malformed or missing required fields
         * @throws IllegalArgumentException if libraryVariant contains an invalid enum value
         * @see toString
         * @since 1.0.0
         */
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
                    online = jsonObject.optBoolean("online", false),
                )
                .apply {
                    if (jsonObject.has("internalDeviceId")) {
                        this.internalDeviceId = jsonObject.optString("internalDeviceId", "")
                    }
                }
        }
    }
}

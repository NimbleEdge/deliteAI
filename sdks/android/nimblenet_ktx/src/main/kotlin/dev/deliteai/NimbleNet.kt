/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai

import android.app.Application
import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.datamodels.NimbleNetResult
import dev.deliteai.datamodels.NimbleNetTensor
import dev.deliteai.datamodels.UserEventData
import dev.deliteai.impl.DependencyContainer
import dev.deliteai.impl.common.MESSAGES
import dev.deliteai.impl.common.toNimbleNetResult
import dev.deliteai.impl.controllers.NimbleNetController
import dev.deliteai.impl.delitePy.proto.impl.ProtoObjectWrapper
import dev.deliteai.impl.loggers.LocalLogger
import org.json.JSONArray
import org.json.JSONObject

/**
 * The main entry point for the NimbleNet Android SDK.
 *
 * ## Basic Usage
 *
 * ```kotlin
 * // 1. Initialize the SDK
 * val config = NimbleNetConfig(
 *     clientId = "your-client-id",
 *     clientSecret = "your-client-secret",
 *     host = "https://your-api-host.com",
 *     deviceId = "unique-device-id",
 *     compatibilityTag = "your-compatibility-tag",
 *     libraryVariant = NIMBLENET_VARIANTS.STATIC,
 *     online = true
 * )
 *
 * val initResult = NimbleNet.initialize(application, config)
 * if (!initResult.status) {
 *     Log.e("NimbleNet", "Initialization failed: ${initResult.error?.message}")
 *     return
 * }
 *
 * // 2. Wait for SDK to be ready
 * while (!NimbleNet.isReady().status) {
 *     delay(1000)
 * }
 *
 * // 3. Execute a model
 * val inputs = hashMapOf(
 *     "input_tensor" to NimbleNetTensor(
 *         data = floatArrayOf(1.0f, 2.0f, 3.0f),
 *         shape = intArrayOf(3),
 *         datatype = DATATYPE.FLOAT
 *     )
 * )
 *
 * val result = NimbleNet.runMethod("my_workflow_script_method", inputs)
 * if (result.status) {
 *     val output = result.payload?.get("output_tensor")
 *     println("Model output: ${output?.data}")
 * }
 *
 * // 4. Track events
 * val eventData = mapOf("action" to "button_click", "screen" to "home")
 * NimbleNet.addEvent(eventData, "user_interaction")
 * ```
 *
 * @since 1.0.0
 */
object NimbleNet {

    @Volatile private lateinit var controller: NimbleNetController

    @Volatile private lateinit var localLogger: LocalLogger

    /**
     * Initializes the NimbleNet SDK with the provided configuration.
     *
     * This method must be called before any other NimbleNet operations. After calling this method,
     * use [isReady] to check when the SDK is fully initialized and ready for model execution.
     *
     * ## Example
     *
     * ```kotlin
     * val config = NimbleNetConfig(
     *     clientId = "my-client-id",
     *     clientSecret = "secret-key",
     *     host = "https://api.nimblenet.ai",
     *     deviceId = "device-${UUID.randomUUID()}",
     *     compatibilityTag = "v1.2.0",
     *     libraryVariant = NIMBLENET_VARIANTS.STATIC,
     *     online = true
     * )
     *
     * val offlineConfig = NimbleNetConfig(online = false)
     *
     * val assetsJson = [
     *             {
     *                 "name": "workflow_script",
     *                 "version": "1.0.0",
     *                 "type": "script",
     *                 "location": {
     *                     "path": "workflow script's ast location relative to assets"
     *                 }
     *             },
     *             {
     *                 "name": "add_model",
     *                 "version": "1.0.0",
     *                 "type": "model",
     *                 "location": {
     *                     "path": "model path relative to assets"
     *                 }
     *             }
     *         ]
     *
     *  The assetsJSON represents a structured assets configuration for loading them from disk.
     *
     *  Each asset json object contains:
     *  - `name`: A unique identifier for the asset.
     *  - `version`: The version of the asset.
     *  - `type`: Type of the asset (e.g., script, retriever, model, document, llm).
     *  - `location` (optional): An object with a `path` field pointing to the file
     *      location of the module asset. The path is relative to the assets folder.
     *  - `arguments` (optional): In case an asset depends on other assets then they are
     *       passed in arguments. For e.g. a retriever might have embedding model and document as its dependencies.
     *
     * // For online initialization
     * val result = NimbleNet.initialize(application, config)
     *
     * // For offline initialization
     * val result = NimbleNet.initialize(application, offlineConfig, assetsJson)
     * if (result.status) {
     *     // SDK initialized successfully
     * } else {
     *     Log.e("NimbleNet", "Init failed: ${result.error?.message}")
     * }
     * ```
     *
     * @param application The Android application context
     * @param config The configuration object containing client credentials and settings
     * @param assetsJson The assets configuration to initialize the SDK when assets such as the
     *   workflow script, models etc are bundled with the app
     * @return [NimbleNetResult]<[Unit]> indicating success or failure
     * @see isReady
     * @see NimbleNetConfig
     * @since 1.0.0
     */
    fun initialize(
        application: Application,
        config: NimbleNetConfig,
        assetsJson: JSONArray? = null,
    ): NimbleNetResult<Unit> {
        val container = DependencyContainer.getInstance(application, config)
        controller = container.getNimbleNetController()
        localLogger = container.getLocalLogger()

        return runCatching { controller.initialize(config, assetsJson) }
            .onFailure(localLogger::e)
            .getOrElse { it.toNimbleNetResult() }
    }

    /**
     * Records a user event with key-value data.
     *
     * ## Example
     *
     * ```kotlin
     * val eventData = mapOf(
     *     "action" to "button_click",
     *     "button_id" to "submit_btn",
     *     "timestamp" to System.currentTimeMillis()
     * )
     *
     * val result = NimbleNet.addEvent(eventData, "user_interaction")
     * if (result.status) {
     *     println("Event recorded successfully")
     * }
     * ```
     *
     * @param eventMap A map containing the event data
     * @param eventType A string identifier for the type of event
     * @return [NimbleNetResult]<[UserEventData]> with event processing results
     * @since 1.0.0
     */
    fun addEvent(eventMap: Map<String, Any>, eventType: String): NimbleNetResult<UserEventData> =
        safeCall {
            controller.addEvent(JSONObject(eventMap).toString(), eventType)
        }

    /**
     * Records a user event using pre-serialized JSON data.
     *
     * ## Example
     *
     * ```kotlin
     * val jsonEvent = """
     * {
     *     "action": "video_play",
     *     "video_id": "abc123",
     *     "duration_ms": 30000
     * }
     * """.trimIndent()
     *
     * val result = NimbleNet.addEvent(jsonEvent, "media_interaction")
     * ```
     *
     * @param serializedEventMap A JSON string representing the event data
     * @param eventType A string identifier for the type of event
     * @return [NimbleNetResult]<[UserEventData]> with event processing results
     * @since 1.0.0
     */
    fun addEvent(serializedEventMap: String, eventType: String): NimbleNetResult<UserEventData> =
        safeCall {
            controller.addEvent(serializedEventMap, eventType)
        }

    /**
     * Records a user event using Protocol Buffer data structures.
     *
     * ## Example
     *
     * ```kotlin
     * val protoWrapper = ProtoObjectWrapper(myProtoMessage, typeRegistry)
     * val result = NimbleNet.addEvent(protoWrapper, "proto_event_type")
     * ```
     *
     * @param protoEvent A [ProtoObjectWrapper] containing the Protocol Buffer message
     * @param eventType A string identifier for the type of event
     * @return [NimbleNetResult]<[UserEventData]> with event processing results
     * @see ProtoObjectWrapper
     * @since 1.0.0
     */
    fun addEvent(
        protoEvent: ProtoObjectWrapper,
        eventType: String,
    ): NimbleNetResult<UserEventData> = safeCall { controller.addEvent(protoEvent, eventType) }

    /**
     * Executes a function from a workflow script written in Python.
     *
     * The workflow script is either uploaded via the NimbleNet dashboard or loaded offline using
     * the offline initialization capabilities.
     *
     * ## Example
     *
     * ```kotlin
     * val inputs = hashMapOf(
     *     "input_data" to NimbleNetTensor(
     *         data = floatArrayOf(1.0f, 2.0f, 3.0f, 4.0f),
     *         shape = intArrayOf(2, 2),
     *         datatype = DATATYPE.FLOAT
     *     )
     * )
     *
     * val result = NimbleNet.runMethod("my_python_function", inputs)
     *
     * when {
     *     result.status -> {
     *         val outputs = result.payload
     *         // Process function outputs
     *     }
     *     else -> {
     *         Log.e("ML", "Function execution failed: ${result.error?.message}")
     *     }
     * }
     * ```
     *
     * @param methodName The name of the Python function to execute from the workflow script
     * @param inputs A map of input tensors. Can be null for functions that don't require inputs.
     * @return [NimbleNetResult]<[HashMap]<[String], [NimbleNetTensor]>> with function outputs
     * @see NimbleNetTensor
     * @since 1.0.0
     */
    fun runMethod(
        methodName: String,
        inputs: HashMap<String, NimbleNetTensor>?,
    ): NimbleNetResult<HashMap<String, NimbleNetTensor>> = safeCall {
        controller.runMethod(methodName = methodName, inputs = inputs)
    }

    /**
     * Checks if the NimbleNet SDK is fully initialized and ready for model execution.
     *
     * ## Example
     *
     * ```kotlin
     * // Wait for SDK to be ready
     * while (!NimbleNet.isReady().status) {
     *     delay(1000)
     * }
     *
     * // Now safe to run models
     * val result = NimbleNet.runMethod("my_model", inputs)
     * ```
     *
     * @return [NimbleNetResult]<[Unit]> indicating if SDK is ready
     * @see initialize
     * @since 1.0.0
     */
    fun isReady(): NimbleNetResult<Unit> = safeCall { controller.isReady() }

    /**
     * Restarts the current session, generates a random sessionId internally.
     *
     * @since 1.0.0
     */
    fun restartSession() {
        runCatching {
                checkInit()
                controller.restartSession("")
            }
            .onFailure(localLogger::e)
    }

    /**
     * Restarts the session with a specific session identifier.
     *
     * @param sessionId A unique identifier for the new session
     * @since 1.0.0
     */
    fun restartSessionWithId(sessionId: String) {
        runCatching {
                checkInit()
                controller.restartSession(sessionId)
            }
            .onFailure(localLogger::e)
    }

    /** Validates that the SDK has been properly initialized. */
    private fun checkInit() {
        if (!this::controller.isInitialized || !controller.isNimbleNetInitialized()) {
            throw IllegalStateException(MESSAGES.SDK_NOT_INITIALIZED)
        }
    }

    /** Executes a block of code with proper error handling and initialization checks. */
    private inline fun <T> safeCall(
        crossinline block: () -> NimbleNetResult<T>
    ): NimbleNetResult<T> =
        runCatching {
                checkInit()
                block()
            }
            .onFailure(localLogger::e)
            .getOrElse { it.toNimbleNetResult() }
}

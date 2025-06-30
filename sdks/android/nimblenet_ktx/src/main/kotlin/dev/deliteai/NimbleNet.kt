/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge

import ai.nimbleedge.datamodels.NimbleNetConfig
import ai.nimbleedge.datamodels.NimbleNetResult
import ai.nimbleedge.datamodels.NimbleNetTensor
import ai.nimbleedge.datamodels.UserEventData
import ai.nimbleedge.impl.DependencyContainer
import ai.nimbleedge.impl.common.MESSAGES
import ai.nimbleedge.impl.common.toNimbleNetResult
import ai.nimbleedge.impl.controllers.NimbleNetController
import ai.nimbleedge.impl.delitePy.proto.impl.ProtoObjectWrapper
import ai.nimbleedge.impl.loggers.LocalLogger
import android.app.Application
import org.json.JSONObject

object NimbleNet {

    @Volatile private lateinit var controller: NimbleNetController

    @Volatile private lateinit var localLogger: LocalLogger

    fun initialize(application: Application, config: NimbleNetConfig): NimbleNetResult<Unit> {
        val container = DependencyContainer.getInstance(application, config)
        controller = container.getNimbleNetController()
        localLogger = container.getLocalLogger()

        return runCatching { controller.initialize(config) }
            .onFailure(localLogger::e)
            .getOrElse { it.toNimbleNetResult() }
    }

    fun addEvent(eventMap: Map<String, Any>, eventType: String): NimbleNetResult<UserEventData> =
        safeCall {
            controller.addEvent(JSONObject(eventMap).toString(), eventType)
        }

    fun addEvent(serializedEventMap: String, eventType: String): NimbleNetResult<UserEventData> =
        safeCall {
            controller.addEvent(serializedEventMap, eventType)
        }

    fun addEvent(
        protoEvent: ProtoObjectWrapper,
        eventType: String,
    ): NimbleNetResult<UserEventData> = safeCall { controller.addEvent(protoEvent, eventType) }

    fun runMethod(
        methodName: String,
        inputs: HashMap<String, NimbleNetTensor>?,
    ): NimbleNetResult<HashMap<String, NimbleNetTensor>> = safeCall {
        controller.runMethod(methodName = methodName, inputs = inputs)
    }

    fun isReady(): NimbleNetResult<Unit> = safeCall { controller.isReady() }

    fun restartSession() {
        runCatching {
                checkInit()
                controller.restartSession("")
            }
            .onFailure(localLogger::e)
    }

    fun restartSessionWithId(sessionId: String) {
        runCatching {
                checkInit()
                controller.restartSession(sessionId)
            }
            .onFailure(localLogger::e)
    }

    private fun checkInit() {
        if (!this::controller.isInitialized || !controller.isNimbleNetInitialized()) {
            throw IllegalStateException(MESSAGES.SDK_NOT_INITIALIZED)
        }
    }

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

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.nativeBridge.impl

import dev.deliteai.datamodels.NimbleNetResult
import dev.deliteai.datamodels.NimbleNetTensor
import dev.deliteai.datamodels.UserEventData
import dev.deliteai.impl.delitePy.proto.impl.ProtoObjectWrapper
import dev.deliteai.impl.nativeBridge.CoreRuntime
import org.json.JSONArray

internal class CoreRuntimeImpl : CoreRuntime {
    external override fun initializeNimbleNet(
        context: Any,
        nimbleNetConfig: String,
        assetsJson: JSONArray?,
        nimbleDirPath: String,
        nimbleNetResult: NimbleNetResult<Unit>,
    )

    external override fun addEvent(
        serializedEventMap: String,
        tableName: String,
        nimbleNetResult: NimbleNetResult<UserEventData>,
    )

    external override fun addEventProto(
        protoEvent: ProtoObjectWrapper,
        eventType: String,
        nimbleNetResult: NimbleNetResult<UserEventData>,
    )

    external override fun runMethod(
        methodName: String,
        inputs: Any,
        nimbleNetResult: NimbleNetResult<HashMap<String, NimbleNetTensor>>,
    )

    external override fun isReady(nimbleNetResult: NimbleNetResult<Unit>)

    external override fun restartSession(sessionId: String)

    external override fun writeMetric(metricType: String, metric: String)

    external override fun networkConnectionEstablishedCallback()

    external override fun writeRunMethodMetric(id: String, totalTimeInUSecs: Long)

    external override fun sendEvents(params: String, homeDir: String): Boolean

    // internal functions
    external override fun reset()

    external override fun deleteDatabase()

    external override fun reloadModelWithEpConfig(modelName: String, epConfig: String): Boolean

    external override fun loadModelFromFile(
        modelFilePath: String,
        inferenceConfigFilePath: String,
        modelId: String,
        epConfigJson: String,
    ): Boolean

    external override fun pushGeminiResponseToQueue(text: String)

    external override fun closeGeminiResponseQueue()
}

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.nativeBridge

import dev.deliteai.datamodels.NimbleNetResult
import dev.deliteai.datamodels.NimbleNetTensor
import dev.deliteai.datamodels.UserEventData
import dev.deliteai.impl.delitePy.proto.impl.ProtoObjectWrapper
import org.json.JSONArray

internal interface CoreRuntime {
    fun initializeNimbleNet(
        context: Any,
        nimbleNetConfig: String,
        assetsJson: JSONArray?,
        nimbleDirPath: String,
        nimbleNetResult: NimbleNetResult<Unit>,
    )

    fun addEvent(
        serializedEventMap: String,
        tableName: String,
        nimbleNetResult: NimbleNetResult<UserEventData>,
    )

    fun addEventProto(
        protoEvent: ProtoObjectWrapper,
        eventType: String,
        nimbleNetResult: NimbleNetResult<UserEventData>,
    )

    fun runMethod(
        methodName: String,
        inputs: Any,
        nimbleNetResult: NimbleNetResult<HashMap<String, NimbleNetTensor>>,
    )

    fun isReady(nimbleNetResult: NimbleNetResult<Unit>)

    fun restartSession(sessionId: String)

    fun writeMetric(metricType: String, metric: String)

    fun networkConnectionEstablishedCallback()

    fun writeRunMethodMetric(id: String, totalTimeInUSecs: Long)

    fun sendEvents(params: String, homeDir: String): Boolean

    // internal functions
    fun reset()

    fun deleteDatabase()

    fun reloadModelWithEpConfig(modelName: String, epConfig: String): Boolean

    fun loadModelFromFile(
        modelFilePath: String,
        inferenceConfigFilePath: String,
        modelId: String,
        epConfigJson: String,
    ): Boolean

    fun pushGeminiResponseToQueue(text: String)

    fun closeGeminiResponseQueue()
}

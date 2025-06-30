/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.impl.controllers

import ai.nimbleedge.datamodels.NimbleNetResult
import ai.nimbleedge.impl.io.FileUtils
import ai.nimbleedge.impl.nativeBridge.CoreRuntime
import android.app.Application

internal class InternalTaskController(
    private val application: Application,
    private val coreRuntime: CoreRuntime,
    private val fileIO: FileUtils,
) {
    private val sdkDirPath by lazy { fileIO.getSDKDirPath() }

    fun sendEvents(params: String): Boolean = coreRuntime.sendEvents(params, sdkDirPath)

    fun resetAppState() {
        coreRuntime.reset()
        coreRuntime.deleteDatabase()
    }

    fun reset() = coreRuntime.reset()

    fun deleteDatabase() = coreRuntime.deleteDatabase()

    fun reloadModelWithEpConfig(modelName: String, epConfig: String): NimbleNetResult<Unit> =
        NimbleNetResult(
            status = coreRuntime.reloadModelWithEpConfig(modelName, epConfig),
            payload = null,
        )

    fun loadModelFromFile(
        modelFilePath: String,
        inferenceConfigFilePath: String,
        modelId: String,
        epConfigJson: String,
    ): NimbleNetResult<Unit> =
        NimbleNetResult(
            status =
                coreRuntime.loadModelFromFile(
                    modelFilePath,
                    inferenceConfigFilePath,
                    modelId,
                    epConfigJson,
                ),
            payload = null,
        )
}

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.controllers

import android.app.Application
import dev.deliteai.datamodels.NimbleNetResult
import dev.deliteai.impl.io.FileUtils
import dev.deliteai.impl.nativeBridge.CoreRuntime

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

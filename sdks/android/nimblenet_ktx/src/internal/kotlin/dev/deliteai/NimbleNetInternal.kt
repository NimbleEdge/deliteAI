/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai

import android.app.Application
import dev.deliteai.datamodels.NimbleNetResult
import dev.deliteai.impl.DependencyContainer
import dev.deliteai.impl.controllers.InternalTaskController

object NimbleNetInternal {
    @Volatile private lateinit var controller: InternalTaskController

    private fun getController(application: Application): InternalTaskController {
        return DependencyContainer.getInstance(null, null).getInternalTaskController().also {
            controller = it
        }
    }

    fun initialize(application: Application) {
        getController(application = application)
    }

    fun resetAppState() {
        controller.resetAppState()
    }

    fun reset() {
        controller.reset()
    }

    fun deleteDatabase() {
        controller.deleteDatabase()
    }

    fun reloadModelWithEpConfig(modelName: String, epConfig: String): NimbleNetResult<Unit> {
        return controller.reloadModelWithEpConfig(modelName, epConfig)
    }

    fun loadModelFromFile(
        modelFilePath: String,
        inferenceConfigFilePath: String,
        modelId: String,
        epConfigJson: String,
    ): NimbleNetResult<Unit> {
        return controller.loadModelFromFile(
            modelFilePath,
            inferenceConfigFilePath,
            modelId,
            epConfigJson,
        )
    }
}

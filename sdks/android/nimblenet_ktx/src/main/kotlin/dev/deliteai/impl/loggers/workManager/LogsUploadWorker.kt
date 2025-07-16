/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.loggers.workManager

import android.app.Application
import android.content.Context
import androidx.work.CoroutineWorker
import androidx.work.WorkerParameters
import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.impl.DependencyContainer
import dev.deliteai.impl.common.WORK_MANAGER.WM_LOGS_UPLOAD_PAYLOAD_ID
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class LogsUploadWorker(context: Context, workerParams: WorkerParameters) :
    CoroutineWorker(context, workerParams) {

    override suspend fun doWork(): Result =
        withContext(Dispatchers.IO) {
            val payload =
                inputData.getString(WM_LOGS_UPLOAD_PAYLOAD_ID)
                    ?: return@withContext Result.failure()

            val container =
                DependencyContainer.getInstance(
                    applicationContext as Application,
                    NimbleNetConfig.fromString(payload),
                )
            val localLogger = container.getLocalLogger()
            val installer = container.getModuleInstaller()
            val taskController = container.getInternalTaskController()

            runCatching {
                    installer.execute()
                    val success = taskController.sendEvents(payload)
                    localLogger.d("send events triggered, status=$success")
                    if (!success) throw IllegalStateException("sendEvents returned false")
                    Result.success()
                }
                .onFailure { localLogger.e(it) }
                .getOrDefault(Result.retry())
        }
}

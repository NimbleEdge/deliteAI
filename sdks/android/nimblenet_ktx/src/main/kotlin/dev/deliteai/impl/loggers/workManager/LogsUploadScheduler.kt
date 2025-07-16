/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.loggers.workManager

import android.app.Application
import androidx.work.BackoffPolicy
import androidx.work.Constraints
import androidx.work.ExistingWorkPolicy
import androidx.work.NetworkType
import androidx.work.OneTimeWorkRequest
import androidx.work.OneTimeWorkRequestBuilder
import androidx.work.WorkManager
import androidx.work.workDataOf
import dev.deliteai.impl.DependencyContainer
import dev.deliteai.impl.common.WORK_MANAGER
import dev.deliteai.impl.common.WORK_MANAGER.WM_LOGS_UPLOAD_PAYLOAD_ID
import java.util.concurrent.TimeUnit

internal class LogsUploadScheduler(private val application: Application) {
    private val workManager = WorkManager.getInstance(application)
    private val dependencyContainer = DependencyContainer.getInstance(application, null)
    private val neLogger = dependencyContainer.getLocalLogger()

    fun schedule(
        initialDelayInSeconds: Long,
        retryIntervalInSecondsIfFailed: Long,
        payload: String,
    ) {
        val oneTimeWorkRequest =
            buildOneTimeWorkRequest(payload, initialDelayInSeconds, retryIntervalInSecondsIfFailed)

        workManager.enqueueUniqueWork(
            WORK_MANAGER.WM_LOGS_UPLOAD_JOB_ID,
            ExistingWorkPolicy.REPLACE,
            oneTimeWorkRequest,
        )

        neLogger.d("logs upload scheduled with payload")
    }

    private fun buildOneTimeWorkRequest(
        payload: String,
        initialDelayInSeconds: Long,
        retryIntervalInSecondsIfFailed: Long,
    ): OneTimeWorkRequest {
        val workerInputParam = workDataOf(WM_LOGS_UPLOAD_PAYLOAD_ID to payload)
        val constraints =
            Constraints.Builder().setRequiredNetworkType(NetworkType.CONNECTED).build()

        return OneTimeWorkRequestBuilder<LogsUploadWorker>()
            .setInitialDelay(initialDelayInSeconds, TimeUnit.SECONDS)
            .setConstraints(constraints)
            .setBackoffCriteria(
                BackoffPolicy.LINEAR,
                retryIntervalInSecondsIfFailed,
                TimeUnit.SECONDS,
            )
            .setInputData(workerInputParam)
            .build()
    }
}

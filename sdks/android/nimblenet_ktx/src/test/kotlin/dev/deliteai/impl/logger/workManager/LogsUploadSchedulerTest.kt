/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.logger.workManager

import android.app.Application
import androidx.work.BackoffPolicy
import androidx.work.Constraints
import androidx.work.ExistingWorkPolicy
import androidx.work.NetworkType
import androidx.work.OneTimeWorkRequest
import androidx.work.WorkManager
import dev.deliteai.impl.DependencyContainer
import dev.deliteai.impl.common.WORK_MANAGER
import dev.deliteai.impl.loggers.LocalLogger
import dev.deliteai.impl.loggers.workManager.LogsUploadScheduler
import dev.deliteai.impl.loggers.workManager.LogsUploadWorker
import io.mockk.MockKAnnotations
import io.mockk.every
import io.mockk.mockk
import io.mockk.mockkObject
import io.mockk.slot
import io.mockk.verify
import java.util.concurrent.TimeUnit
import kotlin.test.assertEquals
import org.junit.Before
import org.junit.Test

class LogsUploadSchedulerTest {
    private var applicationMocked: Application = mockk(relaxed = true)
    private var workManagerMocked: WorkManager = mockk(relaxed = true)
    private var dependencyContainerMocked: DependencyContainer = mockk(relaxed = true)
    private var localLoggerMocked: LocalLogger = mockk(relaxed = true)
    private var logsUploadScheduler: LogsUploadScheduler = mockk(relaxed = true)

    @Before
    fun setUp() {
        MockKAnnotations.init(this, relaxUnitFun = true)

        mockkObject(WorkManager.Companion)
        mockkObject(DependencyContainer.Companion)
        every { WorkManager.getInstance(applicationMocked) } returns workManagerMocked
        every { dependencyContainerMocked.getLocalLogger() } returns localLoggerMocked
        every { DependencyContainer.getInstance(any(), any()) } returns dependencyContainerMocked
        logsUploadScheduler = LogsUploadScheduler(applicationMocked)
    }

    @Test
    fun `schedule should enqueue unique work with correct parameters`() {
        val initialDelayInSeconds = 60L
        val retryIntervalInSecondsIfFailed = 120L
        val payload = "test_payload"

        logsUploadScheduler.schedule(
            initialDelayInSeconds = initialDelayInSeconds,
            retryIntervalInSecondsIfFailed = retryIntervalInSecondsIfFailed,
            payload = payload,
        )

        val workRequestSlot = slot<OneTimeWorkRequest>()
        verify(exactly = 1) {
            workManagerMocked.enqueueUniqueWork(
                WORK_MANAGER.WM_LOGS_UPLOAD_JOB_ID,
                ExistingWorkPolicy.REPLACE,
                capture(workRequestSlot),
            )
        }

        val capturedWorkRequest = workRequestSlot.captured

        assertEquals(
            LogsUploadWorker::class.java.name,
            capturedWorkRequest.workSpec.workerClassName,
        )

        assertEquals(
            initialDelayInSeconds * 1000,
            capturedWorkRequest.workSpec.initialDelay,
            "Initial delay should match",
        )

        val expectedConstraints =
            Constraints.Builder().setRequiredNetworkType(NetworkType.CONNECTED).build()
        assertEquals(
            expectedConstraints,
            capturedWorkRequest.workSpec.constraints,
            "Constraints should match",
        )

        assertEquals(
            BackoffPolicy.LINEAR,
            capturedWorkRequest.workSpec.backoffPolicy,
            "Backoff policy should be LINEAR",
        )

        assertEquals(
            TimeUnit.SECONDS.toMillis(retryIntervalInSecondsIfFailed),
            capturedWorkRequest.workSpec.backoffDelayDuration,
            "Backoff delay duration should match",
        )
    }
}

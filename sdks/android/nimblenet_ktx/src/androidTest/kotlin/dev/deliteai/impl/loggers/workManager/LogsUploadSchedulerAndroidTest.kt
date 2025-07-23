/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.loggers.workManager

import android.app.Application
import androidx.test.core.app.ApplicationProvider
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.work.WorkInfo
import androidx.work.WorkManager
import dev.deliteai.impl.DependencyContainer
import dev.deliteai.impl.common.WORK_MANAGER
import dev.deliteai.testUtils.nnConfig
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withTimeoutOrNull
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class LogsUploadSchedulerAndroidTest {

    private lateinit var application: Application
    private lateinit var workManager: WorkManager
    private lateinit var logsUploadScheduler: LogsUploadScheduler

    @Before
    fun setup() {
        application = ApplicationProvider.getApplicationContext()
        workManager = WorkManager.getInstance(application)

        runBlocking {
            // Cancel any existing work
            workManager.cancelUniqueWork(WORK_MANAGER.WM_LOGS_UPLOAD_JOB_ID)

            withTimeoutOrNull(5000L) {
                while (true) {
                    val workInfoList =
                        workManager
                            .getWorkInfosForUniqueWork(WORK_MANAGER.WM_LOGS_UPLOAD_JOB_ID)
                            .get()
                    if (workInfoList.isEmpty() || workInfoList.all { it.state.isFinished }) {
                        break
                    }
                    delay(100)
                }
            }

            // Remove the job manually if it still exists
            workManager.pruneWork()

            val dc = DependencyContainer.getInstance(application, nnConfig)
            logsUploadScheduler = dc.getLogsUploadScheduler()
        }
    }

    @Test
    fun scheduleShouldEnqueueWorkCorrectly() {
        assertTrue(
            "WorkManager should have no scheduled jobs before test",
            workManager
                .getWorkInfosForUniqueWork(WORK_MANAGER.WM_LOGS_UPLOAD_JOB_ID)
                .get()
                .isEmpty(),
        )

        val initialDelay = 10L
        val retryInterval = 30L
        val payload = "{}"

        logsUploadScheduler.schedule(initialDelay, retryInterval, payload)

        val workInfoList =
            workManager.getWorkInfosForUniqueWork(WORK_MANAGER.WM_LOGS_UPLOAD_JOB_ID).get()
        assertTrue(workInfoList.size == 1)

        val workInfo = workInfoList.firstOrNull()
        assertNotNull(workInfo)
        assertEquals(WorkInfo.State.ENQUEUED, workInfo!!.state)

        assertTrue(
            "LogsUploadWorker should be scheduled",
            workInfo.tags.contains(LogsUploadWorker::class.java.name),
        )
    }

    @Test
    fun scheduleShouldReplaceExistingWork() {
        assertTrue(
            "WorkManager should have no scheduled jobs before test",
            workManager
                .getWorkInfosForUniqueWork(WORK_MANAGER.WM_LOGS_UPLOAD_JOB_ID)
                .get()
                .isEmpty(),
        )

        val initialDelay = 5L
        val retryInterval = 20L
        val payload = "{}"
        val updatedPayload = JSONObject(mapOf("hello" to "world")).toString()

        logsUploadScheduler.schedule(initialDelay, retryInterval, payload)
        logsUploadScheduler.schedule(initialDelay, retryInterval, updatedPayload)

        val workInfoList =
            workManager.getWorkInfosForUniqueWork(WORK_MANAGER.WM_LOGS_UPLOAD_JOB_ID).get()
        assertTrue(workInfoList.size == 1)

        val workInfo = workInfoList.firstOrNull()
        assertNotNull(workInfo)
        assertEquals(WorkInfo.State.ENQUEUED, workInfo!!.state)
    }
}

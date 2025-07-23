/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.logger.workManager

import android.app.Application
import androidx.work.Data
import androidx.work.ListenableWorker.Result
import androidx.work.WorkerParameters
import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.impl.DependencyContainer
import dev.deliteai.impl.common.WORK_MANAGER.WM_LOGS_UPLOAD_PAYLOAD_ID
import dev.deliteai.impl.controllers.InternalTaskController
import dev.deliteai.impl.loggers.workManager.LogsUploadWorker
import dev.deliteai.impl.moduleInstallers.ModuleInstaller
import dev.deliteai.testUtils.nnConfig
import io.mockk.MockKAnnotations
import io.mockk.Runs
import io.mockk.coEvery
import io.mockk.coVerify
import io.mockk.every
import io.mockk.just
import io.mockk.mockk
import io.mockk.mockkObject
import io.mockk.verify
import kotlin.test.assertEquals
import kotlinx.coroutines.runBlocking
import org.json.JSONObject
import org.junit.Before
import org.junit.Test

class LogsUploadWorkerTest {
    private var applicationMocked: Application = mockk(relaxed = true)
    private var workerParamsMocked: WorkerParameters = mockk(relaxed = true)
    private var internalTaskControllerMocked: InternalTaskController = mockk(relaxed = true)
    private var libraryLoaderMocked: ModuleInstaller = mockk(relaxed = true)
    private val payload = JSONObject(mapOf("deviceConfig" to "{}")).toString()
    private var workerParams: WorkerParameters = mockk(relaxed = true)
    private val payloadId = "somePayloadString"
    private val data = mockk<Data>()

    @Before
    fun setUp() {
        MockKAnnotations.init(this, relaxUnitFun = true)

        val dependencyContainerMocked: DependencyContainer = mockk(relaxed = true)
        mockkObject(DependencyContainer.Companion)
        every { DependencyContainer.getInstance(any(), any()) } returns dependencyContainerMocked
        every { dependencyContainerMocked.getModuleInstaller() } returns libraryLoaderMocked
        every { dependencyContainerMocked.getInternalTaskController() } returns
            internalTaskControllerMocked

        mockkObject(NimbleNetConfig.Companion)
        every { NimbleNetConfig.fromString(any()) } returns nnConfig

        every { data.getString(WM_LOGS_UPLOAD_PAYLOAD_ID) } returns payloadId
        every { workerParams.inputData } returns data
    }

    @Test
    fun `doWork should return success when sendEvents is successful`() = runBlocking {
        val data = mockk<Data>()
        every { data.getString(WM_LOGS_UPLOAD_PAYLOAD_ID) } returns "{}"
        every { workerParamsMocked.inputData } returns data

        coEvery { libraryLoaderMocked.execute() } just Runs
        every { internalTaskControllerMocked.sendEvents(any()) } returns true

        val logsUploadWorker = LogsUploadWorker(applicationMocked, workerParamsMocked)
        val actualResult = logsUploadWorker.doWork()

        assertEquals(
            Result.success(),
            actualResult,
            "Worker should return Result.success() when sendEvents is successful",
        )

        coVerify(exactly = 1) { libraryLoaderMocked.execute() }
        verify(exactly = 1) { internalTaskControllerMocked.sendEvents(any()) }
    }

    @Test
    fun `doWork should return failure when payload is missing`() = runBlocking {
        val data = mockk<Data>()
        every { data.getString(WM_LOGS_UPLOAD_PAYLOAD_ID) } returns null
        every { workerParamsMocked.inputData } returns data

        val worker = LogsUploadWorker(applicationMocked, workerParamsMocked)
        val actualResult = worker.doWork()

        assertEquals(
            Result.failure(),
            actualResult,
            "Worker should return Result.failure() when payload is missing",
        )

        coVerify(exactly = 0) { libraryLoaderMocked.execute() }
        verify(exactly = 0) { internalTaskControllerMocked.sendEvents(any()) }
    }

    @Test
    fun `doWork should return retry when sendEvents fails`() = runBlocking {
        every { internalTaskControllerMocked.sendEvents(payload) } returns false

        val worker = LogsUploadWorker(applicationMocked, workerParamsMocked)
        val result = worker.doWork()

        assertEquals(
            Result.retry(),
            result,
            "Worker should return Result.retry() when sendEvents fails",
        )

        coVerify(exactly = 1) { libraryLoaderMocked.execute() }
        verify(exactly = 1) { internalTaskControllerMocked.sendEvents(any()) }
    }

    @Test
    fun `doWork should return retry when exception occurs during execution`() = runBlocking {
        coEvery { libraryLoaderMocked.execute() } throws Exception("Test exception")

        val worker = LogsUploadWorker(applicationMocked, workerParamsMocked)
        val result = worker.doWork()

        assertEquals(
            Result.retry(),
            result,
            "Worker should return Result.retry() when an exception occurs",
        )

        coVerify(exactly = 1) { libraryLoaderMocked.execute() }
        verify(exactly = 0) { internalTaskControllerMocked.sendEvents(any()) }
    }
}

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.io

import android.app.Application
import android.app.DownloadManager
import android.database.Cursor
import android.os.Environment
import dev.deliteai.impl.common.STATUS_CODES
import dev.deliteai.impl.io.datamodels.DownloadManagerCursor
import dev.deliteai.impl.io.datamodels.DownloadTask
import dev.deliteai.impl.loggers.LocalLogger
import io.mockk.every
import io.mockk.mockk
import io.mockk.slot
import io.mockk.spyk
import io.mockk.verify
import java.io.File
import org.junit.Assert.assertEquals
import org.junit.Before
import org.junit.Test

class ChunkDownloadManagerTest {
    private val applicationMocked: Application = mockk(relaxed = true)
    private val downloadManagerMocked: DownloadManager = mockk(relaxed = true)
    private val appPreferencesStoreMocked: AppPreferencesStore = mockk(relaxed = true)
    private val fileUtilsMocked: FileUtils = mockk(relaxed = true)
    private val localLoggerMocked: LocalLogger = mockk(relaxed = true)
    private val fileName = "testFile"
    private val url = "https://example.com/file"
    private val mockkDownloadManagerId = 123L

    @Before
    fun setUp() {
        every { applicationMocked.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS) } returns
            null
        every { downloadManagerMocked.remove(any()) } returns 0
    }

    @Test
    fun `non cached download should trigger DM and return pending`() {
        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )
        val mockkDownloadManagerRequest = mockk<DownloadManager.Request>()

        every { chunkManager.retrieveCachedDownloadTask(any()) } returns null

        every { chunkManager.createDownloadManagerRequest(any(), any(), any()) } returns
            mockkDownloadManagerRequest

        every { downloadManagerMocked.enqueue(any()) } returns mockkDownloadManagerId

        val res = chunkManager.run(fileName, url, "", "")

        assertEquals(res.currentState, STATUS_CODES.FILE_DOWNLOAD_PENDING)
        assertEquals(res.previousState, STATUS_CODES.FILE_DOWNLOAD_UNKNOWN)
        assertEquals(res.downloadManagerDownloadId, mockkDownloadManagerId)

        verify(exactly = 1) {
            chunkManager.createOrUpdateDownloadTaskCache(
                mockkDownloadManagerId,
                STATUS_CODES.FILE_DOWNLOAD_PENDING,
                any(),
            )
        }
    }

    @Test
    fun `cached task with PENDING status and current as RUNNING should return running and update cache`() {
        every { appPreferencesStoreMocked.get(fileName) } returns
            DownloadTask(mockkDownloadManagerId, STATUS_CODES.FILE_DOWNLOAD_PENDING, 0L).toString()

        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        every {
            chunkManager["queryDownloadStatusFromDownloadManager"](mockkDownloadManagerId)
        } returns
            DownloadManagerCursor(
                status = DownloadManager.STATUS_RUNNING,
                reasonCode = 0,
                lastModifiedAt = 100L,
            )

        val result = chunkManager.run(fileName, url, "", "")

        assertEquals(mockkDownloadManagerId, result.downloadManagerDownloadId)
        assertEquals(STATUS_CODES.FILE_DOWNLOAD_RUNNING, result.currentState)
        assertEquals(STATUS_CODES.FILE_DOWNLOAD_PENDING, result.previousState)

        verify(exactly = 1) {
            chunkManager.createOrUpdateDownloadTaskCache(
                mockkDownloadManagerId,
                STATUS_CODES.FILE_DOWNLOAD_RUNNING,
                fileName,
            )
        }
    }

    @Test
    fun `cached task with RUNNING status and current as PAUSED should return paused`() {
        every { appPreferencesStoreMocked.get(any()) } returns
            DownloadTask(mockkDownloadManagerId, STATUS_CODES.FILE_DOWNLOAD_RUNNING, 0L).toString()

        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        every {
            chunkManager["queryDownloadStatusFromDownloadManager"](mockkDownloadManagerId)
        } returns
            DownloadManagerCursor(
                status = DownloadManager.STATUS_PAUSED,
                reasonCode = 0,
                lastModifiedAt = 100L,
            )

        val result = chunkManager.run(fileName, url, "", "")

        assertEquals(mockkDownloadManagerId, result.downloadManagerDownloadId)
        assertEquals(STATUS_CODES.FILE_DOWNLOAD_PAUSED, result.currentState)
        assertEquals(STATUS_CODES.FILE_DOWNLOAD_RUNNING, result.previousState)

        verify(exactly = 1) {
            chunkManager.createOrUpdateDownloadTaskCache(
                mockkDownloadManagerId,
                STATUS_CODES.FILE_DOWNLOAD_PAUSED,
                fileName,
            )
        }
    }

    @Test
    fun `cached task with PAUSED status and current as SUCCESS should return success`() {
        every { appPreferencesStoreMocked.get(any()) } returns
            DownloadTask(mockkDownloadManagerId, STATUS_CODES.FILE_DOWNLOAD_PAUSED, 0L).toString()

        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        every {
            chunkManager["queryDownloadStatusFromDownloadManager"](mockkDownloadManagerId)
        } returns
            DownloadManagerCursor(
                status = DownloadManager.STATUS_SUCCESSFUL,
                reasonCode = 0,
                lastModifiedAt = 100L,
            )
        every { chunkManager.moveFileFromDownloadsDirToTarget(any(), any()) } returns true

        val result = chunkManager.run(fileName, url, "", "")

        assertEquals(mockkDownloadManagerId, result.downloadManagerDownloadId)
        assertEquals(STATUS_CODES.FILE_DOWNLOAD_SUCCESS, result.currentState)
        assertEquals(STATUS_CODES.FILE_DOWNLOAD_PAUSED, result.previousState)

        verify(exactly = 0) { chunkManager.createOrUpdateDownloadTaskCache(any(), any(), any()) }
    }

    @Test
    fun `cached task with PAUSED status and current as FAILURE should return failure`() {
        every { appPreferencesStoreMocked.get(any()) } returns
            DownloadTask(mockkDownloadManagerId, STATUS_CODES.FILE_DOWNLOAD_PAUSED, 0L).toString()

        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        every {
            chunkManager["queryDownloadStatusFromDownloadManager"](mockkDownloadManagerId)
        } returns
            DownloadManagerCursor(
                status = DownloadManager.STATUS_FAILED,
                reasonCode = 0,
                lastModifiedAt = 100L,
            )
        every { chunkManager.moveFileFromDownloadsDirToTarget(any(), any()) } returns true

        val result = chunkManager.run(fileName, url, "", "")

        assertEquals(mockkDownloadManagerId, result.downloadManagerDownloadId)
        assertEquals(STATUS_CODES.FILE_DOWNLOAD_FAILURE, result.currentState)
        assertEquals(STATUS_CODES.FILE_DOWNLOAD_PAUSED, result.previousState)

        verify(exactly = 0) { chunkManager.createOrUpdateDownloadTaskCache(any(), any(), any()) }
    }

    @Test
    fun `cached download task shall be removed in case of FAILURE from DM`() {
        every { appPreferencesStoreMocked.get(any()) } returns
            DownloadTask(mockkDownloadManagerId, STATUS_CODES.FILE_DOWNLOAD_UNKNOWN, 0L).toString()

        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        every {
            chunkManager["queryDownloadStatusFromDownloadManager"](mockkDownloadManagerId)
        } returns
            DownloadManagerCursor(
                status = DownloadManager.STATUS_FAILED,
                reasonCode = 0,
                lastModifiedAt = 100L,
            )
        every { chunkManager.moveFileFromDownloadsDirToTarget(any(), any()) } returns true

        chunkManager.run(fileName, url, "", "")

        verify(exactly = 1) { appPreferencesStoreMocked.delete(any()) }

        verify(exactly = 0) { chunkManager.createOrUpdateDownloadTaskCache(any(), any(), any()) }
    }

    @Test
    fun `cached download task shall be removed in case of FAILURE while moving downlaoded file`() {
        every { appPreferencesStoreMocked.get(any()) } returns
            DownloadTask(mockkDownloadManagerId, STATUS_CODES.FILE_DOWNLOAD_UNKNOWN, 0L).toString()

        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        every {
            chunkManager["queryDownloadStatusFromDownloadManager"](mockkDownloadManagerId)
        } returns
            DownloadManagerCursor(
                status = DownloadManager.STATUS_FAILED,
                reasonCode = 0,
                lastModifiedAt = 100L,
            )

        every { chunkManager.moveFileFromDownloadsDirToTarget(any(), any()) } returns false

        chunkManager.run(fileName, url, "", "")

        verify(exactly = 1) { appPreferencesStoreMocked.delete(any()) }

        verify(exactly = 0) { chunkManager.createOrUpdateDownloadTaskCache(any(), any(), any()) }
    }

    @Test
    fun `cached download task shall be removed in case of SUCCESSFUL download and move`() {
        every { appPreferencesStoreMocked.get(any()) } returns
            DownloadTask(mockkDownloadManagerId, STATUS_CODES.FILE_DOWNLOAD_UNKNOWN, 0L).toString()

        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        every {
            chunkManager["queryDownloadStatusFromDownloadManager"](mockkDownloadManagerId)
        } returns
            DownloadManagerCursor(
                status = DownloadManager.STATUS_FAILED,
                reasonCode = 0,
                lastModifiedAt = 100L,
            )

        every { chunkManager.moveFileFromDownloadsDirToTarget(any(), any()) } returns true

        chunkManager.run(fileName, url, "", "")

        verify(exactly = 1) { appPreferencesStoreMocked.delete(any()) }

        verify(exactly = 0) { chunkManager.createOrUpdateDownloadTaskCache(any(), any(), any()) }
    }

    @Test
    fun `invalid or incompatible json in cache should remove from cache and create new request`() {
        val invalidDownloadTaskCache = "{}"

        every { appPreferencesStoreMocked.get(any()) } returns invalidDownloadTaskCache

        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        val res = chunkManager.retrieveCachedDownloadTask(fileName)

        assertEquals(res, null)

        verify(exactly = 1) { appPreferencesStoreMocked.delete(any()) }

        verify(exactly = 0) { appPreferencesStoreMocked.put(any(), any()) }
    }

    @Test
    fun `retrieveCachedDownloadTask returns null when cache is empty`() {
        every { appPreferencesStoreMocked.get(any()) } returns null

        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        val result = chunkManager.retrieveCachedDownloadTask(fileName)

        assertEquals(null, result)
        verify(exactly = 0) { appPreferencesStoreMocked.delete(any()) }
    }

    @Test
    fun `retrieveCachedDownloadTask returns DownloadTask when cache contains valid json`() {
        val validTask = DownloadTask(mockkDownloadManagerId, STATUS_CODES.FILE_DOWNLOAD_PENDING, 0L)
        every { appPreferencesStoreMocked.get(any()) } returns validTask.toString()

        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        val result = chunkManager.retrieveCachedDownloadTask(fileName)

        assertEquals(validTask, result)
        verify(exactly = 0) { appPreferencesStoreMocked.delete(any()) }
    }

    @Test
    fun `createOrUpdateDownloadTaskCache stores valid DownloadTask in cache`() {
        val currentState = STATUS_CODES.FILE_DOWNLOAD_PENDING
        val startTime = System.currentTimeMillis()
        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        val capturedJson = slot<String>()

        chunkManager.createOrUpdateDownloadTaskCache(mockkDownloadManagerId, currentState, fileName)

        verify(exactly = 1) { appPreferencesStoreMocked.put(fileName, capture(capturedJson)) }

        val cachedTask = DownloadTask.fromJsonString(capturedJson.captured)

        assertEquals(mockkDownloadManagerId, cachedTask.downloadManagerId)
        assertEquals(currentState, cachedTask.latestState)

        val currentTime = System.currentTimeMillis()
        assert(cachedTask.stateStartTime in startTime..currentTime)
    }

    @Test
    fun `moveFileFromDownloadsDirToTarget returns false when file does not exist`() {
        val downloadsDir = createTempDir("downloads")
        val targetDir = createTempDir("target").absolutePath
        val fileName = "testFile"

        val sourceFile = File(downloadsDir, fileName)
        if (sourceFile.exists()) {
            sourceFile.delete()
        }

        every { applicationMocked.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS) } returns
            downloadsDir

        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        val result = chunkManager.moveFileFromDownloadsDirToTarget(fileName, targetDir)

        assertEquals(false, result)
        verify(exactly = 0) { fileUtilsMocked.moveFile(any(), any()) }
    }

    @Test
    fun `queryDownloadStatusFromDownloadManager returns null when cursor is empty`() {
        val mockedCursor = mockk<Cursor>()
        every { mockedCursor.moveToFirst() } returns false

        every { downloadManagerMocked.query(any()) } returns mockedCursor

        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        val result = chunkManager.queryDownloadStatusFromDownloadManager(mockkDownloadManagerId)

        assertEquals(null, result)
    }

    @Test
    fun `queryDownloadStatusFromDownloadManager returns valid DownloadManagerCursor when cursor has data`() {
        val mockedCursor = mockk<Cursor>()
        every { mockedCursor.moveToFirst() } returns true

        // Stub column index lookups for each expected column.
        every { mockedCursor.getColumnIndex(DownloadManager.COLUMN_STATUS) } returns 0
        every { mockedCursor.getColumnIndex(DownloadManager.COLUMN_REASON) } returns 1
        every {
            mockedCursor.getColumnIndex(DownloadManager.COLUMN_LAST_MODIFIED_TIMESTAMP)
        } returns 2

        val expectedStatus: Long = 2L
        val expectedReason: Long = 0L
        val expectedLastModified: Long = 123456789L
        every { mockedCursor.getLong(0) } returns expectedStatus
        every { mockedCursor.getLong(1) } returns expectedReason
        every { mockedCursor.getLong(2) } returns expectedLastModified

        every { downloadManagerMocked.query(any()) } returns mockedCursor

        val chunkManager =
            spyk(
                ChunkDownloadManager(
                    application = applicationMocked,
                    downloadManager = downloadManagerMocked,
                    appPreferencesStore = appPreferencesStoreMocked,
                    fileUtils = fileUtilsMocked,
                    localLogger = localLoggerMocked,
                    showDownloadProgress = false,
                )
            )

        val result = chunkManager.queryDownloadStatusFromDownloadManager(mockkDownloadManagerId)

        assertEquals(expectedStatus.toInt(), result?.status)
        assertEquals(expectedReason.toInt(), result?.reasonCode)
        assertEquals(expectedLastModified, result?.lastModifiedAt)
    }
}

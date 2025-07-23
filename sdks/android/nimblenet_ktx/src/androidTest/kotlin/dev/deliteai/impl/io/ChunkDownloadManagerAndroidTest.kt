/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.io

import android.app.Application
import android.app.DownloadManager
import android.content.Context
import android.os.Environment
import androidx.test.core.app.ApplicationProvider
import androidx.test.platform.app.InstrumentationRegistry
import dev.deliteai.impl.DependencyContainer
import dev.deliteai.impl.loggers.LocalLogger
import dev.deliteai.testUtils.nnConfig
import java.io.File
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Assert.fail
import org.junit.Before
import org.junit.Test

class ChunkDownloadManagerAndroidTest {
    private val context = ApplicationProvider.getApplicationContext<Application>()
    private val fileName = "testFile.tmp"
    private val url = "http://example.com/test"
    private val requestHeaders = "{}"
    private val targetDir = context.filesDir.absolutePath
    private val largeTestUrl = "https://ash-speed.hetzner.com/100MB.bin"
    private val downloadManager =
        context.getSystemService(Context.DOWNLOAD_SERVICE) as DownloadManager
    private lateinit var networking: Networking
    private lateinit var appPreferencesStore: AppPreferencesStore
    private lateinit var fileUtils: FileUtils
    private lateinit var localLogger: LocalLogger

    @Before
    fun setup() {
        val dependencyContainer = DependencyContainer.getInstance(context, nnConfig)
        networking = dependencyContainer.getNetworking()
        appPreferencesStore = dependencyContainer.getAppPreferencesStore()
        fileUtils = dependencyContainer.getFileUtils()
        localLogger = dependencyContainer.getLocalLogger()
    }

    @Test
    fun integrationTest_createDownloadManagerRequestDeletesStaleFile() {
        val downloadsDir = context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS)
        assertNotNull("Downloads directory must not be null", downloadsDir)

        // Create a stale file in the downloads directory
        val staleFile = File(downloadsDir, fileName)
        staleFile.writeText("stale content")
        assertTrue("Stale file must exist before calling method", staleFile.exists())

        val chunkManager =
            ChunkDownloadManager(
                context,
                downloadManager,
                appPreferencesStore,
                fileUtils,
                localLogger,
                nnConfig.showDownloadProgress,
            )

        val request =
            chunkManager.createDownloadManagerRequest(
                fileName = fileName,
                url = url,
                requestHeaders = requestHeaders,
            )

        assertFalse(
            "Stale file should be deleted after calling createDownloadManagerRequest",
            staleFile.exists(),
        )

        assertNotNull("A valid DownloadManager.Request should be returned", request)
    }

    @Test
    fun moveFileFromDownloadsDirToTargetReturnsTrueWhenFileExistsAndMovedSuccessfully() {
        val downloadsDir = context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS)
        assertNotNull("Downloads directory must not be null", downloadsDir)

        val fileToMove = File(downloadsDir, fileName)
        fileToMove.writeText("some content")
        assertTrue(
            "Test file must exist before moveFileFromDownloadsDirToTarget",
            fileToMove.exists(),
        )

        val chunkManager =
            ChunkDownloadManager(
                context,
                downloadManager,
                appPreferencesStore,
                fileUtils,
                localLogger,
                nnConfig.showDownloadProgress,
            )

        val result = chunkManager.moveFileFromDownloadsDirToTarget(fileName, targetDir)

        assertTrue("Expected file to be moved successfully", result)

        assertFalse("File should no longer exist in the downloads directory", fileToMove.exists())

        val movedFile = File(targetDir, fileName)
        assertTrue("File should now exist in the target directory", movedFile.exists())
    }

    @Test
    fun downloadManagerMustResumeDownloadAfterInterruption() = runBlocking {
        val testFileName = "large_test_file.zip"
        val requestHeaders = "{}"
        val targetDir = context.filesDir.absolutePath

        val downloadsDir = context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS)
        val staleFile = File(downloadsDir, testFileName)
        staleFile.delete()

        val chunkManager =
            ChunkDownloadManager(
                context,
                downloadManager,
                appPreferencesStore,
                fileUtils,
                localLogger,
                nnConfig.showDownloadProgress,
            )

        val initialTransition =
            chunkManager.run(
                fileName = fileName,
                url = largeTestUrl,
                requestHeaders = requestHeaders,
                targetDirectory = targetDir,
            )
        val downloadId = initialTransition.downloadManagerDownloadId

        assertTrue("DownloadManager ID should be valid", downloadId > 0)

        var bytesBeforeInterruption = -1L

        val maxWaitTimeMs = 30000L
        var waitedTime = 0L
        while (waitedTime < maxWaitTimeMs) {
            val temp = getDownloadedBytesSoFar(downloadId)
            if (temp > 0) {
                delay(5000)
                bytesBeforeInterruption = temp
                break
            } else {
                delay(100)
                waitedTime += 100
            }
        }
        if (bytesBeforeInterruption <= 0) {
            fail("Download did not progress within expected time")
        }

        interruptDownload()
        delay(5000)
        restoreInternetConnectivity()

        waitedTime = 0L
        while (waitedTime < maxWaitTimeMs) {
            val currentDownloadedBytes = getDownloadedBytesSoFar(downloadId)
            if (currentDownloadedBytes < bytesBeforeInterruption) {
                fail("Download started from the beginning")
            } else if (currentDownloadedBytes > bytesBeforeInterruption) break

            delay(1000)
            waitedTime += 1000
        }
    }

    private fun getDownloadedBytesSoFar(downloadId: Long): Long {
        val query = DownloadManager.Query().setFilterById(downloadId)
        downloadManager.query(query).use { cursor ->
            if (cursor.moveToFirst()) {
                return cursor.getLong(
                    cursor.getColumnIndexOrThrow(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR)
                )
            }
        }
        return 0
    }

    private fun interruptDownload() {
        val uiAutomation = InstrumentationRegistry.getInstrumentation().uiAutomation
        uiAutomation.executeShellCommand("svc wifi disable")
        uiAutomation.executeShellCommand("svc data disable")
    }

    private fun restoreInternetConnectivity() {
        val uiAutomation = InstrumentationRegistry.getInstrumentation().uiAutomation
        uiAutomation.executeShellCommand("svc wifi enable")
        uiAutomation.executeShellCommand("svc data enable")
    }
}

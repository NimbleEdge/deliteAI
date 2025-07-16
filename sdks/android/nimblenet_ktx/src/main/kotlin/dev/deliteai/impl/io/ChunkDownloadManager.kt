/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.io

import android.annotation.SuppressLint
import android.app.Application
import android.app.DownloadManager
import android.net.Uri
import android.os.Environment
import androidx.annotation.VisibleForTesting
import dev.deliteai.impl.common.ERROR_CODES.DM_UNABLE_TO_CREATE_REQUEST
import dev.deliteai.impl.common.STATUS_CODES
import dev.deliteai.impl.common.utils.jsonStringToHeaders
import dev.deliteai.impl.io.datamodels.DownloadManagerCursor
import dev.deliteai.impl.io.datamodels.DownloadTask
import dev.deliteai.impl.io.datamodels.FileDownloadStateTransition
import dev.deliteai.impl.loggers.LocalLogger
import java.io.File

internal class ChunkDownloadManager(
    private val application: Application,
    private val downloadManager: DownloadManager,
    private val appPreferencesStore: AppPreferencesStore,
    private val fileUtils: FileUtils,
    private val localLogger: LocalLogger,
    private val showDownloadProgress: Boolean,
) {
    fun run(
        fileName: String,
        url: String,
        requestHeaders: String,
        targetDirectory: String,
    ): FileDownloadStateTransition {
        val cachedTask = retrieveCachedDownloadTask(fileName)

        // If no task is cached, create a new download request
        if (cachedTask == null) {
            val request =
                createDownloadManagerRequest(fileName, url, requestHeaders)
                    ?: return FileDownloadStateTransition(
                        downloadManagerDownloadId = -1L,
                        currentState = STATUS_CODES.FILE_DOWNLOAD_FAILURE,
                        previousState = STATUS_CODES.FILE_DOWNLOAD_UNKNOWN,
                        timeTaken = 0L,
                        currentStateReasonCode = DM_UNABLE_TO_CREATE_REQUEST,
                    )

            // Enqueue the new request and update the cache with a pending state
            val dmDownloadId = downloadManager.enqueue(request)
            createOrUpdateDownloadTaskCache(
                downloadManagerDownloadId = dmDownloadId,
                currentState = STATUS_CODES.FILE_DOWNLOAD_PENDING,
                fileName = fileName,
            )

            return FileDownloadStateTransition(
                downloadManagerDownloadId = dmDownloadId,
                currentState = STATUS_CODES.FILE_DOWNLOAD_PENDING,
                previousState = STATUS_CODES.FILE_DOWNLOAD_UNKNOWN,
                timeTaken = 0L,
            )
        }

        // If there is an existing cached task, update its status
        val dmDownloadId = cachedTask.downloadManagerId
        val previousState = cachedTask.latestState
        val stateStartTime = cachedTask.stateStartTime

        val currentStatusCursor = queryDownloadStatusFromDownloadManager(dmDownloadId)
        val reasonCode = currentStatusCursor?.reasonCode ?: -1
        val stateEndTime = currentStatusCursor?.lastModifiedAt ?: System.currentTimeMillis()
        var currentState = STATUS_CODES.FILE_DOWNLOAD_UNKNOWN

        when (currentStatusCursor?.status) {
            DownloadManager.STATUS_SUCCESSFUL -> {
                currentState =
                    if (moveFileFromDownloadsDirToTarget(fileName, targetDirectory)) {
                        STATUS_CODES.FILE_DOWNLOAD_SUCCESS
                    } else {
                        STATUS_CODES.FILE_DOWNLOAD_FAILURE
                    }

                appPreferencesStore.delete(fileName)
            }

            DownloadManager.STATUS_PENDING -> {
                currentState = STATUS_CODES.FILE_DOWNLOAD_PENDING
                createOrUpdateDownloadTaskCache(dmDownloadId, currentState, fileName)
            }

            DownloadManager.STATUS_RUNNING -> {
                currentState = STATUS_CODES.FILE_DOWNLOAD_RUNNING
                createOrUpdateDownloadTaskCache(dmDownloadId, currentState, fileName)
            }

            DownloadManager.STATUS_PAUSED -> {
                currentState = STATUS_CODES.FILE_DOWNLOAD_PAUSED
                createOrUpdateDownloadTaskCache(dmDownloadId, currentState, fileName)
            }

            else -> {
                appPreferencesStore.delete(fileName)
                currentState = STATUS_CODES.FILE_DOWNLOAD_FAILURE
                downloadManager.remove(dmDownloadId)
            }
        }

        return FileDownloadStateTransition(
            downloadManagerDownloadId = dmDownloadId,
            currentState = currentState,
            previousState = previousState,
            timeTaken = stateEndTime - stateStartTime,
            currentStateReasonCode = reasonCode,
        )
    }

    @VisibleForTesting(otherwise = VisibleForTesting.PRIVATE)
    fun createDownloadManagerRequest(
        fileName: String,
        url: String,
        requestHeaders: String,
    ): DownloadManager.Request? {
        val notificationVisibility =
            if (showDownloadProgress) {
                DownloadManager.Request.VISIBILITY_VISIBLE
            } else {
                DownloadManager.Request.VISIBILITY_HIDDEN
            }

        val request =
            DownloadManager.Request(Uri.parse(url))
                .setTitle(fileName)
                .setDescription("")
                .setNotificationVisibility(notificationVisibility)
                .setAllowedOverMetered(true)
                .setAllowedOverRoaming(true)

        val parsedHeaders = jsonStringToHeaders(requestHeaders)

        parsedHeaders.forEach { request.addRequestHeader(it.first, it.second) }

        val file = File(application.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS), fileName)

        // below line handles case where download status has return failed but a stale file still
        // exists in the download directory
        if (file.exists()) {
            file.delete()
        }

        request.setDestinationUri(Uri.fromFile(file))

        return request
    }

    @SuppressLint("Range")
    @VisibleForTesting(otherwise = VisibleForTesting.PRIVATE)
    fun queryDownloadStatusFromDownloadManager(downloadID: Long): DownloadManagerCursor? {
        val cursor = downloadManager.query(DownloadManager.Query().setFilterById(downloadID))
        return if (cursor.moveToFirst()) {
            DownloadManagerCursor(
                status =
                    cursor.getLong(cursor.getColumnIndex(DownloadManager.COLUMN_STATUS)).toInt(),
                reasonCode =
                    cursor.getLong(cursor.getColumnIndex(DownloadManager.COLUMN_REASON)).toInt(),
                lastModifiedAt =
                    cursor.getLong(
                        cursor.getColumnIndex(DownloadManager.COLUMN_LAST_MODIFIED_TIMESTAMP)
                    ),
            )
        } else {
            null
        }
    }

    @VisibleForTesting(otherwise = VisibleForTesting.PRIVATE)
    fun moveFileFromDownloadsDirToTarget(fileName: String, targetDirectory: String): Boolean {
        val gzippedFile =
            File(application.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS), fileName)

        val destinationFile = File(targetDirectory, fileName)

        return gzippedFile.exists() && fileUtils.moveFile(gzippedFile, destinationFile)
    }

    @VisibleForTesting(otherwise = VisibleForTesting.PRIVATE)
    fun createOrUpdateDownloadTaskCache(
        downloadManagerDownloadId: Long,
        currentState: Int,
        fileName: String,
    ) {
        appPreferencesStore.put(
            fileName,
            DownloadTask(
                    downloadManagerId = downloadManagerDownloadId,
                    latestState = currentState,
                    stateStartTime = System.currentTimeMillis(),
                )
                .toString(),
        )
    }

    @VisibleForTesting(otherwise = VisibleForTesting.PRIVATE)
    fun retrieveCachedDownloadTask(fileName: String): DownloadTask? {
        return try {
            val value = appPreferencesStore.get(fileName)
            if (value == null) null else DownloadTask.fromJsonString(value)
        } catch (e: Exception) {
            localLogger.e(e)
            appPreferencesStore.delete(fileName)
            null
        }
    }
}

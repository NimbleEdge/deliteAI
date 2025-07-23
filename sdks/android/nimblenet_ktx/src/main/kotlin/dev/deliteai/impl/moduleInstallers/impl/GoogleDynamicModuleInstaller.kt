/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.moduleInstallers.impl

import android.os.SystemClock
import com.google.android.play.core.splitinstall.SplitInstallManager
import com.google.android.play.core.splitinstall.SplitInstallRequest
import com.google.android.play.core.splitinstall.SplitInstallStateUpdatedListener
import com.google.android.play.core.splitinstall.model.SplitInstallSessionStatus
import dev.deliteai.impl.common.METRIC_TYPE
import dev.deliteai.impl.common.SDK_CONSTANTS
import dev.deliteai.impl.common.SHARED_PREFERENCES
import dev.deliteai.impl.common.utils.buildLibDownloadLogBody
import dev.deliteai.impl.io.AppPreferencesStore
import dev.deliteai.impl.loggers.LocalLogger
import dev.deliteai.impl.loggers.RemoteLogger
import dev.deliteai.impl.moduleInstallers.ModuleInstaller

internal class GoogleDynamicModuleInstaller(
    private val splitInstallManager: SplitInstallManager,
    private val prefs: AppPreferencesStore,
    private val localLogger: LocalLogger,
    private val remoteLogger: RemoteLogger,
) : ModuleInstaller {
    private val moduleName = SDK_CONSTANTS.NE_DYNAMIC_MODULE_NAME
    private val libName = SDK_CONSTANTS.NIMBLE_NET_LIB
    private val prefKey = SHARED_PREFERENCES.GDL_DOWNLOAD_START_TIME

    override suspend fun execute() {
        localLogger.d("GOOGLE DYNAMIC LOADING TRIGGERED!!!")
        if (isModuleInstalled()) return loadLibrary()
        installModule()
    }

    private fun isModuleInstalled() = splitInstallManager.installedModules.contains(moduleName)

    private fun loadLibrary() {
        System.loadLibrary(libName)
        localLogger.d("$libName loaded successfully")
    }

    private fun installModule() {
        logDownloadStatus(isNewDownload = true)
        localLogger.d("Foreground dynamic module install triggered")
        registerListener()
        recordStartTimeIfNeeded()
        splitInstallManager.startInstall(buildRequest())
    }

    private fun buildRequest() = SplitInstallRequest.newBuilder().addModule(moduleName).build()

    private fun registerListener() {
        val listener = SplitInstallStateUpdatedListener { state ->
            when (state.status()) {
                SplitInstallSessionStatus.DOWNLOADING -> onDownloading()
                SplitInstallSessionStatus.INSTALLED -> onInstalled()
                SplitInstallSessionStatus.FAILED -> onFailed()
                else -> localLogger.d("Unknown GDL download state")
            }
        }
        splitInstallManager.registerListener(listener)
    }

    private fun recordStartTimeIfNeeded() {
        if (prefs.get(prefKey) == null) {
            prefs.put(prefKey, SystemClock.uptimeMillis().toString())
            logDownloadStatus(isNewDownload = true)
            remoteLogger.flush()
        }
    }

    private fun onDownloading() = localLogger.d("Downloading GDL")

    private fun onInstalled() {
        localLogger.d("Installed GDL")
        val duration = computeDuration()
        logDownloadStatus(isNewDownload = false, status = true, timeElapsed = duration)
        cleanup()
    }

    private fun onFailed() {
        logDownloadStatus(isNewDownload = false, status = false)
        cleanup()
    }

    private fun computeDuration(): Long =
        prefs.get(prefKey)?.toLongOrNull()?.let { SystemClock.uptimeMillis() - it } ?: -1L

    private fun cleanup() {
        remoteLogger.flush()
        prefs.delete(prefKey)
    }

    private fun logDownloadStatus(
        isNewDownload: Boolean,
        status: Boolean? = null,
        timeElapsed: Long? = null,
    ) {
        val logBody = buildLibDownloadLogBody("GDL", status, timeElapsed)
        val metricType =
            if (isNewDownload) METRIC_TYPE.DL_LIBS_DOWNLOAD_STARTED
            else METRIC_TYPE.DL_LIBS_DOWNLOAD

        remoteLogger.bufferMetric(metricType, logBody)
    }
}

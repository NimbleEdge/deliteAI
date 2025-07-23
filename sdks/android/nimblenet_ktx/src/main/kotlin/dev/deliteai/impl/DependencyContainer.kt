/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl

import android.app.Application
import android.app.DownloadManager
import android.content.Context
import com.google.android.play.core.splitinstall.SplitInstallManager
import com.google.android.play.core.splitinstall.SplitInstallManagerFactory
import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.impl.common.HardwareInfo
import dev.deliteai.impl.common.NIMBLENET_VARIANTS
import dev.deliteai.impl.common.SHARED_PREFERENCES
import dev.deliteai.impl.controllers.InternalTaskController
import dev.deliteai.impl.controllers.NimbleNetController
import dev.deliteai.impl.coroutine.DeliteAiScope
import dev.deliteai.impl.io.AppPreferencesStore
import dev.deliteai.impl.io.ChunkDownloadManager
import dev.deliteai.impl.io.FileUtils
import dev.deliteai.impl.io.Networking
import dev.deliteai.impl.loggers.LocalLogger
import dev.deliteai.impl.loggers.RemoteLogger
import dev.deliteai.impl.loggers.workManager.LogsUploadScheduler
import dev.deliteai.impl.moduleInstallers.ModuleInstaller
import dev.deliteai.impl.moduleInstallers.impl.GoogleDynamicModuleInstaller
import dev.deliteai.impl.moduleInstallers.impl.StaticModuleInstaller
import dev.deliteai.impl.nativeBridge.CoreRuntime
import dev.deliteai.impl.nativeBridge.impl.CoreRuntimeImpl
import okhttp3.OkHttpClient

internal class DependencyContainer
private constructor(
    private val application: Application,
    private val nimbleNetConfig: NimbleNetConfig,
) {
    // third-party
    private val okHttpClient = OkHttpClient.Builder().build()
    private val sharedPreferences =
        application.getSharedPreferences(SHARED_PREFERENCES.NAME, Context.MODE_PRIVATE)
    private val downloadManager =
        application.getSystemService(Context.DOWNLOAD_SERVICE) as DownloadManager

    // singletons
    private val deliteAiScopeSingleton: DeliteAiScope by lazy { DeliteAiScope() }

    private val localLoggerSingleton: LocalLogger by lazy { LocalLogger() }

    private val remoteLoggerSingleton: RemoteLogger by lazy {
        RemoteLogger(
            networkingSingleton,
            hardwareInfoSingleton,
            nimbleNetConfig,
            localLoggerSingleton,
            coreRuntimeInterfaceSingleton,
        )
    }

    private val fileUtilitySingleton: FileUtils by lazy {
        FileUtils(application, localLoggerSingleton)
    }

    private val hardwareInfoSingleton: HardwareInfo by lazy {
        HardwareInfo(application, localLoggerSingleton)
    }

    private val coreRuntimeInterfaceSingleton: CoreRuntime by lazy { CoreRuntimeImpl() }

    private val internalTaskControllerSingleton: InternalTaskController by lazy {
        InternalTaskController(application, coreRuntimeInterfaceSingleton, fileUtilitySingleton)
    }

    private val splitInstallManagerSingleton: SplitInstallManager by lazy {
        SplitInstallManagerFactory.create(application)
    }

    private val appPreferencesStoreSingleton: AppPreferencesStore by lazy {
        AppPreferencesStore(sharedPreferences)
    }

    private val moduleInstallerSingleton: ModuleInstaller by lazy { getModuleInstallerInstance() }

    private val logsUploadSchedulerSingleton: LogsUploadScheduler by lazy {
        LogsUploadScheduler(application)
    }

    private val googleDynamicModuleInstallerSingleton: GoogleDynamicModuleInstaller by lazy {
        GoogleDynamicModuleInstaller(
            splitInstallManager = splitInstallManagerSingleton,
            prefs = appPreferencesStoreSingleton,
            localLogger = localLoggerSingleton,
            remoteLogger = remoteLoggerSingleton,
        )
    }

    private val staticModuleInstallerSingleton: StaticModuleInstaller by lazy {
        StaticModuleInstaller(localLogger = localLoggerSingleton)
    }

    private val nimbleNetControllerSingleton: NimbleNetController by lazy {
        NimbleNetController(
            application,
            deliteAiScopeSingleton,
            fileUtilitySingleton,
            hardwareInfoSingleton,
            moduleInstallerSingleton,
            coreRuntimeInterfaceSingleton,
            remoteLoggerSingleton,
        )
    }

    private val chunkDownloadManagerSingleton: ChunkDownloadManager by lazy {
        ChunkDownloadManager(
            application,
            downloadManager,
            appPreferencesStoreSingleton,
            fileUtilitySingleton,
            localLoggerSingleton,
            nimbleNetConfig.showDownloadProgress,
        )
    }

    private val networkingSingleton: Networking by lazy {
        Networking(okHttpClient, localLoggerSingleton, chunkDownloadManagerSingleton)
    }

    // nimblenet_ktx
    fun getNimbleNetController(): NimbleNetController = nimbleNetControllerSingleton

    fun getLocalLogger(): LocalLogger = localLoggerSingleton

    // remove once wm scheduler is a class
    fun getAppPreferencesStore(): AppPreferencesStore = appPreferencesStoreSingleton

    // work manager
    fun getModuleInstaller(): ModuleInstaller = moduleInstallerSingleton

    fun getInternalTaskController(): InternalTaskController = internalTaskControllerSingleton

    // JNI
    fun getNetworking(): Networking = networkingSingleton

    fun getHardwareInfo(): HardwareInfo = hardwareInfoSingleton

    fun getLogsUploadScheduler(): LogsUploadScheduler = logsUploadSchedulerSingleton

    // integration test
    fun getFileUtils(): FileUtils = fileUtilitySingleton

    fun getRemoteLogger(): RemoteLogger = remoteLoggerSingleton

    // gemini
    fun getCoreRuntime(): CoreRuntime = coreRuntimeInterfaceSingleton

    companion object {
        @Volatile private var instance: DependencyContainer? = null

        @JvmStatic
        fun getInstance(
            application: Application?,
            nimbleNetConfig: NimbleNetConfig?,
        ): DependencyContainer {
            return instance
                ?: synchronized(this) {
                    instance
                        ?: DependencyContainer(application!!, nimbleNetConfig!!).also {
                            instance = it
                        }
                }
        }
    }

    private fun getModuleInstallerInstance(): ModuleInstaller {
        return when (nimbleNetConfig.libraryVariant) {
            NIMBLENET_VARIANTS.STATIC -> staticModuleInstallerSingleton
            NIMBLENET_VARIANTS.GOOGLE_PLAY_FEATURE_DYNAMIC -> googleDynamicModuleInstallerSingleton
        }
    }
}

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.scriptWrappers

import ai.nimbleedge.impl.DependencyContainer
import ai.nimbleedge.impl.common.STATUS_CODES
import ai.nimbleedge.impl.loggers.LocalLogger
import ai.nimbleedge.impl.nativeBridge.CoreRuntime
import android.content.Context
import com.google.ai.edge.aicore.GenerativeAIException
import com.google.ai.edge.aicore.GenerativeModel
import com.google.ai.edge.aicore.generationConfig
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch

object GeminiNanoHandler {
    private val supervisorJob = SupervisorJob()
    private val scope = CoroutineScope(supervisorJob + Dispatchers.Default)
    private lateinit var localLogger: LocalLogger
    private lateinit var coreRuntime: CoreRuntime

    @Volatile
    private var status = STATUS_CODES.FILE_DOWNLOAD_PENDING

    @Volatile
    private var model: GenerativeModel? = null

    @Volatile
    private var activeJob: Job? = null

    @JvmStatic
    fun getStatus(): Int = status

    @JvmStatic
    fun initialize(application: Context) {
        if (!(::localLogger.isInitialized && ::coreRuntime.isInitialized)) {
            val dependencyContainer = DependencyContainer.getInstance(null, null)
            coreRuntime = dependencyContainer.getCoreRuntime()
            localLogger = dependencyContainer.getLocalLogger()
        }

        if (status != STATUS_CODES.FILE_DOWNLOAD_PENDING) {
            return
        }
        status = STATUS_CODES.FILE_DOWNLOAD_RUNNING
        val generationConfig = generationConfig {
            context = application
            // TODO: Fetch from cloud config if required
            temperature = 0.2f
            topK = 16
            maxOutputTokens = 1000
        }

        model = GenerativeModel(generationConfig = generationConfig)
        scope.launch {
            try {
                model!!.prepareInferenceEngine()
                localLogger.i("Gemini Nano is ready for inference")
                status = STATUS_CODES.FILE_DOWNLOAD_SUCCESS
            } catch (e: GenerativeAIException) {
                localLogger.e(e)
                status = STATUS_CODES.FILE_DOWNLOAD_FAILURE
            }
        }
    }

    @JvmStatic
    fun prompt(prompt: String) {
        if (status != STATUS_CODES.FILE_DOWNLOAD_SUCCESS) {
            localLogger.e("Gemini model is not ready")
            coreRuntime.closeGeminiResponseQueue()
            return
        }
        activeJob = scope.launch {
            try {
                model!!.generateContentStream(prompt)
                    .collect { response ->
                        val text = response.text!!
                        coreRuntime.pushGeminiResponseToQueue(text)
                    }
            } catch (e: Exception) {
                localLogger.e(e)
            }
            coreRuntime.closeGeminiResponseQueue()
        }
    }

    @JvmStatic
    fun cancel() {
        activeJob?.cancel()
        activeJob = null
    }
}

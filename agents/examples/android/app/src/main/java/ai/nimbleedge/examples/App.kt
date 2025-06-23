/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.examples

import ai.nimbleedge.NimbleNet
import ai.nimbleedge.datamodels.NimbleNetConfig
import ai.nimbleedge.notifications_summarizer.api.NotificationsSummarizerAgent
import ai.nimbleedge.notifications_summarizer.api.dataModels.NotificationSummarizerConfig
import ai.nimbleedge.utils.NIMBLENET_VARIANTS
import android.app.Application
import android.util.Log
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking

class App : Application() {

    override fun onCreate() {
        super.onCreate()

        val res = initializeAgent(applicationContext as Application)
        Log.i("TOP-LEVEL", "onCreate: $res")
    }

    private fun initializeAgent(application: Application): String = runBlocking {
        return@runBlocking runCatching {
            val config = NotificationSummarizerConfig(
                onScheduledSummaryReady = { notificationSummary ->
                    Log.i(HOST_TAG, "initializeAgent: $notificationSummary")
                }
            )

            copyEspeakDataIfNeeded(application, "espeak-ng-data")

            val nimbleConfig = NimbleNetConfig(
            clientId = "d-ai-sample",
            host = "https://api.delite.ai",
            deviceId = "test-device",
            clientSecret = "secret_value",
            debug = true,
            compatibilityTag = "agent_notification_summarizer",
            libraryVariant = NIMBLENET_VARIANTS.STATIC
        )

            val res = NimbleNet.initialize(application, nimbleConfig)
            check(res.status)

            while (!NimbleNet.isReady().status) delay(1000)

            NotificationsSummarizerAgent.initialize(application, config)
            "initialize succeeded"
        }.getOrElse {
            "initialize failed: ${it.message}"
        }
    }
}

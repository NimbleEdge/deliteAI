/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.examples

import android.app.Application
import android.util.Log
import dev.deliteai.NimbleNet
import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.impl.common.NIMBLENET_VARIANTS
import dev.deliteai.notifications_summarizer.NotificationsSummarizerAgent
import dev.deliteai.notifications_summarizer.dataModels.NotificationSummarizerConfig
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking
import org.json.JSONArray

class App : Application() {

    override fun onCreate() {
        super.onCreate()

        //Init for notification summarizer must happen from the Application class
        val res = initializeAgent(applicationContext as Application)
        Log.i("TOP-LEVEL", "onCreate: $res")
    }

    private fun initializeAgent(application: Application): String = runBlocking {
        return@runBlocking runCatching {
            val config = NotificationSummarizerConfig(
                onScheduledSummaryReady = { notificationSummary ->
                    Log.i("AGENTS-EXAMPLE", "initializeAgent: $notificationSummary")
                }
            )

            val assets: List<Map<String, Any>> = listOf(
                mapOf(
                    "name" to "llama-3",
                    "version" to "1.0.0",
                    "type" to "llm",
                    "location" to mapOf(
                        "path" to "llama-3"
                    )
                ),
                mapOf(
                    "name" to "script",
                    "version" to "1.0.0",
                    "type" to "script",
                    "location" to mapOf(
                        "path" to "main.ast"
                    )
                )
            )

            val nnConfig = NimbleNetConfig(
                debug = true,
                online = false
            )

            val res = NimbleNet.initialize(application, nnConfig, JSONArray(assets))
            check(res.status)

            while (!NimbleNet.isReady().status) delay(1000)

            NotificationsSummarizerAgent.initialize(application, config)
            "initialize succeeded"
        }.getOrElse {
            "initialize failed: ${it.message}"
        }
    }
}

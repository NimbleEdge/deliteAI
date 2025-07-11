/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.examples

import android.Manifest
import android.app.AlarmManager
import android.app.Application
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.provider.Settings
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.content.ContextCompat
import dev.deliteai.examples.ui.theme.ExamplesTheme
import dev.deliteai.notifications_summarizer.NotificationsSummarizerAgent
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File
import java.io.FileOutputStream
import java.time.LocalDate


class NotificationSummarizerActivity : ComponentActivity() {
    private val notificationPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { isGranted ->
        // Handle the permission result if needed
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            ExamplesTheme {
                MainScreen()
            }
        }
    }

    private fun requestNotificationPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            if (ContextCompat.checkSelfPermission(
                    this,
                    Manifest.permission.POST_NOTIFICATIONS
                ) != PackageManager.PERMISSION_GRANTED
            ) {
                notificationPermissionLauncher.launch(Manifest.permission.POST_NOTIFICATIONS)
            }
        }
    }

    private fun requestExactAlarmPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            val alarmManager = getSystemService(Context.ALARM_SERVICE) as AlarmManager
            if (!alarmManager.canScheduleExactAlarms()) {
                val intent = Intent(Settings.ACTION_REQUEST_SCHEDULE_EXACT_ALARM).apply {
                    data = Uri.parse("package:$packageName")
                }
                startActivity(intent)
            }
        }
    }

    private fun openNotificationListenerSettings() {
        val intent = Intent(Settings.ACTION_NOTIFICATION_LISTENER_SETTINGS)
        startActivity(intent)
    }

    @Composable
    fun MainScreen() {
        val context = LocalContext.current
        val activity = context as ComponentActivity
        val application = context.applicationContext as Application

        var resultText by remember { mutableStateOf("") }
        val coroutineScope = rememberCoroutineScope()

        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(vertical = 40.dp, horizontal = 16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .height(360.dp),
                elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
            ) {
                Text(
                    text = if (resultText.isEmpty()) "Output will appear here..." else resultText,
                    modifier = Modifier
                        .fillMaxSize()
                        .padding(12.dp)
                        .verticalScroll(rememberScrollState()),
                    fontSize = 14.sp,
                    textAlign = TextAlign.Start
                )
            }

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                Button(
                    onClick = {
                        requestNotificationPermission()
                        resultText = "Requested notification permission"
                    },
                    modifier = Modifier
                        .weight(1f)
                        .height(36.dp)
                ) {
                    Text(text = "Notification", fontSize = 10.sp)
                }

                Button(
                    onClick = {
                        requestExactAlarmPermission()
                        resultText = "Requested exact alarm permission"
                    },
                    modifier = Modifier
                        .weight(1f)
                        .height(36.dp)
                ) {
                    Text(text = "Alarm", fontSize = 10.sp)
                }

                Button(
                    onClick = {
                        openNotificationListenerSettings()
                        resultText = "Opened notification listener settings"
                    },
                    modifier = Modifier
                        .weight(1f)
                        .height(36.dp)
                ) {
                    Text(text = "Listener", fontSize = 10.sp)
                }
            }

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                Button(
                    onClick = {
                        coroutineScope.launch {
                            resultText = scheduleNotificationJob()
                        }
                    },
                    modifier = Modifier
                        .weight(1f)
                        .height(40.dp)
                ) {
                    Text(text = "Schedule", fontSize = 11.sp)
                }
            }

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                Button(
                    onClick = {
                        coroutineScope.launch {
                            resultText = getCurrentNotificationSummary()
                        }
                    },
                    modifier = Modifier
                        .weight(1f)
                        .height(40.dp)
                ) {
                    Text(text = "Summarize Current", fontSize = 11.sp)
                }

                Button(
                    onClick = {
                        coroutineScope.launch {
                            resultText = getSummaryById("test-id")
                        }
                    },
                    modifier = Modifier
                        .weight(1f)
                        .height(40.dp)
                ) {
                    Text(text = "By ID", fontSize = 11.sp)
                }
            }

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                Button(
                    onClick = {
                        coroutineScope.launch {
                            resultText = getSummariesForToday()
                        }
                    },
                    modifier = Modifier
                        .weight(1f)
                        .height(40.dp)
                ) {
                    Text(text = "Today", fontSize = 11.sp)
                }

                Button(
                    onClick = {
                        coroutineScope.launch {
                            resultText = getSummariesForLast7Days()
                        }
                    },
                    modifier = Modifier
                        .weight(1f)
                        .height(40.dp)
                ) {
                    Text(text = "Last 7 Days", fontSize = 11.sp)
                }
            }
        }
    }

    private suspend fun getCurrentNotificationSummary(): String {
        return runCatching {
            val summary = NotificationsSummarizerAgent.getSummaryOfCurrentNotification()
            summary.toString()
        }.getOrElse {
            "getSummaryOfCurrentNotification failed: ${it.message}"
        }
    }

    private suspend fun getSummaryById(id: String): String {
        return runCatching {
            val summary = NotificationsSummarizerAgent.getSummary(id)
            summary.toString()
        }.getOrElse {
            "getSummary(id) failed: ${it.message}"
        }
    }

    private suspend fun getSummariesForToday(): String {
        return runCatching {
            val today = LocalDate.now()
            val list = NotificationsSummarizerAgent.getSummary(today).payload!!
            list.joinToString("\n")
        }.getOrElse {
            "getSummary(date) failed: ${it.message}"
        }
    }

    private suspend fun getSummariesForLast7Days(): String {
        return runCatching {
            val endDate = LocalDate.now()
            val startDate = endDate.minusDays(7)
            val list = NotificationsSummarizerAgent.getSummary(startDate, endDate).payload!!
            list.joinToString("\n")
        }.getOrElse {
            "getSummaries(range) failed: ${it.message}"
        }
    }

    suspend fun copyEspeakDataIfNeeded(context: Context, assetPath: String) {
        val prefs = context.getSharedPreferences(assetPath, Context.MODE_PRIVATE)
        val alreadyCopied = prefs.getBoolean(assetPath, false)

        if (alreadyCopied) return

        withContext(Dispatchers.IO) {
            try {
                val assetFolder = "espeak-ng-data"
                val outputFolder = File(context.filesDir, "nimbleSDK")

                copyAssetFolder(context, assetFolder, outputFolder)

                prefs.edit().putBoolean(assetPath, true).apply()
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }

    private fun copyAssetFolder(context: Context, assetPath: String, outDir: File) {
        val assetManager = context.assets
        val assets = assetManager.list(assetPath) ?: return

        if (!outDir.exists()) {
            outDir.mkdirs()
        }

        for (asset in assets) {
            val subPath = "$assetPath/$asset"
            val outFile = File(outDir, asset)

            val subAssets = assetManager.list(subPath)
            if (subAssets.isNullOrEmpty()) {
                if (!outFile.exists()) {
                    assetManager.open(subPath).use { inputStream ->
                        FileOutputStream(outFile).use { outputStream ->
                            inputStream.copyTo(outputStream)
                        }
                    }
                }
            } else {
                copyAssetFolder(context, subPath, outFile)
            }
        }
    }

    //will schedule a job that will go off in the next 10 seconds for demo
    private suspend fun scheduleNotificationJob(): String {
        return runCatching {
            val timeInMillis = System.currentTimeMillis() + 10_000L
            NotificationsSummarizerAgent.scheduleNotificationSummaryJob(timeInMillis)
            "scheduleNotificationSummaryJob returned"
        }.getOrElse {
            "scheduleNotificationSummaryJob failed: ${it.message}"
        }
    }
}

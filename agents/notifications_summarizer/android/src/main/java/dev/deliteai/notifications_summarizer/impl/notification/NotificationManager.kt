/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.notifications_summarizer.impl.notification

import dev.deliteai.notifications_summarizer.R
import dev.deliteai.notifications_summarizer.impl.common.Constants.CHANNEL_FOREGROUND_DESC
import dev.deliteai.notifications_summarizer.impl.common.Constants.CHANNEL_FOREGROUND_ID
import dev.deliteai.notifications_summarizer.impl.common.Constants.CHANNEL_FOREGROUND_NAME
import android.app.Application
import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import android.os.Build
import androidx.core.app.NotificationCompat

class NotificationManager(
    private val application: Application,
) {
    init {
        createForegroundNotificationsChannel()
    }

    private fun createForegroundNotificationsChannel() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) return

        val foregroundChannel = NotificationChannel(
            CHANNEL_FOREGROUND_ID,
            CHANNEL_FOREGROUND_NAME,
            NotificationManager.IMPORTANCE_LOW
        ).apply {
            description = CHANNEL_FOREGROUND_DESC
            setShowBadge(false)
        }

        val manager =
            application.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        manager.createNotificationChannel(foregroundChannel)
    }

    fun buildForegroundNotification(
        title: String,
        message: String,
    ): Notification {
        return NotificationCompat.Builder(application, CHANNEL_FOREGROUND_ID)
            .setSmallIcon(R.drawable.ne_ic)
            .setContentTitle(title)
            .setContentText(message)
            .setPriority(NotificationCompat.PRIORITY_LOW)
            .setOngoing(true)
            .build()
    }

}

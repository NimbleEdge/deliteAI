/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.notifications_summarizer.impl.broadcast

import dev.deliteai.notifications_summarizer.impl.common.Constants.ACTION_NOTIFICATION_SUMMARIZER_ALARM
import dev.deliteai.notifications_summarizer.impl.common.Constants.EXTRA_AUTO_PLAY
import dev.deliteai.notifications_summarizer.impl.common.Constants.EXTRA_TAP_INTENT
import dev.deliteai.notifications_summarizer.impl.common.Constants.TAG
import dev.deliteai.notifications_summarizer.impl.services.SummarizationForegroundService
import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.util.Log

class AlarmBroadcastReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent?) {
        val action = intent?.action ?: return
        if (action != ACTION_NOTIFICATION_SUMMARIZER_ALARM && action != Intent.ACTION_BOOT_COMPLETED) {
            Log.e(TAG, "Unexpected action: $action")
            return
        }

        val tapPendingIntent: PendingIntent? =
            intent.getParcelableExtra(EXTRA_TAP_INTENT)

        val shouldAutoPlay = intent.getBooleanExtra(EXTRA_AUTO_PLAY, false)

        Intent(context, SummarizationForegroundService::class.java).apply {
            this.action = action

            tapPendingIntent?.let { putExtra(EXTRA_TAP_INTENT, it) }
            putExtra(EXTRA_AUTO_PLAY, shouldAutoPlay)

            addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES or Intent.FLAG_RECEIVER_FOREGROUND)
            context.startForegroundService(this)
        }

        Log.i(TAG, "Requested SummarizationForegroundService (action=$action)")
    }
}

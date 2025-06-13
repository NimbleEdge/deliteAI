package ai.nimbleedge.notifications_summarizer.impl.broadcast

import ai.nimbleedge.notifications_summarizer.impl.common.Constants.ACTION_NOTIFICATION_SUMMARIZER_ALARM
import ai.nimbleedge.notifications_summarizer.impl.common.Constants.EXTRA_AUTO_PLAY
import ai.nimbleedge.notifications_summarizer.impl.common.Constants.EXTRA_TAP_INTENT
import ai.nimbleedge.notifications_summarizer.impl.common.Constants.TAG
import ai.nimbleedge.notifications_summarizer.impl.services.SummarizationForegroundService
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

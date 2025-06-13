package ai.nimbleedge.notifications_summarizer.impl.services

import ai.nimbleedge.notifications_summarizer.api.dataModels.NotificationSummarizerConfig
import ai.nimbleedge.notifications_summarizer.impl.DependencyContainer
import ai.nimbleedge.notifications_summarizer.impl.LlmManager
import ai.nimbleedge.notifications_summarizer.impl.common.Constants.TAG
import ai.nimbleedge.notifications_summarizer.impl.data.local.NotificationSummaryDao
import ai.nimbleedge.notifications_summarizer.impl.data.local.toNotificationSummaryEntity
import ai.nimbleedge.notifications_summarizer.impl.notification.NotificationListener
import ai.nimbleedge.notifications_summarizer.impl.notification.NotificationManager
import android.app.Service
import android.content.Intent
import android.os.IBinder
import android.util.Log
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.delay
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch

class SummarizationForegroundService : Service() {
    private val serviceScope = CoroutineScope(SupervisorJob() + Dispatchers.IO)
    private var summarizeJob: Job? = null
    private lateinit var llmManager: LlmManager
    private lateinit var notificationManager: NotificationManager
    private lateinit var notificationSummaryDao: NotificationSummaryDao
    private lateinit var config: NotificationSummarizerConfig

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        val dependencyContainer = DependencyContainer.getInstance()

        notificationManager = dependencyContainer.getNotificationManager()
        notificationSummaryDao = dependencyContainer.getNotificationSummaryDao()
        config = dependencyContainer.getConfig()

        //we need only one notification for multiple services
        val foregroundNotificationId = 746353
        //TODO: discuss the copy of notification

        startForeground(
            foregroundNotificationId, notificationManager.buildForegroundNotification(
                title = "Notification Summarization",
                message = "Summarization in progress",
            )
        )


        summarizeJob?.cancel()
        summarizeJob = serviceScope.launch {
            summarizeNotifications()
            if (!serviceScope.isActive) stopSelf()
        }
        return START_REDELIVER_INTENT
    }

    override fun onBind(intent: Intent?): IBinder? = null

    private suspend fun summarizeNotifications() {
        waitForNotificationListener()

        val notifications = NotificationListener.getNotifications()
        if (notifications.isEmpty()) return

        val summary = llmManager.getSummary(notifications)

        notificationSummaryDao.insert(
            summary.toNotificationSummaryEntity()
        )

        config.onScheduledSummaryReady(summary)
    }

    private suspend fun waitForNotificationListener() {
        val notificationListenerTimeoutMs = 120000
        val pollFrequencyMs = 1000L
        var timeMs = 0L

        while (!NotificationListener.isListenerConnected()) {
            if (timeMs > notificationListenerTimeoutMs) {
                Log.e(TAG, "summarizeNotifications: Notification Listener Timeout")
                return
            }

            delay(pollFrequencyMs)
            timeMs += pollFrequencyMs
        }
    }

    override fun onDestroy() {
        summarizeJob?.cancel()
        serviceScope.cancel()
        super.onDestroy()
    }
}

package ai.nimbleedge.notifications_summarizer.impl.common

object Constants {
    const val TAG = "NE-AI-NOTIFICATION-SUMMARIZER"

    const val ACTION_NOTIFICATION_SUMMARIZER_ALARM =
        "ai.nimbleedge.notifications_summarizer.ALARM_TRIGGER"

    const val CHANNEL_DEFAULT_ID = "ai.nimbleedge.notifications.DEFAULT"
    const val CHANNEL_DEFAULT_NAME = "Default Notifications"
    const val CHANNEL_DEFAULT_DESC = "App-wide default notification channel"

    const val CHANNEL_FOREGROUND_ID = "ai.nimbleedge.notifications.FOREGROUND_SERVICE"
    const val CHANNEL_FOREGROUND_NAME = "Foreground Service"
    const val CHANNEL_FOREGROUND_DESC = "Notifications for the foreground service"

    const val EXTRA_TAP_INTENT = "EXTRA_TAP_INTENT"
    const val EXTRA_AUTO_PLAY = "EXTRA_AUTO_PLAY"
}

package ai.nimbleedge.notifications_summarizer.api.dataModels

data class NotificationSummarizerConfig(
    val onScheduledSummaryReady: (notificationSummary: NotificationSummary?) -> Unit,
)

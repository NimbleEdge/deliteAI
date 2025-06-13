package ai.nimbleedge.notifications_summarizer.impl

import ai.nimbleedge.notifications_summarizer.api.dataModels.NotificationSummarizerConfig
import ai.nimbleedge.notifications_summarizer.impl.broadcast.AlarmBroadcastScheduler
import ai.nimbleedge.notifications_summarizer.impl.data.local.DB
import ai.nimbleedge.notifications_summarizer.impl.data.local.NotificationSummaryDao
import ai.nimbleedge.notifications_summarizer.impl.notification.NotificationManager
import android.app.AlarmManager
import android.app.Application
import androidx.room.Room

//should get init only from the api/
internal class DependencyContainer private constructor(
    private val application: Application,
    private val config: NotificationSummarizerConfig
) {
    private val alarmManager = application.getSystemService(AlarmManager::class.java)

    private val alarmBroadcastSchedulerSingleton =
        AlarmBroadcastScheduler(application, alarmManager, config)
    private val db = Room.databaseBuilder(
        application,
        DB::class.java,
        "notifications_summarizer.db"
    ).build()
    private val notificationSummaryDaoSingleton = db.notificationSummaryDao()
    private val notificationManagerSingleton = NotificationManager(application)
    private val llmManagerSingleton = LlmManager()

    fun getAlarmBroadcastScheduler(): AlarmBroadcastScheduler = alarmBroadcastSchedulerSingleton
    fun getNotificationSummaryDao(): NotificationSummaryDao = notificationSummaryDaoSingleton
    fun getNotificationManager(): NotificationManager = notificationManagerSingleton
    fun getConfig(): NotificationSummarizerConfig = config
    fun getLlmManager(): LlmManager = llmManagerSingleton
    fun getController(): Controller = Controller(this)

    companion object {
        private var instance: DependencyContainer? = null

        @Synchronized
        fun getInstance(
            application: Application,
            config: NotificationSummarizerConfig
        ): DependencyContainer {
            if (instance == null) {
                instance = DependencyContainer(application, config)
            }

            return instance!!
        }

        fun getInstance(): DependencyContainer {
            if (instance == null) throw Exception("Agent not initialized")

            return instance!!
        }
    }
}

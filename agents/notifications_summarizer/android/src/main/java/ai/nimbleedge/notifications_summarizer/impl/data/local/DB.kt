package ai.nimbleedge.notifications_summarizer.impl.data.local

import androidx.room.Database
import androidx.room.RoomDatabase
import androidx.room.TypeConverters

@Database(entities = [NotificationSummaryEntity::class], version = 1)
@TypeConverters(Converters::class)
abstract class DB : RoomDatabase() {
    abstract fun notificationSummaryDao(): NotificationSummaryDao
}

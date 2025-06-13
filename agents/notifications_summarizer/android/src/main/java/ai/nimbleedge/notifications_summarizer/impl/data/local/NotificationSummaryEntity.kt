package ai.nimbleedge.notifications_summarizer.impl.data.local

import androidx.room.Entity
import androidx.room.PrimaryKey
import java.time.LocalDate

@Entity(tableName = "notification_summary")
data class NotificationSummaryEntity(
    @PrimaryKey val id: String,
    val date: LocalDate,
    val body: String
)

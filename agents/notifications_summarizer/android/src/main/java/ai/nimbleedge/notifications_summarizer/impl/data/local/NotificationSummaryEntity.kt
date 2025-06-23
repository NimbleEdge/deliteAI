/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

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

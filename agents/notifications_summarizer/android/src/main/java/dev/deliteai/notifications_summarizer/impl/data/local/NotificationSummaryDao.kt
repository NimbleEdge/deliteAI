/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.notifications_summarizer.impl.data.local

import androidx.room.Dao
import androidx.room.Insert
import androidx.room.OnConflictStrategy
import androidx.room.Query
import java.time.LocalDate

@Dao
interface NotificationSummaryDao {
    @Insert(onConflict = OnConflictStrategy.ABORT)
    suspend fun insert(summary: NotificationSummaryEntity)

    @Query("SELECT * FROM notification_summary WHERE id = :id")
    suspend fun getById(id: String): NotificationSummaryEntity?

    @Query("SELECT * FROM notification_summary WHERE date = :date")
    suspend fun getByDate(date: LocalDate): List<NotificationSummaryEntity>

    @Query("SELECT * FROM notification_summary WHERE date BETWEEN :startDate AND :endDate")
    suspend fun getByDateRange(
        startDate: LocalDate,
        endDate: LocalDate
    ): List<NotificationSummaryEntity>
}

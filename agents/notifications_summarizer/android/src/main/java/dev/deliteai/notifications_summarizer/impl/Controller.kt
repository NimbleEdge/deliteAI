/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.notifications_summarizer.impl

import dev.deliteai.notifications_summarizer.dataModels.NotificationSummary
import dev.deliteai.notifications_summarizer.impl.data.local.toNotificationSummary
import dev.deliteai.notifications_summarizer.impl.notification.NotificationListener
import java.time.LocalDate

internal class Controller(private val dependencyContainer: DependencyContainer) {
    private val alarmBroadcastScheduler = dependencyContainer.getAlarmBroadcastScheduler()
    private val notificationSummaryDao = dependencyContainer.getNotificationSummaryDao()
    private val llmManager: LlmManager = dependencyContainer.getLlmManager()

    suspend fun schedule(timeInMillis: Long) {
        alarmBroadcastScheduler
            .scheduleNotificationsSummaryAlarm(timeInMillis)
    }

    suspend fun getSummaryForCurrentNotifications(): NotificationSummary {
        val notifications = NotificationListener.getNotifications()
        if (notifications.isEmpty()) throw Exception("No notifications found")

        return llmManager.getSummary(notifications)
    }

    suspend fun get(id: String): NotificationSummary? =
        notificationSummaryDao
            .getById(id)
            ?.toNotificationSummary()

    suspend fun get(date: LocalDate): List<NotificationSummary> =
        notificationSummaryDao
            .getByDate(date)
            .map { it.toNotificationSummary() }

    suspend fun get(
        startDate: LocalDate,
        endDate: LocalDate
    ): List<NotificationSummary> =
        notificationSummaryDao
            .getByDateRange(startDate, endDate)
            .map { it.toNotificationSummary() }
}

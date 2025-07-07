/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.notifications_summarizer.impl.data.local

import dev.deliteai.notifications_summarizer.dataModels.NotificationSummary


fun NotificationSummaryEntity.toNotificationSummary(): NotificationSummary = NotificationSummary(
    id = id,
    date = date,
    body = body,
)

fun NotificationSummary.toNotificationSummaryEntity(): NotificationSummaryEntity =
    NotificationSummaryEntity(
        id = id,
        date = date,
        body = body
    )

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.notifications_summarizer.impl.data.local

import ai.nimbleedge.notifications_summarizer.dataModels.NotificationSummary


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

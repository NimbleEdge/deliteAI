/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.notifications_summarizer.api.dataModels

data class NotificationSummarizerConfig(
    val onScheduledSummaryReady: (notificationSummary: NotificationSummary?) -> Unit,
)

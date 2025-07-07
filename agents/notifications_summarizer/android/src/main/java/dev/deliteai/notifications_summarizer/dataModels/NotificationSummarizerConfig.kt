/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.notifications_summarizer.dataModels

data class NotificationSummarizerConfig(
    val onScheduledSummaryReady: (notificationSummary: NotificationSummary?) -> Unit,
)

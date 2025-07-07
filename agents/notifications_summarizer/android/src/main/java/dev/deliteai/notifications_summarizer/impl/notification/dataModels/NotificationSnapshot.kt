/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.notifications_summarizer.impl.notification.dataModels

import org.json.JSONObject

data class NotificationSnapshot(
    val packageName: String,
    val channel: String,
    val priority: Int,
    val title: String,
    val body: String,
    val subText: String,
) {
    override fun toString(): String =
        JSONObject().apply {
            put("packageName", packageName)
            put("channel", channel)
            put("priority", priority)
            put("title", title)
            put("body", body)
            put("subText", subText)
        }.toString()
}

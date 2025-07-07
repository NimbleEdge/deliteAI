/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.notifications_summarizer.impl

import dev.deliteai.NimbleNet
import dev.deliteai.datamodels.NimbleNetTensor
import dev.deliteai.notifications_summarizer.dataModels.NotificationSummary
import dev.deliteai.notifications_summarizer.impl.notification.dataModels.NotificationSnapshot
import dev.deliteai.utils.DATATYPE
import org.json.JSONArray
import org.json.JSONObject
import java.time.LocalDate
import java.util.UUID

class LlmManager {
    suspend fun getSummary(
        notifications: List<NotificationSnapshot>
    ): NotificationSummary {
        val res = NimbleNet.runMethod(
            methodName = "summarize_notification",
            inputs = hashMapOf(
                "input" to NimbleNetTensor(
                    data = JSONArray(notifications.map { JSONObject(it.toString()) }).toString(),
                    datatype = DATATYPE.STRING,
                    shape = null
                )
            )
        )

        if (!res.status) throw Exception(res.error?.message)

        return NotificationSummary(
            id = UUID.randomUUID().toString(),
            date = LocalDate.now(),
            body = res.payload!!["output"]!!.data as String
        )
    }
}

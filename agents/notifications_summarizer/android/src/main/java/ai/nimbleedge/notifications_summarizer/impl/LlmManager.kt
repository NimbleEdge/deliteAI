/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.notifications_summarizer.impl

import ai.nimbleedge.NimbleNet
import ai.nimbleedge.datamodels.NimbleNetTensor
import ai.nimbleedge.notifications_summarizer.dataModels.NotificationSummary
import ai.nimbleedge.notifications_summarizer.impl.notification.dataModels.NotificationSnapshot
import ai.nimbleedge.utils.DATATYPE
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

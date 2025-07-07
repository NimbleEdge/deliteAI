/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.notifications_summarizer.dataModels

import android.app.Application
import org.json.JSONObject
import java.io.File
import java.time.LocalDate

data class NotificationSummary(
    val id: String,
    val date: LocalDate,
    val body: String,
) {
    fun getGeneratedAudioFile(application: Application): File? =
        File(application.filesDir, "audio_${id}.wav")
            .takeIf { it.exists() }

    override fun toString(): String {
        return JSONObject().apply {
            put("id", id)
            put("date", date)
            put("body", body)
        }.toString()
    }

    companion object {
        fun fromJsonString(json: String): NotificationSummary {
            val obj = JSONObject(json)
            val id = obj.getString("id")
            val date = LocalDate.parse(obj.getString("date"))

            val body = obj.getString("body")

            return NotificationSummary(
                id = id,
                date = date,
                body = body,
            )
        }
    }
}

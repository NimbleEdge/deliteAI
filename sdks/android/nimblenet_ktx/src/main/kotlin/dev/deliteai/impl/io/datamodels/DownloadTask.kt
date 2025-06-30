/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.io.datamodels

import org.json.JSONObject

internal data class DownloadTask(
    val downloadManagerId: Long,
    val latestState: Int,
    val stateStartTime: Long,
) {
    override fun toString(): String {
        val jsonObject = JSONObject()
        jsonObject.put("downloadManagerId", downloadManagerId)
        jsonObject.put("latestState", latestState)
        jsonObject.put("stateStartTime", stateStartTime)
        return jsonObject.toString()
    }

    companion object {
        fun fromJsonString(jsonString: String): DownloadTask {
            val jsonObject = JSONObject(jsonString)

            return DownloadTask(
                downloadManagerId = jsonObject.getLong("downloadManagerId"),
                latestState = jsonObject.getInt("latestState"),
                stateStartTime = jsonObject.getLong("stateStartTime"),
            )
        }
    }
}

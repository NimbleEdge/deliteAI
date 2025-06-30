/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.impl.common.utils

import ai.nimbleedge.impl.DependencyContainer
import okhttp3.Headers
import org.json.JSONArray
import org.json.JSONObject

internal fun jsonStringToHeaders(jsonString: String): Headers {
    val headerList = mutableListOf<String>()
    try {
        val jsonList = JSONArray(jsonString)
        val lookupMap = mutableSetOf<String>()

        for (i in 0 until jsonList.length()) {
            val map = jsonList.getJSONObject(i)
            val keys = map.keys()

            keys.forEach {
                val key = it
                val value = map[it]

                if (!lookupMap.add("$key | $value")) {
                    return@forEach
                }

                headerList.add(it.toString())
                headerList.add(map[it].toString())
            }
        }
    } catch (e: Exception) {
        // TODO:(naman) discuss what would be the best way to handle this?
        val localLogger = DependencyContainer.getInstance(null, null).getLocalLogger()
        localLogger.e(e)
    }

    return Headers.headersOf(*headerList.toTypedArray())
}

internal fun buildLibDownloadLogBody(
    module: String,
    status: Boolean? = null,
    timeElapsed: Long? = null,
): JSONObject {
    val body = mapOf("module" to module, "status" to status, "timeElapsed" to timeElapsed)
    return JSONObject(body)
}

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.android.sampleapp.e2e

import okhttp3.MediaType.Companion.toMediaType
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONArray
import org.json.JSONObject

object MockServerHelper {
    const val mockServerBaseUrl = "http://10.0.2.2:8080/mocker"
    private val client = OkHttpClient()

    suspend fun setExpectation(
        path: String,
        repeatCount: Int,
        expectedStatusCode: Int,
        expectedBody: Map<String, Any>,
        expectedHeaders: Map<String, String>,
        responseDelay: Int = 1,
    ) = makeNetworkCall(
        url = "$mockServerBaseUrl/expectation",
        body = mapOf(
            "path" to path,
            "response_delay" to responseDelay,
            "status_code" to expectedStatusCode,
            "body" to expectedBody as Any,
            "headers" to expectedHeaders,
            "repeat_count" to repeatCount
        ),
        method = "POST"
    )

    suspend fun deleteExpectation() = makeNetworkCall(
        url = "$mockServerBaseUrl/expectation",
        method = "DELETE",
    )

    suspend fun listExpectations(): Map<String, Any> {
        val res = makeNetworkCall(
            url = "$mockServerBaseUrl/expectations",
        )

        val json = JSONObject(res)
        return json.toMap()
    }

    suspend fun resetExpectations() = makeNetworkCall(
        url = "$mockServerBaseUrl/reset",
        method = "POST"
    )

    suspend fun getNetworkCallHistory():JSONArray{
        val res = makeNetworkCall(
            url = "$mockServerBaseUrl/history",
            method = "GET"
        )

        return JSONObject(res).getJSONArray("api_calls")
    }


    private fun JSONObject.toMap(): Map<String, Map<String, Any>> {
        val map = mutableMapOf<String, Map<String, Any>>()
        for (key in keys()) {
            val value = get(key)
            if (value is JSONObject) {
                map[key] = value.toMap()
            }
        }
        return map
    }

    private suspend fun makeNetworkCall(
        url: String,
        method: String = "GET",
        body: Map<String, Any>? = null,
        headers: Map<String, String> = emptyMap()
    ): String {
        val mediaType = "application/json; charset=utf-8".toMediaType()
        val requestBody =
            if (body == null) null else JSONObject(body).toString().toRequestBody(mediaType)

        val requestBuilder = Request.Builder().url(url)

        headers.forEach { (key, value) -> requestBuilder.addHeader(key, value) }

        when (method.uppercase()) {
            "POST" -> requestBuilder.post(requestBody ?: "".toRequestBody(mediaType))
            "DELETE" -> requestBuilder.delete(requestBody)
            "GET" -> requestBuilder.get()
            else -> throw IllegalArgumentException("Unsupported HTTP method: $method")
        }

        val request = requestBuilder.build()

        client.newCall(request).execute().use { response ->
            if (!response.isSuccessful) {
                throw RuntimeException("HTTP error: ${response.code}")
            }

            if (response.code < 200 || response.code >= 300) throw Exception("non-2xx status code")

            return response.body?.string() ?: throw RuntimeException("Response body is null")
        }
    }
}

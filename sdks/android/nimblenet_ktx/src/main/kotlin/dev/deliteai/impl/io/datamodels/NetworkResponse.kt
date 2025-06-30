/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.impl.io.datamodels

import ai.nimbleedge.impl.common.STATUS_CODES
import okhttp3.Response
import org.json.JSONObject

internal data class NetworkResponse(
    val statusCode: Int,
    val headers: String,
    val body: ByteArray,
    val bodyLength: Int,
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as NetworkResponse

        if (statusCode != other.statusCode) return false
        if (headers != other.headers) return false
        if (!body.contentEquals(other.body)) return false
        return bodyLength == other.bodyLength
    }

    override fun hashCode(): Int {
        var result = statusCode
        result = 31 * result + headers.hashCode()
        result = 31 * result + body.contentHashCode()
        result = 31 * result + bodyLength
        return result
    }
}

internal data class Header(val key: String, val value: String)

internal fun Response.toNetworkResponse(): NetworkResponse {
    val byteArrayBody: ByteArray = body?.bytes() ?: byteArrayOf()
    return NetworkResponse(
        statusCode = code,
        headers = JSONObject(headers.toMap()).toString(),
        body = byteArrayBody,
        bodyLength = byteArrayBody.size,
    )
}

internal fun emptyNetworkResponse(e: Throwable): NetworkResponse {
    val body = e.message.toString().toByteArray()
    return NetworkResponse(
        statusCode = STATUS_CODES.EMPTY_NETWORK_RESPONSE,
        headers = JSONObject(emptyMap<String, String>()).toString(),
        body = body,
        bodyLength = body.size,
    )
}

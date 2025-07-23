/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.io

import android.annotation.SuppressLint
import androidx.annotation.VisibleForTesting
import dev.deliteai.impl.common.STATUS_CODES
import dev.deliteai.impl.common.utils.jsonStringToHeaders
import dev.deliteai.impl.io.datamodels.FileDownloadStateTransition
import dev.deliteai.impl.io.datamodels.NetworkResponse
import dev.deliteai.impl.io.datamodels.emptyNetworkResponse
import dev.deliteai.impl.io.datamodels.toNetworkResponse
import dev.deliteai.impl.loggers.LocalLogger
import java.util.concurrent.TimeUnit
import kotlinx.coroutines.delay
import okhttp3.Headers
import okhttp3.MediaType
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.RequestBody
import okhttp3.RequestBody.Companion.toRequestBody
import okio.BufferedSink
import okio.GzipSink
import okio.IOException
import okio.buffer
import org.json.JSONArray

internal class Networking(
    private val client: OkHttpClient,
    private val localLogger: LocalLogger,
    private val chunkDownloadManager: ChunkDownloadManager,
) {
    private val JSON_MEDIA_TYPE = "application/json; charset=utf-8".toMediaType()

    suspend fun downloadNativeLibs(
        filePath: String,
        url: String,
        fileName: String,
        headers: List<Map<String, String>>,
        queryString: String,
        libsDownloadTimeLeftInMs: Long,
    ): Boolean {
        return runCatching {
                val checkFreqMs = 200L
                var timeLeft = libsDownloadTimeLeftInMs
                val urlWithQuery = buildString {
                    append(url)
                    if (queryString.isNotEmpty()) {
                        append('?').append(queryString)
                    }
                }

                while (timeLeft > 0) {
                    localLogger.d("Timeout available for $fileName: $timeLeft ms")
                    val fileDownloadInfo =
                        downloadFileThroughDownloadManager(
                            urlWithQuery,
                            JSONArray(headers).toString(),
                            fileName,
                            filePath,
                        )

                    when (fileDownloadInfo.currentState) {
                        STATUS_CODES.FILE_DOWNLOAD_SUCCESS -> return@runCatching true
                        STATUS_CODES.FILE_DOWNLOAD_FAILURE -> return@runCatching false
                        else -> {
                            timeLeft -= checkFreqMs
                            delay(checkFreqMs)
                        }
                    }
                }

                // timed out
                // intention is to give control back to the thread calling NimbleNet.initialize()
                // DM will continue the download even if the timeout is reached
                false
            }
            .onFailure { localLogger.e(it) }
            .getOrDefault(false)
    }

    fun sendRequest(
        url: String,
        requestHeaders: String,
        requestBody: String,
        requestBodyByte: ByteArray?,
        method: String,
        totalCallTimeoutInSecs: Int,
    ): NetworkResponse =
        runCatching {
                val headers = jsonStringToHeaders(requestHeaders)
                val request: Request =
                    when (method.uppercase()) {
                        "POST" -> buildPostRequest(url, headers, requestBodyByte, requestBody)
                        "GET" -> buildGetRequest(url, headers)
                        else -> error("Unsupported HTTP method: $method")
                    }

                // 0 means default timeout of okhttp
                val httpClient =
                    client
                        .newBuilder()
                        .readTimeout(0, TimeUnit.SECONDS)
                        .writeTimeout(0, TimeUnit.SECONDS)
                        .callTimeout(totalCallTimeoutInSecs.toLong(), TimeUnit.SECONDS)
                        .build()

                httpClient.newCall(request).execute().use { response ->
                    if (response.code in 400..599) {
                        localLogger.e("Request failed: $response")
                    }
                    response.toNetworkResponse()
                }
            }
            .onFailure { localLogger.e(it) }
            .getOrElse { emptyNetworkResponse(it) }

    @VisibleForTesting
    fun gzip(body: RequestBody): RequestBody {
        return object : RequestBody() {
            override fun contentType(): MediaType? {
                return body.contentType()
            }

            override fun contentLength(): Long {
                return -1
            }

            @Throws(IOException::class)
            override fun writeTo(sink: BufferedSink) {
                val gzipSink: BufferedSink = GzipSink(sink).buffer()
                body.writeTo(gzipSink)
                gzipSink.close()
            }
        }
    }

    @SuppressLint("Range")
    fun downloadFileThroughDownloadManager(
        url: String,
        requestHeaders: String,
        fileName: String,
        nimbleSdkDir: String,
    ): FileDownloadStateTransition =
        chunkDownloadManager.run(
            fileName = fileName,
            url = url,
            requestHeaders = requestHeaders,
            targetDirectory = nimbleSdkDir,
        )

    private fun buildPostRequest(
        url: String,
        headers: Headers,
        requestBodyByte: ByteArray?,
        requestBody: String,
    ): Request {
        val body =
            gzip(
                requestBodyByte?.toRequestBody(JSON_MEDIA_TYPE)
                    ?: requestBody.toRequestBody(JSON_MEDIA_TYPE)
            )
        return Request.Builder()
            .url(url)
            .headers(headers)
            .addHeader("Content-Encoding", "gzip")
            .post(body)
            .build()
    }

    private fun buildGetRequest(url: String, headers: Headers): Request =
        Request.Builder().url(url).headers(headers).get().build()
}

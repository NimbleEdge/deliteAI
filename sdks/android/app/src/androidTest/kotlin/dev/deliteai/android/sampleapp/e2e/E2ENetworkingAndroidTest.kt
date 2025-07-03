/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.android.sampleapp.e2e

import dev.deliteai.NimbleNet
import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.impl.common.NIMBLENET_VARIANTS
import android.app.Application
import androidx.test.core.app.ApplicationProvider
import androidx.test.ext.junit.runners.AndroidJUnit4
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withTimeout
import kotlinx.coroutines.withTimeoutOrNull
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class E2ENetworkingAndroidTest {
    private val context = ApplicationProvider.getApplicationContext<Application>()

    private suspend fun reset() {
        MockServerHelper.resetExpectations()
        initializeNimbleNetForNetworkingTests()
    }

    @Test
    fun assetDownloadShouldWorkInIdealNetworkConditions() = runBlocking {
        reset()
        waitForIsReady()
    }

    @Test
    fun assetDownloadShouldWorkAfterInitialServerFailure() = runBlocking {
        reset()

        val modelDownloadUrls = arrayOf(
            "/mds/api/v4/clients/testclient/assets/model/names/multiply_two_model/versions/1.0.0/formats/ort",
            "/mds/api/v4/clients/testclient/assets/model/names/add_model/versions/1.0.0/formats/ort",
        )

        modelDownloadUrls.forEach {
            MockServerHelper.setExpectation(
                it,
                2,
                404,
                expectedBody = mapOf(),
                expectedHeaders = mapOf(),
            )
        }

        initializeNimbleNetForNetworkingTests()

        withTimeout(60000) {
            waitForIsReady()
        }
    }

    @Test
    fun assetDownloadShouldNeverWorkAfterThreeServerFailure() = runBlocking {
        reset()

        val modelDownloadUrls = arrayOf(
            "/mds/api/v4/clients/testclient/assets/model/names/multiply_two_model/versions/1.0.0/formats/ort",
            "/mds/api/v4/clients/testclient/assets/model/names/add_model/versions/1.0.0/formats/ort",
        )

        modelDownloadUrls.forEach {
            MockServerHelper.setExpectation(
                it,
                3,
                404,
                expectedBody = mapOf(),
                expectedHeaders = mapOf(),
            )
        }

        initializeNimbleNetForNetworkingTests()

        withTimeoutOrNull(20000) {
            waitForIsReady()
        }

        Unit
    }

    @Test
    fun onServerFailuresAssetDownloadShouldExactlyTryThreeTimes() = runBlocking {
        reset()

        val modelDownloadUrls = arrayOf(
            "/mds/api/v4/clients/testclient/assets/model/names/multiply_two_model/versions/1.0.0/formats/ort",
            "/mds/api/v4/clients/testclient/assets/model/names/add_model/versions/1.0.0/formats/ort",
        )

        modelDownloadUrls.forEach {
            MockServerHelper.setExpectation(
                it,
                3,
                404,
                expectedBody = mapOf(),
                expectedHeaders = mapOf(),
            )
        }

        initializeNimbleNetForNetworkingTests()

        //wait for the SDK to make all the network calls
        delay(10000)

        val historyJsonArray = MockServerHelper.getNetworkCallHistory()
        val lookupTable = mutableMapOf<String, Int>()

        for (i in 0 until historyJsonArray.length()) {
            val history = historyJsonArray.getJSONObject(i)
            val path = history.getString("path")

            if (!modelDownloadUrls.contains("/$path")) continue

            if (lookupTable.containsKey(path)) {
                lookupTable[path] = lookupTable[path]!! + 1
            } else {
                lookupTable[path] = 1
            }
        }

        assertTrue(lookupTable.isNotEmpty())

        lookupTable.forEach {
            assertTrue(it.value == 3)
        }
    }

    @Test
    fun onSuccessCodeSDKShouldNotMakeExtraNetworkCalls() = runBlocking {
        reset()

        val modelDownloadUrls = arrayOf(
            "/mds/api/v4/clients/testclient/assets/model/names/multiply_two_model/versions/1.0.0/formats/ort",
            "/mds/api/v4/clients/testclient/assets/model/names/add_model/versions/1.0.0/formats/ort",
        )

        initializeNimbleNetForNetworkingTests()

        //wait for the SDK to make all the network calls
        delay(20000)

        val historyJsonArray = MockServerHelper.getNetworkCallHistory()
        val lookupTable = mutableMapOf<String, Int>()

        for (i in 0 until historyJsonArray.length()) {
            val history = historyJsonArray.getJSONObject(i)
            val path = history.getString("path")

            if (!modelDownloadUrls.contains("/$path")) continue

            if (lookupTable.containsKey(path)) {
                lookupTable[path] = lookupTable[path]!! + 1
            } else {
                lookupTable[path] = 1
            }
        }

        assertTrue(lookupTable.isNotEmpty())

        lookupTable.forEach {
            assertTrue(it.value == 1)
        }
    }

    private fun initializeNimbleNetForNetworkingTests() {
        val nimblenetConfig = NimbleNetConfig(
            clientId = "testclient",
            host = "http://10.0.2.2:8080",
            deviceId = "android-test",
            clientSecret = "dummy",
            debug = true,
            initTimeOutInMs = 3000000000,
            compatibilityTag = "MODEL_ADDITION",
            libraryVariant = NIMBLENET_VARIANTS.STATIC
        )

        check(NimbleNet.initialize(context, nimblenetConfig).status)
    }

    private suspend fun waitForIsReady() {
        while (!NimbleNet.isReady().status) delay(1000)
    }
}

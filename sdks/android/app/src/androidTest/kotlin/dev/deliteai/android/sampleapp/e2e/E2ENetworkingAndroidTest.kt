/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.android.sampleapp.e2e

import dev.deliteai.NimbleNet
import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.datamodels.NimbleNetTensor
import dev.deliteai.impl.common.DATATYPE
import dev.deliteai.impl.common.NIMBLENET_VARIANTS
import android.app.Application
import androidx.test.core.app.ApplicationProvider
import androidx.test.ext.junit.runners.AndroidJUnit4
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withTimeout
import kotlinx.coroutines.withTimeoutOrNull
import org.json.JSONArray
import org.json.JSONObject
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


    @Test
    fun initializeSDKInOfflineMode() = runBlocking {
        MockServerHelper.resetExpectations()

        // Provide paths relative to assets folder
        val assetsJson = """
        [
            {
                "name": "workflow_script",
                "version": "1.0.0",
                "type": "script",
                "location": {
                    "path": "e2e_test_assets/add_script.ast"
                }
            },
            {
                "name": "add_model",
                "version": "1.0.0",
                "type": "model",
                "location": {
                    "path": "e2e_test_assets/add_two_model.onnx"
                }
            }
        ]
    """.trimIndent()


        val nimblenetConfig = NimbleNetConfig(
            debug = true,
            online = false
        )

        check(NimbleNet.initialize(context, nimblenetConfig, JSONArray(assetsJson)).status)

        waitForIsReady()

        // Do run method and its output assertion
        val input = hashMapOf(
            "num" to NimbleNetTensor(
                data = longArrayOf(2),
                datatype = DATATYPE.INT64,
                shape = intArrayOf(1)
            )
        )

        val output = NimbleNet.runMethod("add", input)
        assert(output.status)
        assert(output.payload?.size == 1)
        assert((output.payload!!["output"]!!.data as LongArray)[0].toInt() == 4)

        val historyJsonArray = MockServerHelper.getNetworkCallHistory()
        assert(historyJsonArray.length() == 0)
    }

    @Test
    fun runRetrieverInOfflineMode() = runBlocking {
        MockServerHelper.resetExpectations()

        // Provide paths relative to assets folder
        val assetsJson = """
        [
            {
                "name": "workflow_script",
                "version": "1.0.0",
                "type": "script",
                "location": {
                    "path": "retriever/retriever.ast"
                }
            },
            {
                "name": "GroceryRAG",
                "version": "1.0.0",
                "type": "retriever",
                "arguments": [
                    {
                        "name": "embeddingModel",
                        "version": "1.0.0",
                        "type": "model",
                        "location": {
                            "path": "retriever/embedding_model.onnx"
                        }
                    },
                    {
                        "name": "embeddingStoreModel",
                        "version": "1.0.0",
                        "type": "model",
                        "location": {
                            "path": "retriever/embedding_store_model.onnx"
                        }
                    },
                    {
                        "name": "groceryItems",
                        "version": "1.0.0",
                        "type": "document",
                        "location": {
                            "path": "retriever/grocery.json"
                        }
                    }
                ]
            }
        ]
    """.trimIndent()

        val nimblenetConfig = NimbleNetConfig(
            debug = true,
            online = false
        )

        check(NimbleNet.initialize(context, nimblenetConfig, JSONArray(assetsJson)).status)

        waitForIsReady()

        check(NimbleNet.runMethod("run_llm", hashMapOf()).status)

        var output = NimbleNet.runMethod("get_description", hashMapOf())
        assert(output.status)
        assert(output.payload?.size == 1)
        assert(output.payload!!["description"]!!.data as String == "this is a description")

        val itemsList = mutableListOf<String>()

        fun getItems() {
            output = NimbleNet.runMethod("get_next_item", hashMapOf())
            assert(output.status)
            var item = (output.payload!!["item"]!!.data as JSONObject).get("ProductName") as String
            itemsList.add(item)

            while (true) {
                output = NimbleNet.runMethod("get_next_item", hashMapOf())
                assert(output.status)
                if (!output.payload!!.containsKey("relevance_score")){break}
                item = (output.payload!!["item"]!!.data as JSONObject).get("ProductName") as String
                itemsList.add(item)
            }
        }

        getItems()
        check(itemsList.any { it.contains("milk", ignoreCase = true) }) { "No item contains 'milk'" }
        check(itemsList.any { it.contains("paneer", ignoreCase = true) }) { "No item contains 'paneer'" }
        check(itemsList.any { it.contains("noodles", ignoreCase = true) }) { "No item contains 'paneer'" }
        check(itemsList.any { it.contains("eggs", ignoreCase = true) }) { "No item contains 'eggs'" }

        val historyJsonArray = MockServerHelper.getNetworkCallHistory()
        assert(historyJsonArray.length() == 0)
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
            libraryVariant = NIMBLENET_VARIANTS.STATIC,
            online = true,
        )

        check(NimbleNet.initialize(context, nimblenetConfig).status)
    }

    private suspend fun waitForIsReady() {
        while (!NimbleNet.isReady().status) delay(1000)
    }
}

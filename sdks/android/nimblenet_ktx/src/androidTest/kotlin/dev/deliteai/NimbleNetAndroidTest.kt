/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai

import android.app.Application
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import dev.deliteai.datamodels.NimbleNetTensor
import dev.deliteai.impl.common.DATATYPE
import dev.deliteai.testUtils.nnConfig
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.runBlocking
import org.json.JSONArray
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

// nimbleSDK can provide converters from common JVM types to NimbleNetTensor.
// Here are some options:
// (1) Create extension method `.toNimbleNetTensor()` for the supported types, as done below.
//     Con: In the app, it'll need multiple imports. So, feels ugly.
// (2) Create `NimbleNetTensor.create(from: Any)`.
//     It picks the runtime type of the input and returns a NimbleNetTensor for the supported types.
// (3) `NimbleNet.runMethod` accepts `HashMap<String, Any>?` instead of
//     `HashMap<String, NimbleNetTensor>?`.
//     It preserves the tensors, but converts other supported types to tensors internally.
//     Con: It goes a step backwards from type safety. Kotlin devs must be expecting type safety.
fun Boolean.toNimbleNetTensor() =
    NimbleNetTensor(data = this, datatype = DATATYPE.BOOL, shape = intArrayOf())

typealias NimbleNetTensorMap = HashMap<String, NimbleNetTensor>

typealias DelitePyForeignFunction = (NimbleNetTensorMap?) -> NimbleNetTensorMap?

fun createNimbleNetTensorFromForeignFunction(fn: DelitePyForeignFunction) =
    NimbleNetTensor(data = fn, datatype = DATATYPE.FUNCTION, shape = intArrayOf())

@RunWith(AndroidJUnit4::class)
class NimbleNetAndroidTest {
    private val application =
        InstrumentationRegistry.getInstrumentation().targetContext.applicationContext as Application

    @Before
    fun initializeNimbleNet() =
        runBlocking(Dispatchers.Default) {
            val res = NimbleNet.initialize(application, nnConfig)
            if (!res.status) throw RuntimeException(res.error?.message)

            var time = 0
            while (true) {
                val isReady = NimbleNet.isReady()
                if (isReady.status) break
                if (time > 30) throw RuntimeException("TLE")
                time++
                kotlinx.coroutines.delay(5000)
            }
        }

    @Test
    fun verifyInt32Conversion() = runBlocking {
        val modelInputs =
            hashMapOf(
                "model_input" to
                    NimbleNetTensor(
                        data = intArrayOf(1, 2, 3, 4),
                        shape = intArrayOf(4),
                        datatype = DATATYPE.INT32,
                    )
            )
        val result = NimbleNet.runMethod("int32", modelInputs)
        val actualOutput = result.payload!!["model_output"]!!.data as IntArray
        val expectedOutput = modelInputs["model_input"]?.data as IntArray
        assertTrue(
            result.status && actualOutput.contentEquals(expectedOutput.map { it * 2 }.toIntArray())
        )
    }

    @Test
    fun verifyInt64Conversion() = runBlocking {
        val modelInputs =
            hashMapOf(
                "model_input" to
                    NimbleNetTensor(
                        data = longArrayOf(1, 2, 3, 4),
                        shape = intArrayOf(4),
                        datatype = DATATYPE.INT64,
                    )
            )
        val result = NimbleNet.runMethod("int64", modelInputs)
        val actualOutput = result.payload!!["model_output"]!!.data as LongArray
        val expectedOutput = modelInputs["model_input"]?.data as LongArray
        assertTrue(
            result.status && actualOutput.contentEquals(expectedOutput.map { it * 2 }.toLongArray())
        )
    }

    @Test
    fun verifyFp32Conversion() = runBlocking {
        val modelInputs =
            hashMapOf(
                "model_input" to
                    NimbleNetTensor(
                        data = floatArrayOf(1f, 2f, 3f, 4f),
                        shape = intArrayOf(4),
                        datatype = DATATYPE.FLOAT,
                    )
            )
        val result = NimbleNet.runMethod("float32", modelInputs)
        val actualOutput = result.payload!!["model_output"]!!.data as FloatArray
        val expectedOutput = modelInputs["model_input"]?.data as FloatArray
        assertTrue(
            result.status &&
                actualOutput.contentEquals(expectedOutput.map { it * 2 }.toFloatArray())
        )
    }

    @Test
    fun verifyFp64Conversion() = runBlocking {
        val modelInputs =
            hashMapOf(
                "model_input" to
                    NimbleNetTensor(
                        data = doubleArrayOf(1.0, 2.0, 3.0, 4.0),
                        shape = intArrayOf(4),
                        datatype = DATATYPE.DOUBLE,
                    )
            )
        val result = NimbleNet.runMethod("float64", modelInputs)
        val actualOutput = result.payload!!["model_output"]!!.data as DoubleArray
        val expectedOutput = modelInputs["model_input"]?.data as DoubleArray
        assertTrue(
            result.status &&
                actualOutput.contentEquals(expectedOutput.map { it * 2 }.toDoubleArray())
        )
    }

    @Test
    fun verifyStringConversion() = runBlocking {
        val modelInputs =
            hashMapOf(
                "model_input" to
                    NimbleNetTensor(
                        data = arrayOf("MEDIUM-STR"),
                        shape = intArrayOf(1),
                        datatype = DATATYPE.STRING,
                    )
            )
        val result = NimbleNet.runMethod("string", modelInputs)
        val actualOutput = result.payload!!["model_output"]!!.data as Array<String>
        assertTrue(result.status && actualOutput.contentEquals(arrayOf("medium-str")))
    }

    @Test
    fun verifySingularInt32Conversion() = runBlocking {
        val modelInputs =
            hashMapOf(
                "model_input" to NimbleNetTensor(data = 85, shape = null, datatype = DATATYPE.INT32)
            )
        val result = NimbleNet.runMethod("int32_singular", modelInputs)

        // INT32 * constant = INT64 in the script
        val actualOutput = result.payload!!["model_output"]!!.data as Long
        assertTrue(
            result.status &&
                actualOutput == ((modelInputs["model_input"]?.data as Int).toLong() * 2)
        )
    }

    @Test
    fun verifySingularInt64Conversion() = runBlocking {
        val modelInputs =
            hashMapOf(
                "model_input" to
                    NimbleNetTensor(data = 85L, shape = null, datatype = DATATYPE.INT64)
            )
        val result = NimbleNet.runMethod("int64_singular", modelInputs)
        val actualOutput = result.payload!!["model_output"]!!.data as Long
        assertTrue(result.status && actualOutput == (modelInputs["model_input"]?.data as Long * 2))
    }

    @Test
    fun verifySingularFp32Conversion() = runBlocking {
        val modelInputs =
            hashMapOf(
                "model_input" to
                    NimbleNetTensor(data = 85.6f, shape = null, datatype = DATATYPE.FLOAT)
            )
        val result = NimbleNet.runMethod("float32_singular", modelInputs)
        val actualOutput = result.payload!!["model_output"]!!.data as Float
        assertTrue(result.status && actualOutput == (modelInputs["model_input"]?.data as Float * 2))
    }

    @Test
    fun verifySingularFp64Conversion() = runBlocking {
        val modelInputs =
            hashMapOf(
                "model_input" to
                    NimbleNetTensor(data = 85.6, shape = null, datatype = DATATYPE.DOUBLE)
            )
        val result = NimbleNet.runMethod("float64_singular", modelInputs)
        val actualOutput = result.payload!!["model_output"]!!.data as Double
        assertTrue(
            result.status && actualOutput == (modelInputs["model_input"]?.data as Double * 2)
        )
    }

    @Test
    fun verifySingularStringConversion() = runBlocking {
        val modelInputs =
            hashMapOf(
                "model_input" to
                    NimbleNetTensor(
                        data = "singularStringInput",
                        shape = null,
                        datatype = DATATYPE.STRING,
                    )
            )
        val result = NimbleNet.runMethod("string_singular", modelInputs)
        val actualOutput = result.payload!!["model_output"]!!.data as String
        assertTrue(result.status && actualOutput == "rightInputString")
    }

    // Json specific tests
    @Test
    fun verifySimpleJsonConversion() = runBlocking {
        val simpleJson = JSONObject(mapOf("name" to "John Doe", "age" to 30, "isVerified" to true))

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = simpleJson, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(simpleJson, modelOutput))
    }

    @Test
    fun verifyNestedJsonConversion() = runBlocking {
        val nestedJson =
            JSONObject(
                mapOf(
                    "user" to
                        mapOf(
                            "id" to 123,
                            "profile" to mapOf("firstName" to "Jane", "lastName" to "Doe"),
                        )
                )
            )

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = nestedJson, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(nestedJson, modelOutput))
    }

    @Test
    fun verifyMultipleEmptyJsonConversion() = runBlocking {
        val nestedJson =
            JSONObject(
                mapOf(
                    "user" to
                        mapOf(
                            "id" to 123,
                            "profile" to
                                mapOf(
                                    "firstName" to listOf<Int>(),
                                    "lastName" to listOf(JSONObject()),
                                ),
                        )
                )
            )

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = nestedJson, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(nestedJson, modelOutput))
    }

    @Test
    fun verifyJsonArrayConversion() = runBlocking {
        val nestedJson =
            JSONArray(
                listOf(
                    listOf(1, 2, 3),
                    listOf("A", "B", "C"),
                    listOf(true, false, mapOf("hello" to "bro")),
                )
            )

        val modelInputs =
            hashMapOf(
                "key" to
                    NimbleNetTensor(
                        shape = intArrayOf(3),
                        data = nestedJson,
                        datatype = DATATYPE.JSON_ARRAY,
                    )
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONArray

        assertTrue(result.status && compareJsonArrays(nestedJson, modelOutput))
    }

    @Test
    fun verifyJsonArrayInsideJsonConversion() = runBlocking {
        val jsonWithArray =
            JSONObject(
                mapOf(
                    "items" to
                        listOf(
                            mapOf("id" to 1, "value" to "A"),
                            mapOf("id" to 2, "value" to "B"),
                            mapOf("id" to 3, "value" to "C"),
                        )
                )
            )

        val modelInputs =
            hashMapOf(
                "key" to
                    NimbleNetTensor(shape = null, data = jsonWithArray, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonWithArray, modelOutput))
    }

    @Test
    fun verifyJsonArrayInsideJsonArrayConversion() = runBlocking {
        val arrayInsideArray =
            JSONArray(listOf(listOf(1, 2, 3), listOf("A", "B", "C"), listOf(true, false)))

        val modelInputs =
            hashMapOf(
                "key" to
                    NimbleNetTensor(
                        shape = intArrayOf(3),
                        data = arrayInsideArray,
                        datatype = DATATYPE.JSON_ARRAY,
                    )
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONArray

        assertTrue(result.status && compareJsonArrays(arrayInsideArray, modelOutput))
    }

    @Test
    fun verifyEmptyJsonConversion() = runBlocking {
        val emptyJson = JSONObject()

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = emptyJson, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(emptyJson, modelOutput))
    }

    @Test
    fun verifyMixedTypeJsonConversion() = runBlocking {
        val mixedTypeJson =
            JSONObject(
                mapOf(
                    "string" to "value",
                    "number" to 123,
                    "boolean" to true,
                    "nullValue" to null,
                    "array" to listOf("one", 2, true),
                    "nestedObject" to mapOf("key1" to "value1", "key2" to 99),
                )
            )

        val modelInputs =
            hashMapOf(
                "key" to
                    NimbleNetTensor(shape = null, data = mixedTypeJson, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(mixedTypeJson, modelOutput))
    }

    @Test
    fun verifyEmptyArrayConversion() = runBlocking {
        val jsonObject = JSONObject(mapOf("emptyArray" to JSONArray()))

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyNestedEmptyObjectConversion() = runBlocking {
        val jsonObject =
            JSONObject(mapOf("level1" to mapOf("level2" to mapOf("level3" to JSONObject()))))

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyArrayOfObjectsConversion() = runBlocking {
        val jsonObject =
            JSONObject(
                mapOf(
                    "items" to
                        listOf(
                            mapOf("id" to 1, "name" to "Item1"),
                            mapOf("id" to 2, "name" to "Item2"),
                        )
                )
            )

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyArrayOfMixedTypesConversion() = runBlocking {
        val jsonObject = JSONObject(mapOf("mixedArray" to listOf(1, "two", true, JSONObject.NULL)))

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyArrayWithNestedObjectsConversion() = runBlocking {
        val jsonObject =
            JSONObject(
                mapOf(
                    "items" to
                        listOf(
                            mapOf("id" to 1, "details" to mapOf("key" to "value")),
                            mapOf("id" to 2, "details" to mapOf("key" to "anotherValue")),
                        )
                )
            )

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyObjectWithEmptyNestedArrayConversion() = runBlocking {
        val jsonObject = JSONObject(mapOf("data" to mapOf("items" to JSONArray())))

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyJsonWithBooleanKeysConversion() = runBlocking {
        val jsonObject = JSONObject(mapOf("trueKey" to true, "falseKey" to false))

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyJsonWithLargeNumbersConversion() = runBlocking {
        val jsonObject = JSONObject(mapOf("largeNumber" to 99999999, "smallNumber" to 0.0000001))

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyJsonWithEmptyStringsConversion() = runBlocking {
        val jsonObject = JSONObject(mapOf("key1" to "", "key2" to " "))

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyNestedEmptyJsonObjectsConversion() = runBlocking {
        val jsonObject =
            JSONObject(
                mapOf(
                    "level1" to
                        JSONObject(mapOf("level2" to JSONObject(mapOf("level3" to JSONObject()))))
                )
            )

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyDeeplyNestedArraysConversion() = runBlocking {
        val jsonObject =
            JSONObject(mapOf("array" to listOf(listOf(listOf(listOf(listOf(1, 2, 3)))))))

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyMixedNestingConversion() = runBlocking {
        val jsonObject =
            JSONObject(
                mapOf(
                    "level1" to
                        JSONObject(
                            mapOf(
                                "array" to
                                    listOf(
                                        JSONObject(mapOf("key1" to "value1")),
                                        listOf(1, 2, 3),
                                        JSONObject(mapOf("key2" to listOf(4, 5, 6))),
                                    )
                            )
                        )
                )
            )

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyArrayOfEmptyJsonObjectsConversion() = runBlocking {
        val jsonObject =
            JSONObject(mapOf("array" to listOf(JSONObject(), JSONObject(), JSONObject())))

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyArrayOfNestedObjectsConversion() = runBlocking {
        val jsonObject =
            JSONObject(
                mapOf(
                    "array" to
                        listOf(
                            JSONObject(mapOf("level1" to JSONObject(mapOf("level2" to "value")))),
                            JSONObject(mapOf("key1" to "value1")),
                        )
                )
            )

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyNestedObjectsWithMixedDataTypesConversion() = runBlocking {
        val jsonObject =
            JSONObject(
                mapOf(
                    "level1" to
                        JSONObject(
                            mapOf(
                                "boolean" to true,
                                "number" to 123,
                                "string" to "value",
                                "array" to listOf("a", "b", "c"),
                                "object" to JSONObject(mapOf("key" to "value")),
                            )
                        )
                )
            )

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyEmptyArraysInNestedStructures() = runBlocking {
        val jsonObject =
            JSONObject(mapOf("level1" to JSONObject(mapOf("array" to emptyList<Any>()))))

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyArraysWithMixedDataTypes() = runBlocking {
        val jsonObject =
            JSONObject(
                mapOf("array" to listOf(1, "string", true, JSONObject(mapOf("key" to "value"))))
            )

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyLongStringValuesOrLargeNumbers() = runBlocking {
        val jsonObject =
            JSONObject(mapOf("longString" to "a".repeat(100), "largeNumber" to 999999999))

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyDeepNestingExceedingLimits() = runBlocking {
        var nestedJson = JSONObject()
        repeat(100) { nestedJson = JSONObject(mapOf("level" to nestedJson)) }

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = nestedJson, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(nestedJson, modelOutput))
    }

    @Test
    fun verifyHugeJsonInput() = runBlocking {
        val jsonObject = JSONObject()
        for (i in 1..5000) {
            val nestedObject = JSONObject()
            val nestedArray = JSONArray()

            for (j in 1..5) {
                nestedObject.put("nestedKey$j", "nestedValue$j for key$i")
            }

            for (k in 1..3) {
                nestedArray.put("arrayValue$k for key$i")
            }

            jsonObject.put(
                "key$i",
                JSONObject().apply {
                    put("nestedObject", nestedObject)
                    put("nestedArray", nestedArray)
                },
            )
        }

        val modelInputs =
            hashMapOf(
                "key" to NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("complex_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as JSONObject

        assertTrue(result.status && compareJsonObjectsDeep(jsonObject, modelOutput))
    }

    @Test
    fun verifyNullHandlingJSON() = runBlocking {
        val jsonObject =
            JSONObject(
                mapOf(
                    "string" to "value",
                    "number" to 123,
                    "boolean" to true,
                    "nullValue" to null,
                    "array" to listOf("one", 2, true),
                    "nestedObject" to mapOf("key1" to "value1", "key2" to 99),
                )
            )

        val modelInputs =
            hashMapOf(
                "input" to
                    NimbleNetTensor(shape = null, data = jsonObject, datatype = DATATYPE.JSON)
            )

        val result = NimbleNet.runMethod("test_null_json", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as String

        assertTrue(result.status && modelOutput == "NO")
    }

    @Test
    fun verifyNullHandlingJSONArray() = runBlocking {
        val jsonArray =
            JSONArray(
                listOf(
                    mapOf(
                        "string" to "value",
                        "number" to 123,
                        "boolean" to true,
                        "nullValue" to null,
                        "array" to listOf("one", 2, true),
                        "nestedObject" to mapOf("key1" to "value1", "key2" to 99),
                    ),
                    "hello",
                    "world",
                    12.47,
                    -87,
                    null,
                    234234234,
                    listOf("some", null),
                )
            )

        val modelInputs =
            hashMapOf(
                "input" to
                    NimbleNetTensor(
                        shape = intArrayOf(8),
                        data = jsonArray,
                        datatype = DATATYPE.JSON_ARRAY,
                    )
            )

        val result = NimbleNet.runMethod("test_null_jsonarray", modelInputs)
        val modelOutput = result.payload?.get("key")?.data as String

        assertTrue(result.status && modelOutput == "NO")
    }

    @Test
    fun verifyForeignFunctionInvocation() = runBlocking {
        /*
            DelitePy script:
            ```python
            def generate_fibonacci(inp):
                collector = inp["collector"]
                num_emitted = 0

                def emit(v):
                    # num_emitted += 1    # does not work
                    num_emitted = num_emitted + 1
                    return collector({"value": v})["stop"]

                def ret():
                    return {"count": num_emitted}

                a = 0
                stop = emit(a)
                if stop:
                    return ret()

                b = 1
                while True:
                    stop = emit(b)
                    if stop:
                        return ret()

                    a, b = b, a + b
            ```
        */
        fun generateFibonacci(n: Int): List<Long> {
            val fibs = mutableListOf<Long>()
            val res =
                NimbleNet.runMethod(
                    "generate_fibonacci",
                    hashMapOf(
                        "collector" to
                            createNimbleNetTensorFromForeignFunction(
                                fun(inp: NimbleNetTensorMap?): NimbleNetTensorMap? {
                                    val valueTensor = inp!!["value"] as NimbleNetTensor
                                    assertEquals(valueTensor.datatype, DATATYPE.INT64)
                                    val fib = valueTensor.data as Long
                                    fibs.add(fib)
                                    return hashMapOf("stop" to (fibs.size == n).toNimbleNetTensor())
                                }
                            )
                    ),
                )
            val countTensor = res.payload!!["count"]!!
            assertEquals(countTensor.datatype, DATATYPE.INT64)
            assertEquals(countTensor.data, fibs.size.toLong())
            return fibs
        }
        assertEquals(generateFibonacci(6), listOf<Long>(0, 1, 1, 2, 3, 5))

        // TODO (puneet): test behaviour when a null foreign function is passed
        //  pre-requisite: fix needed in verifyUserInputs() in NimbleNetController.kt
    }

    private fun compareJsonObjectsDeep(json1: JSONObject, json2: JSONObject): Boolean {
        val keys1 = json1.keys().asSequence().toSet()
        val keys2 = json2.keys().asSequence().toSet()
        if (keys1 != keys2) return false

        for (key in keys1) {
            val value1 = json1[key]
            val value2 = json2[key]
            when {
                value1 is JSONObject && value2 is JSONObject -> {
                    if (!compareJsonObjectsDeep(value1, value2)) return false
                }

                value1 is JSONArray && value2 is JSONArray -> {
                    if (!compareJsonArrays(value1, value2)) return false
                }

                value1 is Number && value2 is Number -> {
                    if (value1.toDouble() != value2.toDouble()) return false
                }

                value1 != value2 -> {
                    return false
                }
            }
        }
        return true
    }

    private fun compareJsonArrays(array1: JSONArray, array2: JSONArray): Boolean {
        if (array1.length() != array2.length()) return false
        for (i in 0 until array1.length()) {
            val value1 = array1[i]
            val value2 = array2[i]
            when {
                value1 is JSONObject && value2 is JSONObject -> {
                    if (!compareJsonObjectsDeep(value1, value2)) return false
                }

                value1 is JSONArray && value2 is JSONArray -> {
                    if (!compareJsonArrays(value1, value2)) return false
                }

                value1 is Number && value2 is Number -> {
                    if (value1.toDouble() != value2.toDouble()) return false
                }

                value1 != value2 -> {
                    return false
                }
            }
        }
        return true
    }
}

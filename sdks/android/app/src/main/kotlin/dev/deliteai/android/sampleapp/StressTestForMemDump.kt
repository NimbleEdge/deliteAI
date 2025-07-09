/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.android.sampleapp

import dev.deliteai.NimbleNet
import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.datamodels.NimbleNetTensor
import dev.deliteai.impl.common.DATATYPE
import dev.deliteai.impl.common.NIMBLENET_VARIANTS
import android.app.Application
import android.util.Log
import androidx.compose.foundation.layout.Box
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.platform.LocalContext
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import org.json.JSONArray
import kotlin.random.Random

@Composable
fun StressTestForMemDump() {
    val coroutineScope = rememberCoroutineScope()
    val application = LocalContext.current.applicationContext
    val runMethodIterations = 100
    var iteration = 0
    val arrayLength = 100000
    val outputText = remember {
        mutableStateOf("INIT")
    }


    suspend fun initialize() {
        val nimblenetConfig = NimbleNetConfig(
            clientId = "test",
            host = "test",
            deviceId = "test",
            clientSecret = "test",
            debug = false,
            initTimeOutInMs = 1000000000,
            compatibilityTag = "test",
            libraryVariant = NIMBLENET_VARIANTS.STATIC,
            online = true
        )

        check(NimbleNet.initialize(application as Application, nimblenetConfig).status)
        while (!NimbleNet.isReady().status) delay(1000)
        Log.i("STRESS-TEST", "SDK IS READY FOR INFERENCE")
        outputText.value = "PREDICT"
    }

    fun runMethod() {
        Log.i("STRESS-TEST", "runMethod: $iteration")
        iteration += 1

        val inputMap = hashMapOf(
            "int32" to NimbleNetTensor(
                data = IntArray(arrayLength).map { Random.nextInt(0, 100) }.toIntArray(),
                datatype = DATATYPE.INT32,
                shape = intArrayOf(arrayLength)
            ),
            "int64" to NimbleNetTensor(
                data = LongArray(arrayLength).map { Random.nextLong(0, 100) }.toLongArray(),
                datatype = DATATYPE.INT64,
                shape = intArrayOf(arrayLength)
            ),
            "float" to NimbleNetTensor(
                data = FloatArray(arrayLength).map { Random.nextFloat() * 100 }.toFloatArray(),
                datatype = DATATYPE.FLOAT,
                shape = intArrayOf(arrayLength)
            ),
            "double" to NimbleNetTensor(
                data = DoubleArray(arrayLength).map { Random.nextDouble() * 100 }.toDoubleArray(),
                datatype = DATATYPE.DOUBLE,
                shape = intArrayOf(arrayLength)
            ),
            "bool" to NimbleNetTensor(
                data = BooleanArray(arrayLength).map { false }.toBooleanArray(),
                datatype = DATATYPE.BOOL,
                shape = intArrayOf(arrayLength)
            ),
            "string" to NimbleNetTensor(
                data = Array(arrayLength) {
                    "RANDOM WORD"
                },
                datatype = DATATYPE.STRING,
                shape = intArrayOf(arrayLength)
            ),
            "json_double" to NimbleNetTensor(
                data = JSONArray(List(arrayLength) { 2.0 }),
                datatype = DATATYPE.JSON_ARRAY,
                shape = intArrayOf(arrayLength)
            ),
            "json_long" to NimbleNetTensor(
                data = JSONArray(List(arrayLength) { 2L }),
                datatype = DATATYPE.JSON_ARRAY,
                shape = intArrayOf(arrayLength)
            ),
            "json_bool" to NimbleNetTensor(
                data = JSONArray(List(arrayLength) { false }),
                datatype = DATATYPE.JSON_ARRAY,
                shape = intArrayOf(arrayLength)
            ),
            "json_str" to NimbleNetTensor(
                data = JSONArray(List(arrayLength) { "RANDOM WORD" }),
                datatype = DATATYPE.JSON_ARRAY,
                shape = intArrayOf(arrayLength)
            ),


            "int32_s" to NimbleNetTensor(
                data = 234237,
                datatype = DATATYPE.INT32,
                shape = null
            ),
            "int64_s" to NimbleNetTensor(
                data = 274982035734895L,
                datatype = DATATYPE.INT64,
                shape = null
            ),
            "float_s" to NimbleNetTensor(
                data = 843734.75f,
                datatype = DATATYPE.FLOAT,
                shape = null
            ),
            "double_s" to NimbleNetTensor(
                data = 774834.94746748,
                datatype = DATATYPE.DOUBLE,
                shape = null
            ),
            "string_s" to NimbleNetTensor(
                data = "A".repeat(arrayLength),
                datatype = DATATYPE.STRING,
                shape = null
            ),
        )

        val res = NimbleNet.runMethod("mirror_func", inputMap)

        if (!res.status) {
            Log.i("STRESS-TEST", "FAIL | ${res.error?.message}")
        }
    }

    LaunchedEffect(Unit) {
        coroutineScope.launch(Dispatchers.Default) {
            delay(5000)

            initialize()

            for (i in 0 until runMethodIterations) runMethod()

            System.gc();

            Log.i("STRESS-TEST", "Requesting Garbage Collection")

            delay(5000)
            System.exit(0)
        }
    }

    Box { Text(outputText.value) }
}

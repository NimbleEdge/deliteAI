/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.controllers

import android.app.Application
import android.os.SystemClock
import dev.deliteai.NimbleNet
import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.datamodels.NimbleNetResult
import dev.deliteai.datamodels.NimbleNetTensor
import dev.deliteai.datamodels.UserEventData
import dev.deliteai.impl.common.DATATYPE
import dev.deliteai.impl.common.HardwareInfo
import dev.deliteai.impl.common.MESSAGES
import dev.deliteai.impl.common.METRIC_TYPE
import dev.deliteai.impl.coroutine.DeliteAiScope
import dev.deliteai.impl.loggers.RemoteLogger
import dev.deliteai.impl.moduleInstallers.ModuleInstaller
import dev.deliteai.impl.nativeBridge.CoreRuntime
import dev.deliteai.testUtils.nnConfig
import io.mockk.MockKAnnotations
import io.mockk.Runs
import io.mockk.coVerify
import io.mockk.every
import io.mockk.just
import io.mockk.mockk
import io.mockk.mockkConstructor
import io.mockk.mockkObject
import io.mockk.mockkStatic
import io.mockk.slot
import io.mockk.spyk
import io.mockk.verify
import io.mockk.verifyOrder
import junit.framework.Assert.assertFalse
import kotlin.test.assertTrue
import org.json.JSONArray
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

class NimbleNetControllerTest {
    private lateinit var nimbleNetController: NimbleNetController
    private val applicationMocked: Application = mockk(relaxed = true)
    private val coreRuntimeInterfaceMocked: CoreRuntime = mockk(relaxed = true)
    private val moduleInstaller: ModuleInstaller = mockk(relaxed = true)
    private val hardwareInfoMocked: HardwareInfo = mockk(relaxed = true)
    private val remoteLoggerMocked: RemoteLogger = mockk(relaxed = true)
    private val fileUtilsMocked: dev.deliteai.impl.io.FileUtils = mockk(relaxed = true)

    @Before
    fun setup() {
        MockKAnnotations.init(this)
        mockkStatic(SystemClock::class)
        mockkConstructor(NimbleNetResult::class)
        every { SystemClock.elapsedRealtimeNanos() } returns 1234567890L
        mockkObject(NimbleNetConfig)
        every { NimbleNetConfig.fromString(any()) } returns nnConfig

        nimbleNetController =
            NimbleNetController(
                application = applicationMocked,
                deliteAiScope = DeliteAiScope(),
                fileUtils = fileUtilsMocked,
                hardwareInfo = hardwareInfoMocked,
                moduleInstaller = moduleInstaller,
                coreRuntime = coreRuntimeInterfaceMocked,
                remoteLogger = remoteLoggerMocked,
            )
    }

    @Test
    fun `isInitialised should return false by default`() {
        assertFalse(nimbleNetController.isNimbleNetInitialized())
    }

    @Test
    fun `initialize should load library and set isInitialized to true on success`() {
        val nimbleNetConfigModified = nnConfig
        nnConfig.setInternalDeviceId("random")
        val resultSlot = slot<NimbleNetResult<Unit>>()

        every {
            coreRuntimeInterfaceMocked.initializeNimbleNet(
                any(),
                any(),
                any(),
                any(),
                capture(resultSlot),
            )
        } answers { resultSlot.captured.status = true }

        nimbleNetController.initialize(nimbleNetConfigModified)
        assertTrue(nimbleNetController.isNimbleNetInitialized())

        verify { remoteLoggerMocked.writeMetric(METRIC_TYPE.STATIC_DEVICE_METRICS, any()) }
        verify { remoteLoggerMocked.writeMetric(METRIC_TYPE.INTERNAL_STORAGE_METRIC, any()) }
        coVerify { moduleInstaller.execute() }
    }

    @Test
    fun `initialize should add internalDeviceId key in the config string`() {
        val capturedNimbleNetConfig = slot<String>()

        every {
            coreRuntimeInterfaceMocked.initializeNimbleNet(
                any(),
                capture(capturedNimbleNetConfig),
                any(),
                any(),
                any<NimbleNetResult<Unit>>(),
            )
        } just Runs

        nimbleNetController.initialize(nnConfig)

        verify(exactly = 1) {
            coreRuntimeInterfaceMocked.initializeNimbleNet(any(), any(), any(), any(), any())
        }

        assertTrue(capturedNimbleNetConfig.isCaptured, "Captured slot should contain a value")

        assertTrue(
            JSONObject(capturedNimbleNetConfig.captured).has("internalDeviceId"),
            "Config string should contain internalDeviceId",
        )
    }

    @Test
    fun `on init success initialize should call utility functions in order`() {
        val resultSlot = slot<NimbleNetResult<Unit>>()

        every {
            coreRuntimeInterfaceMocked.initializeNimbleNet(
                any(),
                any(),
                any(),
                any(),
                capture(resultSlot),
            )
        } answers { resultSlot.captured.status = true }

        nimbleNetController.initialize(nnConfig)

        verifyOrder {
            fileUtilsMocked.getInternalStorageFolderSizes()
            coreRuntimeInterfaceMocked.initializeNimbleNet(any(), any(), any(), any(), any())
            hardwareInfoMocked.listenToNetworkChanges(any())
            remoteLoggerMocked.writeMetric(METRIC_TYPE.STATIC_DEVICE_METRICS, any())
            remoteLoggerMocked.writeMetric(METRIC_TYPE.INTERNAL_STORAGE_METRIC, any())
        }
    }

    @Test
    fun `on init failure initialize should call utility functions in order`() {
        val resultSlot = slot<NimbleNetResult<Unit>>()

        every {
            coreRuntimeInterfaceMocked.initializeNimbleNet(
                any(),
                any(),
                any(),
                any(),
                capture(resultSlot),
            )
        } answers { resultSlot.captured.status = false }

        nimbleNetController.initialize(nnConfig)

        verifyOrder {
            fileUtilsMocked.getInternalStorageFolderSizes()
            coreRuntimeInterfaceMocked.initializeNimbleNet(any(), any(), any(), any(), any())
        }

        verify(exactly = 0) { hardwareInfoMocked.listenToNetworkChanges(any()) }
        verify(exactly = 0) {
            remoteLoggerMocked.writeMetric(METRIC_TYPE.STATIC_DEVICE_METRICS, any())
        }
        verify(exactly = 0) {
            remoteLoggerMocked.writeMetric(METRIC_TYPE.INTERNAL_STORAGE_METRIC, any())
        }

        assertFalse(nimbleNetController.isNimbleNetInitialized())
    }

    // runMethod()
    @Test
    fun `verify runMethod calls`() {
        val methodName = "testMethod"
        val testInput =
            hashMapOf("key1" to NimbleNetTensor("value1"), "key2" to NimbleNetTensor("value2"))

        every { coreRuntimeInterfaceMocked.runMethod(any(), any(), any()) } returns Unit
        nimbleNetController.runMethod(methodName, testInput)
        verify(exactly = 1) { coreRuntimeInterfaceMocked.runMethod(any(), any(), any()) }
    }

    @Test
    fun `runMethod payload type must be HashMap of String, NimbleNetTensor`() {
        val methodName = "testMethod"
        val testInput =
            hashMapOf("key1" to NimbleNetTensor("value1"), "key2" to NimbleNetTensor("value2"))

        every { coreRuntimeInterfaceMocked.runMethod(any(), any(), any()) } returns Unit

        val result = nimbleNetController.runMethod(methodName, testInput)

        assertTrue(result.payload is HashMap<*, *>)

        verify {
            coreRuntimeInterfaceMocked.runMethod(
                methodName = methodName,
                inputs = testInput,
                nimbleNetResult = any(),
            )
        }
    }

    @Test
    fun `runMethod return type must be NimbleNetResult HashMap String,NimbleNetTensor`() {
        val methodName = "testMethod"
        val testInput =
            hashMapOf("key1" to NimbleNetTensor("value1"), "key2" to NimbleNetTensor("value2"))

        val expectedOutput =
            hashMapOf("key1" to NimbleNetTensor("output1"), "key2" to NimbleNetTensor("output2"))

        every { coreRuntimeInterfaceMocked.runMethod(any(), any(), any()) } answers
            {
                val callback =
                    it.invocation.args[2] as NimbleNetResult<HashMap<String, NimbleNetTensor>>
                callback.payload?.putAll(expectedOutput)
            }

        val result = nimbleNetController.runMethod(methodName, testInput)

        assertTrue(result is NimbleNetResult<*>)
        assertTrue(result.payload is HashMap<*, *>)
        assertEquals(expectedOutput, result.payload)

        verify { coreRuntimeInterfaceMocked.runMethod(methodName, testInput, any()) }
    }

    @Test
    fun `runMethod must verify non-null inputs`() {
        val methodName = "testMethod"
        val testInput =
            hashMapOf("key1" to NimbleNetTensor("value1"), "key2" to NimbleNetTensor("value2"))

        every { coreRuntimeInterfaceMocked.runMethod(any(), any(), any()) } returns Unit

        // used to partially mock the controller
        val spiedController = spyk(nimbleNetController)

        spiedController.runMethod(methodName, testInput)

        verify(exactly = 1) { spiedController.verifyUserInputs(inputs = any()) }
    }

    @Test
    fun `runMethod must measure time taken for the inference to complete and calls writeRunMethodMetric`() {
        val resultSlot = slot<NimbleNetResult<HashMap<String, NimbleNetTensor>>>()

        val methodName = "testMethod"
        val testInput =
            hashMapOf("key1" to NimbleNetTensor("value1"), "key2" to NimbleNetTensor("value2"))

        every { coreRuntimeInterfaceMocked.runMethod(any(), any(), capture(resultSlot)) } answers
            {
                resultSlot.captured.status = true
            }

        // first time returns 5000 then 1000
        every { SystemClock.elapsedRealtimeNanos() } returnsMany listOf(1000L, 5000L)

        val spiedController = spyk(nimbleNetController)

        spiedController.runMethod(methodName, testInput)

        verify(exactly = 1) { coreRuntimeInterfaceMocked.writeRunMethodMetric(any(), 4L) }
    }

    // addEvent
    @Test
    fun `addEvent payload type must be UserEventData`() {
        val eventMapJsonString = "{}"
        val eventId = "event-id"

        every { coreRuntimeInterfaceMocked.addEvent(any(), any(), any()) } returns Unit

        val result = nimbleNetController.addEvent(eventMapJsonString, eventId)

        assertTrue(result.payload is UserEventData)
    }

    @Test
    fun `addEvent must call CoreruntimeInterface Once`() {
        val eventMapJsonString = "{}"
        val eventId = "event-id"

        every { coreRuntimeInterfaceMocked.addEvent(any(), any(), any()) } returns Unit

        nimbleNetController.addEvent(eventMapJsonString, eventId)

        verify(exactly = 1) {
            coreRuntimeInterfaceMocked.addEvent(
                serializedEventMap = eventMapJsonString,
                tableName = eventId,
                nimbleNetResult = any(),
            )
        }
    }

    // isReady
    @Test
    fun `isReady must call CoreruntimeInterface Once`() {
        every { coreRuntimeInterfaceMocked.isReady(any()) } returns Unit

        nimbleNetController.isReady()

        verify(exactly = 1) { coreRuntimeInterfaceMocked.isReady(nimbleNetResult = any()) }
    }

    // RestartSession
    @Test
    fun `restartSession must call CoreruntimeInterface Once`() {
        every { coreRuntimeInterfaceMocked.restartSession(any()) } returns Unit

        nimbleNetController.restartSession("session-id")

        verify(exactly = 1) { coreRuntimeInterfaceMocked.restartSession(any()) }
    }

    // verifyUserInputs
    @Test
    fun `verifyUserInputs must throw ARRAY_SIZE_MISMATCH if array and shape mismatches`() {
        try {
            verifyUserInputsWrapper(intArrayOf(1, 2, 3, 4), intArrayOf(1, 5), DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.ARRAY_SIZE_MISMATCH)
        }

        try {
            verifyUserInputsWrapper(longArrayOf(1L, 2L, 3L, 4L), intArrayOf(1, 5), DATATYPE.INT64)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.ARRAY_SIZE_MISMATCH)
        }

        try {
            verifyUserInputsWrapper(floatArrayOf(1f, 2f, 3f, 4f), intArrayOf(1, 5), DATATYPE.FLOAT)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.ARRAY_SIZE_MISMATCH)
        }

        try {
            verifyUserInputsWrapper(
                doubleArrayOf(1.2, 2.2, 3.2, 4.2),
                intArrayOf(1, 5),
                DATATYPE.DOUBLE,
            )
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.ARRAY_SIZE_MISMATCH)
        }

        try {
            verifyUserInputsWrapper(
                booleanArrayOf(true, false, true, true),
                intArrayOf(1, 5),
                DATATYPE.BOOL,
            )
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.ARRAY_SIZE_MISMATCH)
        }

        try {
            verifyUserInputsWrapper(
                arrayOf("one", "two", "three"),
                intArrayOf(1, 5),
                DATATYPE.STRING,
            )
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.ARRAY_SIZE_MISMATCH)
        }

        try {
            verifyUserInputsWrapper(
                JSONArray(listOf(1, 2, 3, 4)),
                intArrayOf(1, 5),
                DATATYPE.JSON_ARRAY,
            )
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.ARRAY_SIZE_MISMATCH)
        }
    }

    @Test
    fun `verifyUserInputs must throw INVALID_SHAPE_ARRAY if array is passed and shape is null`() {
        try {
            verifyUserInputsWrapper(intArrayOf(1, 2, 3, 4), null, DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_ARRAY)
        }

        try {
            verifyUserInputsWrapper(longArrayOf(1L, 2L, 3L, 4L), null, DATATYPE.INT64)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_ARRAY)
        }

        try {
            verifyUserInputsWrapper(floatArrayOf(1f, 2f, 3f, 4f), null, DATATYPE.FLOAT)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_ARRAY)
        }

        try {
            verifyUserInputsWrapper(doubleArrayOf(1.2, 2.2, 3.2, 4.2), null, DATATYPE.DOUBLE)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_ARRAY)
        }

        try {
            verifyUserInputsWrapper(booleanArrayOf(true, false, true, true), null, DATATYPE.BOOL)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_ARRAY)
        }

        try {
            verifyUserInputsWrapper(arrayOf("one", "two", "three"), null, DATATYPE.STRING)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_ARRAY)
        }

        try {
            verifyUserInputsWrapper(JSONArray(listOf(1, 2, 3, 4)), null, DATATYPE.JSON_ARRAY)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_ARRAY)
        }
    }

    @Test
    fun `verifyUserInputs must throw Unsupported data type if array datatype is unsupported`() {
        try {
            verifyUserInputsWrapper(Array<Byte>(19) { 1 }, null, DATATYPE.FLOAT)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message?.contains("Unsupported array type:") ?: false)
        }
    }

    @Test
    fun `verifyUserInputs must throw Unsupported data type if invalid data is passed as Any`() {
        try {
            verifyUserInputsWrapper(NimbleNet, null, DATATYPE.FLOAT)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message?.contains("Unsupported data type") ?: false)
        }
    }

    @Test
    fun `verifyUserInputs must throw INVALID_SHAPE_SINGULAR if singular input is passed and shape is not null`() {
        try {
            verifyUserInputsWrapper(1, intArrayOf(1), DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_SINGULAR)
        }

        try {
            verifyUserInputsWrapper(1L, intArrayOf(1), DATATYPE.INT64)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_SINGULAR)
        }

        try {
            verifyUserInputsWrapper(1f, intArrayOf(1), DATATYPE.FLOAT)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_SINGULAR)
        }

        try {
            verifyUserInputsWrapper(1.1, intArrayOf(1), DATATYPE.DOUBLE)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_SINGULAR)
        }

        try {
            verifyUserInputsWrapper(false, intArrayOf(1), DATATYPE.BOOL)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_SINGULAR)
        }

        try {
            verifyUserInputsWrapper("one", intArrayOf(1), DATATYPE.STRING)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_SINGULAR)
        }

        try {
            verifyUserInputsWrapper(
                JSONObject(mapOf("input" to "output")),
                intArrayOf(1),
                DATATYPE.JSON,
            )
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message == MESSAGES.INVALID_SHAPE_SINGULAR)
        }
    }

    @Test
    fun `verifyUserInputs must throw INVALID_DATATYPE_SINGULAR if datatype field doesn't match the actual datatype for singular`() {
        try {
            verifyUserInputsWrapper(1, null, DATATYPE.FLOAT)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_SINGULAR))
        }

        try {
            verifyUserInputsWrapper(1L, null, DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_SINGULAR))
        }

        try {
            verifyUserInputsWrapper(1f, null, DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_SINGULAR))
        }

        try {
            verifyUserInputsWrapper(1.1, null, DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_SINGULAR))
        }

        try {
            verifyUserInputsWrapper(true, null, DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_SINGULAR))
        }

        try {
            verifyUserInputsWrapper("test", null, DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_SINGULAR))
        }

        try {
            verifyUserInputsWrapper(JSONObject(mapOf("hello" to "world")), null, DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_SINGULAR))
        }
    }

    @Test
    fun `verifyUserInputs must throw DATATYPE_MISMATCH_ARRAY if datatype field doesn't match the actual datatype for array`() {
        try {
            verifyUserInputsWrapper(intArrayOf(1), intArrayOf(1), DATATYPE.FLOAT)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_ARRAY))
        }

        try {
            verifyUserInputsWrapper(longArrayOf(1), intArrayOf(1), DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_ARRAY))
        }

        try {
            verifyUserInputsWrapper(floatArrayOf(1f), intArrayOf(1), DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_ARRAY))
        }

        try {
            verifyUserInputsWrapper(doubleArrayOf(1.1), intArrayOf(1), DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_ARRAY))
        }

        try {
            verifyUserInputsWrapper(booleanArrayOf(true), intArrayOf(1), DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_ARRAY))
        }

        try {
            verifyUserInputsWrapper(arrayOf("test"), intArrayOf(1), DATATYPE.INT32)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_ARRAY))
        }

        try {
            verifyUserInputsWrapper(
                JSONArray(listOf("hello", "world")),
                intArrayOf(2),
                DATATYPE.INT32,
            )
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains(MESSAGES.DATATYPE_MISMATCH_ARRAY))
        }
    }

    @Test
    fun `verifyUserInputs must throw 'Unsupported data type' if unsupported datatype field is passed`() {
        try {
            verifyUserInputsWrapper(listOf("test"), intArrayOf(1), DATATYPE.BFLOAT16)
            assertTrue(false)
        } catch (e: Exception) {
            assertTrue(e.message!!.contains("Unsupported"))
        }
    }

    // helpers
    private fun verifyUserInputsWrapper(data: Any, shape: IntArray?, datatype: DATATYPE) {
        nimbleNetController.verifyUserInputs(
            hashMapOf("input1" to NimbleNetTensor(datatype = datatype, data = data, shape = shape))
        )
    }
}

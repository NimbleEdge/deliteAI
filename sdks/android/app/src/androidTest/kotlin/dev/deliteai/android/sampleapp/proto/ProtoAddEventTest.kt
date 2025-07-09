/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.android.sampleapp.proto

import dev.deliteai.NimbleNet
import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.impl.delitePy.proto.impl.ProtoObjectWrapper
import dev.deliteai.impl.common.NIMBLENET_VARIANTS
import android.app.Application
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.BeforeClass
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class ProtoAddEventTest {

    companion object {

        @JvmStatic
        @BeforeClass
        fun initializeNimbleNet(): Unit = runBlocking(Dispatchers.Default) {
            val config = NimbleNetConfig(
                clientId = "testclient",
                // IP of host machine(running mock server on port 8080) from inside emulator
                host = "http://10.0.2.2:8080",
                deviceId = "proto-integration-test",
                clientSecret = "dummy",
                debug = true,
                initTimeOutInMs = 20000,
                compatibilityTag = "ADD_EVENT_PROTO",
                libraryVariant = NIMBLENET_VARIANTS.STATIC,
                online = true
            )
            val context =
                InstrumentationRegistry.getInstrumentation().targetContext.applicationContext as Application
            val res = NimbleNet.initialize(context, config)
            if (!res.status) throw RuntimeException("Init failed")

            var time = 0
            while (true) {
                val isReady = NimbleNet.isReady()
                if (isReady.status) break
                if (time > 30) throw RuntimeException("TLE")
                time++
                delay(5000)
            }
        }
    }

    @Test
    fun testAddEventProto() {
        val companyProto = createCompany(2, 3, 1)

        val numEvents = 10
        for (i in 1..numEvents) {
            val status  = NimbleNet.addEvent(ProtoObjectWrapper(companyProto, typeRegistry), "test_event")
            assertTrue("add_event status is false", status.status)
            assertEquals("add_event eventType in payload should be null", status.payload?.eventType, null)
            assertEquals("add_event eventJsonString in payload should be null", status.payload?.eventJsonString, null)
        }

        val expectedRevenueSum = numEvents * companyProto.getDepartments(0).revenue
        val expectedPincodes = List(numEvents) { companyProto.officePincodesList }.flatten().map { it.toLong() }.toLongArray()

        val retVal = NimbleNet.runMethod("run_method", hashMapOf())
        assertTrue("runMethod status is false", retVal.status)
        assertTrue("runMethod payload does not contain companyProcessorOutput", retVal.payload!!.containsKey("companyProcessorOutput"))
        assertTrue("runMethod payload does not contain officePincodes", retVal.payload!!.containsKey("officePincodes"))
        assertTrue("companyProcessorOutput size does not match",
            (retVal.payload?.get("companyProcessorOutput")?.shape as IntArray).contentEquals(intArrayOf(1)))
        assertTrue("companyProcessorOutput payload does not match",
            (retVal.payload?.get("companyProcessorOutput")?.data as FloatArray).contentEquals(floatArrayOf(expectedRevenueSum)))
        assertTrue("officePincodes size does not match",
            (retVal.payload?.get("officePincodes")?.shape as IntArray).contentEquals(intArrayOf(numEvents, companyProto.officePincodesList.size)))
        assertTrue("officePincodes data does not match",
            (retVal.payload?.get("officePincodes")?.data as LongArray).contentEquals(expectedPincodes))
    }
}

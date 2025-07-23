/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.common

import android.app.ActivityManager
import android.app.Application
import android.content.Context
import android.content.pm.PackageManager
import android.net.ConnectivityManager
import android.net.NetworkInfo
import android.os.Build
import android.os.StatFs
import android.system.Os
import android.system.OsConstants
import android.telephony.TelephonyManager
import androidx.core.content.ContextCompat
import dev.deliteai.impl.common.utils.EnvironmentDelegate
import dev.deliteai.impl.loggers.LocalLogger
import dev.deliteai.testUtils.Reflection
import io.mockk.MockKAnnotations
import io.mockk.clearAllMocks
import io.mockk.every
import io.mockk.mockk
import io.mockk.mockkConstructor
import io.mockk.mockkObject
import io.mockk.mockkStatic
import io.mockk.unmockkAll
import java.io.File
import kotlin.test.assertTrue
import org.json.JSONObject
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Before
import org.junit.Test

class HardwareInfoTest {
    private lateinit var hardwareInfo: HardwareInfo
    private var applicationMocked: Application = mockk(relaxed = true)
    private var connectivityManagerMocked: ConnectivityManager = mockk(relaxed = true)
    private var activityManagerMocked: ActivityManager = mockk(relaxed = true)
    private var localLogger: LocalLogger = mockk(relaxed = true)

    @Before
    fun setUp() {
        MockKAnnotations.init(this, relaxUnitFun = true)

        mockkStatic(ContextCompat::class)
        every { applicationMocked.getSystemService(Context.CONNECTIVITY_SERVICE) } returns
            connectivityManagerMocked
        every { applicationMocked.getSystemService(Context.ACTIVITY_SERVICE) } returns
            activityManagerMocked

        mockkStatic(Build::class)
        mockkStatic(Build.VERSION::class)

        hardwareInfo = HardwareInfo(applicationMocked, localLogger)
    }

    @After
    fun tearDown() {
        unmockkAll()
        clearAllMocks()
    }

    @Test
    fun `getNetworkType should return NO_NETWORK_STATE_PERMISSION when permission is denied`() {
        every { ContextCompat.checkSelfPermission(any(), any()) } returns
            PackageManager.PERMISSION_DENIED

        val result: String = Reflection.callPrivateMethod(hardwareInfo, "getNetworkType")
        assertEquals("NO_NETWORK_STATE_PERMISSION", result)
    }

    @Test
    fun `getNetworkType should return NO_INTERNET when there is no active network`() {
        every { ContextCompat.checkSelfPermission(any(), any()) } returns
            PackageManager.PERMISSION_GRANTED

        every { connectivityManagerMocked.activeNetworkInfo } returns null

        val result: String = Reflection.callPrivateMethod(hardwareInfo, "getNetworkType")
        assertEquals("NO_INTERNET", result)
    }

    @Test
    fun `getNetworkType should return WIFI when connected to Wi-Fi`() {
        every { ContextCompat.checkSelfPermission(any(), any()) } returns
            PackageManager.PERMISSION_GRANTED

        val wifiNetworkInfo =
            mockk<NetworkInfo> { every { type } returns ConnectivityManager.TYPE_WIFI }
        every { connectivityManagerMocked.activeNetworkInfo } returns wifiNetworkInfo

        val result: String = Reflection.callPrivateMethod(hardwareInfo, "getNetworkType")

        assertEquals("WIFI", result)
    }

    @Test
    fun `getNetworkType should return ETHERNET when connected to Ethernet`() {
        every { ContextCompat.checkSelfPermission(any(), any()) } returns
            PackageManager.PERMISSION_GRANTED

        val ethernetNetworkInfo =
            mockk<NetworkInfo> { every { type } returns ConnectivityManager.TYPE_ETHERNET }
        every { connectivityManagerMocked.activeNetworkInfo } returns ethernetNetworkInfo

        val result: String = Reflection.callPrivateMethod(hardwareInfo, "getNetworkType")
        assertEquals("ETHERNET", result)
    }

    @Test
    fun `getNetworkType should return 4G when connected to 4G mobile network`() {
        every { ContextCompat.checkSelfPermission(any(), any()) } returns
            PackageManager.PERMISSION_GRANTED

        val mobileNetworkInfo4G =
            mockk<NetworkInfo> {
                every { type } returns ConnectivityManager.TYPE_MOBILE
                every { subtype } returns TelephonyManager.NETWORK_TYPE_LTE
            }
        every { connectivityManagerMocked.activeNetworkInfo } returns mobileNetworkInfo4G

        val result: String = Reflection.callPrivateMethod(hardwareInfo, "getNetworkType")
        assertEquals("4G", result)
    }

    @Test
    fun `getNetworkType should return UNKNOWN when connected to mobile network with unknown subtype`() {
        every { ContextCompat.checkSelfPermission(any(), any()) } returns
            PackageManager.PERMISSION_GRANTED

        val mobileNetworkInfoUnknown =
            mockk<NetworkInfo> {
                every { type } returns ConnectivityManager.TYPE_MOBILE
                every { subtype } returns 9999 // Unknown subtype
            }
        every { connectivityManagerMocked.activeNetworkInfo } returns mobileNetworkInfoUnknown

        val result: String = Reflection.callPrivateMethod(hardwareInfo, "getNetworkType")
        assertEquals("UNKNOWN", result)
    }

    @Test
    fun `getStaticDeviceMetrics should contain all expected keys`() {
        mockkStatic(Os::class)
        mockkObject(EnvironmentDelegate.Companion)
        every { EnvironmentDelegate.getDataDirectory() } returns File("/random/path")
        every { Os.sysconf(OsConstants._SC_NPROCESSORS_CONF) } returns 8L
        every { Os.sysconf(OsConstants._SC_CLK_TCK) } returns 250L

        mockkConstructor(StatFs::class)

        every { anyConstructed<StatFs>().blockSizeLong } returns 4096L
        every { anyConstructed<StatFs>().blockCountLong } returns 1000000L
        every { anyConstructed<StatFs>().availableBlocksLong } returns 500000L

        val expectedKeys =
            listOf(
                "deviceBrand",
                "deviceModel",
                "systemSdkVersion",
                "totalRamInMB",
                "numCores",
                "clockSpeedInHz",
                "networkType",
                "totalInternalStorageInBytes",
                "freeInternalStorageInBytes",
            )

        val result = hardwareInfo.getStaticDeviceMetrics()
        val json = JSONObject(result)

        expectedKeys.forEach { key ->
            assertTrue(json.has(key), "Expected JSON to have the key '$key' but it was missing.")
        }
    }
}

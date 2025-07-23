/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.common

import android.app.Application
import android.content.Context
import android.net.ConnectivityManager
import androidx.test.core.app.ApplicationProvider
import androidx.test.ext.junit.runners.AndroidJUnit4
import dev.deliteai.impl.DependencyContainer
import dev.deliteai.testUtils.nnConfig
import io.mockk.MockKAnnotations
import org.json.JSONObject
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class HardwareInfoAndroidTest {
    private lateinit var connectivityManager: ConnectivityManager
    private lateinit var hardwareInfo: HardwareInfo

    @Before
    fun setup() {
        val context = ApplicationProvider.getApplicationContext<Application>()
        MockKAnnotations.init(this, relaxed = true)
        connectivityManager =
            context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager

        val dependencyContainer =
            DependencyContainer.getInstance(
                application = context.applicationContext as Application,
                nnConfig,
            )
        hardwareInfo = dependencyContainer.getHardwareInfo()
    }

    @Test
    fun getStaticDeviceMetricsShouldReturnValidJson() {
        val result = hardwareInfo.getStaticDeviceMetrics()
        assertNotNull(result)

        val json = JSONObject(result)
        assertTrue(json.has("deviceBrand"))
        assertTrue(json.has("deviceModel"))
        assertTrue(json.has("systemSdkVersion"))
        assertTrue(json.has("totalRamInMB"))
        assertTrue(json.has("numCores"))
        assertTrue(json.has("clockSpeedInHz"))
        assertTrue(json.has("networkType"))
        assertTrue(json.has("totalInternalStorageInBytes"))
        assertTrue(json.has("freeInternalStorageInBytes"))
    }

    @Test
    fun getStaticDeviceMetricsShouldNotContainNullValues() {
        val result = hardwareInfo.getStaticDeviceMetrics()
        assertNotNull(result)

        val json = JSONObject(result)
        json.keys().forEach { key ->
            assertNotNull("Value for key $key should not be null", json.get(key))
        }
    }

    @Test
    fun getInternalStorageInfoShouldReturnValidValues() {
        val storageInfo = hardwareInfo.getStaticDeviceMetrics()
        val json = JSONObject(storageInfo)

        val totalStorage = json.getLong("totalInternalStorageInBytes")
        val freeStorage = json.getLong("freeInternalStorageInBytes")

        assertTrue(totalStorage > 0)
        assertTrue(freeStorage >= 0)
    }

    @Test
    fun getDeviceArchShouldReturnValidArchitecture() {
        val validArchs = setOf("arm64-v8a", "x86_64", "x86", "armeabi-v7a")
        val arch = hardwareInfo.getDeviceArch()
        assertNotNull(arch)
        assertTrue(arch.isNotEmpty())
        assertTrue(validArchs.contains(arch))
    }

    @Test
    fun getInternalDeviceIdShouldReturnValidString() {
        val deviceId = hardwareInfo.getInternalDeviceId()
        assertNotNull(deviceId)
        assertTrue(deviceId.isNotEmpty())
    }
}

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.common

import android.Manifest.permission
import android.annotation.SuppressLint
import android.app.ActivityManager
import android.app.Application
import android.content.Context
import android.content.pm.PackageManager
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import android.os.Build
import android.os.StatFs
import android.provider.Settings
import android.system.Os
import android.system.OsConstants
import android.telephony.TelephonyManager
import androidx.core.content.ContextCompat
import dev.deliteai.impl.common.utils.EnvironmentDelegate
import dev.deliteai.impl.loggers.LocalLogger
import org.json.JSONObject

internal class HardwareInfo(
    private val application: Application,
    private val localLogger: LocalLogger,
) {

    private val activityManager: ActivityManager by lazy {
        application.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
    }

    private val connectivityManager: ConnectivityManager by lazy {
        application.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
    }

    fun getDeviceArch(): String = Build.SUPPORTED_ABIS.firstOrNull().orEmpty()

    @SuppressLint("HardwareIds")
    fun getInternalDeviceId(): String =
        runCatching {
                Settings.Secure.getString(application.contentResolver, Settings.Secure.ANDROID_ID)
            }
            .getOrDefault("null")

    fun getStaticDeviceMetrics(): String =
        runCatching {
                val memInfo =
                    ActivityManager.MemoryInfo().apply { activityManager.getMemoryInfo(this) }

                val info =
                    buildMap<String, Any> {
                        put("deviceBrand", Build.BRAND ?: JSONObject.NULL)
                        put("deviceModel", Build.MODEL ?: JSONObject.NULL)
                        put("systemSdkVersion", "Android${Build.VERSION.SDK_INT}")
                        put("totalRamInMB", memInfo.totalMem / MEGABYTE)
                        put("numCores", Os.sysconf(OsConstants._SC_NPROCESSORS_CONF))
                        put("clockSpeedInHz", Os.sysconf(OsConstants._SC_CLK_TCK))
                        put("networkType", getNetworkType())
                        putAll(getInternalStorageInfo())
                    }

                JSONObject(info).toString()
            }
            .onFailure { localLogger.e(it) }
            .getOrDefault("{}")

    private fun getInternalStorageInfo(): Map<String, Long> {
        val stat = StatFs(EnvironmentDelegate.getDataDirectory().absolutePath)
        return mapOf(
            "totalInternalStorageInBytes" to (stat.blockCountLong * stat.blockSizeLong),
            "freeInternalStorageInBytes" to (stat.availableBlocksLong * stat.blockSizeLong),
        )
    }

    @SuppressLint("MissingPermission")
    fun listenToNetworkChanges(onAvailable: () -> Unit) {
        if (!hasPermission(permission.ACCESS_NETWORK_STATE)) return

        val callback =
            object : ConnectivityManager.NetworkCallback() {
                override fun onAvailable(network: Network) = onAvailable()
            }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            connectivityManager.registerDefaultNetworkCallback(callback)
        } else {
            connectivityManager.registerNetworkCallback(
                NetworkRequest.Builder()
                    .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                    .build(),
                callback,
            )
        }
    }

    private fun getNetworkType(): String {
        if (!hasPermission(permission.ACCESS_NETWORK_STATE)) {
            return "NO_NETWORK_STATE_PERMISSION"
        }

        return connectivityManager.activeNetworkInfo?.let { info ->
            when (info.type) {
                ConnectivityManager.TYPE_WIFI -> "WIFI"
                ConnectivityManager.TYPE_ETHERNET -> "ETHERNET"
                ConnectivityManager.TYPE_MOBILE ->
                    when (info.subtype) {
                        in setOf(
                            TelephonyManager.NETWORK_TYPE_GPRS,
                            TelephonyManager.NETWORK_TYPE_EDGE,
                            TelephonyManager.NETWORK_TYPE_CDMA,
                            TelephonyManager.NETWORK_TYPE_1xRTT,
                            TelephonyManager.NETWORK_TYPE_IDEN,
                        ) -> "2G"
                        in setOf(
                            TelephonyManager.NETWORK_TYPE_UMTS,
                            TelephonyManager.NETWORK_TYPE_EVDO_0,
                            TelephonyManager.NETWORK_TYPE_EVDO_A,
                            TelephonyManager.NETWORK_TYPE_HSDPA,
                            TelephonyManager.NETWORK_TYPE_HSUPA,
                            TelephonyManager.NETWORK_TYPE_HSPA,
                            TelephonyManager.NETWORK_TYPE_EVDO_B,
                            TelephonyManager.NETWORK_TYPE_EHRPD,
                            TelephonyManager.NETWORK_TYPE_HSPAP,
                        ) -> "3G"
                        TelephonyManager.NETWORK_TYPE_LTE -> "4G"
                        TelephonyManager.NETWORK_TYPE_NR -> "5G"
                        else -> "UNKNOWN"
                    }
                else -> "NO_INTERNET"
            }
        } ?: "NO_INTERNET"
    }

    private fun hasPermission(perm: String): Boolean =
        ContextCompat.checkSelfPermission(application, perm) == PackageManager.PERMISSION_GRANTED

    private companion object {
        const val MEGABYTE = 0x100000L
    }
}

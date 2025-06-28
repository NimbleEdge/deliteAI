/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.impl.controllers

import ai.nimbleedge.datamodels.NimbleNetConfig
import ai.nimbleedge.datamodels.NimbleNetResult
import ai.nimbleedge.datamodels.NimbleNetTensor
import ai.nimbleedge.datamodels.UserEventData
import ai.nimbleedge.impl.common.DATATYPE
import ai.nimbleedge.impl.common.HardwareInfo
import ai.nimbleedge.impl.common.MESSAGES
import ai.nimbleedge.impl.common.METRIC_TYPE
import ai.nimbleedge.impl.coroutine.NimbleEdgeScope
import ai.nimbleedge.impl.delitePy.proto.ProtoMemberExtender
import ai.nimbleedge.impl.delitePy.proto.impl.ProtoObjectWrapper
import ai.nimbleedge.impl.io.FileUtils
import ai.nimbleedge.impl.loggers.RemoteLogger
import ai.nimbleedge.impl.moduleInstallers.ModuleInstaller
import ai.nimbleedge.impl.nativeBridge.CoreRuntime
import android.app.Application
import android.os.SystemClock
import androidx.annotation.VisibleForTesting
import java.util.concurrent.atomic.AtomicBoolean
import kotlinx.coroutines.runBlocking
import org.json.JSONArray
import org.json.JSONObject

internal class NimbleNetController(
    private val application: Application,
    private val nimbleEdgeScope: NimbleEdgeScope,
    private val fileUtils: FileUtils,
    private val hardwareInfo: HardwareInfo,
    private val moduleInstaller: ModuleInstaller,
    private val coreRuntime: CoreRuntime,
    private val remoteLogger: RemoteLogger,
) {
    private val isNimbleNetInitialized = AtomicBoolean()

    fun isNimbleNetInitialized() = isNimbleNetInitialized.get()

    @Synchronized
    fun initialize(config: NimbleNetConfig): NimbleNetResult<Unit> =
        runBlocking(nimbleEdgeScope.secondary.coroutineContext) {
            val result = NimbleNetResult<Unit>(payload = null)
            val storageInfo = fileUtils.getInternalStorageFolderSizes()

            config.setInternalDeviceId(hardwareInfo.getInternalDeviceId())
            moduleInstaller.execute()

            coreRuntime.initializeNimbleNet(
                application,
                config.toString(),
                fileUtils.getSDKDirPath(),
                result,
            )

            if (result.status) {
                handleInitSuccess(storageInfo)
            }

            result
        }

    fun runMethod(
        methodName: String,
        inputs: HashMap<String, NimbleNetTensor>?,
    ): NimbleNetResult<HashMap<String, NimbleNetTensor>> =
        runBlocking(nimbleEdgeScope.primary.coroutineContext) {
            val startTime = SystemClock.elapsedRealtimeNanos()
            inputs?.let { verifyUserInputs(it) }

            val result = NimbleNetResult<HashMap<String, NimbleNetTensor>>(payload = hashMapOf())

            coreRuntime.runMethod(
                methodName = methodName,
                inputs = inputs as Any,
                nimbleNetResult = result,
            )

            if (result.status) {
                buildProtoObjects(result)
                val durationMicros = (SystemClock.elapsedRealtimeNanos() - startTime) / 1_000
                writeRunMethodMetricToCore(methodName, durationMicros)
            }

            result
        }

    fun addEvent(eventMapJson: String, tableName: String): NimbleNetResult<UserEventData> =
        runBlocking(nimbleEdgeScope.secondary.coroutineContext) {
            val result = NimbleNetResult(payload = UserEventData())
            coreRuntime.addEvent(eventMapJson, tableName, nimbleNetResult = result)
            result
        }

    fun addEvent(
        protoEvent: ProtoObjectWrapper,
        eventType: String,
    ): NimbleNetResult<UserEventData> =
        runBlocking(nimbleEdgeScope.secondary.coroutineContext) {
            val result = NimbleNetResult(payload = UserEventData())
            coreRuntime.addEventProto(protoEvent, eventType, result)
            result
        }

    fun isReady(): NimbleNetResult<Unit> =
        runBlocking(nimbleEdgeScope.secondary.coroutineContext) {
            val result = NimbleNetResult<Unit>(payload = null)
            coreRuntime.isReady(result)
            result
        }

    fun restartSession(sessionId: String) =
        runBlocking(nimbleEdgeScope.secondary.coroutineContext) {
            coreRuntime.restartSession(sessionId)
        }

    // private functions
    @VisibleForTesting
    fun verifyUserInputs(inputs: HashMap<String, NimbleNetTensor>) {
        inputs.forEach { (_, nimblenetTensor) ->
            val data = nimblenetTensor.data!!
            val shape = nimblenetTensor.shape

            fun validateArray(expectedType: DATATYPE) {
                if (nimblenetTensor.datatype != expectedType)
                    throw Exception("${MESSAGES.DATATYPE_MISMATCH_ARRAY} $expectedType")
                if (shape == null || shape.isEmpty()) throw Exception(MESSAGES.INVALID_SHAPE_ARRAY)

                var expectedSize = 1

                shape.forEach { expectedSize *= it }

                val actualSize =
                    when (data) {
                        is IntArray -> data.size
                        is LongArray -> data.size
                        is FloatArray -> data.size
                        is DoubleArray -> data.size
                        is ByteArray -> data.size
                        is BooleanArray -> data.size
                        is Array<*> -> data.size
                        is JSONArray -> data.length()
                        else -> throw Exception("Unsupported data type: ${data::class.simpleName}")
                    }

                if (actualSize != expectedSize) throw Exception(MESSAGES.ARRAY_SIZE_MISMATCH)
            }

            fun validateSingular(expectedType: DATATYPE) {
                if (nimblenetTensor.datatype != expectedType)
                    throw Exception("${MESSAGES.DATATYPE_MISMATCH_SINGULAR} $expectedType")
                if (shape != null && shape.isNotEmpty())
                    throw Exception(MESSAGES.INVALID_SHAPE_SINGULAR)
            }

            fun isDerivedFromProtoMemberExtender(obj: Any): Boolean {
                return try {
                    val clazz =
                        Class.forName("ai.nimbleedge.impl.scriptWrappers.proto.ProtoMemberExtender")
                    clazz.isAssignableFrom(obj::class.java)
                } catch (e: ClassNotFoundException) {
                    false
                }
            }
            if (nimblenetTensor.datatype == DATATYPE.FE_OBJ) {
                if (isDerivedFromProtoMemberExtender(data)) {
                    return
                } else {
                    throw Exception("Unsupported data type: ${data::class.simpleName}")
                }
            }
            when (data) {
                is IntArray -> validateArray(DATATYPE.INT32)
                is LongArray -> validateArray(DATATYPE.INT64)
                is FloatArray -> validateArray(DATATYPE.FLOAT)
                is DoubleArray -> validateArray(DATATYPE.DOUBLE)
                is BooleanArray -> validateArray(DATATYPE.BOOL)
                is Array<*> -> {
                    if (data.isArrayOf<String>()) {
                        validateArray(DATATYPE.STRING)
                    } else {
                        throw Exception("Unsupported array type: ${data::class.simpleName}")
                    }
                }

                is JSONArray -> validateArray(DATATYPE.JSON_ARRAY)

                is Int -> validateSingular(DATATYPE.INT32)
                is Long -> validateSingular(DATATYPE.INT64)
                is Float -> validateSingular(DATATYPE.FLOAT)
                is Double -> validateSingular(DATATYPE.DOUBLE)
                is Boolean -> validateSingular(DATATYPE.BOOL)
                is String -> validateSingular(DATATYPE.STRING)
                is JSONObject -> validateSingular(DATATYPE.JSON)

                // We can't validate the function's in and out types here.
                is Function1<*, *> -> validateSingular(DATATYPE.FUNCTION)

                else -> throw Exception("Unsupported data type: ${data::class.simpleName}")
            }
        }
    }

    // Builds proto objects if present in result payload
    private fun buildProtoObjects(
        nimbleNetResult: NimbleNetResult<HashMap<String, NimbleNetTensor>>
    ) {
        nimbleNetResult.payload?.values?.forEach { value ->
            (value.data as? ProtoMemberExtender)?.build()
        }
    }

    private fun handleInitSuccess(internalStorageInfo: String?) {
        hardwareInfo.listenToNetworkChanges { coreRuntime.networkConnectionEstablishedCallback() }

        writeStaticDeviceMetricToCore()
        writeInternalStorageSizeMetricToCore(internalStorageInfo)
        isNimbleNetInitialized.set(true)
    }

    private fun writeStaticDeviceMetricToCore() {
        remoteLogger.writeMetric(
            METRIC_TYPE.STATIC_DEVICE_METRICS,
            hardwareInfo.getStaticDeviceMetrics(),
        )
    }

    private fun writeInternalStorageSizeMetricToCore(internalStorageInfo: String?) {
        if (internalStorageInfo != null) {
            remoteLogger.writeMetric(METRIC_TYPE.INTERNAL_STORAGE_METRIC, internalStorageInfo)
        }
    }

    private fun writeRunMethodMetricToCore(id: String, totalTimeInUSecs: Long) {
        coreRuntime.writeRunMethodMetric(id, totalTimeInUSecs)
    }
}

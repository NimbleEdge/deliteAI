/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.common

internal object SDK_CONSTANTS {
    const val LOG_TAG = "NIMBLE-SDK"
    const val NIMBLE_NET_LIB = "nimblenet"
    const val NE_DYNAMIC_MODULE_NAME = "nimblenet_dynamic"
    const val INIT_DEFAULT_TIMEOUT = 50000.toLong()
    const val NIMBLE_SDK_FOLDER_NAME = "nimbleSDK"
    const val NIMBLE_SDK_LIBS_FOLDER_NAME = "nimbleSDKLibs"
    const val NUM_THREADS_FOR_PRIMARY_COROUTINE_SCOPE = 1
    const val NUM_THREADS_FOR_SECONDARY_COROUTINE_SCOPE = 1
    const val DELITE_ASSETS_TEMP_STORAGE = "deliteAssets"
    const val DELITE_ASSETS_TEMP_FILES_EXPIRY_IN_MILLIS = 7 * 24 * 60 * 60 * 1000L
}

internal object STATUS_CODES {
    const val EMPTY_NETWORK_RESPONSE = 900
    const val FILE_DOWNLOAD_RUNNING = 10001
    const val FILE_DOWNLOAD_SUCCESS = 10002
    const val FILE_DOWNLOAD_FAILURE = 10003
    const val FILE_DOWNLOAD_PAUSED = 10004
    const val FILE_DOWNLOAD_PENDING = 10005
    const val FILE_DOWNLOAD_UNKNOWN = 10006
}

internal object ERROR_CODES {
    const val KOTLIN_ERROR = 7000
    const val DM_UNABLE_TO_CREATE_REQUEST = 7001
}

internal object MESSAGES {
    const val FAILED_TO_GET_MODEL_STATUS_MSG = "Failed to retrieve the model status."
    const val NE_INIT_FAILED_EXCEPTION = "NimbleNet initialization failed due to an exception."
    const val NE_ADD_EVENT_FAILED_EXCEPTION =
        "Failed to add event to NimbleNet due to an exception."
    const val DEPRECATED_API_MESSAGE =
        "This function is deprecated and may be removed in future versions."
    const val SDK_NOT_INITIALIZED =
        "The SDK has not been initialized. Please initialize it before use."
    const val DATATYPE_MISMATCH_ARRAY =
        "Data type mismatch: The provided array's type does not match the expected type: "
    const val DATATYPE_MISMATCH_SINGULAR =
        "Data type mismatch: The provided value's type does not match the expected type: "
    const val INVALID_SHAPE_ARRAY =
        "Invalid shape: The shape cannot be null or empty for an array input."
    const val INVALID_SHAPE_SINGULAR =
        "Invalid shape: The shape must be null or empty for a singular input."
    const val ARRAY_SIZE_MISMATCH =
        "Array size mismatch: The actual array size does not match the specified shape."
}

internal object SHARED_PREFERENCES {
    const val NAME = "ne_sp_pref"
    const val GDL_DOWNLOAD_START_TIME = "ne_sp_gdlDownloadStartTime"
}

internal object WORK_MANAGER {
    const val WM_LOGS_UPLOAD_JOB_ID = "wmNimbleSDKLogsUploadId"
    const val WM_LOGS_UPLOAD_PAYLOAD_ID = "payload"
}

internal object METRIC_TYPE {
    const val INFERENCE_METRIC = "inference"
    const val STATIC_DEVICE_METRICS = "staticDevice"
    const val DL_LIBS_DOWNLOAD = "libDownload"
    const val DL_LIBS_DOWNLOAD_STARTED = "libDownloadStarted"
    const val INTERNAL_STORAGE_METRIC = "internalStorage"
}

// The Int values here must stay in sync with `enum DATATYPE`
// in `coreruntime/nimblenet/CrossPlatform/include/nimblenetUtil.h`.
enum class DATATYPE(val value: Int) {
    UNDEFINED(0),
    FLOAT(1),
    UINT16(4),
    INT16(5),
    INT32(6),
    INT64(7),
    STRING(8),
    BOOL(9),
    FLOAT16(10),
    DOUBLE(11),
    UINT32(12),
    UINT64(13),
    COMPLEX64(14),
    COMPLEX128(15),
    BFLOAT16(16),
    JSON(670),
    JSON_ARRAY(681),
    FUNCTION(682),
    FE_OBJ(700),
    NONE(667);

    companion object {
        private val map = values().associateBy(DATATYPE::value)

        fun fromValue(value: Any?): Int {
            return when (value) {
                null -> NONE.value
                is Float -> FLOAT.value
                is Double -> DOUBLE.value
                is Short -> INT16.value
                is Int -> INT32.value
                is Long -> INT64.value
                is UShort -> UINT16.value
                is UInt -> UINT32.value
                is ULong -> UINT64.value
                is String -> STRING.value
                is Boolean -> BOOL.value
                is List<*> -> JSON_ARRAY.value
                is Function1<*, *> -> FUNCTION.value
                is Map<*, *> -> JSON.value
                is Any -> FE_OBJ.value
                else -> UNDEFINED.value
            }
        }

        fun fromInt(typeValue: Int): DATATYPE {
            return map[typeValue] ?: UNDEFINED
        }
    }
}

enum class PROTO_DATATYPE(val value: Int) {
    UNDEFINED(0),
    PROTO_LIST(701),
    PROTO_MAP(702),
    PROTO_OBJECT(703);

    companion object {
        fun fromValue(value: Any?): Int {
            return when (value) {
                is List<*> -> PROTO_LIST.value
                is Map<*, *> -> PROTO_MAP.value
                is Any -> PROTO_OBJECT.value
                else -> UNDEFINED.value
            }
        }
    }
}

enum class NIMBLENET_VARIANTS {
    STATIC,
    GOOGLE_PLAY_FEATURE_DYNAMIC,
}

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.datamodels

import dev.deliteai.impl.common.ERROR_CODES

/**
 * Represents error information for failed NimbleNet operations.
 *
 * This class encapsulates error details returned by the NimbleNet SDK when operations fail. It
 * provides both a numeric error code for programmatic handling and a human-readable message for
 * debugging and logging purposes.
 *
 * ## Usage Example
 *
 * ```kotlin
 * val result = NimbleNet.runMethod("my_model", inputs)
 * if (!result.status) {
 *     val error = result.error
 *     Log.e("NimbleNet", "Operation failed: ${error?.message}")
 * }
 * ```
 *
 * @param code Numeric error code identifying the specific type of error. Defaults to
 *   [ERROR_CODES.KOTLIN_ERROR] if not specified.
 * @param message Human-readable description of the error. May be empty if not specified.
 * @see ERROR_CODES
 * @see NimbleNetResult
 * @since 1.0.0
 */
class NimbleNetError(var code: Int = ERROR_CODES.KOTLIN_ERROR, var message: String = "") {
    private fun populateErrorObject(code: Int, message: String) {
        this.code = code
        this.message = message
    }

    /**
     * Returns a formatted string representation of this error.
     *
     * @return Formatted string containing error code and message
     * @since 1.0.0
     */
    override fun toString(): String {
        return "code: $code \n message: $message"
    }
}

/**
 * Generic result wrapper for all NimbleNet SDK operations.
 *
 * This class provides a consistent way to handle both successful and failed operations across the
 * entire NimbleNet SDK. It encapsulates success status, payload data, and error information in a
 * single object.
 *
 * ## Result States
 *
 * ### Success State
 * - `status = true`
 * - `payload` contains the actual result data
 * - `error = null`
 *
 * ### Failure State
 * - `status = false`
 * - `payload = null`
 * - `error` contains detailed failure information
 *
 * ## Usage Example
 *
 * ```kotlin
 * val result = NimbleNet.runMethod("classifier", inputs)
 * if (result.status) {
 *     // Success - use result.payload
 *     val outputs = result.payload
 *     processResults(outputs)
 * } else {
 *     // Failure - check result.error
 *     Log.e("ML", "Inference failed: ${result.error?.message}")
 * }
 * ```
 *
 * @param T The type of data contained in the payload when the operation succeeds
 * @param status Boolean indicating whether the operation succeeded (true) or failed (false).
 * @param payload The actual result data when the operation succeeds. Will be null for failed
 *   operations.
 * @param error Detailed error information when the operation fails. Will be null for successful
 *   operations.
 * @see NimbleNetError
 * @since 1.0.0
 */
class NimbleNetResult<T>(
    var status: Boolean = false,
    var payload: T?,
    var error: NimbleNetError? = NimbleNetError(),
) {
    /**
     * Returns a formatted string representation of this result.
     *
     * @return Formatted string containing result status, error, and payload
     * @since 1.0.0
     */
    override fun toString(): String {
        return "status: $status \nerror: $error \npayload: $payload"
    }
}

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.loggers

import android.util.Log
import dev.deliteai.impl.common.SDK_CONSTANTS

internal class LocalLogger {
    fun d(msg: String) {
        Log.d(SDK_CONSTANTS.LOG_TAG, msg)
    }

    fun e(msg: String) {
        Log.e(SDK_CONSTANTS.LOG_TAG, msg)
    }

    fun e(t: Throwable) {
        Log.e(SDK_CONSTANTS.LOG_TAG, t.message.toString())
    }

    fun i(msg: String) {
        Log.i(SDK_CONSTANTS.LOG_TAG, msg)
    }
}

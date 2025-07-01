/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.common.utils

internal object TypeCaster {
    @JvmStatic
    fun castToInt(value: Any): Int {
        return value as Int
    }

    @JvmStatic
    fun castToDouble(value: Any): Double {
        return value as Double
    }

    @JvmStatic
    fun castToFloat(value: Any): Float {
        return value as Float
    }

    @JvmStatic
    fun castToBoolean(value: Any): Boolean {
        return value as Boolean
    }

    @JvmStatic
    fun castToLong(value: Any): Long {
        return value as Long
    }

    @JvmStatic
    fun castToByte(value: Any): Byte {
        return value as Byte
    }

    @JvmStatic
    fun castToString(value: Any): String {
        return value as String
    }
}

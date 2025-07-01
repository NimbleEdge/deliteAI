/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.testUtils

import kotlin.reflect.full.declaredFunctions
import kotlin.reflect.jvm.isAccessible

object Reflection {
    inline fun <reified T : Any, reified R> callPrivateMethod(instance: T, methodName: String): R {
        val kfun =
            T::class.declaredFunctions.first { it.name == methodName }.apply { isAccessible = true }
        @Suppress("UNCHECKED_CAST")
        return kfun.call(instance) as R
    }
}

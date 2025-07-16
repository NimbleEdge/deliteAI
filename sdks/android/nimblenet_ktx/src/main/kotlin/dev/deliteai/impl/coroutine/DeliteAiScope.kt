/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.coroutine

import androidx.annotation.VisibleForTesting
import dev.deliteai.impl.common.SDK_CONSTANTS
import java.util.concurrent.Executors
import kotlin.coroutines.CoroutineContext
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.asCoroutineDispatcher

internal class DeliteAiScope {
    val primary =
        getScope("primaryDeliteAiScope", SDK_CONSTANTS.NUM_THREADS_FOR_PRIMARY_COROUTINE_SCOPE)
    val secondary =
        getScope("secondaryDeliteAiScope", SDK_CONSTANTS.NUM_THREADS_FOR_SECONDARY_COROUTINE_SCOPE)

    @VisibleForTesting(otherwise = VisibleForTesting.PRIVATE)
    fun getScope(name: String, numThreads: Int): CoroutineScope {
        val context: CoroutineContext =
            Executors.newFixedThreadPool(numThreads, NamedThreadFactory(name))
                .asCoroutineDispatcher()
        return CoroutineScope(context)
    }
}

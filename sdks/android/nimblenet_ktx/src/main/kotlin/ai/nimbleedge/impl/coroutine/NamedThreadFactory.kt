/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.impl.coroutine

import java.util.concurrent.ThreadFactory
import java.util.concurrent.atomic.AtomicInteger

internal class NamedThreadFactory(private val name: String) : ThreadFactory {
    private val threadNumber = AtomicInteger(1)

    override fun newThread(runnable: Runnable): Thread {
        return Thread(runnable, "$name-${threadNumber.getAndIncrement()}")
    }
}

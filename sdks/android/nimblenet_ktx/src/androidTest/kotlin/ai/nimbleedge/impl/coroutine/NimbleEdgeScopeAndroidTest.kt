/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.impl.coroutine

import ai.nimbleedge.impl.common.SDK_CONSTANTS
import androidx.test.ext.junit.runners.AndroidJUnit4
import java.util.concurrent.ThreadPoolExecutor
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.ExecutorCoroutineDispatcher
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class NimbleEdgeScopeAndroidTest {

    @OptIn(ExperimentalStdlibApi::class)
    private fun extractThreadPool(scope: CoroutineScope): ThreadPoolExecutor {
        val dispatcher =
            scope.coroutineContext[ExecutorCoroutineDispatcher.Key] as ExecutorCoroutineDispatcher
        return dispatcher.executor as ThreadPoolExecutor
    }

    @Test
    fun testPrimaryAndSecondaryScopeInitialization() = runTest {
        val scope = NimbleEdgeScope()
        assertNotNull(scope.primary)
        assertNotNull(scope.secondary)
    }

    @Test
    fun testThreadPoolSizeForPrimaryScope() = runTest {
        val scope = NimbleEdgeScope()
        val expectedThreads = SDK_CONSTANTS.NUM_THREADS_FOR_PRIMARY_COROUTINE_SCOPE
        val executor = extractThreadPool(scope.primary)
        assertEquals(expectedThreads, executor.corePoolSize)
    }

    @Test
    fun testThreadPoolSizeForSecondaryScope() = runTest {
        val scope = NimbleEdgeScope()
        val expectedThreads = SDK_CONSTANTS.NUM_THREADS_FOR_SECONDARY_COROUTINE_SCOPE
        val executor = extractThreadPool(scope.secondary)
        assertEquals(expectedThreads, executor.corePoolSize)
    }
}

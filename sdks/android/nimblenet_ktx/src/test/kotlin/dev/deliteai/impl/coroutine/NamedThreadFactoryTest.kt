/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.coroutine

import junit.framework.TestCase.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class NamedThreadFactoryTest {

    @Test
    fun `newThread should create thread with correct name and number`() {
        val factory = NamedThreadFactory("TestThread")
        val runnable = Runnable {}
        val thread = factory.newThread(runnable)
        assertEquals("TestThread-1", thread.name)
    }

    @Test
    fun `newThread should increment thread number correctly`() {
        val factory = NamedThreadFactory("Worker")
        val runnable = Runnable {}
        val thread1 = factory.newThread(runnable)
        val thread2 = factory.newThread(runnable)
        val thread3 = factory.newThread(runnable)
        assertEquals("Worker-1", thread1.name)
        assertEquals("Worker-2", thread2.name)
        assertEquals("Worker-3", thread3.name)
    }

    @Test
    fun `newThread should handle multiple factories independently`() {
        val factoryA = NamedThreadFactory("FactoryA")
        val factoryB = NamedThreadFactory("FactoryB")
        val runnable = Runnable {}
        val threadA1 = factoryA.newThread(runnable)
        val threadA2 = factoryA.newThread(runnable)
        val threadB1 = factoryB.newThread(runnable)
        assertEquals("FactoryA-1", threadA1.name)
        assertEquals("FactoryA-2", threadA2.name)
        assertEquals("FactoryB-1", threadB1.name)
    }

    @Test
    fun `newThread should create thread with provided runnable`() {
        val factory = NamedThreadFactory("RunnableThread")
        var executed = false
        val runnable = Runnable { executed = true }
        val thread = factory.newThread(runnable)
        thread.run()
        assertTrue(executed)
    }
}

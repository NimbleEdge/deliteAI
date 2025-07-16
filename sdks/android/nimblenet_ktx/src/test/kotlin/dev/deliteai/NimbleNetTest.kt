/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai

import android.app.Application
import dev.deliteai.impl.DependencyContainer
import dev.deliteai.impl.common.MESSAGES
import dev.deliteai.impl.controllers.NimbleNetController
import dev.deliteai.impl.delitePy.proto.impl.ProtoObjectWrapper
import dev.deliteai.impl.loggers.LocalLogger
import dev.deliteai.testUtils.nnConfig
import io.mockk.MockKAnnotations
import io.mockk.clearMocks
import io.mockk.every
import io.mockk.mockk
import io.mockk.mockkObject
import io.mockk.unmockkAll
import io.mockk.verify
import org.junit.After
import org.junit.Assert.assertTrue
import org.junit.Assert.fail
import org.junit.Before
import org.junit.Test

class NimbleNetTest {
    private val applicationMocked: Application = mockk(relaxed = true)
    private val nimbleNetControllerMocked: NimbleNetController = mockk(relaxed = true)
    private val localLoggerMocked: LocalLogger = mockk(relaxed = true)

    @Before
    fun `setup mocks and instance`() {
        MockKAnnotations.init(this)

        every { localLoggerMocked.e(any<Throwable>()) } returns Unit

        val dependencyContainerMocked: DependencyContainer = mockk(relaxed = true)
        mockkObject(DependencyContainer.Companion)
        every { DependencyContainer.getInstance(any(), any()) } returns dependencyContainerMocked
        every { dependencyContainerMocked.getNimbleNetController() } returns
            nimbleNetControllerMocked
        every { dependencyContainerMocked.getLocalLogger() } returns localLoggerMocked
    }

    @After
    fun `clean up mocks`() {
        unmockkAll()
        clearMocks(applicationMocked, nimbleNetControllerMocked, localLoggerMocked)
    }

    @Test
    fun `every function must fail gracefully if SDK has not been initialised`() {
        every { nimbleNetControllerMocked.isNimbleNetInitialized() } returns false

        val res2 = NimbleNet.addEvent(mapOf(), "")
        if (res2.error?.message != MESSAGES.SDK_NOT_INITIALIZED) fail()

        val res3 = NimbleNet.addEvent("", "")
        if (res3.error?.message != MESSAGES.SDK_NOT_INITIALIZED) fail()

        val res4 = NimbleNet.runMethod("", null)
        if (res4.error?.message != MESSAGES.SDK_NOT_INITIALIZED) fail()

        val res5 = NimbleNet.isReady()
        if (res5.error?.message != MESSAGES.SDK_NOT_INITIALIZED) fail()

        NimbleNet.restartSession()
        verify(exactly = 0) { nimbleNetControllerMocked.restartSession(any()) }

        NimbleNet.restartSessionWithId("")
        verify(exactly = 0) { nimbleNetControllerMocked.restartSession(any()) }
    }

    @Test
    fun `every function should fail gracefully in case of an exception from the internal layer`() {
        every { nimbleNetControllerMocked.isNimbleNetInitialized() } returns true
        every { nimbleNetControllerMocked.initialize(any()) } throws Exception("TEST_EXCEPTION")
        every { nimbleNetControllerMocked.addEvent(any() as String, any()) } throws
            Exception("TEST_EXCEPTION")
        every { nimbleNetControllerMocked.addEvent(any() as ProtoObjectWrapper, any()) } throws
            Exception("TEST_EXCEPTION")
        every { nimbleNetControllerMocked.runMethod(any(), any()) } throws
            Exception("TEST_EXCEPTION")
        every { nimbleNetControllerMocked.isReady() } throws Exception("TEST_EXCEPTION")
        every { nimbleNetControllerMocked.restartSession(any()) } throws Exception("TEST_EXCEPTION")

        val res1 = NimbleNet.initialize(applicationMocked, nnConfig)
        assertTrue(res1.error?.message == "TEST_EXCEPTION")

        val res2 = NimbleNet.initialize(applicationMocked, nnConfig)
        assertTrue(res2.error?.message == "TEST_EXCEPTION")

        val res3 = NimbleNet.addEvent(mapOf(), "")
        assertTrue(res3.error?.message == "TEST_EXCEPTION")

        val res4 = NimbleNet.addEvent("", "")
        assertTrue(res4.error?.message == "TEST_EXCEPTION")

        val res5 = NimbleNet.runMethod("", hashMapOf())
        assertTrue(res5.error?.message == "TEST_EXCEPTION")

        val res6 = NimbleNet.isReady()
        assertTrue(res6.error?.message == "TEST_EXCEPTION")

        NimbleNet.restartSession()
        NimbleNet.restartSessionWithId("")
    }
}

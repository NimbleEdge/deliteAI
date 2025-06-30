/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.impl.moduleInstallers.impl

import ai.nimbleedge.impl.common.SDK_CONSTANTS
import ai.nimbleedge.impl.loggers.LocalLogger
import ai.nimbleedge.impl.moduleInstallers.ModuleInstaller

internal class StaticModuleInstaller(private val localLogger: LocalLogger) : ModuleInstaller {
    override suspend fun execute() {
        localLogger.d("STATIC LOADING TRIGGERED!!!")

        try {
            System.loadLibrary(SDK_CONSTANTS.NIMBLE_NET_LIB)
            localLogger.d("${SDK_CONSTANTS.NIMBLE_NET_LIB} loaded successfully")
        } catch (e: Exception) {
            throw e
        }
    }
}

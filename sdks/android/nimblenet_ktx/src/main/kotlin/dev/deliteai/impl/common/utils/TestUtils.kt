/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.impl.common.utils

import android.os.Environment
import java.io.File

internal class EnvironmentDelegate {
    companion object {
        fun getDataDirectory(): File {
            return Environment.getDataDirectory()
        }
    }
}

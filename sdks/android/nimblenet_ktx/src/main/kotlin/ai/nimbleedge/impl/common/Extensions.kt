/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.impl.common

import ai.nimbleedge.datamodels.NimbleNetError
import ai.nimbleedge.datamodels.NimbleNetResult

internal fun <T> Throwable.toNimbleNetResult(): NimbleNetResult<T> {
    val error = NimbleNetError(code = -1, message = this.message ?: "")
    return NimbleNetResult(status = false, payload = null, error = error)
}

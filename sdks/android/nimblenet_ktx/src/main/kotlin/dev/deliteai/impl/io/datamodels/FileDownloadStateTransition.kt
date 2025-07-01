/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.io.datamodels

internal data class FileDownloadStateTransition(
    val downloadManagerDownloadId: Long,
    val currentState: Int,
    val previousState: Int,
    val timeTaken: Long,
    val currentStateReasonCode: Int = -1,
)

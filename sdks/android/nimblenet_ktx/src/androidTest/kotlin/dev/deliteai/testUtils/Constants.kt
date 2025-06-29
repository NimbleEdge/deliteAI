/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.deliteai.testUtils

import ai.deliteai.datamodels.NimbleNetConfig
import ai.deliteai.impl.common.NIMBLENET_VARIANTS

val nnConfig =
    NimbleNetConfig(
        clientId = "test",
        host = "test",
        deviceId = "test",
        clientSecret = "test",
        debug = true,
        initTimeOutInMs = 1000000000,
        compatibilityTag = "test",
        libraryVariant = NIMBLENET_VARIANTS.STATIC,
        showDownloadProgress = true,
    )

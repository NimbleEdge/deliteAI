/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.testUtils

import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.impl.common.NIMBLENET_VARIANTS

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
        online = true,
    )

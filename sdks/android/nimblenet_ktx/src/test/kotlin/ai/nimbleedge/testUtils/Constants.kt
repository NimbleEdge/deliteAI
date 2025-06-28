/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.testUtils

import ai.nimbleedge.datamodels.NimbleNetConfig
import ai.nimbleedge.impl.common.NIMBLENET_VARIANTS

val nnConfig =
    NimbleNetConfig(
        clientId = "testclient",
        host = "https://apiv3.nimbleedge-staging.com",
        deviceId = "nimon_rulezzz",
        clientSecret = "samplekey123",
        debug = true,
        initTimeOutInMs = 1000000000,
        compatibilityTag = "android-output-verification",
        libraryVariant = NIMBLENET_VARIANTS.STATIC,
    )

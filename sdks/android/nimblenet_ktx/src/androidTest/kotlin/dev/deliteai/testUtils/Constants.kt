/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.testUtils

import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.impl.common.NIMBLENET_VARIANTS
import dev.deliteai.nimblenet_ktx.BuildConfig

val nnConfig =
    NimbleNetConfig(
        clientId = BuildConfig.ANDROID_TEST_CLIENT_ID,
        host = BuildConfig.ANDROID_TEST_HOST,
        deviceId = "android-test",
        clientSecret = BuildConfig.ANDROID_TEST_CLIENT_SECRET,
        debug = true,
        initTimeOutInMs = 20000,
        compatibilityTag = "android-output-verification",
        libraryVariant = NIMBLENET_VARIANTS.STATIC,
        online = true,
    )

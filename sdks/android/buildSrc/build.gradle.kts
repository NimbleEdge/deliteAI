/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

plugins {
    `kotlin-dsl`
}

repositories {
    mavenCentral()
    google()
}

dependencies {
    //TODO:(naman) any better solution for this???
    //need to hardcode this because we need this to compile BuildSrc
    //depending upon the BuildSrc classes will cause circular dep
    implementation("com.android.tools.build:gradle:8.7.3")
}

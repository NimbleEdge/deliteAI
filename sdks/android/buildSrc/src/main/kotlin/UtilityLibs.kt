/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

object UtilityLibs {
    const val kotlinVersion = "1.9.22"

    private const val kotlinGroup = "org.jetbrains.kotlin"

    private const val kotlinxGroup = "org.jetbrains.kotlinx"

    private const val kmpCoroutinesVersion = "1.3.8"
    private const val coroutinesVersion = "1.6.3"
    private const val serializationVersion = "1.3.3"
    private const val okhttpVersion = "4.9.1"

    private const val ndkCrashLibVersion = "0.8"

    const val kotlinxSerialization =
        "$kotlinxGroup:kotlinx-serialization-json:$serializationVersion"

    const val kotlinxCoroutinesCore = "$kotlinxGroup:kotlinx-coroutines-android:$coroutinesVersion"
    const val androidLibJRE = "org.jetbrains.kotlin:kotlin-stdlib-jdk7:$kotlinVersion"
    const val okhttp = "com.squareup.okhttp3:okhttp:$okhttpVersion"
    const val workManagerRuntime = "androidx.work:work-runtime-ktx:2.8.1"
}

object TestingLibs {
    const val JunitLib = "junit:junit:4.13.2"
    const val RobolectricLib = "org.robolectric:robolectric:4.10"
    const val MockkLib = "io.mockk:mockk:1.13.4"
    const val TestCore = "androidx.test:core:1.5.0"
    const val JSONObject = "org.json:json:20190722"
    const val KotlinTest = "org.jetbrains.kotlin:kotlin-test:1.8.0"
    const val CoroutineTest = "org.jetbrains.kotlinx:kotlinx-coroutines-test:1.6.3"

    //android tests
    const val JunitExt = "androidx.test.ext:junit:1.2.1"
    const val TestRunner = "androidx.test:runner:1.6.2"
    const val TestRules = "androidx.test:rules:1.6.1"
    const val MockkAndroidLib = "io.mockk:mockk-android:1.13.4"
    const val WorkManagerTesting = "androidx.work:work-testing:2.8.1"
}

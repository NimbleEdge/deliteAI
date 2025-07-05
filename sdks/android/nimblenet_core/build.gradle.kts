/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

val neGradleConfig = NEGradleConfig(project.extra)
val localProperties = fetchLocalProperties(rootDir)

plugins {
    id(Deps.Plugins.ANDROID_LIBRARY)
    id(Deps.Plugins.MAVEN_PUBLISH)
    id(Deps.Plugins.KOTLINX_SERIALIZATION)
    id(Deps.Plugins.KOTLIN_PARCELIZE)
}

apply(plugin = Deps.Plugins.KOTLIN_ANDROID)

android {
    namespace = "dev.deliteai.nimblenet_core"
    compileSdk = neGradleConfig.compileSdk

    defaultConfig {
        minSdk = neGradleConfig.minSdk
        targetSdk = neGradleConfig.targetSdk

        ndk {
            val hasGenAi = neGradleConfig.commonCmakeArguments.any { it == "-DGENAI=1" } ||
                neGradleConfig.androidCmakeArguments.any { it == "-DGENAI=1" }

            //noinspection ChromeOsAbiSupport
            abiFilters += if (hasGenAi) {
                // ONNXRuntime GenAI doesn't support x86 and armeabi-v7
                listOf("arm64-v8a", "x86_64")
            } else {
                listOf("arm64-v8a", "x86_64", "x86", "armeabi-v7a")
            }
            ndkVersion = neGradleConfig.ndkVersion
        }

        externalNativeBuild {
            cmake {
                neGradleConfig.commonCmakeArguments.forEach { arguments += it }
                neGradleConfig.androidCmakeArguments.forEach { arguments += it }
            }
        }

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        consumerProguardFiles("consumer-rules.txt")
    }

    packagingOptions {
        resources {
            excludes += "META-INF/LICENSE.md"
            excludes += "META-INF/LICENSE-notice.md"
            excludes += "META-INF/NOTICE.md"
            excludes += "META-INF/NOTICE.txt"
        }
    }

    flavorDimensions += "default"

    productFlavors {
        create("internal") { dimension = "default" }
        create("external") { dimension = "default" }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
        }
    }

    // Apply publishing configuration using the Publishing class
    Publishing(project, neGradleConfig, "nimblenet_core").apply()

}

tasks.withType<org.jetbrains.kotlin.gradle.tasks.KotlinCompile> {
    kotlinOptions.jvmTarget = "1.8"
}

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

val neGradleConfig = NEGradleConfig(project.extra)

plugins {
    id(UtilityPlugins.androidLibraryPlugin)
    id(UtilityPlugins.mavenPublish)
    id(UtilityPlugins.kotlinxSerialization)
    id(UtilityPlugins.kotlinParcelize)
}

apply(plugin = "kotlin-android")

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

    afterEvaluate {
        publishing {
            publications {
                createMavenPublication("internalRelease", "nimblenet_core")
                createMavenPublication("externalRelease", "nimblenet_core")
            }

            repositories {
                maven {
                    name = "deliteai_android"
                    url = uri(neGradleConfig.awsS3Url)
                    credentials(AwsCredentials::class) {
                        accessKey = neGradleConfig.awsAccessKey
                        secretKey = neGradleConfig.awsSecretKey
                    }
                }
            }
        }
    }

}

tasks.withType<org.jetbrains.kotlin.gradle.tasks.KotlinCompile> {
    kotlinOptions.jvmTarget = "1.8"
}

// Helper function for Maven publications
fun PublishingExtension.createMavenPublication(name: String, artifactId: String) {
    publications {
        register<MavenPublication>(name) {
            from(components[name])
            groupId = "dev.deliteai"
            this.artifactId = artifactId
            version = neGradleConfig.releaseVersion
        }
    }
}

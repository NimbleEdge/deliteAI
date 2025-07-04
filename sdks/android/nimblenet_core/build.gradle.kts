/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import java.util.Properties

val neGradleConfig = NEGradleConfig(project.extra)
val localProperties =
    File(rootDir, "local.properties").let { file ->
        Properties().apply {
            if (file.exists()) {
                load(file.inputStream())
            }
        }
    }

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
                    url = uri(getLocalProperty("AWS_S3_URL"))
                    credentials(AwsCredentials::class) {
                        accessKey = getLocalProperty("AWS_ACCESS_KEY_ID")
                        secretKey =  getLocalProperty("AWS_SECRET_ACCESS_KEY")
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

private fun com.android.build.api.dsl.DefaultConfig.addStringConfigsFromLocalProperties(
    keys: List<String>,
    project: Project
) {
    keys.forEach { key ->
        val value = project.getLocalProperty(key)
        buildConfigField("String", key, "\"$value\"")
    }
}

private fun Project.getLocalProperty(key: String): String {
    val propsFile = rootProject.file("local.properties")
    val props = Properties()
    if (propsFile.exists()) {
        props.load(propsFile.inputStream())
    }
    return props.getProperty(key) ?: throw GradleException("Missing local property: $key")
}

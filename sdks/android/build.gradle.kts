/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import java.io.File
import org.yaml.snakeyaml.Yaml

// TODO: (naman) cleanup this file
// Top-level build file where you can add configuration options common to all sub-projects/modules.
plugins {
    /**
     * You should use `apply false` in the top-level build.gradle file to add
     * a Gradle plugin as a build dependency, but not apply it to the current
     * (root) project. You should not use `apply false` in sub-projects. For
     * more information, see Applying external plugins with same version to
     * subprojects.
     */

    id("com.android.application") version "8.7.3" apply false
    id("com.android.library") version "7.4.2" apply false
    id("org.jetbrains.kotlin.android") version "1.6.21" apply false
    id("org.gradle.maven-publish")
    id("com.android.test") version "7.4.2" apply false
    id("com.android.dynamic-feature") version "7.4.2" apply false
    id("org.jetbrains.kotlin.plugin.compose") version "2.0.0" apply false
}

buildscript {
    val compose_version by extra("1.3.0")
    val agp_version by extra("8.7.3")
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
        maven(url = "https://jitpack.io")
        maven(url = "https://plugins.gradle.org/m2/")
    }

    dependencies {
        classpath(GradleDeps.androidBuildTools)
        classpath(GradleDeps.kotlinGradle)
        classpath(GradleDeps.kotlinxSerialization)
        classpath("org.yaml:snakeyaml:2.2")
        classpath("org.jetbrains.kotlin:kotlin-gradle-plugin:${UtilityLibs.kotlinVersion}")
        classpath("software.amazon.awssdk:s3:2.20.8")
    }
}

allprojects {
    val yaml = Yaml()
    val configFile = File("$rootDir/../config.yml")
    val configData = yaml.load<Map<String, Map<String, String>>>(configFile.reader())

    ext.set("sdkReleaseVersion", configData["common"]?.get("sdk_version"))
    ext.set("cmakeArgumentsCommon", configData["common"]?.get("cmake_args"))
    ext.set("cmakeArgumentsAndroid", configData["android"]?.get("cmake_args"))
    ext.set("ndkVersion", configData["android"]?.get("ndk"))

    repositories {
        mavenCentral()
        gradlePluginPortal()
        google()
    }
}

tasks.register<Delete>("cleanAll") {
    delete(rootProject.buildDir)
    rootProject.subprojects.forEach {
        delete(it.buildDir)
    }
    delete(File("buildSrc/build"))
}

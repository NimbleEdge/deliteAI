/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import java.io.File
import org.yaml.snakeyaml.Yaml

// Top-level build file where you can add configuration options common to all sub-projects/modules.
plugins {
    id(Deps.Plugins.MAVEN_PUBLISH)
    id(Deps.Plugins.KOTLIN_COMPOSE) version Versions.COMPOSE_PLUGIN apply false
}

buildscript {
    val compose_version by extra(Versions.COMPOSE)
    val agp_version by extra(Versions.AGP)
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
        maven(url = "https://jitpack.io")
        maven(url = "https://plugins.gradle.org/m2/")
    }

    dependencies {
        classpath(Deps.BuildScript.ANDROID_GRADLE_PLUGIN)
        classpath(Deps.BuildScript.KOTLIN_GRADLE_PLUGIN)
        classpath(Deps.BuildScript.KOTLIN_SERIALIZATION)
        classpath(Deps.BuildScript.SNAKE_YAML)
        classpath(Deps.BuildScript.AWS_SDK)
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

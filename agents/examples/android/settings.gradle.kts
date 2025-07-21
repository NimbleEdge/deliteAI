/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

pluginManagement {
    repositories {
        google {
            content {
                includeGroupByRegex("com\\.android.*")
                includeGroupByRegex("com\\.google.*")
                includeGroupByRegex("androidx.*")
            }
        }
        mavenCentral()
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
        mavenLocal()
    }
}

// Include the SDK as a composite build
includeBuild("../../../sdks/android") {
    dependencySubstitution {
        substitute(module("dev.deliteai:nimblenet_ktx")).using(project(":nimblenet_ktx"))
        substitute(module("dev.deliteai:nimblenet_core")).using(project(":nimblenet_core"))
        // Explicit version-specific substitutions for IDE sync scenarios
        substitute(module("dev.deliteai:nimblenet_ktx:0.1.0-dev")).using(project(":nimblenet_ktx"))
        substitute(module("dev.deliteai:nimblenet_core:0.1.0-dev")).using(project(":nimblenet_core"))
    }
}

rootProject.name = "Examples"
include(":app")
include(":notifications_summarizer")
include(":gmail_assistant")

project(":notifications_summarizer").projectDir = file("../../notifications_summarizer/android")
project(":gmail_assistant").projectDir = file("../../gmail_assistant/android")

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
        maven(url = "s3://nimblesdk-aar-test.s3.ap-south-1.amazonaws.com") {
            credentials(AwsCredentials::class) {
                accessKey = "AKIATVLEHMTQA6G557EQ"
                secretKey = "LwfJI5XqoivqN4V8PA7pkDRfTk9R+I4wtOOjBAoa"
            }
        }
    }
}

rootProject.name = "Examples"
include(":app")
include(":notifications_summarizer")

project(":notifications_summarizer").projectDir = file("../../notifications_summarizer/android")

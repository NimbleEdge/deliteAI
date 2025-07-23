import org.jetbrains.dokka.gradle.DokkaTask

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
    id(Deps.Plugins.JACOCO)
    id(Deps.Plugins.KTFMT) version Versions.KTFMT
    id(Deps.Plugins.DOKKA) version Versions.DOKKA
}

jacoco { toolVersion = Versions.JACOCO }

apply(plugin = Deps.Plugins.KOTLIN_ANDROID)

android {
    namespace = "dev.deliteai.nimblenet_ktx"
    compileSdk = neGradleConfig.compileSdk

    defaultConfig {
        minSdk = neGradleConfig.minSdk
        targetSdk = neGradleConfig.targetSdk
        consumerProguardFiles("consumer-rules.txt")
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        addStringConfigsFromLocalProperties(
            listOf(
                "ANDROID_TEST_CLIENT_ID",
                "ANDROID_TEST_CLIENT_SECRET",
                "ANDROID_TEST_HOST",
                "REMOTE_LOGGER_KEY",
                "REMOTE_LOGGER_URL",
            ),
            project,
        )
    }

    buildFeatures { buildConfig = true }

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

    if (neGradleConfig.geminiEnabled) {
        sourceSets["main"].java.srcDir("src/gemini/kotlin")
    }

    // Package androidTest assets into the app APK under test
    sourceSets { getByName("androidTest") { assets.srcDir("src/androidTest/assets") } }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    testOptions { unitTests.isReturnDefaultValues = true }

    Publishing(project, neGradleConfig, "nimblenet_ktx").apply()
}

tasks.register("jacocoTestExternalDebugUnitTestReport", JacocoReport::class) {
    group = "Test Code Coverage Reporting"
    description = "Generate Jacoco coverage reports for externalDebug unit tests."

    executionData.setFrom(fileTree(buildDir) { include("jacoco/testExternalDebugUnitTest.exec") })

    classDirectories.setFrom(
        fileTree("$buildDir/tmp/kotlin-classes/externalDebug") {
            exclude(
                "**/R.class",
                "**/R$*.class",
                "**/BuildConfig.*",
                "**/Manifest*.*",
                "**/*Test*.*",
            )
        }
    )

    sourceDirectories.setFrom(
        files("$projectDir/src/main/kotlin", "$projectDir/src/external/kotlin")
    )

    reports {
        xml.required.set(true)
        xml.outputLocation.set(file("$buildDir/reports/jacoco/externalDebug/jacoco.xml"))
        html.required.set(true)
        html.outputLocation.set(file("$buildDir/reports/jacoco/externalDebug/html"))
        csv.required.set(false)
    }

    dependsOn("testExternalDebugUnitTest")
}

dependencies {
    implementation(Deps.Coroutines.ANDROID)
    implementation(Deps.Network.OKHTTP)
    implementation(Deps.Kotlin.STDLIB_JDK7)
    implementation(Deps.AndroidX.WORK_RUNTIME_KTX)
    implementation(Deps.Google.FEATURE_DELIVERY_KTX)
    implementation(Deps.Google.PROTOBUF_JAVA)
    implementation(Deps.Google.PROTOBUF_JAVA_UTIL)
    implementation(Deps.AndroidX.WORK_TESTING)

    if (neGradleConfig.geminiEnabled) {
        implementation(Deps.Google.AI_CORE)
    }

    testImplementation(Deps.Testing.JUNIT)
    testImplementation(Deps.Testing.MOCKK)
    testImplementation(Deps.AndroidX.TEST_CORE)
    testImplementation(Deps.Kotlin.TEST)
    testImplementation(Deps.Testing.JSON)
    testImplementation(Deps.Testing.ROBOLECTRIC)
    testImplementation(Deps.Coroutines.TEST)
    testImplementation(Deps.Kotlin.REFLECT)

    androidTestImplementation(Deps.AndroidX.TEST_EXT_JUNIT)
    androidTestImplementation(Deps.Coroutines.TEST)
    androidTestImplementation(Deps.AndroidX.TEST_RUNNER)
    androidTestImplementation(Deps.AndroidX.TEST_RULES)
    androidTestImplementation(Deps.Testing.MOCKK_ANDROID)
    androidTestImplementation(Deps.AndroidX.WORK_TESTING_ANDROID)
    androidTestImplementation(Deps.AndroidX.TEST_UI_AUTOMATOR)
    androidTestImplementation(project(":nimblenet_core"))
}

tasks.withType<Test> {
    useJUnitPlatform()
    useJUnit()
}

tasks.withType<org.jetbrains.kotlin.gradle.tasks.KotlinCompile> { kotlinOptions.jvmTarget = "1.8" }

ktfmt {
    kotlinLangStyle()
    removeUnusedImports.set(true)
    manageTrailingCommas.set(true)
    srcSetPathExclusionPattern = Regex(".*generated.*")
}

tasks.register("formatKotlin") {
    group = "formatting"
    description = "Apply ktlint formatting to all Kotlin sources"
    dependsOn("ktfmtFormat")
}

// Dokka configuration for API documentation generation
tasks.withType<org.jetbrains.dokka.gradle.DokkaTask>().configureEach {
    dokkaSourceSets {
        named("main") {
            displayName.set("NimbleNet Android SDK")

            // Include all source directories for documentation
            sourceRoots.from(file("src/main/kotlin"))

            // Documentation and linking configuration
            moduleName.set("NimbleNet Android SDK")
            moduleVersion.set(neGradleConfig.releaseVersion)

            // Configure API visibility
            documentedVisibilities.set(
                setOf(
                    org.jetbrains.dokka.DokkaConfiguration.Visibility.PUBLIC,
                    org.jetbrains.dokka.DokkaConfiguration.Visibility.PROTECTED,
                )
            )

            // Package documentation - exclude implementation packages
            perPackageOption {
                matchingRegex.set("dev\\.deliteai\\.impl.*")
                suppress.set(true)
            }
        }
    }
}

tasks.named<DokkaTask>("dokkaGfm") {
    group = "documentation"
    description = "Generate API docs as GitHub-Flavored Markdown"

    dokkaSourceSets {
        named("main") {
            moduleName.set("nimblenet_ktx")
            moduleVersion.set(neGradleConfig.releaseVersion)
            sourceRoots.from(file("src/main/kotlin"))
            documentedVisibilities.set(
                setOf(
                    org.jetbrains.dokka.DokkaConfiguration.Visibility.PUBLIC,
                    org.jetbrains.dokka.DokkaConfiguration.Visibility.PROTECTED,
                )
            )
            perPackageOption {
                matchingRegex.set("dev\\.deliteai\\.impl.*")
                suppress.set(true)
            }
        }
    }
}

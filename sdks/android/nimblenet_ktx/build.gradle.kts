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
    id("jacoco")
    id("com.ncorti.ktfmt.gradle") version "0.22.0"
}

jacoco { toolVersion = "0.8.8" }

apply(plugin = "kotlin-android")

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
                "TEST_CLIENT_ID",
                "TEST_CLIENT_SECRET",
                "TEST_HOST",
                "TEST_COMPATIBILITY_TAG",
                "ANDROID_TEST_CLIENT_ID",
                "ANDROID_TEST_CLIENT_SECRET",
                "ANDROID_TEST_HOST",
                "ANDROID_TEST_COMPATIBILITY_TAG",
                "REMOTE_LOGGER_KEY"),
            project
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

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    testOptions { unitTests.isReturnDefaultValues = true }

    afterEvaluate {
        publishing {
            publications {
                createMavenPublication("internalRelease", "nimblenet_ktx")
                createMavenPublication("externalRelease", "nimblenet_ktx")
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

tasks.register("jacocoTestExternalDebugUnitTestReport", JacocoReport::class) {
    group = "Test Code Coverage Reporting"
    description = "Generate Jacoco coverage reports for externalDebug unit tests."

    executionData.setFrom(fileTree(buildDir) { include("jacoco/testExternalDebugUnitTest.exec") })

    // make sure to change the path here, if the flavors change
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
    implementation(UtilityLibs.kotlinxCoroutinesCore)
    implementation(UtilityLibs.okhttp)
    implementation(UtilityLibs.androidLibJRE)
    implementation(UtilityLibs.workManagerRuntime)
    implementation("com.google.android.play:feature-delivery-ktx:2.1.0")
    implementation("com.google.protobuf:protobuf-java:3.25.2")
    implementation("com.google.protobuf:protobuf-java-util:3.25.2")
    implementation("androidx.work:work-testing:2.10.1")

    if (neGradleConfig.geminiEnabled) {
        implementation("com.google.ai.edge.aicore:aicore:0.0.1-exp01")
    }

    testImplementation(TestingLibs.JunitLib)
    testImplementation(TestingLibs.MockkLib)
    testImplementation(TestingLibs.TestCore)
    testImplementation(TestingLibs.KotlinTest)
    testImplementation(TestingLibs.JSONObject)
    testImplementation(TestingLibs.RobolectricLib)
    testImplementation(TestingLibs.CoroutineTest)
    testImplementation("org.jetbrains.kotlin:kotlin-reflect:1.7.10")

    androidTestImplementation(TestingLibs.JunitExt)
    androidTestImplementation(TestingLibs.CoroutineTest)
    androidTestImplementation(TestingLibs.TestRunner)
    androidTestImplementation(TestingLibs.TestRules)
    androidTestImplementation(TestingLibs.MockkAndroidLib)
    androidTestImplementation(TestingLibs.WorkManagerTesting)
    androidTestImplementation("androidx.test.uiautomator:uiautomator:2.2.0")
    androidTestImplementation(project(":nimblenet_core"))
}

tasks.withType<Test> {
    useJUnitPlatform()
    useJUnit()
}

tasks.withType<org.jetbrains.kotlin.gradle.tasks.KotlinCompile> { kotlinOptions.jvmTarget = "1.8" }

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

ktfmt {
    // Enforce Kotlinlang style (4-space indent, 100-column wrap)
    kotlinLangStyle()

    removeUnusedImports.set(true)
    manageTrailingCommas.set(true)

    // Exclude generated source sets:
    srcSetPathExclusionPattern = Regex(".*generated.*")
}

tasks.register("formatKotlin") {
    group = "formatting"
    description = "Apply ktlint formatting to all Kotlin sources"
    dependsOn("ktfmtFormat")
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

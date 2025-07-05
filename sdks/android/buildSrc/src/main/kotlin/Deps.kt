object Deps {
    object Plugins {
        const val ANDROID_LIBRARY = "com.android.library"
        const val KOTLIN_ANDROID = "kotlin-android"
        const val KOTLIN_PARCELIZE = "kotlin-parcelize"
        const val KOTLINX_SERIALIZATION = "kotlinx-serialization"
        const val MAVEN_PUBLISH = "org.gradle.maven-publish"
        const val KOTLIN_COMPOSE = "org.jetbrains.kotlin.plugin.compose"
        const val JACOCO = "jacoco"
        const val KTFMT = "com.ncorti.ktfmt.gradle"
    }

    object BuildScript {
        const val ANDROID_GRADLE_PLUGIN = "com.android.tools.build:gradle:${Versions.AGP}"
        const val KOTLIN_GRADLE_PLUGIN = "org.jetbrains.kotlin:kotlin-gradle-plugin:${Versions.KOTLIN}"
        const val KOTLIN_SERIALIZATION = "org.jetbrains.kotlin:kotlin-serialization:${Versions.KOTLIN}"
        const val SNAKE_YAML = "org.yaml:snakeyaml:${Versions.SNAKE_YAML}"
        const val AWS_SDK = "software.amazon.awssdk:s3:${Versions.AWS_SDK}"
    }
    
    object Kotlin {
        const val STDLIB_JDK7 = "org.jetbrains.kotlin:kotlin-stdlib-jdk7:${Versions.KOTLIN}"
        const val TEST = "org.jetbrains.kotlin:kotlin-test:${Versions.KOTLIN_TEST}"
        const val REFLECT = "org.jetbrains.kotlin:kotlin-reflect:${Versions.KOTLIN_REFLECT}"
    }
    
    object Coroutines {
        const val ANDROID = "org.jetbrains.kotlinx:kotlinx-coroutines-android:${Versions.KOTLINX_COROUTINES_ANDROID}"
        const val TEST = "org.jetbrains.kotlinx:kotlinx-coroutines-test:${Versions.KOTLINX_COROUTINES_TEST}"
    }
    
    object Network {
        const val OKHTTP = "com.squareup.okhttp3:okhttp:${Versions.OKHTTP}"
    }
    
    object AndroidX {
        const val WORK_RUNTIME_KTX = "androidx.work:work-runtime-ktx:${Versions.WORK_RUNTIME}"
        const val WORK_TESTING = "androidx.work:work-testing:${Versions.WORK_TESTING}"
        const val WORK_TESTING_ANDROID = "androidx.work:work-testing:${Versions.WORK_TESTING_ANDROID}"
        const val TEST_CORE = "androidx.test:core:${Versions.ANDROIDX_TEST_CORE}"
        const val TEST_EXT_JUNIT = "androidx.test.ext:junit:${Versions.ANDROIDX_TEST_EXT_JUNIT}"
        const val TEST_RUNNER = "androidx.test:runner:${Versions.ANDROIDX_TEST_RUNNER}"
        const val TEST_RULES = "androidx.test:rules:${Versions.ANDROIDX_TEST_RULES}"
        const val TEST_UI_AUTOMATOR = "androidx.test.uiautomator:uiautomator:${Versions.ANDROIDX_TEST_UI_AUTOMATOR}"
    }
    
    object Google {
        const val FEATURE_DELIVERY_KTX = "com.google.android.play:feature-delivery-ktx:${Versions.FEATURE_DELIVERY_KTX}"
        const val PROTOBUF_JAVA = "com.google.protobuf:protobuf-java:${Versions.PROTOBUF_JAVA}"
        const val PROTOBUF_JAVA_UTIL = "com.google.protobuf:protobuf-java-util:${Versions.PROTOBUF_JAVA}"
        const val AI_CORE = "com.google.ai.edge.aicore:aicore:${Versions.AI_CORE}"
    }
    
    object Testing {
        const val JUNIT = "junit:junit:${Versions.JUNIT}"
        const val MOCKK = "io.mockk:mockk:${Versions.MOCKK}"
        const val MOCKK_ANDROID = "io.mockk:mockk-android:${Versions.MOCKK}"
        const val JSON = "org.json:json:${Versions.JSON}"
        const val ROBOLECTRIC = "org.robolectric:robolectric:${Versions.ROBOLECTRIC}"
    }
}

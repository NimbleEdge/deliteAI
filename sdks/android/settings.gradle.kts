pluginManagement {
    repositories {
        gradlePluginPortal()
        google()
        mavenCentral()
        maven(url = "https://jitpack.io")
    }
}

rootProject.name = "android-sdk"
include(":app")
include(":nimblenet_core")
include(":nimblenet_ktx")

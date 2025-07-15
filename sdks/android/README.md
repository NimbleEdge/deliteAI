# DeliteAI Android SDK

[![Platform](https://img.shields.io/badge/platform-Android-green.svg)](https://www.android.com)
[![Language](https://img.shields.io/badge/language-Kotlin-orange.svg)](https://kotlinlang.org)

*A powerful On-Device Android SDK for creating real-time AI-powered experiences natively integrated in your applications.*


## Table of Contents

- [Android Project Overview](#android-project-overview)
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Integrating DeliteAI Android SDK into your App](#integrating-deliteai-android-sdk-into-your-app)
- [Configuration](#configuration)
- [Integration Example](#integration-example)
- [Detailed Documentation](#detailed-documentation)
- [Running Tests](#running-tests)
- [API Reference](#api-reference)

## Android Project Overview

The NimbleNet Android SDK features a **modular architecture** designed for flexibility and separation of concerns. It leverages a high-performance **native C++ core** while exposing a clean and developer-friendly **Kotlin API**.

### Project Structure

```
sdks/android/
├── app/                   # Sample application demonstrating SDK usage
├── buildSrc/              # Gradle build configuration and dependencies
├── nimblenet_ktx/         # Kotlin SDK - Main developer interface
├── nimblenet_core/        # Native bridge and C++ core runtime
└── benchmarking/          # Performance benchmarking tools
```

### Modules

#### 1. `buildSrc`
Contains centralized Gradle dependencies, versions, and build tasks shared across all modules. This ensures consistent dependency management and build configuration.

#### 2. `nimblenet_ktx`
The primary Kotlin SDK module that developers interact with. It provides:
- High-level Kotlin APIs for AI model execution
- Event tracking and analytics
- Configuration management
- Coroutine-based asynchronous operations

#### 3. `nimblenet_core`
The native bridge module that contains:
- JNI bindings to the C++ core runtime
- Native library management
- Performance-critical operations
- Symlinked core runtime components

#### 4. `benchmarking`
Performance measurement tools including:
- Python scripts for SDK benchmarking
- Memory usage analysis
- Latency measurement utilities

### Build Flavors

The SDK supports two build flavors to accommodate different use cases:

#### 1. `External` (Release)
- Standard production-ready build
- Optimized for performance and size
- Contains all essential LLM and AI capabilities
- Recommended for production applications

#### 2. `Internal` (Debug)
- Extended debugging capabilities over External flavor
- Development utilities (database reset, cache management)

## Prerequisites

Before you begin, ensure your development environment meets the following requirements:

#### 1. Android Studio
- **Version**: Arctic Fox (2020.3.1) or later
- **Gradle**: 7.4 or later
- **Build Tools**: 33.0.0 or later

#### 2. Java Development Kit (JDK)
- **Version**: JDK 11 or later
- Ensure `JAVA_HOME` environment variable is properly set

#### 3. Android Debug Bridge (ADB)
- Required for device testing and debugging
- Usually installed with Android SDK Platform Tools
- Verify installation: `adb version`

#### 4. Device Requirements
- **Minimum API Level**: 21 (Android 5.0)
- **Architecture**: ARM64 (arm64-v8a) recommended
- **RAM**: 4GB minimum, 8GB recommended
- **Storage**: 2GB free space for models and cache

#### 5. Project Configuration

Create a `local.properties` file in the root of `deliteAI/sdks/android` to configure your credentials and build settings:

```properties
# Android SDK path
sdk.dir=/path/to/android/sdk

# NimbleNet Configuration (Required)
APP_CLIENT_ID=your_client_id
APP_CLIENT_SECRET=your_client_secret
APP_HOST=https://your-api-endpoint.com

# Optional: Android Test Configuration
ANDROID_TEST_CLIENT_ID=test_client_id
ANDROID_TEST_CLIENT_SECRET=test_client_secret
ANDROID_TEST_HOST=https://test-api-endpoint.com

# Optional: Remote Logging
REMOTE_LOGGER_KEY=your_logger_key
REMOTE_LOGGER_URL=https://your-logging-endpoint.com

# Optional: Publishing (For SDK developers only)
ANDROID_DEV_AWS_ACCESS_KEY_ID=your_aws_key
ANDROID_DEV_AWS_SECRET_ACCESS_KEY=your_aws_secret
ANDROID_DEV_AWS_S3_URL=your_s3_url
OSS_USER=your_maven_user
OSS_PASSWORD=your_maven_password

# Optional: Code Signing (For release builds)
storeFile=/path/to/keystore.jks
storePassword=your_store_password
keyPassword=your_key_password
keyAlias=your_key_alias
```

#### 6. External Dependencies 

Run the following command to download all the required dependencies:

```bash
cd $(git rev-parse --show-toplevel) && ./setup.sh --sdk android
```
## Quick Start

Follow these steps to get the sample application running in minutes.

### Run Sample Application

#### Using Android Studio
1. Open `deliteAI/sdks/android` in Android Studio
2. Wait for Gradle sync to complete
3. Select the `app` module
4. Click **Run** or press `Ctrl+R` (Windows/Linux) / `Cmd+R` (Mac)

#### Using Command Line
```bash
cd deliteAI/sdks/android

# Build and install debug version
./gradlew assembleExternalDebug
./gradlew installExternalDebug

# Launch the app
adb shell monkey -p dev.deliteai.android.sampleapp.debug -c android.intent.category.LAUNCHER 1
```

## Integrating DeliteAI Android SDK into your App

Add the following dependencies to your module-level `build.gradle.kts` or `build.gradle` file.

### Gradle (Kotlin DSL)

```kotlin
dependencies {
    implementation("dev.deliteai:nimblenet_ktx:1.0.0")
    implementation("dev.deliteai:nimblenet_core:1.0.0")
}
```

### Gradle (Groovy)

```groovy
dependencies {
    implementation 'dev.deliteai:nimblenet_ktx:1.0.0'
    implementation 'dev.deliteai:nimblenet_core:1.0.0'
}
```

## Configuration

### SDK Configuration

The SDK's native behavior is controlled by `deliteAI/config.yml`.

```yaml
common:
  sdk_version: "1.0.0"
  cmake_args: "-DONNX_EXECUTOR=1 -DONNXGENAI_EXECUTOR=1 -DCMAKE_BUILD_TYPE=Release -DSCRIPTING=1"

android:
  ndk: "25.1.8937393"
  cmake_args: "-DANDROID_STL=c++_shared -DEXECUTORCH_EXECUTOR=0 -DONNXBuild=1_21_0_full"
```

#### Configuration Options

- **sdk_version**: Defines the SDK version for release builds.
- **common.cmake_args**: Common CMake arguments applied across all platforms (Android, iOS).
- **android.cmake_args**: Additional CMake arguments applied only to the Android build.
- **android.ndk**: Specifies the Android NDK version used for native C++ compilation.

(TODO: replace with working link) For a complete list of supported flags, refer to our [Core Runtime Documentation](../../coreruntime/README.md).

## Integration Example

### Initialize the SDK

```kotlin
import dev.deliteai.NimbleNet
import dev.deliteai.NimbleNetConfig
import dev.deliteai.NIMBLENET_VARIANTS
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        initializeNimbleNet()
    }
    
    private fun initializeNimbleNet() {
        val onlineConfig = NimbleNetConfig(
            clientId = "your-client-id",
            clientSecret = "your-client-secret",
            host = "https://your-api-endpoint.com",
            deviceId = "unique-device-identifier",
            compatibilityTag = "YourModelVersion",
            libraryVariant = NIMBLENET_VARIANTS.STATIC,
            online = true
        )
        
        val offlineConfig = NimbleNetConfig(online = false)
        
        // To initialize the SDK in offline mode place script and model in the assets folder and create the corresponding config
        // To get started there are some sample scripts and models placed in mockserverAssets
        val assetsJsonStr = """
            [
                {
                    "name": "workflow_script",
                    "version": "1.0.0",
                    "type": "script",
                    "location": {
                        "path": "add_script.ast"
                    }
                },
                {
                    "name": "add_model",
                    "version": "1.0.0",
                    "type": "model",
                    "location": {
                        "path": "add_two_model.onnx"
                    }
                }
          ]"""
        
        val assetsJson = JSONArray(assetsJsonStr)

        CoroutineScope(Dispatchers.Default).launch {
            // To initialize SDK in online mode
            val result = NimbleNet.initialize(applicationContext, onlineConfig)
            
            // To initialize SDK in offline mode
            // val result = NimbleNet.initialize(applicationContext, onlineConfig, assetsJson)
            
            if (result.status) {
                Log.d("NimbleNet", "SDK initialized successfully")
                // SDK is ready, you can now track events or run models
            } else {
                Log.e("NimbleNet", "Initialization failed: ${result.error?.message}")
                // Handle failure, e.g., show an error message or retry
            }
        }
    }
}
```

### Check SDK Readiness

```kotlin
private fun checkSDKReadiness() {
    val readyStatus = NimbleNet.isReady()
    
    if (readyStatus.status) {
        Log.d("NimbleNet", "SDK is ready for use")
        // Safe to call other NimbleNet methods
    } else {
        Log.w("NimbleNet", "SDK not ready: ${readyStatus.error?.message}")
        // Wait for initialization to complete or handle the error
    }
}
```

### Track Events

```kotlin
private fun trackUserInteraction() {
    val eventData = mapOf(
        "user_id" to 12345,
        "item_id" to 8001,
        "action" to "click",
        "category" to "electronics",
        "price" to 299.99,
        "timestamp" to System.currentTimeMillis()
    )

    val result = NimbleNet.addEvent(
        eventData = eventData,
        tableName = "UserInteractionsTable"
    )

    if (result.status) {
        Log.d("NimbleNet", "Event tracked successfully")
    } else {
        Log.e("NimbleNet", "Failed to track event: ${result.error?.message}")
    }
}
```

### Execute AI/ML Models

```kotlin
import dev.deliteai.ModelInput
import dev.deliteai.DATATYPE

private fun getPersonalizedRecommendations() {
    val inputs = mapOf(
        "user_id" to ModelInput(
            data = 12345,
            datatype = DATATYPE.INT,
            shape = intArrayOf()
        ),
        "category" to ModelInput(
            data = "electronics",
            datatype = DATATYPE.STRING,
            shape = intArrayOf()
        ),
        "limit" to ModelInput(
            data = 10,
            datatype = DATATYPE.INT,
            shape = intArrayOf()
        )
    )

    CoroutineScope(Dispatchers.IO).launch {
        val result = NimbleNet.runMethod(
            methodName = "get_recommendations",
            inputs = inputs
        )

        withContext(Dispatchers.Main) {
            if (result.status) {
                val outputs = result.data!!
                val recommendations = outputs["recommended_items"]?.data as? Array<Int>
                
                Log.d("NimbleNet", "Recommendations: ${recommendations?.joinToString()}")
                // Update UI with new recommendations
            } else {
                Log.e("NimbleNet", "Failed to get recommendations: ${result.error?.message}")
                // Handle failure, e.g., show a default list or an error message
            }
        }
    }
}
```

## Running Tests

The NimbleNet Android SDK includes comprehensive test suites to ensure reliability and functionality. You can run both unit tests and instrumented Android tests.

### Unit Tests

Run unit tests to verify core business logic and SDK functionality:

```bash
./gradlew :nimblenet_ktx:testExternalDebugUnitTest
```

### Android Tests

Run instrumented tests on a connected device or emulator:

```bash
./gradlew :nimblenet_ktx:connectedExternalDebugAndroidTest
```

**Prerequisites for Android Tests:**
- Connected Android device or running emulator
- Device meets the minimum requirements listed in [Prerequisites](#-prerequisites)
- Proper configuration in `local.properties`

## API Reference

#### (TODO: replace with working link) For detailed API documentation, see our [API Reference Guide](docs/api-reference.md).

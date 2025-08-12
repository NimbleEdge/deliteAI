# DeliteAI Android SDK

[![Platform](https://img.shields.io/badge/platform-Android-green.svg)](https://www.android.com)
[![Language](https://img.shields.io/badge/language-Kotlin-orange.svg)](https://kotlinlang.org)

*A powerful On-Device Android SDK for creating real-time AI-powered experiences natively integrated
in your applications.*

## Table of Contents

- [Android Project Overview](#android-project-overview)
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Configuration](#configuration)
- [Integrating DeliteAI Android SDK into your App](#integrating-deliteai-android-sdk-into-your-app)
- [Integration Example](#integration-example)
- [Running Tests](#running-tests)
- [API Reference](#api-reference)

## Android Project Overview

The DeliteAI Android SDK features a **modular architecture** designed for flexibility and separation
of concerns. It leverages a high-performance **native C++ core** while exposing a clean and
developer-friendly **Kotlin API**.

### Project Structure

```text
sdks/android/
├── app/                   # Sample application demonstrating SDK usage
├── buildSrc/              # Gradle build configuration and dependencies
├── nimblenet_ktx/         # Kotlin SDK - Main developer interface
├── nimblenet_core/        # Native bridge and C++ core runtime
└── benchmarking/          # Performance benchmarking tools
```

### Modules

#### 1. `buildSrc`

Contains centralized Gradle dependencies, versions, and build tasks shared across all modules. This
ensures consistent dependency management and build configuration.

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

### 1. Android Studio

- **Version**: Arctic Fox (2020.3.1) or later
- **Gradle**: 7.4 or later
- **Build Tools**: 33.0.0 or later

### 2. Java Development Kit (JDK)

- **Version**: JDK 11 or later
- Ensure `JAVA_HOME` environment variable is properly set

### 3. Android Debug Bridge (ADB)

- Required for device testing and debugging
- Usually installed with Android SDK Platform Tools
- Verify installation: `adb version`

### 4. Device Requirements

- **Minimum API Level**: 21 (Android 5.0)
- **Architecture**: ARM64 (arm64-v8a) recommended
- **RAM**: 4GB minimum, 8GB recommended
- **Storage**: 2GB free space for models and cache

### 5. Project Configuration

Create a `local.properties` file in the root of `deliteAI/sdks/android` to configure your
credentials and build settings:

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

### 6. External Dependencies

Run the following command to download all the required dependencies:

```bash
cd "$(git rev-parse --show-toplevel)" && ./setup.sh --sdk android
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

## Configuration

> **Note**: If you're using the pre-released version of the SDK from our repository, you can skip this section entirely.

This section describes how to configure the SDK for development and release builds. It is primarily relevant for:
- SDK maintainers and contributors
- Organizations creating custom SDK builds
- Developers who need to modify SDK behavior at build time

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

(TODO: replace with working link)
For a complete list of supported flags, refer to our
[CoreRuntime Documentation](../../coreruntime/README.md).

## Integrating DeliteAI Android SDK into your App

Add the following dependencies to your module-level `build.gradle.kts` or `build.gradle` file.

### Gradle (Kotlin DSL)

```kotlin
dependencies {
    implementation("dev.deliteai:nimblenet_ktx:x.y.z")
    implementation("dev.deliteai:nimblenet_core:x.y.z")
}
```

### Gradle (Groovy)

```groovy
dependencies {
    implementation 'dev.deliteai:nimblenet_ktx:x.y.z'
    implementation 'dev.deliteai:nimblenet_core:x.y.z'
}
```

## Integration Example

This section demonstrates a complete integration workflow for the DeliteAI Android SDK. Follow these steps to get started with implementing AI-powered functionality in your Android application.

### Overview

The integration workflow includes:

1. **Initialize the SDK** - Configure and start the NimbleNet SDK
2. **Check SDK readiness** - Verify the SDK is ready for use
3. **Execute AI models** - Run Python workflows from Kotlin
4. **Track events** - Optional analytics and event tracking

### 1. Initialize the SDK

The SDK supports two mutually exclusive initialization modes:

#### Online Mode

- Downloads all required assets (models, scripts, configuration) on first launch
- Caches assets locally for subsequent offline use
- Automatically checks for updates when online

#### Offline Mode (Default)

- Bundles all required assets in your app's `assets/` folder at build time
- No network calls at runtime - perfect for fully offline deployments

##### Example Python Script

For offline mode, create a Python script with your AI workflow:

```python
def function1(inputMap):
    text = inputMap["input_key"]
    print(text)  # Visible in Logcat
    return {"output_key": text + "Output"}
```

##### Convert Script to AST (Offline Only)

Android assets cannot contain raw Python source files. Convert your script to AST format:

```bash
cd "$(git rev-parse --show-toplevel)"
python3 coreruntime/scripts/gen_python_ast.py path/to/your_script.py
```

This generates `your_script.ast`. Place this file in `app/src/main/assets/`.

##### SDK Initialization Code

```kotlin
import dev.deliteai.NimbleNet
import dev.deliteai.NimbleNetConfig
import dev.deliteai.NIMBLENET_VARIANTS
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

private fun initializeNimbleNet() {
    val offlineConfig = NimbleNetConfig(online = false)

    // Configuration for offline mode assets
    val assetsJsonStr = """
        [
            {
                "name": "workflow_script",
                "version": "1.0.0",
                "type": "script",
                "location": {
                    "path": "your_script.ast"
                }
            }
        ]"""

    val assetsJson = JSONArray(assetsJsonStr)

    CoroutineScope(Dispatchers.Default).launch {
        // Initialize SDK in offline mode
        val result = NimbleNet.initialize(applicationContext, offlineConfig, assetsJson)

        if (result.status) {
            Log.d("NimbleNet", "SDK initialized successfully")
            // SDK is ready for use
        } else {
            Log.e("NimbleNet", "Initialization failed: ${result.error?.message}")
            // Handle initialization failure
        }
    }
}
```

### 2. Check SDK Readiness

Before using SDK functionality, verify it's ready:

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

### 3. Execute AI Models

Run Python workflows from your Kotlin code:

```kotlin
import dev.deliteai.datamodels.NimbleNetTensor
import dev.deliteai.impl.common.DATATYPE

private fun runPyWorkflow() {
    val inputs = hashMapOf(
        "input_key" to NimbleNetTensor(
            data = "hello world",
            datatype = DATATYPE.STRING,
            shape = intArrayOf() // Populate only if data is an array
        )
    )

    CoroutineScope(Dispatchers.Default).launch {
        val result = NimbleNet.runMethod(
            methodName = "myFunction",
            inputs = inputs
        )

        withContext(Dispatchers.Main) {
            if (result.status) {
                val outputs = result.payload!!
                val output = outputs["output_key"]?.data as? String

                Log.d("NimbleNet", "Output: $output")
            } else {
                Log.e("NimbleNet", "Failed to get output: ${result.error?.message}")
            }
        }
    }
}
```

### 4. Track Events (Optional)

Track user interactions and custom analytics events in Python workflow scripts for use in your AI logic:

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

## Running Tests

The DeliteAI Android SDK includes comprehensive test suites to ensure reliability and functionality.
You can run both unit tests and instrumented Android tests.

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
- Device meets the minimum requirements listed in [Prerequisites](#prerequisites)
- Proper configuration in `local.properties`

## API Reference

<!--
### (TODO: replace with working link) For detailed API documentation, see our [API Reference Guide](docs/api-reference.md).
-->

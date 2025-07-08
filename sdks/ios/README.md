# DeliteAI iOS SDK

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](../../LICENSE)
[![Platform](https://img.shields.io/badge/platform-iOS-orange.svg)](https://github.com/NimbleEdge/deliteAI)
[![Languages](https://img.shields.io/badge/language-Swift-orange.svg)](https://github.com/NimbleEdge/deliteAI)

**DeliteAI iOS SDK** is an easy to integrate SDK into your iOS applications enabling you to build **AI agentic workflows** and experiences in your apps without worrying about low level interfaces or privacy challenges as the AI is run **locally on-device**.

---

## 🧭 Navigation

* **Integrating DeliteAI iOS SDK into Your Project**: If you want to integrate the DeliteAI iOS SDK into your existing application.
    * [Installation](#-installation)
    * [Integrating DeliteAI iOS SDK in Code](#-integrating-deliteai-ios-sdk-in-code)
* **Local Development & Testing**: If you want to run the iOS SDK locally via the example app or execute its unit tests.
    * [Running the Example App Locally](#-running-the-example-app-locally)
    * [Running Unit Tests](#-running-unit-tests)


# Integrating DeliteAI iOS SDK into Your Project

This section guides you through the process of adding the DeliteAI iOS SDK to your application and making your first API calls.

## 🚀 Installation

### CocoaPods

DeliteAI iOS SDK is available as a CocoaPod. To integrate it into your Xcode project:

1.  Add the DeliteAI iOS SDK source and pod to your **Podfile**:

    ```ruby
    source 'git@github.com:NimbleEdge/deliteAI-iOS-Podspecs.git' # deliteAI source
    source 'https://github.com/CocoaPods/Specs.git' # cocoaPods source

    platform :ios, '12.0'

    target 'YourAppTarget' do
      use_frameworks!

      pod 'DeliteAI',
    end
    ```

2.  Install the pod:

    ```bash
    pod install
    ```

3.  Open the generated `.xcworkspace` file in Xcode.

## 📦 Integrating DeliteAI iOS SDK in Code

Once you’ve installed the SDK, follow the steps below to integrate DeliteAI into your app code.

1.  Import the Package

    Import DeliteAI into your Swift files to access DeliteAI iOS SDK's core APIs

    ```swift
    import DeliteAI
    ```

2.  Initialize the SDK

    Before using any methods, you must initialize the SDK with your credentials and endpoint.

    ```swift
    let nimbleNetConfig = NimbleNetConfig(
        clientId: "quick-start-client",
        clientSecret: "quick-start-secret",
        host: "[https://your-server.com/](https://your-server.com/)",
        deviceId: "qs-iOS",
        debug: true,
        compatibilityTag: "RankerV1"
    )

    let result = NimbleNetApi.initialize(config: nimbleNetConfig)
    if result.status {
        print("NimbleNet initialized successfully")
      } else {
        print("NimbleNet initialization failed with error: \(result.error?.localizedDescription ?? "")")
    }
    ```

3.  Check SDK Readiness

    Always verify the SDK is ready before making further calls:

    ```swift
    let ready = NimbleNetApi.isReady()
    if (ready.status) {
        print("NimbleNet sdk is ready")
    } else {
        print("NimbleNet sdk is not ready: \(ready.error?.localizedDescription ?? "")")
    }
    ```

4.  Record Restaurant Click Events

    Track user clicks with the required fields:

    ```swift
    var eventData: [String : Any] = [
        "restaurant_id": 8001,
        "category": "desserts",
        "priceForTwo": 2500,
        "distance": 4.3
    ]

    var eventResult = NimbleNetApi.addEvent(events: eventData, eventType: "RestaurantClicksTable")

    if (eventResult.status) {
      print("NimbleNet Event recorded")
    } else {
      print("NimbleNet Event failed: \(eventResult.error?.message ?? "Unknown error")")
    }
    ```

5.  Fetch Restaurant Rankings

    Invoke the `rerank_restaurant` workflow to get top restaurants:

    ```kotlin
    let inputs: [String: NimbleNetTensor] = [
       "category": NimbleNetTensor(
            data: "desserts",
            datatype: DataType.string,
           shape: []
        )
    ]

    let ranking = NimbleNetApi.runMethod(methodName: "rerank_restaurant", inputs: inputs)

    if (ranking.status) {
       let outputs = ranking.payload?.map
       let topRestaurants = outputs?["topRestaurants"]
       print("NimbleNet Top restaurants: \(topRestaurants?.name ?? "")")
    } else {
       print("NimbleNet Ranking failed: \(ranking.error?.message ?? "Unknown error")")
    }
    ```


# Local Development & Testing

This section details how to run and test the DeliteAI iOS SDK directly from its source code using the provided example application and dedicated unit tests.

## 🧪 Running the Example App Locally

To effectively test and observe the functionalities of the DeliteAI iOS SDK locally, you can utilize the provided `NimbleNetExample` application. Follow these detailed steps to get it up and running:

1.  **Navigate to the Example Directory:**
    Open your terminal or command prompt and change your current directory to the `example` folder located within your cloned DeliteAI iOS SDK repository.

2.  **Install Dependencies via CocoaPods:**
    From within the `example` directory, execute the following command. This will fetch all necessary dependencies, including the DeliteAI iOS SDK itself from your local code, ensuring you're testing against your current development version.

    ```bash
    pod install
    ```

3.  **Open the Xcode Workspace:**
    Once the `pod install` command completes successfully, open the generated Xcode workspace file. It's crucial to open the `.xcworkspace` file, not the `.xcodeproj` file, to ensure all pods are correctly linked.

    ```bash
    open NimbleNetExample.xcworkspace
    ```

4.  **Select the Target:**
    In Xcode, ensure that `NimbleNetExample` is selected as the active target for compilation and running. You can find this dropdown menu next to the play/stop buttons in the Xcode toolbar.

5.  **Build and Run the Application:**
    With the target selected, click the **Run** (play) button in Xcode. Choose your desired simulator or a connected iOS device. As the app runs, closely observe the **Debug Area**. This output will help you verify the core functionalities of the SDK.

## 🔬 Running Unit Tests

The `NimbleNetExample` project includes a dedicated test target, `NimbleNetExample_Tests`, with several unit tests to ensure the robustness and correctness of the DeliteAI iOS SDK's functionalities. Developers can run these tests to verify specific components and behaviors.

The following test files are available:

1. RunMethodInstrumentation.swift
2. ProtoTest.swift
3. Keychaintest.swift

To run these test cases:

1.  **Open the Xcode Workspace:**
    If not already open, open `NimbleNetExample.xcworkspace` in Xcode as described in the installation steps.

2.  **Navigate to Test Files:**
    In Xcode's Project Navigator (left sidebar), locate the `Test` directory. Expand it to find the source files, including `RunMethodInstrumentation.swift`, `ProtoTest.swift`, and `Keychaintest.swift`.

3.  **Execute Specific Tests:**
    You have a few options to run the tests:
    * **Run a Single Test Class:** Open any of the test files (e.g., `RunMethodInstrumentation.swift`). You will see a small diamond-shaped "play" button next to the `class` declaration line. Clicking this button will run all test methods within that specific class.
    * **Run a Single Test Function:** Within a test file, each individual `func` (test method) also has a diamond-shaped "play" button next to its declaration. Clicking this will execute only that particular test function.

4.  **Observe Test Results:**
    After running tests, Xcode will display the results in the **Debug Area** and the **Test Navigator** (accessible via `Cmd + 6`). Successful tests will be marked with a green checkmark, while failures will show a red 'X' along with details about the assertion that failed.


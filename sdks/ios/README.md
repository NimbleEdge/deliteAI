# DeliteAI iOS SDK

[![Platform](https://img.shields.io/badge/platform-iOS-orange.svg)](https://developer.apple.com/ios/)
[![Languages](https://img.shields.io/badge/language-Swift-orange.svg)](https://www.swift.org/)

**DeliteAI iOS SDK** is an powerful yet easy to integrate SDK into your iOS applications enabling you to build **AI agentic workflows** and experiences in your apps without worrying about low level interfaces or privacy challenges as the AI is run **locally on-device**.

---

## Table of Contents

* **Integrating DeliteAI iOS SDK into your application**: If you want to integrate the DeliteAI iOS SDK into your existing application.
  * [Installation](#installation)
  * [Integrating DeliteAI iOS SDK in Code](#integrating-deliteai-ios-sdk-in-code)
* **Local Development & Testing**: If you want to run the iOS SDK locally via the example app or execute its unit tests.
  * [Running the Example App Locally](./docs/DEVELOPMENT.md#running-the-sdk-locally-through-example-app)
  * [Running Unit Tests](./docs/DEVELOPMENT.md#running-unit-tests)

## Integrating DeliteAI iOS SDK into Your Project

This section guides you through the process of adding the DeliteAI iOS SDK to your application and making your first API calls.

### Installation

#### CocoaPods

DeliteAI iOS SDK is available as a CocoaPod. To integrate it into your Xcode project:

1. Add the DeliteAI iOS SDK source and pod to your **Podfile**:

    ```ruby
    source 'git@github.com:NimbleEdge/deliteAI-iOS-Podspecs.git' # deliteAI source
    source 'https://github.com/CocoaPods/Specs.git' # cocoaPods source

    platform :ios, '12.0'

    target 'YourAppTarget' do
      use_frameworks!

      pod 'DeliteAI',
    end
    ```

2. Install the pod:

    ```bash
    pod install
    ```

3. Open the generated `.xcworkspace` file in Xcode.

### Integrating DeliteAI iOS SDK in Code

Once you’ve installed the SDK, follow the steps below to integrate DeliteAI into your app code.

1. Import the Package

    Import DeliteAI into your Swift files to access DeliteAI iOS SDK's core APIs

    ```swift
    import DeliteAI
    ```

2. Initialize the SDK

    Before using any methods, you must initialize the SDK with your credentials and endpoint.

    ```swift
    let nimbleNetConfig = NimbleNetConfig(
        clientId: "quick-start-client",
        clientSecret: "quick-start-secret",
        host: "[https://your-server.com/](https://your-server.com/)",
        deviceId: "qs-iOS",
        debug: true,
        compatibilityTag: "RankerV1",
        online: true
    )

    let result = NimbleNetApi.initialize(config: nimbleNetConfig)
    if result.status {
        print("NimbleNet initialized successfully")
      } else {
        print("NimbleNet initialization failed with error: \(result.error?.localizedDescription ?? "")")
    }
    ```

3. Check SDK Readiness

    Always verify the SDK is ready before making further calls:

    ```swift
    let ready = NimbleNetApi.isReady()
    if (ready.status) {
        print("NimbleNet sdk is ready")
    } else {
        print("NimbleNet sdk is not ready: \(ready.error?.localizedDescription ?? "")")
    }
    ```

4. Record Restaurant Click Events

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

5. Fetch Restaurant Rankings

    Invoke the `rerank_restaurant` workflow to get top restaurants:

    ```swift
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

# DeliteAI

[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](../../LICENSE)
[![Platform](https://img.shields.io/badge/platform-iOS-orange.svg)](https://github.com/NimbleEdge/deliteAI)
[![Languages](https://img.shields.io/badge/language-Swift-orange.svg)](https://github.com/NimbleEdge/deliteAI)


**DeliteAI** is a powerful **on-device AI platform** designed for developers to build **agentic workflows**. It empowers you to deliver secure, privacy-aware, and high-performance AI-native experiences and applications across various platforms and devices, leveraging the power of local AI inference.

## 🚀 Installation

### CocoaPods

DeliteAI is available as a CocoaPod. To integrate it into your Xcode project:

1. Add the DeliteAI source and pod to your **Podfile**:

    ```ruby
    source 'git@github.com:NimbleEdge/deliteAI-iOS-Podspecs.git' # deliteAI source
    source '[https://cdn.cocoapods.org/](https://cdn.cocoapods.org/)' # cocoaPods source

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

---

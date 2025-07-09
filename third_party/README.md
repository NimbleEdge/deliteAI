## Build Dependencies
Executors used in the SDK are downloaded from S3(Bucket: **deliteai**). Following are the steps that were used to create them.

## Onnxruntime

### Android
Version 1.21.0 of onnxruntime is taken from [maven](https://repo1.maven.org/maven2/com/microsoft/onnxruntime/onnxruntime-android/1.21.0/). The aar file is unzipped and the headers and jni sub-folders are uploaded to S3.

### Ios
Follow this [README](https://github.com/NimbleEdge/onnxruntime/tree/ne-v1.21.0-base-full/nimbleedge) to generate the onnxruntime.xcframework.

## Ort Extensions

### Android
Version 0.13.0 is taken from [maven](https://repo1.maven.org/maven2/com/microsoft/onnxruntime/onnxruntime-extensions-android/0.13.0/). The aar is unzipped and the jni sub-folder is uploaded to S3.

### Ios
Follow these steps to get the xcframework.

1. Create a Sample ios app.
2. Add the following line to your `Podfile`:

   ```ruby
   pod 'onnxruntime-extensions-c', '0.7.0'
   ```

2. Run the following command to install the pod:

   ```bash
   pod install
   ```

3. Use the generated binary from the `Pods` directory in your integration.

Note: run Strip -S -x on generated binaries to reduce thier size

### Unix

TODO: Add steps to build the binaries.

## Onnx-genai

### Android

Version 0.7.0 is downloaded from onnxruntime-genai [releases](https://github.com/microsoft/onnxruntime-genai/releases/download/v0.7.0/onnxruntime-genai-android-0.7.0.aar). The aar is then unzipped and headers and jni sub-folders are uploaded to S3.

### Ios
Follow these steps to generate onnxruntime-genai.xcframework
```bash
# Checkout the onnx-genai repository
cd onnx-genai
```
Checkout the tag corresponding to the version you want to build (replace <version-tag> with the actual version tag you want to build):
```bash
git checkout tags/<version-tag>
```
```bash

# Build the Apple framework using the provided script
python3 tools/ci_build/github/apple/build_apple_framework.py \
  --config Release \
  tools/ci_build/github/apple/default_full_ios_framework_build_settings.json
```

## Executorch

### Android

The header files were selected manually by recursively navigating, starting from `runner.h` in executorch [repo](https://github.com/NimbleEdge/executorch).

The binaries are created by following the steps present [here](https://github.com/NimbleEdge/executorch/blob/main/examples/demo-apps/android/LlamaDemo/docs/delegates/xnnpack_README.md#build-aar-library) and then extracting and renaming libexecutorch.so to libexecutorch_jni.so for both x86 and arm architectures.

### Ios

The Executorch_Headers folder is same as the one used in android.

The LLaMARunner.xcframework is created using the following steps.

1. `git clone https://github.com/NimbleEdge/executorch`
2. `./install_executorch.sh --clean`
3. `./scripts/build_apple_frameworks.sh --Release --Debug --coreml --mps --xnnpack --custom --optimized --portable --quantized`
4. Open executorch/examples/demo-apps/apple_ios/LLaMA/LLaMA.xcodeproj in xcode and run the app.
5. Remove portable_kernels and portable_kernels_debug from the backends in LLaMARunner target.
5. `xcodebuild archive -scheme LLaMARunner -destination="iOS" -archivePath /tmp/xcf/ios.xcarchive -derivedDataPath /tmp/iphoneos -sdk iphoneos SKIP_INSTALL=NO BUILD_LIBRARIES_FOR_DISTRIBUTION=YES`
6. `xcodebuild archive -scheme LLaMARunner -destination="iOS Simulator" -archivePath /tmp/xcf/iossimulator.xcarchive -derivedDataPath /tmp/iphoneos -sdk iphonesimulator SKIP_INSTALL=NO BUILD_LIBRARIES_FOR_DISTRIBUTION=YES`
7. `xcodebuild -create-xcframework \ -framework  /private/tmp/xcf/ios.xcarchive/Products/Library/Frameworks/LLaMARunner.framework \ -framework  /private/tmp/xcf/iossimulator.xcarchive/Products/Library/Frameworks/LLaMARunner.framework \ -output /private/tmp/xcf/LLaMARunner.xcframework`
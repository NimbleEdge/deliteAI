#!/bin/sh
set -eExuo pipefail

uname -m
arch

CONFIG="Debug"
BASE_DIR="$(realpath "$(dirname "${BASH_SOURCE[0]}")/../../..")"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --Release)
            CONFIG="Release"
            ;;
        *)
            echo "Unknown parameter passed: $1"
            echo "Usage: $0 [--Release]"
            exit 1
            ;;
    esac
    shift
done

echo "Building NimbleSDK with configuration: $CONFIG"
echo "Base Directory: $BASE_DIR"
# Read config.properties file
PROPERTIES_FILE="$BASE_DIR/config.yml"

cmake_args_common=$(grep -A 3 "common:" $PROPERTIES_FILE | grep "cmake_args" | awk -F ": " '{print $2}' | tr -d '"')
cmake_args_ios=$(grep -A 3 "ios:" $PROPERTIES_FILE | grep "cmake_args" | awk -F ": " '{print $2}' | tr -d '"')

cd $BASE_DIR/coreruntime

cmake -B $BASE_DIR/sdks/ios/script/build_os64 -G Xcode -DCMAKE_TOOLCHAIN_FILE=$BASE_DIR/sdks/ios/script/ios.toolchain.cmake -DPLATFORM=OS64 $cmake_args_common $cmake_args_ios
cmake -B $BASE_DIR/sdks/ios/script/build_simulator -G Xcode -DCMAKE_TOOLCHAIN_FILE=$BASE_DIR/sdks/ios/script/ios.toolchain.cmake -DPLATFORM=SIMULATOR64COMBINED $cmake_args_common $cmake_args_ios

cmake --build "$BASE_DIR/sdks/ios/script/build_os64" --config "$CONFIG" -- CODE_SIGNING_ALLOWED=NO
cmake --build "$BASE_DIR/sdks/ios/script/build_simulator" --config "$CONFIG" -- CODE_SIGNING_ALLOWED=NO

# adding .h file to the nimblenet target
EXPOSED_HEADERS_DIR="$BASE_DIR/sdks/ios/script/exposed_header"
rm -rf "$EXPOSED_HEADERS_DIR"
mkdir -p "$EXPOSED_HEADERS_DIR"

# Define header file locations as an array
HEADER_FILES=(
    "$BASE_DIR/coreruntime/platform/ios/include/client.h"
    "$BASE_DIR/coreruntime/nimblenet/cross_platform/include/executor_structs.h"
    "$BASE_DIR/coreruntime/platform/ios/include/frontend_layer.h"
    "$BASE_DIR/coreruntime/nimblenet/cross_platform/include/nimble_net_util.hpp"
    "$BASE_DIR/coreruntime/nimblenet/nimble_net/include/nimblejson.hpp"
    "$BASE_DIR/coreruntime/nimblenet/nimble_net/include/nimblenet.h"
    # Add more header file paths here, one per line
)

# # Copy header files to the exposed_header directory using a loop
for header_file in "${HEADER_FILES[@]}"; do
    cp "$header_file" "$EXPOSED_HEADERS_DIR"
done

xcodebuild -create-xcframework \
    -library "$BASE_DIR/sdks/ios/script/build_os64/nimblenet/$CONFIG-iphoneos/libnimblenet.a" \
    -headers "$EXPOSED_HEADERS_DIR" \
    -library "$BASE_DIR/sdks/ios/script/build_simulator/nimblenet/$CONFIG-iphonesimulator/libnimblenet.a" \
    -headers "$EXPOSED_HEADERS_DIR" \
    -output nimblenet.xcframework


cp -rf nimblenet.xcframework "$BASE_DIR/sdks/ios/deliteAI/Assets"
rm -rf nimblenet.xcframework

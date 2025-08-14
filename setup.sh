#!/usr/bin/env bash
# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

set -x

# Ensure aws CLI is installed
if ! command -v aws &> /dev/null; then
    echo "aws CLI not found. Attempting to install it..."

    os=$(uname -s | tr '[:upper:]' '[:lower:]')
    arch=$(uname -m)

    if [[ "$os" == "darwin" ]]; then
        if command -v brew &> /dev/null; then
            brew install awscli
        else
            echo "Homebrew not found. Please install Homebrew or manually install AWS CLI from https://docs.aws.amazon.com/cli/latest/userguide/install-cliv2.html"
            exit 1
        fi
    elif [[ "$os" == "linux" ]]; then
        tmp_dir=$(mktemp -d)
        cd "$tmp_dir" || exit 1
        curl "https://awscli.amazonaws.com/awscli-exe-linux-$arch.zip" -o "awscliv2.zip"
        unzip awscliv2.zip
        sudo ./aws/install
        cd - || exit 1
        rm -rf "$tmp_dir"
    else
        echo "Unsupported OS: $os. Please install AWS CLI manually from https://docs.aws.amazon.com/cli/latest/userguide/install-cliv2.html"
        exit 1
    fi

    # Recheck after attempted install
    if ! command -v aws &> /dev/null; then
        echo "Failed to install aws CLI. Exiting..."
        exit 1
    else
        echo "aws CLI installed successfully."
    fi
fi

arch=$(uname -p)
if [ "$arch" = "unknown" ]; then
    arch=$(uname -m)
fi

installGTest=0
sdk="android"
download_onnx=0
download_onnxgenai=0
download_executorch=0
download_ort_extensions=0

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -s|--sdk)
            sdk="$2"
            if [[ "$sdk" != "android" && "$sdk" != "ios" && "$sdk" != "python" ]]; then
                echo "Invalid sdk input. Expected android, ios, or python. Exiting..."
                exit 1
            fi
            shift 2
            ;;
        --onnx)
            download_onnx=1
            shift
            ;;
        --onnxgenai)
            download_onnxgenai=1
            shift
            ;;
        --executorch)
            download_executorch=1
            shift
            ;;
        --ort_extensions)
            download_ort_extensions=1
            shift
            ;;
        -g|--gtest)
            installGTest=1
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# If no executors flags are set, enable all
if [[ $download_onnx -eq 0 && $download_onnxgenai -eq 0 && $download_executorch -eq 0 && $download_ort_extensions -eq 0 ]]; then
    download_onnx=1
    download_onnxgenai=1
    download_executorch=1
    download_ort_extensions=1
fi

version="1.21.0"
onnxruntime_genai_version="0.7.0"

if [[ "$sdk" == "python" ]]; then
    if [[ $download_onnx -eq 1 ]]; then
        if [[ $arch == "arm" ]]; then
            wget https://github.com/microsoft/onnxruntime/releases/download/v$version/onnxruntime-osx-arm64-$version.tgz
            tar -xvf onnxruntime-osx-arm64-$version.tgz
            mkdir -p third_party/runtime/onnx/unix
            mv onnxruntime-osx-arm64-$version/* third_party/runtime/onnx/unix/
            rm -f onnxruntime-osx-arm64-$version.tgz
            rm -rf onnxruntime-osx-arm64-$version
        elif [[ $arch == "x86_64" ]]; then
            folderName="onnxruntime-linux-x64-$version"
            wget https://github.com/microsoft/onnxruntime/releases/download/v$version/$folderName.tgz
            tar -xvf "$folderName.tgz"
            mkdir -p third_party/runtime/onnx/unix
            mv "$folderName"/* third_party/runtime/onnx/unix/
            rm -f "$folderName".tgz
            rm -rf "$folderName"
        else
            echo "Unsupported architecture: $arch"
            exit 1
        fi
    fi

    if [[ $download_ort_extensions -eq 1 ]]; then
        if [[ $arch == "arm" ]]; then
            wget -O third_party/runtime/onnx/unix/lib/ort_extensions.zip "https://deliteai.s3.amazonaws.com/build-dependencies/ort-extensions-0.13.0/unix/ort_extensions_0.13.0_macos.zip"
        else
            wget -O third_party/runtime/onnx/unix/lib/ort_extensions.zip "https://deliteai.s3.amazonaws.com/build-dependencies/ort-extensions-0.13.0/unix/ort_extensions_0.13.0_x86.zip"
        fi
        unzip -o third_party/runtime/onnx/unix/lib/ort_extensions.zip -d third_party/runtime/onnx/unix/lib/
        rm -f third_party/runtime/onnx/unix/lib/ort_extensions.zip
    fi

    if [[ $download_onnxgenai -eq 1 ]]; then
        if [[ $arch == "arm" ]]; then
            folder="onnxruntime-genai-$onnxruntime_genai_version-osx-arm64"
        else
            folder="onnxruntime-genai-$onnxruntime_genai_version-linux-x64"
        fi
        wget https://github.com/microsoft/onnxruntime-genai/releases/download/v$onnxruntime_genai_version/$folder.tar.gz
        tar -xvf $folder.tar.gz
        mkdir -p third_party/runtime/onnxgenai/unix
        mv $folder/* third_party/runtime/onnxgenai/unix/
        rm -f $folder.tar.gz
        rm -rf $folder
    fi

    if [[ $enable_executorch -eq 1 ]]; then
        echo "Executorch not supported for Python SDK. Exiting..."
        exit 1
    fi

elif [[ "$sdk" == "android" ]]; then
    if [[ $download_ort_extensions -eq 1 ]]; then
        mkdir -p third_party/runtime/onnx/android/0_13_0_ort_extensions
        aws s3 cp s3://deliteai/build-dependencies/ort-extensions-0.13.0/android third_party/runtime/onnx/android/0_13_0_ort_extensions/ --recursive --no-sign-request
    fi

    if [[ $download_onnx -eq 1 ]]; then
        mkdir -p third_party/runtime/onnx/android/1_21_0_full
        aws s3 cp s3://deliteai/build-dependencies/onnx-1.21.0/android third_party/runtime/onnx/android/1_21_0_full --recursive --no-sign-request
    fi

    if [[ $download_onnxgenai -eq 1 ]]; then
        mkdir -p third_party/runtime/onnx/android/0_7_0_onnx_genai
        aws s3 cp s3://deliteai/build-dependencies/onnxgenai-0.7.0/android third_party/runtime/onnx/android/0_7_0_onnx_genai --recursive --no-sign-request
    fi

    if [[ $download_executorch -eq 1 ]]; then
        mkdir -p third_party/runtime/executorch/android/d4906e21_executorch
        aws s3 cp s3://deliteai/build-dependencies/executorch-d4906e21/android third_party/runtime/executorch/android/d4906e21_executorch --recursive --no-sign-request
    fi

elif [[ "$sdk" == "ios" ]]; then
     if [[ $download_ort_extensions -eq 1 ]]; then
        mkdir -p third_party/runtime/onnx/ios/0_13_0_ort_extension
        aws s3 cp s3://deliteai/build-dependencies/ort-extensions-0.13.0/ios third_party/runtime/onnx/ios/0_13_0_ort_extension/ --recursive --no-sign-request
    fi

    if [[ $download_onnx -eq 1 ]]; then
        mkdir -p third_party/runtime/onnx/ios/1_21_0_ios_full
        aws s3 cp s3://deliteai/build-dependencies/onnx-1.21.0/ios third_party/runtime/onnx/ios/1_21_0_ios_full --recursive --no-sign-request
    fi

    if [[ $download_onnxgenai -eq 1 ]]; then
        mkdir -p third_party/runtime/onnx/ios/0_7_0_ios_onnx-genai
        aws s3 cp s3://deliteai/build-dependencies/onnxgenai-0.7.0/ios third_party/runtime/onnx/ios/0_7_0_ios_onnx-genai --recursive --no-sign-request
    fi

    if [[ $download_executorch -eq 1 ]]; then
        mkdir -p third_party/runtime/executorch/ios/e9c11a40_executorch
        aws s3 cp s3://deliteai/build-dependencies/executorch-e9c11a40/ios third_party/runtime/executorch/ios/e9c11a40_executorch --recursive --no-sign-request
    fi
fi

# install gtest for coreruntime testing
if [[ "$arch" == "arm" && $installGTest -eq 1 ]]; then
    if ! command -v brew &> /dev/null; then
        echo "Homebrew is not installed. Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    brew update
    brew install googletest
    echo "Googletest installed successfully"
fi

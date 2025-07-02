#!/usr/bin/env bash
# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

set -x

arch=`uname -p`
if [ "$arch" = "unknown" ]; then
    arch=`uname -m`
fi

os=`uname -s`
currDir=`pwd`
installGTest=0

input="onnx"
while getopts ":e:g" opt; do
    case $opt in
        e)
            if [ "$OPTARG" == "onnx" ]; then
                input="onnx"
            else
                echo "Invalid executor input. Expected: onnx as an argument. Exiting..."
                exit 1
            fi
        ;;
        g)
            installGTest=1
        ;;
        \?)
            echo "Invalid option: -$OPTARG"
            exit 1
            ;;
    esac
done

if [ "$input" == "onnx" ]; then
    version="1.21.0"
    onnxruntime_genai_version="0.7.0"
    if [ $arch == "arm" ]
    then
        wget https://github.com/microsoft/onnxruntime/releases/download/v$version/onnxruntime-osx-arm64-$version.tgz
        tar -xvf onnxruntime-osx-arm64-$version.tgz
        mkdir -p ../third_party/runtime/onnx/unix
        mv onnxruntime-osx-arm64-$version/* ../third_party/runtime/onnx/unix/
        rm -f onnxruntime-osx-arm64-$version.tgz
        rm -rf onnxruntime-osx-arm64-$version

         ## Dowloading ORT_EXTENSIONS as well
        wget -O ../third_party/runtime/onnx/unix/lib/ort_extensions.zip "https://drive.google.com/uc?export=download&id=1gksCRBvKWAkronzalVjxJ90E65drqeEh"
        unzip -o ../third_party/runtime/onnx/unix/lib/ort_extensions.zip -d ../third_party/runtime/onnx/unix/lib/
        rm -f ../third_party/runtime/onnx/unix/lib/ort_extensions.zip

        # Install onnxruntime_genai
        onnxruntime_genai_foldername="onnxruntime-genai-$onnxruntime_genai_version-osx-arm64"
        wget https://github.com/microsoft/onnxruntime-genai/releases/download/v$onnxruntime_genai_version/$onnxruntime_genai_foldername.tar.gz
        tar -xvf $onnxruntime_genai_foldername.tar.gz
        mkdir -p ../third_party/runtime/onnxgenai/unix
        mv $onnxruntime_genai_foldername/* ../third_party/runtime/onnxgenai/unix/
        rm -f $onnxruntime_genai_foldername.tar.gz
        rm -rf $onnxruntime_genai_foldername

    elif [ $arch == "x86_64" ]
    then
        folderName="onnxruntime-linux-x64-$version"
        wget https://github.com/microsoft/onnxruntime/releases/download/v$version/$folderName.tgz
        tar -xvf "$folderName.tgz"
        mkdir -p ../third_party/runtime/onnx/unix
        mv "$folderName"/* ../third_party/runtime/onnx/unix/
        rm -f "$folderName".tgz
        rm -rf "$folderName"

        ## Dowloading ORT_EXTENSIONS as well
        wget -O ../third_party/runtime/onnx/unix/lib/ort_extensions.zip "https://drive.google.com/uc?export=download&id=1TrA8DAZLwTaxcYAfz5pm9jFPWS8EEmBo"
        unzip -o ../third_party/runtime/onnx/unix/lib/ort_extensions.zip -d ../third_party/runtime/onnx/unix/lib/
        rm -f ../third_party/runtime/onnx/unix/lib/ort_extensions.zip

        # Download onnxruntime_genai
        onnxruntime_genai_foldername="onnxruntime-genai-$onnxruntime_genai_version-linux-x64"
        wget https://github.com/microsoft/onnxruntime-genai/releases/download/v$onnxruntime_genai_version/$onnxruntime_genai_foldername.tar.gz
        tar -xvf $onnxruntime_genai_foldername.tar.gz
        mkdir -p ../third_party/runtime/onnxgenai/unix
        mv $onnxruntime_genai_foldername/* ../third_party/runtime/onnxgenai/unix/
        rm -f $onnxruntime_genai_foldername.tar.gz
        rm -rf $onnxruntime_genai_foldername
    else
        echo "Unsupported architecture: $arch"
        exit 1
    fi
fi

# install gtest for testing
if [ $arch == "arm" ]
then
if [ $installGTest -eq 1 ]
then
    if ! command -v brew &> /dev/null; then
        echo "Homebrew is not installed. Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    else
        echo "Homebrew is already installed."
    fi
    brew update
    brew install googletest
    echo "Googletest installed successfully"
fi
fi
#TODO: Add steps for gtest in x86_64 and other architectures

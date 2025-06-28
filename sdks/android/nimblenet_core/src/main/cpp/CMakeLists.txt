# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
# SPDX-License-Identifier: Apache-2.0

# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)
SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )

# Declares and names the project.

project("nimblenet")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/)
#include_directories(.)

add_subdirectory(coreruntime)
#target_include_directories(nimblenet PRIVATE jni/shadow_classes)
target_sources(nimblenet PRIVATE jni/jni.cpp jni/jni_internal.cpp jni/utils/input_transformers.cpp jni/utils/output_transformers.cpp)

target_include_directories(nimblenet PUBLIC jni/shadow_classes/)
target_link_libraries(nimblenet PUBLIC android log z)

add_subdirectory(dljni)
target_link_libraries(nimblenet PRIVATE dljni)

if(ONNX_EXECUTOR)
    add_library(onnx SHARED IMPORTED)
    set_target_properties(onnx PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/onnx_builds/android/${ONNXBuild}/jni/${ANDROID_ABI}/libonnxruntime.so")
    target_include_directories(onnx INTERFACE "onnx_builds/android/${ONNXBuild}/headers")
    target_link_libraries(nimblenet PUBLIC onnx)

    if(ORT_EXTENSIONS)
        add_library(ort-extensions SHARED IMPORTED)
        set_target_properties(ort-extensions PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/onnx_builds/android/${ONNXext}/jni/${ANDROID_ABI}/libortextensions.so)
        target_link_libraries(nimblenet PUBLIC ort-extensions)
    endif()
endif ()
if (GENAI AND ONNXGENAI_EXECUTOR)
        add_library(onnxgenai SHARED IMPORTED)
        set_target_properties(onnxgenai PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/onnx_builds/android/${ONNXgenai}/jni/${ANDROID_ABI}/libonnxruntime-genai.so)
        target_include_directories(onnxgenai INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/onnx_builds/android/${ONNXgenai}/headers)
        target_link_libraries(nimblenet PUBLIC onnxgenai)
endif ()
if (GENAI AND EXECUTORCH_EXECUTOR)
    add_library(executorch SHARED IMPORTED)
    set_target_properties(executorch PROPERTIES
            IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/executorch_builds/android/${Executorchgenai}/jni/${ANDROID_ABI}/libexecutorch_jni.so
    )
    target_include_directories(executorch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/executorch_builds/android/${Executorchgenai}/headers)
    target_link_libraries(nimblenet PUBLIC executorch)
endif ()
					# Searches for a specified prebuilt library and stores the path as a
					# variable. Because CMake includes system libraries in the search path by
					# default, you only need to specify the name of the public NDK library
					# you want to add. CMake verifies that the library exists before
					# completing its build.

#					add_library(lib_foo SHARED IMPORTED)
#					set_property(TARGET lib_foo PROPERTY IMPORTED_NO_SONAME 1)
#					set_target_properties(lib_foo PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libnimblenet.so)

					# Specifies libraries CMake should link to your target library. You
					# can link multiple libraries, such as libraries you define in this
					# build script, prebuilt third-party libraries, or system libraries.
					#target_link_libraries(golibrary lib_foo log ${PYTORCH_LIBRARY} ${FBJNI_LIBRARY} ${PYTORCH_LINK_DIRS}/libpytorch_jni.so)

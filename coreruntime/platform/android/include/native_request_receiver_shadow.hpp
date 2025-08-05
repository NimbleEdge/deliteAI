/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <string>
#include <cstdarg>

/**
 * @brief JNI shadow class for native request receiver operations.
 *
 * Provides static methods to initialize JNI references and dispatch native requests from the native layer.
 */
class NativeRequestReceiverShadow {
 public:
  /**
   * @brief Initializes JNI references for the NativeRequestReceiver class and its methods/instance.
   *
   * @param env JNI environment pointer.
   */
  static void init(JNIEnv* env);

  /**
   * @brief Dispatches a native request using the Kotlin NativeRequestReceiver class via JNI.
   *
   * @param env JNI environment pointer.
   * @param functionName The name of the function to dispatch.
   * @param argCount Number of arguments.
   * @param ... Variable number of jobject arguments.
   * @return jobject Java object representing the result of the dispatch.
   */
  static jobject dispatch(JNIEnv* env, const std::string& functionName, int argCount, ...);

 private:
  inline static jclass nativeRequestReceiverClass = nullptr; /**< Global reference to NativeRequestReceiver Kotlin class. */
  inline static jmethodID dispatchMethodId = nullptr; /**< Method ID for dispatch. */
  inline static jobject nativeRequestReceiverKotlinInstance = nullptr; /**< Global reference to NativeRequestReceiver Kotlin instance. */
}; 
/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <string>

/**
 * @brief JNI shadow class for networking operations (HTTP requests, file downloads).
 *
 * Provides static methods to initialize JNI references and perform network operations from the native layer.
 */
class NetworkingShadow {
 public:
  /**
   * @brief Initializes JNI references for the Networking class and its methods/instance.
   *
   * @param env JNI environment pointer.
   */
  static void init(JNIEnv* env);

  /**
   * @brief Sends an HTTP request using the Kotlin Networking class via JNI.
   *
   * @param env JNI environment pointer.
   * @param url Target URL as a string.
   * @param requestHeaders HTTP headers as a string.
   * @param requestBody HTTP body as a string.
   * @param requestBodyByte HTTP body as a byte array (optional).
   * @param method HTTP method as a string (e.g., "GET", "POST").
   * @param totalCallTimeoutInSecs Timeout for the call in seconds.
   * @return jobject Java object representing the network response.
   */
  static jobject sendRequest(JNIEnv* env, const std::string& url, const std::string& requestHeaders,
                             const std::string& requestBody, jbyteArray requestBodyByte,
                             const std::string& method, jint totalCallTimeoutInSecs);

  /**
   * @brief Downloads a file using the Kotlin Networking class via JNI.
   *
   * @param env JNI environment pointer.
   * @param url File URL as a string.
   * @param requestHeaders HTTP headers as a string.
   * @param fileName Target file name as a string.
   * @param nimbleSdkDir Directory for storing the file as a string.
   * @return jobject Java object representing the file download state transition.
   */
  static jobject downloadFileThroughDownloadManager(JNIEnv* env, const std::string& url,
                                                    const std::string& requestHeaders,
                                                    const std::string& fileName,
                                                    const std::string& nimbleSdkDir);

 private:
  inline static jclass networkingClass = nullptr; /**< Global reference to Networking Kotlin class. */
  inline static jmethodID sendRequestMethodId = nullptr; /**< Method ID for sendRequest. */
  inline static jmethodID downloadFileMethodId = nullptr; /**< Method ID for downloadFileThroughDownloadManager. */
  inline static jobject networkingKotlinInstance = nullptr; /**< Global reference to Networking Kotlin instance. */
};

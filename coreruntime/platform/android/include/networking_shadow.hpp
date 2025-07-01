/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <string>

class NetworkingShadow {
 public:
  static void init(JNIEnv* env);
  static jobject sendRequest(JNIEnv* env, const std::string& url, const std::string& requestHeaders,
                             const std::string& requestBody, jbyteArray requestBodyByte,
                             const std::string& method, jint totalCallTimeoutInSecs);
  static jobject downloadFileThroughDownloadManager(JNIEnv* env, const std::string& url,
                                                    const std::string& requestHeaders,
                                                    const std::string& fileName,
                                                    const std::string& nimbleSdkDir);

 private:
  inline static jclass networkingClass = nullptr;
  inline static jmethodID sendRequestMethodId = nullptr;
  inline static jmethodID downloadFileMethodId = nullptr;
  inline static jobject networkingKotlinInstance = nullptr;
};

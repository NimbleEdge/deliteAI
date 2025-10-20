/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <string>

class NativeRequestReceiverShadow {
 public:
  static void init(JNIEnv* env);
  static jobject dispatch(JNIEnv* env, const std::string& functionName, int argCount, ...);

 private:
  inline static jclass nativeRequestReceiverClass = nullptr;
  inline static jmethodID dispatchMethodId = nullptr;
  inline static jobject nativeRequestReceiverKotlinInstance = nullptr;
};

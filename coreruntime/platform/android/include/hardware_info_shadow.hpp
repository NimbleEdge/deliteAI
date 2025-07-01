/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <string>

class HardwareInfoShadow {
 public:
  static void init(JNIEnv* env);
  static jstring getStaticDeviceMetrics(JNIEnv* env);

 private:
  inline static jclass hardwareInfoClass = nullptr;
  inline static jmethodID getStaticDeviceMetricsMethodId = nullptr;
  inline static jobject hardwareInfoKotlinInstance = nullptr;
};

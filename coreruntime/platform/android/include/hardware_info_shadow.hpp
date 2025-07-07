/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <string>

/**
 * @brief JNI shadow class for accessing hardware information from the Android layer.
 *
 * Provides static methods to initialize JNI references and fetch device metrics from the Kotlin layer.
 */
class HardwareInfoShadow {
 public:
  /**
   * @brief Initializes JNI references for the HardwareInfo class and its methods/instance.
   *
   * @param env JNI environment pointer.
   */
  static void init(JNIEnv* env);

  /**
   * @brief Calls the Kotlin/Java method to get static device metrics as a string.
   *
   * @param env JNI environment pointer.
   * @return jstring Device metrics as a Java string.
   */
  static jstring getStaticDeviceMetrics(JNIEnv* env);

 private:
  inline static jclass hardwareInfoClass = nullptr; /**< Global reference to HardwareInfo Kotlin class. */
  inline static jmethodID getStaticDeviceMetricsMethodId = nullptr; /**< Method ID for getStaticDeviceMetrics. */
  inline static jobject hardwareInfoKotlinInstance = nullptr; /**< Global reference to HardwareInfo Kotlin instance. */
};

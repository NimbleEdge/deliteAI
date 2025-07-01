/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <stdexcept>

class DependencyContainerShadow {
 public:
  static void init(JNIEnv *env);
  static jobject getNetworkingInstance(JNIEnv *env);
  static jobject getHardwareInfoInstance(JNIEnv *env);
  static jobject getLogsUploadSchedulerInstance(JNIEnv *env);

 private:
  inline static jobject dependencyContainerInstance = nullptr;
  inline static jmethodID getInstanceMethodId = nullptr;

  inline static jclass dependencyContainerClass = nullptr;
  inline static jmethodID getNetworkingMethodId = nullptr;
  inline static jmethodID getHardwareInfoMethodId = nullptr;
  inline static jmethodID getLogsUploadSchedulerMethodId = nullptr;

  static void setDependencyContainerInstance(JNIEnv *env);
  static void setMethodIds(JNIEnv *env);
};

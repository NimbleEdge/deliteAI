/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <stdexcept>

/**
 * @brief JNI shadow class for managing and retrieving singleton dependencies from the Android layer.
 *
 * Provides static methods to initialize, retrieve, and manage JNI references to various service instances.
 */
class DependencyContainerShadow {
 public:
  /**
   * @brief Initializes JNI references for the DependencyContainer class and its methods/instance.
   *
   * @param env JNI environment pointer.
   */
  static void init(JNIEnv *env);

  /**
   * @brief Retrieves the Networking instance from the DependencyContainer via JNI.
   *
   * @param env JNI environment pointer.
   * @return jobject Java Networking instance.
   */
  static jobject getNetworkingInstance(JNIEnv *env);

  /**
   * @brief Retrieves the HardwareInfo instance from the DependencyContainer via JNI.
   *
   * @param env JNI environment pointer.
   * @return jobject Java HardwareInfo instance.
   */
  static jobject getHardwareInfoInstance(JNIEnv *env);

  /**
   * @brief Retrieves the LogsUploadScheduler instance from the DependencyContainer via JNI.
   *
   * @param env JNI environment pointer.
   * @return jobject Java LogsUploadScheduler instance.
   */
  static jobject getLogsUploadSchedulerInstance(JNIEnv *env);

 private:
  inline static jobject dependencyContainerInstance = nullptr; /**< Global reference to DependencyContainer instance. */
  inline static jmethodID getInstanceMethodId = nullptr; /**< Method ID for getInstance. */
  inline static jclass dependencyContainerClass = nullptr; /**< Global reference to DependencyContainer Kotlin class. */
  inline static jmethodID getNetworkingMethodId = nullptr; /**< Method ID for getNetworking. */
  inline static jmethodID getHardwareInfoMethodId = nullptr; /**< Method ID for getHardwareInfo. */
  inline static jmethodID getLogsUploadSchedulerMethodId = nullptr; /**< Method ID for getLogsUploadScheduler. */

  /**
   * @brief Sets the global DependencyContainer instance via JNI.
   *
   * @param env JNI environment pointer.
   */
  static void setDependencyContainerInstance(JNIEnv *env);

  /**
   * @brief Sets the method IDs for the DependencyContainer class via JNI.
   *
   * @param env JNI environment pointer.
   */
  static void setMethodIds(JNIEnv *env);
};

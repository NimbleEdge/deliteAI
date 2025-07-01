/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <stdexcept>
#include <string>

#include "../utils/jni_logger.h"

/**
 * @brief JNI shadow class for scheduling log uploads via Android WorkManager.
 *
 * Provides static methods to initialize JNI references and schedule log uploads from the native layer.
 */
class LogsUploadSchedulerShadow {
 public:
  /**
   * @brief Initializes JNI references for the LogsUploadScheduler class and its methods/instance.
   *
   * @param env JNI environment pointer.
   */
  static void init(JNIEnv *env);

  /**
   * @brief Schedules log upload using the WorkManager via JNI.
   *
   * @param env JNI environment pointer.
   * @param application Android application context object.
   * @param initialDelayInSeconds Initial delay before the first upload (seconds).
   * @param retryIntervalInSecondsIfFailed Retry interval if upload fails (seconds).
   * @param payload JSON string payload for configuration.
   */
  static void schedule(JNIEnv *env, jobject application, jlong initialDelayInSeconds,
                       jlong retryIntervalInSecondsIfFailed, const std::string &payload);

 private:
  inline static jclass logsUploadSchedulerClass = nullptr; /**< Global reference to LogsUploadScheduler Kotlin class. */
  inline static jmethodID scheduleMethodId = nullptr; /**< Method ID for schedule. */
  inline static jobject logsUploadSchedulerKotlinInstance = nullptr; /**< Global reference to LogsUploadScheduler Kotlin instance. */
};

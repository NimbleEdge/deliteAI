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

class LogsUploadSchedulerShadow {
 public:
  static void init(JNIEnv *env);
  static void schedule(JNIEnv *env, jobject application, jlong initialDelayInSeconds,
                       jlong retryIntervalInSecondsIfFailed, const std::string &payload);

 private:
  inline static jclass logsUploadSchedulerClass = nullptr;
  inline static jmethodID scheduleMethodId = nullptr;
  inline static jobject logsUploadSchedulerKotlinInstance = nullptr;
};

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <string>

#include "nimble_net_util.hpp"

class GeminiNanoHandlerShadow {
 private:
  jclass geminiNanoHandlerClass;
  jmethodID initializeMethodId;
  jmethodID promptMethodId;
  jmethodID cancelMethodId;
  jmethodID getStatusMethodId;

 public:
  explicit GeminiNanoHandlerShadow(JNIEnv *env);

  void initialize(JNIEnv *env, jobject context);
  void prompt(JNIEnv *env, const std::string &prompt);
  void cancel(JNIEnv *env);
  FileDownloadStatus getStatus(JNIEnv *env);
};

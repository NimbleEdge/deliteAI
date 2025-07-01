/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <string>

#include "nimble_net_util.hpp"

/**
 * @brief JNI shadow class for interacting with Gemini Nano handler in the Android layer.
 *
 * Provides methods to initialize, prompt, cancel, and get status from the Gemini Nano handler via JNI.
 */
class GeminiNanoHandlerShadow {
 private:
  jclass geminiNanoHandlerClass; /**< Global reference to GeminiNanoHandler Kotlin class. */
  jmethodID initializeMethodId;  /**< Method ID for initialize. */
  jmethodID promptMethodId;      /**< Method ID for prompt. */
  jmethodID cancelMethodId;      /**< Method ID for cancel. */
  jmethodID getStatusMethodId;   /**< Method ID for getStatus. */

 public:
  /**
   * @brief Constructs the GeminiNanoHandlerShadow and initializes JNI references.
   *
   * @param env JNI environment pointer.
   */
  explicit GeminiNanoHandlerShadow(JNIEnv *env);

  /**
   * @brief Initializes the Gemini Nano handler with the given context.
   *
   * @param env JNI environment pointer.
   * @param context Android context object.
   */
  void initialize(JNIEnv *env, jobject context);

  /**
   * @brief Sends a prompt to the Gemini Nano handler.
   *
   * @param env JNI environment pointer.
   * @param prompt Prompt string to send.
   */
  void prompt(JNIEnv *env, const std::string &prompt);

  /**
   * @brief Cancels the current Gemini Nano operation.
   *
   * @param env JNI environment pointer.
   */
  void cancel(JNIEnv *env);

  /**
   * @brief Gets the current status of the Gemini Nano handler.
   *
   * @param env JNI environment pointer.
   * @return FileDownloadStatus Enum value representing the download status.
   */
  FileDownloadStatus getStatus(JNIEnv *env);
};

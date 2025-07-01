/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android/log.h>

#include <string>

#include "dependency_container_shadow.hpp"
#include "executor_structs.h"
#include "gemini_nano_handler_shadow.hpp"
#include "jni.h"
#include "networking_shadow.hpp"
#include "nimble_net_util.hpp"

#pragma GCC visibility push(default)

extern JavaVM* globalJvm;
extern std::string androidClass;
extern jobject gClassLoader;
extern jmethodID gFindClassMethod;
extern jobject context;
extern thread_local JNIEnv *_threadLocalEnv;
#ifdef GEMINI
inline GeminiNanoHandlerShadow geminiNanoHandlerShadow(nullptr);
#endif  // GEMINI

CNetworkResponse send_request(const char* body, const char* headers, const char* url,
                              const char* method, int length);

FileDownloadInfo download_to_file_async(const char* url, const char* headers, const char* fileName,
                                        const char* nimbleSdkDir);

static inline void log_verbose(const char* message) {
  __android_log_print(ANDROID_LOG_VERBOSE, "NIMBLE-CORE", "%s", message);
}

static inline void log_debug(const char* message) {
  __android_log_print(ANDROID_LOG_DEBUG, "NIMBLE-CORE", "%s", message);
}

static inline void log_info(const char* message) {
  __android_log_print(ANDROID_LOG_INFO, "NIMBLE-CORE", "%s", message);
}

static inline void log_warn(const char* message) {
  __android_log_print(ANDROID_LOG_WARN, "NIMBLE-CORE", "%s", message);
}

static inline void log_error(const char* message) {
  __android_log_print(ANDROID_LOG_ERROR, "NIMBLE-CORE", "%s", message);
}

static inline void log_fatal(const char* message) {
  __android_log_print(ANDROID_LOG_FATAL, "NIMBLE-CORE", "%s", message);
}

CNetworkResponse emptyResponse();

FileDownloadInfo emptyFileDownloadInfo();

char *get_hardware_info();

#ifdef GEMINI
void initializeGemini();
FileDownloadStatus getGeminiStatus();
#endif  // GEMINI

bool set_thread_priority_min();

bool set_thread_priority_max();

bool schedule_logs_upload(long repeatIntervalInMinutes, long retryIntervalInMinutesIfFailed,
                          const char* workManagerConfigJsonChar);

bool deallocate_frontend_tensors(CTensors cTensors);

bool free_frontend_function_context(void* context);

#pragma GCC visibility pop

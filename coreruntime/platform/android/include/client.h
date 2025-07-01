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

extern JavaVM* globalJvm;           /**< Global Java VM pointer for JNI operations. */
extern std::string androidClass;    /**< Android class name as a string. */
extern jobject gClassLoader;        /**< Global class loader object for JNI. */
extern jmethodID gFindClassMethod;  /**< Method ID for finding classes via JNI. */
extern jobject context;             /**< Android application context object. */
extern thread_local JNIEnv *_threadLocalEnv; /**< Thread-local JNI environment pointer. */
#ifdef GEMINI
inline GeminiNanoHandlerShadow geminiNanoHandlerShadow(nullptr); /**< Gemini Nano handler shadow instance. */
#endif  // GEMINI

/**
 * @brief Sends a network request using JNI and returns the response.
 *
 * @param body Request body as a C-string.
 * @param headers Request headers as a C-string.
 * @param url Target URL as a C-string.
 * @param method HTTP method as a C-string (e.g., "GET", "POST").
 * @param length Length of the body, or -1 if not applicable.
 * @return CNetworkResponse Struct containing status code, headers, and body.
 */
CNetworkResponse send_request(const char* body, const char* headers, const char* url,
                              const char* method, int length);

/**
 * @brief Downloads a file asynchronously using JNI and returns download info.
 *
 * @param url File URL as a C-string.
 * @param headers Request headers as a C-string.
 * @param fileName Target file name as a C-string.
 * @param nimbleSdkDir Directory for storing the file as a C-string.
 * @return FileDownloadInfo Struct containing download status and metadata.
 */
FileDownloadInfo download_to_file_async(const char* url, const char* headers, const char* fileName,
                                        const char* nimbleSdkDir);

/**
 * @brief Logs a verbose message to Android logcat.
 *
 * @param message Message to log.
 */
static inline void log_verbose(const char* message) {
  __android_log_print(ANDROID_LOG_VERBOSE, "NIMBLE-CORE", "%s", message);
}

/**
 * @brief Logs a debug message to Android logcat.
 *
 * @param message Message to log.
 */
static inline void log_debug(const char* message) {
  __android_log_print(ANDROID_LOG_DEBUG, "NIMBLE-CORE", "%s", message);
}

/**
 * @brief Logs an info message to Android logcat.
 *
 * @param message Message to log.
 */
static inline void log_info(const char* message) {
  __android_log_print(ANDROID_LOG_INFO, "NIMBLE-CORE", "%s", message);
}

/**
 * @brief Logs a warning message to Android logcat.
 *
 * @param message Message to log.
 */
static inline void log_warn(const char* message) {
  __android_log_print(ANDROID_LOG_WARN, "NIMBLE-CORE", "%s", message);
}

/**
 * @brief Logs an error message to Android logcat.
 *
 * @param message Message to log.
 */
static inline void log_error(const char* message) {
  __android_log_print(ANDROID_LOG_ERROR, "NIMBLE-CORE", "%s", message);
}

/**
 * @brief Logs a fatal message to Android logcat.
 *
 * @param message Message to log.
 */
static inline void log_fatal(const char* message) {
  __android_log_print(ANDROID_LOG_FATAL, "NIMBLE-CORE", "%s", message);
}

/**
 * @brief Returns an empty CNetworkResponse with error code and null pointers.
 *
 * @return CNetworkResponse Empty response struct.
 */
CNetworkResponse emptyResponse();

/**
 * @brief Returns an empty FileDownloadInfo with unknown status and -1 time elapsed.
 *
 * @return FileDownloadInfo Empty file download info struct.
 */
FileDownloadInfo emptyFileDownloadInfo();

/**
 * @brief Retrieves hardware information as a JSON string using JNI.
 *
 * @return char* Pointer to a null-terminated JSON string.
 */
char *get_hardware_info();

#ifdef GEMINI
/**
 * @brief Initializes the Gemini Nano handler via JNI.
 */
void initializeGemini();

/**
 * @brief Gets the current Gemini Nano download status via JNI.
 * @return FileDownloadStatus Enum value representing the download status.
 */
FileDownloadStatus getGeminiStatus();
#endif  // GEMINI

/**
 * @brief Sets the current thread's priority to minimum using JNI.
 * @return bool True if successful, false otherwise.
 */
bool set_thread_priority_min();

/**
 * @brief Sets the current thread's priority to maximum using JNI.
 * @return bool True if successful, false otherwise.
 */
bool set_thread_priority_max();

/**
 * @brief Schedules periodic logs upload using Android WorkManager via JNI.
 *
 * @param repeatIntervalInMinutes Interval in minutes between uploads.
 * @param retryIntervalInMinutesIfFailed Retry interval in minutes if upload fails.
 * @param workManagerConfigJsonChar JSON string for WorkManager configuration.
 * @return bool True if scheduling was successful, false otherwise.
 */
bool schedule_logs_upload(long repeatIntervalInMinutes, long retryIntervalInMinutesIfFailed,
                          const char* workManagerConfigJsonChar);

/**
 * @brief Deallocates memory for frontend tensors. (No-op on Android)
 *
 * @param cTensors Struct containing tensors to deallocate.
 * @return bool Always returns true.
 */
bool deallocate_frontend_tensors(CTensors cTensors);

/**
 * @brief Frees the memory for a frontend function context. (No-op on Android)
 *
 * @param context Pointer to the context to free.
 * @return bool Always returns true.
 */
bool free_frontend_function_context(void* context);

#pragma GCC visibility pop

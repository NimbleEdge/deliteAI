/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "executor_structs.h"
#include "nimble_net_util.hpp"

#include "frontend_layer.h"

/**
 * @brief Function pointer type for sending a network request from iOS.
 *
 * @param body Request body as a C-string.
 * @param headers Request headers as a C-string.
 * @param url Target URL as a C-string.
 * @param method HTTP method as a C-string (e.g., "GET", "POST").
 * @param length Length of the body, or -1 if not applicable.
 * @return CNetworkResponse Struct containing status code, headers, and body.
 */
typedef CNetworkResponse (*send_request_type)(const char *body, const char *headers,
                                              const char *url, const char *method, int length);
/**
 * @brief Function pointer type for logging a verbose message.
 *
 * @param message Message to log.
 */
typedef void (*log_verbose_type)(const char *message);
/**
 * @brief Function pointer type for logging a debug message.
 *
 * @param message Message to log.
 */
typedef void (*log_debug_type)(const char *message);
/**
 * @brief Function pointer type for logging an info message.
 *
 * @param message Message to log.
 */
typedef void (*log_info_type)(const char *message);
/**
 * @brief Function pointer type for logging a warning message.
 *
 * @param message Message to log.
 */
typedef void (*log_warn_type)(const char *message);
/**
 * @brief Function pointer type for logging an error message.
 *
 * @param message Message to log.
 */
typedef void (*log_error_type)(const char *message);
/**
 * @brief Function pointer type for logging a fatal message.
 *
 * @param message Message to log.
 */
typedef void (*log_fatal_type)(const char *message);
/**
 * @brief Function pointer type for getting hardware information as a JSON string.
 *
 * @return char* Pointer to a null-terminated JSON string.
 */
typedef char *(*get_hardware_info_type)();
/**
 * @brief Function pointer type for downloading a model file asynchronously.
 *
 * @param url File URL as a C-string.
 * @param headers Request headers as a C-string.
 * @param fileName Target file name as a C-string.
 * @param nimbleSdkDir Directory for storing the file as a C-string.
 * @return FileDownloadInfo Struct containing download status and metadata.
 */
typedef struct FileDownloadInfo (*download_model_type)(const char *url, const char *headers,
                                                       const char *fileName,
                                                       const char *nimbleSdkDir);
/**
 * @brief Function pointer type for setting the current thread's priority to minimum.
 *
 * @return bool True if successful, false otherwise.
 */
typedef bool (*set_thread_priority_min_type)();
/**
 * @brief Function pointer type for setting the current thread's priority to maximum.
 *
 * @return bool True if successful, false otherwise.
 */
typedef bool (*set_thread_priority_max_type)();

/**
 * @brief Function pointer type for getting phonemes.
 *
 * @param text text to convert.
 *
 * @return char* Pointer to phoneme string.
 */
typedef char* (*get_phonemes_type)(const char* text);

extern send_request_type send_request_global; /**< Global function pointer for sending requests. */
extern log_verbose_type log_verbose_global; /**< Global function pointer for verbose logging. */
extern log_debug_type log_debug_global; /**< Global function pointer for debug logging. */
extern log_info_type log_info_global; /**< Global function pointer for info logging. */
extern log_warn_type log_warn_global; /**< Global function pointer for warning logging. */
extern log_error_type log_error_global; /**< Global function pointer for error logging. */
extern log_fatal_type log_fatal_global; /**< Global function pointer for fatal logging. */
extern get_hardware_info_type get_hardware_info_global; /**< Global function pointer for hardware info. */
extern download_model_type download_model_global; /**< Global function pointer for model download. */
extern set_thread_priority_min_type set_thread_priority_min_global; /**< Global function pointer for min thread priority. */
extern set_thread_priority_max_type set_thread_priority_max_global; /**< Global function pointer for max thread priority. */
extern get_phonemes_type get_phonemes_global; /**< Global function pointer for getting phonemes. */

/**
 * @brief Logs a verbose message to the iOS log system.
 *
 * @param message Message to log.
 */
void log_verbose(const char *message);
/**
 * @brief Logs a debug message to the iOS log system.
 *
 * @param message Message to log.
 */
void log_debug(const char *message);
/**
 * @brief Logs an info message to the iOS log system.
 *
 * @param message Message to log.
 */
void log_info(const char *message);
/**
 * @brief Logs a warning message to the iOS log system.
 *
 * @param message Message to log.
 */
void log_warn(const char *message);
/**
 * @brief Logs an error message to the iOS log system.
 *
 * @param message Message to log.
 */
void log_error(const char *message);
/**
 * @brief Logs a fatal message to the iOS log system.
 *
 * @param message Message to log.
 */
void log_fatal(const char *message);
/**
 * @brief Retrieves hardware information as a JSON string.
 *
 * @return char* Pointer to a null-terminated JSON string.
 */
char *get_hardware_info();
/**
 * @brief Sets the current thread's priority to minimum.
 *
 * @return bool True if successful, false otherwise.
 */
bool set_thread_priority_min();
/**
 * @brief Sets the current thread's priority to maximum.
 *
 * @return bool True if successful, false otherwise.
 */
bool set_thread_priority_max();
/**
 * @brief Schedules periodic logs upload using iOS background tasks.
 *
 * @param repeatIntervalInMinutes Interval in minutes between uploads.
 * @param retryIntervalInMinutesIfFailed Retry interval in minutes if upload fails.
 * @param workManagerConfigJsonChar JSON string for configuration.
 * @return bool True if scheduling was successful, false otherwise.
 */
bool schedule_logs_upload(long repeatIntervalInMinutes, long retryIntervalInMinutesIfFailed,
                          const char *workManagerConfigJsonChar);
/**
 * @brief Sends a network request and returns the response.
 *
 * @param body Request body as a C-string.
 * @param headers Request headers as a C-string.
 * @param url Target URL as a C-string.
 * @param method HTTP method as a C-string (e.g., "GET", "POST").
 * @param length Length of the body, or -1 if not applicable.
 * @return CNetworkResponse Struct containing status code, headers, and body.
 */
CNetworkResponse send_request(const char *body, const char *headers, const char *url,
                              const char *method, int length);
/**
 * @brief Returns an empty CNetworkResponse with error code and null pointers.
 *
 * @return CNetworkResponse Empty response struct.
 */
CNetworkResponse emptyResponse();
/**
 * @brief Downloads a file asynchronously and returns download info.
 *
 * @param url File URL as a C-string.
 * @param headers Request headers as a C-string.
 * @param fileName Target file name as a C-string.
 * @param nimbleSdkDir Directory for storing the file as a C-string.
 * @return FileDownloadInfo Struct containing download status and metadata.
 */
struct FileDownloadInfo download_to_file_async(const char *url, const char *headers,
                                               const char *fileName, const char *nimbleSdkDir);

/**
 * @brief Return phonemes of the given string.
 *
 * @return char* Pointer to phoneme string.
 */
char* get_phonemes(const char* text);

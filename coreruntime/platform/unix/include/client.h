/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include "executor_structs.h"
#include "nimble_net_util.hpp"
#include "nimble_net/c_tensor.h"

/**
 * @brief Sends a network request using libcurl and returns the response.
 *
 * @param body Request body as a C-string.
 * @param headers Request headers as a C-string (JSON format).
 * @param url Target URL as a C-string.
 * @param method HTTP method as a C-string (e.g., "GET", "POST").
 * @param length Length of the body, or -1 if not applicable.
 * @return CNetworkResponse Struct containing status code, headers, and body.
 */
CNetworkResponse send_request(const char *body, const char *headers, const char *url,
                              const char *method, int length);

/**
 * @brief Logs a verbose message to stderr.
 *
 * @param message Message to log.
 */
static inline void log_verbose(const char *message) { fprintf(stderr, "VERBOSE:%s\n", message); }

/**
 * @brief Logs a debug message to stderr.
 *
 * @param message Message to log.
 */
static inline void log_debug(const char *message) { fprintf(stderr, "DEBUG:%s\n", message); }

/**
 * @brief Logs an info message to stderr.
 *
 * @param message Message to log.
 */
static inline void log_info(const char *message) { fprintf(stderr, "INFO:%s\n", message); }

/**
 * @brief Logs a warning message to stderr.
 *
 * @param message Message to log.
 */
static inline void log_warn(const char *message) { fprintf(stderr, "WARN:%s\n", message); }

/**
 * @brief Logs an error message to stderr.
 *
 * @param message Message to log.
 */
static inline void log_error(const char *message) { fprintf(stderr, "ERROR:%s\n", message); }

/**
 * @brief Logs a fatal message to stderr.
 *
 * @param message Message to log.
 */
static inline void log_fatal(const char *message) { fprintf(stderr, "FATAL:%s\n", message); }

/**
 * @brief Returns a static hardware info JSON string (simulated for Unix).
 *
 * @return char* Pointer to a statically allocated JSON string.
 */
static inline char *get_hardware_info() {
  return strdup(R"(
  {
    "deviceBrand": "Samsung",
    "deviceModel": "Galaxy S24",
    "chipset": "Exynos 990",
    "numCores": "8",
    "totalRamInMB": "128000"
  }
  )");
}

/**
 * @brief Downloads a file asynchronously and returns download info.
 *
 * @param url File URL as a C-string.
 * @param headers Request headers as a C-string.
 * @param fileName Target file name as a C-string.
 * @param nimbleSdkDir Directory for storing the file as a C-string.
 * @return FileDownloadInfo Struct containing download status and metadata.
 */
FileDownloadInfo download_to_file_async(const char *url, const char *headers, const char *fileName,
                                        const char *nimbleSdkDir);

/**
 * @brief Sets the current thread's priority to minimum (no-op on Unix).
 *
 * @return bool Always returns true.
 */
static inline bool set_thread_priority_min() { return true; }

/**
 * @brief Sets the current thread's priority to maximum (no-op on Unix).
 *
 * @return bool Always returns true.
 */
static inline bool set_thread_priority_max() { return true; }

/**
 * @brief Schedules periodic logs upload (no-op on Unix).
 *
 * @param repeatIntervalInMinutes Interval in minutes between uploads.
 * @param retryIntervalInMinutesIfFailed Retry interval in minutes if upload fails.
 * @param workManagerConfigJsonChar JSON string for configuration.
 * @return bool Always returns true.
 */
static inline bool schedule_logs_upload(long repeatIntervalInMinutes,
                                        long retryIntervalInMinutesIfFailed,
                                        const char *workManagerConfigJsonChar) {
  return true;
}

/**
 * @brief Function pointer type for deallocating frontend tensors.
 *
 * @param cTensors Struct containing tensors to deallocate.
 * @return bool True if successful.
 */
typedef bool (*DeallocateFrontendType)(CTensors cTensors);

extern DeallocateFrontendType globalDeallocate; /**< Global function pointer for tensor deallocation. */

/**
 * @brief Deallocates memory for frontend tensors (Unix implementation).
 *
 * @param cTensors Struct containing tensors to deallocate.
 * @return bool True if successful.
 */
bool deallocate_frontend_tensors(CTensors cTensors);

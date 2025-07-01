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

CNetworkResponse send_request(const char *body, const char *headers, const char *url,
                              const char *method, int length);

static inline void log_verbose(const char *message) { fprintf(stderr, "VERBOSE:%s\n", message); }

static inline void log_debug(const char *message) { fprintf(stderr, "DEBUG:%s\n", message); }

static inline void log_info(const char *message) { fprintf(stderr, "INFO:%s\n", message); }

static inline void log_warn(const char *message) { fprintf(stderr, "WARN:%s\n", message); }

static inline void log_error(const char *message) { fprintf(stderr, "ERROR:%s\n", message); }

static inline void log_fatal(const char *message) { fprintf(stderr, "FATAL:%s\n", message); }

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

FileDownloadInfo download_to_file_async(const char *url, const char *headers, const char *fileName,
                                        const char *nimbleSdkDir);

static inline bool set_thread_priority_min() { return true; }

static inline bool set_thread_priority_max() { return true; }

static inline bool schedule_logs_upload(long repeatIntervalInMinutes,
                                        long retryIntervalInMinutesIfFailed,
                                        const char *workManagerConfigJsonChar) {
  return true;
}

typedef bool (*DeallocateFrontendType)(CTensors cTensors);
typedef bool (*FreeFrontendContextType)(void *context);

extern DeallocateFrontendType globalDeallocate;
extern FreeFrontendContextType globalFrontendContextFree;

bool deallocate_frontend_tensors(CTensors cTensors);

bool free_frontend_function_context(void *context);

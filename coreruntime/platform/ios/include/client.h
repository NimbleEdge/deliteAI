/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "executor_structs.h"
#include "nimble_net_util.hpp"

#include "frontend_layer.h"

#pragma once

typedef CNetworkResponse (*send_request_type)(const char *body, const char *headers,
                                              const char *url, const char *method, int length);
typedef void (*log_verbose_type)(const char *message);
typedef void (*log_debug_type)(const char *message);
typedef void (*log_info_type)(const char *message);
typedef void (*log_warn_type)(const char *message);
typedef void (*log_error_type)(const char *message);
typedef void (*log_fatal_type)(const char *message);
typedef char *(*get_hardware_info_type)();
typedef struct FileDownloadInfo (*download_model_type)(const char *url, const char *headers,
                                                       const char *fileName,
                                                       const char *nimbleSdkDir);
typedef bool (*set_thread_priority_min_type)();
typedef bool (*set_thread_priority_max_type)();

extern send_request_type send_request_global;
extern log_verbose_type log_verbose_global;
extern log_debug_type log_debug_global;
extern log_info_type log_info_global;
extern log_warn_type log_warn_global;
extern log_error_type log_error_global;
extern log_fatal_type log_fatal_global;
extern get_hardware_info_type get_hardware_info_global;
extern download_model_type download_model_global;
extern set_thread_priority_min_type set_thread_priority_min_global;
extern set_thread_priority_max_type set_thread_priority_max_global;

void log_verbose(const char *message);
void log_debug(const char *message);
void log_info(const char *message);
void log_warn(const char *message);
void log_error(const char *message);
void log_fatal(const char *message);
char *get_hardware_info();
bool set_thread_priority_min();
bool set_thread_priority_max();
bool schedule_logs_upload(long repeatIntervalInMinutes, long retryIntervalInMinutesIfFailed,
                          const char *workManagerConfigJsonChar);
CNetworkResponse send_request(const char *body, const char *headers, const char *url,
                              const char *method, int length);
CNetworkResponse emptyResponse();
struct FileDownloadInfo download_to_file_async(const char *url, const char *headers,
                                               const char *fileName, const char *nimbleSdkDir);

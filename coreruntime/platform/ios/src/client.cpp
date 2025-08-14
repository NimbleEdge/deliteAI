/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "client.h"

#include <string>

#include "nimble_net_util.hpp"

#define SWIFT_EXTERN extern "C"
#define SWIFT_WARN_UNUSED_RESULT __attribute__((warn_unused_result))

send_request_type send_request_global = nullptr;
log_verbose_type log_verbose_global = nullptr;
log_debug_type log_debug_global = nullptr;
log_info_type log_info_global = nullptr;
log_warn_type log_warn_global = nullptr;
log_error_type log_error_global = nullptr;
log_fatal_type log_fatal_global = nullptr;
get_hardware_info_type get_hardware_info_global = nullptr;
download_model_type download_model_global = nullptr;
set_thread_priority_min_type set_thread_priority_min_global = nullptr;
set_thread_priority_max_type set_thread_priority_max_global = nullptr;
get_phonemes_type get_phonemes_global = nullptr;

bool schedule_logs_upload(long repeatIntervalInMinutes, long retryIntervalInMinutesIfFailed,
                          const char *workManagerConfigJsonChar) {
  return true;
}

bool set_thread_priority_min() {
  if (set_thread_priority_min_global != nullptr) {
    return set_thread_priority_min_global();
  }
  return false;
}

bool set_thread_priority_max() {
  if (set_thread_priority_max_global != nullptr) {
    return set_thread_priority_max_global();
  }
  return false;
}

void log_verbose(const char *message) {
  if (log_verbose_global != nullptr) {
    log_verbose_global(message);
  }
}

void log_debug(const char *message) {
  if (log_debug_global != nullptr) {
    log_debug_global(message);
  }
}

void log_info(const char *message) {
  if (log_info_global != nullptr) {
    log_info_global(message);
  }
}

void log_warn(const char *message) {
  if (log_warn_global != nullptr) {
    log_warn_global(message);
  }
}

void log_error(const char *message) {
  if (log_error_global != nullptr) {
    log_error_global(message);
  }
}

void log_fatal(const char *message) {
  if (log_fatal_global != nullptr) {
    log_fatal_global(message);
  }
}

char *get_hardware_info() {
  if (get_hardware_info_global != nullptr) {
    return get_hardware_info_global();
  }
  return nullptr;
}

CNetworkResponse emptyResponse() {
  CNetworkResponse response{};
  response.statusCode = EMPTY_ERROR_CODE;
  response.headers = nullptr;
  response.body = nullptr;
  return response;
}

CNetworkResponse send_request(const char *body, const char *headers, const char *url,
                              const char *method, int length) {
  if (send_request_global != nullptr) {
    return send_request_global(body, headers, url, method, length);
  }

  return emptyResponse();
}

FileDownloadInfo download_to_file_async(const char *url, const char *headers, const char *fileName,
                                        const char *nimbleSdkDir) {
  if (download_model_global != nullptr) {
    return download_model_global(url, headers, fileName, nimbleSdkDir);
  }
  FileDownloadInfo f;
  return f;
}

char* get_phonemes(const char* text) {
  if (get_phonemes_global == nullptr) {
    return nullptr;
  }
  return get_phonemes_global(text);
}

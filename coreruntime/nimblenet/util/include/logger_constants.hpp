/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>

namespace loggerconstants {
static inline const int LogTimeIntervalSecs = 5;
static inline const std::string DefaultLogUploadURL = "https://logs.nimbleedge.com/v2";
static inline const int MaxLogFileSizeKB = 1;
static inline const int MetricsCollectionIntervalSecs = 30;
static inline const int MaxBytesInKB = 1024;
static inline const int MaxConcurrentLogFailures = 3;
static inline const std::string MetricDir = "/metrics";
static inline const std::string LogDir = "/logs";
static inline const std::string ExternalLogDir = "/clogs";
static inline const int InferenceMetricLogInterval = 1;
static inline const float LogSendProbability = 1;
static inline const int MaxFilesToSend = 5;
static inline float MaxEventsSizeKBs = 5000;  // 5 MB
}  // namespace loggerconstants

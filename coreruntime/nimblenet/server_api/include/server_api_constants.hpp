/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string>

namespace serverconstants {
static inline const std::string SyncerAPIVersion = "/api/v1";
static inline const std::string ApiVersionV4 = "/api/v4";
static inline const std::string CDNHostIdentifier = "cdn";
static inline const std::string ServiceHostIdentifier = "service";

static inline const int MaxAuthErrorRetries = 1;
static inline const int MaxRegisterRetries = 3;

static inline const std::string ModelService = "/mds";
static inline const std::string TaskService = "/mds";
static inline const std::string AuthInfoFile = "AuthInfo.txt";
}  // namespace serverconstants

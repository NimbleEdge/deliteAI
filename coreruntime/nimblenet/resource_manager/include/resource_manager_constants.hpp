/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>

namespace rmconstants {
static inline const std::string InferenceMetadataFileName = "inferencePlanData.txt";
static inline const std::string InferenceFileName = "inferencePlan.txt";
static inline const std::string TaskDataFileName = "task.txt";
static inline const std::string DocumentDataFileName = "jsonDocument.txt";
static inline const std::string LLMFolderName = "llm";
static inline const std::string ORTFileType = "ort";
static inline const std::string TARFileType = "tar";
static inline const std::string MobileBenchmarksAssetName = "MobileBenchmarks";
static inline const std::string GeminiModelName = "gemini:nano:on-device";

static inline const int LoadResourceRetries = 3;
}  // namespace rmconstants

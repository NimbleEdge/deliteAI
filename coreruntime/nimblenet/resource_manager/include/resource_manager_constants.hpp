/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>

/**
 * @namespace rmconstants
 * @brief Contains constants used by the Resource Manager.
 *
 * These constants define standardized file names, folder names, model identifiers, and retry limits
 * used throughout the inference and asset/resource handling subsystems.
 */
namespace rmconstants {

// Suffix to be addded to the file name for storing ML model plan which contains the input, output
// and processor names.
static inline const std::string InferenceMetadataFileName = "inferencePlanData.txt";

// Suffix to be addded to the file name for storing ML model.
static inline const std::string InferenceFileName = "inferencePlan.txt";

// Suffix to be addded to the file name storing delitepy script.
static inline const std::string TaskDataFileName = "task.txt";

// Suffix to be addded to the file name for storing document-type data in JSON format.
static inline const std::string DocumentDataFileName = "jsonDocument.txt";

// Suffix to be addded to the subdirectory name used to store LLM-related assets.
static inline const std::string LLMFolderName = "llm";

// Logical name or identifier for benchmarking assets.
static inline const std::string MobileBenchmarksAssetName = "MobileBenchmarks";

// Internal identifier for the default on-device Gemini model.
static inline const std::string GeminiModelName = "gemini:nano:on-device";

// Number of times to retry loading a resource before failing.
static inline const int LoadResourceRetries = 3;
}  // namespace rmconstants

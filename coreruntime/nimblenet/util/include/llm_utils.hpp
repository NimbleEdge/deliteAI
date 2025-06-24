/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstring>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "command_center.hpp"
#include "data_variable.hpp"

/**
 * @brief Utilities for LLM selection based on device capabilities.
 */
namespace llmutil {

/**
 * @brief Provider of the LLM - OS or CUSTOM.
 */
struct Provider {
  static constexpr const char* OS = "os";      /**< OS provider string. */
  static constexpr const char* CUSTOM = "custom"; /**< Custom provider string. */
};

/**
 * @brief Enum for device performance tiers.
 */
enum class DeviceTier { ONE, TWO, UNSUPPORTED };

/**
 * @brief Configuration for device tiers and benchmarks.
 */
struct DeviceTierConfig {
  /**
   * @brief Tier requirements for a device tier.
   */
  struct Tier {
    int min_ram;              /**< Minimum RAM required. */
    int min_num_cores;        /**< Minimum number of CPU cores required. */
    int min_multi_core_score; /**< Minimum multi-core benchmark score required. */
  };

  /**
   * @brief Benchmark information for a device.
   */
  struct Benchmark {
    std::string device;           /**< Device name. */
    std::string chipset;          /**< Chipset name. */
    int multi_core_score;         /**< Multi-core benchmark score. */
  };

  Tier tier1; /**< Configuration for tier 1 devices. */
  Tier tier2; /**< Configuration for tier 2 devices. */
  std::vector<Benchmark> historical_benchmarks; /**< List of historical device benchmarks. */
};

/**
 * @brief Retrieves device hardware info as a map.
 * 
 * @return Map of device hardware properties to values.
 */
std::map<std::string, std::string> get_device_info();

/**
 * @brief Parses a DeviceTierConfig from a raw JSON string.
 * 
 * @param jsonStr The JSON string to parse.
 * @return Parsed DeviceTierConfig object.
 */
DeviceTierConfig from_raw_json(const std::string& jsonStr);

/**
 * @brief Determines the device tier based on hardware capabilities.
 * 
 * @param commandCenter Pointer to CommandCenter.
 * @return The device's performance tier.
 */
DeviceTier get_device_tier(CommandCenter* commandCenter);

/**
 * @brief Retrieves historical benchmark data for devices from backend.
 * 
 * @param commandCenter Pointer to CommandCenter for deployment access.
 * @return JSON string of historical benchmarks.
 */
std::string get_historical_benchmarks(CommandCenter* commandCenter);

/**
 * @brief Gets all compatible LLMs - custom and os provided for a device model and tier.
 * 
 * @param commandCenter Pointer to CommandCenter.
 * @param device_model The device model string.
 * @param tier The device's performance tier.
 * @return Vector of maps, each containing an LLM's name and provider.
 */
std::vector<std::map<std::string, std::string>> get_all_llms(CommandCenter* commandCenter,
                                                             const std::string& device_model,
                                                             llmutil::DeviceTier tier);

/**
 * @brief Gets the OS-supported LLM for a device model, if available.
 * 
 * @param device_model The device model string.
 * @return Optional string with the OS-supported LLM name, or std::nullopt if not supported.
 */
std::optional<std::string> get_os_supported_llm(const std::string& device_model);

}  // namespace llmutil

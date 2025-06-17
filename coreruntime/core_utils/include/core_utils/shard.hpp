/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file shard.hpp
 * @brief Header file containing sharding utility functions and constants
 */

#pragma once

#include <cassert>
#include <string>

#include "core_utils/fmt.hpp"
#include "core_utils/ne_md5.hpp"

/** @brief Total number of shards in the system */
constexpr int TOTAL_SHARDS = 1000;

namespace util {

/**
 * @brief Calculates MD5 hash of the input string
 * @param input The input string to calculate MD5 hash for
 * @return std::string The MD5 hash of the input string
 */
std::string get_md5(const std::string& input);

/**
 * @brief Calculates the shard number for a given device ID
 * @param deviceId The device ID to calculate shard number for
 * @return int The shard number (0 to TOTAL_SHARDS-1)
 */
int calculate_shard_number(const std::string& deviceId);

}  // namespace util

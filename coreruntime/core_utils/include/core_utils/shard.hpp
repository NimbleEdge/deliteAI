/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cassert>
#include <string>

#include "core_utils/fmt.hpp"
#include "core_utils/ne_md5.hpp"

constexpr int TOTAL_SHARDS = 1000;

namespace util {

std::string get_md5(const std::string& input);
int calculate_shard_number(const std::string& deviceId);

}  // namespace util

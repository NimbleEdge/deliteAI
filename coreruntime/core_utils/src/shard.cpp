/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core_utils/shard.hpp"

namespace util {

std::string get_md5(const std::string& input) { return MD5()(input); }

int calculate_shard_number(const std::string& deviceId) {
  std::string hash = get_md5(deviceId);
  if (hash.size() != 32) {
    THROW("MD5 hash of deviceId %s expected to be 32 characters, found %d instead",
          deviceId.c_str(), hash.size());
  }

  const auto lastEightBytes = hash.substr(24);
  // ^This will not throw since pos (24) < size of string (32)
  assert(lastEightBytes.size() == 8);

  unsigned long long hashInt;
  std::size_t numParsedChars = 0;
  try {
    hashInt = std::stoull(lastEightBytes, &numParsedChars, 16);
  } catch (const std::exception& e) {
    THROW("Could not convert deviceId %s, last eight chars of MD5 %s, to shard number: %s",
          deviceId.c_str(), lastEightBytes.c_str(), e.what());
  }

  if (numParsedChars != 8) {
    THROW(
        "Could not parse for deviceId %s, last eight chars of MD5 %s, parsed till %d "
        "characters",
        deviceId.c_str(), lastEightBytes.c_str(), numParsedChars);
  }

  return hashInt % TOTAL_SHARDS;
}

}  // namespace util

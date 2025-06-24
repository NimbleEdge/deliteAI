/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string>

#include "nlohmann_json.hpp"
#include "util.hpp"
using json = nlohmann::json;

namespace jsonparser {
bool get_json(json& j, const std::string& s) {
  try {
    j = json::parse(s);
    return true;
  } catch (json::exception& e) {
    LOG_TO_ERROR("String is not a valid json %s. error=%s", s.c_str(), e.what());
  }
  return false;
}
}  // namespace jsonparser

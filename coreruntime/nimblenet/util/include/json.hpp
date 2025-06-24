/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include "logger.hpp"
#include "nlohmann_json.hpp"

using json = nlohmann::json;

/**
 * @brief Namespace for JSON parsing utilities.
 */
namespace jsonparser {

/**
 * @brief Parses a JSON string into a json object.
 * 
 * @param j Reference to json object to populate.
 * @param s Input string containing JSON data.
 * @return True if parsing was successful, false otherwise.
 */
bool get_json(json& j, const std::string& s);

/**
 * @brief Converts a json object to a C++ object of type T.
 * 
 * @tparam T The type to convert to.
 * @param j The json object to convert.
 * @return The converted object of type T, or default-constructed T on error.
 */
template <class T>
T get_from_json(const json& j) {
  try {
    return j.get<T>();
  } catch (json::exception& e) {
    std::string jDump = j.dump();
    LOG_TO_ERROR("JSON object=%s could not converted to object of type=%s. error=%s", jDump.c_str(),
                 typeid(T).name(), e.what());
  }
  return T();
}

/**
 * @brief Parses a JSON string and converts it to a C++ object of type T.
 * 
 * @tparam T The type to convert to.
 * @param jsonString The input JSON string.
 * @return The converted object of type T, or default-constructed T on error.
 */
template <class T>
T get(const std::string& jsonString) {
  json j;
  if (!get_json(j, jsonString)) {
    return T();
  }
  return get_from_json<T>(j);
}

}  // namespace jsonparser

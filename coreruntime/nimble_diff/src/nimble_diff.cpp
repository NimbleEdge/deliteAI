/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file nimble_diff.cpp
 * @brief Provides utilities for generating, applying, and comparing JSON diffs using nlohmann::json.
 *
 * This file contains functions to load JSON from files, generate JSON diffs, apply patches, and compare JSON objects.
 * The main function provides a CLI interface for these operations.
 */

#include <fstream>
#include <iostream>
#include <string>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * @brief Loads JSON data from a file.
 *
 * @param fileName The path to the JSON file.
 * @return json The parsed JSON data.
 */
json loadJsonFromFile(const std::string& fileName) {
  std::ifstream file(fileName);
  json data;
  file >> data;
  return data;
}

/**
 * @brief Generates a JSON diff (patch) between two JSON objects.
 *
 * @param oldData The original JSON object.
 * @param newData The modified JSON object.
 * @return json The JSON patch representing the difference.
 */
json generateDiff(const json& oldData, const json& newData) {
  json patch = nlohmann::json::diff(oldData, newData);
  return patch;
}

/**
 * @brief Applies a JSON patch to a JSON object.
 *
 * @param data The original JSON object.
 * @param patch The JSON patch to apply.
 * @return json The resulting JSON object after applying the patch.
 */
json applyDiff(const json& data, const json& patch) {
  json result = data.patch(patch);
  return result;
}

/**
 * @brief Compares two JSON objects for equality.
 *
 * @param d1 The first JSON object.
 * @param d2 The second JSON object.
 * @return true if the objects are equal, false otherwise.
 */
bool compareJson(const json& d1, const json& d2) { return d1 == d2; }

/**
 * @brief Main entry point for the nimble_diff utility.
 *
 * Usage: nimble_diff <old_file.json> <new_file.json> <operation> <output_file>
 *
 * Supported operations:
 *   - diff: Generate a JSON diff (patch) from old_file.json to new_file.json
 *   - patch: Apply a JSON patch (new_file.json) to old_file.json
 *   - compare: Compare two JSON files for equality
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int Exit code.
 */
int main(int argc, char* argv[]) {
  if (argc != 5) {
    std::cout << "Usage: " << argv[0]
              << " <old_file.json> <new_file.json> <operation> <output_file>" << std::endl;
    return 1;
  }

  std::string fileName1 = argv[1];
  std::string fileName2 = argv[2];
  std::string operation = argv[3];
  std::string outputFileName = argv[4];

  json data1 = loadJsonFromFile(fileName1);
  json data2 = loadJsonFromFile(fileName2);
  std::ofstream outFile(outputFileName);

  if (operation == "diff") {
    json patch = generateDiff(data1, data2);
    outFile << patch << std::flush;
  } else if (operation == "patch") {
    json result = applyDiff(data1, data2);
    outFile << result << std::flush;
  } else if (operation == "compare") {
    outFile << compareJson(data1, data2) << std::flush;
  } else {
    outFile << "Invalid operation. Use 'diff' or 'patch'." << std::endl;
  }

  outFile.close();

  return 0;
}

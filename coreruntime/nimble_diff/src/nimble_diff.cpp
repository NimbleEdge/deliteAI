/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fstream>
#include <iostream>
#include <string>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

// Function to load JSON data from a file
json loadJsonFromFile(const std::string& fileName) {
  std::ifstream file(fileName);
  json data;
  file >> data;
  return data;
}

// Function to generate JSON diff
json generateDiff(const json& oldData, const json& newData) {
  json patch = nlohmann::json::diff(oldData, newData);
  return patch;
}

// Function to apply JSON diff
json applyDiff(const json& data, const json& patch) {
  json result = data.patch(patch);
  return result;
}

bool compareJson(const json& d1, const json& d2) { return d1 == d2; }

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

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "user_events_struct.hpp"

using namespace std;

void from_json(const json& j, PreProcessorInfo& preProcessorInfo) {
  j.at("rollingWindowsInSecs").get_to(preProcessorInfo.rollingWindowsInSecs);
  auto operations = j.at("operations").get<std::vector<json>>();
  for (const auto& operationJson : operations) {
    if (operationJson.find("columnName") == operationJson.end()) {
      LOG_TO_CLIENT_ERROR(
          "%s", "Could not find columnName key in operations object for the preprocessor");
      return;
    }
    if (operationJson.find("operator") == operationJson.end()) {
      LOG_TO_CLIENT_ERROR("%s",
                          "Could not find operator key in operations object for the preprocessor");
      return;
    }
    if (operationJson.find("default") == operationJson.end()) {
      LOG_TO_CLIENT_ERROR("%s",
                          "Could not find default key in operations object for the preprocessor");
      return;
    }
    preProcessorInfo.columnsToAggregate.push_back(std::string(operationJson["columnName"]));
    preProcessorInfo.aggregateOperators.push_back(std::string(operationJson["operator"]));
    preProcessorInfo.defaultVector.push_back(operationJson["default"].get<double>());
  }
  if (j.find("tableName") != j.end()) {
    j.at("tableName").get_to(preProcessorInfo.tableName);
  }
  j.at("groupBy").get_to(preProcessorInfo.groupColumns);
  preProcessorInfo.valid = true;
}

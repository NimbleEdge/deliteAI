/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include <string>
#include <vector>

#include "nlohmann_json.hpp"

struct APICall {
  std::string path;
  int status_code;
  bool is_mocked;

  bool operator==(const APICall& other) const {
    return (path == other.path) && (status_code == other.status_code) &&
           (is_mocked == other.is_mocked);
  }
};

struct HistoricalAPIs {
  std::vector<APICall> api_calls;
  std::vector<nlohmann::json> external_logger_events;
  std::vector<nlohmann::json> external_logger_scriptlogs;
  std::vector<nlohmann::json> unauthenticated_external_logs;

  bool operator==(const HistoricalAPIs& other) const;
};

void from_json(const nlohmann::json& j, APICall& a);
void from_json(const nlohmann::json& j, HistoricalAPIs& h);
void to_json(nlohmann::json& j, const APICall& a);
void to_json(nlohmann::json& j, const HistoricalAPIs& h);
void remove_ignored_fields_from_logs(nlohmann::json& event);

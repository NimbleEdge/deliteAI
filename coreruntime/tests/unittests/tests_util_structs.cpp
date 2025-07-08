/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests_util_structs.hpp"

#include "core_utils/fmt.hpp"

void from_json(const nlohmann::json& j, APICall& a) {
  if (j.find("path") == j.end()) {
    THROW("%s", "path must be present in APICall struct.");
  }
  j.at("path").get_to(a.path);
  if (j.find("status_code") == j.end()) {
    THROW("%s", "status_code must be present in APICall struct.");
  }
  j.at("status_code").get_to(a.status_code);
  if (j.find("is_mocked") == j.end()) {
    THROW("%s", "is_mocked must be present in APICall struct.");
  }
  j.at("is_mocked").get_to(a.is_mocked);
}

void from_json(const nlohmann::json& j, HistoricalAPIs& h) {
  if (j.find("api_calls") == j.end()) {
    THROW("%s", "api_calls must be present in HistoricalAPIs struct.");
  }
  j.at("api_calls").get_to(h.api_calls);
  if (j.find("external_logger_events") != j.end()) {
    j.at("external_logger_events").get_to(h.external_logger_events);
  }
  if (j.find("external_logger_scriptlogs") != j.end()) {
    j.at("external_logger_scriptlogs").get_to(h.external_logger_scriptlogs);
  }
  if (j.find("unauthenticated_external_logs") != j.end()) {
    j.at("unauthenticated_external_logs").get_to(h.unauthenticated_external_logs);
  }
}

void to_json(nlohmann::json& j, const APICall& a) {
  j = nlohmann::json(
      {{"path", a.path}, {"status_code", a.status_code}, {"is_mocked", a.is_mocked}});
}

void to_json(nlohmann::json& j, const HistoricalAPIs& h) {
  // Helper lambda to process event vectors
  auto process_events = [](const std::vector<nlohmann::json>& events) {
    std::vector<nlohmann::json> filtered_events = events;
    for (auto& event : filtered_events) {
      remove_ignored_fields_from_logs(event);
    }
    return filtered_events;
  };

  j = nlohmann::json({
      {"api_calls", h.api_calls},
      {"external_logger_events", process_events(h.external_logger_events)},
      {"external_logger_scriptlogs", process_events(h.external_logger_scriptlogs)},
      {"unauthenticated_external_logs", process_events(h.unauthenticated_external_logs)},
  });
}

void remove_ignored_fields_from_logs(nlohmann::json& event) {
  event.erase("@timestamp");
  event.erase("sessionId");
  event.erase("timestamp");
  event.erase("deviceID");
  event.erase("message");
  event.erase("source_type");
}

bool assert_disk_writes(const std::vector<nlohmann::json>& this_logs,
                        const std::vector<nlohmann::json>& other_logs) {
  auto filtered_this = this_logs;
  auto filtered_other = other_logs;
  for (auto& log : filtered_this) remove_ignored_fields_from_logs(log);
  for (auto& log : filtered_other) remove_ignored_fields_from_logs(log);
  return filtered_this == filtered_other;
}

bool HistoricalAPIs::operator==(const HistoricalAPIs& other) const {
  // API Assertions
  // Filter out /logger and /externalLogger calls
  auto filter = [](const APICall& call) {
    return call.path == "logger" || call.path == "externalLogger";
  };
  std::vector<APICall> filtered_this_api_calls = api_calls;
  std::vector<APICall> filtered_other_api_calls = other.api_calls;

  filtered_this_api_calls.erase(
      std::remove_if(filtered_this_api_calls.begin(), filtered_this_api_calls.end(), filter),
      filtered_this_api_calls.end());
  filtered_other_api_calls.erase(
      std::remove_if(filtered_other_api_calls.begin(), filtered_other_api_calls.end(), filter),
      filtered_other_api_calls.end());

  return (filtered_other_api_calls == filtered_this_api_calls) &&
         assert_disk_writes(external_logger_events, other.external_logger_events) &&
         assert_disk_writes(external_logger_scriptlogs, other.external_logger_scriptlogs) &&
         assert_disk_writes(unauthenticated_external_logs, other.unauthenticated_external_logs);
}

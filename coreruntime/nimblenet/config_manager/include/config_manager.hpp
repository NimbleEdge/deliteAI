/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "database_constants.hpp"
#include "logger_constants.hpp"
#include "nlohmann_json.hpp"

using json = nlohmann::json;

class CommandCenter;

class Config {
  // if adding new members here, remember to add in the copy constructor
  mutable std::mutex _configMutex;
  std::vector<std::string> modelIds;
  void init(const nlohmann::json& j);

 public:
  std::string configJsonString;
  std::string compatibilityTag;
  std::string deviceId;
  std::string clientId;
  std::string host;
  std::string clientSecret;
  std::string internalDeviceId;
  std::vector<json> tableInfos;
  bool debug = false;
  int maxInputsToSave = 0;
  float maxDBSizeKBs = dbconstants::MaxDBSizeKBs;
  float maxEventsSizeKBs = loggerconstants::MaxEventsSizeKBs;
  nlohmann::json cohortIds = nlohmann::json::array();
#ifdef SIMULATION_MODE
  // By default simulator to be run in offline mode
  bool online = false;
  // Be default simulator to require TIMESTAMP for user events
  bool isTimeSimulated = true;
#else
  // For non-simulation mode, offline mode is not allowed, assets are always fetched from cloud
  bool online = true;
  // For non-simulation mode, time simulation is not allowed
  bool isTimeSimulated = false;
#endif

  char* c_str() {
    std::string tables = "[";
    for (const auto& it : tableInfos) {
      tables += it.dump() + ",";
    }
    tables += "]";
    std::string models = "[";
    for (const auto& model : modelIds) {
      models += model + ",";
    }
    models += "]";
    auto cohortDump = cohortIds.dump();

    char* ret;
    asprintf(&ret,
             "deviceId=%s,clientId=%s,clientSecret=****,host=%s,compatibilityTag=%s,"
             "modelIds=%s, "
             "databaseConfig=%s, debug:%s, maxInputsToSave:%d, online:%d, internalDeviceId: %s, "
             "isTimeSimulated:%d, maxDBSizeKBs:%f, maxEventSizeKBS: %f, cohorts: %s",
             deviceId.c_str(), clientId.c_str(), host.c_str(), compatibilityTag.c_str(),
             models.c_str(), tables.c_str(), debug ? "true" : "false", maxInputsToSave, online,
             internalDeviceId.c_str(), isTimeSimulated, maxDBSizeKBs, maxEventsSizeKBs,
             cohortDump.c_str());
    return ret;
  }

  bool isDebug() const { return debug; }

  std::vector<std::string> get_modelIds() const {
    std::lock_guard<std::mutex> lck(_configMutex);
    auto models = modelIds;
    return models;
  }

  bool add_model(const std::string& modelId) {
    std::lock_guard<std::mutex> lck(_configMutex);
    auto it = std::find(modelIds.begin(), modelIds.end(), modelId);
    if (it == modelIds.end()) {
      modelIds.push_back(modelId);
      return true;
    }
    return false;
  }

  Config(const std::string& configJsonString);
  Config(const nlohmann::json& json);

  Config() = delete;
  Config(const Config&) = delete;

  friend class CommandCenter;
};

inline const void to_json(nlohmann::json& j, const Config& config) {
  j = nlohmann::json{{"compatibilityTag", config.compatibilityTag},
                     {"cohortIds", config.cohortIds}};
}

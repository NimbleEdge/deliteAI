/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "config_manager.hpp"

#include "core_sdk_constants.hpp"
#include "logger.hpp"
#include "util.hpp"

using namespace std;
using json = nlohmann::json;

Config::Config(const nlohmann::json& j) { init(j); }

void Config::init(const nlohmann::json& j) {
  if (j.find("databaseConfig") != j.end()) {
    j.at("databaseConfig").get_to(tableInfos);
  }
  if (j.find("maxInputsToSave") != j.end()) {
    j.at("maxInputsToSave").get_to(maxInputsToSave);
  }
  if (j.find("modelIds") != j.end()) {
    THROW("%s", "modelIds key should not be present in config.");
  }
  if (j.find("cohortIds") != j.end()) {
    if (!j.at("cohortIds").is_array()) {
      THROW("%s", "CohortIds must be array of cohorts.");
    }
    j.at("cohortIds").get_to(cohortIds);
  }

// Set isTimeSimulated flag only in SIMULATION_MODE/TESTING, by default the value will be
// false
#if defined(SIMULATION_MODE) || defined(TESTING)
  if (j.find("isTimeSimulated") != j.end()) {
    j.at("isTimeSimulated").get_to(isTimeSimulated);
  }
#endif
  if (j.find("debug") != j.end()) {
    j.at("debug").get_to(debug);
  }
  if (j.find("online") != j.end()) {
    j.at("online").get_to(online);
  }

  if (j.find("maxDBSizeKBs") != j.end()) {
    j.at("maxDBSizeKBs").get_to(maxDBSizeKBs);
  }
  if (j.find("maxEventsSizeKBs") != j.end()) {
    j.at("maxEventsSizeKBs").get_to(maxEventsSizeKBs);
  }
  if (online) {
    j.at("compatibilityTag").get_to(compatibilityTag);
  } else {
    if (!j.contains("compatibilityTag") ||
        (j.contains("compatibilityTag") && j.at("compatibilityTag") == "")) {
      compatibilityTag = coresdkconstants::DefaultCompatibilityTag;
    } else {
      j.at("compatibilityTag").get_to(compatibilityTag);
    }
  }

  if (online) {
    j.at("clientId").get_to(clientId);
    if (clientId == "") {
      THROW("%s", "Expected clientId, found empty string");
    }

    j.at("clientSecret").get_to(clientSecret);
    if (clientSecret == "") {
      THROW("%s", "Expected clientSecret, found empty string");
    }

#ifdef SIMULATION_MODE
    j.at("clientId").get_to(internalDeviceId);
    j.at("clientId").get_to(deviceId);
#else
    j.at("internalDeviceId").get_to(internalDeviceId);
    if (internalDeviceId == "") {
      THROW("%s", "Expected internalDeviceId, found empty string");
    }
    if (j.find("deviceId") != j.end()) {
      j.at("deviceId").get_to(deviceId);
    }
    // In case deviceId was not given in config or given as empty string, use internalDeviceId
    if (deviceId == "") {
      deviceId = internalDeviceId;
    }
#endif
    j.at("host").get_to(host);
    while (host.back() == '/') {
      host = host.substr(0, host.size() - 1);
    }
    if (host.empty()) {
      // FEATURE: Can use regex here to check if it's a proper URL
      THROW("%s", "Expected host to be a proper URL, found empty");
    }
  }
  configJsonString = j.dump();

  if (j.find("sessionId") != j.end()) {
    auto sessionIdString = j.at("sessionId").get<std::string>();
    util::set_session_id(sessionIdString);
  } else {
    util::set_session_id("");
  }
}

Config::Config(const std::string& configJsonString) {
  nlohmann::json j;
  try {
    j = nlohmann::json::parse(configJsonString);
  } catch (std::exception& e) {
    THROW("error=%s in config parsing", e.what());
  } catch (...) {
    THROW("%s", "configstr not a valid json");
  }
  init(j);
}

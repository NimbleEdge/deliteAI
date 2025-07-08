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

/**
 * @class Config
 * @brief Holds configuration settings for the application passed in initialize API. This
 * includes device identity, client credentials, database settings, model information, and runtime
 * flags.
 */
class Config {
  /** Mutex to protect access to modelIds. */
  mutable std::mutex _configMutex;

  /** List of model identifiers. */
  std::vector<std::string> modelIds;

  /**
   * @brief Initializes the configuration from a JSON object.
   *
   * @param j JSON object containing configuration fields.
   */
  void init(const nlohmann::json& j);

 public:
  /** Raw JSON string representing the configuration. */
  std::string configJsonString;

  /** Tag representing the compatibility version of the configuration. */
  std::string compatibilityTag;

  /** Unique device identifier passed on by caller. */
  std::string deviceId;

  /**
   * Unique client identifier.
   * Used for identifying the client when connecting to a secure SaaS
   * platform.
   */
  std::string clientId;

  /** Host address for server communication to the SaaS platform. */
  std::string host;

  /** Client secret for authentication. Used along with clientId for secure SaaS platform access. */
  std::string clientSecret;

  /** Internal device identifier added by the SDK. */
  std::string internalDeviceId;

  /** Table metadata for use in on-device DB. */
  std::vector<json> tableInfos;

  /** Debug flag to enable verbose logging or diagnostic behavior. */
  bool debug = false;

  /**
   * @brief Maximum number of inputs to persist.
   *
   * @note To be deprecated.
   */
  int maxInputsToSave = 0;

  /** Maximum size of the database in kilobytes. */
  float maxDBSizeKBs = dbconstants::MaxDBSizeKBs;

  /** Maximum size of event logs in kilobytes. */
  float maxEventsSizeKBs = loggerconstants::MaxEventsSizeKBs;

  /** List of cohort identifiers where this configuration will be used. */
  nlohmann::json cohortIds = nlohmann::json::array();

  /** Flag to indicate whether assets should be fetched from cloud or provided from disk. */
  bool online = false;

#ifdef SIMULATION_MODE
  /**
   * @brief Flag indicating whether time is simulated.
   * Defaults to true in simulation mode.
   */
  bool isTimeSimulated = true;
#else
  /** Time simulation is disabled outside of simulation. */
  bool isTimeSimulated = false;
#endif  // SIMULATION_MODE

  /**
   * @brief Returns a C-style string representing the current configuration state.
   *
   * @return A dynamically allocated char* string (must be freed by the caller).
   */
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

  /**
   * @brief Checks if the configuration is in debug mode.
   *
   * @return True if debug is enabled, false otherwise.
   */
  bool isDebug() const { return debug; }

  /**
   * @brief Retrieves a thread-safe copy of the list of model IDs.
   *
   * @return Vector of model ID strings.
   */
  std::vector<std::string> get_modelIds() const {
    std::lock_guard<std::mutex> lck(_configMutex);
    auto models = modelIds;
    return models;
  }

  /**
   * @brief Adds a new model ID to the list if it's not already present.
   *
   * @param modelId The model identifier to add.
   * @return True if the model ID was added, false if it already existed.
   */
  bool add_model(const std::string& modelId) {
    std::lock_guard<std::mutex> lck(_configMutex);
    auto it = std::find(modelIds.begin(), modelIds.end(), modelId);
    if (it == modelIds.end()) {
      modelIds.push_back(modelId);
      return true;
    }
    return false;
  }

  /**
   * @brief Constructs the configuration from a JSON string.
   *
   * @param configJsonString Raw JSON string containing configuration.
   */
  Config(const std::string& configJsonString);

  /**
   * @brief Constructs the configuration from a JSON object.
   *
   * @param json JSON object containing configuration.
   */
  Config(const nlohmann::json& json);

  /** Default constructor is deleted. */
  Config() = delete;

  /** Copy constructor is deleted. */
  Config(const Config&) = delete;

  /** Grant access to private members for CommandCenter. */
  friend class CommandCenter;
};

/**
 * @brief Serializes selected Config fields to a JSON object. These fields are exposed in the
 * workflow script.
 *
 * @param j JSON object to populate.
 * @param config Configuration object to serialize.
 */
inline const void to_json(nlohmann::json& j, const Config& config) {
  j = nlohmann::json{{"compatibilityTag", config.compatibilityTag},
                     {"cohortIds", config.cohortIds}};
}

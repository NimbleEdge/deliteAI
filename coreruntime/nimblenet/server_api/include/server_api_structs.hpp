/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>

#include "asset_manager.hpp"
#include "core_sdk_constants.hpp"
#include "log_sender.hpp"
#include "logger.hpp"
#include "logger_constants.hpp"
#include "nlohmann_json.hpp"
#include "resource_manager_constants.hpp"
#include "time_manager.hpp"
#include "util.hpp"

#ifdef GENAI
#include "base_llm_executor.hpp"
#endif  // GENAI

using json = nlohmann::json;

/**
 * @brief Request structure for device registration.
 *
 * Contains client ID, device ID, and a list of model IDs to register.
 */
struct RegisterRequest {
  std::string clientId; /**< Client identifier. */
  std::string deviceId; /**< Device identifier. */
  std::vector<std::string> modelIds; /**< List of model IDs. */

  /**
   * @brief Constructs a RegisterRequest.
   *
   * @param clientId_ Client identifier.
   * @param deviceId_ Device identifier.
   * @param models_ List of model IDs.
   */
  RegisterRequest(const std::string& clientId_, const std::string& deviceId_,
                  const std::vector<std::string>& models_) {
    clientId = clientId_;
    deviceId = deviceId_;
    modelIds = models_;
  }
};

/**
 * @brief Response structure for device registration.
 *
 * Contains HTTP headers and query parameters returned by the server.
 */
struct RegisterResponse {
  nlohmann::json headers; /**< HTTP headers returned by the server. */
  std::string queryParams; /**< Query parameters returned by the server. */
};

/**
 * @brief Response structure for a task request.
 *
 * Contains the task AST, version, task name, and validity flag.
 */
struct TaskResponse {
  nlohmann::json taskAST; /**< Task AST as JSON. */
  std::string version; /**< Task version string. */
  std::string taskName; /**< Name of the task. */
  bool valid = false; /**< Indicates if the response is valid. */
};

/**
 * @brief Metadata for a model.
 *
 * Contains version, execution provider config version, and validity flag.
 */
struct ModelMetadata {
  std::string version; /**< Model version string. */
  int epConfigVersion; /**< Execution provider config version. */
  bool valid; /**< Indicates if the metadata is valid. */

  ModelMetadata() {
    version = "";
    epConfigVersion = -1;
    valid = false;
  }
};

/**
 * @brief Metadata for a task.
 *
 * Contains version and validity flag.
 */
struct TaskMetadata {
  std::string version; /**< Task version string. */
  bool valid; /**< Indicates if the metadata is valid. */

  TaskMetadata() {
    version = "";
    valid = false;
  }
};

/**
 * @brief Response structure for a model download request.
 *
 * Contains status and file name.
 */
struct DownloadModelResponse {
  int status = 0; /**< Download status code. */
  std::string fileName; /**< Name of the downloaded file. */
};

/**
 * @brief Logger configuration structure.
 *
 * Contains sender and writer configuration for logging.
 */
struct LoggerConfig {
  LogSendingConfig senderConfig; /**< Configuration for log sending. */
  LogWritingConfig writerConfig; /**< Configuration for log writing. */
};

/**
 * @brief Converts JSON to LoggerConfig.
 *
 * @param j JSON object.
 * @param loggerConfig LoggerConfig to populate.
 */
void from_json(const json j, LoggerConfig& loggerConfig);

/**
 * @brief Converts LoggerConfig to JSON.
 *
 * @param j JSON object to populate.
 * @param loggerConfig LoggerConfig to convert.
 */
void to_json(json& j, const LoggerConfig& loggerConfig);

/**
 * @brief State of the cloud configuration.
 */
enum class CloudConfigState { Invalid, Valid, Unmodified };

/**
 * @brief Deployment information structure.
 *
 * Contains deployment ID, update flag, script asset, module assets, and eTag.
 */
struct Deployment {
  int Id = -1; /**< Deployment ID. */
  bool forceUpdate = false; /**< Indicates if a force update is required. */
  std::shared_ptr<Asset> script; /**< Main script asset. */
  std::vector<std::shared_ptr<Asset>> modules; /**< List of module assets. */
  std::string eTag; /**< Entity tag for versioning. */

  /**
   * @brief Retrieves a module asset by name and type.
   *
   * @param moduleName Name of the module.
   * @param type Asset type.
   * @return Shared pointer to the asset if found, nullptr otherwise.
   */
  std::shared_ptr<Asset> get_module(const std::string& moduleName, AssetType type) const {
    for (auto module : modules) {
      if (module->name == moduleName && module->type == type) {
        return module;
      }
    }
    return nullptr;
  }
};

/**
 * @brief Converts JSON to Deployment.
 *
 * @param j JSON object.
 * @param dep Deployment to populate.
 */
void from_json(const json& j, Deployment& dep);

/**
 * @brief Converts Deployment to JSON.
 *
 * @param j JSON object to populate.
 * @param dep Deployment to convert.
 */
void to_json(json& j, const Deployment& dep);

/**
 * @brief Cloud configuration response structure.
 *
 * Contains configuration parameters, logger configs, server time, pegged device time, state, and ads host.
 */
struct CloudConfigResponse {
  std::map<std::string, std::string> requestToHostMap; /**< Maps request types to hosts. */
  int inferenceMetricLogInterval = loggerconstants::InferenceMetricLogInterval; /**< Inference metric log interval. */
  long long int threadSleepTimeUSecs = coresdkconstants::LongRunningThreadSleepUTime; /**< Thread sleep time in microseconds. */
  float fileDeleteTimeInDays = coresdkconstants::FileDeleteTimeInDays; /**< File delete time in days. */
  LoggerConfig nimbleLoggerConfig; /**< Nimble logger configuration. */
  LoggerConfig externalLoggerConfig; /**< External logger configuration. */
  uint64_t serverTimeMicros = 0;  /**< Server time in microseconds from UTC. */
  PeggedDeviceTime peggedDeviceTime; /**< Local and server time at config fetch. */
  CloudConfigState state = CloudConfigState::Invalid; /**< State of the cloud config. */
  std::string adsHost = ""; /**< ADS host for private assets. */

#ifdef GENAI
  LLMExecutorConfig llmExecutorConfig; /**< LLM executor configuration (GENAI only). */
#endif  // GENAI

  CloudConfigResponse() {
    nimbleLoggerConfig.senderConfig._host = loggerconstants::DefaultLogUploadURL;
    nimbleLoggerConfig.senderConfig.valid = true;
    nimbleLoggerConfig.senderConfig._secretKey = nimbleLoggerConfig.senderConfig._defaultSecretKey;
  }
};

/**
 * @brief Log request body structure.
 *
 * Contains host, headers, and body for a log upload request.
 */
struct LogRequestBody {
  std::string host; /**< Host endpoint for log upload. */
  json headers; /**< HTTP headers for the log request. */
  std::string body; /**< Log request body. */

  /**
   * @brief Constructs a LogRequestBody.
   *
   * @param logheaders HTTP headers.
   * @param logbody Log body string.
   * @param hostendpoint Host endpoint string.
   */
  LogRequestBody(const json& logheaders, const std::string& logbody,
                 const std::string& hostendpoint) {
    host = hostendpoint;
    body = logbody;
    headers = logheaders;
  }
};

/**
 * @brief Authentication information structure.
 *
 * Contains validity flag, API headers, and API query string.
 */
struct AuthenticationInfo {
  bool valid = false; /**< Indicates if the authentication info is valid. */
  std::string apiHeaders; /**< API headers as a string. */
  std::string apiQuery; /**< API query string. */
};

/**
 * @brief Converts JSON to ModelMetadata.
 *
 * @param j JSON object.
 * @param metadata ModelMetadata to populate.
 */
void from_json(const json& j, ModelMetadata& metadata);

/**
 * @brief Converts ModelMetadata to JSON.
 *
 * @param j JSON object to populate.
 * @param metadata ModelMetadata to convert.
 */
void to_json(json& j, const ModelMetadata& metadata);

/**
 * @brief Converts JSON to TaskMetadata.
 *
 * @param j JSON object.
 * @param metadata TaskMetadata to populate.
 */
void from_json(const json& j, TaskMetadata& metadata);

/**
 * @brief Converts TaskMetadata to JSON.
 *
 * @param j JSON object to populate.
 * @param metadata TaskMetadata to convert.
 */
void to_json(json& j, const TaskMetadata& metadata);

/**
 * @brief Converts JSON to CloudConfigResponse.
 *
 * @param j JSON object.
 * @param logKeyResponse CloudConfigResponse to populate.
 */
void from_json(const json& j, CloudConfigResponse& logKeyResponse);

/**
 * @brief Converts CloudConfigResponse to JSON.
 *
 * @param j JSON object to populate.
 * @param cloudConfig CloudConfigResponse to convert.
 */
void to_json(json& j, const CloudConfigResponse& cloudConfig);

/**
 * @brief Converts JSON to RegisterResponse.
 *
 * @param j JSON object.
 * @param registerResponse RegisterResponse to populate.
 */
void from_json(const json& j, RegisterResponse& registerResponse);

// AuthenticationInfo
/**
 * @brief Converts JSON to AuthenticationInfo.
 *
 * @param j JSON object.
 * @param info AuthenticationInfo to populate.
 */
void from_json(const json& j, AuthenticationInfo& info);

/**
 * @brief Converts AuthenticationInfo to JSON.
 *
 * @param j JSON object to populate.
 * @param authInfo AuthenticationInfo to convert.
 */
void to_json(json& j, const AuthenticationInfo& authInfo);

/**
 * @brief Parses cloud config and deployment from JSON.
 *
 * @param j JSON object containing both cloud config and deployment.
 * @return Pair of CloudConfigResponse and Deployment.
 */
std::pair<CloudConfigResponse, Deployment> get_config_and_deployment_from_json(
    const nlohmann::json& j);

/**
 * @brief Converts JSON to TaskResponse.
 *
 * @param j JSON object.
 * @param task TaskResponse to populate.
 */
inline const void from_json(const json& j, TaskResponse& task) {
  j.at("AST").get_to(task.taskAST);
  if (j.find("version") != j.end()) {
    j.at("version").get_to(task.version);
  }
  task.valid = true;
}

/**
 * @brief Converts TaskResponse to JSON.
 *
 * @param j JSON object to populate.
 * @param task TaskResponse to convert.
 */
inline const void to_json(json& j, const TaskResponse& task) {
  j = json{{"AST", task.taskAST}, {"version", task.version}};
}

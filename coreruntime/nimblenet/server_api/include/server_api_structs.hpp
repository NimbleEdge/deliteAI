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

struct RegisterRequest {
  std::string clientId;
  std::string deviceId;
  std::vector<std::string> modelIds;

  RegisterRequest(const std::string& clientId_, const std::string& deviceId_,
                  const std::vector<std::string>& models_) {
    clientId = clientId_;
    deviceId = deviceId_;
    modelIds = models_;
  }
};

struct RegisterResponse {
  nlohmann::json headers;
  std::string queryParams;
};

struct TaskResponse {
  nlohmann::json taskAST;
  std::string version;
  std::string taskName;
  bool valid = false;
};

struct ModelMetadata {
  std::string version;
  int epConfigVersion;
  bool valid;

  ModelMetadata() {
    version = "";
    epConfigVersion = -1;
    valid = false;
  }
};

struct TaskMetadata {
  std::string version;
  bool valid;

  TaskMetadata() {
    version = "";
    valid = false;
  }
};

struct DownloadModelResponse {
  int status = 0;
  std::string fileName;
};

struct LoggerConfig {
  LogSendingConfig senderConfig;
  LogWritingConfig writerConfig;
};

void from_json(const json j, LoggerConfig& loggerConfig);

void to_json(json& j, const LoggerConfig& loggerConfig);

enum class CloudConfigState { Invalid, Valid, Unmodified };

struct Deployment {
  int Id = -1;
  bool forceUpdate = false;
  std::shared_ptr<Asset> script;
  std::vector<std::shared_ptr<Asset>> modules;
  std::string eTag;

  std::shared_ptr<Asset> get_module(const std::string& moduleName, AssetType type) const {
    for (auto module : modules) {
      if (module->name == moduleName && module->type == type) {
        return module;
      }
    }
    return nullptr;
  }
};

void from_json(const json& j, Deployment& dep);
void to_json(json& j, const Deployment& dep);

struct CloudConfigResponse {
  std::map<std::string, std::string> requestToHostMap;
  int inferenceMetricLogInterval = loggerconstants::InferenceMetricLogInterval;
  long long int threadSleepTimeUSecs = coresdkconstants::LongRunningThreadSleepUTime;
  float fileDeleteTimeInDays = coresdkconstants::FileDeleteTimeInDays;
  LoggerConfig nimbleLoggerConfig;
  LoggerConfig externalLoggerConfig;
  uint64_t serverTimeMicros = 0;  // Server time in microseconds from UTC
  // Need this here to capture local time at time of cloud config fetching
  PeggedDeviceTime peggedDeviceTime;
  CloudConfigState state = CloudConfigState::Invalid;
  std::string adsHost = "";

#ifdef GENAI
  LLMExecutorConfig llmExecutorConfig;
#endif  // GENAI

  CloudConfigResponse() {
    nimbleLoggerConfig.senderConfig._host = loggerconstants::DefaultLogUploadURL;
    nimbleLoggerConfig.senderConfig.valid = true;
    nimbleLoggerConfig.senderConfig._secretKey = nimbleLoggerConfig.senderConfig._defaultSecretKey;
  }
};

struct LogRequestBody {
  std::string host;
  json headers;
  std::string body;

  LogRequestBody(const json& logheaders, const std::string& logbody,
                 const std::string& hostendpoint) {
    host = hostendpoint;
    body = logbody;
    headers = logheaders;
  }
};

struct AuthenticationInfo {
  bool valid = false;
  std::string apiHeaders;
  std::string apiQuery;
};

void from_json(const json& j, ModelMetadata& metadata);
void to_json(json& j, const ModelMetadata& metadata);
void from_json(const json& j, TaskMetadata& metadata);
void to_json(json& j, const TaskMetadata& metadata);
void from_json(const json& j, CloudConfigResponse& logKeyResponse);
void to_json(json& j, const CloudConfigResponse& cloudConfig);
void from_json(const json& j, RegisterResponse& registerResponse);

// AuthenticationInfo
void from_json(const json& j, AuthenticationInfo& info);
void to_json(json& j, const AuthenticationInfo& authInfo);

std::pair<CloudConfigResponse, Deployment> get_config_and_deployment_from_json(
    const nlohmann::json& j);

inline const void from_json(const json& j, TaskResponse& task) {
  j.at("AST").get_to(task.taskAST);
  if (j.find("version") != j.end()) {
    j.at("version").get_to(task.version);
  }
  task.valid = true;
}

inline const void to_json(json& j, const TaskResponse& task) {
  j = json{{"AST", task.taskAST}, {"version", task.version}};
}

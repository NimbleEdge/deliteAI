/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "config_manager.hpp"
#include "core_sdk_constants.hpp"
#include "job.hpp"
#include "log_sender.hpp"
#include "native_interface.hpp"
#include "ne_fwd.hpp"
#include "server_api_structs.hpp"
#include "time_manager.hpp"
#include "util.hpp"

struct UserEventsData {
  NimbleNetStatus* status;
  std::string updatedEventName;
  OpReturnType updatedEventDataVariable;

  UserEventsData(NimbleNetStatus* status_) { status = status_; }

  UserEventsData(std::string updatedEventName_, OpReturnType updatedEventDataVariable_) {
    status = nullptr;
    updatedEventName = updatedEventName_;
    updatedEventDataVariable = updatedEventDataVariable_;
  }

  UserEventsData() = delete;
};

struct InferenceTime {
  long long int minInferenceTime;
  long long int maxInferenceTime;
  long long int totalInferenceTime;
};

struct MinimalInitializationConfig {
  std::shared_ptr<Config> deviceConfig = nullptr;
  LoggerConfig externalLoggerConfig;
  LoggerConfig nimbleLoggerConfig;

  MinimalInitializationConfig(std::shared_ptr<Config> deviceConfig_,
                              const LoggerConfig& externalLoggerConfig_,
                              const LoggerConfig& nimbleLoggerConfig_) {
    deviceConfig = deviceConfig_;
    externalLoggerConfig = externalLoggerConfig_;
    nimbleLoggerConfig = nimbleLoggerConfig_;
  };

  MinimalInitializationConfig(){};
};

static inline const void to_json(json& j, const MinimalInitializationConfig& wm) {
  j = nlohmann::json{{"deviceConfig", wm.deviceConfig->configJsonString},
                     {"externalLoggerConfig", wm.externalLoggerConfig},
                     {"nimbleLoggerConfig", wm.nimbleLoggerConfig}};
};

static inline const void from_json(const json& j, MinimalInitializationConfig& wm) {
  if (j.find("deviceConfig") != j.end()) {
    std::string st = j.at("deviceConfig");
    wm.deviceConfig = std::make_shared<Config>(st);
  }
  if (j.find("externalLoggerConfig") != j.end()) {
    j.at("externalLoggerConfig").get_to(wm.externalLoggerConfig);
  }
  if (j.find("nimbleLoggerConfig") != j.end()) {
    j.at("nimbleLoggerConfig").get_to(wm.nimbleLoggerConfig);
  }
};

struct ResourceRunAggregates {
  int inferenceCount;
  InferenceTime totalTime;
  std::string resourceVersion;
  std::string resourceName;
  std::string resourceType;
  int deploymentId = -1;

  std::string to_json_string() {
    nlohmann::json abc;
    abc["name"] = resourceName;
    abc["version"] = resourceVersion;
    abc["type"] = resourceType;
    abc["time"] = nlohmann::json{{"min", totalTime.minInferenceTime},
                                 {"max", totalTime.maxInferenceTime},
                                 {"total", totalTime.totalInferenceTime}};
    abc["count"] = inferenceCount;
    abc["deploymentId"] = deploymentId;
    return std::string(abc.dump());
  }

  ResourceRunAggregates(const std::string& resourceName_, const std::string& resourceVersion_,
                        const std::string& resourceType_, int deploymentId_) {
    resourceName = resourceName_;
    resourceVersion = resourceVersion_;
    resourceType = resourceType_;
    deploymentId = deploymentId_;
    totalTime.minInferenceTime = 100000000;
    totalTime.maxInferenceTime = 0;
    totalTime.totalInferenceTime = 0;

    inferenceCount = 0;
  }

  void update_time(long long androidTime) {
    totalTime.minInferenceTime = std::min(androidTime, totalTime.minInferenceTime);
    totalTime.maxInferenceTime = std::max(totalTime.maxInferenceTime, androidTime);
    totalTime.totalInferenceTime += androidTime;
    inferenceCount++;
  }
};

struct MetricsAgent {
  nlohmann::json metricsCollection;
  std::shared_ptr<Logger> metricsLogger = std::make_shared<Logger>();
  std::chrono::time_point<std::chrono::high_resolution_clock> lastMetricTime;
  int _inferenceCount = 0;
  std::mutex _inferenceLogMutex;
  std::map<std::string, ResourceRunAggregates> _inferenceAggregates;
  std::map<std::string, ResourceRunAggregates> _scriptRunAggregates;

  MetricsAgent() {
    lastMetricTime = Time::get_high_resolution_clock_time() -
                     std::chrono::seconds(2 * loggerconstants::MetricsCollectionIntervalSecs);
  }

  void initialize(std::shared_ptr<Logger> logger) { metricsLogger = logger; }

  void log_metrics(const char* metricType, const nlohmann::json& metricJson) {
    std::string jsonDump = metricJson.dump();
    metricsLogger->LOGMETRICS(metricType, jsonDump.c_str());
  }

  void write_inference_metric(const char* modelString, const std::string& modelVersion,
                              int deploymentId, long long androidTime) {
    std::string modelId = std::string(modelString);
    std::lock_guard<std::mutex> locker(_inferenceLogMutex);
    if (_inferenceAggregates.find(modelId) == _inferenceAggregates.end()) {
      _inferenceAggregates.insert(
          {modelId, ResourceRunAggregates(modelId, modelVersion, MODELTYPE, deploymentId)});
    }
    _inferenceCount++;
    _inferenceAggregates.at(modelId).update_time(androidTime);
  }

  void write_run_method_metric(const char* methodString, const std::string& scriptVersion,
                               int deploymentId, long long androidTime) {
    std::string methodName = std::string(methodString);
    std::lock_guard<std::mutex> locker(_inferenceLogMutex);
    if (_scriptRunAggregates.find(methodName) == _scriptRunAggregates.end()) {
      _scriptRunAggregates.insert(
          {methodName, ResourceRunAggregates(methodName, scriptVersion, SCRIPTTYPE, deploymentId)});
    }
    _inferenceCount++;
    _scriptRunAggregates.at(methodName).update_time(androidTime);
  }

  void flush_inference_metrics(int inferenceMetricLogInterval) {
    std::lock_guard<std::mutex> locker(_inferenceLogMutex);
    if (_inferenceCount >= inferenceMetricLogInterval) {
      for (auto& it : _inferenceAggregates) {
        std::string inferenceAggString = it.second.to_json_string();
        this->metricsLogger->LOGMETRICS(INFERENCEV4, inferenceAggString.c_str());
      }
      for (auto& it : _scriptRunAggregates) {
        std::string inferenceAggString = it.second.to_json_string();
        this->metricsLogger->LOGMETRICS(INFERENCEV4, inferenceAggString.c_str());
      }
      _inferenceAggregates.clear();
      _scriptRunAggregates.clear();
      _inferenceCount = 0;
    }
  }

  void save_metrics(const std::string metricType, const nlohmann::json& metricJson) {
    if (metricsCollection.contains(metricType)) {
      for (auto& metricKey : metricJson.items()) {
        metricsCollection[metricType][metricKey.key()] = metricJson[metricKey.key()];
      }
      return;
    } else {
      metricsCollection[metricType] = {};
      for (auto& metricKey : metricJson.items()) {
        metricsCollection[metricType][metricKey.key()] = metricJson[metricKey.key()];
      }
      return;
    }
  }

  ~MetricsAgent() { flush_inference_metrics(1); }
};

// TODO: Move this someplace else which actually will do the logging
struct LogJob : public Job<void> {
  int deploymentId;
  std::string type;
  nlohmann::json data;
  std::shared_ptr<Logger> _loggerToUse;

  LogJob(int deploymentId, std::string&& type, nlohmann::json&& data,
         std::shared_ptr<Logger> loggerToUse)
      : Job("LogJob"),
        deploymentId(deploymentId),
        type(std::move(type)),
        data(std::move(data)),
        _loggerToUse(loggerToUse) {}

  ~LogJob() override = default;

  Job::Status process() override {
    if (_loggerToUse == nullptr) return Job::Status::COMPLETE;
    auto jsonString = data.dump();
    _loggerToUse->script_log(deploymentId, type.c_str(), jsonString.c_str());
    return Job::Status::COMPLETE;
  }
};

namespace util {

static inline void rename_deployment_to_old_deployment(std::shared_ptr<Config> config) noexcept {
  auto cloudConfigFileName = nativeinterface::get_full_file_path_common(
      config->compatibilityTag + coresdkconstants::DeploymentFileName);
  auto oldCloudConfigFileName = nativeinterface::get_full_file_path_common(
      config->compatibilityTag + coresdkconstants::OldDeploymentFileName);
  rename(cloudConfigFileName.c_str(), oldCloudConfigFileName.c_str());
}

static inline bool save_deployment_on_device(const Deployment& deployment,
                                             const std::string& compatibilityTag) {
  return nativeinterface::save_file_on_device_common(
             nlohmann::json(deployment).dump(),
             compatibilityTag + coresdkconstants::DeploymentFileName) != "";
}

static inline void read_session_metrics(const std::string& sessionFilePath,
                                        MetricsAgent* metricsAgent) {
  long long int sessionLength = 0;
  std::string metricData;
  if (nativeinterface::get_file_from_device_common(sessionFilePath, metricData)) {
    metricsAgent->metricsLogger->LOGMETRICS(SESSIONMETRICS, metricData.c_str());
  }
  for (auto& metric : metricsAgent->metricsCollection.items()) {
    std::string metricValue = std::string(metricsAgent->metricsCollection[metric.key()].dump());
    metricsAgent->metricsLogger->LOGMETRICS(metric.key().c_str(), metricValue.c_str());
  }
}

static inline int64_t sleep_flush_and_update_session_time(
    const std::chrono::time_point<std::chrono::high_resolution_clock> start,
    const int64_t threadSleepTimeUSecs, const int64_t& sessionLength) {
  int64_t timeTakenInMicros = Time::get_elapsed_time_in_micro(start);
  auto timeToSleep = std::max((int64_t)0, threadSleepTimeUSecs - timeTakenInMicros);
  usleep(timeToSleep);

  // save session metrics on device
  nlohmann::json j;
  int64_t updatedSessionLength =
      sessionLength + (std::max(timeTakenInMicros, threadSleepTimeUSecs)) / 1000;
  j["sessionLength"] = updatedSessionLength;
  nativeinterface::save_file_on_device_common(j.dump(), coresdkconstants::SessionFilePath);

  return updatedSessionLength;
}
}  // namespace util

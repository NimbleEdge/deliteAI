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

/**
 * @brief Structure holding user event data for internal processing.
 * The Json/MapDataVariable event from frontend layers after processing using the delitepy script is
 * converted to this struct. Once populated, based on the status response is returned to frontend
 * via CUserEventsData.
 */
struct UserEventsData {
  NimbleNetStatus* status;      /**< Status pointer indicating the result of event handling. */
  std::string updatedEventName; /**< Name of the updated event. */
  OpReturnType updatedEventDataVariable; /**< Data associated with the updated event. */

  UserEventsData(NimbleNetStatus* status_) { status = status_; }

  UserEventsData(std::string updatedEventName_, OpReturnType updatedEventDataVariable_) {
    status = nullptr;
    updatedEventName = updatedEventName_;
    updatedEventDataVariable = updatedEventDataVariable_;
  }

  UserEventsData() = delete;
};

/**
 * @brief Structure holding inference timing statistics.
 */
struct InferenceTime {
  long long int minInferenceTime;   /**< Minimum inference time recorded. */
  long long int maxInferenceTime;   /**< Maximum inference time recorded. */
  long long int totalInferenceTime; /**< Cumulative inference time. */
};

/**
 * @brief Configuration required for minimal initialization of the SDK.
 */
struct MinimalInitializationConfig {
  std::shared_ptr<Config> deviceConfig = nullptr; /**< Shared pointer to device configuration. */
  LoggerConfig externalLoggerConfig;              /**< Logger config for external logs. */
  LoggerConfig nimbleLoggerConfig;                /**< Logger config for internal logs. */

  MinimalInitializationConfig(std::shared_ptr<Config> deviceConfig_,
                              const LoggerConfig& externalLoggerConfig_,
                              const LoggerConfig& nimbleLoggerConfig_) {
    deviceConfig = deviceConfig_;
    externalLoggerConfig = externalLoggerConfig_;
    nimbleLoggerConfig = nimbleLoggerConfig_;
  };

  MinimalInitializationConfig(){};
};

/**
 * @brief Serializes a MinimalInitializationConfig to JSON.
 *
 * @param j The JSON object to populate.
 * @param wm The minimal config to serialize.
 */
static inline const void to_json(json& j, const MinimalInitializationConfig& wm) {
  j = nlohmann::json{{"deviceConfig", wm.deviceConfig->configJsonString},
                     {"externalLoggerConfig", wm.externalLoggerConfig},
                     {"nimbleLoggerConfig", wm.nimbleLoggerConfig}};
};

/**
 * @brief Deserializes a MinimalInitializationConfig from JSON.
 *
 * @param j The JSON object.
 * @param wm The MinimalInitializationConfig to populate.
 */
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

/**
 * @brief Aggregated metrics for a specific resource run.
 */
struct ResourceRunAggregates {
  int inferenceCount;          /**< Number of inference runs. */
  InferenceTime totalTime;     /**< Aggregated inference timing info. */
  std::string resourceVersion; /**< Version of the resource used. */
  std::string resourceName;    /**< Name of the resource. */
  std::string resourceType;    /**< Type of resource (e.g., model/script). */
  int deploymentId = -1;       /**< Deployment ID associated with the resource. */

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

/**
 * @brief Collects and logs metrics during SDK runtime.
 */
struct MetricsAgent {
  nlohmann::json metricsCollection; /**< Stores various metrics as JSON. */
  std::shared_ptr<Logger> metricsLogger =
      std::make_shared<Logger>(); /**< Logger for metric logs. */
  std::chrono::time_point<std::chrono::high_resolution_clock>
      lastMetricTime;            /**< Timestamp of last metric flush. */
  int _inferenceCount = 0;       /**< Tracks total inferences since last flush. */
  std::mutex _inferenceLogMutex; /**< Guards access to metric maps. */
  std::map<std::string, ResourceRunAggregates>
      _inferenceAggregates; /**< Aggregated inference metrics by model ID. */
  std::map<std::string, ResourceRunAggregates>
      _scriptRunAggregates; /**< Aggregated script execution metrics. */

  /**
   * @brief Constructs a MetricsAgent and initializes the last metric time to an earlier timestamp
   * to ensure first flush is not skipped.
   */
  MetricsAgent() {
    lastMetricTime = Time::get_high_resolution_clock_time() -
                     std::chrono::seconds(2 * loggerconstants::MetricsCollectionIntervalSecs);
  }

  /**
   * @brief Initializes the metrics logger.
   *
   * @param logger Shared pointer to a Logger instance to be used.
   */
  void initialize(std::shared_ptr<Logger> logger) { metricsLogger = logger; }

  /**
   * @brief Logs a custom metric as a JSON string using the logger.
   *
   * @param metricType String identifier for the type of metric.
   * @param metricJson JSON object containing the metric data.
   */
  void log_metrics(const char* metricType, const nlohmann::json& metricJson) {
    std::string jsonDump = metricJson.dump();
    metricsLogger->LOGMETRICS(metricType, jsonDump.c_str());
  }

  /**
   * @brief Records inference timing for a specific model.
   *
   * @param modelString Identifier for the model (e.g., name or path).
   * @param modelVersion Version string of the model.
   * @param deploymentId Deployment ID associated with the model.
   * @param androidTime Inference execution time in microseconds.
   */
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

  /**
   * @brief Records execution timing for a script method.
   *
   * @param methodString Name of the method or script.
   * @param scriptVersion Version string of the script.
   * @param deploymentId Deployment ID associated with the script.
   * @param androidTime Execution time in microseconds.
   */
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

  /**
   * @brief Flushes collected inference and script metrics if the count exceeds the threshold.
   *
   * @param inferenceMetricLogInterval Threshold after which metrics should be logged and reset.
   */
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

  /**
   * @brief Adds or updates metric data under a given metric type.
   *
   * @param metricType Identifier for the metric category.
   * @param metricJson JSON data to store under the category.
   */
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

/**
 * @brief A job responsible for logging metrics via a specified logger.
 */
struct LogJob : public Job<void> {
  int deploymentId;                     /**< ID of the deployment the log is associated with. */
  std::string type;                     /**< Type/category of the log. */
  nlohmann::json data;                  /**< JSON-formatted log data. */
  std::shared_ptr<Logger> _loggerToUse; /**< Logger instance to write the logs. */

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

/**
 * @brief Renames the current deployment file to the old deployment file.
 */
static inline void rename_deployment_to_old_deployment(std::shared_ptr<Config> config) noexcept {
  auto cloudConfigFileName = nativeinterface::get_full_file_path_common(
      config->compatibilityTag + coresdkconstants::DeploymentFileName);
  auto oldCloudConfigFileName = nativeinterface::get_full_file_path_common(
      config->compatibilityTag + coresdkconstants::OldDeploymentFileName);
  rename(cloudConfigFileName.c_str(), oldCloudConfigFileName.c_str());
}

/**
 * @brief Saves the deployment information to device storage.
 */
static inline bool save_deployment_on_device(const Deployment& deployment,
                                             const std::string& compatibilityTag) {
  return nativeinterface::save_file_on_device_common(
             nlohmann::json(deployment).dump(),
             compatibilityTag + coresdkconstants::DeploymentFileName) != "";
}

/**
 * @brief Reads stored session metrics and logs them using the provided MetricsAgent.
 */
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

/**
 * @brief Sleeps the current thread and updates the session duration metric.
 *
 * @param start Time point representing the start of a session.
 * @param threadSleepTimeUSecs Duration to sleep in microseconds.
 * @param sessionLength Previously recorded session duration.
 * @return Updated session duration.
 */
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

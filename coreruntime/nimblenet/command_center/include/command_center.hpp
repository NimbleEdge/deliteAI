/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <memory>

#include "config_manager.hpp"
#include "core_sdk_structs.hpp"
#include "job_scheduler.hpp"
#include "resource_downloader.hpp"
#include "resource_loader.hpp"
#include "resource_manager.hpp"
#include "server_api_structs.hpp"
#include "time_manager.hpp"
#include "user_events_manager.hpp"

#ifdef GENAI
#include "base_llm_executor.hpp"
#endif  // GENAI

class MapDataVariable;
class ServerAPI;
class Executor;
struct MetricsAgent;
class Database;
class Task;
class ScriptReadyJob;

/**
 * @brief Central coordinator for managing resources, inferences, logger, scheduler etc. Basically
 * the current state of SDK is managed by CommandCenter.
 */
class CommandCenter {
  bool _isReady = false;
  bool _retriesExhausted = false;
  std::unique_ptr<ResourceManager> _resourceManager;
  std::unique_ptr<ResourceLoader> _resourceLoader;
  std::unique_ptr<ResourceDownloader> _resourceDownloader;
  std::unique_ptr<UserEventsManager> _userEventsManager;

  std::shared_ptr<ServerAPI> _serverAPI = nullptr;
  std::shared_ptr<Config> _config = nullptr;
  MetricsAgent* _metricsAgent = nullptr;
  std::shared_ptr<Logger> _externalLogger = nullptr;
  std::mutex _tensorStoreMutex;
  std::unordered_map<int, std::shared_ptr<MapDataVariable>> _outputs;
  int _outputIndex = 0;
  std::string _missingResource = "STATE";
  PeggedDeviceTime _peggedDeviceTime;
  std::shared_ptr<JobScheduler> _jobScheduler;
  std::shared_ptr<Task> _task;
  bool _taskLoaded = false;
  Deployment _deployment;
  std::shared_ptr<ScriptReadyJob> _scriptReadyJob = nullptr;

  bool _currentState = true;

  /**
   * @brief Logs a user event to a file and returns related event data if applicable.
   *
   * @param userEventsData The input event data.
   * @param eventType The type of the event.
   * @return The resulting UserEventsData.
   */
  UserEventsData log_event_and_return_if_needed(const UserEventsData userEventsData,
                                                const std::string& eventType);

  /**
   * @brief Prepares and initializes the task.
   */
  void prepare_task();

 public:
  /**
   * @brief Constructs a CommandCenter instance.
   *
   * @param serverAPI Pointer to ServerAPI instance.
   * @param config Shared pointer to configuration.
   * @param metricsAgent Pointer to metrics agent.
   * @param database Pointer to database instance.
   * @param jobScheduler Shared pointer to job scheduler.
   * @param externalLogger Optional external logger.
   * @param currentState Current internal state (defaults to true).
   * @param deployment Deployment configuration.
   */
  CommandCenter(std::shared_ptr<ServerAPI> serverAPI, std::shared_ptr<Config> config,
                MetricsAgent* metricsAgent, Database* database,
                std::shared_ptr<JobScheduler> jobScheduler,
                std::shared_ptr<Logger> externalLogger = nullptr, bool currentState = true,
                const Deployment& deployment = Deployment());

  /** @brief Gets the current deployment ETag string. */
  std::string get_deployment_eTag() const { return _deployment.eTag; }

  /** @brief Gets the server API instance. */
  std::shared_ptr<ServerAPI> get_serverAPI() const { return _serverAPI; }

  /** @brief Gets the resource loader. */
  ResourceLoader& get_resource_loader() const { return *_resourceLoader; }

  /** @brief Gets the resource downloader. */
  ResourceDownloader& get_resource_downloader() const { return *_resourceDownloader; }

  /** @brief Gets the user events manager. */
  UserEventsManager& get_userEventsManager() const { return *_userEventsManager; }

  /** @brief Gets the loaded task. */
  std::shared_ptr<Task> get_task() const noexcept { return _task; }

  /** @brief Gets the resource manager. */
  ResourceManager& get_resource_manager() const { return *_resourceManager; }

  /** @brief Returns whether the task is still in initialization phase. */
  bool is_task_initializing() const noexcept { return !_taskLoaded; }

  /** @brief Gets the external logger. */
  std::shared_ptr<Logger> get_external_logger() const { return _externalLogger; }

  /** @brief Gets the configuration. */
  std::shared_ptr<Config> get_config() const { return _config; }

  /**
   * @brief Sets the current task by parsing its main module and adding a _scriptReadyJob in
   * scheduler.
   *
   * @param task Shared pointer to the task.
   */
  void set_task(std::shared_ptr<Task> task);

  /**
   * @brief Logs a metric in the metrics system.
   *
   * @param metricType Metric type as string.
   * @param metricJson Metric payload.
   */
  void log_metrics(const char* metricType, const nlohmann::json& metricJson);

  /**
   * @brief Updates the deployment configuration.
   *
   * @param cloudConfig New deployment configuration.
   */
  void update_deployment(const Deployment& cloudConfig);

  /**
   * @brief Adds a user event from a JSON string.
   *
   * @param eventMapJsonString Event payload as JSON string.
   * @param eventType Type of the event.
   * @return UserEventsData result.
   */
  UserEventsData add_user_event(const std::string& eventMapJsonString,
                                const std::string& eventType);

  /**
   * @brief Adds a user event from a structured object.
   *
   * @param event Event structure.
   * @param eventType Type of the event.
   * @return UserEventsData result.
   */
  UserEventsData add_user_event(const OpReturnType event, const std::string& eventType);

  /**
   * @brief Loads a task.
   *
   * @param taskName Name of the task.
   * @param taskVersion Version of the task.
   * @param taskCode AST for the task.
   * @return True if task was successfully loaded.
   */
  bool load_task(const std::string& taskName, const std::string& taskVersion,
                 std::string&& taskCode);

  /**
   * @brief Runs the task with raw tensor input/output.
   */
  NimbleNetStatus* run_task(const char* taskName, const char* functionName, const CTensors inputs,
                            CTensors* outputs);

  /**
   * @brief Runs the task with MapDataVariable input/output.
   */
  NimbleNetStatus* run_task(const char* taskName, const char* functionName,
                            std::shared_ptr<MapDataVariable> inputs,
                            std::shared_ptr<MapDataVariable> outputs);

  /** @brief Try to achieve a valid state in offline mode i.e. try to load the script and models
   * defined in the deployment by reading them from disk. */
  void achieve_state_in_offline_mode();

  /**
   * @brief Deallocates output memory.
   *
   * @param output Pointer to output tensor structure.
   * @return True if deallocation was successful.
   */
  bool deallocate_output_memory(CTensors* output);

  /** @brief Notifies that internet connection is restored. */
  void internet_switched_on();

  /** @brief Returns true if retries have been exhausted. */
  bool retries_exhausted() { return _retriesExhausted; }

  /**
   * @brief Internal ready-state check.
   *
   * @note Should not be used for external status.
   */
  bool is_ready() { return _isReady; }

  /** @brief Determines if CommandCenter is ready for external usage. */
  NimbleNetStatus* is_ready_for_exposing();

  /** @brief Returns reference to metrics agent. */
  MetricsAgent& get_metrics_agent() { return *_metricsAgent; }

  /**
   * @brief Writes inference-related metric.
   *
   * @param modelString Model identifier.
   * @param androidTime Timestamp in Android time.
   */
  void write_inference_metric(const std::string& modelString, long long int androidTime);

  /**
   * @brief Writes method invocation metric.
   *
   * @param methodName Name of the method.
   * @param androidTime Timestamp.
   */
  void write_run_method_metric(const char* methodName, long long int androidTime);

  /**
   * @brief Gets plan version associated with a model.
   *
   * @param modelId Model identifier.
   * @return The plan version string.
   */
  std::string get_plan_version(const std::string& modelId);

  /**
   * @brief Adds model ID to configuration.
   *
   * @param modelId Identifier of the model.
   */
  void add_modelId_in_config(const std::string& modelId);

  /** @brief Gets the deployment object. */
  Deployment get_deployment() const { return _deployment; }

  /** @brief Gets the deployment ID. */
  int get_deployment_id() { return _deployment.Id; }

  /**
   * @brief Placeholder for model reloading with endpoint config.
   *
   * @note Currently not implemented.
   */
  bool reload_model_with_epConfig(const char* modelName, const char* epConfig) { return false; }

  /** @brief Returns pegged device time reference. */
  const auto& pegged_device_time() const noexcept { return _peggedDeviceTime; }

  /**
   * @brief Updates pegged device time from config.
   *
   * @param cloudConfig Configuration containing pegged device time.
   */
  void pegged_device_time(const CloudConfigResponse& cloudConfig) noexcept {
    _peggedDeviceTime = cloudConfig.peggedDeviceTime;
  }

  /** @brief Gets the job scheduler. */
  auto job_scheduler() const noexcept { return _jobScheduler; }

  /**
   * @brief Marks CommandCenter as ready or not.
   *
   * @param value New ready state.
   */
  void set_is_ready(bool value) noexcept;

  /**
   * @brief Updates dependency for ScriptReadyJob.
   *
   * @param job Shared pointer to base job.
   */
  void update_dependency_of_script_ready_job(std::shared_ptr<BaseJob> job);

  /**
   * @brief Updates pegged device time.
   *
   * @param peggedDeviceTime New pegged device time.
   */
  void updated_pegged_device_time(const PeggedDeviceTime& peggedDeviceTime);

  /** @brief Returns whether this is current commandCenter being used for inferencing or just used
   * for saving a new state on device. */
  bool is_current() const noexcept { return _currentState; }

#ifdef TESTING
  /**
   * @brief Sets pegged device time (testing only).
   *
   * @param peggedDeviceTime New time to set.
   */
  void pegged_device_time(PeggedDeviceTime peggedDeviceTime) noexcept {
    _peggedDeviceTime = peggedDeviceTime;
  }
#endif  // TESTING

#ifdef GENAI
  /**
   * @brief Returns default LLM executor config.
   *
   * @note Temporary placeholder.
`   */
  auto get_llm_executor_config() const noexcept { return LLMExecutorConfig{}; }
#endif
};

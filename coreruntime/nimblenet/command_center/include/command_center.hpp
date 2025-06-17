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
  std::string _missingResource = "STATE";  // since initially missing resource is the STATE
  PeggedDeviceTime _peggedDeviceTime;
  std::shared_ptr<JobScheduler> _jobScheduler;
  std::shared_ptr<Task> _task;
  bool _taskLoaded = false;
  Deployment _deployment;
  std::shared_ptr<ScriptReadyJob> _scriptReadyJob = nullptr;

  bool _currentState = true;
  UserEventsData log_event_and_return_if_needed(const UserEventsData userEventsData,
                                                const std::string& eventType);

  void prepare_task();

 public:
  CommandCenter(std::shared_ptr<ServerAPI> serverAPI, std::shared_ptr<Config> config,
                MetricsAgent* metricsAgent, Database* database,
                std::shared_ptr<JobScheduler> jobScheduler,
                std::shared_ptr<Logger> externalLogger = nullptr, bool currentState = true,
                const Deployment& deployment = Deployment());

  std::string get_deployment_eTag() const { return _deployment.eTag; }

  std::shared_ptr<ServerAPI> get_serverAPI() const { return _serverAPI; }

  ResourceLoader& get_resource_loader() const { return *_resourceLoader; }

  ResourceDownloader& get_resource_downloader() const { return *_resourceDownloader; }

  UserEventsManager& get_userEventsManager() const { return *_userEventsManager; }

  std::shared_ptr<Task> get_task() const noexcept { return _task; }

  ResourceManager& get_resource_manager() const { return *_resourceManager; }

  bool is_task_initializing() const noexcept { return !_taskLoaded; }

  std::shared_ptr<Logger> get_external_logger() const { return _externalLogger; }

  std::shared_ptr<Config> get_config() const { return _config; }

  void set_task(std::shared_ptr<Task> task);

  void log_metrics(const char* metricType, const nlohmann::json& metricJson);

  void update_deployment(const Deployment& cloudConfig);

  UserEventsData add_user_event(const std::string& eventMapJsonString,
                                const std::string& eventType);
  UserEventsData add_user_event(const OpReturnType event, const std::string& eventType);
  bool load_task(const std::string& taskName, const std::string& taskVersion,
                 std::string&& taskCode);
  NimbleNetStatus* run_task(const char* taskName, const char* functionName, const CTensors inputs,
                            CTensors* outputs);
  NimbleNetStatus* run_task(const char* taskName, const char* functionName,
                            std::shared_ptr<MapDataVariable> inputs,
                            std::shared_ptr<MapDataVariable> outputs);
  void achieve_state_in_offline_mode();
  bool deallocate_output_memory(CTensors* output);
  void internet_switched_on();

  bool retries_exhausted() { return _retriesExhausted; }

  // is_ready() is an internal function, should not be called when returning status outside SDK.
  bool is_ready() { return _isReady; }

  NimbleNetStatus* is_ready_for_exposing();

  MetricsAgent& get_metrics_agent() { return *_metricsAgent; }

  void write_inference_metric(const std::string& modelString, long long int androidTime);
  void write_run_method_metric(const char* methodName, long long int androidTime);
  std::string get_plan_version(const std::string& modelId);
  void add_modelId_in_config(const std::string& modelId);

  Deployment get_deployment() const { return _deployment; }

  int get_deployment_id() { return _deployment.Id; }

  bool reload_model_with_epConfig(const char* modelName, const char* epConfig) { return false; }

  const auto& pegged_device_time() const noexcept { return _peggedDeviceTime; }

  void pegged_device_time(const CloudConfigResponse& cloudConfig) noexcept {
    _peggedDeviceTime = cloudConfig.peggedDeviceTime;
  }

  auto job_scheduler() const noexcept { return _jobScheduler; }

  void set_is_ready(bool value) noexcept;
  void update_dependency_of_script_ready_job(std::shared_ptr<BaseJob> job);

  void updated_pegged_device_time(const PeggedDeviceTime& peggedDeviceTime);

  bool is_current() const noexcept { return _currentState; }

#ifdef TESTING
  void pegged_device_time(PeggedDeviceTime peggedDeviceTime) noexcept {
    _peggedDeviceTime = peggedDeviceTime;
  }
#endif  // TESTING

#ifdef GENAI
  // Temporary until we have an LLM asset
  // TODO: Fix this by getting actual executor config from somewhere
  auto get_llm_executor_config() const noexcept { return LLMExecutorConfig{}; }
#endif
};

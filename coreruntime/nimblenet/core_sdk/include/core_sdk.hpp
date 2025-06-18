/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "config_manager.hpp"
#include "core_sdk_constants.hpp"
#include "core_sdk_structs.hpp"
#include "data_variable.hpp"
#include "executor_structs.h"
#include "job_scheduler.hpp"
#include "log_sender.hpp"
#include "map_data_variable.hpp"
#include "resource_loader.hpp"
#include "resource_manager.hpp"
#include "result.hpp"
#include "server_api.hpp"
#include "user_events_manager.hpp"
#include "util.hpp"

class TaskManager;
class ResourceManager;
class Executor;
class LogSender;
class UserEventsManager;
class Config;

class CoreSDK {
  // Should only be accessed by the atomic_* functions
  std::shared_ptr<CommandCenter> _atomicCommandCenter = nullptr;
  std::atomic<bool> _threadRunning = false;

  /// @brief  Set these  to their default values in internet_switched_on callback as well
  std::atomic<int> _cloudConfigFetchRetries = coresdkconstants::DefaultFetchCloudConfigRetries;
  std::atomic<int> _sendCrashLogRetries = coresdkconstants::DefaultSendCrashLogRetries;
  std::atomic<int> _threadPriorityTries = coresdkconstants::DefaultThreadPriorityTries;
  //////////////////////////////////////////////////////////////////////////////////////

  std::mutex _initMutex;  // ensures only one initialize is ever called
  ne::NullableAtomicPtr<ServerAPI> _atomicServerAPI;
  std::shared_ptr<LogSender> _atomicExternalSender = nullptr;
  std::shared_ptr<Logger> _atomicExternalLogger = nullptr;
  LogSender* _logSender = nullptr;
  Database* _database = nullptr;
  std::shared_ptr<Config> _config = nullptr;
  std::atomic<bool> _initializeSuccess = false;
  std::atomic<bool> _commandCenterReady = false;
  CloudConfigResponse _deviceConfiguration;
  bool _cloudConfigFetched = false;
  MetricsAgent _metricsAgent;
  std::thread _cmdThread;
  std::shared_ptr<JobScheduler> _jobScheduler;

  bool _isCleanupDone = false;
  Executor* _mlExecutor = nullptr;

#ifdef SIMULATION_MODE
  // Stores the user events given by the user in simulator
  std::vector<nlohmann::json> _simulatedUserEvents;
#endif

  Deployment load_deployment_from_device();
  Deployment load_old_deployment_from_device();

  std::shared_ptr<CommandCenter> command_center() {
    if (!_commandCenterReady.load()) return nullptr;
    return std::atomic_load(&_atomicCommandCenter);
  }

  std::shared_ptr<ServerAPI> serverAPI() { return _atomicServerAPI.load(); }

  std::shared_ptr<Logger> externalLogger() { return std::atomic_load(&_atomicExternalLogger); }

  std::shared_ptr<LogSender> externalLogSender() {
    return std::atomic_load(&_atomicExternalSender);
  }

  void new_command_center(const Deployment& cloudConfig);
  void replace_command_center(const Deployment& cloudConfig);
  void schedule_work_manager(const CloudConfigResponse& cloudConfig);
  bool save_cloud_config_on_device(const CloudConfigResponse& cloudConfig);
  std::string get_latest_eTag();

  Deployment load_deployment_offline();
  void perform_long_running_tasks();
  void send_logs_and_metrics();
  void thread_initializer();
  void update_resource_configs(const CloudConfigResponse& validCoreSDKConfig);
  void send_crash_logs();
  void atomic_repeatable_minimal_initialize(const MinimalInitializationConfig& minInitConfig);
  NimbleNetStatus* process_add_user_event_response(const UserEventsData& userEventsData,
                                                   CUserEventsData* cUserEventsData);
#ifdef SIMULATION_MODE
  bool add_simulation_user_events(nlohmann::json& eventMapJsonString, const std::string& tableName);
  bool validateUserEvent(nlohmann::json& userEventJson);
#endif

 public:
  CoreSDK() = default;
  ~CoreSDK();
  void initialize_coreSDK();
  void load_cloud_config_from_device();

  NimbleNetStatus* add_user_event(const std::string& eventMapJsonString,
                                  const std::string& eventType, CUserEventsData* cUserEventsData);
  NimbleNetStatus* add_user_event(const OpReturnType event, const std::string& eventType,
                                  CUserEventsData* cUserEventsData);

  std::pair<CloudConfigResponse, Deployment> get_cloud_config_and_update_configurations();

  void update_session(const std::string& sessionIdString) { util::set_session_id(sessionIdString); }

  MetricsAgent& get_metrics_agent() { return _metricsAgent; }

  std::shared_ptr<Config> get_config() const { return _config; }

  void log_metrics(const char* metricType, const nlohmann::json& metric);

  void write_metric(const char* metricType, const char* metricJson);

  void internet_switched_on();
  bool save_labels_for_inference_input(const std::string& modelId, const InferenceRequest& inputs,
                                       const InferenceRequest& labels);

  void write_run_method_metric(const char* methodName, long long int androidTime);

  NimbleNetStatus* initialize(std::shared_ptr<Config> config);
  bool load_task(const std::string& taskName, const std::string& taskVersion,
                 std::string&& taskCode);

  NimbleNetStatus* run_task(const char* taskName, const char* functionName, const CTensors inputs,
                            CTensors* outputs);

  NimbleNetStatus* run_task(const char* taskName, const char* functionName,
                            std::shared_ptr<MapDataVariable> inputs,
                            std::shared_ptr<MapDataVariable> outputs);

  bool deallocate_output_memory(CTensors* output);

  bool reload_model_with_epConfig(const char* modelName, const char* epConfig);
  bool load_model_from_file(const char* modelFilePath, const char* inferenceConfigFilePath,
                            const char* modelId, const char* epConfigJsonChar);
  bool send_events(const char* params);
  NimbleNetStatus* is_ready();

  static void attach_cleanup_to_thread();
  void achieve_state();

#ifdef TESTING
  CoreSDK(std::shared_ptr<Config> config) { _config = config; };
#endif

#ifdef SIMULATION_MODE
  bool add_events_from_file(const char* userEventsInputFilePath, const char* tableName);
  bool add_events_from_buffer(const char* userEventsInputBuffer, const char* tableName);

  bool add_simulation_user_events_upto_timestamp(int64_t timestamp);
  bool run_task_upto_timestamp(const char* taskName, const char* functionName, const CTensors input,
                               CTensors* output, int64_t timestamp);
#endif  // SIMULATION_MODE
};

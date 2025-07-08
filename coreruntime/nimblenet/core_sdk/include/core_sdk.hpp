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
#ifdef SIMULATION_MODE
#include "nimblenet_py_interface.hpp"
#endif  // SIMULATION_MODE

class TaskManager;
class ResourceManager;
class Executor;
class LogSender;
class UserEventsManager;
class Config;

/**
 * @brief CoreSDK is the main interface for managing SDK lifecycle across deployments.
 *
 * It has the following main functions:
 * 1. During SDK initialization, CoreSDK makes sure that the correct
 *    deployment from disk/SaaS platform is loaded and corresponding CommandCenter is created.
 * 2. CoreSDK also starts a background thread which makes all the network calls for downloading
 *    assets or sending logs.
 * 3. Acts as an interface between nimblenet.cpp and commandCenter. All the public APIs exposed via
 *    nimblenet.hpp are routed to the correct commandCenter via CoreSDK.
 */
class CoreSDK {
  std::shared_ptr<CommandCenter> _atomicCommandCenter =
      nullptr;                              /**< Current CommandCenter being used by the SDK. */
  std::atomic<bool> _threadRunning = false; /**< Indicates if the background thread is active. */

  /**< Retries and priority values reset when internet is turned back on. */
  std::atomic<int> _cloudConfigFetchRetries = coresdkconstants::DefaultFetchCloudConfigRetries;
  std::atomic<int> _sendCrashLogRetries = coresdkconstants::DefaultSendCrashLogRetries;
  std::atomic<int> _threadPriorityTries = coresdkconstants::DefaultThreadPriorityTries;

  std::mutex _initMutex; /**< Ensures thread-safe initialization. */
  ne::NullableAtomicPtr<ServerAPI>
      _atomicServerAPI; /**< Atomic pointer to the Server API interface. */
  std::shared_ptr<LogSender> _atomicExternalSender =
      nullptr; /**< Shared reference to external log sender. */
  std::shared_ptr<Logger> _atomicExternalLogger =
      nullptr; /**< Shared reference to external logger. */

  LogSender* _logSender = nullptr;           /**< LogSender instance for handling logs. */
  Database* _database = nullptr;             /**< Local database reference for storage. */
  std::shared_ptr<Config> _config = nullptr; /**< Configuration object for the SDK. */
  std::atomic<bool> _initializeSuccess =
      false; /**< True if initialization completed successfully. */
  std::atomic<bool> _commandCenterReady = false; /**< True if CommandCenter is initialized. */

  CloudConfigResponse _deviceConfiguration; /**< Cached configuration retrieved from device. */
  bool _cloudConfigFetched = false; /**< Whether cloud config has been fetched at least once. */
  MetricsAgent _metricsAgent;       /**< Agent to collect and emit metrics. */
  std::thread _cmdThread;           /**< Thread for background command center tasks. */
  std::shared_ptr<JobScheduler>
      _jobScheduler; /**< Schedules background jobs like uploads or syncs. */

  bool _isCleanupDone = false;     /**< True if cleanup has been performed after shutdown. */
  Executor* _mlExecutor = nullptr; /**< Pointer to ML execution engine. */

#ifdef SIMULATION_MODE
  std::vector<nlohmann::json>
      _simulatedUserEvents; /**< Stores simulated events in simulator mode. */

#endif  // SIMULATION_MODE

  /**
   * @brief Load deployment configuration from device storage.
   */
  Deployment load_deployment_from_device();

  /**
   * @brief Load fallback (older) deployment configuration.
   */
  Deployment load_old_deployment_from_device();

  /**
   * @brief Returns the initialized CommandCenter.
   */
  std::shared_ptr<CommandCenter> command_center() {
    if (!_commandCenterReady.load()) return nullptr;
    return std::atomic_load(&_atomicCommandCenter);
  }

  std::shared_ptr<ServerAPI> serverAPI() { return _atomicServerAPI.load(); }

  std::shared_ptr<Logger> externalLogger() { return std::atomic_load(&_atomicExternalLogger); }

  std::shared_ptr<LogSender> externalLogSender() {
    return std::atomic_load(&_atomicExternalSender);
  }

  /**
   * @brief Creates a new CommandCenter.
   */
  void new_command_center(const Deployment& cloudConfig);

  /**
   * @brief Replace old commandCenter with the new one.
   */
  void replace_command_center(const Deployment& cloudConfig);

  /**
   * @brief Schedule logs upload in background.
   */
  void schedule_work_manager(const CloudConfigResponse& cloudConfig);

  /**
   * @brief Saves cloud config locally.
   */
  bool save_cloud_config_on_device(const CloudConfigResponse& cloudConfig);

  /**
   * @brief Returns the latest cloud config etag.
   */
  std::string get_latest_eTag();

  /**
   * @brief Loads deployment in offline mode i.e. with the files present on disk.
   */
  Deployment load_deployment_offline();

  /**
   * @brief Function used by the background thread to make API calls and send logs.
   */
  void perform_long_running_tasks();

  /**
   * @brief Flush logs/metrics and send via API.
   */
  void send_logs_and_metrics();

  /**
   * @brief Creates a new instance of thread and invokes perform_long_running_task function.
   */
  void thread_initializer();

  /**
   * @brief Updates server and logger configs.
   */
  void update_resource_configs(const CloudConfigResponse& validCoreSDKConfig);

  /**
   * @brief Send crash logs.
   */
  void send_crash_logs();

  /**
   * @brief Does the minimal initialize of SDK, this is done so we can get logs even if there are
   * failure in fetching deployment from cloud.
   */
  void atomic_repeatable_minimal_initialize(const MinimalInitializationConfig& minInitConfig);
  NimbleNetStatus* process_add_user_event_response(const UserEventsData& userEventsData,
                                                   CUserEventsData* cUserEventsData);
  NimbleNetStatus* process_module_info(const nlohmann::json assetsJson, const std::string& homeDir);
  /**
   * @brief Copies an asset from the provided asset.location to homedir with the name accepted by
   * SDK.
   */
  static void copy_module(const std::shared_ptr<Asset> asset, Deployment& deployment,
                          bool addToDeployment);
#ifdef SIMULATION_MODE
  /**
   * @brief Add simulated user events in internal vector.
   */
  bool add_simulation_user_events(nlohmann::json& eventMapJsonString, const std::string& tableName);
  /**
   * @brief Validates whether the event sent as input is valid.
   */
  bool validateUserEvent(nlohmann::json& userEventJson);
#endif

 public:
  /**
   * @brief Default constructor.
   */
  CoreSDK() = default;

  /**
   * @brief Destructor to ensure cleanup.
   */
  ~CoreSDK();

  /**
   * @brief Initializes the CoreSDK.
   */
  void initialize_coreSDK();

  /**
   * @brief Loads previously saved cloud configuration.
   */
  void load_cloud_config_from_device();

  /**
   * @brief Adds a user event with raw JSON string input.
   */
  NimbleNetStatus* add_user_event(const std::string& eventMapJsonString,
                                  const std::string& eventType, CUserEventsData* cUserEventsData);

  /**
   * @brief Adds a structured user event.
   */
  NimbleNetStatus* add_user_event(const OpReturnType event, const std::string& eventType,
                                  CUserEventsData* cUserEventsData);

  /**
   * @brief Fetches the latest cloud config and updates SDK configurations.
   */
  std::pair<CloudConfigResponse, Deployment> get_cloud_config_and_update_configurations();

  /**
   * @brief Updates the session ID used in logs and metrics.
   */
  void update_session(const std::string& sessionIdString) { util::set_session_id(sessionIdString); }

  /**
   * @brief Returns reference to the metrics agent.
   */
  MetricsAgent& get_metrics_agent() { return _metricsAgent; }

  /**
   * @brief Returns current configuration.
   */
  std::shared_ptr<Config> get_config() const { return _config; }

  /**
   * @brief Logs a metric using internal logger.
   */
  void log_metrics(const char* metricType, const nlohmann::json& metric);

  /**
   * @brief Logs a raw JSON metric string.
   */
  void write_metric(const char* metricType, const char* metricJson);

  /**
   * @brief Callback for when internet is detected.
   */
  void internet_switched_on();

  /**
   * @brief Saves labels corresponding to inference inputs.
   *
   * @note To be deprecated.
   */
  bool save_labels_for_inference_input(const std::string& modelId, const InferenceRequest& inputs,
                                       const InferenceRequest& labels);

  /**
   * @brief Logs execution time of a script or method.
   */
  void write_run_method_metric(const char* methodName, long long int androidTime);

  /**
   * @brief Initializes SDK with configuration.
   */
  NimbleNetStatus* initialize(std::shared_ptr<Config> config);

  /**
   * @brief Loads a task into memory.
   */
  bool load_task(const std::string& taskName, const std::string& taskVersion,
                 std::string&& taskCode);

  /**
   * @brief Runs a task with raw C-style tensors.
   */
  NimbleNetStatus* run_task(const char* taskName, const char* functionName, const CTensors inputs,
                            CTensors* outputs);

  /**
   * @brief Runs a task using map-style inputs and outputs.
   */
  NimbleNetStatus* run_task(const char* taskName, const char* functionName,
                            std::shared_ptr<MapDataVariable> inputs,
                            std::shared_ptr<MapDataVariable> outputs);

  /**
   * @brief Frees memory allocated for outputs.
   */
  bool deallocate_output_memory(CTensors* output);

  NimbleNetStatus* load_modules(const char* assetsJson, const char* homeDir);

  NimbleNetStatus* load_modules(const OpReturnType assetsJson, const std::string& homeDir);

  NimbleNetStatus* load_modules(const nlohmann::json assetsJson, const std::string& homeDir);

  /**
   * @brief Reloads a model with a given execution provider config.
   */
  bool reload_model_with_epConfig(const char* modelName, const char* epConfig);

  /**
   * @brief Loads a model from local files.
   */
  bool load_model_from_file(const char* modelFilePath, const char* inferenceConfigFilePath,
                            const char* modelId, const char* epConfigJsonChar);

  /**
   * @brief Sends events to the server.
   */
  bool send_events(const char* params);

  /**
   * @brief Checks whether the SDK is ready for operations.
   */
  NimbleNetStatus* is_ready();

  /**
   * @brief Registers cleanup callback for the current thread.
   */
  static void attach_cleanup_to_thread();

  /**
   * @brief Ensures CoreSDK reaches expected initialization state.
   */
  void achieve_state();

#ifdef TESTING
  /**
   * @brief Constructor for testing with direct config injection.
   */
  CoreSDK(std::shared_ptr<Config> config) { _config = config; };
#endif

#ifdef SIMULATION_MODE
  /**
   * @brief Adds user events from a local file.
   */
  bool add_events_from_file(const char* userEventsInputFilePath, const char* tableName);

  /**
   * @brief Adds user events from a memory buffer.
   */
  bool add_events_from_buffer(const char* userEventsInputBuffer, const char* tableName);

  /**
   * @brief Adds simulation user events up to a specific timestamp.
   */
  bool add_simulation_user_events_upto_timestamp(int64_t timestamp);

  /**
   * @brief Runs a task with inputs until a timestamp is reached.
   */
  bool run_task_upto_timestamp(const char* taskName, const char* functionName, const CTensors input,
                               CTensors* output, int64_t timestamp);
#endif  // SIMULATION_MODE
};

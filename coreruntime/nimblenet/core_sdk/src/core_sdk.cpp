/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core_sdk.hpp"

#include <dirent.h>
#include <sched.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <fstream>
#include <list>
#include <memory>
#include <vector>

#include "command_center.hpp"
#include "core_sdk_constants.hpp"
#include "core_sdk_structs.hpp"
#include "data_variable.hpp"
#include "executor_structs.h"
#include "job_scheduler.hpp"
#include "map_data_variable.hpp"
#include "native_interface.hpp"
#include "nimble_exec_info.hpp"
#include "server_api_structs.hpp"
#include "time_manager.hpp"
#include "util.hpp"

using namespace std;

#if defined(__APPLE__) && defined(__MACH__)
#define FORCE_USED_SECTION \
  __attribute__((used)) __attribute__((section("__NIMBLE_METADATA,nimble_metadata")))
#else
#define FORCE_USED_SECTION __attribute__((used)) __attribute__((section("nimble_metadata")))
#endif

// These variables are populated by CMake, and these are stored in nimble_metadata section of
// resulting library
FORCE_USED_SECTION static inline const char NIMBLE_git_rev[] = "Rev: " NIMBLE_GIT_REV;
FORCE_USED_SECTION static inline const char NIMBLE_git_branch[] = "Branch: " NIMBLE_GIT_BRANCH;

std::shared_ptr<Logger> logger = std::make_shared<Logger>(LogWritingConfig());

void CoreSDK::thread_initializer() {
  _threadRunning = true;
  _cmdThread = thread(&CoreSDK::perform_long_running_tasks, this);
}

// All initialization steps called here, either should be thread safe and idempotent. Also, you
// should not use any CoreSDK variable inside this method. Everything needed here should be passed
// into the method
void CoreSDK::atomic_repeatable_minimal_initialize(
    const MinimalInitializationConfig& minimalInitConfig) {
  logger->update_log_config(minimalInitConfig.nimbleLoggerConfig.writerConfig);
  _metricsAgent.initialize(logger);

  _atomicServerAPI.store(
      std::make_shared<ServerAPI>(&_metricsAgent, minimalInitConfig.deviceConfig));

  auto externalLoggerPtr =
      std::make_shared<Logger>(minimalInitConfig.externalLoggerConfig.writerConfig);
  externalLoggerPtr->set_max_size_limit(minimalInitConfig.deviceConfig->maxEventsSizeKBs);
  externalLoggerPtr->init_logger(nativeinterface::HOMEDIR + loggerconstants::ExternalLogDir);
  std::atomic_store(&_atomicExternalLogger, externalLoggerPtr);

  auto externalSender =
      std::make_shared<LogSender>(serverAPI(), minimalInitConfig.deviceConfig, externalLogger(),
                                  minimalInitConfig.externalLoggerConfig.senderConfig);
  std::atomic_store(&_atomicExternalSender, externalSender);
}

void CoreSDK::initialize_coreSDK() {
  auto minimalInitConfig = MinimalInitializationConfig(
      _config, _deviceConfiguration.externalLoggerConfig, _deviceConfiguration.nimbleLoggerConfig);

  atomic_repeatable_minimal_initialize(minimalInitConfig);

  _logSender = new LogSender(serverAPI(), _config, logger,
                             _deviceConfiguration.nimbleLoggerConfig.senderConfig);
  _jobScheduler = std::make_shared<JobScheduler>(coresdkconstants::JobSchedulerCapacity);

  auto deployment = load_deployment_offline();

  replace_command_center(deployment);
  return;
}

void CoreSDK::load_cloud_config_from_device() {
  string cloudConfigString;
  if (!nativeinterface::get_file_from_device_common(
          _config->compatibilityTag + coresdkconstants::CloudConfigFileName, cloudConfigString)) {
    return;
  }
  _deviceConfiguration = jsonparser::get<CloudConfigResponse>(cloudConfigString);
  // try loading current cloud config

  if (_deviceConfiguration.state != CloudConfigState::Valid) {
    // DISK CLOUD CONFIG IS ALWAYS VALID, NEVER UNMODIFIED
    LOG_TO_DEBUG("%s",
                 "Failed to load cloudConfig from device. Using default values of Cloud Config");
    return;
  }
}

Deployment CoreSDK::load_deployment_offline() {
  // try loading current cloud config
  auto deployment = load_deployment_from_device();
  if (deployment.Id == -1) {
    // current config unavailable, use oldCloudConfig
    deployment = load_old_deployment_from_device();
    if (deployment.Id != -1) {
      // if old cloudConfig available save as current cloud config
      util::save_deployment_on_device(deployment, _config->compatibilityTag);
    }
  }
  return deployment;
}

NimbleNetStatus* CoreSDK::initialize(std::shared_ptr<Config> config) {
  if (!_initMutex.try_lock()) {
    return util::nimblestatus(
        TERMINAL_ERROR, "%s",
        "Initialization is already in progress, might be called from different thread");
  }
  std::lock_guard<std::mutex> guard(_initMutex, std::adopt_lock);
  if (_initializeSuccess) {
    LOG_TO_CLIENT_ERROR("%s", "NimbleNet is already initialized");
    return nullptr;
  }
  LOG_TO_CLIENT_INFO("%s", "Initializing NimbleNet");
  _config = config;

  // populates deviceConfiguration which is used for Minimal Repeatable Initialization
  if (_config->online) {
    load_cloud_config_from_device();
  }

  // uses deviceConfiguration and picks up deployment from device to initialize CommandCenter and
  // other CoreSDK components
  initialize_coreSDK();

  // updates all the coreSDK resources with picked up deviceConfiguration
  if (_deviceConfiguration.state != CloudConfigState::Invalid) {
    update_resource_configs(_deviceConfiguration);
  }

  _jobScheduler->do_all_non_priority_jobs();
  if (_config->online) {
    thread_initializer();
  }

  _initializeSuccess = true;
  LOG_TO_CLIENT_INFO("%s", "Initialize NimbleNet succeeded.");
  return nullptr;
}

void CoreSDK::send_crash_logs() {
  if (_sendCrashLogRetries <= 0) return;
  std::string crashFileName = nativeinterface::HOMEDIR + "/segfault.log";
  FILE* file = fopen(crashFileName.c_str(), "r");
  if (file != NULL) {
    // Deletion of log file after sending is handled in logSender itself
    fclose(file);
    auto status = _logSender->send_logs({crashFileName});
    if (!status) {
      _sendCrashLogRetries--;
    }
  }
}

void CoreSDK::attach_cleanup_to_thread() {
  struct sigaction sa;
  sa.sa_sigaction = ne::handle_crash_signal;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO & SA_RESETHAND;
  sigaction(SIGSEGV, &sa, nullptr);
  sigaction(SIGABRT, &sa, nullptr);
  sigaction(SIGILL, &sa, nullptr);
  sigaction(SIGFPE, &sa, nullptr);
  sigaction(SIGBUS, &sa, nullptr);
  // sigaction(SIGTERM, &sa, nullptr);
  sigaction(SIGINT, &sa, nullptr);
  sigaction(SIGTRAP, &sa, nullptr);
}

void CoreSDK::schedule_work_manager(const CloudConfigResponse& cloudConfig) {
  auto workManagerConfig = nlohmann::json(MinimalInitializationConfig(
      _config, cloudConfig.externalLoggerConfig, cloudConfig.nimbleLoggerConfig));

  // Explicit addition of secret key into json for usage in the arguments that came from Work
  // Manager
  std::string workManagerConfigString;
  try {
    workManagerConfig["externalLogger"]["sender"]["key"] =
        cloudConfig.externalLoggerConfig.senderConfig._secretKey;
    workManagerConfig["nimbleLogger"]["sender"]["key"] =
        cloudConfig.nimbleLoggerConfig.senderConfig._secretKey;
    workManagerConfigString = workManagerConfig.dump();
  } catch (...) {
    LOG_TO_ERROR("%s", "Error in adding security keys to work Manager");
  }

  if (!nativeinterface::schedule_logs_upload(
          cloudConfig.externalLoggerConfig.senderConfig.background_timer_interval,
          cloudConfig.externalLoggerConfig.senderConfig.background_timer_interval,
          workManagerConfigString.c_str())) {
    LOG_TO_WARN("%s", "Could not schedule Logs upload in work Manager");
  }
}

void CoreSDK::new_command_center(const Deployment& deployment) {
  // This creates a ScriptReadyJob internally that will delete the new CommandCenter when finished
  static_cast<void>(new CommandCenter(serverAPI(), get_config(), &_metricsAgent, _database,
                                      _jobScheduler, nullptr, false, deployment));
}

void CoreSDK::replace_command_center(const Deployment& deployment) {
  if (deployment.Id == -1) return;

  _commandCenterReady.store(false);

  _database = new Database(&_metricsAgent);
  auto commandCenter =
      std::make_shared<CommandCenter>(serverAPI(), get_config(), &_metricsAgent, _database,
                                      _jobScheduler, externalLogger(), true, deployment);

  std::atomic_store(&_atomicCommandCenter, commandCenter);

  _commandCenterReady.store(true);
  util::save_deployment_on_device(deployment, _config->compatibilityTag);
}

void CoreSDK::achieve_state() {
  serverAPI()->init();
#ifndef SIMULATION_MODE
  send_crash_logs();
#endif

  if (_cloudConfigFetched) {
    return;
  }
  _threadPriorityTries--;

  auto [cloudConfig, deployment] = get_cloud_config_and_update_configurations();
  if (cloudConfig.state == CloudConfigState::Invalid) {
    return;
  }
  _cloudConfigFetched = true;
  // Here it is ensured that the cloud config is definitely fetched;

  if (cloudConfig.state == CloudConfigState::Unmodified) {
  } else {
    _deviceConfiguration = cloudConfig;
    if (deployment.forceUpdate) {
      replace_command_center(deployment);
    } else if (_commandCenterReady.load() && command_center()->is_ready()) {
      if (command_center()->get_deployment_id() != deployment.Id) {
        new_command_center(deployment);
      }
    } else {
      replace_command_center(deployment);
    }
  }

  // Logic executed irrespective of cloud unmodified or valid
  schedule_work_manager(cloudConfig);
  // Guaranteed that commandCenter is definitely initialized and stored in atomicCommandCenter
  if (command_center()->is_ready() || _threadPriorityTries == 0) {
    nativeinterface::set_thread_priority_min();
  }
}

void CoreSDK::perform_long_running_tasks() {
  LOG_TO_DEBUG("%s", "Initiating the long running tasks.");
  CoreSDK::attach_cleanup_to_thread();
  auto commandCenter = command_center();
  if (commandCenter != nullptr && command_center()->is_ready())
    nativeinterface::set_thread_priority_min();
  else
    nativeinterface::set_thread_priority_max();

  int64_t sessionLength = 0;
  util::read_session_metrics(coresdkconstants::SessionFilePath, &_metricsAgent);

  while (_threadRunning) {
    auto start = Time::get_high_resolution_clock_time();
    achieve_state();
    _jobScheduler->do_jobs();

    util::delete_extra_files(nativeinterface::HOMEDIR, _deviceConfiguration.fileDeleteTimeInDays);

    send_logs_and_metrics();

    sessionLength = util::sleep_flush_and_update_session_time(
        start, _deviceConfiguration.threadSleepTimeUSecs, sessionLength);
  }
  LOG_TO_INFO("%s", "Completed running thread");
}

void CoreSDK::internet_switched_on() {
  if (!_commandCenterReady.load()) {
    return;
  }
  _cloudConfigFetchRetries = coresdkconstants::DefaultFetchCloudConfigRetries;
  _sendCrashLogRetries = coresdkconstants::DefaultSendCrashLogRetries;
  serverAPI()->reset_register_retries();
  auto commandCenter = command_center();
  commandCenter->internet_switched_on();
  _logSender->reset_sender_retries();
  _jobScheduler->notify_online();
  if (!commandCenter->is_ready()) {
    _threadPriorityTries = coresdkconstants::DefaultThreadPriorityTries;
  }
}

void CoreSDK::send_logs_and_metrics() {
  _metricsAgent.flush_inference_metrics(_deviceConfiguration.inferenceMetricLogInterval);
  _logSender->send_pending_logs();
  if (_deviceConfiguration.externalLoggerConfig.writerConfig.collectEvents) {
    externalLogSender()->send_pending_logs();
  }
}

void CoreSDK::update_resource_configs(const CloudConfigResponse& validCoreSDKConfig) {
  assert(validCoreSDKConfig.state == CloudConfigState::Valid);

  // DO NOT NEED TO UPDATE LOGGER CONFIG HERE SINCE IT IS PART OF MINIMAL INIT UPDATES
  // logger->update_log_config(validCoreSDKConfig.nimbleLoggerConfig.writerConfig);
  _logSender->update_sender_config(validCoreSDKConfig.nimbleLoggerConfig.senderConfig);

  _metricsAgent.metricsLogger->update_log_config(
      validCoreSDKConfig.nimbleLoggerConfig.writerConfig);

  serverAPI()->update_request_to_host_map(validCoreSDKConfig.requestToHostMap);

  serverAPI()->update_ads_host(validCoreSDKConfig.adsHost);

  externalLogger()->update_log_config(validCoreSDKConfig.externalLoggerConfig.writerConfig);

  externalLogSender()->update_sender_config(validCoreSDKConfig.externalLoggerConfig.senderConfig);
}

Deployment CoreSDK::load_deployment_from_device() {
  string cloudConfigString;
  if (!nativeinterface::get_file_from_device_common(
          _config->compatibilityTag + coresdkconstants::DeploymentFileName, cloudConfigString)) {
    return Deployment();
  }
  return jsonparser::get<Deployment>(cloudConfigString);
}

Deployment CoreSDK::load_old_deployment_from_device() {
  string cloudConfigString;
  if (!nativeinterface::get_file_from_device_common(
          _config->compatibilityTag + coresdkconstants::OldDeploymentFileName, cloudConfigString)) {
    return Deployment();
  }
  return jsonparser::get<Deployment>(cloudConfigString);
}

bool CoreSDK::save_cloud_config_on_device(const CloudConfigResponse& cloudConfig) {
  return nativeinterface::save_file_on_device_common(
             nlohmann::json(cloudConfig).dump(),
             _config->compatibilityTag + coresdkconstants::CloudConfigFileName) != "";
}

std::string CoreSDK::get_latest_eTag() {
  if (!_commandCenterReady.load()) return "";
  return command_center()->get_deployment_eTag();
}

/////////////////////////////////////////////////////////////////////

std::pair<CloudConfigResponse, Deployment> CoreSDK::get_cloud_config_and_update_configurations() {
  if (_cloudConfigFetchRetries == 0) return {CloudConfigResponse(), Deployment()};
  _cloudConfigFetchRetries--;
  // In case of 304, cloudConfig and deployment will not be present
  auto [cloudConfig, deployment] = serverAPI()->get_cloud_config(get_latest_eTag());
  switch (cloudConfig.state) {
    case CloudConfigState::Invalid:
      return {CloudConfigResponse(), Deployment()};
    case CloudConfigState::Valid: {
      update_resource_configs(cloudConfig);
      save_cloud_config_on_device(cloudConfig);
      return {cloudConfig, deployment};
    }
    case CloudConfigState::Unmodified:
      return {cloudConfig, deployment};
  }
}

void CoreSDK::log_metrics(const char* metricType, const nlohmann::json& metric) {
  _metricsAgent.log_metrics(metricType, metric);
}

void CoreSDK::write_metric(const char* metricType, const char* metricJson) {
  if (!_commandCenterReady.load()) {
    return;
  }
  _metricsAgent.metricsLogger->LOGMETRICS(metricType, metricJson);
}

void CoreSDK::write_run_method_metric(const char* methodName, long long int androidTime) {
  if (!_commandCenterReady.load()) {
    return;
  }
#ifdef SCRIPTING
  auto commandCenter = command_center();
  _metricsAgent.write_run_method_metric(methodName, commandCenter->get_task()->get_version(),
                                        commandCenter->get_deployment_id(), androidTime);
  return;
#endif
  LOG_TO_ERROR("%s", "Scripting not enabled");
}

template <typename T>
using CoreSDKResult = ne::Result<T, NimbleNetStatus*>;

static inline CoreSDKResult<CUserEventsData> get_C_UserEventsData(UserEventsData userEventsData) {
  CUserEventsData data;

  if (userEventsData.status != nullptr) {
    return {userEventsData.status};
  }

  if (userEventsData.updatedEventDataVariable == nullptr) {
    data.eventType = NULL;
    data.eventJsonString = NULL;
    return {data};
  }

  size_t eventTypeSize = userEventsData.updatedEventName.length();
  data.eventType = (char*)malloc((eventTypeSize + 1) * sizeof(char));
  data.eventType[eventTypeSize] = '\0';
  strcpy(data.eventType, userEventsData.updatedEventName.c_str());

  auto updatedEvent = userEventsData.updatedEventDataVariable->print();
  size_t eventSize = updatedEvent.length();
  data.eventJsonString = (char*)malloc((eventSize + 1) * sizeof(char));
  data.eventJsonString[eventSize] = '\0';

  strcpy(data.eventJsonString, updatedEvent.c_str());

  return {data};
}

NimbleNetStatus* CoreSDK::add_user_event(const string& eventMapJsonString, const string& eventType,
                                         CUserEventsData* cUserEventsData) {
  if (!_commandCenterReady.load()) {
    return util::nimblestatus(1, "%s", "NimbleNet is not initialized");
  }

  auto commandCenter = command_center();
  if (commandCenter->is_ready()) {
    auto userEventsData = commandCenter->add_user_event(eventMapJsonString, eventType);
    return process_add_user_event_response(userEventsData, cUserEventsData);
  }

  return util::nimblestatus(400, "%s", "Cannot add/update event since NimbleEdge is not ready");
}

NimbleNetStatus* CoreSDK::add_user_event(const OpReturnType event, const string& eventType,
                                         CUserEventsData* cUserEventsData) {
  if (!_commandCenterReady.load()) {
    return util::nimblestatus(1, "%s", "NimbleNet is not initialized");
  }

  auto commandCenter = command_center();
  if (commandCenter->is_ready()) {
    auto userEventsData = commandCenter->add_user_event(event, eventType);
    return process_add_user_event_response(userEventsData, cUserEventsData);
  }

  return util::nimblestatus(400, "%s", "Cannot add/update event since NimbleEdge is not ready");
}

NimbleNetStatus* CoreSDK::process_add_user_event_response(const UserEventsData& userEventsData,
                                                          CUserEventsData* cUserEventsData) {
  auto res = get_C_UserEventsData(userEventsData);
  return res.populate_data_or_return_error(cUserEventsData);
}

bool CoreSDK::save_labels_for_inference_input(const std::string& modelId,
                                              const InferenceRequest& inputs,
                                              const InferenceRequest& labels) {
  if (!_commandCenterReady.load()) {
    return false;
  }
  return false;
}

bool CoreSDK::load_task(const std::string& taskName, const std::string& taskVersion,
                        std::string&& taskCode) {
  if (!_commandCenterReady.load()) {
    return false;
  }
  return command_center()->load_task(taskName, taskVersion, std::move(taskCode));
}

NimbleNetStatus* CoreSDK::run_task(const char* taskName, const char* functionName,
                                   const CTensors input, CTensors* outputs) {
  if (!_commandCenterReady.load()) {
    return util::nimblestatus(1, "%s", "NimbleNet is not initialized");
  }
  auto commandCenter = command_center();
  if (commandCenter->is_ready()) {
    return command_center()->run_task(taskName, functionName, input, outputs);
  }
  return util::nimblestatus(400, "Cannot run method %s since NimbleEdge is not ready",
                            functionName);
}

NimbleNetStatus* CoreSDK::run_task(const char* taskName, const char* functionName,
                                   std::shared_ptr<MapDataVariable> inputs,
                                   std::shared_ptr<MapDataVariable> outputs) {
  if (!_commandCenterReady.load()) {
    return util::nimblestatus(1, "%s", "NimbleNet is not initialized");
  }
  return command_center()->run_task(taskName, functionName, inputs, outputs);
}

bool CoreSDK::deallocate_output_memory(CTensors* output) {
  if (!_commandCenterReady.load()) {
    return false;
  }
  return command_center()->deallocate_output_memory(output);
}

bool CoreSDK::reload_model_with_epConfig(const char* modelName, const char* epConfig) {
  if (!_commandCenterReady.load()) {
    return false;
  }
  return command_center()->reload_model_with_epConfig(modelName, epConfig);
}

NimbleNetStatus* CoreSDK::is_ready() {
  if (!_commandCenterReady.load()) {
    return util::nimblestatus(1, "%s", "NimbleNet is not initialized");
  }
  return command_center()->is_ready_for_exposing();
}

bool CoreSDK::send_events(const char* minInitConfigJsonChar) {
  if (_initializeSuccess.load()) {
    return false;
  }
  auto minInitConfig = jsonparser::get<MinimalInitializationConfig>(minInitConfigJsonChar);
  atomic_repeatable_minimal_initialize(minInitConfig);
  auto status = externalLogSender()->send_all_logs();
  return status;
}

void CoreSDK::copy_module(const std::shared_ptr<Asset> asset, Deployment& deployment,
                          bool addToDeployment) {
  switch (asset->type) {
    case AssetType::SCRIPT: {
      std::string delitepycontent;
#ifdef SIMULATION_MODE
      delitepycontent = parseScriptToAST(asset->location.path);
#else
      nativeinterface::get_unencrypted_file_from_device_common(asset->location.path,
                                                               delitepycontent, true);
#endif  // SIMULATION_MODE
      nativeinterface::write_data_to_file(std::move(delitepycontent),
                                          asset->get_file_name_on_device(), false);
      if (addToDeployment) deployment.script = asset;
      break;
    }
    case AssetType::MODEL: {
      std::string outputFilePath =
          nativeinterface::get_full_file_path_common(asset->get_file_name_on_device());
      nativeinterface::create_symlink(fs::absolute(asset->location.path), outputFilePath);
      if (addToDeployment) deployment.modules.push_back(asset);
      break;
    }
#ifdef GENAI
    case AssetType::RETRIEVER: {
      for (const auto& asset : asset->arguments) {
        copy_module(asset, deployment, false);
      }
      if (addToDeployment) deployment.modules.push_back(asset);
      break;
    }
    case AssetType::DOCUMENT: {
      std::string outputFilePath =
          nativeinterface::get_full_file_path_common(asset->get_file_name_on_device());
      nativeinterface::create_symlink(fs::absolute(asset->location.path), outputFilePath);
      if (addToDeployment) deployment.modules.push_back(asset);
      break;
    }
    case AssetType::LLM: {
      std::string outputFilePath =
          nativeinterface::get_full_file_path_common(asset->get_file_name_on_device());
      nativeinterface::create_symlink(fs::absolute(asset->location.path), outputFilePath);
      if (addToDeployment) deployment.modules.push_back(asset);
      break;
    }
#endif  // GENAI
    default:
      THROW("AssetType %s not supported in simulator.",
            assetmanager::get_string_from_asset_type(asset->type).c_str());
  }
}

NimbleNetStatus* CoreSDK::process_module_info(const nlohmann::json assetsJson,
                                              const std::string& homeDir) {
  nativeinterface::HOMEDIR = homeDir + "/";
  if (!nativeinterface::create_folder(nativeinterface::HOMEDIR)) {
    THROW("Could not create directory %s", nativeinterface::HOMEDIR.c_str());
  }
  Deployment deployment = {.Id = 1};
  for (const auto& it : assetsJson) {
    std::shared_ptr<Asset> asset = assetmanager::parse_module_info(it);
    copy_module(asset, deployment, true);
  }
  // Write deployment_config to disk
  util::save_deployment_on_device(deployment, coresdkconstants::DefaultCompatibilityTag);
  return nullptr;
}

NimbleNetStatus* CoreSDK::load_modules(const OpReturnType assetsJson, const std::string& homeDir) {
  return process_module_info(assetsJson->to_json(), homeDir);
}

NimbleNetStatus* CoreSDK::load_modules(const char* assetsJson, const char* homeDir) {
  nlohmann::json jsonConfig = nlohmann::json::parse(assetsJson);
  return process_module_info(jsonConfig, homeDir);
}

NimbleNetStatus* CoreSDK::load_modules(const nlohmann::json assetsJson,
                                       const std::string& homeDir) {
  return process_module_info(assetsJson, homeDir);
}

CoreSDK::~CoreSDK() {
  if (_threadRunning == true) {
    _threadRunning = false;
    _cmdThread.join();
  }
  delete _logSender;
  delete _database;
}

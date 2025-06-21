/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "command_center.hpp"

#include <exception>
#include <memory>

#include "core_sdk_structs.hpp"
#include "executor_structs.h"
#include "job_scheduler.hpp"
#include "logger.hpp"
#include "nimble_net_util.hpp"
#include "resource_manager.hpp"
#include "script_load_job.hpp"
#include "server_api.hpp"
#include "server_api_structs.hpp"
#include "time_manager.hpp"
#include "util.hpp"
#ifdef SCRIPTING
#include "map_data_variable.hpp"
#include "task.hpp"

#endif
#include "resource_downloader.hpp"
#include "resource_loader.hpp"
#include "script_ready_job.hpp"
#include "user_events_manager.hpp"
using namespace std;

CommandCenter::CommandCenter(std::shared_ptr<ServerAPI> serverAPI, std::shared_ptr<Config> config,
                             MetricsAgent* metricsAgent, Database* database,
                             std::shared_ptr<JobScheduler> jobScheduler,
                             std::shared_ptr<Logger> externalLogger, bool currentState,
                             const Deployment& deployment)
    : _jobScheduler(std::move(jobScheduler)) {
  if (!currentState) {
    LOG_TO_INFO("%s", "New CommandCenter created for state change");
  }
  _currentState = currentState;
  _metricsAgent = metricsAgent;
  _externalLogger = externalLogger;
  _config = config;
  _serverAPI = serverAPI;
  _resourceManager = std::make_unique<ResourceManager>(this);
  _resourceDownloader = std::make_unique<ResourceDownloader>(this);
  _resourceLoader = std::make_unique<ResourceLoader>(this, currentState);
  _deployment = deployment;
  _scriptReadyJob = std::make_shared<ScriptReadyJob>(this);
  _userEventsManager = std::make_unique<UserEventsManager>(this, database, _config);
  achieve_state_in_offline_mode();
  TimeManagerConfig timeManagerConfig = {.isOnline = true};

  // TODO: Should this code be placed somewhere better
  if (!_config->online) {
    timeManagerConfig.isOnline = false;
    // In offline mode, set _peggedDeviceTime to any non null value since server time does not mater
    _peggedDeviceTime = PeggedDeviceTime(
        DeviceTime::zero.add_duration(Duration::from_microseconds(1)), Duration::zero);
  }
  Time::setConfig(timeManagerConfig);
  DeviceTime::setConfig(timeManagerConfig);
}

void CommandCenter::updated_pegged_device_time(const PeggedDeviceTime& peggedDeviceTime) {
  _peggedDeviceTime = peggedDeviceTime;
}

void CommandCenter::internet_switched_on() {
  for (auto modelId : _config->get_modelIds()) {
    if (_resourceManager) {
      _resourceManager->reset_model_retries(modelId);
    }
  }
}

void CommandCenter::achieve_state_in_offline_mode() {
#ifdef SCRIPTING
  if (_deployment.script != nullptr && _deployment.script->valid) {
    auto scriptJob = std::make_shared<ScriptLoadJob>(_deployment.script, this);
    scriptJob->init();
  }
#endif
}

UserEventsData CommandCenter::add_user_event(const string& eventMapJsonString,
                                             const string& eventType) {
  try {
    auto userEventsData = get_userEventsManager().add_event(eventMapJsonString, eventType);
    return log_event_and_return_if_needed(userEventsData, eventType);
  } catch (std::exception& e) {
    return UserEventsData(util::nimblestatus(400, "%s", e.what()));
  }
}

UserEventsData CommandCenter::add_user_event(const OpReturnType event, const string& eventType) {
  try {
    auto userEventsData = get_userEventsManager().add_event(event, eventType);
    return log_event_and_return_if_needed(userEventsData, eventType);
  } catch (std::exception& e) {
    return UserEventsData(util::nimblestatus(400, "%s", e.what()));
  }
}

UserEventsData CommandCenter::log_event_and_return_if_needed(const UserEventsData userEventsData,
                                                             const std::string& eventType) {
  try {
    if (userEventsData.status != nullptr || userEventsData.updatedEventDataVariable == nullptr) {
      return userEventsData;
    };

    auto eventString = userEventsData.updatedEventDataVariable->to_json_str();
    auto isNeeded =
        _externalLogger->event_log(userEventsData.updatedEventName.c_str(), eventString.c_str());

    if (_externalLogger->is_new_event_type(userEventsData.updatedEventName)) {
      std::shared_ptr<Job<void>> updateNimbleCloudForEventTypeJob =
          std::make_shared<RegisterNewEventJob>(userEventsData.updatedEventName, _serverAPI,
                                                _jobScheduler);
      // This job should not throw any exceptions, so discarding the future returned here
      static_cast<void>(_jobScheduler->add_job(updateNimbleCloudForEventTypeJob));
    }
    if (!isNeeded) {
      return UserEventsData(nullptr);
    }
    return userEventsData;
  } catch (std::exception& e) {
    return UserEventsData(util::nimblestatus(400, "%s", e.what()));
  }
}

bool CommandCenter::load_task(const std::string& taskName, const std::string& taskVersion,
                              std::string&& taskCode) {
#ifdef SCRIPTING
  _task = std::make_shared<Task>(taskVersion, std::move(taskCode), this);
  prepare_task();
  return true;
#endif
  LOG_TO_ERROR("%s", "Not built  for running tasks");
  return false;
}

void CommandCenter::set_task(std::shared_ptr<Task> task) {
  _task = task;
  prepare_task();
}

void CommandCenter::prepare_task() {
  _task->parse_main_module();
  _taskLoaded = true;
  // Script ready job doesn't throw, so not saving its future
  static_cast<void>(_jobScheduler->add_job(std::static_pointer_cast<Job<void>>(_scriptReadyJob)));
}

NimbleNetStatus* CommandCenter::run_task(const char* taskName, const char* functionName,
                                         const CTensors input, CTensors* outputs) {
#ifdef SCRIPTING
  auto inputTensor = std::make_shared<MapDataVariable>(input);
  auto outputDataVariable = std::make_shared<MapDataVariable>();
  try {
    // Store MapDataVariable, so as to deallocate later
    // Note that this is always stored so that the frontend can always access even in case of error.
    // This can be used by the script to return more information to the frontend
    _outputs[_outputIndex] = outputDataVariable;
    // Set outputIndex in CTensors, which will be used to call deallocate_output_memory2 function
    {
      std::lock_guard<std::mutex> locker(_tensorStoreMutex);
      outputs->outputIndex = _outputIndex;
    }

    outputs->numTensors = 0;
    outputs->tensors = nullptr;
    _outputIndex++;

    _task->operate(functionName, inputTensor, outputDataVariable);

    NimbleNetStatus* retStatus = nullptr;
    {
      const auto& outputMap = outputDataVariable->get_map();
      const auto it = outputMap.find(Task::EXIT_STATUS_KEY);
      if (it != outputMap.end() && !it->second->get_bool()) {
        retStatus = util::nimblestatus(999, "%s", "Script returned false");
      }
    }

    // Convert MapDataVariable to CTensor
    outputDataVariable->convert_to_cTensors(outputs);

    return retStatus;
  } catch (std::system_error& e) {
    return util::nimblestatus(e);
  } catch (std::exception& e) {
    return util::nimblestatus(1000, "%s", e.what());
  }
#endif
  LOG_TO_ERROR("%s", "Not built  for running tasks");
  return util::nimblestatus(STATUS::RESOURCE_NOT_FOUND_ERR, "%s", "Not built for Tasks.");
}

NimbleNetStatus* CommandCenter::run_task(const char* taskName, const char* functionName,
                                         std::shared_ptr<MapDataVariable> inputTensor,
                                         std::shared_ptr<MapDataVariable> outputDataVariable) {
#ifdef SCRIPTING
  try {
    _task->operate(functionName, inputTensor, outputDataVariable);

    NimbleNetStatus* retStatus = nullptr;
    {
      const auto& outputMap = outputDataVariable->get_map();
      const auto it = outputMap.find(Task::EXIT_STATUS_KEY);
      if (it != outputMap.end() && !it->second->get_bool()) {
        retStatus = util::nimblestatus(999, "%s", "Script returned false");
      }
    }

    return retStatus;
  } catch (std::system_error& e) {
    return util::nimblestatus(e);
  } catch (std::exception& e) {
    return util::nimblestatus(1000, "%s", e.what());
  }
#endif
  LOG_TO_ERROR("%s", "Not built  for running tasks");
  return util::nimblestatus(STATUS::RESOURCE_NOT_FOUND_ERR, "%s", "Not built for Tasks.");
}

bool CommandCenter::deallocate_output_memory(CTensors* output) {
  std::lock_guard<std::mutex> locker(_tensorStoreMutex);
  if (output->outputIndex >= _outputIndex) {
    LOG_TO_ERROR("Could not find output with index: %d to deallocate its output memory.",
                 output->outputIndex);
    return false;
  }
  _outputs.erase(output->outputIndex);
  delete[] output->tensors;
  return true;
}

void CommandCenter::log_metrics(const char* metricType, const nlohmann::json& metric) {
  std::string metricDump = metric.dump();
  _metricsAgent->log_metrics(metricType, metricDump.c_str());
}

void CommandCenter::write_inference_metric(const std::string& modelId,
                                           long long int timeTakenInMicros) {
  _metricsAgent->write_inference_metric(modelId.c_str(), "1.0.0", get_deployment_id(),
                                        timeTakenInMicros);
  return;
}

void CommandCenter::add_modelId_in_config(const std::string& modelId) {
  _config->add_model(modelId);
}

NimbleNetStatus* CommandCenter::is_ready_for_exposing() {
  if (_isReady) {
    return nullptr;
  } else {
    return util::nimblestatus(STATUS::RESOURCE_NOT_FOUND_ERR, "%s", "Not ready for exposing.");
  }
}

void CommandCenter::set_is_ready(bool value) noexcept { _isReady = value; }

void CommandCenter::update_dependency_of_script_ready_job(std::shared_ptr<BaseJob> job) {
  _scriptReadyJob->add_child_job(job);
}

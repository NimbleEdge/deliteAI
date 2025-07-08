/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "nimblenet.h"

#include <memory>

#include "config_manager.hpp"
#include "core_sdk.hpp"
#include "executor_structs.h"
#include "native_interface.hpp"
#include "nimblenet.hpp"
#include "util.hpp"

#ifndef MINIMAL_BUILD
#include "concurrent_executor_variable.hpp"
#endif  // MINIMAL_BUILD

using namespace std;

std::unique_ptr<CoreSDK> coreSDK = std::make_unique<CoreSDK>();

// nimblenet C start
/**
 * Initialise logger -> load user configs -> initialize coreSDK
 */
NimbleNetStatus* initialize_nimblenet_unwrapped(const char* configJson, const char* homeDirectory) {
  auto config = std::shared_ptr<Config>(new Config(std::string(configJson)));
  logger->set_debug_flag(config->debug);
  nativeinterface::HOMEDIR = std::string(homeDirectory) + "/";
  if (!nativeinterface::create_folder(nativeinterface::HOMEDIR)) {
    return util::nimblestatus(1, "%s", "Could not create homeDir");
  }

  bool initLogger;
  initLogger = logger->init_logger(nativeinterface::HOMEDIR + loggerconstants::LogDir);

  // Do not initialize nimbleSDK if the logger is unable to initialize
  if (!initLogger) return util::nimblestatus(TERMINAL_ERROR, "%s", "unable to init logger");

  return coreSDK->initialize(config);
}

void send_crash_log(const char* errorMessage) {
  nativeinterface::save_file_on_device_common(errorMessage, "segfault.log");
}

NimbleNetStatus* initialize_nimblenet(const char* configJson, const char* homeDirectory) {
  TRY_CATCH_RETURN_NIMBLESTATUS(initialize_nimblenet_unwrapped(configJson, homeDirectory));
}

void write_metric(const char* metricType, const char* metricJson) {
  TRY_CATCH_RETURN_VOID(coreSDK->write_metric(metricType, metricJson));
}

NimbleNetStatus* add_event(const char* eventMapJsonString, const char* eventType,
                           CUserEventsData* cUserEventsData) {
  return nimblenet::add_event(std::string{eventMapJsonString}, std::string{eventType},
                              cUserEventsData);
}

NimbleNetStatus* add_event(const OpReturnType event, const char* eventType,
                           CUserEventsData* cUserEventsData) {
  return nimblenet::add_event(event, std::string{eventType}, cUserEventsData);
}

NimbleNetStatus* is_ready() { TRY_CATCH_RETURN_NIMBLESTATUS(coreSDK->is_ready()); }

void update_session(const char* sessionIdString) { coreSDK->update_session(sessionIdString); }

void deallocate_nimblenet() {
  coreSDK.reset();
  logger.reset();

#ifndef MINIMAL_BUILD
  ConcurrentExecutorVariable::reset_threadpool();
#endif  // MINIMAL_BUILD
}

void internet_switched_on() { TRY_CATCH_RETURN_VOID(coreSDK->internet_switched_on()); }

bool save_labels_for_inference_input(const char* modelId, const InferenceRequest inputs,
                                     const InferenceRequest labels) {
  return coreSDK->save_labels_for_inference_input(modelId, inputs, labels);
}

void write_run_method_metric(const char* methodName, long long int totalTimeInUSecs) {
  TRY_CATCH_RETURN_VOID(coreSDK->write_run_method_metric(methodName, totalTimeInUSecs));
}

NimbleNetStatus* run_method(const char* functionName, const CTensors inputs, CTensors* outputs) {
  TRY_CATCH_RETURN_NIMBLESTATUS(coreSDK->run_task(GLOBALTASKNAME, functionName, inputs, outputs));
}

bool deallocate_output_memory2(CTensors* output) {
  TRY_CATCH_RETURN_DEFAULT(coreSDK->deallocate_output_memory(output), false);
}

NimbleNetStatus* load_modules(const char* assetsJson, const char* homeDir) {
  TRY_CATCH_RETURN_NIMBLESTATUS(coreSDK->load_modules(assetsJson, homeDir));
}

#ifdef SIMULATION_MODE

const char** get_build_flags() {
  const char** buildFlags = new const char*[10];
  int idx = 0;

#ifdef GENAI
  buildFlags[idx++] = "GENAI";
#endif  // GENAI

#ifdef ORT_EXTENSIONS
  buildFlags[idx++] = "ORT_EXTENSIONS";
#endif  // ORT_EXTENSIONS

#ifdef MINIMAL_BUILD
  buildFlags[idx++] = "MINIMAL_BUILD";
#endif  // MINIMAL_BUILD

  // NOTE: This should always come in the end
  buildFlags[idx] = nullptr;
  return buildFlags;
}

#endif  // SIMULATION_MODE
// nimblenet C end
#ifdef SIMULATION_MODE

bool add_events_from_file(const char* userInputFilePath, const char* tableName) {
  return coreSDK->add_events_from_file(userInputFilePath, tableName);
}

bool add_events_from_buffer(const char* userInputBuffer, const char* tableName) {
  return coreSDK->add_events_from_buffer(userInputBuffer, tableName);
}

bool run_task_upto_timestamp(const char* functionName, const CTensors input, CTensors* output,
                             int64_t timestamp) {
  return coreSDK->run_task_upto_timestamp(GLOBALTASKNAME, functionName, input, output, timestamp);
}

#endif

// nimblenetInternal C start

void reset() {
  coreSDK = std::make_unique<CoreSDK>();
  logger = std::make_shared<Logger>();
  Time::reset();

#ifndef MINIMAL_BUILD
  ConcurrentExecutorVariable::reset_threadpool();
#endif  // MINIMAL_BUILD
}

// Load model and inference configs from a given file and then save in _session
bool load_model_from_file(const char* modelFilePath, const char* inferenceConfigFilePath,
                          const char* modelId, const char* epConfigJsonChar) {
  return coreSDK->load_model_from_file(modelFilePath, inferenceConfigFilePath, modelId,
                                       epConfigJsonChar);
}

void delete_database() {
  auto fileName = (nativeinterface::HOMEDIR + DEFAULT_SQLITE_DB_NAME);
  remove(fileName.c_str());
}

bool reload_model_with_epConfig(const char* modelName, const char* epConfig) {
  return coreSDK->reload_model_with_epConfig(modelName, epConfig);
}

void* create_json_object_from_string(const char* json_string) {
  try {
    nlohmann::json* j = new nlohmann::json(nlohmann::json::parse(json_string));
    return reinterpret_cast<void*>(j);
  } catch (...) {
    return nullptr;
  }
}

bool load_task(const char* taskCode) {
  return coreSDK->load_task(GLOBALTASKNAME, "1.0.0", taskCode);
}

bool attach_cleanup_to_thread() {
  CoreSDK::attach_cleanup_to_thread();
  return true;
}

bool send_events(const char* params, const char* homeDirectory) {
  nativeinterface::HOMEDIR = std::string(homeDirectory) + "/";
  nativeinterface::create_folder(nativeinterface::HOMEDIR);
  bool initLogger;
  initLogger = logger->init_logger(nativeinterface::HOMEDIR + loggerconstants::LogDir);

  // Do not initialize nimbleSDK if the logger is unable to initialize
  if (!initLogger) return false;

  return coreSDK->send_events(params);
}

// nimblenetInternal C end

/// adding implementations for cxx header

namespace nimblenet {
NimbleNetStatus* initialize_nimblenet(const std::string& configJson,
                                      const std::string& homeDirectory) {
  return ::initialize_nimblenet(configJson.c_str(), homeDirectory.c_str());
}

NimbleNetStatus* add_event(const std::string& eventMapJsonString, const std::string& eventType,
                           CUserEventsData* cUserEventsData) {
  TRY_CATCH_RETURN_NIMBLESTATUS(
      coreSDK->add_user_event(eventMapJsonString, eventType, cUserEventsData));
}

NimbleNetStatus* add_event(const OpReturnType event, const std::string& eventType,
                           CUserEventsData* cUserEventsData) {
  TRY_CATCH_RETURN_NIMBLESTATUS(coreSDK->add_user_event(event, eventType, cUserEventsData));
}

// v2
NimbleNetStatus* run_method(const std::string& functionName,
                            std::shared_ptr<MapDataVariable> inputs,
                            std::shared_ptr<MapDataVariable> outputs) {
  TRY_CATCH_RETURN_NIMBLESTATUS(
      coreSDK->run_task(GLOBALTASKNAME, functionName.c_str(), inputs, outputs));
}

NimbleNetStatus* is_ready() { return ::is_ready(); }

void update_session(const std::string& sessionIdString) {
  return ::update_session(sessionIdString.c_str());
}

void deallocate_nimblenet() { ::deallocate_nimblenet(); }

NimbleNetStatus* load_modules(const OpReturnType assetsJson, const std::string& homeDir) {
  TRY_CATCH_RETURN_NIMBLESTATUS(coreSDK->load_modules(assetsJson, homeDir));
}

NimbleNetStatus* load_modules(const nlohmann::json assetsJson, const std::string& homeDir) {
  TRY_CATCH_RETURN_NIMBLESTATUS(coreSDK->load_modules(assetsJson, homeDir));
}

// internal
void send_crash_log(const std::string& errorMessage) {
  return ::send_crash_log(errorMessage.c_str());
}

void internet_switched_on() { return ::internet_switched_on(); }

void write_metric(const std::string& metricType, const std::string& metricJson) {
  return ::write_metric(metricType.c_str(), metricJson.c_str());
}

void write_run_method_metric(const std::string& methodName, long long int totalTimeInUSecs) {
  return ::write_run_method_metric(methodName.c_str(), totalTimeInUSecs);
}

bool send_events(const std::string& params, const std::string& homeDirectory) {
  return ::send_events(params.c_str(), homeDirectory.c_str());
}

}  // namespace nimblenet

namespace nimblenetInternal {
////////// For lambda testing and DemoApp

bool reload_model_with_epConfig(const std::string& modelName, const std::string& epConfig) {
  return ::reload_model_with_epConfig(modelName.c_str(), epConfig.c_str());
}

bool load_model_from_file(const std::string& modelFilePath,
                          const std::string& inferenceConfigFilePath, const std::string& modelId,
                          const std::string& epConfigJsonChar) {
  return ::load_model_from_file(modelFilePath.c_str(), inferenceConfigFilePath.c_str(),
                                modelId.c_str(), epConfigJsonChar.c_str());
}

void reset() { ::reset(); }

void delete_database() { ::delete_database(); }

bool attach_cleanup_to_thread() { return ::attach_cleanup_to_thread(); }

//////////
}  // namespace nimblenetInternal

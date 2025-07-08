/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include "executor_structs.h"
#include "list_data_variable.hpp"
#include "map_data_variable.hpp"

class DataVariable;
class MapDataVariable;

#pragma GCC visibility push(default)

// ==============================
// Public C++ style NimbleNet API Functions
// ==============================

namespace nimblenet {

/**
 * @brief Initializes the NimbleNet runtime with the given configuration.
 *
 * @param configJson      JSON string containing config parameters.
 * @param homeDirectory   Path to the directory on device where SDK with store all the assets, logs
 * and user events.
 * @return Pointer to NimbleNetStatus indicating success/failure.
 */
NimbleNetStatus* initialize_nimblenet(const std::string& configJson,
                                      const std::string& homeDirectory);

/**
 * @brief Adds a single event to the event store.
 *
 * @param eventMapJsonString JSON string representing event key-value pairs.
 * @param eventType          String identifier for event type.
 * @param cUserEventsData    Pointer to struct with updated eventType and event.
 * @return Pointer to NimbleNetStatus indicating result.
 */
NimbleNetStatus* add_event(const std::string& eventMapJsonString, const std::string& eventType,
                           CUserEventsData* cUserEventsData);

/**
 * @brief Adds a single event to the event store.
 *
 * @param event              Event denoted as a MapDataVariable.
 * @param eventType          String identifier for event type.
 * @param cUserEventsData    Pointer to struct with updated eventType and event.
 * @return Pointer to NimbleNetStatus indicating result.
 */
NimbleNetStatus* add_event(const OpReturnType event, const std::string& eventType,
                           CUserEventsData* cUserEventsData);

/**
 * @brief Returns whether the SDK is ready to accept delitepy function calls, user events etc.
 *
 * @return Pointer to NimbleNetStatus representing readiness.
 */
NimbleNetStatus* is_ready();

/**
 * @brief Runs a method from the delitepy script with the given inputs and collects outputs.
 *
 * @param functionName Name of the method to invoke.
 * @param inputs       shared pointer of input map to the function being invoked.
 * @param outputs      shared pointer of output returned by the function being invoked.
 * @return NimbleNetStatus* indicating success/failure.
 */
NimbleNetStatus* run_method(const std::string& functionName,
                            std::shared_ptr<MapDataVariable> inputs,
                            std::shared_ptr<MapDataVariable> outputs);

/**
 * @brief Updates the session context with the given session ID string.
 */
void update_session(const std::string& sessionIdString);

/**
 * @brief Frees allocated nimblenet resources.
 */
void deallocate_nimblenet();

/**
 * @brief Loads assets provided from disk into homeDir.
 */
NimbleNetStatus* load_modules(const OpReturnType assetsJson, const std::string& homeDirectory = "");

/**
 * @brief Loads assets provided from disk into homeDir.
 */
NimbleNetStatus* load_modules(const nlohmann::json assetsJson,
                              const std::string& homeDirectory = "");
/**
 * @brief Sends a crash log to the monitoring backend.
 *
 * @param errorMessage Description or traceback of the crash.
 */
void send_crash_log(const std::string& errorMessage);

/**
 * @brief Records a generic metric to internal logs.
 */
void write_metric(const std::string& metricType, const std::string& metricJson);

/**
 * @brief Records timing data for a run_method invocation.
 */
void write_run_method_metric(const std::string& methodName, long long int totalTimeInUSecs);

/**
 * @brief Sends all the events stored on disk to cloud.
 *
 * @param params        Optional parameters (e.g. metadata).
 * @param homeDirectory Filesystem path context.
 * @return true if successful, false otherwise.
 */
bool send_events(const std::string& params, const std::string& homeDirectory);

/**
 * @brief Indicates to NimbleNet that network access is restored.
 */
void internet_switched_on();
}  // namespace nimblenet

namespace nimblenetInternal {
// ==============================
// Simulation/Test Mode Functions
// ==============================

/**
 * @brief Resets internal NimbleNet state.
 */
void reset();

/**
 * @brief Deletes the local NimbleNet database (used for events, etc.).
 *
 * @note use this method only if sqlite based database is used i.e. code compiled with NOAQL flag as
 * false.
 */
void delete_database();

/**
 * @brief Reloads a model with a new execution provider configuration.
 */
bool reload_model_with_epConfig(const std::string& modelName, const std::string& epConfig);

/**
 * @brief Loads a model and its inference configuration from disk.
 */
bool load_model_from_file(const std::string& modelFilePath,
                          const std::string& inferenceConfigFilePath, const std::string& modelId,
                          const std::string& epConfigJsonChar);

/**
 * @brief Attaches cleanup logic to the current thread for handling crashes.
 */
bool attach_cleanup_to_thread();
}  // namespace nimblenetInternal

#pragma GCC visibility pop

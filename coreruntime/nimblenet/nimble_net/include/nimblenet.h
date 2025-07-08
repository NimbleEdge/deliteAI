/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "executor_structs.h"

#ifdef SIMULATION_MODE
#include <inttypes.h>
#endif

#pragma GCC visibility push(default)

#ifdef __cplusplus
extern "C" {
#endif

// ==============================
// Public C-style NimbleNet API Functions
// ==============================

/**
 * @brief Initializes the NimbleNet runtime with the given configuration.
 *
 * @param configJson      JSON string containing config parameters.
 * @param homeDirectory   Path to the directory on device where SDK with store all the assets, logs
 * and user events.
 * @return Pointer to NimbleNetStatus indicating success/failure.
 */
NimbleNetStatus* initialize_nimblenet(const char* configJson, const char* homeDirectory);

/**
 * @brief Adds a single event to the event store.
 *
 * @param eventMapJsonString JSON string representing event key-value pairs.
 * @param eventType          String identifier for event type.
 * @param cUserEventsData    Pointer to struct with updated eventType and event.
 * @return Pointer to NimbleNetStatus indicating result.
 */
NimbleNetStatus* add_event(const char* eventMapJsonString, const char* eventType,
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
 * @param inputs       Struct containing input tensors.
 * @param outputs      Pointer to an output tensor struct to be filled.
 * @return NimbleNetStatus* indicating success/failure.
 */
NimbleNetStatus* run_method(const char* functionName, const CTensors inputs, CTensors* outputs);

/**
 * @brief Updates the session context with the given session ID string.
 */
void update_session(const char* sessionIdString);

/**
 * @brief Frees allocated nimblenet resources.
 */
void deallocate_nimblenet();

/**
 * @brief Copies assets provided from disk into homeDirectory.
 */
NimbleNetStatus* load_modules(const char* assetsJson, const char* homeDirectory);

/**
 * @brief Sends a crash log to the monitoring backend.
 *
 * @param errorMessage Description or traceback of the crash.
 */
void send_crash_log(const char* errorMessage);

/**
 * @brief Records a generic metric to internal logs.
 */
void write_metric(const char* metricType, const char* metricJson);

/**
 * @brief Records timing data for a run_method invocation.
 */
void write_run_method_metric(const char* methodName, long long int totalTimeInUSecs);

/**
 * @brief Sends all the events stored on disk to cloud.
 *
 * @param params        Optional parameters (e.g. metadata).
 * @param homeDirectory Filesystem path context.
 * @return true if successful, false otherwise.
 */
bool send_events(const char* params, const char* homeDirectory);

/**
 * @brief Indicates to NimbleNet that network access is restored.
 */
void internet_switched_on();

/**
 * @brief Associates labels with a given model input for training or validation.
 * @deprecated
 *
 * @param modelId   Identifier of the model.
 * @param input     Input tensors.
 * @param label     Corresponding label tensors.
 * @return true on success, false on failure.
 */
bool save_labels_for_inference_input(const char* modelId, const InferenceRequest input,
                                     const InferenceRequest label);

/**
 * @brief Frees memory allocated to output tensors.
 */
bool deallocate_output_memory2(CTensors* output);

#ifdef SIMULATION_MODE
// ==============================
// Simulation/Test Mode Functions
// ==============================

/**
 * @brief Loads user events from a file and adds them to the event table.
 */
bool add_events_from_file(const char* userEventsFilePath, const char* tableName);

/**
 * @brief Adds user events from an in-memory buffer.
 */
bool add_events_from_buffer(const char* userEventsBuffer, const char* tableName);

/**
 * @brief Runs a method from delitepy script up to the specified simulation timestamp.
 */
bool run_task_upto_timestamp(const char* functionName, const CTensors input, CTensors* output,
                             int64_t timestamp);

/**
 * @brief Returns build flags used while compiling. Used when running tests in nimblenet_py.
 */
const char** get_build_flags();
#endif  // SIMULATION_MODE

// ==============================
// Utilities (Testing / App-Specific)
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
bool reload_model_with_epConfig(const char* modelName, const char* epConfig);

/**
 * @brief Loads a model and its inference configuration from disk.
 */
bool load_model_from_file(const char* modelFilePath, const char* inferenceConfigFilePath,
                          const char* modelId, const char* epConfigJsonChar);

/**
 * @brief Parses a JSON string and returns a pointer to a created JSON object.
 */
void* create_json_object_from_string(const char* json_string);

/**
 * @brief Loads a serialized delitepy script into memory for execution.
 */
bool load_task(const char* taskCode);

/**
 * @brief Attaches cleanup logic to the current thread for handling crashes.
 */
bool attach_cleanup_to_thread();

#ifdef __cplusplus
}
#endif

#pragma GCC visibility pop

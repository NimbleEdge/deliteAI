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

NimbleNetStatus* initialize_nimblenet(const char* configJson, const char* homeDirectory);
NimbleNetStatus* add_event(const char* eventMapJsonString, const char* eventType,
                           CUserEventsData* cUserEventsData);
NimbleNetStatus* is_ready();

// v2
NimbleNetStatus* run_method(const char* functionName, const CTensors inputs, CTensors* outputs);
void update_session(const char* sessionIdString);
void deallocate_nimblenet();

// internal
void send_crash_log(const char* errorMessage);
void write_metric(const char* metricType, const char* metricJson);
void write_run_method_metric(const char* methodName, long long int totalTimeInUSecs);
bool send_events(const char* params, const char* homeDirectory);

void internet_switched_on();
bool save_labels_for_inference_input(const char* modelId, const InferenceRequest input,
                                     const InferenceRequest label);

bool deallocate_output_memory2(CTensors* output);

#ifdef SIMULATION_MODE
const char** get_build_flags();
#endif

#ifdef SIMULATION_MODE

int initialize_nimblenet_from_file(const char* configFilePath);

bool add_events_from_file(const char* userEventsFilePath, const char* tableName);

bool add_events_from_buffer(const char* userEventsBuffer, const char* tableName);

bool run_task_upto_timestamp(const char* functionName, const CTensors input, CTensors* output,
                             int64_t timestamp);

#endif

////////// For lambda testing and DemoApp
void reset();
void delete_database();
bool reload_model_with_epConfig(const char* modelName, const char* epConfig);
bool load_model_from_file(const char* modelFilePath, const char* inferenceConfigFilePath,
                          const char* modelId, const char* epConfigJsonChar);
//////////

void* create_json_object_from_string(const char* json_string);

bool load_task(const char* taskCode);
bool attach_cleanup_to_thread();

#ifdef __cplusplus
}
#endif

#pragma GCC visibility pop

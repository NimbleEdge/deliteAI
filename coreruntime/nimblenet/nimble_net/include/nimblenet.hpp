/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <memory>

#include "executor_structs.h"
#include "map_data_variable.hpp"

class DataVariable;
class MapDataVariable;

#pragma GCC visibility push(default)

namespace nimblenet {
NimbleNetStatus* initialize_nimblenet(const std::string& configJson,
                                      const std::string& homeDirectory);
NimbleNetStatus* add_event(const std::string& eventMapJsonString, const std::string& eventType,
                           CUserEventsData* cUserEventsData);
NimbleNetStatus* add_event(const OpReturnType event, const std::string& eventType,
                           CUserEventsData* cUserEventsData);
NimbleNetStatus* is_ready();

// v2
NimbleNetStatus* run_method(const std::string& functionName,
                            std::shared_ptr<MapDataVariable> inputs,
                            std::shared_ptr<MapDataVariable> outputs);
void update_session(const std::string& sessionIdString);
void deallocate_nimblenet();

// internal
void send_crash_log(const std::string& errorMessage);
void write_metric(const std::string& metricType, const std::string& metricJson);
void write_run_method_metric(const std::string& methodName, long long int totalTimeInUSecs);
bool send_events(const std::string& params, const std::string& homeDirectory);

void internet_switched_on();
}  // namespace nimblenet

namespace nimblenetInternal {
////////// For lambda testing and DemoApp
void reset();
void delete_database();
bool reload_model_with_epConfig(const std::string& modelName, const std::string& epConfig);
bool load_model_from_file(const std::string& modelFilePath,
                          const std::string& inferenceConfigFilePath, const std::string& modelId,
                          const std::string& epConfigJsonChar);
//////////
bool attach_cleanup_to_thread();
}  // namespace nimblenetInternal

#pragma GCC visibility pop

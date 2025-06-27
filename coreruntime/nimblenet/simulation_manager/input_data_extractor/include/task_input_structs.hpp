/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "nlohmann/json_fwd.hpp"

#include "executor_structs.h"

#pragma GCC visibility push(default)

struct TaskInputData {
#ifdef SIMULATION_MODE
  static void* get_list_from_json_object_for_simulator(nlohmann::json&& j);
  static void* get_map_from_json_object_for_simulator(nlohmann::json&& j);
  static nlohmann::json get_json_from_OpReturnType(void* data);

  static void* create_function_data_variable(void* context,
                                             FrontendFunctionPtr frontEndFunctionPtr);

  static void deallocate_OpReturnType(void* data);
#endif
};

CTensors get_CTensors(const char* fileName);
CTensors get_CTensors_from_json(const char* json);
void deallocate_CTensors(CTensors cTensors);

#pragma GCC visibility pop

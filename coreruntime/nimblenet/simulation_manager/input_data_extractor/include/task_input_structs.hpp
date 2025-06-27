/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "nlohmann/json_fwd.hpp"

#include "executor_structs.h"

#pragma GCC visibility push(default)

/**
 * @brief Utility struct for handling task input data and conversions for simulation and C API interop.
 *
 * Provides static methods for converting between JSON and internal tensor representations.
 */
struct TaskInputData {
#ifdef SIMULATION_MODE
  /**
   * @brief Convert a JSON object to a list representation for the simulator.
   *
   * @param j JSON object to convert.
   * @return Pointer to the created list object.
   */
  static void* get_list_from_json_object_for_simulator(nlohmann::json&& j);

  /**
   * @brief Convert a JSON object to a map representation for the simulator.
   *
   * @param j JSON object to convert.
   * @return Pointer to the created map object.
   */
  static void* get_map_from_json_object_for_simulator(nlohmann::json&& j);

  /**
   * @brief Convert an OpReturnType pointer to a JSON object.
   *
   * @param data Pointer to OpReturnType.
   * @return JSON object representing the data.
   */
  static nlohmann::json get_json_from_OpReturnType(void* data);

  /**
   * @brief Create a function data variable for use in simulation.
   *
   * @param context Pointer to the function context.
   * @param frontEndFunctionPtr Function pointer to the frontend callback.
   * @return Pointer to the created function data variable.
   */
  static void* create_function_data_variable(void* context,
                                             FrontendFunctionPtr frontEndFunctionPtr);

  /**
   * @brief Deallocate an OpReturnType pointer.
   *
   * @param data Pointer to OpReturnType to deallocate.
   */
  static void deallocate_OpReturnType(void* data);
#endif
};

/**
 * @brief Create CTensors from a file containing tensor data in JSON format.
 *
 * @param fileName Path to the file containing tensor data.
 * @return CTensors structure populated from the file.
 */
CTensors get_CTensors(const char* fileName);

/**
 * @brief Create CTensors from a JSON string.
 *
 * @param json JSON string containing tensor data.
 * @return CTensors structure populated from the JSON string.
 */
CTensors get_CTensors_from_json(const char* json);

/**
 * @brief Deallocate memory associated with a CTensors structure.
 *
 * @param cTensors CTensors structure to deallocate.
 */
void deallocate_CTensors(CTensors cTensors);

#pragma GCC visibility pop

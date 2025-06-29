/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <vector>

#include "executor_structs.h"
#include "nimble_net_util.hpp"

#pragma GCC visibility push(default)

/**
 * @brief Structure representing a user input for inference.
 *
 * Holds the input data, type, and length, and manages memory cleanup.
 */
struct UserInput {
  CUserInput inp;   /**< Underlying C-style user input struct. */
  int dataType;     /**< Data type of the input. */
  int length;       /**< Length of the input data. */

  /**
   * @brief Destructor for UserInput.
   *
   * Frees allocated memory for input name and data based on data type.
   */
  ~UserInput() {
    delete[] inp.name;
    if (inp.dataType == DATATYPE::JSON) {
      delete (nlohmann::json*)inp.data;
    } else if (inp.dataType == DATATYPE::FLOAT) {
      delete[] (float*)inp.data;
    } else if (inp.dataType == DATATYPE::INT32) {
      delete[] (int32_t*)inp.data;
    } else if (inp.dataType == DATATYPE::INT64) {
      delete[] (long long*)inp.data;
    } else if (inp.dataType == DATATYPE::DOUBLE) {
      delete[] (double*)inp.data;
    }
  }
};

/**
 * @brief Structure for managing a collection of user inputs.
 */
struct InputData {
  int totalInputs;  /**< Total number of user inputs. */
  std::vector<std::shared_ptr<UserInput>> inputs; /**< Vector of user input pointers. */

  /**
   * @brief Default constructor.
   */
  InputData();

  /**
   * @brief Construct InputData from a file or buffer.
   *
   * @param filename Path to the file or buffer containing input data.
   */
  InputData(const std::string& filename);

  /**
   * @brief Load input data from a file.
   *
   * @param filename Path to the input file.
   */
  void getInputFromFile(const std::string& filename);

  /**
   * @brief Load input data from a JSON buffer string.
   *
   * @param buffer JSON string containing input data.
   */
  void getInputFromBuffer(const std::string& buffer);
};

/**
 * @brief Structure for holding input and output results for a user inference call.
 */
struct UserReturn {
  InferenceReturn input;   /**< Input for inference result. */
  InferenceReturn output;  /**< Output of inference result. */

  /**
   * @brief Destructor for UserReturn.
   *
   * Frees memory allocated for input and output inference results.
   */
  ~UserReturn() {
    deallocate_output_memory(&input);
    deallocate_output_memory(&output);
  }
};

#pragma GCC visibility pop

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

struct UserInput {
  CUserInput inp;
  int dataType;
  int length;

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

struct InputData {
  int totalInputs;
  std::vector<std::shared_ptr<UserInput>> inputs;
  InputData();
  InputData(const std::string& filename);
  void getInputFromFile(const std::string& filename);
  void getInputFromBuffer(const std::string& buffer);
};

struct UserReturn {
  InferenceReturn input;
  InferenceReturn output;

  ~UserReturn() {
    deallocate_output_memory(&input);
    deallocate_output_memory(&output);
  }
};

#pragma GCC visibility pop

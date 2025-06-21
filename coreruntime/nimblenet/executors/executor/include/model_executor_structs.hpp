/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "nlohmann_json.hpp"
#include "user_events_struct.hpp"
using json = nlohmann::json;
#include <vector>

struct TensorInfo {
  std::string name;
  int dataType;
  std::vector<int64_t> shape;
  int size;
  json preprocessorJson;
  bool toPreprocess = false;
};

struct PreProcessorInputInfo {
  std::string name;
  std::vector<std::string> inputNames;
};

struct ModelInfo {
  bool valid = false;
  std::vector<TensorInfo> inputs;
  std::vector<TensorInfo> outputs;
  std::vector<PreProcessorInputInfo> preprocessorInputs;
};

struct SavedInputTensor {
  std::shared_ptr<ModelInput> modelInput;
  TensorInfo* tensorInfoPtr;

  SavedInputTensor(std::shared_ptr<ModelInput> modelInput_, TensorInfo* tensorInfo_) {
    tensorInfoPtr = tensorInfo_;
    modelInput = modelInput_;
  }
};

void from_json(const json& j, TensorInfo& tensorInfo);
void from_json(const json& j, PreProcessorInputInfo& preProcessorInputInfo);
void from_json(const json& j, ModelInfo& modelInfo);
void to_json(json& j, const TensorInfo& tensorInfo);

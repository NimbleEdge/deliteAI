/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "model_executor_structs.hpp"
using json = nlohmann::json;

void from_json(const json& j, TensorInfo& tensorInfo) {
  j.at("name").get_to(tensorInfo.name);
  j.at("dataType").get_to(tensorInfo.dataType);
  if (j.find("processor") == j.end()) {
    tensorInfo.toPreprocess = false;
  } else {
    tensorInfo.toPreprocess = true;
    //    j.at("processor").get_to(tensorInfo.preprocessorJson);
    tensorInfo.preprocessorJson = j.at("processor");
  }
  j.at("shape").get_to(tensorInfo.shape);
  tensorInfo.size = 1;
  for (const auto val : tensorInfo.shape) {
    tensorInfo.size *= val;
  }
}

void from_json(const json& j, ModelInfo& modelInfo) {
  j.at("inputs").get_to(modelInfo.inputs);
  j.at("outputs").get_to(modelInfo.outputs);
  if (j.find("inputsToProcessors") != j.end())
    j.at("inputsToProcessors").get_to(modelInfo.preprocessorInputs);
  modelInfo.valid = true;
}

void from_json(const json& j, PreProcessorInputInfo& preProcessorInputInfo) {
  j.at("name").get_to(preProcessorInputInfo.name);
  j.at("inputNames").get_to(preProcessorInputInfo.inputNames);
}

void to_json(json& j, const TensorInfo& tensorInfo) {
  j = json{{"name", tensorInfo.name}, {"processor", tensorInfo.preprocessorJson}};
}

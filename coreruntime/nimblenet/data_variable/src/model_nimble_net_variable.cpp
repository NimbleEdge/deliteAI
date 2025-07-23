/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "model_nimble_net_variable.hpp"

#include "asset_load_job.hpp"
#include "map_data_variable.hpp"
#include "task_onnx_model.hpp"

std::shared_ptr<FutureDataVariable> ModelNimbleNetVariable::load_async(
    const std::string& modelName, CommandCenter* commandCenter) {
  const auto deployment = commandCenter->get_deployment();
  auto modelAsset = deployment.get_module(modelName, AssetType::MODEL);
  if (modelAsset == nullptr) {
    THROW("Model  %s not present in deployment", modelName.c_str());
  }

  auto modelLoadJob = std::make_shared<AssetLoadJob>(modelAsset, commandCenter);
  return modelLoadJob->init();
}

OpReturnType ModelNimbleNetVariable::run_model(const std::vector<OpReturnType>& arguments) {
  // Check if we have a single dictionary argument (new interface)
  if (arguments.size() == 1) {
    auto mapVar = std::dynamic_pointer_cast<MapDataVariable>(arguments[0]);
    if (mapVar) {
      // Use dictionary-based inference if available
      auto onnxModel = std::dynamic_pointer_cast<TaskONNXModel>(_model);
      if (onnxModel) {
        OpReturnType output;
        try {
          auto start = std::chrono::high_resolution_clock::now();

          // Use the new dictionary interface (corrected signature)
          int infStatus = onnxModel->invoke_inference_dict(output, arguments[0]);

          auto stop = std::chrono::high_resolution_clock::now();
          long long duration =
              std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
          _commandCenter->write_inference_metric(_modelName, duration);

          if (infStatus != SUCCESS || !output) {
            // inference failed return None
            return OpReturnType(new NoneVariable());
          }

          return output;
        } catch (...) {
          THROW("%s", "Error occurred while trying to get inference using dictionary interface.");
        }
      }
    }
  }

  // Fall back to traditional vector interface
  std::vector<const char*> inputNames = _model->get_input_names();
  if (inputNames.size() != arguments.size()) {
    THROW("Model takes %d inputs, %d inputs provided. Cannot run model.", inputNames.size(),
          arguments.size());
  }

  OpReturnType output;
  try {
    // TODO: Need to figure out a way to take "sampleInferId" from inputs of main function
    auto start = std::chrono::high_resolution_clock::now();
    int infStatus = _model->get_inference("sampleInferId", arguments, output);
    auto stop = std::chrono::high_resolution_clock::now();
    long long duration =
        std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
    _commandCenter->write_inference_metric(_modelName, duration);
    if (infStatus != SUCCESS) {
      // inference failed return None
      return OpReturnType(new NoneVariable());
    }
  } catch (...) {
    // In case of exception from onnx or
    THROW("%s",
          "Error occured while trying to get inference in run_model function from codeBlocks.");
  }
  return output;
}

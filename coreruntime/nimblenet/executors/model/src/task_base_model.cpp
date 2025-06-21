/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "task_base_model.hpp"

#include <fstream>

#include "native_interface.hpp"

#ifdef ONNX_EXECUTOR
#include "task_onnx_model.hpp"
typedef class TaskONNXModel Model;
#endif  // ONNX_EXECUTOR

void TaskBaseModel::initialize_model() { load_model_from_buffer(); }

TaskBaseModel::TaskBaseModel(const std::string& modelFileName, const std::string& version,
                             const std::string& modelId,
                             const nlohmann::json& executionProviderConfig,
                             const int epConfigVersion, CommandCenter* commandCenter,
                             bool runDummyInference) {
  std::lock_guard<std::mutex> locker(_modelMutex);
  const auto modelBufferOpt =
      nativeinterface::read_potentially_compressed_file(modelFileName, false);
  const auto successfulRead = modelBufferOpt.first;
  if (!successfulRead) {
    THROW("Model file=%s not present", modelFileName.c_str());
  }
  _modelBuffer = std::move(modelBufferOpt.second);
  _epConfig = executionProviderConfig;
  _epConfigVersion = epConfigVersion;
  _commandCenter = commandCenter;
  _version = version;
  _modelId = modelId;
  _runDummyInference = runDummyInference;
}

int TaskBaseModel::get_inference(const std::string& inferId, const std::vector<OpReturnType>& req,
                                 OpReturnType& ret) {
  std::vector<Ort::Value> inputTensors;
  // Create tensors for input and store them
  for (int i = 0; i < req.size(); i++) {
    Ort::Value inputTensor = Ort::Value(nullptr);
    if (create_input_tensor_and_set_data_ptr(req[i], i, std::move(inputTensor)) != SUCCESS) {
      return TERMINAL_ERROR;
    }
    inputTensors.push_back(std::move(inputTensor));
  }
  int status = invoke_inference(ret, inputTensors);
  return status;
}

void TaskBaseModel::get_model_status(ModelStatus* status) {
  status->isModelReady = true;
  asprintf(&(status->version), "%s", _version.c_str());
}

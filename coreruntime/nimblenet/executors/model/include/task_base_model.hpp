/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <shared_mutex>

#include "command_center.hpp"
#include "data_variable.hpp"
#include "executor_structs.h"
#include "map_data_variable.hpp"
#include "model_executor_structs.hpp"
#ifdef ONNX_EXECUTOR
#include "onnx.hpp"
#endif  // ONNX_EXECUTOR

class CommandCenter;

class TaskBaseModel {
 protected:
  CommandCenter* _commandCenter;
  std::string _modelBuffer;
  nlohmann::json _epConfig;
  int _epConfigVersion;
  std::string _modelId;
  std::mutex _modelMutex;
  std::string _version;
  bool _runDummyInference;

  virtual void initialize_model();

  virtual bool check_input(const std::string& inferId, int inputIndex, int dataType,
                           int inputSizeBytes) {
    throw std::runtime_error("Check Input function not implemented.");
  };

  virtual void print_input() { throw std::runtime_error("Print Input function not implemented."); }

  virtual void print_output() {
    throw std::runtime_error("Print Output function not implemented.");
  }

  virtual void print_tensors(bool forInput, const std::vector<TensorInfo>& tensorsInfo) {
    throw std::runtime_error("Print tensors function not implemented.");
  }

  virtual void* get_data_buff_input_tensor(const int index) {
    throw std::runtime_error("Get Input tensor data buffer function not implemented.");
  }

  virtual void* get_data_buff_output_tensor(const int index) {
    throw std::runtime_error("Get output tensor data buffer function not implemented.");
  }

  virtual int create_input_tensor_and_set_data_ptr(const int index, void* dataPtr) = 0;
#ifdef ONNX_EXECUTOR
  virtual int create_input_tensor_and_set_data_ptr(const OpReturnType req, int modelInputIndex,
                                                   Ort::Value&& returnedInputTensor) = 0;
  virtual int invoke_inference(OpReturnType& ret, const std::vector<Ort::Value>& inputTensors) = 0;
#endif  // ONNX_EXECUTOR
  virtual int create_output_tensor_and_set_data_ptr(const int index, void* dataPtr) = 0;
  virtual void load_model_from_buffer() = 0;
  virtual int invoke_inference(InferenceReturn* ret) = 0;
  virtual void run_dummy_inference() = 0;

 public:
  TaskBaseModel(const std::string& plan, const std::string& version, const std::string& modelId,
                const nlohmann::json& epConfig, const int epConfigVersion,
                CommandCenter* commandCenter, bool runDummyInference = true);

  virtual int get_inference(const std::string& inferId, const std::vector<OpReturnType>& req,
                            OpReturnType& ret);

  virtual int get_inference(const std::string& inferId, const InferenceRequest& req,
                            InferenceReturn* ret,
                            std::vector<SavedInputTensor>& preprocessorInputsToFill,
                            bool can_save_input) {
    throw std::runtime_error(
        "Get inference function with InferenceRequest struct in V1 model not implemented.");
  }

  const std::string get_plan_version() const { return _version; }

  const int get_ep_config_version() const { return _epConfigVersion; }

  void get_model_status(ModelStatus* status);

  virtual std::vector<const char*> get_input_names() = 0;
  virtual std::vector<const char*> get_output_names() = 0;

  TaskBaseModel(const TaskBaseModel& other) = delete;
  TaskBaseModel(TaskBaseModel&& other) = delete;
  TaskBaseModel& operator=(TaskBaseModel&& other) = delete;
  virtual ~TaskBaseModel() = default;
};

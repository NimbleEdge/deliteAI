/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "data_variable.hpp"
#include "nimble_net_util.hpp"
#include "task_base_model.hpp"
#include "tensor_data_variable.hpp"

class TaskONNXModel : public TaskBaseModel {
 private:
  OrtAllocator* _allocator = nullptr;
  Ort::SessionOptions _sessionOptions;
  Ort::MemoryInfo _memoryInfo;
  static Ort::Env _myEnv;
  static Ort::ThreadingOptions tp;
  Ort::Session* _session = nullptr;
  std::vector<const char*> _inputNames;
  std::vector<const char*> _outputNames;

  void load_model_meta_data();
  void load_model_from_buffer() override final;
  int invoke_inference(OpReturnType& ret,
                       const std::vector<Ort::Value>& inputTensors) override final;

  int invoke_inference(InferenceReturn* ret) override final {
    throw std::runtime_error(
        "Invoke inference with InferenceReturn struct in model run from task is not implemented.");
  }

  int create_input_tensor_and_set_data_ptr(const OpReturnType req, int modelInputIndex,
                                           Ort::Value&& returnedInputTensor) override final;

  int create_input_tensor_and_set_data_ptr(const int index, void* dataPtr) override final {
    throw std::runtime_error(
        "Create input tensor using InferenceRequest in model run from task is not implemented");
  }

  virtual int create_output_tensor_and_set_data_ptr(const int index, void* dataPtr) override final {
    throw std::runtime_error("Output tensor creation in model run from task is not required.");
  }

  void run_dummy_inference() override final;
  static Ort::SessionOptions get_session_options_from_json(const nlohmann::json& epConfig);

  static OpReturnType get_tensor_variable_from_onnx_tensor(Ort::Value onnx_tensor);

 public:
  TaskONNXModel(const std::string& plan, const std::string& version, const std::string& modelId,
                const nlohmann::json& epConfig, const int epConfigVersion,
                CommandCenter* commandCenter, bool runDummyInference);

  std::vector<const char*> get_input_names() override { return _inputNames; }

  std::vector<const char*> get_output_names() override { return _outputNames; }

  virtual ~TaskONNXModel() override;
};

class OrtTensorVariable : public BaseTypedTensorVariable {
  Ort::Value _onnxTensor = Ort::Value{nullptr};

  void* get_raw_ptr() final { return _onnxTensor.GetTensorMutableRawData(); }

  int get_containerType() const final { return CONTAINERTYPE::VECTOR; }

 public:
  OrtTensorVariable(Ort::Value&& onnx_tensor, DATATYPE dataType)
      : BaseTypedTensorVariable(dataType) {
    _onnxTensor = std::move(onnx_tensor);
    std::vector<int64_t> shape = _onnxTensor.GetTensorTypeAndShapeInfo().GetShape();
    int length = 1;
    for (int i = 0; i < shape.size(); i++) {
      length *= shape[i];
    }
    BaseTensorVariable::shape = shape;
    BaseTensorVariable::numElements = length;
  }
};

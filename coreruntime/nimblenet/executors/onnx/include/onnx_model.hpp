/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "base_model.hpp"
#include "onnx_operators.hpp"

// To be removed when InferenceRequest and InferenceReturn will not be used anymore
class ONNXModel : public BaseModel {
 private:
  OrtAllocator* _allocator = nullptr;
  Ort::SessionOptions _sessionOptions;
  Ort::MemoryInfo _memoryInfo;
  static Ort::Env _myEnv;
  static Ort::ThreadingOptions tp;
  Ort::Session* _session = nullptr;
  std::vector<Ort::Value> _outputTensors;
  std::vector<Ort::Value> _inputTensors;

  std::vector<const char*> _inputNames;
  std::vector<const char*> _outputNames;

  void delete_input_memory(void** input, int size);
  static Ort::SessionOptions get_session_options_from_json(const nlohmann::json& epConfig);

  int create_input_tensor_and_set_data_ptr(const int index, void* data) override final;

  int create_input_tensor_and_set_data_ptr(const OpReturnType req, int modelInputIndex,
                                           Ort::Value&& returnedInputTensor) override final {
    throw std::runtime_error(
        "Create input tensor using DataVariable in V1 model is not implemented.");
  }

  int create_output_tensor_and_set_data_ptr(const int index, void* data) override final;
  void* get_data_buff_input_tensor(const int index) override final;
  void* get_data_buff_output_tensor(const int index) override final;
  void load_model_from_buffer() override final;
  int invoke_inference(InferenceReturn* ret) override final;

  int invoke_inference(OpReturnType& ret,
                       const std::vector<Ort::Value>& inputTensors) override final {
    throw std::runtime_error(
        "Invoke inference with Datavariable struct in V1 model is not implemented.");
  }

  void run_dummy_inference() override final;

 public:
  ONNXModel(const ModelInfo& modelInfo, const std::string& plan, const std::string& version,
            const std::string& modelId, const std::vector<nlohmann::json>& epConfig,
            const int epConfigVersion, CommandCenter* commandCenter);

  std::vector<const char*> get_input_names() override { return _inputNames; }

  std::vector<const char*> get_output_names() override { return _outputNames; }

  virtual ~ONNXModel();
};

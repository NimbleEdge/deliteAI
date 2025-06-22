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

/**
 * @brief TaskONNXModel is a specialized implementation of TaskBaseModel
 *        that supports running ONNX models using ONNX Runtime when invoked from delitepy script.
 */
class TaskONNXModel : public TaskBaseModel {
 private:
  OrtAllocator* _allocator = nullptr;    /**< Allocator used by ONNX Runtime */
  Ort::SessionOptions _sessionOptions;   /**< Options to configure ONNX session */
  Ort::MemoryInfo _memoryInfo;           /**< Memory info for tensor allocations */
  static Ort::Env _myEnv;                /**< Static environment shared by all sessions */
  static Ort::ThreadingOptions tp;       /**< Threading configuration */
  Ort::Session* _session = nullptr;      /**< ONNX session handle */
  std::vector<const char*> _inputNames;  /**< Cached input names */
  std::vector<const char*> _outputNames; /**< Cached output names */

  /**
   * @brief Loads model metadata such as input/output names.
   */
  void load_model_meta_data();

  /**
   * @brief Loads the model from the internal buffer.
   */
  void load_model_from_buffer() override final;

  /**
   * @brief Invokes inference using a vector of ONNX input tensors.
   *
   * @param ret Output structure to populate.
   * @param inputTensors Prepared input tensors.
   * @return status
   */
  int invoke_inference(OpReturnType& ret,
                       const std::vector<Ort::Value>& inputTensors) override final;

  int invoke_inference(InferenceReturn* ret) override final {
    throw std::runtime_error(
        "Invoke inference with InferenceReturn struct in model run from task is not implemented.");
  }

  /**
   * @brief Creates an ONNX input tensor and sets the data pointer.
   *
   * @param req Operator request.
   * @param modelInputIndex Index of input tensor in model.
   * @param returnedInputTensor Constructed ONNX tensor.
   * @return 0 on success.
   */
  int create_input_tensor_and_set_data_ptr(const OpReturnType req, int modelInputIndex,
                                           Ort::Value&& returnedInputTensor) override final;

  int create_input_tensor_and_set_data_ptr(const int index, void* dataPtr) override final {
    throw std::runtime_error(
        "Create input tensor using InferenceRequest in model run from task is not implemented");
  }

  virtual int create_output_tensor_and_set_data_ptr(const int index, void* dataPtr) override final {
    throw std::runtime_error("Output tensor creation in model run from task is not required.");
  }

  /**
   * @brief Runs a dummy inference to initialize graph and memory.
   */
  void run_dummy_inference() override final;

  /**
   * @brief Constructs session options from JSON-based execution provider config.
   *
   * @param epConfig Execution provider configuration.
   * @return Session options configured accordingly.
   */
  static Ort::SessionOptions get_session_options_from_json(const nlohmann::json& epConfig);

  /**
   * @brief Extracts a tensor variable from an ONNX tensor.
   *
   * @param onnx_tensor Tensor to convert.
   * @return Corresponding OpReturnType variable.
   */
  static OpReturnType get_tensor_variable_from_onnx_tensor(Ort::Value onnx_tensor);

 public:
  /**
   * @brief Constructs a TaskONNXModel instance.
   *
   * @param plan Model plan string.
   * @param version Model version.
   * @param modelId Identifier for the model.
   * @param epConfig Execution provider config in JSON.
   * @param epConfigVersion Version of config.
   * @param commandCenter Owning command center.
   * @param runDummyInference Whether to run a dummy inference during setup.
   */
  TaskONNXModel(const std::string& plan, const std::string& version, const std::string& modelId,
                const nlohmann::json& epConfig, const int epConfigVersion,
                CommandCenter* commandCenter, bool runDummyInference);

  /**
   * @brief Returns input tensor names from the ONNX model.
   */
  std::vector<const char*> get_input_names() override { return _inputNames; }

  /**
   * @brief Returns output tensor names from the ONNX model.
   */
  std::vector<const char*> get_output_names() override { return _outputNames; }

  /**
   * @brief Destructor for TaskONNXModel. Cleans up session.
   */
  virtual ~TaskONNXModel() override;
};

/**
 * @brief OrtTensorVariable wraps an ONNX tensor and exposes it
 *        as a typed tensor variable compatible with delitepy datavariable.
 */
class OrtTensorVariable : public BaseTypedTensorVariable {
  Ort::Value _onnxTensor = Ort::Value{nullptr}; /**< Wrapped ONNX tensor */

  /**
   * @brief Returns a mutable pointer to the tensor's raw memory.
   */
  void* get_raw_ptr() final { return _onnxTensor.GetTensorMutableRawData(); }

  /**
   * @brief Specifies that this tensor is backed by a vector container.
   */
  int get_containerType() const final { return CONTAINERTYPE::VECTOR; }

 public:
  /**
   * @brief Constructs an OrtTensorVariable from an ONNX tensor.
   *
   * @param onnx_tensor The tensor to wrap.
   * @param dataType The internal data type.
   */
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

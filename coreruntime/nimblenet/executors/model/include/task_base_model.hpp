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

/**
 * @brief Abstract base class for model inference via delitepy.
 *
 * This class defines a common interface and base logic for executing models,
 * including input/output handling and lifecycle management.
 */
class TaskBaseModel {
 protected:
  CommandCenter* _commandCenter; /**< Pointer to the command center. */
  std::string _modelBuffer;      /**< Serialized model data. */
  nlohmann::json _epConfig;      /**< Execution provider configuration in JSON. */
  int _epConfigVersion;          /**< Version number of the EP config. */
  std::string _modelId;          /**< Identifier for the model. */
  std::mutex _modelMutex;        /**< Mutex to guard model access. */
  std::string _version;          /**< Version string of the model plan. */
  bool _runDummyInference; /**< Flag indicating whether dummy inference should be run for this model
                              or not. */

  /**
   * @brief Initialize the model.
   */
  virtual void initialize_model();

  virtual bool check_input(const std::string& inferId, int inputIndex, int dataType,
                           int inputSizeBytes) {
    throw std::runtime_error("Check Input function not implemented.");
  }

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

  /** @brief Create input tensor and set data pointer.
   *
   *  @param index Tensor index.
   *  @param dataPtr Pointer to data buffer.
   *  @return Status code.
   */
  virtual int create_input_tensor_and_set_data_ptr(const int index, void* dataPtr) = 0;

#ifdef ONNX_EXECUTOR
  /** @brief Create ONNX input tensor and assign data pointer.
   *
   *  @param req Input structure.
   *  @param modelInputIndex Index in the model input list.
   *  @param returnedInputTensor ONNX tensor value.
   *  @return Status code.
   */
  virtual int create_input_tensor_and_set_data_ptr(const OpReturnType req, int modelInputIndex,
                                                   Ort::Value&& returnedInputTensor) = 0;

  /** @brief Perform ONNX inference.
   *
   *  @param ret Output structure.
   *  @param inputTensors List of input ONNX tensors.
   *  @return Status code.
   */
  virtual int invoke_inference(OpReturnType& ret, const std::vector<Ort::Value>& inputTensors) = 0;
#endif  // ONNX_EXECUTOR

  /** @brief Create output tensor and set data pointer.
   *
   *  @param index Tensor index.
   *  @param dataPtr Pointer to data buffer.
   *  @return Status code.
   */
  virtual int create_output_tensor_and_set_data_ptr(const int index, void* dataPtr) = 0;

  /**
   *  @brief Load model from serialized buffer.
   */
  virtual void load_model_from_buffer() = 0;

  /**
   *  @brief Legacy API to invoke inference using internal model representation.
   *
   *  @param ret Output result.
   *  @return Status code.
   */
  virtual int invoke_inference(InferenceReturn* ret) = 0;

  /**
   * @brief Run dummy inference. This is done during model load so that memory is pre-allocated when
   * the actual inference happens this reducing latency.
   */
  virtual void run_dummy_inference() = 0;

 public:
  /**
   * @brief Constructor for TaskBaseModel.
   *
   * @param plan Model plan/config name.
   * @param version Version string of the model.
   * @param modelId Unique identifier for the model.
   * @param epConfig JSON config for execution provider.
   * @param epConfigVersion Version of EP config.
   * @param commandCenter Command center reference.
   * @param runDummyInference Whether to run a dummy inference on load.
   */
  TaskBaseModel(const std::string& plan, const std::string& version, const std::string& modelId,
                const nlohmann::json& epConfig, const int epConfigVersion,
                CommandCenter* commandCenter, bool runDummyInference = true);

  /**
   * @brief Perform inference using a vector of input requests.
   *
   * @param inferId Inference operation identifier.
   * @param req Vector of input requests.
   * @param ret Output container.
   * @return Status code.
   */
  virtual int get_inference(const std::string& inferId, const std::vector<OpReturnType>& req,
                            OpReturnType& ret);

  virtual int get_inference(const std::string& inferId, const InferenceRequest& req,
                            InferenceReturn* ret,
                            std::vector<SavedInputTensor>& preprocessorInputsToFill,
                            bool can_save_input) {
    throw std::runtime_error(
        "Get inference function with InferenceRequest struct in V1 model not implemented.");
  }

  /** @brief Get the model version.
   *
   *  @return Plan version string.
   */
  const std::string get_plan_version() const { return _version; }

  /** @brief Get execution provider config version.
   *
   *  @return Config version integer.
   */
  const int get_ep_config_version() const { return _epConfigVersion; }

  /**
   * @brief Fill status structure with model information.
   *
   * @param status Pointer to ModelStatus object.
   */
  void get_model_status(ModelStatus* status);

  /**
   * @brief Retrieve list of input tensor names.
   *
   * @return Vector of C-string input names.
   */
  virtual std::vector<const char*> get_input_names() = 0;

  /**
   * @brief Retrieve list of output tensor names.
   *
   * @return Vector of C-string output names.
   */
  virtual std::vector<const char*> get_output_names() = 0;

  TaskBaseModel(const TaskBaseModel& other) = delete;       /**< Deleted copy constructor. */
  TaskBaseModel(TaskBaseModel&& other) = delete;            /**< Deleted move constructor. */
  TaskBaseModel& operator=(TaskBaseModel&& other) = delete; /**< Deleted move assignment. */

  /**
   * @brief Virtual destructor.
   */
  virtual ~TaskBaseModel() = default;
};

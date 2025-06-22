/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <shared_mutex>

#include "command_center.hpp"
#include "model_executor_structs.hpp"
#include "task_base_model.hpp"

class CommandCenter;
class BasePreProcessor;

/**
 * @brief Legacy APIs to orchestrate model execution with the ONNX executor.
 *
 * These APIs are deprecated and should not be used/enhanced. Instead use TaskBaseModel.
 */
class BaseModel : public TaskBaseModel {
 protected:
  ModelInfo _info;
  std::map<std::string, int> _inputNamesToIdMap;
  std::map<std::string, BasePreProcessor*> _input2PreprocessorMap;
  bool allocate_output_memory(InferenceReturn* ret);
  int load_inputs(const std::string& inferId, const InferenceRequest& req,
                  std::vector<SavedInputTensor>& preprocessorInputsToFill, bool can_save_input);
  int process_preprocessor_inputs(const std::string& inferId, const InferenceRequest& req,
                                  std::vector<SavedInputTensor>& preprocessorInputsToFill);

  void initialize_model() override final;
  void print_input() override;
  void print_output() override;
  void print_tensors(bool forInput, const std::vector<TensorInfo>& tensorsInfo) override;
  bool check_input(const std::string& inferId, int inputIndex, int dataType,
                   int inputSizeBytes) override;

 public:
  BaseModel(const ModelInfo& modelInfo, const std::string& plan, const std::string& version,
            const std::string& modelId, const nlohmann::json& epConfig, const int epConfigVersion,
            CommandCenter* commandCenter);

  int get_inference(const std::string& inferId, const InferenceRequest& req, InferenceReturn* ret,
                    std::vector<SavedInputTensor>& preprocessorInputsToFill,
                    bool can_save_input) override final;

  BaseModel(const BaseModel& other) = delete;
  BaseModel(BaseModel&& other) = delete;
  BaseModel& operator=(BaseModel&& other) = delete;
  ~BaseModel() = default;
};

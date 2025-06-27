/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include "command_center.hpp"
#include "data_variable.hpp"
#include "future_data_variable.hpp"
#include "single_variable.hpp"
#include "task_base_model.hpp"

class CommandCenter;

/*
DELITEPY_DOC_BLOCK_BEGIN
from delitepy.nimblenet.tensor.tensor import Tensor

class Model:
    """
    Model class is used to interact with AI and ML models from your DelitePy workflow scripts.
    It can be used to perform different actions like loading the model from disk, checking its status and executing it.
    """
DELITEPY_DOC_BLOCK_END
*/
/**
 * @brief Represents a model variable in the NimbleNet system for AI/ML model interactions
 *
 * This class provides functionality to load, manage, and execute AI/ML models within the DeliteAI SDK.
 * It inherits from DataVariable and implements model-specific operations like inference execution
 * and status checking.
 */
class ModelNimbleNetVariable final : public DataVariable {
  CommandCenter* _commandCenter; /**< Pointer to the command center for SDK operations */
  std::string _modelName; /**< Name identifier of the loaded model */
  std::shared_ptr<TaskBaseModel> _model; /**< Shared pointer to the underlying model implementation */

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::NIMBLENET; }

  /*
  DELITEPY_DOC_BLOCK_BEGIN

    def run(self, *args:Tensor) -> tuple[Tensor, ...]:
        """
        Run the model to get inference output, given the inputs.

        Parameters
        ----------
        input : *Tensor
            Inputs tensors to the model in the order they are expected in the model.

        Returns
        ----------
        modelOutput : tuple[Tensor, ...]
            Returns the output tensors of model as a tuple. The order of tensors is the same as defined during model construction.
        """
        pass
  DELITEPY_DOC_BLOCK_END
  */
  /**
   * @brief Executes the model inference with provided input arguments
   *
   * Validates input arguments against model requirements, performs inference,
   * records timing metrics, and handles errors gracefully.
   *
   * @param arguments Vector of input tensors for model inference
   * @return OpReturnType containing the model output or NoneVariable on failure
   * @throws std::exception if input count doesn't match model requirements
   */
  OpReturnType run_model(const std::vector<OpReturnType>& arguments);

  /*
  DELITEPY_DOC_BLOCK_BEGIN

    def status(self) -> bool:
        """
        Checks whether the model is loaded successfully or not in the DeliteAI SDK. If the return value is True then the model is ready for inference.

        Returns
        ----------
        status : bool
            True if the model is loaded else False.
        """
        pass
  DELITEPY_DOC_BLOCK_END
  */
  OpReturnType get_model_status(const std::vector<OpReturnType>& arguments) {
    THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::GETMODELSTATUS);
    return OpReturnType(new SingleVariable<bool>(true));
  }

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override {
    switch (memberFuncIndex) {
      case MemberFuncType::RUNMODEL:
        return run_model(arguments);
      case MemberFuncType::GETMODELSTATUS:
        return get_model_status(arguments);
    }
    THROW("%s not implemented for nimblenet",
          DataVariable::get_member_func_string(memberFuncIndex));
  }

  nlohmann::json to_json() const override { return "[Model]"; }

 public:
  /*
  DELITEPY_DOC_BLOCK_BEGIN

    def __init__(self, modelName: str) -> 'Model':
        """
        Create a new model object. And provides instructions to the DeliteAI SDK to load the model and keep it ready for usage.

        Parameters
        ----------
        modelName : str
            Name of the ML model for the DeliteAI SDK to load.
        """
        pass
  DELITEPY_DOC_BLOCK_END
  */
  ModelNimbleNetVariable(CommandCenter* commandCenter, const std::string& modelName,
                         std::shared_ptr<TaskBaseModel> model) {
    _commandCenter = commandCenter;
    _modelName = modelName;
    _model = model;
  }

  /**
   * @brief Asynchronously loads a model by name
   *
   * Creates a future data variable that will contain the loaded model once
   * the asset loading job completes.
   *
   * @param modelName Name of the model to load
   * @param commandCenter Pointer to the command center for deployment access
   * @return std::shared_ptr<FutureDataVariable> Future containing the model once loaded
   */
  static std::shared_ptr<FutureDataVariable> load_async(const std::string& modelName,
                                                        CommandCenter* commandCenter);

  std::string print() override { return fallback_print(); }
};

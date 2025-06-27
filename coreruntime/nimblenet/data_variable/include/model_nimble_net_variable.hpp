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
class ModelNimbleNetVariable final : public DataVariable {
  CommandCenter* _commandCenter;
  std::string _modelName;
  std::shared_ptr<TaskBaseModel> _model;

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

  static std::shared_ptr<FutureDataVariable> load_async(const std::string& modelName,
                                                        CommandCenter* commandCenter);

  std::string print() override { return fallback_print(); }
};

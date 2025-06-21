/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "base_model.hpp"

#include "user_events_manager.hpp"
#ifdef ONNX_EXECUTOR
#include "onnx_model.hpp"
typedef class ONNXModel Model;
#endif  // ONNX_EXECUTOR

void BaseModel::initialize_model() {
  load_model_from_buffer();
  for (int i = 0; i < _info.inputs.size(); i++) {
    if (_info.inputs[i].toPreprocess) {
      BasePreProcessor* preprocessor = nullptr;
      int dataType = _info.inputs[i].dataType;
      switch (dataType) {
        case DATATYPE::FLOAT:
        case DATATYPE::INT64:
        case DATATYPE::INT32:
        case DATATYPE::DOUBLE:
          preprocessor = _commandCenter->get_userEventsManager().create_preprocessor(
              _info.inputs[i].preprocessorJson, static_cast<DATATYPE>(dataType));
          break;
        default:
          LOG_TO_CLIENT_ERROR("Preprocessor not defined for type=%d", _info.inputs[i].dataType);
          throw std::runtime_error("Preprocessor not defined for type=" +
                                   std::to_string(_info.inputs[i].dataType));
      }
      if (!preprocessor) {
        LOG_TO_CLIENT_ERROR("Could not create preprocessor for inputName=%s",
                            _info.inputs[i].name.c_str());
        throw std::runtime_error("Could not create preprocessor for inputName=" +
                                 _info.inputs[i].name);
      }
      _input2PreprocessorMap[_info.inputs[i].name] = preprocessor;
    }
    _inputNamesToIdMap[_info.inputs[i].name] = i;
  }
}

BaseModel::BaseModel(const ModelInfo& modelInfo, const std::string& plan,
                     const std::string& version, const std::string& modelId,
                     const nlohmann::json& executionProviderConfig, const int epConfigVersion,
                     CommandCenter* commandCenter)
    : TaskBaseModel(plan, version, modelId, executionProviderConfig, epConfigVersion,
                    commandCenter) {
  std::lock_guard<std::mutex> locker(_modelMutex);
  // super(plan, version, modelId, executionProviderConfig, epConfigVersion, commandCenter);
  // _modelBuffer = plan;
  // _epConfig = executionProviderConfig;
  // _epConfigVersion = epConfigVersion;
  // _commandCenter = commandCenter;
  _info = modelInfo;
  // _version = version;
  // _modelId = modelId;
}

// Memory allocated here must be freed by the consumer (android/ios). The
// consumer may not support new/delete, hence use malloc here.
bool BaseModel::allocate_output_memory(InferenceReturn* ret) {
  ret->numOutputs = _info.outputs.size();
  ret->outputs = (void**)malloc(sizeof(void*) * ret->numOutputs);
  ret->outputNames = (char**)malloc(sizeof(char*) * ret->numOutputs);
  ret->outputTypes = (int*)malloc(sizeof(int) * ret->numOutputs);
  ret->outputShapes = (int**)malloc(sizeof(int*) * ret->numOutputs);
  ret->outputLengths = (int*)malloc(sizeof(int) * ret->numOutputs);
  ret->outputShapeLengths = (int*)malloc(sizeof(int) * ret->numOutputs);

  for (int i = 0; i < ret->numOutputs; i++) {
    ret->outputs[i] = nullptr;
    ret->outputShapes[i] = nullptr;
  }

  // iterate over outputs size to check the ONNX data type's to allocate memory
  // for output
  for (int i = 0; i < ret->numOutputs; i++) {
    int fieldSize = util::get_field_size_from_data_type(_info.outputs[i].dataType);
    if (fieldSize == 0) return false;
    ret->outputs[i] = malloc(_info.outputs[i].size * fieldSize);

    // give pointer of out-data buffer to tensor
    if (create_output_tensor_and_set_data_ptr(i, ret->outputs[i]) != SUCCESS) return false;

    ret->outputShapeLengths[i] = _info.outputs[i].shape.size();
    ret->outputShapes[i] = (int*)malloc(sizeof(int) * ret->outputShapeLengths[i]);
    for (int j = 0; j < _info.outputs[i].shape.size(); j++)
      ret->outputShapes[i][j] = _info.outputs[i].shape[j];
    ret->outputLengths[i] = _info.outputs[i].size;
    ret->outputNames[i] = const_cast<char*>(_info.outputs[i].name.c_str());
    ret->outputTypes[i] = _info.outputs[i].dataType;
  }
  return true;
}

int BaseModel::process_preprocessor_inputs(
    const std::string& inferId, const InferenceRequest& req,
    std::vector<SavedInputTensor>& preprocessorInputsToFill) {
  for (auto preprocessorInputInfo : _info.preprocessorInputs) {
    // loading client input
    int clientInputIndex = -1;
    for (int j = 0; j < req.numInputs; j++) {
      if (preprocessorInputInfo.name == req.inputs[j].name) {
        clientInputIndex = j;
        break;
      }
    }
    if (clientInputIndex == -1) {
      LOG_TO_CLIENT_ERROR("Id:%s Inference: preprocessorInputName=%s not provided for model %s",
                          inferId.c_str(), preprocessorInputInfo.name.c_str(), _modelId.c_str());
      return TERMINAL_ERROR;
    }
    if (req.inputs[clientInputIndex].dataType != DATATYPE::JSON) {
      LOG_TO_CLIENT_ERROR(
          "For inputName=%s DataType=%d (interpreted as ModelInput), "
          "but should be of type "
          "UserInput",
          req.inputs[clientInputIndex].name, req.inputs[clientInputIndex].dataType);
      return TERMINAL_ERROR;
    }
    for (auto inputName : preprocessorInputInfo.inputNames) {
      int preprocessorInputIndex = -1;
      if (_inputNamesToIdMap.find(inputName) == _inputNamesToIdMap.end()) {
        LOG_TO_CLIENT_ERROR(
            "Id:%s Inference: inputName=%s does not exist for model "
            "%s given in "
            "preprocessorInput=%s",
            inferId.c_str(), inputName.c_str(), _modelId.c_str());
        return TERMINAL_ERROR;
      } else {
        preprocessorInputIndex = _inputNamesToIdMap[inputName];
      }
      if (!_info.inputs[preprocessorInputIndex].toPreprocess) {
        LOG_TO_CLIENT_ERROR(
            "Id:%s Inference: inputName=%s does not contain a "
            "preprocessor, for modelId=%s given "
            "in preprocessorInput=%s",
            inferId.c_str(), inputName.c_str(), _modelId.c_str());
        return TERMINAL_ERROR;
      }
      if (_input2PreprocessorMap.find(inputName) == _input2PreprocessorMap.end()) {
        LOG_TO_CLIENT_ERROR("preprocessor not found for input=%s", inputName.c_str());
        return TERMINAL_ERROR;
      }
      auto userInput = _input2PreprocessorMap[inputName]->get_model_input(
          *((nlohmann::json*)req.inputs[clientInputIndex].data));
      if (!userInput) {
        LOG_TO_CLIENT_ERROR("preprocessor feature input not valid for inputName=%s modelId=%s",
                            _info.inputs[preprocessorInputIndex].name.c_str(), _modelId.c_str());
        return TERMINAL_ERROR;
      }
      preprocessorInputsToFill.push_back(
          SavedInputTensor(userInput, &_info.inputs[preprocessorInputIndex]));
      if (!check_input(inferId, preprocessorInputIndex,
                       _info.inputs[preprocessorInputIndex].dataType,
                       userInput->length * util::get_field_size_from_data_type(
                                               _info.inputs[preprocessorInputIndex].dataType))) {
        return TERMINAL_ERROR;
      }
      if (create_input_tensor_and_set_data_ptr(preprocessorInputIndex, userInput->data.get()) !=
          SUCCESS)
        return TERMINAL_ERROR;
    }
  }
  return SUCCESS;
}

int BaseModel::load_inputs(const std::string& inferId, const InferenceRequest& req,
                           std::vector<SavedInputTensor>& inputsToFill, bool can_save_input) {
  std::vector<std::shared_lock<std::shared_mutex>> readLocks(_info.inputs.size());
  std::lock_guard<std::mutex> locker(_modelMutex);
  // setting the data pointer of generated tensors to available input data
  // buffer
  for (int i = 0; i < _info.inputs.size(); i++) {
    if (_info.inputs[i].toPreprocess) {
      // Filled later from input values from corresponding Preprocessor classes.
      continue;
    } else {
      // loading client input
      int clientInputIndex = -1;
      for (int j = 0; j < req.numInputs; j++) {
        if (_info.inputs[i].name == req.inputs[j].name) {
          clientInputIndex = j;
          break;
        }
      }
      if (clientInputIndex == -1) {
        LOG_TO_CLIENT_ERROR("Id:%s Inference: inputName=%s not provided for model %s",
                            inferId.c_str(), _info.inputs[i].name.c_str(), _modelId.c_str());
        return TERMINAL_ERROR;
      }
      if (!check_input(
              inferId, i, req.inputs[clientInputIndex].dataType,
              req.inputs[clientInputIndex].length *
                  util::get_field_size_from_data_type(req.inputs[clientInputIndex].dataType))) {
        return TERMINAL_ERROR;  // error logged in check_input
      }
      if (create_input_tensor_and_set_data_ptr(i, req.inputs[clientInputIndex].data) != SUCCESS)
        return TERMINAL_ERROR;
      if (can_save_input) {
        int numOfBytesOfInput = _info.inputs[i].size * util::get_field_size_from_data_type(
                                                           req.inputs[clientInputIndex].dataType);
        void* copyOfInput = operator new[](numOfBytesOfInput);
        memcpy(copyOfInput, req.inputs[clientInputIndex].data, numOfBytesOfInput);
        auto userInput = std::make_shared<ModelInput>(copyOfInput, _info.inputs[i].size);
        inputsToFill.push_back(SavedInputTensor(userInput, &_info.inputs[i]));
      }
    }
  }
  if (process_preprocessor_inputs(inferId, req, inputsToFill) != SUCCESS) {
    return TERMINAL_ERROR;
  }
  return SUCCESS;
}

int BaseModel::get_inference(const std::string& inferId, const InferenceRequest& req,
                             InferenceReturn* ret, std::vector<SavedInputTensor>& inputsToFill,
                             bool can_save_input) {
  if (load_inputs(inferId, req, inputsToFill, can_save_input) != SUCCESS) {
    return TERMINAL_ERROR;
  }
  if (!allocate_output_memory(ret)) {
    deallocate_output_memory(ret);
    return TERMINAL_ERROR;
  }
  if (_commandCenter->get_config()->isDebug()) {
    print_input();
  }

  int status = invoke_inference(ret);

  if (status == SUCCESS && _commandCenter->get_config()->isDebug()) {
    print_output();
  }
  return status;
}

void BaseModel::print_input() { print_tensors(true, _info.inputs); }

void BaseModel::print_output() { print_tensors(false, _info.outputs); }

void BaseModel::print_tensors(bool forInput, const std::vector<TensorInfo>& tensorsInfo) {
  for (int i = 0; i < tensorsInfo.size(); i++) {
    std::vector<int64_t> shape = tensorsInfo[i].shape;
    std::string tensorString = "";
    int dataType = tensorsInfo[i].dataType;
    void* dataBuffPointer;
    if (forInput)
      dataBuffPointer = get_data_buff_input_tensor(i);
    else
      dataBuffPointer = get_data_buff_output_tensor(i);
    if (dataBuffPointer == nullptr) {
      LOG_TO_DEBUG("Index:%d out of bounds for tensors of size:%d", i, tensorsInfo.size());
      return;
    }
    switch (dataType) {
      case DATATYPE::STRING:
        tensorString = util::recursive_string(shape, 0, static_cast<char**>(dataBuffPointer), 0,
                                              tensorsInfo[i].size);
        for (int j = 0; j < tensorsInfo[i].size; j++) {
          free((void*)(static_cast<char**>(dataBuffPointer))[j]);
        }
        free(dataBuffPointer);
        break;
      case DATATYPE::FLOAT:
        tensorString = util::recursive_string(shape, 0, static_cast<float*>(dataBuffPointer), 0,
                                              tensorsInfo[i].size);
        break;
      case DATATYPE::INT32:
        tensorString = util::recursive_string(shape, 0, static_cast<int32_t*>(dataBuffPointer), 0,
                                              tensorsInfo[i].size);
        break;
      case DATATYPE::DOUBLE:
        tensorString = util::recursive_string(shape, 0, static_cast<double*>(dataBuffPointer), 0,
                                              tensorsInfo[i].size);
        break;
      case DATATYPE::INT64:
        tensorString = util::recursive_string(shape, 0, static_cast<int64_t*>(dataBuffPointer), 0,
                                              tensorsInfo[i].size);
        break;
    }
    LOG_TO_CLIENT_DEBUG("CLIENTDEBUG: %s=%s", tensorsInfo[i].name.c_str(), tensorString.c_str());
  }
}

bool BaseModel::check_input(const std::string& inferId, int inputIndex, int dataType,
                            int inputSizeBytes) {
  int fieldSize = util::get_field_size_from_data_type(dataType);
  if (fieldSize == 0) return false;  // inputType not supported
  int expectedSizeBytes = _info.inputs[inputIndex].size * fieldSize;
  if (expectedSizeBytes != inputSizeBytes) {
    LOG_TO_CLIENT_ERROR(
        "Id:%s Inference: inputName=%s is of wrong length=%d, should "
        "be of length=%d",
        inferId.c_str(), _info.inputs[inputIndex].name.c_str(), inputSizeBytes, expectedSizeBytes);
    return false;
  } else if (_info.inputs[inputIndex].dataType != dataType) {
    LOG_TO_CLIENT_ERROR(
        "Id:%s Inference: inputName=%s should be of dataType=%d, "
        "given input dataType=%d",
        inferId.c_str(), _info.inputs[inputIndex].name.c_str(), _info.inputs[inputIndex].dataType,
        dataType);
    return false;
  }
  return true;
}

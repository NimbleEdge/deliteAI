/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "onnx_model.hpp"

#include "log_sender.hpp"

Ort::Env ONNXModel::_myEnv =
    Ort::Env(OrtLoggingLevel::ORT_LOGGING_LEVEL_FATAL, "ONNX  Inference Environment");

int ONNXModel::create_input_tensor_and_set_data_ptr(const int index, void* dataPtr) {
  try {
    if (_info.inputs[index].dataType == DATATYPE::STRING) {
      _inputTensors[index] =
          Ort::Value::CreateTensor(_allocator, _info.inputs[index].shape.data(),
                                   _info.inputs[index].size, ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING);
      _inputTensors[index].FillStringTensor(static_cast<const char**>(dataPtr),
                                            _info.inputs[index].size);
    } else {
      int fieldSize = util::get_field_size_from_data_type(_info.inputs[index].dataType);
      _inputTensors[index] = Ort::Value::CreateTensor(
          _memoryInfo, dataPtr, fieldSize * _info.inputs[index].size,
          _info.inputs[index].shape.data(), _info.inputs[index].shape.size(),
          (ONNXTensorElementDataType)_info.inputs[index].dataType);
    }
  } catch (Ort::Exception& e) {
    LOG_TO_CLIENT_ERROR(
        "Exception in set_input_tensor_and_set_data_ptr:%s with errorCode:%d, for modelId=%s",
        e.what(), e.GetOrtErrorCode(), _modelId.c_str());
    return TERMINAL_ERROR;
  } catch (...) {
    LOG_TO_CLIENT_ERROR("Failed to create Input Tensor with index:%d for modelId:%d", index,
                        _modelId.c_str());
    return TERMINAL_ERROR;
  }
  return SUCCESS;
}

int ONNXModel::create_output_tensor_and_set_data_ptr(const int index, void* dataPtr) {
  try {
    if (_info.outputs[index].dataType == DATATYPE::STRING) {
      _outputTensors[index] =
          Ort::Value::CreateTensor(_allocator, _info.outputs[index].shape.data(),
                                   _info.outputs[index].size, ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING);
      _outputTensors[index].FillStringTensor(static_cast<const char**>(dataPtr),
                                             _info.outputs[index].size);
    } else {
      int fieldSize = util::get_field_size_from_data_type(_info.outputs[index].dataType);
      _outputTensors[index] = Ort::Value::CreateTensor(
          _memoryInfo, dataPtr, fieldSize * _info.outputs[index].size,
          _info.outputs[index].shape.data(), _info.outputs[index].shape.size(),
          (ONNXTensorElementDataType)_info.outputs[index].dataType);
    }
  } catch (Ort::Exception& e) {
    LOG_TO_CLIENT_ERROR(
        "Exception in set_output_tensor_and_set_data_ptr:%s with errorCode:%d, for modelId=%s",
        e.what(), e.GetOrtErrorCode(), _modelId.c_str());
    return TERMINAL_ERROR;
  } catch (...) {
    LOG_TO_CLIENT_ERROR("Failed to create Output Tensor with index:%d for modelId:%d", index,
                        _modelId.c_str());
    return TERMINAL_ERROR;
  }
  return SUCCESS;
}

int ONNXModel::invoke_inference(InferenceReturn* ret) {
  try {
    _session->Run(Ort::RunOptions{nullptr}, _inputNames.data(), _inputTensors.data(),
                  _inputNames.size(), _outputNames.data(), _outputTensors.data(),
                  _outputNames.size());
  } catch (Ort::Exception& e) {
    LOG_TO_CLIENT_ERROR("Exception in get_inference:%s with errorCode:%d, for modelId=%s", e.what(),
                        e.GetOrtErrorCode(), _modelId.c_str());
    deallocate_output_memory(ret);
    return TERMINAL_ERROR;
  } catch (...) {
    LOG_TO_CLIENT_ERROR("Exception in get_inference ONNXSessionRun for modelId=%s",
                        _modelId.c_str());
    deallocate_output_memory(ret);
    return TERMINAL_ERROR;
  }
  return SUCCESS;
}

void* ONNXModel::get_data_buff_input_tensor(const int index) {
  if (index >= _inputTensors.size()) return nullptr;
  if (_info.inputs[index].dataType == DATATYPE::STRING) {
    int length = _inputTensors[index].GetTensorTypeAndShapeInfo().GetElementCount();
    char** data = (char**)malloc(length * sizeof(char*));

#ifndef IOS
    for (int i = 0; i < length; i++) {
      const auto& stringdata = _inputTensors[index].GetStringTensorElement(i);
      asprintf(&data[i], "%s", stringdata.c_str());
    }
#endif  // IOS

    return static_cast<void*>(data);
  }

  return _inputTensors[index].GetTensorMutableData<void*>();
}

void* ONNXModel::get_data_buff_output_tensor(const int index) {
  if (index >= _outputTensors.size()) return nullptr;
  if (_info.outputs[index].dataType == DATATYPE::STRING) {
    int length = _outputTensors[index].GetTensorTypeAndShapeInfo().GetElementCount();
    char** data = (char**)malloc(length * sizeof(char*));

#ifndef IOS
    for (int i = 0; i < length; i++) {
      const auto& stringdata = _inputTensors[index].GetStringTensorElement(i);
      asprintf(&data[i], "%s", stringdata.c_str());
    }
#endif  // IOS

    return static_cast<void*>(data);
  }
  return _outputTensors[index].GetTensorMutableData<void*>();
}

Ort::SessionOptions ONNXModel::get_session_options_from_json(const nlohmann::json& epConfig) {
  Ort::SessionOptions sessionOptions;
  sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
  std::string configProviderName = epConfig.find("providerName") != epConfig.end()
                                       ? epConfig["providerName"].get<std::string>()
                                       : "";
#if defined(__ANDROID__) && defined(NNAPI)
  if (configProviderName == "XNNPACK") {
    std::unordered_map<std::string, std::string> provider_options = {};
    if (epConfig.find("providerSettings") != epConfig.end()) {
      provider_options =
          epConfig["providerOptions"].get<std::unordered_map<std::string, std::string>>();
    }
    sessionOptions.AppendExecutionProvider("XNNPACK", provider_options);
  } else if (configProviderName == "NNAPI") {
    uint32_t nnapi_flags = 0;
    if (epConfig.find("providerSettings") != epConfig.end()) {
      nnapi_flags = epConfig["providerSettings"].get<uint32_t>();
    }
    Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_Nnapi(sessionOptions, nnapi_flags));
  }
#elif IOS
#endif
  if (epConfig.find("intraOpNumThreads") != epConfig.end()) {
    sessionOptions.SetIntraOpNumThreads(epConfig["intraOpNumThreads"].get<int>());
  }
  if (epConfig.find("intraOpSpinning") != epConfig.end()) {
    std::string spinning = epConfig["intraOpSpinning"].get<std::string>();
    sessionOptions.AddConfigEntry("session.intra_op.allow_spinning", spinning.c_str());
  }
  if (epConfig.find("interOpNumThreads") != epConfig.end()) {
    sessionOptions.SetExecutionMode(ExecutionMode::ORT_PARALLEL);
    sessionOptions.SetInterOpNumThreads(epConfig["interOpNumThreads"].get<int>());
  }
  return sessionOptions;
}

void ONNXModel::load_model_from_buffer() {
  Ort::CustomOpDomain nimbleedge_operator_domain{"com.nimbleedge"};
  register_custom_onnx_operators(nimbleedge_operator_domain);

  for (auto& epConfig : _epConfig) {
    std::string epConfigString = epConfig.dump();
    try {
      // If os field found in epConfig and is not a substring of PLATFORM don't then load model with
      // epConfig
      if (epConfig.find("os") != epConfig.end()) {
        std::string os = epConfig.at("os");
        if (PLATFORM.find(os) == std::string::npos) {
          LOG_TO_DEBUG(
              "epConfig=%s not loaded for modelId=%s and version=%s as os=%s not compatible with "
              "platform=%s.",
              epConfigString.c_str(), _modelId.c_str(), _version.c_str(), os.c_str(),
              PLATFORM.c_str());
          continue;
        }
      }

      _sessionOptions = get_session_options_from_json(epConfig);
      _sessionOptions.Add(nimbleedge_operator_domain);
      _session =
          new Ort::Session(_myEnv, _modelBuffer.c_str(), _modelBuffer.length(), _sessionOptions);
      LOG_TO_DEBUG("Created ONNX Model for model=%s, version=%s, with epConfig=%s",
                   _modelId.c_str(), _version.c_str(), epConfigString.c_str());
      return;
    } catch (std::exception& e) {
      LOG_TO_CLIENT_ERROR(
          "Could not load model %s with the ep Config specified: %s with the error: %s",
          _modelId.c_str(), epConfigString.c_str(), e.what());
    }
  }
  // model should have loaded by now, if not loaded using default options
  Ort::SessionOptions newSessionOptions;
  newSessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
  _sessionOptions = std::move(newSessionOptions);
  _sessionOptions.Add(nimbleedge_operator_domain);
  _session = new Ort::Session(_myEnv, _modelBuffer.c_str(), _modelBuffer.length(), _sessionOptions);
  //_modelBuffer is not used anywhere hence clearing (onnx maintains the
  // buffer by itself)
  _modelBuffer.clear();
}

ONNXModel::ONNXModel(const ModelInfo& modelInfo, const std::string& plan,
                     const std::string& version, const std::string& modelId,
                     const std::vector<nlohmann::json>& executionProviderConfig,
                     const int epConfigVersion, CommandCenter* commandCenter)
    : BaseModel(modelInfo, plan, version, modelId, executionProviderConfig, epConfigVersion,
                commandCenter),
      _memoryInfo(Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator,
                                             OrtMemType::OrtMemTypeDefault)) {
  const auto& ortApi = Ort::GetApi();
  Ort::ThrowOnError(ortApi.GetAllocatorWithDefaultOptions(&_allocator));
  for (int i = 0; i < _info.inputs.size(); i++) _inputTensors.push_back(Ort::Value(nullptr));
  for (int i = 0; i < _info.outputs.size(); i++) _outputTensors.push_back(Ort::Value(nullptr));

  for (int i = 0; i < _info.inputs.size(); i++) {
    _inputNames.push_back(_info.inputs[i].name.c_str());
  }
  for (int i = 0; i < _info.outputs.size(); i++) {
    _outputNames.push_back(_info.outputs[i].name.c_str());
  }
  initialize_model();
  run_dummy_inference();
}

void ONNXModel::delete_input_memory(void** input, int size) {
  for (int i = 0; i < size; i++) {
    if (_info.inputs[i].dataType == DATATYPE::STRING) continue;
    free(input[i]);
  }
  free(input);
}

void ONNXModel::run_dummy_inference() {
  void** input;
  InferenceReturn ret;
  std::vector<const char*> inputNames;
  std::vector<std::vector<const char*>> stringinputs;
  int numInputs = _info.inputs.size();
  input = (void**)malloc(sizeof(void*) * numInputs);
  for (int i = 0; i < _info.inputs.size(); i++) {
    inputNames.push_back(_info.inputs[i].name.c_str());
    if (_info.inputs[i].dataType == DATATYPE::STRING) {
      std::vector<const char*> inputData(_info.inputs[i].size, "aaa");
      stringinputs.push_back(inputData);
      input[i] = (void*)stringinputs.back().data();
      create_input_tensor_and_set_data_ptr(i, static_cast<void*>(input[i]));
    } else {
      input[i] = (void*)malloc(util::get_field_size_from_data_type(_info.inputs[i].dataType) *
                               _info.inputs[i].size);
      memset((char*)input[i], 0,
             util::get_field_size_from_data_type(_info.inputs[i].dataType) * _info.inputs[i].size);

      create_input_tensor_and_set_data_ptr(i, static_cast<void*>(input[i]));
    }
  }
  if (!allocate_output_memory(&ret))
    throw std::runtime_error("");  // error already logged in create_ort_tensor

  if (invoke_inference(&ret) == SUCCESS) deallocate_output_memory(&ret);
  // deleting input memory
  delete_input_memory(input, numInputs);
}

ONNXModel::~ONNXModel() { delete _session; }

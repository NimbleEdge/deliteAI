/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "task_onnx_model.hpp"

#include "data_variable.hpp"
#include "nimble_net_util.hpp"
#include "onnx_operators.hpp"
#include "tensor_data_variable.hpp"

Ort::Env TaskONNXModel::_myEnv =
    Ort::Env(OrtLoggingLevel::ORT_LOGGING_LEVEL_FATAL, "ONNX  Inference Environment");

int TaskONNXModel::create_input_tensor_and_set_data_ptr(const OpReturnType req, int modelInputIndex,
                                                        Ort::Value&& returnedInputTensor) {
  try {
    Ort::Value inputTensor = Ort::Value{nullptr};
    if (req->get_dataType_enum() == DATATYPE::STRING) {
      int numOfElements = req->get_numElements();

      // Get char** from std::vector<std::string> stored in StringTensorVariable
      std::string* s = (std::string*)(req->get_raw_ptr());
      char** strings = new char*[numOfElements];
      for (int i = 0; i < numOfElements; i++) {
        strings[i] = (char*)(s[i].c_str());
      }
      inputTensor =
          Ort::Value::CreateTensor(_allocator, req->get_shape().data(), req->get_shape().size(),
                                   ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING);
      inputTensor.FillStringTensor(strings, numOfElements);
      delete[] strings;
    } else {
      int fieldSize = util::get_field_size_from_data_type(req->get_dataType_enum());
      inputTensor = Ort::Value::CreateTensor(_memoryInfo, req->get_raw_ptr(),
                                             fieldSize * req->get_numElements(),
                                             req->get_shape().data(), req->get_shape().size(),
                                             (ONNXTensorElementDataType)req->get_dataType_enum());
    }
    returnedInputTensor = std::move(inputTensor);
    return SUCCESS;
  } catch (Ort::Exception& e) {
    LOG_TO_CLIENT_ERROR(
        "Exception in set_input_tensor_and_set_data_ptr:%s with errorCode:%d, for modelId=%s",
        e.what(), e.GetOrtErrorCode(), _modelId.c_str());
    return TERMINAL_ERROR;
  } catch (...) {
    LOG_TO_CLIENT_ERROR("Failed to create Input Tensor : %s for modelId:%s",
                        _inputNames[modelInputIndex], _modelId.c_str());
    return TERMINAL_ERROR;
  }
  return TERMINAL_ERROR;
}

int TaskONNXModel::invoke_inference(OpReturnType& ret,
                                    const std::vector<Ort::Value>& inputTensors) {
  try {
    std::vector<Ort::Value> output_onnx_tensors =
        _session->Run(Ort::RunOptions{nullptr}, _inputNames.data(), inputTensors.data(),
                      _inputNames.size(), _outputNames.data(), _outputNames.size());
    assert(output_onnx_tensors.front().IsTensor());
    std::vector<OpReturnType> outputs_tensors;
    for (int i = 0; i < output_onnx_tensors.size(); i++) {
      outputs_tensors.push_back(
          get_tensor_variable_from_onnx_tensor(std::move(output_onnx_tensors[i])));
    }
    ret = std::make_shared<TupleDataVariable>(outputs_tensors);
  }

  catch (Ort::Exception& e) {
    LOG_TO_CLIENT_ERROR("Exception in get_inference:%s with errorCode:%d, for modelId=%s", e.what(),
                        e.GetOrtErrorCode(), _modelId.c_str());
    return TERMINAL_ERROR;
  }

  catch (...) {
    LOG_TO_CLIENT_ERROR("Exception in get_inference ONNXSessionRun for modelId=%s",
                        _modelId.c_str());
    return TERMINAL_ERROR;
  }

  return SUCCESS;
}

OpReturnType TaskONNXModel::get_tensor_variable_from_onnx_tensor(Ort::Value onnx_tensor) {
  Ort::TensorTypeAndShapeInfo tensor_info = onnx_tensor.GetTensorTypeAndShapeInfo();
  auto dataType = (DATATYPE)tensor_info.GetElementType();
  switch (dataType) {
    case DATATYPE::FLOAT:
    case DATATYPE::DOUBLE:
    case DATATYPE::INT32:
    case DATATYPE::INT64:
      return OpReturnType(new OrtTensorVariable(std::move(onnx_tensor), dataType));
    case DATATYPE::STRING: {
      std::vector<std::string> strings;
      for (int i = 0; i < tensor_info.GetElementCount(); i++) {
        strings.push_back(onnx_tensor.GetStringTensorElement(i));
      }

      return OpReturnType(new StringTensorVariable(std::move(strings), tensor_info.GetShape(),
                                                   tensor_info.GetShape().size()));
    }
    default:
      LOG_TO_ERROR(
          "Requested data type = %d not supported when converting ONNX tensor to DataVariable.",
          tensor_info.GetElementType());
      THROW("%s", "Unsupported dataType returned from model.");
  }
  THROW("%s", "Unsupported dataType returned from model.");
}

Ort::SessionOptions TaskONNXModel::get_session_options_from_json(const nlohmann::json& epConfig) {
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

#ifdef ORT_EXTENSIONS
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

OrtStatus* ORT_API_CALL RegisterCustomOps(OrtSessionOptions* options, const OrtApiBase* api);

int ORT_API_CALL GetActiveOrtAPIVersion();

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // ORT_EXTENSIONS

void add_common_session_options(Ort::SessionOptions& sessionOptions) {
  sessionOptions.AddConfigEntry("session.use_ort_model_bytes_directly", "1");
#ifdef ORT_EXTENSIONS
  Ort::ThrowOnError(RegisterCustomOps((OrtSessionOptions*)sessionOptions, OrtGetApiBase()));
#endif  // ORT_EXTENSIONS
}

void TaskONNXModel::load_model_from_buffer() {
  Ort::CustomOpDomain deliteai_operator_domain{"dev.deliteai"};
  register_custom_onnx_operators(deliteai_operator_domain);
  nlohmann::json epConfigListToCheck = nlohmann::json::array();
#if defined(__ANDROID__)
  if (_epConfig.contains("android")) {
    epConfigListToCheck = _epConfig.at("android");
  }
#elif IOS
  if (_epConfig.contains("ios")) {
    epConfigListToCheck = _epConfig.at("ios");
  }
#endif
  for (auto& epConfig : epConfigListToCheck) {
    // Only load epConfig which has runtime key present and has value onnx
    if (!epConfig.contains("runtime") || epConfig.at("runtime") != "onnx") {
      continue;
    }
    std::string epConfigString = epConfig.dump();
    try {
      // If os field found in epConfig and is not a substring of PLATFORM then don't load model with
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
      add_common_session_options(_sessionOptions);
      _sessionOptions.Add(deliteai_operator_domain);
      _session =
          new Ort::Session(_myEnv, _modelBuffer.c_str(), _modelBuffer.length(), _sessionOptions);
      LOG_TO_DEBUG("Created ONNX Model for model=%s, version=%s, with epConfig=%s",
                   _modelId.c_str(), _version.c_str(), epConfigString.c_str());
      load_model_meta_data();
      return;
    } catch (std::exception& e) {
      LOG_TO_CLIENT_INFO(
          "Could not load model %s with the ep Config specified: %s with the error: %s",
          _modelId.c_str(), epConfigString.c_str(), e.what());
    }
  }
  // model should have loaded by now, if not loaded using default options
  Ort::SessionOptions newSessionOptions;
  newSessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
  _sessionOptions = std::move(newSessionOptions);
  _sessionOptions.Add(deliteai_operator_domain);
  add_common_session_options(_sessionOptions);
  _session = new Ort::Session(_myEnv, _modelBuffer.c_str(), _modelBuffer.length(), _sessionOptions);
  //_modelBuffer is used directly by ONNX so we have to maintain it as long as the session exists
  load_model_meta_data();
}

void TaskONNXModel::load_model_meta_data() {
  int numInputs = _session->GetInputCount();
  int numOutputs = _session->GetOutputCount();
  for (int i = 0; i < numInputs; i++) {
    Ort::AllocatedStringPtr allocatedInputName = _session->GetInputNameAllocated(i, _allocator);
    int nameSize = std::strlen(allocatedInputName.get());
    char* inputName = new char[nameSize + 1];
    std::strcpy(inputName, allocatedInputName.get());
    inputName[nameSize] = 0;
    _inputNames.push_back(inputName);
  }
  for (int i = 0; i < numOutputs; i++) {
    Ort::AllocatedStringPtr allocatedOutputName = _session->GetOutputNameAllocated(i, _allocator);
    int nameSize = std::strlen(allocatedOutputName.get());
    char* outputName = new char[nameSize + 1];
    std::strcpy(outputName, allocatedOutputName.get());
    outputName[nameSize] = 0;
    _outputNames.push_back(outputName);
  }
}

TaskONNXModel::TaskONNXModel(const std::string& plan, const std::string& version,
                             const std::string& modelId,
                             const nlohmann::json& executionProviderConfig,
                             const int epConfigVersion, CommandCenter* commandCenter,
                             bool runDummyInference)
    : TaskBaseModel(plan, version, modelId, executionProviderConfig, epConfigVersion, commandCenter,
                    runDummyInference),
      _memoryInfo(Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator,
                                             OrtMemType::OrtMemTypeDefault)) {
  const auto& ortApi = Ort::GetApi();
  Ort::ThrowOnError(ortApi.GetAllocatorWithDefaultOptions(&_allocator));

  initialize_model();
  if (_runDummyInference) {
    run_dummy_inference();
  }
}

void TaskONNXModel::run_dummy_inference() {
  int numInputs = _session->GetInputCount();
  std::vector<Ort::Value> inputTensors;
  std::vector<OpReturnType> tensorsToClear;

  for (int i = 0; i < numInputs; i++) {
    Ort::TypeInfo tensor_info = _session->GetInputTypeInfo(i);
    int data_type = tensor_info.GetTensorTypeAndShapeInfo().GetElementType();
    // If any of the dimension is -1 in shape, then its a variable sized input, assume this
    // dimension to be 1
    std::vector<int64_t> shape = tensor_info.GetTensorTypeAndShapeInfo().GetShape();
    for (auto& dim : shape) {
      if (dim == -1) {
        dim = 1;
      }
    }

    Ort::Value inputTensor = Ort::Value{nullptr};
    switch ((DATATYPE)data_type) {
      case DATATYPE::FLOAT:
      case DATATYPE::DOUBLE:
      case DATATYPE::INT32:
      case DATATYPE::INT64: {
        OpReturnType req =
            OpReturnType(new TensorVariable(shape, static_cast<DATATYPE>(data_type)));
        create_input_tensor_and_set_data_ptr(req, i, std::move(inputTensor));
        tensorsToClear.push_back(req);
        break;
      }
      // case DATATYPE::BOOLEAN: {
      //   OpReturnType req = OpReturnType(
      //       new TensorVariable<double>((double*)input, shape.data(), shape_dimensions, false));
      //   create_input_tensor_and_set_data_ptr(req, i);
      //   break;
      // }
      case DATATYPE::STRING: {
        int size = 1;
        for (auto dim : shape) {
          size *= dim;
        }
        std::vector<std::string> strings(size, "dummyString");
        OpReturnType req = OpReturnType(
            new StringTensorVariable(std::move(strings), std::move(shape), shape.size()));
        create_input_tensor_and_set_data_ptr(req, i, std::move(inputTensor));
        tensorsToClear.push_back(req);
        break;
      }
      default:
        LOG_TO_ERROR(
            "Requested data type = %s not supported when converting ONNX tensor to DataVariable.",
            data_type);
        break;
    }
    inputTensors.push_back(std::move(inputTensor));
  }
  OpReturnType ret;
  if (invoke_inference(ret, inputTensors) != SUCCESS) {
    LOG_TO_ERROR("%s", "Dumy inference failed.");
  }
}

TaskONNXModel::~TaskONNXModel() {
  for (auto inputName : _inputNames) {
    delete[] inputName;
  }
  for (auto outputName : _outputNames) {
    delete[] outputName;
  }
  delete _session;
  // not supposed to delete _allocator as we are using default (as per ONNX docs)
}

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Dictionary-based interface usage examples using MapDataVariable:
 *
 * // Example 1: Using MapDataVariable interface for inference
 * OpReturnType inputs = OpReturnType(new MapDataVariable());
 * OpReturnType outputs;
 * auto input_map = std::dynamic_pointer_cast<MapDataVariable>(inputs);
 *
 * // Prepare inputs
 * input_map->set_value_in_map("input_ids", input_ids_tensor);
 * input_map->set_value_in_map("attention_mask", attention_mask_tensor);
 * input_map->set_value_in_map("position_ids", position_ids_tensor);
 *
 * // Add cache inputs
 * for (int i = 0; i < num_layers; i++) {
 *   input_map->set_value_in_map("past_key_values." + std::to_string(i) + ".key", past_key_tensor);
 *   input_map->set_value_in_map("past_key_values." + std::to_string(i) + ".value", past_value_tensor);
 * }
 *
 * // Run inference
 * int result = model->invoke_inference_dict(outputs, inputs);
 * auto output_map = std::dynamic_pointer_cast<MapDataVariable>(outputs);
 *
 * // Access outputs by name
 * auto logits = output_map->get_string_subscript("logits");
 * auto next_token = output_map->get_string_subscript("next_token_id");
 * auto is_eos = output_map->get_string_subscript("is_eos");
 * auto updated_attention = output_map->get_string_subscript("updated_attention_mask");
 *
 * // Example 2: Converting from tuple result to MapDataVariable
 * OpReturnType tuple_result;
 * model->invoke_inference(tuple_result, input_tensors);
 *
 * OpReturnType output_dict = model->convert_tuple_to_dict(tuple_result);
 * auto output_map = std::dynamic_pointer_cast<MapDataVariable>(output_dict);
 *
 * // Now access outputs by name instead of position
 * auto logits = output_map->get_string_subscript("logits");
 */

#include "task_onnx_model.hpp"

#include <unordered_map>
#include "data_variable.hpp"
#include "map_data_variable.hpp"
#include "nimble_net_util.hpp"
#include "nimble_net/config.h"
#include "onnx_operators.hpp"
#include "tensor_data_variable.hpp"
#include "tuple_data_variable.hpp"

#ifdef ORT_EXTENSIONS
DELITEAI_EXTERN_C_BEGIN

OrtStatus* ORT_API_CALL RegisterCustomOps(OrtSessionOptions* options, const OrtApiBase* api);

int ORT_API_CALL GetActiveOrtAPIVersion();

DELITEAI_EXTERN_C_END
#endif  // ORT_EXTENSIONS

// =================================================================================================

namespace {

// Number of threads for the XNNPACK backend
int xnnpack_intra_op_num_threads = 6;

/**
 * @brief Configures common ONNX session options.
 *
 * @param sessionOptions Session options to configure.
 */
void add_common_session_options(Ort::SessionOptions& sessionOptions) {
  sessionOptions.AddConfigEntry("session.use_ort_model_bytes_directly", "1");
#if DELITEAI_TARGET_OS_ANDROID || DELITEAI_TARGET_OS_IOS
  sessionOptions.AppendExecutionProvider(
      "XNNPACK", {std::pair<std::string, std::string>(
                     "intra_op_num_threads", ne::fmt("%d", xnnpack_intra_op_num_threads).str)});
#endif  // DELITEAI_TARGET_OS_ANDROID || DELITEAI_TARGET_OS_IOS
#ifdef ORT_EXTENSIONS
  Ort::ThrowOnError(RegisterCustomOps((OrtSessionOptions*)sessionOptions, OrtGetApiBase()));
#endif  // ORT_EXTENSIONS
}

}  // namespace

// =================================================================================================

Ort::Env TaskONNXModel::_myEnv =
    Ort::Env(OrtLoggingLevel::ORT_LOGGING_LEVEL_FATAL, "ONNX Inference Environment");

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

      // Map DeliteAI DATATYPE to ONNX tensor element data type
      ONNXTensorElementDataType onnxDataType;
      switch (req->get_dataType_enum()) {
        case DATATYPE::FLOAT:
          onnxDataType = ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
          break;
        case DATATYPE::FLOAT16:
          onnxDataType = ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16;
          break;
        case DATATYPE::DOUBLE:
          onnxDataType = ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE;
          break;
        case DATATYPE::INT32:
          onnxDataType = ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32;
          break;
        case DATATYPE::INT64:
          onnxDataType = ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64;
          break;
        case DATATYPE::BOOLEAN:
          onnxDataType = ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL;
          break;
        default:
          LOG_TO_CLIENT_ERROR("Unsupported data type %d for ONNX tensor creation", req->get_dataType_enum());
          return TERMINAL_ERROR;
      }

      inputTensor = Ort::Value::CreateTensor(_memoryInfo, req->get_raw_ptr(),
                                             fieldSize * req->get_numElements(),
                                             req->get_shape().data(), req->get_shape().size(),
                                             onnxDataType);
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

int TaskONNXModel::invoke_inference_dict(OpReturnType& output_dict, const OpReturnType& input_dict) {
  try {
    // Convert input MapDataVariable to vector format for existing inference
    auto input_map = std::dynamic_pointer_cast<MapDataVariable>(input_dict);
    if (!input_map) {
      LOG_TO_CLIENT_ERROR("Input is not a MapDataVariable for modelId=%s", _modelId.c_str());
      return TERMINAL_ERROR;
    }

    std::vector<Ort::Value> inputTensors;
    inputTensors.reserve(_inputNames.size());

    for (size_t i = 0; i < _inputNames.size(); i++) {
      std::string inputName(_inputNames[i]);

      try {
        OpReturnType input_tensor = input_map->get_string_subscript(inputName);
        Ort::Value inputTensor = Ort::Value{nullptr};
        int result = create_input_tensor_and_set_data_ptr(input_tensor, i, std::move(inputTensor));
        if (result != SUCCESS) {
          return result;
        }
        inputTensors.push_back(std::move(inputTensor));
      } catch (...) {
        LOG_TO_CLIENT_ERROR("Missing input tensor '%s' for modelId=%s", inputName.c_str(), _modelId.c_str());
        return TERMINAL_ERROR;
      }
    }

    // Run inference using existing method
    std::vector<Ort::Value> output_onnx_tensors =
        _session->Run(Ort::RunOptions{nullptr}, _inputNames.data(), inputTensors.data(),
                      _inputNames.size(), _outputNames.data(), _outputNames.size());

    // Create output MapDataVariable
    output_dict = OpReturnType(new MapDataVariable());
    auto output_map = std::dynamic_pointer_cast<MapDataVariable>(output_dict);

    for (size_t i = 0; i < output_onnx_tensors.size(); i++) {
      std::string outputName(_outputNames[i]);
      OpReturnType tensor_var = get_tensor_variable_from_onnx_tensor(std::move(output_onnx_tensors[i]));
      output_map->set_value_in_map(outputName, tensor_var);
    }

    return SUCCESS;
  }
  catch (Ort::Exception& e) {
    LOG_TO_CLIENT_ERROR("Exception in invoke_inference_dict:%s with errorCode:%d, for modelId=%s",
                        e.what(), e.GetOrtErrorCode(), _modelId.c_str());
    return TERMINAL_ERROR;
  }
  catch (...) {
    LOG_TO_CLIENT_ERROR("Exception in invoke_inference_dict ONNXSessionRun for modelId=%s",
                        _modelId.c_str());
    return TERMINAL_ERROR;
  }
}

OpReturnType TaskONNXModel::convert_tuple_to_dict(const OpReturnType& tuple_result) {
  try {
    // Check if result is a TupleDataVariable
    auto tuple_var = std::dynamic_pointer_cast<TupleDataVariable>(tuple_result);
    if (!tuple_var) {
      LOG_TO_CLIENT_ERROR("Result is not a TupleDataVariable for modelId=%s", _modelId.c_str());
      return OpReturnType(new NoneVariable());
    }

    // Convert tuple elements to MapDataVariable using output names
    auto tuple_elements = tuple_var->get_members();
    if (tuple_elements.size() != _outputNames.size()) {
      LOG_TO_CLIENT_ERROR("Mismatch between output count (%zu) and expected names (%zu) for modelId=%s",
                          tuple_elements.size(), _outputNames.size(), _modelId.c_str());
      return OpReturnType(new NoneVariable());
    }

    OpReturnType output_dict = OpReturnType(new MapDataVariable());
    auto output_map = std::dynamic_pointer_cast<MapDataVariable>(output_dict);

    for (size_t i = 0; i < tuple_elements.size(); i++) {
      std::string outputName(_outputNames[i]);
      output_map->set_value_in_map(outputName, tuple_elements[i]);
    }

    return output_dict;
  }
  catch (...) {
    LOG_TO_CLIENT_ERROR("Exception in convert_tuple_to_dict for modelId=%s", _modelId.c_str());
    return OpReturnType(new NoneVariable());
  }
}

OpReturnType TaskONNXModel::get_tensor_variable_from_onnx_tensor(Ort::Value onnx_tensor) {
  Ort::TensorTypeAndShapeInfo tensor_info = onnx_tensor.GetTensorTypeAndShapeInfo();
      ONNXTensorElementDataType onnxType = tensor_info.GetElementType();

  // Handle ONNX data type to DATATYPE mapping
  switch (onnxType) {
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
      return OpReturnType(new OrtTensorVariable(std::move(onnx_tensor), DATATYPE::FLOAT));
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE:
      return OpReturnType(new OrtTensorVariable(std::move(onnx_tensor), DATATYPE::DOUBLE));
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32:
      return OpReturnType(new OrtTensorVariable(std::move(onnx_tensor), DATATYPE::INT32));
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
      return OpReturnType(new OrtTensorVariable(std::move(onnx_tensor), DATATYPE::INT64));
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL:
      return OpReturnType(new OrtTensorVariable(std::move(onnx_tensor), DATATYPE::BOOLEAN));
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16:
        return OpReturnType(new OrtTensorVariable(std::move(onnx_tensor), DATATYPE::FLOAT16));
    case ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING: {
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
          onnxType);
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
#if DELITEAI_TARGET_OS_ANDROID && defined(NNAPI)
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
#elif DELITEAI_TARGET_OS_IOS
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

void TaskONNXModel::load_model_from_buffer() {
  Ort::CustomOpDomain deliteai_operator_domain{"dev.deliteai"};
  register_custom_onnx_operators(deliteai_operator_domain);
  nlohmann::json epConfigListToCheck = nlohmann::json::array();
#if DELITEAI_TARGET_OS_ANDROID
  if (_epConfig.contains("android")) {
    epConfigListToCheck = _epConfig.at("android");
  }
#elif DELITEAI_TARGET_OS_IOS
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

void TaskONNXModel::set_xnnpack_intra_op_num_threads(int num_threads) {
  xnnpack_intra_op_num_threads = num_threads;
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
  // if (_runDummyInference) {
  //   run_dummy_inference();
  // }
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
      case DATATYPE::INT64:
      case DATATYPE::FLOAT16: {
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

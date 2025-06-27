#include "nimble_net_data_variable.hpp"

#include <memory>
#include <type_traits>

#include "asset_load_job.hpp"
#include "asset_manager.hpp"
#include "command_center.hpp"
#include "data_variable.hpp"
#include "model_nimble_net_variable.hpp"
#include "nimble_net_util.hpp"
#include "nlohmann/json_fwd.hpp"
#include "pre_processor_nimble_net_variable.hpp"
#include "raw_event_store_data_variable.hpp"

#ifdef GENAI
#include "llm_data_variable.hpp"
#include "llm_utils.hpp"
#include "retriever.hpp"
#endif  // GENAI

OpReturnType NimbleNetDataVariable::create_tensor(const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 2, MemberFuncType::CREATETENSOR);

  std::vector<int64_t> shape;
  for (int i = 0; i < arguments[0]->get_size(); i++) {
    shape.push_back(arguments[0]->get_int_subscript(i)->get_int64());
  }
  auto dtypeString = arguments[1]->get_string();
  auto dtype = util::get_enum_from_string(dtypeString.c_str());
  if (dtype == -1) {
    THROW("zeros() failed %s is not a type", dtypeString.c_str());
  }
  auto tensor = DataVariable::create_tensor(dtype, shape);
  if (tensor == nullptr) {
    THROW("zeros() not implemented for %s", dtypeString.c_str());
  }
  return tensor;
}

OpReturnType NimbleNetDataVariable::load_model(const std::vector<OpReturnType>& arguments,
                                               CallStack& stack) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::LOADMODEL);
  std::string modelId = arguments[0]->get_string();
  return ModelNimbleNetVariable::load_async(modelId, _commandCenter);
}

OpReturnType NimbleNetDataVariable::load_llm(const std::vector<OpReturnType>& arguments,
                                             CallStack& stack) {
#ifdef GENAI
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, LLM);
  if (arguments[0]->get_containerType() != CONTAINERTYPE::MAP) {
    THROW("%s", "Expected LLM argument to be a map");
  }

  return LLMDataVariable::load_async(arguments[0]->get_map(), _commandCenter);
#else   // GENAI
  THROW("%s", "Add GENAI flag to call load_llm");
#endif  // GENAI
}

OpReturnType NimbleNetDataVariable::get_current_time(const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::GET_TIME);
  auto time = Time::get_time();
  return OpReturnType(new SingleVariable<int64_t>(time));
}

OpReturnType NimbleNetDataVariable::get_config(const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::GET_CONFIG);

  auto configJson = nlohmann::json(*_commandCenter->get_config());
  return DataVariable::get_map_from_json_object(std::move(configJson));
}

OpReturnType NimbleNetDataVariable::get_exp(const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::EXP);
  if (arguments[0]->get_containerType() != CONTAINERTYPE::SINGLE) {
    THROW("exp function expects a single variable. Given %s type.",
          arguments[0]->get_containerType_string());
  }
  if (!arguments[0]->is_numeric()) {
    THROW("exp function expects a numeric value. Given %s type.",
          util::get_string_from_enum(arguments[0]->get_dataType_enum()));
  }

  return OpReturnType(new SingleVariable<double>(exp(arguments[0]->get_double())));
}

OpReturnType NimbleNetDataVariable::get_pow(const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 2, MemberFuncType::POW);
  if (arguments[0]->get_containerType() != CONTAINERTYPE::SINGLE ||
      arguments[1]->get_containerType() != CONTAINERTYPE::SINGLE) {
    THROW("pow function expects both single variables. Given %s and %s types",
          arguments[0]->get_containerType_string(), arguments[1]->get_containerType_string());
  }
  if (!arguments[0]->is_numeric() || !arguments[1]->is_numeric()) {
    THROW("pow function expects both arguments as numeric values. Given %s and %s types.",
          util::get_string_from_enum(arguments[0]->get_dataType_enum()),
          util::get_string_from_enum(arguments[1]->get_dataType_enum()));
  }
  double ret = pow(arguments[0]->get_double(), arguments[1]->get_double());
  return OpReturnType(new SingleVariable<double>(ret));
}

OpReturnType NimbleNetDataVariable::get_raw_events_store(
    const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 3, MemberFuncType::GET_RAW_EVENTS_STORE);
  std::string tableName = arguments[0]->get_string();
  std::string expiryType = arguments[1]->get_string();
  int expiryValue = arguments[2]->get_int32();
  return OpReturnType(
      new RawEventStoreDataVariable(_commandCenter, tableName, expiryType, expiryValue));
}

OpReturnType NimbleNetDataVariable::get_dataframe(const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::GET_DATAFRAME);
  auto schema = arguments[0]->get_map();
  return OpReturnType(new DataframeVariable(_commandCenter, schema));
}

OpReturnType NimbleNetDataVariable::min(const std::vector<OpReturnType>& args) {
  THROW_ARGUMENTS_NOT_MATCH(args.size(), 1, MemberFuncType::MIN);

  auto findMin = [tensor = args[0]](auto typeObj) -> OpReturnType {
    using T = decltype(typeObj);
    auto typedTensor = std::dynamic_pointer_cast<BaseTypedTensorVariable>(tensor);
    if (!typedTensor) {
      THROW("min expected a tensor, got %s", tensor->get_containerType_string());
    }

    const auto resultIt = std::min_element(typedTensor->begin<T>(), typedTensor->end<T>());
    if (resultIt == typedTensor->end<T>()) {
      THROW("%s", "Expected a non-empty tensor");
    }
    auto result = *resultIt;
    return std::make_shared<SingleVariable<decltype(result)>>(result);
  };

  return util::call_function_for_dataType(findMin,
                                          static_cast<DATATYPE>(args[0]->get_dataType_enum()));
}

OpReturnType NimbleNetDataVariable::max(const std::vector<OpReturnType>& args) {
  THROW_ARGUMENTS_NOT_MATCH(args.size(), 1, MemberFuncType::MAX);

  auto findMax = [tensor = args[0]](auto typeObj) -> OpReturnType {
    using T = decltype(typeObj);
    auto typedTensor = std::dynamic_pointer_cast<BaseTypedTensorVariable>(tensor);
    if (!typedTensor) {
      THROW("max expected a tensor, got %s", tensor->get_containerType_string());
    }

    const auto resultIt = std::max_element(typedTensor->begin<T>(), typedTensor->end<T>());
    if (resultIt == typedTensor->end<T>()) {
      THROW("%s", "Expected a non-empty tensor");
    }
    auto result = *resultIt;
    return std::make_shared<SingleVariable<decltype(result)>>(result);
  };

  return util::call_function_for_dataType(findMax,
                                          static_cast<DATATYPE>(args[0]->get_dataType_enum()));
}

OpReturnType NimbleNetDataVariable::sum(const std::vector<OpReturnType>& args) {
  THROW_ARGUMENTS_NOT_MATCH(args.size(), 1, MemberFuncType::SUM);

  auto findSum = [tensor = args[0]](auto typeObj) -> OpReturnType {
    using ElementType = decltype(typeObj);

    if constexpr (!std::is_integral_v<ElementType> && !std::is_floating_point_v<ElementType>) {
      THROW("%s", "sum only supports integral and floating point tensors");
    }

    auto typedTensor = std::dynamic_pointer_cast<BaseTypedTensorVariable>(tensor);
    if (!typedTensor) {
      THROW("sum expected a tensor, got %s", tensor->get_containerType_string());
    }

    auto result = std::accumulate(typedTensor->begin<ElementType>(),
                                  typedTensor->end<ElementType>(), ElementType{0});
    return std::make_shared<SingleVariable<decltype(result)>>(result);
  };

  return util::call_function_for_dataType(findSum,
                                          static_cast<DATATYPE>(args[0]->get_dataType_enum()));
}

OpReturnType NimbleNetDataVariable::mean(const std::vector<OpReturnType>& args) {
  THROW_ARGUMENTS_NOT_MATCH(args.size(), 1, MemberFuncType::MEAN);

  auto findMean = [tensor = args[0]](auto typeObj) -> OpReturnType {
    using ElementType = decltype(typeObj);
    auto typedTensor = std::dynamic_pointer_cast<BaseTypedTensorVariable>(tensor);
    if (!typedTensor) {
      THROW("mean expected a tensor, got %s", tensor->get_containerType_string());
    }

    if constexpr (!std::is_integral_v<ElementType> && !std::is_floating_point_v<ElementType>) {
      THROW("%s", "mean only supports integral and floating point tensors");
    } else {
      auto sum = std::accumulate(typedTensor->begin<ElementType>(), typedTensor->end<ElementType>(),
                                 ElementType{0});
      double mean = static_cast<double>(sum) / typedTensor->get_numElements();

      return std::make_shared<SingleVariable<decltype(mean)>>(mean);
    }
  };

  return util::call_function_for_dataType(findMean,
                                          static_cast<DATATYPE>(args[0]->get_dataType_enum()));
}

OpReturnType NimbleNetDataVariable::log(const std::vector<OpReturnType>& args) {
  THROW_ARGUMENTS_NOT_MATCH(args.size(), 2, MemberFuncType::LOG);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(args[0]->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::LOG);
  auto type = args[0]->get_string();

  if (args[1]->get_containerType() != CONTAINERTYPE::MAP) {
    THROW("%s", "Expected second argument of log to be a map");
  }
  auto data = args[1]->to_json();

  std::shared_ptr<Job<void>> job =
      std::make_unique<LogJob>(_commandCenter->get_deployment_id(), std::move(type),
                               std::move(data), _commandCenter->get_external_logger());
  // FIXME: Assuming that LogJob will never throw
  static_cast<void>(_commandCenter->job_scheduler()->add_priority_job(std::move(job)));
  return std::make_shared<NoneVariable>();
}

OpReturnType NimbleNetDataVariable::create_retriever(const std::vector<OpReturnType>& arguments,
                                                     CallStack& stack) {
#ifdef GENAI
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::RETRIEVER);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[0]->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::RETRIEVER);
  auto name = arguments[0]->get_string();
  const auto asset = _commandCenter->get_deployment().get_module(name, AssetType::RETRIEVER);
  if (!asset) {
    THROW("Could not find Retriever of name %s", name.c_str());
  }
  return AssetLoadJob::fetch(asset, _commandCenter);
#else   // GENAI
  THROW("%s", "Add GENAI flag to build Retriever");
#endif  // GENAI
}

/*
 * 1. Get device tier
 * 2. Get the LLMs in deployment from asset manager in cloud
 * 3. Get on device LLM if supported
 * 4. Return the list of compatible LLMs.
 */
std::vector<std::map<std::string, std::string>> NimbleNetDataVariable::get_compatible_llms(
    CommandCenter* commandCenter) {
#ifdef GENAI
  llmutil::DeviceTier device_tier = llmutil::get_device_tier(commandCenter);
  std::map<std::string, std::string> device_info = llmutil::get_device_info();

  return llmutil::get_all_llms(commandCenter, device_info["deviceModel"], device_tier);
#else   // GENAI
  THROW("%s", "Add GENAI flag to list llms");
#endif  // GENAI
}

OpReturnType NimbleNetDataVariable::list_compatible_llms(
    const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::LIST_COMPATIBLE_LLMS);
  std::vector<std::map<std::string, std::string>> all_llms = get_compatible_llms(_commandCenter);
  OpReturnType list = OpReturnType(new ListDataVariable());
  for (const auto& map : all_llms) {
    OpReturnType m = OpReturnType(new MapDataVariable());
    for (const auto& keyValuePair : map) {
      m->set_value_in_map(keyValuePair.first,
                          OpReturnType(new SingleVariable<std::string>(keyValuePair.second)));
    }
    list->append(m);
  }
  return list;
}

OpReturnType NimbleNetDataVariable::create_concurrent_executor(
    const std::vector<OpReturnType>& arguments) {
#ifndef MINIMAL_BUILD
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::CREATE_CONCURRENT_EXECUTOR);
  return OpReturnType(new ConcurrentExecutorVariable());
#else   // MINIMAL_BUILD
  THROW("Creating a concurrent executor is not supported in minimal build");
#endif  // MINIMAL_BUILD
}

OpReturnType NimbleNetDataVariable::set_threads(const std::vector<OpReturnType>& arguments) {
#ifndef MINIMAL_BUILD
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::SET_THREADS);
  int numThreads = arguments[0]->get_int32();
  ConcurrentExecutorVariable::set_threadpool_threads(numThreads);
  return std::make_shared<NoneVariable>();
#else   // MINIMAL_BUILD
  THROW("Not supported in minimal build");
#endif  // MINIMAL_BUILD
}

OpReturnType NimbleNetDataVariable::call_function(int memberFuncIndex,
                                                  const std::vector<OpReturnType>& arguments,
                                                  CallStack& stack) {
  switch (memberFuncIndex) {
    case MemberFuncType::LOADMODEL:
      return load_model(arguments, stack);
    case MemberFuncType::LLM:
      return load_llm(arguments, stack);
    case MemberFuncType::CREATETENSOR:
      return create_tensor(arguments);
    case MemberFuncType::GET_TIME:
      return get_current_time(arguments);
    case MemberFuncType::GET_CONFIG:
      return get_config(arguments);
    case MemberFuncType::EXP:
      return get_exp(arguments);
    case MemberFuncType::POW:
      return get_pow(arguments);
    case MemberFuncType::GET_RAW_EVENTS_STORE:
      return get_raw_events_store(arguments);
    case MemberFuncType::GET_DATAFRAME:
      return get_dataframe(arguments);
    case MemberFuncType::CREATE_CONCURRENT_EXECUTOR:
      return create_concurrent_executor(arguments);
    case MemberFuncType::SET_THREADS:
      return set_threads(arguments);
    case MemberFuncType::TO_TENSOR: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 2, memberFuncIndex);
      return arguments[0]->to_tensor(arguments[1]);
    }
    case MemberFuncType::MIN:
      return min(arguments);
    case MemberFuncType::MAX:
      return max(arguments);
    case MemberFuncType::SUM:
      return sum(arguments);
    case MemberFuncType::MEAN:
      return mean(arguments);
    case MemberFuncType::PARSE_JSON: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, memberFuncIndex);
      nlohmann::json j = nlohmann::json::parse(arguments[0]->get_string());
      if (j.is_array()) {
        return DataVariable::get_list_from_json_array(std::move(j));
      }
      return DataVariable::get_map_from_json_object(std::move(j));
    }
    case MemberFuncType::LOG:
      return log(arguments);
    case MemberFuncType::RETRIEVER:
      return create_retriever(arguments, stack);
    case MemberFuncType::JSON_DOCUMENT:
      return create_json_document(arguments, stack);
    case MemberFuncType::LIST_COMPATIBLE_LLMS:
      return list_compatible_llms(arguments);
  }
  THROW("%s not implemented for nimblenet", DataVariable::get_member_func_string(memberFuncIndex));
}

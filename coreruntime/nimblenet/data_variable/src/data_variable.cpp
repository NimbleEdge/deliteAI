/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "data_variable.hpp"

#include <memory>

#include "frontend_data_variable.hpp"
#include "list_data_variable.hpp"
#include "map_data_variable.hpp"
#include "nimble_net_util.hpp"
#include "nlohmann/json.hpp"
#include "single_variable.hpp"
#include "tensor_data_variable.hpp"
#include "util.hpp"

std::map<std::string, int> DataVariable::_memberFuncMap = {
    {"Model", MemberFuncType::LOADMODEL},
    {"run", MemberFuncType::RUNMODEL},
    {"filter", MemberFuncType::FEATURE_FILTER},
    {"fetch", MemberFuncType::FEATURE_FETCH},
    {"reshape", MemberFuncType::RESHAPE},
    {"zeros", MemberFuncType::CREATETENSOR},
    {"shape", MemberFuncType::GETSHAPE},
    {"status", MemberFuncType::GETMODELSTATUS},
    {"processor", MemberFuncType::CREATE_PROCESSOR_INIT},
    {"rollingWindow", MemberFuncType::CREATE_ROLLINGWINDOW_PROCESSOR},
    {"groupBy", MemberFuncType::CREATE_GROUPBY_COLUMNS_PROCESSOR},
    {"add_computation", MemberFuncType::ADD_COMPUTATION_PROCESSOR},
    {"get_for_items", MemberFuncType::GET_PROCESSOR_OUTPUT},
    {"get", MemberFuncType::GET_PROCESSOR_OUTPUT_FOR_GROUP},
    {"create", MemberFuncType::CREATE_PROCESSOR},
    {"append", MemberFuncType::APPEND},
    {"tensor", MemberFuncType::TO_TENSOR},
    {"filter_all", MemberFuncType::FEATURE_FILTER_ALL},
    {"time", MemberFuncType::GET_TIME},
    {"filter_by_function", MemberFuncType::FEATURE_FILTER_FUNCTION},
    {"num_keys", MemberFuncType::NUM_KEYS},
    {"get_config", MemberFuncType::GET_CONFIG},
    {"get_chrono_time", MemberFuncType::GET_CHRONO_TIME},
    {"RawEventStore", MemberFuncType::GET_RAW_EVENTS_STORE},
    {"Dataframe", MemberFuncType::GET_DATAFRAME},
    {"exp", MemberFuncType::EXP},
    {"pow", MemberFuncType::POW},
    {"sort", MemberFuncType::SORT},
    {"argsort", MemberFuncType::ARGSORT},
    {"topk", MemberFuncType::TOPK},
    {"arrange", MemberFuncType::ARRANGE},
    {"is_integer", MemberFuncType::ISINTEGER},
    {"is_float", MemberFuncType::ISFLOAT},
    {"is_string", MemberFuncType::ISSTRING},
    {"min", MemberFuncType::MIN},
    {"max", MemberFuncType::MAX},
    {"sum", MemberFuncType::SUM},
    {"mean", MemberFuncType::MEAN},
    {"parse_json", MemberFuncType::PARSE_JSON},
    {"log", MemberFuncType::LOG},
    {"match", MemberFuncType::REGEX_MATCH},
    {"search", MemberFuncType::REGEX_SEARCH},
    {"fullmatch", MemberFuncType::REGEX_FULLMATCH},
    {"split", MemberFuncType::REGEX_SPLIT},
    {"findall", MemberFuncType::REGEX_FINDALL},
    {"finditer", MemberFuncType::REGEX_FINDITER},
    {"sub", MemberFuncType::REGEX_SUB},
    {"subn", MemberFuncType::REGEX_SUBN},
    {"group", MemberFuncType::REGEX_MATCHOBJECT_GROUP},
    {"groups", MemberFuncType::REGEX_MATCHOBJECT_GROUPS},
    {"start", MemberFuncType::REGEX_MATCHOBJECT_START},
    {"end", MemberFuncType::REGEX_MATCHOBJECT_END},
    {"span", MemberFuncType::REGEX_MATCHOBJECT_SPAN},
    {"upper", MemberFuncType::STRING_UPPER},
    {"lower", MemberFuncType::STRING_LOWER},
    {"strip", MemberFuncType::STRING_STRIP},
    {"join", MemberFuncType::STRING_JOIN},
    {"create_simulated_char_stream", MemberFuncType::CREATE_SIM_CHAR_STREAM},
    {"to_json_stream", MemberFuncType::TO_JSON_STREAM},
    {"finished", MemberFuncType::FINISHED},
    {"iterator", MemberFuncType::ITERATOR},
    {"next", MemberFuncType::NEXT},
    {"next_available", MemberFuncType::NEXT_AVAILABLE},
    {"get_blocking", MemberFuncType::GET_BLOCKING},
    {"next_blocking", MemberFuncType::NEXT_BLOCKING},
    {"wait_for_completion", MemberFuncType::WAIT_FOR_COMPLETION},
    {"get_blocking_str", MemberFuncType::GET_BLOCKING_STR},
    {"llm", MemberFuncType::LLM},
    {"prompt", MemberFuncType::PROMPT},
    {"skip_text_and_get_json_stream", MemberFuncType::SKIP_TEXT_AND_GET_JSON_STREAM},
    {"Retriever", MemberFuncType::RETRIEVER},
    {"pop", MemberFuncType::POP},
    {"keys", MemberFuncType::KEYS},
    {"JsonDocument", MemberFuncType::JSON_DOCUMENT},
    {"max_input_num_tokens", MemberFuncType::MAX_INPUT_NUM_TOKENS},
    {"__init__", MemberFuncType::CONSTRUCTOR},
    {"unicode", MemberFuncType::UNICODE},
    {"sync", MemberFuncType::SYNC},
    {"run_parallel", MemberFuncType::RUNPARALLEL},
    {"ConcurrentExecutor", MemberFuncType::CREATE_CONCURRENT_EXECUTOR},
    {"set_threadpool_threads", MemberFuncType::SET_THREADS},
    {"cancel", MemberFuncType::CANCEL},
    {"clear_context", MemberFuncType::CLEAR_CONTEXT},
    {"add_context", MemberFuncType::ADD_CONTEXT},
    {"list_compatible_llms", MemberFuncType::LIST_COMPATIBLE_LLMS},
#ifdef IOS
    {"convert_text_to_phonemes", MemberFuncType::CONVERT_TEXT_TO_PHONEMES},
#endif  // IOS
};

std::map<int, std::string> DataVariable::_inverseMemberFuncMap = {
    {MemberFuncType::LOADMODEL, "Model"},
    {MemberFuncType::RUNMODEL, "run"},
    {MemberFuncType::RESHAPE, "reshape"},
    {MemberFuncType::CREATETENSOR, "zeros"},
    {MemberFuncType::GETSHAPE, "shape"},
    {MemberFuncType::FEATURE_FILTER, "filter"},
    {MemberFuncType::FEATURE_FETCH, "fetch"},
    {MemberFuncType::GETMODELSTATUS, "status"},
    {MemberFuncType::CREATE_PROCESSOR_INIT, "processor"},
    {MemberFuncType::CREATE_ROLLINGWINDOW_PROCESSOR, "rollingWindow"},
    {MemberFuncType::CREATE_GROUPBY_COLUMNS_PROCESSOR, "groupBy"},
    {MemberFuncType::ADD_COMPUTATION_PROCESSOR, "add_computation"},
    {MemberFuncType::GET_PROCESSOR_OUTPUT, "get_for_items"},
    {MemberFuncType::GET_PROCESSOR_OUTPUT_FOR_GROUP, "get"},
    {MemberFuncType::CREATE_PROCESSOR, "create"},
    {MemberFuncType::APPEND, "append"},
    {MemberFuncType::TO_TENSOR, "tensor"},
    {MemberFuncType::FEATURE_FILTER_ALL, "filter_all"},
    {MemberFuncType::GET_TIME, "time"},
    {MemberFuncType::FEATURE_FILTER_FUNCTION, "filter_by_function"},
    {MemberFuncType::NUM_KEYS, "num_keys"},
    {MemberFuncType::GET_CONFIG, "get_config"},
    {MemberFuncType::GET_CHRONO_TIME, "get_chrono_time"},
    {MemberFuncType::GET_RAW_EVENTS_STORE, "RawEventStore"},
    {MemberFuncType::GET_DATAFRAME, "Dataframe"},
    {MemberFuncType::EXP, "exp"},
    {MemberFuncType::POW, "pow"},
    {MemberFuncType::SORT, "sort"},
    {MemberFuncType::ARGSORT, "argsort"},
    {MemberFuncType::TOPK, "topk"},
    {MemberFuncType::ARRANGE, "arrange"},
    {MemberFuncType::ISINTEGER, "is_integer"},
    {MemberFuncType::ISFLOAT, "is_float"},
    {MemberFuncType::ISSTRING, "is_string"},
    {MemberFuncType::MIN, "min"},
    {MemberFuncType::MAX, "max"},
    {MemberFuncType::SUM, "sum"},
    {MemberFuncType::MEAN, "mean"},
    {MemberFuncType::PARSE_JSON, "parse_json"},
    {MemberFuncType::LOG, "log"},
    {MemberFuncType::REGEX_MATCH, "match"},
    {MemberFuncType::REGEX_SEARCH, "search"},
    {MemberFuncType::REGEX_FULLMATCH, "fullmatch"},
    {MemberFuncType::REGEX_SPLIT, "split"},
    {MemberFuncType::REGEX_FINDALL, "findall"},
    {MemberFuncType::REGEX_FINDITER, "finditer"},
    {MemberFuncType::REGEX_SUB, "sub"},
    {MemberFuncType::REGEX_SUBN, "subn"},
    {MemberFuncType::REGEX_MATCHOBJECT_GROUP, "group"},
    {MemberFuncType::REGEX_MATCHOBJECT_GROUPS, "groups"},
    {MemberFuncType::REGEX_MATCHOBJECT_START, "start"},
    {MemberFuncType::REGEX_MATCHOBJECT_END, "end"},
    {MemberFuncType::REGEX_MATCHOBJECT_SPAN, "span"},
    {MemberFuncType::STRING_UPPER, "upper"},
    {MemberFuncType::STRING_LOWER, "lower"},
    {MemberFuncType::STRING_STRIP, "strip"},
    {MemberFuncType::STRING_JOIN, "join"},
    {MemberFuncType::CREATE_SIM_CHAR_STREAM, "create_simulated_char_stream"},
    {MemberFuncType::TO_JSON_STREAM, "to_json_stream"},
    {MemberFuncType::FINISHED, "finished"},
    {MemberFuncType::ITERATOR, "iterator"},
    {MemberFuncType::NEXT, "next"},
    {MemberFuncType::NEXT_AVAILABLE, "next_available"},
    {MemberFuncType::GET_BLOCKING, "get_blocking"},
    {MemberFuncType::NEXT_BLOCKING, "next_blocking"},
    {MemberFuncType::WAIT_FOR_COMPLETION, "wait_for_completion"},
    {MemberFuncType::GET_BLOCKING_STR, "get_blocking_str"},
    {MemberFuncType::LLM, "llm"},
    {MemberFuncType::PROMPT, "prompt"},
    {MemberFuncType::SKIP_TEXT_AND_GET_JSON_STREAM, "skip_text_and_get_json_stream"},
    {MemberFuncType::RETRIEVER, "Retriever"},
    {MemberFuncType::POP, "pop"},
    {MemberFuncType::KEYS, "keys"},
    {MemberFuncType::JSON_DOCUMENT, "jsonDocument"},
    {MemberFuncType::MAX_INPUT_NUM_TOKENS, "max_input_num_tokens"},
    {MemberFuncType::CONSTRUCTOR, "__init__"},
    {MemberFuncType::UNICODE, "unicode"},
    {MemberFuncType::SYNC, "sync"},
    {MemberFuncType::RUNPARALLEL, "run_parallel"},
    {MemberFuncType::CREATE_CONCURRENT_EXECUTOR, "ConcurrentExecutor"},
    {MemberFuncType::SET_THREADS, "set_threadpool_threads"},
    {MemberFuncType::CANCEL, "cancel"},
    {MemberFuncType::CLEAR_CONTEXT, "clear_context"},
    {MemberFuncType::ADD_CONTEXT, "add_context"},
    {MemberFuncType::LIST_COMPATIBLE_LLMS, "list_compatible_llms"},
#ifdef IOS
    {MemberFuncType::CONVERT_TEXT_TO_PHONEMES, "convert_text_to_phonemes"},
#endif  // IOS
};

int DataVariable::add_and_get_member_func_index(const std::string& memberFuncString) {
  if (_memberFuncMap.find(memberFuncString) != _memberFuncMap.end()) {
    return _memberFuncMap[memberFuncString];
  }
  int newIndex = _memberFuncMap.size();
  _memberFuncMap[memberFuncString] = newIndex;
  _inverseMemberFuncMap[newIndex] = memberFuncString;
  return newIndex;
}

int DataVariable::get_member_func_index(const std::string& memberFuncString) {
  if (_memberFuncMap.find(memberFuncString) != _memberFuncMap.end()) {
    return _memberFuncMap[memberFuncString];
  }
  return -1;
}

const char* DataVariable::get_member_func_string(int funcIndex) {
  if (_inverseMemberFuncMap.find(funcIndex) != _inverseMemberFuncMap.end()) {
    return _inverseMemberFuncMap[funcIndex].c_str();
  }
  return "";
}

OpReturnType DataVariable::get_SingleVariableFrom_JSON(const nlohmann::json& value) {
  switch (value.type()) {
    case nlohmann::detail::value_t::number_integer:
    case nlohmann::detail::value_t::number_unsigned:
      return std::make_shared<SingleVariable<int64_t>>(value.get<int64_t>());
    case nlohmann::detail::value_t::number_float:
      return std::make_shared<SingleVariable<double>>(value.get<double>());
    case nlohmann::detail::value_t::string:
      return std::make_shared<SingleVariable<std::string>>(value.get<std::string>());
    case nlohmann::detail::value_t::null:
      return std::make_shared<NoneVariable>();
    case nlohmann::detail::value_t::boolean:
      return std::make_shared<SingleVariable<bool>>(value.get<bool>());
    case nlohmann::detail::value_t::object:
      return OpReturnType(
          DataVariable::get_map_from_json_object(std::move(value.get<nlohmann::json>())));
    case nlohmann::detail::value_t::array: {
      return OpReturnType(
          DataVariable::get_list_from_json_array(std::move(value.get<nlohmann::json>())));
    }
    default:
      THROW("Constant value of type=%s not supported", value.type_name());
  }
}

OpReturnType DataVariable::get_list_from_json_array(nlohmann::json&& j) {
  if (!j.is_array()) {
    THROW("%s", "Trying to create json array from non array.");
  }
  OpReturnType list = OpReturnType(new ListDataVariable());
  for (auto it = j.begin(); it != j.end(); it++) {
    switch (it->type()) {
      case nlohmann::detail::value_t::number_float:
        list->append(OpReturnType(new SingleVariable<double>(it.value())));
        break;
      case nlohmann::detail::value_t::number_unsigned:
      case nlohmann::detail::value_t::number_integer:
        list->append(OpReturnType(new SingleVariable<int64_t>(it.value())));
        break;
      case nlohmann::detail::value_t::string:
        list->append(OpReturnType(new SingleVariable<std::string>(it.value())));
        break;
      case nlohmann::detail::value_t::array:
        list->append(DataVariable::get_list_from_json_array(std::move(it.value())));
        break;
      case nlohmann::detail::value_t::object:
        list->append(DataVariable::get_map_from_json_object(std::move(it.value())));
        break;
      case nlohmann::detail::value_t::boolean:
        list->append(OpReturnType(new SingleVariable<bool>(it.value())));
        break;
      case nlohmann::detail::value_t::null:
        list->append(OpReturnType(new NoneVariable()));
        break;
      default:
        THROW("%s", "Datatype not supported as a item in json array");
    }
  }
  return list;
}

OpReturnType DataVariable::get_map_from_json_object(nlohmann::json&& j) {
  if (!j.is_object()) {
    THROW("%s", "Trying to create a json object from non object.");
  }
  OpReturnType map = OpReturnType(new MapDataVariable());
  for (auto it = j.begin(); it != j.end(); it++) {
    switch (it->type()) {
      case nlohmann::detail::value_t::number_float:
        map->set_value_in_map(it.key(), OpReturnType(new SingleVariable<double>(it.value())));
        break;
      case nlohmann::detail::value_t::number_unsigned:
      case nlohmann::detail::value_t::number_integer:
        map->set_value_in_map(it.key(), OpReturnType(new SingleVariable<int64_t>(it.value())));
        break;
      case nlohmann::detail::value_t::string:
        map->set_value_in_map(it.key(), OpReturnType(new SingleVariable<std::string>(it.value())));
        break;
      case nlohmann::detail::value_t::array:
        map->set_value_in_map(it.key(),
                              DataVariable::get_list_from_json_array(std::move(it.value())));
        break;
      case nlohmann::detail::value_t::object:
        map->set_value_in_map(it.key(),
                              DataVariable::get_map_from_json_object(std::move(it.value())));
        break;
      case nlohmann::detail::value_t::boolean:
        map->set_value_in_map(it.key(), OpReturnType(new SingleVariable<bool>(it.value())));
        break;
      case nlohmann::detail::value_t::null:
        map->set_value_in_map(it.key(), OpReturnType(new NoneVariable()));
        break;
      default:
        THROW("%s", "Datatype not supported as a item in json array");
    }
  }
  return map;
}

const char* DataVariable::get_containerType_string() const {
  switch (get_containerType()) {
    case CONTAINERTYPE::SINGLE:
      return "Scalar";
    case CONTAINERTYPE::VECTOR:
      return "Tensor";
    case CONTAINERTYPE::TUPLE:
      return "Tuple";
    case CONTAINERTYPE::MAP:
      return "Map";
    case CONTAINERTYPE::SLICE:
      return "Slice";
    case CONTAINERTYPE::RANGE:
      return "Range";
    case CONTAINERTYPE::LIST:
      return "List";
    case CONTAINERTYPE::FUNCTIONDEF:
      return "Function";
    case CONTAINERTYPE::CLASS:
      return "Class";
  }
  return "UNKNOWN";
}

CTensor DataVariable::to_cTensor(char* name, void* rawPtr) {
  CTensor cTensor;
  cTensor.name = const_cast<char*>(name);
  int dataType = get_dataType_enum();

  switch (get_containerType()) {
    case CONTAINERTYPE::MAP:
      cTensor.dataType = DATATYPE::JSON;
      cTensor.shape = nullptr;
      cTensor.shapeLength = 0;
      cTensor.data = rawPtr;
      break;
    case CONTAINERTYPE::LIST:
      cTensor.dataType = DATATYPE::JSON_ARRAY;
      cTensor.shape = const_cast<int64_t*>(get_shape().data());
      cTensor.shapeLength = get_shape().size();
      cTensor.data = rawPtr;
      break;
    case CONTAINERTYPE::SINGLE:
      cTensor.dataType = dataType;
      cTensor.shape = nullptr;
      cTensor.shapeLength = 0;
      if (dataType == DATATYPE::STRING) {
        cTensor.data = static_cast<void*>(get_string_ptr());
      } else {
        cTensor.data = get_raw_ptr();
      }
      break;
    case CONTAINERTYPE::VECTOR:
      // Tensor container
      cTensor.dataType = dataType;
      cTensor.shape = const_cast<int64_t*>(get_shape().data());
      cTensor.shapeLength = get_shape().size();
      if (dataType == DATATYPE::STRING) {
        cTensor.data = static_cast<void*>(get_string_ptr());
      } else {
        cTensor.data = get_raw_ptr();
      }
      break;
    default:
      THROW("Cannot convert container of type %s to cTensor", get_containerType_string());
  }

  return cTensor;
}

OpReturnType DataVariable::call_function(int memberFuncIndex,
                                         const std::vector<OpReturnType>& arguments,
                                         CallStack& stack) {
  switch (memberFuncIndex) {
    case MemberFuncType::GETSHAPE: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, memberFuncIndex);
      auto s = get_shape();
      return std::make_shared<ListDataVariable>(s);
    }
    case MemberFuncType::RESHAPE: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, memberFuncIndex);
      auto val = arguments[0];
      std::vector<int64_t> shape;
      int dimensions = val->get_size();
      for (int i = 0; i < dimensions; i++) {
        shape.push_back(val->get_int_subscript(i)->get_int64());
      }

      bool status = reshape(shape);
      if (!status) {
        THROW("%s", "reshape failed size does not match");
      }
      return shared_from_this();
    }
    case MemberFuncType::APPEND: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, memberFuncIndex);
      return append(arguments[0]);
    }
    case MemberFuncType::SORT: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, memberFuncIndex);
      return sort(arguments[0]);
    }
    case MemberFuncType::ARGSORT: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, memberFuncIndex);
      return argsort(arguments[0]);
    }
    case MemberFuncType::TOPK: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 2, memberFuncIndex);
      return topk(arguments);
    }
    case MemberFuncType::ARRANGE: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, memberFuncIndex);
      return arrange(arguments[0]);
    }
    case MemberFuncType::ISINTEGER: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, memberFuncIndex);
      return OpReturnType(new SingleVariable<bool>(is_integer()));
    }
    case MemberFuncType::ISFLOAT: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, memberFuncIndex);
      return OpReturnType(new SingleVariable<bool>(is_numeric() && !is_integer()));
    }
    case MemberFuncType::ISSTRING: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, memberFuncIndex);
      return OpReturnType(new SingleVariable<bool>(is_string()));
    }
  }
  THROW("%s not supported for variable %s(%s)", get_member_func_string(memberFuncIndex),
        get_containerType_string(), util::get_string_from_enum(get_dataType_enum()));
}

OpReturnType DataVariable::create_tensor(const CTensor& c, CreateTensorType type) {
  std::vector<int64_t> shape(c.shape, c.shape + c.shapeLength);
  switch (c.dataType) {
    case DATATYPE::FLOAT:
    case DATATYPE::DOUBLE:
    case DATATYPE::INT32:
    case DATATYPE::INT64:
    case DATATYPE::BOOLEAN:
      return OpReturnType(
          new TensorVariable(c.data, static_cast<DATATYPE>(c.dataType), shape, type));
    case DATATYPE::JSON_ARRAY:
      // Comes as ListDataVariable from outside
      return *(static_cast<OpReturnType*>(c.data));
    case DATATYPE::STRING:
      return OpReturnType(new StringTensorVariable(c.data, c.shape, c.shapeLength));
    case DATATYPE::FE_OBJ:
      return FrontendDataVariable::create(c.data);
    default:
      THROW("%s datatype is not supported for a tensor variable in input to script.",
            util::get_string_from_enum(c.dataType));
  }
}

OpReturnType DataVariable::create_single_variable(const CTensor& c) {
  switch (c.dataType) {
    case DATATYPE::FLOAT:
      return OpReturnType(new SingleVariable<float>(c.data));
    case DATATYPE::DOUBLE:
      return OpReturnType(new SingleVariable<double>(c.data));
    case DATATYPE::INT32:
      return OpReturnType(new SingleVariable<int32_t>(c.data));
    case DATATYPE::INT64:
      return OpReturnType(new SingleVariable<int64_t>(c.data));
    case DATATYPE::BOOLEAN:
      return OpReturnType(new SingleVariable<bool>(c.data));
    case DATATYPE::STRING:
      return OpReturnType(new SingleVariable<std::string>(((char**)c.data)[0]));
    case DATATYPE::JSON:
    case DATATYPE::FUNCTION:
      return *(static_cast<OpReturnType*>(c.data));
    case DATATYPE::FE_OBJ:
      return FrontendDataVariable::create(c.data);
    case DATATYPE::NONE:
      return OpReturnType(new NoneVariable{});
    default:
      THROW("%s datatype is not supported for a single variable in input to script.",
            util::get_string_from_enum(c.dataType));
  }
}

OpReturnType DataVariable::create_tensor(int dType, const std::vector<int64_t>& shape) {
  if (shape.size() == 1 && shape[0] == 0) {
    return OpReturnType(new EmptyTensorVariable(dType));
  }

  switch (dType) {
    case DATATYPE::FLOAT:
    case DATATYPE::DOUBLE:
    case DATATYPE::INT32:
    case DATATYPE::INT64:
    case DATATYPE::BOOLEAN:
    // TODO: Might have to change these as well
    case DATATYPE::JSON:
      return OpReturnType(new TensorVariable(shape, static_cast<DATATYPE>(dType)));
    case DATATYPE::STRING:
      return OpReturnType(new StringTensorVariable(shape));
  }
  THROW("cannot create tensor with dType=%s", util::get_string_from_enum(dType));
}

int ListSliceVariable::get_start(int size) const {
  int start = 0;
  int step = get_step();
  // Get start value if specified
  if (_start->get_bool()) {
    start = _start->get_int32();
    if (start < 0) start += size;
    // Clamp start based on step direction
    if (step > 0) {
      if (start < 0) start = 0;
      // For positive step, start must be < size
      if (start >= size) start = size;
    } else {
      // For negative step, start can be size-1 at maximum
      if (start < 0) start = 0;
      if (start >= size) start = size - 1;
    }
  } else {
    // Default start depends on step direction
    if (_start->is_none()) {
      start = (step > 0) ? 0 : size - 1;
    } else {
      start = _start->get_int32();
    }
  }
  return start;
}

int ListSliceVariable::get_stop(int size) const {
  int stop = size;
  int step = get_step();
  // Get stop value if specified
  if (_stop->get_bool()) {
    stop = _stop->get_int32();
    if (stop < 0) stop += size;
    // Clamp stop based on step direction
    if (step > 0) {
      if (stop < 0) stop = 0;
      // For positive step, stop can be at most size
      if (stop > size) stop = size;
    } else {
      // For negative step, stop can be at minimum -1
      // (to include 0 when iterating)
      if (stop < -1) stop = -1;
      if (stop >= size) stop = size - 1;
    }
  } else {
    // Default stop depends on step direction
    if (_stop->is_none()) {
      stop = (step > 0) ? size : -1;
    } else {
      stop = _stop->get_int32();
    }
  }
  return stop;
}

int ListSliceVariable::get_step() const {
  if (_step->get_bool()) {
    int step = _step->get_int32();
    if (step == 0) {
      THROW("slice step cannot be zero");
    }
    return step;
  }
  return 1;
}

std::string ListSliceVariable::print() {
  std::string result = "slice(";
  result += _start->print() + ", ";
  result += _stop->print() + ", ";
  result += _step->print() + ")";
  return result;
}

nlohmann::json ListSliceVariable::to_json() const {
  auto output = nlohmann::json::object();
  output["start"] = _start->to_json();
  output["stop"] = _stop->to_json();
  output["step"] = _step->to_json();
  return output;
}

/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "pre_processor_nimble_net_variable.hpp"

#include "command_center.hpp"
#include "data_variable.hpp"
#include "nimble_net_util.hpp"
#include "tuple_data_variable.hpp"
#include "user_events_manager.hpp"

PreProcessorNimbleNetVariable::PreProcessorNimbleNetVariable(CommandCenter* commandCenter,
                                                             std::shared_ptr<TableStore> tableStore,
                                                             const DATATYPE dataType) {
  _commandCenter = commandCenter;
  _dataType = dataType;
  _tableStore = tableStore;
  _rollingWindow = OpReturnType(new NoneVariable());
  _groupByColumns = OpReturnType(new NoneVariable());
  _computations = std::vector<OpReturnType>();
}

OpReturnType PreProcessorNimbleNetVariable::add_rolling_window(
    const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::CREATE_ROLLINGWINDOW_PROCESSOR);
  if (_isPreProcessorCreated) {
    THROW("%s", "Cannot add rolling window after the preProcessor is created.");
  }
  if (arguments[0]->get_containerType() != CONTAINERTYPE::VECTOR &&
      arguments[0]->get_containerType() != CONTAINERTYPE::LIST &&
      arguments[0]->get_containerType() != CONTAINERTYPE::TUPLE) {
    THROW("rollingWindow expects tensor/list/tuple argument. Given %s type.",
          arguments[0]->get_containerType_string());
  }
  if (!arguments[0]->get_int_subscript(0)->is_numeric()) {
    THROW("rollingWindow should have numeric values. Given %s type.",
          util::get_string_from_enum(arguments[0]->get_dataType_enum()));
  }
  _rollingWindow = arguments[0];
  return shared_from_this();
}

OpReturnType PreProcessorNimbleNetVariable::add_groupBy_columns(
    const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::CREATE_GROUPBY_COLUMNS_PROCESSOR);
  if (_isPreProcessorCreated) {
    THROW("%s", "Cannot add groupBy columns after the preProcessor is created.");
  }

  if (arguments[0]->get_containerType() != CONTAINERTYPE::VECTOR &&
      arguments[0]->get_containerType() != CONTAINERTYPE::LIST &&
      arguments[0]->get_containerType() != CONTAINERTYPE::TUPLE) {
    THROW("groupBy expects tensor/list/tuple argument. Given %s type.",
          arguments[0]->get_containerType_string());
  }
  _groupByColumns = arguments[0];
  return shared_from_this();
}

OpReturnType PreProcessorNimbleNetVariable::add_computation(
    const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 3, MemberFuncType::ADD_COMPUTATION_PROCESSOR);
  if (_isPreProcessorCreated) {
    THROW("%s", "Cannot add computation after the preProcessor is created.");
  }

  if (arguments[0]->get_containerType() != CONTAINERTYPE::SINGLE) {
    THROW("add_compuation's first argument should be a single variable. Given %s",
          arguments[0]->get_containerType());
  }

  if (arguments[0]->get_dataType_enum() != DATATYPE::STRING) {
    THROW("add_compuation's first argument should be a string. Given %s",
          util::get_string_from_enum(arguments[0]->get_dataType_enum()));
  }

  // TODO: Second argument can be taken as an enum (Operator enum)
  if (arguments[1]->get_containerType() != CONTAINERTYPE::SINGLE) {
    THROW("add_compuation's second argument should be a single variable. Given %s",
          util::get_string_from_enum(arguments[1]->get_containerType()));
  }

  if (arguments[1]->get_dataType_enum() != DATATYPE::STRING) {
    THROW("add_compuation's second argument should be a string. Given %s",
          util::get_string_from_enum(arguments[1]->get_dataType_enum()));
  }

  if (arguments[2]->get_containerType() != CONTAINERTYPE::SINGLE) {
    THROW("add_compuation's third argument should be a single variable. Given %s",
          arguments[2]->get_containerType());
  }

  if (!arguments[2]->is_numeric()) {
    THROW("add_computation's third argument should have a numeric value. Given %s type.",
          util::get_string_from_enum(arguments[2]->get_dataType_enum()));
  }
  if (_computations.size() == 1) {
    THROW("%s", "cannot add more that 1 computation");
  }
  _computations.push_back(OpReturnType(new TupleDataVariable(arguments)));

  return shared_from_this();
}

OpReturnType PreProcessorNimbleNetVariable::create_processor(
    const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::CREATE_PROCESSOR);
  if (_computations.size() != 1) {
    THROW("%s", "cannot create processor without adding computations");
  }
  switch (static_cast<int>(_dataType)) {
    case DATATYPE::FLOAT:
    case DATATYPE::INT32:
    case DATATYPE::INT64:
    case DATATYPE::DOUBLE:
      std::vector<double> defaultVector;
      for (int i = 0; i < _computations.size(); i++) {
        defaultVector.push_back(_computations[i]->get_int_subscript(2)->get_double());
      }
      _processor = create_processor(defaultVector, _dataType);
      break;
  }
  if (!_processor) {
    THROW("%s", "Could not create pre-processor");
  }
  _isPreProcessorCreated = true;

  return shared_from_this();
}

OpReturnType PreProcessorNimbleNetVariable::get_processor_output(
    const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::GET_PROCESSOR_OUTPUT);
  if (!_isPreProcessorCreated) {
    THROW("%s", "Cannot get preProcessor result before it is created.");
  }
  if (arguments[0]->get_containerType() != CONTAINERTYPE::LIST &&
      arguments[0]->get_containerType() != CONTAINERTYPE::VECTOR) {
    THROW("get's first argument should be a tensor variable. Given %s",
          arguments[0]->get_containerType_string());
  }

  std::vector<int64_t> newShape = arguments[0]->get_shape();
  newShape.push_back(_rollingWindow->get_size());

  // Converting to json will be slow and is in the hot path, so this API should be discouraged
  OpReturnType preProcessorInputToModel =
      _processor->get_model_input_data_variable(arguments[0]->to_json());
  if (preProcessorInputToModel == nullptr) {
    THROW("%s", "Failed to get preprocessorOutput");
  }
  preProcessorInputToModel->reshape(newShape);

  return preProcessorInputToModel;
}

OpReturnType PreProcessorNimbleNetVariable::get_processor_output_by_group(
    const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::GET_PROCESSOR_OUTPUT_FOR_GROUP);
  if (!_isPreProcessorCreated) {
    THROW("%s", "Cannot get preProcessor result before it is created.");
  }
  auto groups = arguments[0];

  // check if argument is a single group
  {
    int size = groups->get_size();
    if ((size == 0) || (size > 0 && groups->get_int_subscript(0)->is_single())) {
      // single group is given as argument
      std::vector<std::string> g;
      for (int i = 0; i < size; i++) {
        auto groupVal = groups->get_int_subscript(i);
        if (!groupVal->is_single()) {
          THROW("cannot get_group from variable of container=%s",
                groupVal->get_containerType_string());
        }
        if (groupVal->get_dataType_enum() == DATATYPE::STRING)
          g.push_back(groupVal->get_string());
        else if (groupVal->is_numeric()) {
          g.push_back(groupVal->print());
        } else {
          THROW("Group should be numeric or string found=%s",
                util::get_string_from_enum(groupVal->get_dataType_enum()));
        }
      }
      std::vector<std::vector<std::string>> allGroups = {g};
      OpReturnType modelInput = _processor->get_model_input_data_variable(allGroups);
      if (modelInput == nullptr) {
        THROW("%s", "Failed to get preprocessorOutput");
      }
      std::vector<int64_t> newShape = {_rollingWindow->get_size()};
      modelInput->reshape(newShape);
      return modelInput;
    }
  }
  // this assumes argument is a list of groups
  int size = groups->get_size();
  std::vector<std::vector<std::string>> allGroups(size);
  for (int i = 0; i < size; i++) {
    auto groupContainer = groups->get_int_subscript(i);
    std::vector<std::string> g;
    for (int j = 0; j < groupContainer->get_size(); j++) {
      auto groupVal = groupContainer->get_int_subscript(j);
      if (!groupVal->is_single()) {
        THROW("cannot get_group from variable of container=%s",
              groupVal->get_containerType_string());
      }
      if (groupVal->get_dataType_enum() == DATATYPE::STRING)
        g.push_back(groupVal->get_string());
      else if (groupVal->is_numeric()) {
        g.push_back(groupVal->print());
      } else {
        THROW("Group should be numeric or string found=%s",
              util::get_string_from_enum(groupVal->get_dataType_enum()));
      }
    }
    allGroups[i] = std::move(g);
  }
  OpReturnType modelInput = _processor->get_model_input_data_variable(allGroups);
  if (modelInput == nullptr) {
    THROW("%s", "Failed to get preprocessorOutput");
  }
  std::vector<int64_t> newShape = {size, _rollingWindow->get_size()};
  modelInput->reshape(newShape);
  return modelInput;
}

OpReturnType PreProcessorNimbleNetVariable::call_function(
    int memberFuncIndex, const std::vector<OpReturnType>& arguments, CallStack& stack) {
  switch (memberFuncIndex) {
    case MemberFuncType::CREATE_ROLLINGWINDOW_PROCESSOR:
      return add_rolling_window(arguments);
    case MemberFuncType::CREATE_GROUPBY_COLUMNS_PROCESSOR:
      return add_groupBy_columns(arguments);
    case MemberFuncType::ADD_COMPUTATION_PROCESSOR:
      return add_computation(arguments);
    case MemberFuncType::GET_PROCESSOR_OUTPUT:
      return get_processor_output(arguments);
    case MemberFuncType::GET_PROCESSOR_OUTPUT_FOR_GROUP:
      return get_processor_output_by_group(arguments);
    case MemberFuncType::CREATE_PROCESSOR:
      return create_processor(arguments);
  }
  THROW("%s not implemented for nimblenet", DataVariable::get_member_func_string(memberFuncIndex));
}

std::string PreProcessorNimbleNetVariable::print() {
  std::string output = "Processor(";

  output += "dataType: " + std::string(util::get_string_from_enum(_dataType)) + ", ";
  output += "rollingWindow : " + _rollingWindow->print() + ", ";
  output += "groupBy : " + _groupByColumns->print() + ", ";
  output += "computations: [";
  for (int i = 0; i < _computations.size() - 1; i++) {
    output += _computations[i]->print() + ", ";
  }
  output += _computations[_computations.size() - 1]->print() + "]";
  output += ")";
  return output;
}

BasePreProcessor* PreProcessorNimbleNetVariable::create_processor(std::vector<double> defaultVector,
                                                                  DATATYPE dataType) {
  std::vector<std::string> columnsToAggregate;
  std::vector<std::string> aggregateOperators;
  for (int i = 0; i < _computations.size(); i++) {
    columnsToAggregate.push_back(_computations[i]->get_int_subscript(0)->get_string());
    aggregateOperators.push_back(_computations[i]->get_int_subscript(1)->get_string());
  }
  PreProcessorInfo preProcessorInfo;
  if (_rollingWindow->get_dataType_enum() == DATATYPE::NONE) {
    THROW("%s", "create called before rollingWindow defined on processor");
  }
  for (int i = 0; i < _rollingWindow->get_size(); i++) {
    preProcessorInfo.rollingWindowsInSecs.push_back(
        _rollingWindow->get_int_subscript(i)->get_float());
  }
  if (_groupByColumns->get_dataType_enum() == DATATYPE::NONE) {
    THROW("%s", "create called before groupBy columns defined on processor");
  }
  for (int i = 0; i < _groupByColumns->get_size(); i++) {
    preProcessorInfo.groupColumns.push_back(_groupByColumns->get_int_subscript(i)->get_string());
  }
  preProcessorInfo.valid = true;
  preProcessorInfo.columnsToAggregate = columnsToAggregate;
  preProcessorInfo.aggregateOperators = aggregateOperators;
  preProcessorInfo.defaultVector = defaultVector;
  preProcessorInfo.dataType = dataType;
  return _tableStore->create_preprocessor(preProcessorInfo);
}

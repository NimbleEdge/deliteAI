#include <memory>
#include <vector>

#include "data_variable.hpp"
#include "dataframe_variable.hpp"
#include "list_data_variable.hpp"
#include "variable_scope.hpp"

OpReturnType FilteredDataframeVariable::call_function(int memberFuncIndex,
                                                      const std::vector<OpReturnType>& arguments,
                                                      CallStack& stack) {
  switch (memberFuncIndex) {
    case MemberFuncType::FEATURE_FETCH:
      return feature_fetch(arguments);
    case MemberFuncType::FEATURE_FILTER_ALL: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::FEATURE_FILTER_ALL);
      return shared_from_this();
      // Will change this to support in place
      // modifications in future
    }
    case MemberFuncType::FEATURE_FILTER_FUNCTION:
      return filter_by_function(arguments, stack);
    case MemberFuncType::NUM_KEYS:
      return OpReturnType(new SingleVariable<int32_t>(_selectedIndices.size()));
  }
  THROW("%s not implemented for events_store",
        DataVariable::get_member_func_string(memberFuncIndex));
}

OpReturnType FilteredDataframeVariable::filter_by_function(
    const std::vector<OpReturnType>& arguments, CallStack& stack) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::FEATURE_FILTER_FUNCTION);
  if (arguments[0]->get_containerType() != CONTAINERTYPE::FUNCTIONDEF) {
    THROW("filter_by_function expects argument of type function, provided : %s",
          arguments[0]->get_containerType_string());
  }

  std::vector<int> selectedIndices;
  for (const int i : _selectedIndices) {
    auto& event = _tableData->allEvents.at(i);
    OpReturnType eventVariable =
        OpReturnType(new TableEventDataVariable(&event, &_tableData->columnToIdMap));
    OpReturnType functionReturn = arguments[0]->execute_function({eventVariable}, stack);
    if (functionReturn->get_bool()) {
      selectedIndices.push_back(i);
    }
  }

  return std::shared_ptr<FilteredDataframeVariable>(
      new FilteredDataframeVariable(_tableData, std::move(selectedIndices)));
}

OpReturnType FilteredDataframeVariable::all_events(std::shared_ptr<TableData> tableData) {
  std::vector<int> selectedIndices(tableData->allEvents.size());
  std::iota(selectedIndices.begin(), selectedIndices.end(), 0);
  return std::shared_ptr<FilteredDataframeVariable>(
      new FilteredDataframeVariable(tableData, std::move(selectedIndices)));
}

OpReturnType FilteredDataframeVariable::events_filtered_by_function(
    std::shared_ptr<TableData> tableData, OpReturnType func, CallStack& stack) {
  std::vector<int> selectedIndices;
  for (int i = 0; i < tableData->allEvents.size(); i++) {
    auto& event = tableData->allEvents.at(i);
    OpReturnType eventVariable =
        OpReturnType(new TableEventDataVariable(&event, &tableData->columnToIdMap));
    OpReturnType functionReturn = func->execute_function({eventVariable}, stack);
    if (functionReturn->get_bool()) {
      selectedIndices.push_back(i);
    }
  }
  return std::shared_ptr<FilteredDataframeVariable>(
      new FilteredDataframeVariable(tableData, std::move(selectedIndices)));
}

OpReturnType FilteredDataframeVariable::feature_fetch(const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 2, MemberFuncType::FEATURE_FETCH);
  auto type = util::get_enum_from_string(arguments[1]->get_string().c_str());
  auto key = arguments[0]->get_string();
  if (key == "TIMESTAMP" || key == "timestamp") {
    return feature_fetch_timestamp(arguments[1]);
  }
  if (_tableData->columnToIdMap.find(key) == _tableData->columnToIdMap.end()) {
    THROW("key=%s not found in dataframe", key.c_str());
  }
  if (_selectedIndices.size() == 0) {
    THROW("%s", "Either no events filtered or filtering returned 0 events");
  }
  int rowIndex = _tableData->columnToIdMap.at(key);
  if (!util::is_dType_array(type)) {
    auto outputTensor = DataVariable::create_tensor(type, {(long long)_selectedIndices.size()});
    void* tensorPtr = outputTensor->get_raw_ptr();
    for (int i = 0; i < _selectedIndices.size(); i++) {
      const TableEvent& data = _tableData->allEvents.at(_selectedIndices[i]);
      switch (type) {
        case DATATYPE::INT32:
          ((int32_t*)tensorPtr)[i] = data.row[rowIndex]->get_int32();
          break;
        case DATATYPE::FLOAT:
          ((float*)tensorPtr)[i] = data.row[rowIndex]->get_float();
          break;
        case DATATYPE::INT64:
          ((int64_t*)tensorPtr)[i] = data.row[rowIndex]->get_int64();
          break;
        case DATATYPE::DOUBLE:
          ((double*)tensorPtr)[i] = data.row[rowIndex]->get_double();
          break;
        case DATATYPE::STRING:
          ((std::string*)tensorPtr)[i] = data.row[rowIndex]->get_string();
          break;
        case DATATYPE::BOOLEAN:
          ((bool*)tensorPtr)[i] = data.row[rowIndex]->get_bool();
          break;
        default:
          THROW("data type %s is not supported from events store.",
                util::get_string_from_enum(type));
      }
    }
    return outputTensor;
  }
  // Return a list data variable
  // for each tensor need to cast it to data type given as an argument
  std::vector<OpReturnType> members;
  for (int i = 0; i < _selectedIndices.size(); i++) {
    const TableEvent& data = _tableData->allEvents.at(_selectedIndices[i]);
    OpReturnType storedTensor = data.row[rowIndex];
    auto castedTensor = DataVariable::create_tensor(util::get_primitive_dType(type),
                                                    {(long long)storedTensor->get_numElements()});
    void* tensorPtr = castedTensor->get_raw_ptr();
    switch (type) {
      case DATATYPE::INT32_ARRAY: {
        for (int i = 0; i < storedTensor->get_numElements(); i++) {
          ((int32_t*)tensorPtr)[i] = storedTensor->get_int_subscript(i)->get_int32();
        }
        members.push_back(castedTensor);
        break;
      }
      case DATATYPE::FLOAT_ARRAY: {
        for (int i = 0; i < storedTensor->get_numElements(); i++) {
          ((float*)tensorPtr)[i] = storedTensor->get_int_subscript(i)->get_float();
        }
        members.push_back(castedTensor);
        break;
      }
      case DATATYPE::INT64_ARRAY: {
        for (int i = 0; i < storedTensor->get_numElements(); i++) {
          ((int64_t*)tensorPtr)[i] = storedTensor->get_int_subscript(i)->get_int64();
        }
        members.push_back(castedTensor);
        break;
      }
      case DATATYPE::DOUBLE_ARRAY: {
        for (int i = 0; i < storedTensor->get_numElements(); i++) {
          ((double*)tensorPtr)[i] = storedTensor->get_int_subscript(i)->get_double();
        }
        members.push_back(castedTensor);
        break;
      }
      case DATATYPE::STRING_ARRAY: {
        for (int i = 0; i < storedTensor->get_numElements(); i++) {
          ((std::string*)tensorPtr)[i] = storedTensor->get_int_subscript(i)->get_string();
        }
        members.push_back(castedTensor);
        break;
      }
      default:
        THROW("data type %s is not supported from events store.", util::get_string_from_enum(type));
    }
  }
  return OpReturnType(new ListDataVariable(std::move(members)));
}

OpReturnType FilteredDataframeVariable::feature_fetch_timestamp(OpReturnType typeArgument) {
  if (_selectedIndices.size() == 0) {
    THROW("%s", "Either no events filtered or filtering returned 0 events");
  }
  auto type = util::get_enum_from_string(typeArgument->get_string().c_str());
  auto outputTensor = DataVariable::create_tensor(type, {(long long)_selectedIndices.size()});
  void* tensorPtr = outputTensor->get_raw_ptr();
  for (int i = 0; i < _selectedIndices.size(); i++) {
    const TableEvent& data = _tableData->allEvents.at(_selectedIndices[i]);
    switch (type) {
      case DATATYPE::INT32:
        ((int32_t*)tensorPtr)[i] = (int32_t)data.timestamp;
        break;
      case DATATYPE::FLOAT:
        ((float*)tensorPtr)[i] = (float)data.timestamp;
        break;
      case DATATYPE::INT64:
        ((int64_t*)tensorPtr)[i] = (int64_t)data.timestamp;
        break;
      case DATATYPE::DOUBLE:
        ((double*)tensorPtr)[i] = (double)data.timestamp;
        break;
      case DATATYPE::STRING:
        ((std::string*)tensorPtr)[i] = std::to_string(data.timestamp);
        break;
      default:
        THROW("data type %s is not supported for fetching TIMESTAMP from events store.",
              util::get_string_from_enum(type));
    }
  }
  return outputTensor;
}

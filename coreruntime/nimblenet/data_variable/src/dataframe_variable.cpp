#include "dataframe_variable.hpp"

#include "command_center.hpp"
#include "list_data_variable.hpp"
#include "user_events_constants.hpp"
#include "user_events_manager.hpp"
#include "variable_scope.hpp"

OpReturnType TableEventDataVariable::get_string_subscript(const std::string& key) {
  if (key == "timestamp" || key == "TIMESTAMP") {
    return OpReturnType(new SingleVariable<int64_t>(_eventPtr->timestamp));
  }
  if (_headerMapPtr->find(key) == _headerMapPtr->end()) {
    THROW("key=%s not found in event", key.c_str());
  }
  int index = _headerMapPtr->at(key);
  return _eventPtr->row[index];
}

OpReturnType DataframeVariable::filter_all(const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::FEATURE_FILTER_ALL);

  return FilteredDataframeVariable::all_events(_tableStore->get_data());
}

OpReturnType DataframeVariable::events_filter_by_function(
    const std::vector<OpReturnType>& arguments, CallStack& stack) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::FEATURE_FILTER_FUNCTION);
  if (arguments[0]->get_containerType() != CONTAINERTYPE::FUNCTIONDEF) {
    THROW("filter_by_function expects argument of type function, provided : %s",
          arguments[0]->get_containerType_string());
  }
  return FilteredDataframeVariable::events_filtered_by_function(_tableStore->get_data(),
                                                                arguments[0], stack);
}

OpReturnType DataframeVariable::append(const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::APPEND);
  if (arguments[0]->get_containerType() != CONTAINERTYPE::MAP) {
    THROW("append expects argument of type Json, provided : %s",
          arguments[0]->get_containerType_string());
  }
  const auto& map = arguments[0]->get_map();
  TableRow r;
  for (const auto& [key, value] : map) {
    r.row[key] = value;
  }
  if (const auto it = map.find(usereventconstants::TimestampField); it != map.end())
    r.timestamp = it->second->get_int64();
  else
    r.timestamp = Time::get_time();
  _tableStore->add_row(r);
  return shared_from_this();
}

OpReturnType DataframeVariable::create_processor(const std::vector<OpReturnType>& arguments) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, MemberFuncType::CREATE_PROCESSOR_INIT);
  std::string dTypeString = arguments[0]->get_string();
  int dType = util::get_enum_from_string(dTypeString.c_str());
  if (dType == -1) {
    THROW("processor failed, %s is not a data type", dTypeString.c_str());
  }
  return OpReturnType(
      new PreProcessorNimbleNetVariable(_commandCenter, _tableStore, static_cast<DATATYPE>(dType)));
}

OpReturnType DataframeVariable::call_function(int memberFuncIndex,
                                              const std::vector<OpReturnType>& arguments,
                                              CallStack& stack) {
  switch (memberFuncIndex) {
    case MemberFuncType::FEATURE_FILTER_ALL:
      return filter_all(arguments);
    case MemberFuncType::FEATURE_FILTER_FUNCTION:
      return events_filter_by_function(arguments, stack);
    case MemberFuncType::APPEND:
      return append(arguments);
    case MemberFuncType::CREATE_PROCESSOR_INIT:
      return create_processor(arguments);
  }
  THROW("%s not implemented for Dataframe", DataVariable::get_member_func_string(memberFuncIndex));
}

DataframeVariable::DataframeVariable(CommandCenter* commandCenter_,
                                     const std::map<std::string, OpReturnType>& schemaMap) {
  _commandCenter = commandCenter_;
  std::map<std::string, int> schema;
  for (const auto& [key, value] : schemaMap) {
    schema[key] = util::get_enum_from_string(value->get_string().c_str());
  }
  _tableStore = std::make_shared<TableStore>(schema);
}

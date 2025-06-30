/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "table_store.hpp"

#ifdef SCRIPTING
#include "statements.hpp"
#include "variable_scope.hpp"
#endif
#include "time_manager.hpp"

void TableStore::add_row(const TableRow& r) {
  int newIndex = _tableData->allEvents.size();
  TableEvent e;
  e.timestamp = r.timestamp;
  e.row = std::vector<OpReturnType>(_tableData->columns.size(), OpReturnType(new NoneVariable()));
  for (auto& requiredColumn : _tableData->columns) {
    // Row might have extra fields which are not required by TableEvent
    auto it = r.row.find(requiredColumn);
    if (it == r.row.end()) {
      LOG_TO_CLIENT_ERROR("Event Not added to dataframe as column=%s is missing",
                          requiredColumn.c_str());
      return;
    }
    if (!verify_key(requiredColumn, it->second)) {
      return;
    }
    e.row[_tableData->columnToIdMap[requiredColumn]] = it->second;
  }
  _tableData->allEvents.push_back(e);
  for (auto preprocessor : _preprocessors) {
    _tableData->allEvents[newIndex].groups.push_back(preprocessor->get_group_from_event(e));
    preprocessor->add_event(_tableData->allEvents.size() - 1);
  }
}

BasePreProcessor* TableStore::create_preprocessor(const PreProcessorInfo& info) {
  if (_isInvalid) {
    LOG_TO_ERROR(
        "Could not create proprocessor as Event Store is invalid. Please verify "
        "enrich_event decorator for tableName=%s",
        _tableName.c_str());
    return nullptr;
  }
  BasePreProcessor* bpreprocessor = nullptr;
  int newPreprocessorId = _preprocessors.size();
  std::vector<int> groupIds;
  std::vector<int> columnIds;
  for (int i = 0; i < info.groupColumns.size(); i++) {
    if (_tableData->columnToIdMap.find(info.groupColumns[i]) == _tableData->columnToIdMap.end()) {
      LOG_TO_CLIENT_ERROR("Column %s(to group by) not present in table %s",
                          info.groupColumns[i].c_str(), info.tableName.c_str());
      return nullptr;
    }
    groupIds.push_back(_tableData->columnToIdMap[info.groupColumns[i]]);
  }
  for (int i = 0; i < info.columnsToAggregate.size(); i++) {
    std::string columnName = info.columnsToAggregate[i];
    if (_tableData->columnToIdMap.find(columnName) == _tableData->columnToIdMap.end()) {
      LOG_TO_CLIENT_ERROR("Column %s(to aggregate on) not present in table %s", columnName.c_str(),
                          info.tableName.c_str());
      return nullptr;
    }
    std::string aggregateOperator = info.aggregateOperators[i];
    auto dataTypeOfColumn = _tableData->schema[columnName];
    if (aggregateOperator != "Count" && dataTypeOfColumn == DATATYPE::STRING) {
      LOG_TO_CLIENT_ERROR("Column=%s cannot be aggregated using operator=%s", columnName.c_str(),
                          aggregateOperator.c_str());
      return nullptr;
    }
    columnIds.push_back(_tableData->columnToIdMap[info.columnsToAggregate[i]]);
  }
  try {
    bpreprocessor = new PreProcessor(newPreprocessorId, info, groupIds, columnIds, _tableData);
  } catch (...) {
    LOG_TO_CLIENT_ERROR("%s", "PreProcessor could not be created");
    return nullptr;
  }

  // preprocessor object created, adding events to preprocessor from allEvents
  for (int i = 0; i < _tableData->allEvents.size(); i++) {
    _tableData->allEvents[i].groups.push_back(
        bpreprocessor->get_group_from_event(_tableData->allEvents[i]));
    bpreprocessor->add_event(i);
  }
  _preprocessors.push_back(bpreprocessor);
  return bpreprocessor;
}

TableStore::TableStore(const std::map<std::string, int>& schema) {
  int id = 0;
  _tableData->schema = schema;
  for (const auto& it : _tableData->schema) {
    _tableData->columnToIdMap[it.first] = id++;
    _tableData->columns.push_back(it.first);
  }
}

void TableStore::update_column_meta_data(const std::string& columnName) {
  auto it1 = std::find(_tableData->columns.begin(), _tableData->columns.end(), columnName);
  if (it1 == _tableData->columns.end()) {
    _tableData->columns.push_back(columnName);
    _tableData->columnToIdMap[columnName] = _tableData->columns.size() - 1;
  }
}

#ifdef SCRIPTING

bool check_native_dataTypes(int expectedDataType, int currentDataType, const std::string& key,
                            const std::string& expectedDataTypeString,
                            const std::string& currentDataTypeString) {
  if (expectedDataType == DATATYPE::STRING && currentDataType != DATATYPE::STRING) {
    LOG_TO_CLIENT_ERROR("Key=%s in Json has invalid data type expected=%s and provided=%s",
                        key.c_str(), expectedDataTypeString.c_str(), currentDataTypeString.c_str());
    return false;
  }
  if ((expectedDataType == DATATYPE::INT32 || expectedDataType == DATATYPE::INT64) &&
      (currentDataType != DATATYPE::INT32 && currentDataType != DATATYPE::INT64)) {
    LOG_TO_CLIENT_ERROR("Key=%s in Json has invalid data type expected=%s and provided=%s",
                        key.c_str(), expectedDataTypeString.c_str(), currentDataTypeString.c_str());
    return false;
  }
  if ((expectedDataType == DATATYPE::FLOAT || expectedDataType == DATATYPE::DOUBLE) &&
      (currentDataType != DATATYPE::FLOAT && currentDataType != DATATYPE::INT32 &&
       currentDataType != DATATYPE::INT64 && currentDataType != DATATYPE::DOUBLE)) {
    LOG_TO_CLIENT_ERROR("Key=%s in Json has invalid data type expected=%s and provided=%s",
                        key.c_str(), expectedDataTypeString.c_str(), currentDataTypeString.c_str());
    return false;
  }
  if (expectedDataType == DATATYPE::BOOLEAN && currentDataType != DATATYPE::BOOLEAN) {
    LOG_TO_CLIENT_ERROR("Key=%s in Json has invalid data type expected=%s and provided=%s",
                        key.c_str(), expectedDataTypeString.c_str(), currentDataTypeString.c_str());
    return false;
  }
  return true;
}

bool TableStore::verify_key(const std::string& key, OpReturnType val) {
  int currentDataType = val->get_dataType_enum();
  auto currentContainerType = val->get_containerType();
  std::string currentDataTypeString =
      (currentContainerType == CONTAINERTYPE::VECTOR)
          ? util::get_string_from_enum(util::get_array_dataType(currentDataType))
          : util::get_string_from_enum(currentDataType);
  auto expectedDataType = _tableData->schema[key];
  std::string expectedDataTypeString = util::get_string_from_enum(expectedDataType);
  // Both data types should be either of type array or not
  if (util::is_dType_array(expectedDataType) ^ (currentContainerType == CONTAINERTYPE::VECTOR ||
                                                currentContainerType == CONTAINERTYPE::LIST)) {
    LOG_TO_CLIENT_ERROR(
        "Key=%s in Json has invalid container type expected=%s and provided=%s", key.c_str(),
        util::get_string_from_enum(util::get_containerType_from_dataType(expectedDataType)),
        util::get_string_from_enum(currentContainerType));
    return false;
  }
  if (util::is_dType_array(expectedDataType)) {
    expectedDataType = util::get_primitive_dType(expectedDataType);
  }
  // If current is a list then check all elements of list have expectedDataType
  if (currentContainerType == CONTAINERTYPE::LIST) {
    for (int i = 0; i < val->get_size(); i++) {
      int dataType = val->get_int_subscript(i)->get_dataType_enum();
      if (!check_native_dataTypes(expectedDataType, dataType, key, expectedDataTypeString,
                                  util::get_string_from_enum(dataType))) {
        return false;
      }
    }
    return true;
  }
  if (!check_native_dataTypes(expectedDataType, currentDataType, key, expectedDataTypeString,
                              currentDataTypeString)) {
    return false;
  }
  return true;
}

#endif

#ifdef TESTING
TableStore::TableStore(CommandCenter* commandCenter, const std::string& tableName,
                       const nlohmann::json& schema) {
  _tableName = tableName;
  for (auto& item : schema.items()) {
    std::string dataTypeDump = item.value().dump();
    _tableData->schema[item.key()] = util::get_enum_from_string(dataTypeDump.c_str());
    update_column_meta_data(item.key());
  }
}
#endif

TableStore::~TableStore() {
  for (auto proc : _preprocessors) {
    delete proc;
  }
}

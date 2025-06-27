/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "pre_processor.hpp"

#include "tensor_data_variable.hpp"
#include "time_based_rolling_window.hpp"
#include "util.hpp"

using namespace std;

PreProcessor::PreProcessor(int id, const PreProcessorInfo& info, const std::vector<int>& groupIds,
                           const std::vector<int>& columnIds, std::shared_ptr<TableData> tableData)
    : BasePreProcessor(id) {
  _tableData = tableData;
  _info = info;
  _groupIds = groupIds;
  _columnIds = columnIds;
  for (auto rWindow : _info.rollingWindowsInSecs) {
    _rollingWindows.push_back(new TimeBasedRollingWindow(id, _info, rWindow));
  }
  _defaultFeature =
      std::vector<T>(_info.rollingWindowsInSecs.size() * _info.columnsToAggregate.size());
  for (int i = 0; i < _defaultFeature.size(); i++) {
    _defaultFeature[i] = _info.defaultVector[i % _info.columnsToAggregate.size()];
  }
}

string PreProcessor::get_group_from_event(const TableEvent& e) {
  string group = "";
  for (auto groupId : _groupIds) {
    // magic way to create group
    group += e.row[groupId]->print() + "+";
  }
  return group;
}

bool PreProcessor::get_group_from_row(const std::vector<std::string>& row,
                                      const std::vector<bool>& columnsFilled,
                                      std::string& retGroup) {
  string group = "";
  for (auto groupId : _groupIds) {
    if (!columnsFilled[groupId]) {
      LOG_TO_CLIENT_ERROR("Could not form group for entity, groupId=%d is missing", groupId);
      return false;
    }
    // magic way to create group
    group += row[groupId] + "+";
  }
  retGroup = std::move(group);
  return true;
}

string PreProcessor::get_group_from_group_vector(const std::vector<std::string>& groupVec) {
  string group = "";
  for (auto groupVal : groupVec) {
    group += groupVal + "+";
  }
  return group;
}

std::vector<std::string> PreProcessor::get_groups_from_json(
    const nlohmann::json& preprocessorInput) {
  std::vector<std::string> groups;
  for (const auto& inputjson : preprocessorInput) {
    std::vector<std::string> row(_tableData->columns.size());
    std::vector<bool> columnFilled(_tableData->columns.size(), false);
    for (const auto& it : inputjson.items()) {
      auto key = it.key();
      auto value = it.value();
      if (_tableData->columnToIdMap.find(key) == _tableData->columnToIdMap.end()) {
        continue;
      }
      int columnIndex = _tableData->columnToIdMap[key];
      columnFilled[columnIndex] = true;
      if (value.type() == nlohmann::json::value_t::string) {
        row[columnIndex] = value.get<std::string>();
      } else {
        row[columnIndex] = value.dump();
      }
    }
    std::string group;
    if (!get_group_from_row(row, columnFilled, group)) {
      return std::vector<std::string>();
    }
    groups.push_back(group);
  }
  return groups;
}

std::shared_ptr<ModelInput> PreProcessor::get_model_input(const nlohmann::json& preprocessorInput) {
  if (_isUseless) {
    LOG_TO_ERROR("%s", "Preprocessor get_model_input failed");
    return nullptr;
  }
  auto groups = get_groups_from_json(preprocessorInput);
  if (groups.size() != preprocessorInput.size()) {
    return nullptr;
  }
  for (const auto& rollingwindow : _rollingWindows) {
    rollingwindow->update_window(_tableData->allEvents);
  }

  auto func = [this, &groups](auto typeObj) -> std::shared_ptr<ModelInput> {
    using T = decltype(typeObj);

    T* inputData = new T[groups.size() * _defaultFeature.size()];
    for (int i = 0; i < groups.size(); i++) {
      string group = groups[i];
      if (_groupWiseFeatureMap.find(group) == _groupWiseFeatureMap.end()) {
        for (int j = 0; j < _defaultFeature.size(); j++) {
          inputData[i * _defaultFeature.size() + j] = _defaultFeature[j];
        }
      } else {
        auto& vec = _groupWiseFeatureMap[group];
        for (int j = 0; j < _defaultFeature.size(); j++) {
          inputData[i * _defaultFeature.size() + j] = vec[j];
        }
      }
    }
    int length = groups.size() * _defaultFeature.size();
    return std::make_shared<ModelInput>((void*)inputData, length);
  };

  return util::call_function_for_numeric_dataType(func, _info.dataType);
}

OpReturnType PreProcessor::get_model_input_data_variable(const std::vector<std::string>& groups) {
  if (_isUseless) {
    LOG_TO_ERROR("%s", "Preprocessor get_model_input failed");
    return nullptr;
  }
  auto& allEvents = _tableData->allEvents;

  for (const auto& rollingwindow : _rollingWindows) {
    rollingwindow->update_window(allEvents);
  }

  auto func = [this, &groups](auto typeObj) -> OpReturnType {
    using T = decltype(typeObj);

    T* inputData = (T*)malloc(groups.size() * _defaultFeature.size() * sizeof(T));
    for (int i = 0; i < groups.size(); i++) {
      string group = groups[i];
      if (_groupWiseFeatureMap.find(group) == _groupWiseFeatureMap.end()) {
        for (int j = 0; j < _defaultFeature.size(); j++) {
          inputData[i * _defaultFeature.size() + j] = _defaultFeature[j];
        }
      } else {
        auto& vec = _groupWiseFeatureMap[group];
        for (int j = 0; j < _defaultFeature.size(); j++) {
          inputData[i * _defaultFeature.size() + j] = vec[j];
        }
      }
    }
    int length = groups.size() * _defaultFeature.size();
    return OpReturnType(
        new TensorVariable(inputData, _info.dataType, length, CreateTensorType::MOVE));
  };

  return util::call_function_for_numeric_dataType(func, _info.dataType);
}

OpReturnType PreProcessor::get_model_input_data_variable(
    const std::vector<std::vector<std::string>>& allGroups) {
  std::vector<std::string> groups;
  int groupsSize = get_num_of_groupBys();
  for (int i = 0; i < allGroups.size(); i++) {
    auto& groupVec = allGroups[i];
    if (groupVec.size() != groupsSize) {
      LOG_TO_CLIENT_ERROR("Expected group size=%d got %d at index %d", groupsSize, groupVec.size(),
                          i);
      return nullptr;
    }
    auto group = get_group_from_group_vector(groupVec);
    groups.push_back(group);
  }
  return get_model_input_data_variable(groups);
}

OpReturnType PreProcessor::get_model_input_data_variable(const nlohmann::json& json) {
  auto groups = get_groups_from_json(json);
  if (groups.size() != json.size()) {
    return nullptr;
  }
  return get_model_input_data_variable(groups);
}

void PreProcessor::add_event(int newEventIndex) {
  auto& allEvents = _tableData->allEvents;
  const auto& e = allEvents[newEventIndex];
  const auto& group = e.groups[_id];
  if (_groupWiseFeatureMap.find(group) == _groupWiseFeatureMap.end()) {
    _groupWiseFeatureMap[group] = std::vector<T>(_defaultFeature.size());
    _groupWiseFeatureMap[group] = _defaultFeature;
    for (int i = 0; i < _rollingWindows.size(); i++) {
      bool isSuccess = _rollingWindows[i]->create_aggregate_columns_for_group(
          group, _columnIds, _groupWiseFeatureMap[group], i * _info.columnsToAggregate.size());
      if (!isSuccess) _isUseless = true;
    }
  }
  for (auto rollingWindow : _rollingWindows) {
    rollingWindow->add_event(_tableData->allEvents, newEventIndex);
  }
}

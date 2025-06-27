/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "map_data_variable.hpp"

#include "list_data_variable.hpp"
#include "task.hpp"
#include "variable_scope.hpp"

bool MapDataVariable::get_bool() {
  std::shared_lock lock(_mutex);
  return _map.size();
}

int MapDataVariable::get_size() {
  std::shared_lock lock(_mutex);
  return _map.size();
}

void MapDataVariable::set_subscript(const OpReturnType& subscriptVal, const OpReturnType& d) {
  auto key = subscriptVal->get_string();
  std::unique_lock lock(_mutex);
  _map[key] = d;
}

void MapDataVariable::set_value_in_map(const std::string& key, const OpReturnType& d) {
  std::unique_lock lock(_mutex);
  _map[key] = d;
}

bool MapDataVariable::in(const OpReturnType& elem) {
  auto key = elem->get_string();
  std::shared_lock lock(_mutex);
  if (_map.find(key) != _map.end()) {
    return true;
  }
  return false;
}

JsonIterator* MapDataVariable::get_json_iterator() {
  std::shared_lock lock(_mutex);
  // TODO: Will have to take a lock when getting json values in IOS
  return new JsonIterator(_map.begin(), _map.end());
}

nlohmann::json MapDataVariable::to_json() const {
  auto output = nlohmann::json::object();
  std::shared_lock lock(_mutex);
  for (const auto& [key, val] : _map) {
    output[key] = val->to_json();
  }
  return output;
}

std::string MapDataVariable::to_json_str() const {
  std::string output = "{";
  std::shared_lock lock(_mutex);
  bool first = true;
  for (const auto& [key, val] : _map) {
    if (!first) {
      output += ",";
    }
    first = false;
    output += "\"" + key + "\":" + val->to_json_str();
  }
  output += "}";
  return output;
}

const std::map<std::string, OpReturnType>& MapDataVariable::get_map() {
  std::shared_lock lock(_mutex);
  return _map;
}

OpReturnType MapDataVariable::call_function(int memberFuncIndex,
                                            const std::vector<OpReturnType>& arguments,
                                            CallStack& stack) {
  switch (memberFuncIndex) {
    case MemberFuncType::POP: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, memberFuncIndex);
      std::string key = arguments[0]->get_string();
      std::unique_lock lock(_mutex);
      auto it = _map.find(key);
      if (it == _map.end()) {
        THROW("%s key not present in map.", key.c_str());
      }
      OpReturnType value = std::move(it->second);
      _map.erase(it);
      return value;
    }
    case MemberFuncType::KEYS: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::KEYS);
      OpReturnType list = OpReturnType(new ListDataVariable());
      std::shared_lock lock(_mutex);
      for (auto it : _map) {
        list->append(OpReturnType(new SingleVariable<std::string>(it.first)));
      }
      return list;
    }
  }
  THROW("%s not implemented for dict.", DataVariable::get_member_func_string(memberFuncIndex));
}

OpReturnType MapDataVariable::get_string_subscript(const std::string& key) {
  std::shared_lock lock(_mutex);
  if (_map.find(key) == _map.end()) {
    THROW("%s key not found in dict", key.c_str());
  }
  return _map[key];
}

MapDataVariable::MapDataVariable(const std::vector<OpReturnType>& keys,
                                 const std::vector<OpReturnType>& values) {
  for (int i = 0; i < keys.size(); i++) {
    std::string key = keys[i]->get_string();
    _map[key] = values[i];
  }
}

MapDataVariable::MapDataVariable(const CTensors& inputs) {
  for (int i = 0; i < inputs.numTensors; i++) {
    // Input can contain both single variables and tensor
    if (inputs.tensors[i].shapeLength == 0) {
      _map[std::string(inputs.tensors[i].name)] =
          DataVariable::create_single_variable(inputs.tensors[i]);
    } else {
      _map[std::string(inputs.tensors[i].name)] =
          DataVariable::create_tensor(inputs.tensors[i], CreateTensorType::COPY);
    }
  }
}

bool MapDataVariable::add_or_update(OpReturnType mapVariable) {
  const auto& newMap = mapVariable->get_map();
  for (auto const& it : newMap) {
    _map[it.first] = it.second;
  }
  return true;
}

void MapDataVariable::convert_to_cTensors(CTensors* cTensors) {
  std::shared_lock lock(_mutex);
  try {
    CTensor* allocatedCTensors = new CTensor[get_size()];
    int index = 0;
    for (auto const& it : _map) {
      // This is required if run_model returns a NoneVariable
      if ((it.second->get_dataType_enum() == DATATYPE::NONE)) {
        THROW(
            "Invalid output returned from the script. It should return a map with key of type "
            "string and value of type tensor. Bad variable key: %s",
            it.first.c_str());
      }

      // Remove special key
      if (strncmp(it.first.data(), Task::EXIT_STATUS_KEY, strlen(Task::EXIT_STATUS_KEY)) == 0) {
        continue;
      }

      allocatedCTensors[index] = it.second->to_cTensor(const_cast<char*>(it.first.data()),
                                                       const_cast<OpReturnType*>(&(it.second)));
      index++;
    }
    cTensors->tensors = allocatedCTensors;
    cTensors->numTensors = index;
    return;
  } catch (...) {
    THROW("%s",
          "Invalid output returned from the script. It should return a map with key of type "
          "string and value of type tensor.");
  }
}

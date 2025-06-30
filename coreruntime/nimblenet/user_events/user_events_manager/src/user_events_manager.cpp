/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "user_events_manager.hpp"

#include "command_center.hpp"
#include "config_manager.hpp"
#include "single_variable.hpp"
#ifdef SCRIPTING
#include "statements.hpp"
#include "variable_scope.hpp"
#endif
#include "util.hpp"

using namespace std;

static inline UserEventsData update_event_name_and_event(OpReturnType functionReturnValues) {
  if (functionReturnValues->get_containerType() != CONTAINERTYPE::TUPLE) {
    THROW("%s", "pre_add_event hook can only return Tuple(String, JSONEvent) ");
  }
  if (functionReturnValues->get_size() != 2) {
    THROW("%s", "Size of return tuple from pre_add_event hook should be 2");
  }
  auto eventNamePtr = functionReturnValues->get_int_subscript(0);
  if (eventNamePtr->get_dataType_enum() != DATATYPE::STRING) {
    THROW("%s", "pre_add_event hook can only return Tuple(String, JSONEvent) ");
  }
  std::string eventName = eventNamePtr->get_string();
  auto eventDataVariable = functionReturnValues->get_int_subscript(1);
  if (eventDataVariable->get_containerType() != CONTAINERTYPE::MAP) {
    THROW("%s", "pre_add_event hook can only return Tuple(String, JSONEvent) ");
  }
  return UserEventsData(eventName, eventDataVariable);
}

UserEventsManager::UserEventsManager(CommandCenter* commandCenter_, Database* database,
                                     std::shared_ptr<const Config> config)
    : _config(config), _debugMode(config->debug) {
  _database = database;
  _commandCenter = commandCenter_;
  if (!create_tables(_config)) {
    LOG_TO_CLIENT_ERROR("%s", "Could not create tables.");
  }
}

bool UserEventsManager::create_tables(const std::shared_ptr<const Config> config) {
  // If tableSchema and expiryTimeInMins are coming from config,
  // then enrich the EventsType table and delete old rows from Events table
  if (config->tableInfos.size() != 0) {
    auto tableInfos = config->tableInfos;
    for (int i = 0; i < tableInfos.size(); i++) {
      auto tableInfoJson = tableInfos[i];
      TableInfo tableInfo = jsonparser::get_from_json<TableInfo>(tableInfoJson);
      if (tableInfo.valid) {
        _rawStoreMap[tableInfo.name] =
            std::make_shared<RawStore>(_commandCenter, _database, tableInfo);
      }
    }
  }

  return true;
}

BasePreProcessor* UserEventsManager::create_preprocessor(const json& preprocessorJson,
                                                         DATATYPE dataType) {
  PreProcessorInfo info = jsonparser::get_from_json<PreProcessorInfo>(preprocessorJson);
  if (!info.valid) {
    auto preprocessorJsonString = preprocessorJson.dump();
    LOG_TO_CLIENT_ERROR("PreprocessorInfo could not be parsed for %s",
                        preprocessorJsonString.c_str());
    return nullptr;
  }
  info.dataType = dataType;
  return create_preprocessor(info);
}

BasePreProcessor* UserEventsManager::create_preprocessor(const PreProcessorInfo& info) {
  if (_rawStoreMap.find(info.tableName) == _rawStoreMap.end()) {
    LOG_TO_CLIENT_ERROR("EventStore does not exist for %s", info.tableName.c_str());
    return nullptr;
  }
  return _rawStoreMap[info.tableName]->create_processor(info);
}

std::string get_value_as_string(const json& j) {
  stringstream ss;
  if (j.is_string())
    ss << j.get<std::string>();
  else
    ss << j;
  return ss.str();
}

UserEventsData UserEventsManager::add_event(const string& eventMapJsonString,
                                            const string& eventType) {
  nlohmann::json eventMapTable;
  try {
    eventMapTable = nlohmann::json::parse(eventMapJsonString);
  } catch (json::exception& e) {
    return UserEventsData(util::nimblestatus(
        400, "Error in parsing event for table:%s with eventMap: %s with error: %s",
        eventType.c_str(), eventMapJsonString.c_str(), e.what()));

  } catch (...) {
    return UserEventsData(
        util::nimblestatus(400, "Unknown Error in parsing event for table:%s with eventMap: %s.",
                           eventType.c_str(), eventMapJsonString.c_str()));
  }

  OpReturnType eventDataVariable = DataVariable::get_map_from_json_object(std::move(eventMapTable));
  return add_event(eventDataVariable, eventType);
}

UserEventsData UserEventsManager::add_event(const OpReturnType eventDataVariable,
                                            const string& eventType) {
  auto preProcessFunc = _rawEventsTypesToPreProcess.find(eventType);
  auto updatedUserEventsData = UserEventsData(nullptr);
  if (preProcessFunc != _rawEventsTypesToPreProcess.end()) {
    // Always provide new copied stack before calling function, so that no cleanup is required
    auto funcReturn = preProcessFunc->second->execute_function(
        {OpReturnType(new SingleVariable<std::string>(eventType)), eventDataVariable});
    if (funcReturn->is_none()) {
      return UserEventsData(nullptr);
    }
    updatedUserEventsData = update_event_name_and_event(funcReturn);
  } else {
    updatedUserEventsData = UserEventsData(eventType, eventDataVariable);
  }
  // add event to persistence
  if (!_database->add_event_in_db(updatedUserEventsData.updatedEventName,
                                  updatedUserEventsData.updatedEventDataVariable)) {
    return UserEventsData(util::nimblestatus(400, "%s", "Failed to add event to DB"));
  }

  if (_rawStoreMap.find(updatedUserEventsData.updatedEventName) == _rawStoreMap.end()) {
    return updatedUserEventsData;
  }
  OpReturnType copyEventDataVariable = updatedUserEventsData.updatedEventDataVariable;
  if (_rawStoreMap[updatedUserEventsData.updatedEventName]->add_event(copyEventDataVariable)) {
    return updatedUserEventsData;
  } else {
    return UserEventsData(util::nimblestatus(400, "%s", "Unable to add data to OnAddEvent Hook"));
  }
}

void from_json(const json& j, TableInfo& tableInfo) {
  j.at("tableName").get_to(tableInfo.name);
  std::map<std::string, std::string> schema =
      j.at("schema").get<std::map<std::string, std::string>>();
  for (auto const& it : schema) {
    std::string dataType = it.second;
    std::transform(dataType.begin(), dataType.end(), dataType.begin(), ::tolower);
    if (dataType == "real") {
      tableInfo.schema[it.first] = DATATYPE::FLOAT;
    } else if (dataType == "text") {
      tableInfo.schema[it.first] = DATATYPE::STRING;
    } else if (dataType == "int") {
      tableInfo.schema[it.first] = DATATYPE::INT32;
    } else {
      LOG_TO_CLIENT_ERROR("Invalid dataType=%s provided for key=%s", dataType.c_str(),
                          it.first.c_str());
      tableInfo.valid = false;
      return;
    }
  }
  j.at("expiryInMins").get_to(tableInfo.expiryTimeInMins);
  tableInfo.valid = true;
}

#ifdef SCRIPTING

std::shared_ptr<RawStore> UserEventsManager::create_raw_store(const std::string& eventType,
                                                              const std::string& expiryType,
                                                              int expiryValue) {
  if (_rawStoreMap.find(eventType) != _rawStoreMap.end()) {
    THROW("RawStore for eventType=%s already exists", eventType.c_str());
  }
  return _rawStoreMap[eventType] = std::make_shared<RawStore>(_commandCenter, _database, eventType,
                                                              expiryType, expiryValue);
}

void UserEventsManager::script_loaded_trigger() {
  _database->delete_old_entries_from_eventsType_Table();
  int maxDBSizeAllowed = _config->maxDBSizeKBs * 1024;
  int currentDBSize;
  auto sanityStatus = _database->get_db_size(currentDBSize);
  if (sanityStatus != 0) {
    return;
  }
  if (maxDBSizeAllowed < currentDBSize) {
    LOG_TO_CLIENT_ERROR("Current DBsize=%d has exceeded maximum limit of DB size=%d", currentDBSize,
                        maxDBSizeAllowed);
    _database->set_full();
  }
}

void UserEventsManager::add_pre_event_hook(OpReturnType functionDataVariable,
                                           std::vector<std::string>&& types) {
  assert(functionDataVariable != nullptr);
  for (auto& type : types) {
    auto it = _rawEventsTypesToPreProcess.find(type);
    if (it != _rawEventsTypesToPreProcess.end()) {
      THROW(
          "Pre Process method already created for the type %s, cannot add multiple preProcessors "
          "for the same type",
          type.c_str());
    } else {
      _rawEventsTypesToPreProcess[type] = functionDataVariable;
    }
  }
}

#endif

#ifdef TESTING
bool UserEventsManager::add_eventType(const std::string& tableName, const nlohmann::json& schema) {
  if (!_database->update_eventsType_table(tableName)) {
    return false;
  }
  return true;
}

int UserEventsManager::get_count_from_eventsTable(const std::string& eventType) {
  return _database->get_count_from_eventsTable(eventType);
}
#endif
